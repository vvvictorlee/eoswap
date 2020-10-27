/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/impl/Storage.hpp>
#include <eosdos/intf/IDODOLpToken.hpp>
#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/SafeERC20.hpp>
#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/lib/Types.hpp>

/**
 * @title Settlement
 * @author DODO Breeder
 *
 * @notice Functions for assets settlement
 */
class Settlement : virtual  public Storage {
 private:
   DODOStore& stores;
   IStorage&  storage;

 public:
   Settlement(DODOStore& _stores, IStorage& _storage)
       : stores(_stores)
       , storage(_storage)
       , Storage(_stores, _storage) {}
   // ============ Events ============

   // ============ Assets IN/OUT Functions ============

   void _baseTokenTransferIn(address from, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      require(
          add(stores.store._BASE_BALANCE_, amount) <= stores.store._BASE_BALANCE_LIMIT_, "BASE_BALANCE_LIMIT_EXCEEDED");
      //   IERC20(_BASE_TOKEN_).safeTransferFrom(from, address(this), amount);
      storage.get_transfer_mgmt().transfer(from, storage.get_self(), tokenamount, "");
      stores.store._BASE_BALANCE_ = add(stores.store._BASE_BALANCE_, amount);
   }

   void _quoteTokenTransferIn(address from, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      require(
          add(stores.store._QUOTE_BALANCE_, amount) <= stores.store._QUOTE_BALANCE_LIMIT_,
          "QUOTE_BALANCE_LIMIT_EXCEEDED");
      //   IERC20(stores.store._QUOTE_TOKEN_).safeTransferFrom(from, address(this), amount);
      storage.get_transfer_mgmt().transfer(from, storage.get_self(), tokenamount, "");
      stores.store._QUOTE_BALANCE_ = add(stores.store._QUOTE_BALANCE_, amount);
   }

   void _baseTokenTransferOut(address to, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      //   IERC20(_BASE_TOKEN_).safeTransfer(to, amount);
      storage.get_transfer_mgmt().transfer(storage.get_self(), to, tokenamount, "");
      stores.store._BASE_BALANCE_ = sub(stores.store._BASE_BALANCE_, amount);
   }

   void _quoteTokenTransferOut(address to, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      //   IERC20(_QUOTE_TOKEN_).safeTransfer(to, amount);
      storage.get_transfer_mgmt().transfer(storage.get_self(), to, tokenamount, "");
      stores.store._QUOTE_BALANCE_ = sub(stores.store._QUOTE_BALANCE_, amount);
   }

   // ============ Donate to Liquidity Pool Functions ============

   void _donateBaseToken(uint256 amount) {
      stores.store._TARGET_BASE_TOKEN_AMOUNT_ = add(stores.store._TARGET_BASE_TOKEN_AMOUNT_, amount);
   }

   void _donateQuoteToken(uint256 amount) {
      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = add(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_, amount);
   }

   void donateBaseToken(const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      _baseTokenTransferIn(getMsgSender(), tokenamount);
      _donateBaseToken(amount);
   }

   void donateQuoteToken(const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      _quoteTokenTransferIn(getMsgSender(), tokenamount);
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
         uint256 spareQuote = sub(stores.store._QUOTE_BALANCE_, stores.store._TARGET_QUOTE_TOKEN_AMOUNT_);
         stores.store._BASE_CAPITAL_RECEIVE_QUOTE_ = DecimalMath::divFloor(spareQuote, totalBaseCapital);
      } else {
         stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = stores.store._QUOTE_BALANCE_;
      }

      if (stores.store._BASE_BALANCE_ > stores.store._TARGET_BASE_TOKEN_AMOUNT_) {
         uint256 spareBase = sub(stores.store._BASE_BALANCE_, stores.store._TARGET_BASE_TOKEN_AMOUNT_);
         stores.store._QUOTE_CAPITAL_RECEIVE_BASE_ = DecimalMath::divFloor(spareBase, totalQuoteCapital);
      } else {
         stores.store._TARGET_BASE_TOKEN_AMOUNT_ = stores.store._BASE_BALANCE_;
      }

      stores.store._R_STATUS_ = Types::RStatus::ONE;
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
         quoteAmount = div(mul(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_, quoteCapital), getTotalQuoteCapital());
      }
      uint256 baseAmount = 0;
      if (baseCapital > 0) {
         baseAmount = div(mul(stores.store._TARGET_BASE_TOKEN_AMOUNT_, baseCapital), getTotalBaseCapital());
      }

      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = sub(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_, quoteAmount);
      stores.store._TARGET_BASE_TOKEN_AMOUNT_  = sub(stores.store._TARGET_BASE_TOKEN_AMOUNT_, baseAmount);

      quoteAmount = add(quoteAmount,DecimalMath::mul(baseCapital, stores.store._BASE_CAPITAL_RECEIVE_QUOTE_));
      baseAmount  = add(baseAmount,DecimalMath::mul(quoteCapital, stores.store._QUOTE_CAPITAL_RECEIVE_BASE_));

      _baseTokenTransferOut(getMsgSender(), extended_asset(baseAmount,stores.store._BASE_TOKEN_));
      _quoteTokenTransferOut(getMsgSender(), extended_asset(quoteAmount,stores.store._QUOTE_TOKEN_));

      //   IDODOLpToken(stores.store._BASE_CAPITAL_TOKEN_).burn(getMsgSender(), baseCapital);
      //   IDODOLpToken(stores.store._QUOTE_CAPITAL_TOKEN_).burn(getMsgSender(), quoteCapital);

      storage.get_lptoken(
          stores.store._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(getMsgSender(), baseCapital); });

      storage.get_lptoken(
          stores.store._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(getMsgSender(), quoteCapital); });

      return;
   }

   // in case someone transfer to contract directly
   void retrieve(const extended_asset& tokenamount) {
      // address token, uint256 amount
      const extended_symbol token  = tokenamount.get_extended_symbol();
      uint256               amount = tokenamount.quantity.amount;
      if (token == stores.store._BASE_TOKEN_) {
         asset balance = storage.get_transfer_mgmt().get_balance(storage.get_self(), stores.store._BASE_TOKEN_);
         require(balance.amount >= add(stores.store._BASE_BALANCE_,amount), "DODO_BASE_BALANCE_NOT_ENOUGH");
         //  require(
         //      IERC20(stores.store._BASE_TOKEN_).balanceOf(address(this)) >= stores.store._BASE_BALANCE_,amount),
         //      "DODO_BASE_BALANCE_NOT_ENOUGH");
      }
      if (token == stores.store._QUOTE_TOKEN_) {
         asset balance = storage.get_transfer_mgmt().get_balance(storage.get_self(), stores.store._QUOTE_TOKEN_);
         require(balance.amount >= add(stores.store._QUOTE_BALANCE_,amount), "DODO_QUOTE_BALANCE_NOT_ENOUGH");
         //  require(
         //      IERC20(stores.store._QUOTE_TOKEN_).balanceOf(address(this)) >=
         //      stores.store._QUOTE_BALANCE_,amount), "DODO_QUOTE_BALANCE_NOT_ENOUGH");
      }
      //   IERC20(token).safeTransfer(getMsgSender(), amount);
      storage.get_transfer_mgmt().transfer(storage.get_self(), getMsgSender(), tokenamount, "");
   }
};
