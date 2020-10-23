/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

#include <eodos/intf/IDODO.hpp>
#include <eodos/intf/IERC20.hpp>
#include <eodos/intf/IWETH.hpp>
#include <eodos/lib/ReentrancyGuard.hpp>
#include <eodos/lib/SafeERC20.hpp>
#include <eodos/lib/SafeMath.hpp>

class IDODOZoo {
 public:
   virtual address getDODO(address baseToken, address quoteToken) = 0;
}

/**
 * @title DODO Eth Proxy
 * @author DODO Breeder
 *
 * @notice Handle ETH-WETH converting for users.
 */
class DODOEthProxy : public ReentrancyGuard {
 public:
   // ============ Functions ============
   DODOEthProxy(address dodoZoo, address payable weth) {
      _DODO_ZOO_ = dodoZoo;
      _WETH_     = weth;
   }

   void fallback() { require(msg.sender == _WETH_, "WE_SAVED_YOUR_ETH_:)"); }

   void receive() { require(msg.sender == _WETH_, "WE_SAVED_YOUR_ETH_:)"); }

   uint256 sellEthToToken(address quoteTokenAddress, uint256 ethAmount, uint256 minReceiveTokenAmount) {
      require(msg.value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(_WETH_, quoteTokenAddress);
      require(DODO != address(0), "DODO_NOT_EXIST");
      IWETH(_WETH_).deposit{value : ethAmount}();
      IWETH(_WETH_).approve(DODO, ethAmount);
      receiveTokenAmount = IDODO(DODO).sellBaseToken(ethAmount, minReceiveTokenAmount, "");
      _transferOut(quoteTokenAddress, msg.sender, receiveTokenAmount);

      return receiveTokenAmount;
   }

   uint256 buyEthWithToken(address quoteTokenAddress, uint256 ethAmount, uint256 maxPayTokenAmount) {
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(_WETH_, quoteTokenAddress);
      require(DODO != address(0), "DODO_NOT_EXIST");
      payTokenAmount = IDODO(DODO).queryBuyBaseToken(ethAmount);
      _transferIn(quoteTokenAddress, msg.sender, payTokenAmount);
      IERC20(quoteTokenAddress).safeApprove(DODO, payTokenAmount);
      IDODO(DODO).buyBaseToken(ethAmount, maxPayTokenAmount, "");
      IWETH(_WETH_).withdraw(ethAmount);
      msg.sender.transfer(ethAmount);

      return payTokenAmount;
   }

   uint256 sellTokenToEth(address baseTokenAddress, uint256 tokenAmount, uint256 minReceiveEthAmount) {
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(baseTokenAddress, _WETH_);
      require(DODO != address(0), "DODO_NOT_EXIST");
      IERC20(baseTokenAddress).safeApprove(DODO, tokenAmount);
      _transferIn(baseTokenAddress, msg.sender, tokenAmount);
      receiveEthAmount = IDODO(DODO).sellBaseToken(tokenAmount, minReceiveEthAmount, "");
      IWETH(_WETH_).withdraw(receiveEthAmount);
      msg.sender.transfer(receiveEthAmount);

      return receiveEthAmount;
   }

   uint256 buyTokenWithEth(address baseTokenAddress, uint256 tokenAmount, uint256 maxPayEthAmount) {
      require(msg.value == maxPayEthAmount, "ETH_AMOUNT_NOT_MATCH");
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(baseTokenAddress, _WETH_);
      require(DODO != address(0), "DODO_NOT_EXIST");
      payEthAmount = IDODO(DODO).queryBuyBaseToken(tokenAmount);
      IWETH(_WETH_).deposit{value : payEthAmount}();
      IWETH(_WETH_).approve(DODO, payEthAmount);
      IDODO(DODO).buyBaseToken(tokenAmount, maxPayEthAmount, "");
      _transferOut(baseTokenAddress, msg.sender, tokenAmount);
      uint256 refund = maxPayEthAmount.sub(payEthAmount);
      if (refund > 0) {
         msg.sender.transfer(refund);
      }

      return payEthAmount;
   }

   void depositEthAsBase(uint256 ethAmount, address quoteTokenAddress) {
      require(msg.value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(_WETH_, quoteTokenAddress);
      require(DODO != address(0), "DODO_NOT_EXIST");
      IWETH(_WETH_).deposit{value : ethAmount}();
      IWETH(_WETH_).approve(DODO, ethAmount);
      IDODO(DODO).depositBaseTo(msg.sender, ethAmount);
   }

   uint256 withdrawAmount withdrawEthAsBase(uint256 ethAmount, address quoteTokenAddress) {
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(_WETH_, quoteTokenAddress);
      require(DODO != address(0), "DODO_NOT_EXIST");
      address ethLpToken = IDODO(DODO)._BASE_CAPITAL_TOKEN_();

      // transfer all pool shares to proxy
      uint256 lpBalance = IERC20(ethLpToken).balanceOf(msg.sender);
      IERC20(ethLpToken).transferFrom(msg.sender, address(this), lpBalance);
      IDODO(DODO).withdrawBase(ethAmount);

      // transfer remain shares back to msg.sender
      lpBalance = IERC20(ethLpToken).balanceOf(address(this));
      IERC20(ethLpToken).transfer(msg.sender, lpBalance);

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to msg.sender
      uint256 wethAmount = IERC20(_WETH_).balanceOf(address(this));
      IWETH(_WETH_).withdraw(wethAmount);
      msg.sender.transfer(wethAmount);

      return wethAmount;
   }

   uint256 withdrawAmount withdrawAllEthAsBase(address quoteTokenAddress) {
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(_WETH_, quoteTokenAddress);
      require(DODO != address(0), "DODO_NOT_EXIST");
      address ethLpToken = IDODO(DODO)._BASE_CAPITAL_TOKEN_();

      // transfer all pool shares to proxy
      uint256 lpBalance = IERC20(ethLpToken).balanceOf(msg.sender);
      IERC20(ethLpToken).transferFrom(msg.sender, address(this), lpBalance);
      IDODO(DODO).withdrawAllBase();

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to msg.sender
      uint256 wethAmount = IERC20(_WETH_).balanceOf(address(this));
      IWETH(_WETH_).withdraw(wethAmount);
      msg.sender.transfer(wethAmount);

      return wethAmount;
   }

   void depositEthAsQuote(uint256 ethAmount, address baseTokenAddress) {
      require(msg.value == ethAmount, "ETH_AMOUNT_NOT_MATCH");
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(baseTokenAddress, _WETH_);
      require(DODO != address(0), "DODO_NOT_EXIST");
      IWETH(_WETH_).deposit{value : ethAmount}();
      IWETH(_WETH_).approve(DODO, ethAmount);
      IDODO(DODO).depositQuoteTo(msg.sender, ethAmount);
   }

   uint256 withdrawAmount withdrawEthAsQuote(uint256 ethAmount, address baseTokenAddress) {
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(baseTokenAddress, _WETH_);
      require(DODO != address(0), "DODO_NOT_EXIST");
      address ethLpToken = IDODO(DODO)._QUOTE_CAPITAL_TOKEN_();

      // transfer all pool shares to proxy
      uint256 lpBalance = IERC20(ethLpToken).balanceOf(msg.sender);
      IERC20(ethLpToken).transferFrom(msg.sender, address(this), lpBalance);
      IDODO(DODO).withdrawQuote(ethAmount);

      // transfer remain shares back to msg.sender
      lpBalance = IERC20(ethLpToken).balanceOf(address(this));
      IERC20(ethLpToken).transfer(msg.sender, lpBalance);

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to msg.sender
      uint256 wethAmount = IERC20(_WETH_).balanceOf(address(this));
      IWETH(_WETH_).withdraw(wethAmount);
      msg.sender.transfer(wethAmount);

      return wethAmount;
   }

   uint256 withdrawAmount withdrawAllEthAsQuote(address baseTokenAddress) {
      address DODO = IDODOZoo(_DODO_ZOO_).getDODO(baseTokenAddress, _WETH_);
      require(DODO != address(0), "DODO_NOT_EXIST");
      address ethLpToken = IDODO(DODO)._QUOTE_CAPITAL_TOKEN_();

      // transfer all pool shares to proxy
      uint256 lpBalance = IERC20(ethLpToken).balanceOf(msg.sender);
      IERC20(ethLpToken).transferFrom(msg.sender, address(this), lpBalance);
      IDODO(DODO).withdrawAllQuote();

      // because of withdraw penalty, withdrawAmount may not equal to ethAmount
      // query weth amount first and than transfer ETH to msg.sender
      uint256 wethAmount = IERC20(_WETH_).balanceOf(address(this));
      IWETH(_WETH_).withdraw(wethAmount);
      msg.sender.transfer(wethAmount);

      return wethAmount;
   }

   // ============ Helper Functions ============

   void _transferIn(address tokenAddress, address from, uint256 amount) {
      IERC20(tokenAddress).safeTransferFrom(from, address(this), amount);
   }

   void _transferOut(address tokenAddress, address to, uint256 amount) {
      IERC20(tokenAddress).safeTransfer(to, amount);
   }
}
