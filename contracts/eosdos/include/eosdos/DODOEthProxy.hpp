/*

    Copyright 2020 _dodo ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/DODOZoo.hpp>
#include <eosdos/intf/IDODO.hpp>
#include <eosdos/intf/IERC20.hpp>
#include <eosdos/intf/IWETH.hpp>
#include <eosdos/lib/ReentrancyGuard.hpp>
#include <eosdos/lib/SafeERC20.hpp>
#include <eosdos/lib/SafeMath.hpp>

class IDODOZoo {
 public:
   virtual address getDODO(address baseToken, address quoteToken) = 0;
};

/**
 * @title DODO Eth Proxy
 * @author DODO Breeder
 *
 * @notice Handle ETH-WETH converting for users.
 */
class DODOEthProxy : public ReentrancyGuard {
 private:
   name           self;
   name           msg_sender;
   instance_mgmt& _instance_mgmt;
   DODOZoo&       zoo;
   ProxyStorage&  stores;

 public:
   DODOEthProxy(name _self, instance_mgmt& __instance_mgmt, DODOZoo& _zoo)
       : self(_self)
       , _instance_mgmt(__instance_mgmt)
       , zoo(_zoo)
       , stores(__instance_mgmt.get_storage_mgmt().get_proxy_store())
       , ReentrancyGuard(__instance_mgmt.get_storage_mgmt().get_proxy_store().guard) {}
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) {
      require_auth(_msg_sender);
      msg_sender = _msg_sender;
      zoo.setMsgSender(_msg_sender);
   }

   // ============ Functions ============
   void init(address dodoZoo, const extended_symbol& weth, const extended_symbol& core_symbol) {
      stores.proxy._DODO_ZOO_  = dodoZoo;
      stores.proxy._WETH_      = weth;
      stores.proxy.core_symbol = core_symbol;
   }

   //    void fallback() { require(getMsgSender() == stores.proxy._WETH_, "WE_SAVED_YOUR_ETH_:)"); }

   //    void receive() { require(getMsgSender() == stores.proxy._WETH_, "WE_SAVED_YOUR_ETH_:)"); }

   uint256 sellEthToToken(const extended_asset& ethToken, const extended_asset& minReceiveToken) {
      // address quoteTokenAddress, uint256 ethAmount, uint256 minReceiveTokenAmount
      namesym ethtoken              = to_namesym(ethToken.get_extended_symbol());
      uint256 ethAmount             = ethToken.quantity.amount;
      namesym quoteTokenAddress     = to_namesym(minReceiveToken.get_extended_symbol());
      uint256 minReceiveTokenAmount = minReceiveToken.quantity.amount;

      //   require(msg_value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address _dodo = zoo.getDODO(to_namesym(stores.proxy._WETH_), quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      /// msg.sender send msg.value payable
      transfer_mgmt::static_transfer(getMsgSender(), self, extended_asset(ethAmount, stores.proxy.core_symbol));
      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   IWETH(stores.proxy._WETH_).deposit();//self,ethAmount
         //   IWETH(stores.proxy._WETH_).approve(_dodo, ethAmount);
         token.deposit(ethAmount);
         //  token.approve(_dodo, ethAmount);
      });
      uint256 receiveTokenAmount = 0;
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         receiveTokenAmount = dodo.sellBaseToken(ethAmount, minReceiveTokenAmount, {});
      });
      _transferOut(getMsgSender(), extended_asset(receiveTokenAmount, minReceiveToken.get_extended_symbol()));
      return receiveTokenAmount;
   }

   uint256 buyEthWithToken(const extended_asset& ethToken, const extended_asset& maxPayToken) {
      // address quoteTokenAddress, uint256 ethAmount, uint256 maxPayTokenAmount
      namesym ethtoken          = to_namesym(ethToken.get_extended_symbol());
      uint256 ethAmount         = ethToken.quantity.amount;
      namesym quoteTokenAddress = to_namesym(maxPayToken.get_extended_symbol());
      uint256 maxPayTokenAmount = maxPayToken.quantity.amount;

      address _dodo = zoo.getDODO(to_namesym(stores.proxy._WETH_), quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      uint256 payTokenAmount = 0;
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         payTokenAmount = dodo.queryBuyBaseToken(ethAmount);
         _transferIn(getMsgSender(), extended_asset(payTokenAmount, maxPayToken.get_extended_symbol()));
         //  IERC20(quoteTokenAddress).safeApprove(_dodo, payTokenAmount);
         dodo.buyBaseToken(ethAmount, maxPayTokenAmount, {});
      });

      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   IWETH(stores.proxy._WETH_).withdraw(ethAmount);
         token.withdraw(ethAmount);
      });
      //   getMsgSender().transfer(ethAmount);
      transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(ethAmount, stores.proxy.core_symbol));

      return payTokenAmount;
   }

   uint256 sellTokenToEth(const extended_asset& baseToken, const extended_asset& minReceiveEth) {
      // address baseTokenAddress, uint256 tokenAmount, uint256 minReceiveEthAmount
      namesym baseTokenAddress    = to_namesym(baseToken.get_extended_symbol());
      uint256 tokenAmount         = baseToken.quantity.amount;
      namesym ethTokenAddress     = to_namesym(minReceiveEth.get_extended_symbol());
      uint256 minReceiveEthAmount = minReceiveEth.quantity.amount;

      address _dodo = zoo.getDODO(baseTokenAddress, to_namesym(stores.proxy._WETH_));
      require(_dodo != address(0), "DODO_NOT_EXIST");
      //   _instance_mgmt.get_token<WETH9>(self,baseToken.get_extended_symbol(), [&](auto& token) {
      //      //   IERC20(baseTokenAddress).safeApprove(_dodo, tokenAmount);
      //      token.approve(_dodo, tokenAmount);
      //   });
      _transferIn(getMsgSender(), baseToken);
      uint256 receiveEthAmount = 0;
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         receiveEthAmount = dodo.sellBaseToken(tokenAmount, minReceiveEthAmount, {});
      });

      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   IWETH(stores.proxy._WETH_).withdraw(receiveEthAmount);
         token.withdraw(receiveEthAmount);
      });

      //   getMsgSender().transfer(receiveEthAmount);
      transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(receiveEthAmount, stores.proxy.core_symbol));

      return receiveEthAmount;
   }

   uint256 buyTokenWithEth(const extended_asset& baseToken, const extended_asset& maxPayEth) {
      // address baseTokenAddress, uint256 tokenAmount, uint256 maxPayEthAmount
      namesym baseTokenAddress = to_namesym(baseToken.get_extended_symbol());
      uint256 tokenAmount      = baseToken.quantity.amount;
      namesym ethTokenAddress  = to_namesym(maxPayEth.get_extended_symbol());
      uint256 maxPayEthAmount  = maxPayEth.quantity.amount;

      //   require(msg_value == maxPayEthAmount, "ETH_AMOUNT_NOT_MATCH");
      address _dodo = zoo.getDODO(baseTokenAddress, to_namesym(stores.proxy._WETH_));
      require(_dodo != address(0), "DODO_NOT_EXIST");
      // msg.sender send msg.value payable
      transfer_mgmt::static_transfer(getMsgSender(), self, extended_asset(maxPayEthAmount, stores.proxy.core_symbol));

      uint256 payEthAmount = 0;
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         payEthAmount = dodo.queryBuyBaseToken(tokenAmount);

         _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
            //   IWETH(stores.proxy._WETH_).deposit{value : payEthAmount}();
            //   IWETH(stores.proxy._WETH_).approve(_dodo, payEthAmount);
            token.deposit(payEthAmount);
            //  token.approve(_dodo, payEthAmount);
         });

         dodo.buyBaseToken(tokenAmount, maxPayEthAmount, {});
      });

      _transferOut(getMsgSender(), baseToken);
      uint256 refund = sub(maxPayEthAmount, payEthAmount);

      if (refund > 0) {
         //  getMsgSender().transfer(refund);
         transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(refund, stores.proxy.core_symbol));
      }

      return payEthAmount;
   }

   void depositEthAsBase(const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      // uint256 ethAmount, address quoteTokenAddress
      namesym quoteTokenAddress = to_namesym(quoteToken);
      namesym ethTokenAddress   = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount         = ethtokenamount.quantity.amount;

      // require(msg_value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address _dodo = zoo.getDODO(ethTokenAddress, quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      // msgsender.send(msg.value);
      transfer_mgmt::static_transfer(getMsgSender(), self, extended_asset(ethAmount, stores.proxy.core_symbol));

      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   IWETH(stores.proxy._WETH_).deposit{value : ethAmount}();
         //   IWETH(stores.proxy._WETH_).approve(_dodo, ethAmount);
         token.deposit(ethAmount);
         //  token.approve(_dodo, ethAmount);
      });

      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) { dodo.depositBaseTo(getMsgSender(), ethAmount); });
   }

   uint256 withdrawEthAsBase(const extended_asset& ethtokenamount, const extended_symbol& quoteToken, uint8_t state) {
      // uint256 ethAmount, address quoteTokenAddress
      namesym quoteTokenAddress = to_namesym(quoteToken);
      namesym ethTokenAddress   = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount         = ethtokenamount.quantity.amount;

      address _dodo = zoo.getDODO(ethTokenAddress, quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      if (1 == state) {
         _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
            const extended_symbol& ethLpToken = dodo._BASE_CAPITAL_TOKEN_();
            // transfer all pool shares to proxy
            _instance_mgmt.get_lptoken(self, ethLpToken, [&](auto& token) {
               //   uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
               //   IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
               uint256 lpBalance = token.balanceOf(getMsgSender());
               token.transferFrom(getMsgSender(), self, lpBalance);
            });
         });
         return 0;
      }
      if (2 == state) {
         _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
            const extended_symbol& ethLpToken = dodo._BASE_CAPITAL_TOKEN_();

            // IDODO(DODO).withdrawBase(ethAmount);
            dodo.withdrawBase(ethAmount);
         });
         return 0;
      }

      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._BASE_CAPITAL_TOKEN_();

         // transfer remain shares back to getMsgSender()
         _instance_mgmt.get_lptoken(self, ethLpToken, [&](auto& token) {
            //   lpBalance = IERC20(ethLpToken).balanceOf(address(this));
            //   IERC20(ethLpToken).transfer(getMsgSender(), lpBalance);
            uint256 lpBalance = token.balanceOf(self);
            token.transfer(getMsgSender(), lpBalance);
         });
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      uint256 wethAmount = 0;
      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
         //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
         wethAmount = token.balanceOf(self);
         token.withdraw(wethAmount);
      });

      //   getMsgSender().transfer(wethAmount);
      transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(wethAmount, stores.proxy.core_symbol));

      return wethAmount;
   }

   uint256 withdrawAllEthAsBase(const extended_symbol& quoteToken) {
      // address quoteTokenAddress
      namesym quoteTokenAddress = to_namesym(quoteToken);
      address _dodo             = zoo.getDODO(to_namesym(stores.proxy._WETH_), quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._BASE_CAPITAL_TOKEN_();

         // transfer all pool shares to proxy
         _instance_mgmt.get_lptoken(self, ethLpToken, [&](auto& token) {
            //  uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
            //  IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
            uint256 lpBalance = token.balanceOf(getMsgSender());
            token.transfer(self, lpBalance);
         });

         dodo.withdrawAllBase();
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      uint256 wethAmount = 0;
      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
         //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
         wethAmount = token.balanceOf(self);
         token.withdraw(wethAmount);
      });
      //   getMsgSender().transfer(wethAmount);
      transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(wethAmount, stores.proxy.core_symbol));

      return wethAmount;
   }

   void depositEthAsQuote(const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      // uint256 ethAmount, address baseTokenAddress
      namesym baseTokenAddress = to_namesym(baseToken);
      namesym ethTokenAddress  = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount        = ethtokenamount.quantity.amount;

      //   require(msg_value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address _dodo = zoo.getDODO(baseTokenAddress, ethTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");

      // msg.sender payable send(msg.value)
      transfer_mgmt::static_transfer(getMsgSender(), self, extended_asset(ethAmount, stores.proxy.core_symbol));

      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   IWETH(stores.proxy._WETH_).deposit{value : ethAmount}();
         //   IWETH(stores.proxy._WETH_).approve(_dodo, ethAmount);
         token.deposit(ethAmount);
         //  token.approve(_dodo, ethAmount);
      });

      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) { dodo.depositQuoteTo(getMsgSender(), ethAmount); });
   }

   uint256 withdrawEthAsQuote(const extended_asset& ethtokenamount, const extended_symbol& baseToken, uint8_t state) {
      // uint256 ethAmount, address baseTokenAddress
      namesym baseTokenAddress = to_namesym(baseToken);
      namesym ethTokenAddress  = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount        = ethtokenamount.quantity.amount;

      address _dodo = zoo.getDODO(baseTokenAddress, to_namesym(stores.proxy._WETH_));
      require(_dodo != address(0), "DODO_NOT_EXIST");
      if (1 == state) {
         _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
            const extended_symbol& ethLpToken = dodo._QUOTE_CAPITAL_TOKEN_();

            // transfer all pool shares to proxy
            _instance_mgmt.get_lptoken(self, ethLpToken, [&](auto& token) {
               //  uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
               //  IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
               uint256 lpBalance = token.balanceOf(getMsgSender());
               token.transfer(self, lpBalance);
            });
         });
         return 0;
      }

      if (2 == state) {
         _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
            const extended_symbol& ethLpToken = dodo._QUOTE_CAPITAL_TOKEN_();
            dodo.withdrawQuote(ethAmount);
         });
         return 0;
      }

      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._QUOTE_CAPITAL_TOKEN_();

         // transfer remain shares back to getMsgSender()
         _instance_mgmt.get_lptoken(self, ethLpToken, [&](auto& token) {
            //   lpBalance = IERC20(ethLpToken).balanceOf(address(this));
            //   IERC20(ethLpToken).transfer(getMsgSender(), lpBalance);
            uint256 lpBalance = token.balanceOf(self);
            token.transfer(getMsgSender(), lpBalance);
         });
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      uint256 wethAmount = 0;
      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
         //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
         wethAmount = token.balanceOf(self);
         token.withdraw(wethAmount);
      });

      //   getMsgSender().transfer(wethAmount);
      transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(wethAmount, stores.proxy.core_symbol));

      return wethAmount;
   }

   uint256 withdrawAllEthAsQuote(const extended_symbol& baseToken) {
      // address baseTokenAddress
      namesym baseTokenAddress = to_namesym(baseToken);

      address _dodo = zoo.getDODO(baseTokenAddress, to_namesym(stores.proxy._WETH_));
      require(_dodo != address(0), "DODO_NOT_EXIST");
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._QUOTE_CAPITAL_TOKEN_();

         // transfer all pool shares to proxy
         _instance_mgmt.get_lptoken(self, ethLpToken, [&](auto& token) {
            //  uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
            //  IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
            uint256 lpBalance = token.balanceOf(getMsgSender());
            token.transferFrom(getMsgSender(), self, lpBalance);
         });

         dodo.withdrawAllQuote();
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      uint256 wethAmount = 0;
      _instance_mgmt.get_token<WETH9>(self, stores.proxy._WETH_, [&](auto& token) {
         //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
         //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
         wethAmount = token.balanceOf(self);
         token.withdraw(wethAmount);
      });

      //   getMsgSender().transfer(wethAmount);
      transfer_mgmt::static_transfer(self, getMsgSender(), extended_asset(wethAmount, stores.proxy.core_symbol));

      return wethAmount;
   }

   // ============ Helper Functions ============
   void _transferIn(address from, const extended_asset& tokenamount) {
      // address tokenAddress, address from, uint256 amount
      uint256 amount = tokenamount.quantity.amount;
      _instance_mgmt.get_token<TestERC20>(self, tokenamount.get_extended_symbol(), [&](auto& token) {
         //   IERC20(tokenAddress).safeTransferFrom(from, address(this), amount);
         token.transferFrom(from, self, amount);
      });
   }

   void _transferOut(address to, const extended_asset& tokenamount) {
      // address tokenAddress, address to, uint256 amount
      uint256 amount = tokenamount.quantity.amount;
      _instance_mgmt.get_token<TestERC20>(self, tokenamount.get_extended_symbol(), [&](auto& token) {
         //   IERC20(tokenAddress).safeTransfer(to, amount);
         token.transfer(to, amount);
      });
   }
};
