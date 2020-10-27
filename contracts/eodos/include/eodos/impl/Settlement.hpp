/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eodos/Storage.hpp>
#include <eodos/intf/IDODOLpToken.hpp>
#include <eodos/intf/IERC20.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/SafeERC20.hpp>
#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/Types.hpp>

/**
 * @title Settlement
 * @author DODO Breeder
 *
 * @notice Functions for assets settlement
 */
class DODOZoo;
class Settlement : public Storage {
 private:
   DODOStore& stores;
   DODOZoo&   zoo;

 public:
   Settlement(DODOStore& _stores, DODOZoo& _zoo)
       : stores(_stores)
       , zoo(_zoo)
       , Storage(_stores) {}
   // ============ Events ============

   // ============ Assets IN/OUT Functions ============

   void _baseTokenTransferIn(address from, uint256 amount) {
      require(
          stores.store._BASE_BALANCE_.add(amount) <= stores.store._BASE_BALANCE_LIMIT_, "BASE_BALANCE_LIMIT_EXCEEDED");
      //   IERC20(_BASE_TOKEN_).safeTransferFrom(from, address(this), amount);

      stores.store._BASE_BALANCE_ = stores.store._BASE_BALANCE_.add(amount);
   }

   void _quoteTokenTransferIn(address from, uint256 amount) {
      require(
          stores.store._QUOTE_BALANCE_.add(amount) <= stores.store._QUOTE_BALANCE_LIMIT_,
          "QUOTE_BALANCE_LIMIT_EXCEEDED");
      IERC20(stores.store._QUOTE_TOKEN_).safeTransferFrom(from, address(this), amount);
      stores.store._QUOTE_BALANCE_ = stores.store._QUOTE_BALANCE_.add(amount);
   }

   void _baseTokenTransferOut(address to, uint256 amount) {
      IERC20(_BASE_TOKEN_).safeTransfer(to, amount);
      stores.store._BASE_BALANCE_ = stores.store._BASE_BALANCE_.sub(amount);
   }

   void _quoteTokenTransferOut(address to, uint256 amount) {
      IERC20(_QUOTE_TOKEN_).safeTransfer(to, amount);
      stores.store._QUOTE_BALANCE_ = stores.store._QUOTE_BALANCE_.sub(amount);
   }

   // ============ Donate to Liquidity Pool Functions ============

   void _donateBaseToken(uint256 amount) {
      stores.store._TARGET_BASE_TOKEN_AMOUNT_ = stores.store._TARGET_BASE_TOKEN_AMOUNT_.add(amount);
   }

   void _donateQuoteToken(uint256 amount) {
      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = stores.store._TARGET_QUOTE_TOKEN_AMOUNT_.add(amount);
   }

   void donateBaseToken(uint256 amount) {
      _baseTokenTransferIn(getMsgSender(), amount);
      _donateBaseToken(amount);
   }

   void donateQuoteToken(uint256 amount) {
      _quoteTokenTransferIn(getMsgSender(), amount);
      _donateQuoteToken(amount);
   }

   // ============ Final Settlement Functions ============

   // last step to shut down dodo
   void finalSettlement() {
      stores.store._CLOSED_                = true;
      stores.store._DEPOSIT_QUOTE_ALLOWED_ = false;
      stores.store._DEPOSIT_BASE_ALLOWED_  = false;
      stores.store._TRADE_ALLOWED_         = false;
      uint256 totalBaseCapital             = getTotalBaseCapital();
      uint256 totalQuoteCapital            = getTotalQuoteCapital();

      if (stores.store._QUOTE_BALANCE_ > stores.store._TARGET_QUOTE_TOKEN_AMOUNT_) {
         uint256 spareQuote = stores.store._QUOTE_BALANCE_.sub(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_);
         stores.store._BASE_CAPITAL_RECEIVE_QUOTE_ = DecimalMath.divFloor(spareQuote, totalBaseCapital);
      } else {
         stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = stores.store._QUOTE_BALANCE_;
      }

      if (stores.store._BASE_BALANCE_ > stores.store._TARGET_BASE_TOKEN_AMOUNT_) {
         uint256 spareBase = stores.store._BASE_BALANCE_.sub(stores.store._TARGET_BASE_TOKEN_AMOUNT_);
         stores.store._QUOTE_CAPITAL_RECEIVE_BASE_ = DecimalMath.divFloor(spareBase, totalQuoteCapital);
      } else {
         stores.store._TARGET_BASE_TOKEN_AMOUNT_ = stores.store._BASE_BALANCE_;
      }

      _R_STATUS_ = Types::RStatus::ONE;
   }

