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
class Settlement : virtual public Storage {
 private:
   DODOStore& stores;
   IFactory&  factory;

 public:
   Settlement(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , factory(_factory)
       , Storage(_stores, _factory) {}
   // ============ Events ============

   // ============ Assets IN/OUT Functions ============

   void _baseTokenTransferIn(address from, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      require(add(stores._BASE_BALANCE_, amount) <= stores._BASE_BALANCE_LIMIT_, "BASE_BALANCE_LIMIT_EXCEEDED");
      //   IERC20(_BASE_TOKEN_).safeTransferFrom(from, address(this), amount);
      transfer_mgmt::static_transfer(from, stores.dodo_name, tokenamount);
      stores._BASE_BALANCE_ = add(stores._BASE_BALANCE_, amount);
   }

   void _quoteTokenTransferIn(address from, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      require(add(stores._QUOTE_BALANCE_, amount) <= stores._QUOTE_BALANCE_LIMIT_, "QUOTE_BALANCE_LIMIT_EXCEEDED");
      //   IERC20(stores._QUOTE_TOKEN_).safeTransferFrom(from, address(this), amount);
      transfer_mgmt::static_transfer(from, stores.dodo_name, tokenamount);
      stores._QUOTE_BALANCE_ = add(stores._QUOTE_BALANCE_, amount);
   }

   void _baseTokenTransferOut(address to, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      //   IERC20(_BASE_TOKEN_).safeTransfer(to, amount);
      transfer_mgmt::static_transfer(stores.dodo_name, to, tokenamount, "");
      stores._BASE_BALANCE_ = sub(stores._BASE_BALANCE_, amount);
   }

   void _quoteTokenTransferOut(address to, const extended_asset& tokenamount) {
      uint256 amount = tokenamount.quantity.amount;
      //   IERC20(_QUOTE_TOKEN_).safeTransfer(to, amount);
      transfer_mgmt::static_transfer(stores.dodo_name, to, tokenamount, "");
      stores._QUOTE_BALANCE_ = sub(stores._QUOTE_BALANCE_, amount);
   }

   void _tokenTransferDiff(int64_t diff) {
      factory.get_transfer_mgmt().transfer_diff(diff);
   }

   // ============ Donate to Liquidity Pool Functions ============

   void _donateBaseToken(uint256 amount) {
      stores._TARGET_BASE_TOKEN_AMOUNT_ = add(stores._TARGET_BASE_TOKEN_AMOUNT_, amount);
   }

   void _donateQuoteToken(uint256 amount) {
      stores._TARGET_QUOTE_TOKEN_AMOUNT_ = add(stores._TARGET_QUOTE_TOKEN_AMOUNT_, amount);
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
      stores._CLOSED_                = true;
      stores._DEPOSIT_QUOTE_ALLOWED_ = false;
      stores._DEPOSIT_BASE_ALLOWED_  = false;
      stores._TRADE_ALLOWED_         = false;
      uint256 totalBaseCapital       = getTotalBaseCapital();
      uint256 totalQuoteCapital      = getTotalQuoteCapital();

      if (stores._QUOTE_BALANCE_ > stores._TARGET_QUOTE_TOKEN_AMOUNT_) {
         uint256 spareQuote                  = sub(stores._QUOTE_BALANCE_, stores._TARGET_QUOTE_TOKEN_AMOUNT_);
         stores._BASE_CAPITAL_RECEIVE_QUOTE_ = DecimalMath::divFloor(spareQuote, totalBaseCapital);
      } else {
         stores._TARGET_QUOTE_TOKEN_AMOUNT_ = stores._QUOTE_BALANCE_;
      }

      if (stores._BASE_BALANCE_ > stores._TARGET_BASE_TOKEN_AMOUNT_) {
         uint256 spareBase                   = sub(stores._BASE_BALANCE_, stores._TARGET_BASE_TOKEN_AMOUNT_);
         stores._QUOTE_CAPITAL_RECEIVE_BASE_ = DecimalMath::divFloor(spareBase, totalQuoteCapital);
      } else {
         stores._TARGET_BASE_TOKEN_AMOUNT_ = stores._BASE_BALANCE_;
      }

      stores._R_STATUS_ = Types::RStatus::ONE;
   }

   // claim remaining assets after final settlement
   void claimAssets() {
      require(stores._CLOSED_, "DODO_NOT_CLOSED");
      require(!stores._CLAIMED_[getMsgSender()], "ALREADY_CLAIMED");
      stores._CLAIMED_[getMsgSender()] = true;

      uint256 quoteCapital = getQuoteCapitalBalanceOf(getMsgSender());
      uint256 baseCapital  = getBaseCapitalBalanceOf(getMsgSender());

      uint256 quoteAmount = 0;
      if (quoteCapital > 0) {
         quoteAmount = div(mul(stores._TARGET_QUOTE_TOKEN_AMOUNT_, quoteCapital), getTotalQuoteCapital());
      }
      uint256 baseAmount = 0;
      if (baseCapital > 0) {
         baseAmount = div(mul(stores._TARGET_BASE_TOKEN_AMOUNT_, baseCapital), getTotalBaseCapital());
      }

      stores._TARGET_QUOTE_TOKEN_AMOUNT_ = sub(stores._TARGET_QUOTE_TOKEN_AMOUNT_, quoteAmount);
      stores._TARGET_BASE_TOKEN_AMOUNT_  = sub(stores._TARGET_BASE_TOKEN_AMOUNT_, baseAmount);

      quoteAmount = add(quoteAmount, DecimalMath::mul(baseCapital, stores._BASE_CAPITAL_RECEIVE_QUOTE_));
      baseAmount  = add(baseAmount, DecimalMath::mul(quoteCapital, stores._QUOTE_CAPITAL_RECEIVE_BASE_));

      _baseTokenTransferOut(getMsgSender(), extended_asset(static_cast<uint64_t>(baseAmount), stores._BASE_TOKEN_));
      _quoteTokenTransferOut(getMsgSender(), extended_asset(static_cast<uint64_t>(quoteAmount), stores._QUOTE_TOKEN_));

      //   IDODOLpToken(stores._BASE_CAPITAL_TOKEN_).burn(getMsgSender(), baseCapital);
      //   IDODOLpToken(stores._QUOTE_CAPITAL_TOKEN_).burn(getMsgSender(), quoteCapital);

      factory.get_lptoken(getMsgSender(), stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) {
         lptoken.burn(getMsgSender(), baseCapital);
      });

      factory.get_lptoken(getMsgSender(), stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) {
         lptoken.burn(getMsgSender(), quoteCapital);
      });

