/*

    Copyright 2020 _dodo ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eodos/DODOZoo.hpp>
#include <eodos/intf/IDODO.hpp>
#include <eodos/intf/IERC20.hpp>
#include <eodos/intf/IWETH.hpp>
#include <eodos/lib/ReentrancyGuard.hpp>
#include <eodos/lib/SafeERC20.hpp>
#include <eodos/lib/SafeMath.hpp>
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
   name          msg_sender;
   name          self;
   DODOZoo       zoo;
   ProxyStorage& stores;

 public:
   DODOEthProxy(name _self):self(_self),zoo(_self),stores(zoo.get_storage_mgmt().get_proxy_store()), ReentrancyGuard(zoo.get_storage_mgmt().get_proxy_store().guard)
        {}
   DODOZoo& getZoo() { return zoo; }
   name     getMsgSender() { return msg_sender; }
   void     setMsgSender(name _msg_sender) {
      msg_sender = _msg_sender;
      zoo.setMsgSender(_msg_sender);
   }

   // ============ Functions ============
   void init(address dodoZoo, const extended_symbol& weth) {
      stores.proxy._DODO_ZOO_ = dodoZoo;
      stores.proxy._WETH_     = weth;
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
      zoo.get_transfer_mgmt().transfer(getMsgSender(), self, ethToken, "");
      //   IWETH(stores.proxy._WETH_).deposit();//self,ethAmount
      //   IWETH(stores.proxy._WETH_).approve(_dodo, ethAmount);
      uint256 receiveTokenAmount = 0;
      zoo.get_dodo(
          _dodo, [&](auto& dodo) { receiveTokenAmount = dodo.sellBaseToken(ethAmount, minReceiveTokenAmount, {}); });

      _transferOut(
          getMsgSender(), extended_asset(receiveTokenAmount, minReceiveToken.get_extended_symbol()));

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
      zoo.get_dodo(_dodo, [&](auto& dodo) {
         payTokenAmount = dodo.queryBuyBaseToken(ethAmount);
         _transferIn(
             getMsgSender(),
             extended_asset(payTokenAmount, maxPayToken.get_extended_symbol()));
         //  IERC20(quoteTokenAddress).safeApprove(_dodo, payTokenAmount);
         dodo.buyBaseToken(ethAmount, maxPayTokenAmount, {});
      });

      zoo.get_transfer_mgmt().transfer(self, getMsgSender(), ethToken, "");

      //   IWETH(stores.proxy._WETH_).withdraw(ethAmount);
      //   getMsgSender().transfer(ethAmount);

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
      //   IERC20(baseTokenAddress).safeApprove(_dodo, tokenAmount);
      _transferIn(getMsgSender(), baseToken);
      uint256 receiveEthAmount = 0;
      zoo.get_dodo(
          _dodo, [&](auto& dodo) { receiveEthAmount = dodo.sellBaseToken(tokenAmount, minReceiveEthAmount, {}); });

      //   IWETH(stores.proxy._WETH_).withdraw(receiveEthAmount);
      //   getMsgSender().transfer(receiveEthAmount);
      zoo.get_transfer_mgmt().transfer(
          self, getMsgSender(), extended_asset(receiveEthAmount, minReceiveEth.get_extended_symbol()), "");

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

      uint256 payEthAmount = 0;
      zoo.get_dodo(_dodo, [&](auto& dodo) {
         payEthAmount = dodo.queryBuyBaseToken(tokenAmount);
         dodo.buyBaseToken(tokenAmount, maxPayEthAmount, {});
      });

      //   IWETH(stores.proxy._WETH_).deposit{value : payEthAmount}();
      //   IWETH(stores.proxy._WETH_).approve(_dodo, payEthAmount);
      zoo.get_transfer_mgmt().transfer(
          getMsgSender(), self, extended_asset(payEthAmount, maxPayEth.get_extended_symbol()), "");

      _transferOut(getMsgSender(), baseToken);
      uint256 refund = sub(maxPayEthAmount,payEthAmount);
      if (refund > 0) {
         //  getMsgSender().transfer(refund);
         zoo.get_transfer_mgmt().transfer(
             self, getMsgSender(), extended_asset(refund, maxPayEth.get_extended_symbol()), "");
      }

      return payEthAmount;
   }

   void depositEthAsBase(const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      // uint256 ethAmount, address quoteTokenAddress
      namesym quoteTokenAddress = to_namesym(quoteToken);
      namesym ethTokenAddress   = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount         = ethtokenamount.quantity.amount;

      //require(msg_value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address _dodo = zoo.getDODO(ethTokenAddress, quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      //   IWETH(stores.proxy._WETH_).deposit{value : ethAmount}();
      //   IWETH(stores.proxy._WETH_).approve(_dodo, ethAmount);
      zoo.get_transfer_mgmt().transfer(getMsgSender(), self, ethtokenamount, "");

      zoo.get_dodo(_dodo, [&](auto& dodo) { dodo.depositBaseTo(getMsgSender(), ethAmount); });
   }

   uint256 withdrawEthAsBase(const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      // uint256 ethAmount, address quoteTokenAddress
      namesym quoteTokenAddress = to_namesym(quoteToken);
      namesym ethTokenAddress   = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount         = ethtokenamount.quantity.amount;

      address _dodo = zoo.getDODO(ethTokenAddress, quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      zoo.get_dodo(_dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._BASE_CAPITAL_TOKEN_();

         // transfer all pool shares to proxy
         //   uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
         //   IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
         asset lpbalance = transfer_mgmt::get_balance(getMsgSender(), ethLpToken);
         zoo.get_transfer_mgmt().transfer(
             getMsgSender(), self, extended_asset(lpbalance, ethLpToken.get_contract()), "");

         dodo.withdrawBase(ethAmount);
         // transfer remain shares back to getMsgSender()
         //   lpBalance = IERC20(ethLpToken).balanceOf(address(this));
         //   IERC20(ethLpToken).transfer(getMsgSender(), lpBalance);
         lpbalance = transfer_mgmt::get_balance(self, ethLpToken);
         zoo.get_transfer_mgmt().transfer(
             self, getMsgSender(), extended_asset(lpbalance, ethLpToken.get_contract()), "");
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
      //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
      //   getMsgSender().transfer(wethAmount);
      asset wethAmount = transfer_mgmt::get_balance(self, stores.proxy._WETH_);
      zoo.get_transfer_mgmt().transfer(
          self, getMsgSender(), extended_asset(wethAmount, stores.proxy._WETH_.get_contract()), "");

      return wethAmount.amount;
   }

   uint256 withdrawAllEthAsBase(const extended_symbol& quoteToken) {
      // address quoteTokenAddress
      namesym quoteTokenAddress = to_namesym(quoteToken);
      address _dodo              = zoo.getDODO(to_namesym(stores.proxy._WETH_), quoteTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      zoo.get_dodo(_dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._BASE_CAPITAL_TOKEN_();

         // transfer all pool shares to proxy
         //  uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
         //  IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
         asset lpbalance = transfer_mgmt::get_balance(getMsgSender(), ethLpToken);
         zoo.get_transfer_mgmt().transfer(
             getMsgSender(), self, extended_asset(lpbalance, ethLpToken.get_contract()), "");

         dodo.withdrawAllBase();
      });
      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
      //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
      //   getMsgSender().transfer(wethAmount);
      asset wethAmount = transfer_mgmt::get_balance(self, stores.proxy._WETH_);
      zoo.get_transfer_mgmt().transfer(
          self, getMsgSender(), extended_asset(wethAmount, stores.proxy._WETH_.get_contract()), "");

      return wethAmount.amount;
   }

   void depositEthAsQuote(const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      // uint256 ethAmount, address baseTokenAddress
      namesym baseTokenAddress = to_namesym(baseToken);
      namesym ethTokenAddress  = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount        = ethtokenamount.quantity.amount;

    //   require(msg_value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address _dodo = zoo.getDODO(baseTokenAddress, ethTokenAddress);
      require(_dodo != address(0), "DODO_NOT_EXIST");
      //   IWETH(stores.proxy._WETH_).deposit{value : ethAmount}();
      //   IWETH(stores.proxy._WETH_).approve(_dodo, ethAmount);
      zoo.get_transfer_mgmt().transfer(getMsgSender(), self, ethtokenamount, "");

      zoo.get_dodo(_dodo, [&](auto& dodo) { dodo.depositQuoteTo(getMsgSender(), ethAmount); });
   }

   uint256 withdrawEthAsQuote(const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      // uint256 ethAmount, address baseTokenAddress
      namesym baseTokenAddress = to_namesym(baseToken);
      namesym ethTokenAddress   = to_namesym(ethtokenamount.get_extended_symbol());
      uint256 ethAmount         = ethtokenamount.quantity.amount;

      address _dodo = zoo.getDODO(baseTokenAddress, to_namesym(stores.proxy._WETH_));
      require(_dodo != address(0), "DODO_NOT_EXIST");
      zoo.get_dodo(_dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._QUOTE_CAPITAL_TOKEN_();

         // transfer all pool shares to proxy
         //  uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
         //  IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
         asset lpbalance = transfer_mgmt::get_balance(getMsgSender(), ethLpToken);
         zoo.get_transfer_mgmt().transfer(
             getMsgSender(), self, extended_asset(lpbalance, ethLpToken.get_contract()), "");

         dodo.withdrawQuote(ethAmount);
         // transfer remain shares back to getMsgSender()
         //   lpBalance = IERC20(ethLpToken).balanceOf(address(this));
         //   IERC20(ethLpToken).transfer(getMsgSender(), lpBalance);
         lpbalance = transfer_mgmt::get_balance(self, ethLpToken);
         zoo.get_transfer_mgmt().transfer(
             self, getMsgSender(), extended_asset(lpbalance, ethLpToken.get_contract()), "");
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
      //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
      //   getMsgSender().transfer(wethAmount);
      asset wethAmount = transfer_mgmt::get_balance(self, stores.proxy._WETH_);
      zoo.get_transfer_mgmt().transfer(
          self, getMsgSender(), extended_asset(wethAmount, stores.proxy._WETH_.get_contract()), "");

      return wethAmount.amount;
   }

   uint256 withdrawAllEthAsQuote(const extended_symbol& baseToken) {
      // address baseTokenAddress
      namesym baseTokenAddress = to_namesym(baseToken);

      address _dodo = zoo.getDODO(baseTokenAddress, to_namesym(stores.proxy._WETH_));
      require(_dodo != address(0), "DODO_NOT_EXIST");
      zoo.get_dodo(_dodo, [&](auto& dodo) {
         const extended_symbol& ethLpToken = dodo._QUOTE_CAPITAL_TOKEN_();

         // transfer all pool shares to proxy
         //  uint256 lpBalance = IERC20(ethLpToken).balanceOf(getMsgSender());
         //  IERC20(ethLpToken).transferFrom(getMsgSender(), address(this), lpBalance);
         asset lpbalance = transfer_mgmt::get_balance(getMsgSender(), ethLpToken);
         zoo.get_transfer_mgmt().transfer(
             getMsgSender(), self, extended_asset(lpbalance, ethLpToken.get_contract()), "");

         dodo.withdrawAllQuote();
      });

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to getMsgSender()
      //   uint256 wethAmount = IERC20(stores.proxy._WETH_).balanceOf(address(this));
      //   IWETH(stores.proxy._WETH_).withdraw(wethAmount);
      //   getMsgSender().transfer(wethAmount);
      asset wethAmount = transfer_mgmt::get_balance(self, stores.proxy._WETH_);
      zoo.get_transfer_mgmt().transfer(
          self, getMsgSender(), extended_asset(wethAmount, stores.proxy._WETH_.get_contract()), "");

      return wethAmount.amount;
   }

   // ============ Helper Functions ============

   void _transferIn(address from, const extended_asset& amount) {
      // address tokenAddress, address from, uint256 amoun
      //   IERC20(tokenAddress).safeTransferFrom(from, address(this), amount);
      zoo.get_transfer_mgmt().transfer(from, self, amount, "");
   }

   void _transferOut(address to, const extended_asset& amount) {
      // address tokenAddress, address to, uint256 amount
      //   IERC20(tokenAddress).safeTransfer(to, amount);
      zoo.get_transfer_mgmt().transfer(self, to, amount, "");
   }
};