   // claim remaining assets after final settlement
   void claimAssets() {
      require(stores.store._CLOSED_, "DODO_NOT_CLOSED");
      require(!stores.store._CLAIMED_[getMsgSender()], "ALREADY_CLAIMED");
      stores.store._CLAIMED_[getMsgSender()] = true;

      uint256 quoteCapital = getQuoteCapitalBalanceOf(getMsgSender());
      uint256 baseCapital  = getBaseCapitalBalanceOf(getMsgSender());

      uint256 quoteAmount = 0;
      if (quoteCapital > 0) {
         quoteAmount = stores.store._TARGET_QUOTE_TOKEN_AMOUNT_.mul(quoteCapital).div(getTotalQuoteCapital());
      }
      uint256 baseAmount = 0;
      if (baseCapital > 0) {
         baseAmount = stores.store._TARGET_BASE_TOKEN_AMOUNT_.mul(baseCapital).div(getTotalBaseCapital());
      }

      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = stores.store._TARGET_QUOTE_TOKEN_AMOUNT_.sub(quoteAmount);
      stores.store._TARGET_BASE_TOKEN_AMOUNT_  = stores.store._TARGET_BASE_TOKEN_AMOUNT_.sub(baseAmount);

      quoteAmount = quoteAmount.add(DecimalMath.mul(baseCapital, stores.store._BASE_CAPITAL_RECEIVE_QUOTE_));
      baseAmount  = baseAmount.add(DecimalMath.mul(quoteCapital, stores.store._QUOTE_CAPITAL_RECEIVE_BASE_));

      _baseTokenTransferOut(getMsgSender(), baseAmount);
      _quoteTokenTransferOut(getMsgSender(), quoteAmount);

      //   IDODOLpToken(stores.store._BASE_CAPITAL_TOKEN_).burn(getMsgSender(), baseCapital);
      //   IDODOLpToken(stores.store._QUOTE_CAPITAL_TOKEN_).burn(getMsgSender(), quoteCapital);

      zoo.get_lptoken(stores.store._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(getMsgSender(), baseCapital); });

      zoo.get_lptoken(stores.store._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(getMsgSender(), quoteCapital); });

      return;
   }

   // in case someone transfer to contract directly
   void retrieve(const extended_asset& tokenamount) {
      // address token, uint256 amount
      const extended_symbol token  = tokenamount.get_extended_symbol();
      uint256               amount = tokenamount.quantity.amount;
      if (token == stores.store._BASE_TOKEN_) {
         asset balance = zoo.get_transfer_mgmt().get_balance(zoo.get_self(), stores.store._BASE_TOKEN_);
         require(balance.amount >= stores.store._BASE_BALANCE_.add(amount), "DODO_BASE_BALANCE_NOT_ENOUGH");
         //  require(
         //      IERC20(stores.store._BASE_TOKEN_).balanceOf(address(this)) >= stores.store._BASE_BALANCE_.add(amount),
         //      "DODO_BASE_BALANCE_NOT_ENOUGH");
      }
      if (token == stores.store._QUOTE_TOKEN_) {
         asset balance = zoo.get_transfer_mgmt().get_balance(zoo.get_self(), stores.store._QUOTE_TOKEN_);
         require(balance.amount >= stores.store._QUOTE_BALANCE_.add(amount), "DODO_QUOTE_BALANCE_NOT_ENOUGH");
         //  require(
         //      IERC20(stores.store._QUOTE_TOKEN_).balanceOf(address(this)) >=
         //      stores.store._QUOTE_BALANCE_.add(amount), "DODO_QUOTE_BALANCE_NOT_ENOUGH");
      }
      //   IERC20(token).safeTransfer(getMsgSender(), amount);
      zoo.get_transfer_mgmt().transfer(
          zoo.get_self(), getMsgSender(), tokenamount, "");
   }
};