      return;
   }

   // in case someone transfer to contract directly
   void retrieve(const extended_asset& tokenamount) {
      // address token, uint256 amount
      const extended_symbol token  = tokenamount.get_extended_symbol();
      uint256               amount = tokenamount.quantity.amount;
      if (token == stores._BASE_TOKEN_) {
         uint256 balance = factory.get_transfer_mgmt().get_balance(getMsgSender(), stores._BASE_TOKEN_);
         require(balance >= add(stores._BASE_BALANCE_, amount), "DODO_BASE_BALANCE_NOT_ENOUGH");
         //  require(
         //      IERC20(stores._BASE_TOKEN_).balanceOf(address(this)) >= stores._BASE_BALANCE_,amount),
         //      "DODO_BASE_BALANCE_NOT_ENOUGH");
      }
      if (token == stores._QUOTE_TOKEN_) {
         uint256 balance = factory.get_transfer_mgmt().get_balance(getMsgSender(), stores._QUOTE_TOKEN_);
         require(balance >= add(stores._QUOTE_BALANCE_, amount), "DODO_QUOTE_BALANCE_NOT_ENOUGH");
         //  require(
         //      IERC20(stores._QUOTE_TOKEN_).balanceOf(address(this)) >=
         //      stores._QUOTE_BALANCE_,amount), "DODO_QUOTE_BALANCE_NOT_ENOUGH");
      }
      //   IERC20(token).safeTransfer(getMsgSender(), amount);
      transfer_mgmt::static_transfer(factory.get_self(), getMsgSender(), tokenamount, "");
   }
};
