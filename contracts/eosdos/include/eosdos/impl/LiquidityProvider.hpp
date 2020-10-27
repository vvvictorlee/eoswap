/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/impl/Pricing.hpp>
#include <eosdos/impl/Settlement.hpp>
#include <eosdos/impl/Storage.hpp>
#include <eosdos/intf/IDODOLpToken.hpp>
#include <eosdos/lib/DODOMath.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/lib/Types.hpp>
/**
 * @title LiquidityProvider
 * @author DODO Breeder
 *
 * @notice Functions for liquidity provider operations
 */
class LiquidityProvider : virtual public Storage, public Pricing, public Settlement {
 private:
   DODOStore& stores;
   IStorage&  storage;
   name       getMsgSender() { return getMsgSender(); }

 public:
   LiquidityProvider(DODOStore& _stores, IStorage& _storage)
       : stores(_stores)
       , storage(_storage)
       , Storage(_stores, _storage)
       , Pricing(_stores, _storage)
       , Settlement(_stores, _storage) {}
   // ============ Events ============

   void depositQuoteAllowed() { require(stores.store._DEPOSIT_QUOTE_ALLOWED_, "DEPOSIT_QUOTE_NOT_ALLOWED"); }

   void depositBaseAllowed() { require(stores.store._DEPOSIT_BASE_ALLOWED_, "DEPOSIT_BASE_NOT_ALLOWED"); }

   void dodoNotClosed() { require(!stores.store._CLOSED_, "DODO_CLOSED"); }

   // ============ Routine Functions ============

   uint256 withdrawBase(uint256 amount) { return withdrawBaseTo(getMsgSender(), amount); }

   uint256 depositBase(uint256 amount) { return depositBaseTo(getMsgSender(), amount); }

   uint256 withdrawQuote(uint256 amount) { return withdrawQuoteTo(getMsgSender(), amount); }

   uint256 depositQuote(uint256 amount) { return depositQuoteTo(getMsgSender(), amount); }

   uint256 withdrawAllBase() { return withdrawAllBaseTo(getMsgSender()); }

   uint256 withdrawAllQuote() { return withdrawAllQuoteTo(getMsgSender()); }

   // ============ Deposit Functions ============

   uint256 depositQuoteTo(address to, uint256 amount) {
      preventReentrant();
      depositQuoteAllowed();
      uint256 quoteTarget                = 0;
      std::tie(std::ignore, quoteTarget) = getExpectedTarget();
      uint256 totalQuoteCapital          = getTotalQuoteCapital();
      uint256 capital                    = amount;
      if (totalQuoteCapital == 0) {
         // give remaining quote token to lp as a gift
         capital = add(amount, quoteTarget);
      } else if (quoteTarget > 0) {
         capital = div(mul(amount, totalQuoteCapital), quoteTarget);
      }

      // settlement
      _quoteTokenTransferIn(getMsgSender(), extended_asset(amount, stores.store._QUOTE_TOKEN_));
      _mintQuoteCapital(to, capital);
      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = add(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_, amount);

      return capital;
   }

   uint256 depositBaseTo(address to, uint256 amount) {
      preventReentrant();
      depositBaseAllowed();
      uint256 baseTarget                = 0;
      std::tie(baseTarget, std::ignore) = getExpectedTarget();
      uint256 totalBaseCapital          = getTotalBaseCapital();
      uint256 capital                   = amount;
      if (totalBaseCapital == 0) {
         // give remaining base token to lp as a gift
         capital = add(amount, baseTarget);
      } else if (baseTarget > 0) {
         capital = div(mul(amount, totalBaseCapital), baseTarget);
      }

      // settlement
      _baseTokenTransferIn(getMsgSender(), extended_asset(amount, stores.store._BASE_TOKEN_));
      _mintBaseCapital(to, capital);
      stores.store._TARGET_BASE_TOKEN_AMOUNT_ = add(stores.store._TARGET_BASE_TOKEN_AMOUNT_, amount);

      return capital;
   }

   // ============ Withdraw Functions ============

   uint256 withdrawQuoteTo(address to, uint256 amount) {
      preventReentrant();
      dodoNotClosed();
      // calculate capital
      uint256 quoteTarget                = 0;
      std::tie(std::ignore, quoteTarget) = getExpectedTarget();
      uint256 totalQuoteCapital          = getTotalQuoteCapital();
      require(totalQuoteCapital > 0, "NO_QUOTE_LP");

      uint256 requireQuoteCapital = DecimalMath::divCeil(mul(amount, totalQuoteCapital), quoteTarget);
      require(requireQuoteCapital <= getQuoteCapitalBalanceOf(getMsgSender()), "LP_QUOTE_CAPITAL_BALANCE_NOT_ENOUGH");

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawQuotePenalty(amount);
      require(penalty < amount, "PENALTY_EXCEED");

      // settlement
      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = sub(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_, amount);
      _burnQuoteCapital(getMsgSender(), requireQuoteCapital);
      _quoteTokenTransferOut(to, extended_asset(sub(amount, penalty), stores.store._QUOTE_TOKEN_));
      _donateQuoteToken(penalty);

      return sub(amount, penalty);
   }

   uint256 withdrawBaseTo(address to, uint256 amount) {
      preventReentrant();
      dodoNotClosed();
      // calculate capital
      uint256 baseTarget                = 0;
      std::tie(baseTarget, std::ignore) = getExpectedTarget();
      uint256 totalBaseCapital          = getTotalBaseCapital();
      require(totalBaseCapital > 0, "NO_BASE_LP");

      uint256 requireBaseCapital = DecimalMath::divCeil(mul(amount, totalBaseCapital), baseTarget);
      require(requireBaseCapital <= getBaseCapitalBalanceOf(getMsgSender()), "LP_BASE_CAPITAL_BALANCE_NOT_ENOUGH");

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawBasePenalty(amount);
      require(penalty <= amount, "PENALTY_EXCEED");

      // settlement
      stores.store._TARGET_BASE_TOKEN_AMOUNT_ = sub(stores.store._TARGET_BASE_TOKEN_AMOUNT_, amount);
      _burnBaseCapital(getMsgSender(), requireBaseCapital);
      _baseTokenTransferOut(to, extended_asset(sub(amount, penalty), stores.store._BASE_TOKEN_));
      _donateBaseToken(penalty);

      return sub(amount, penalty);
   }

   // ============ Withdraw all Functions ============

   uint256 withdrawAllQuoteTo(address to) {
      preventReentrant();
      dodoNotClosed();
      uint256 withdrawAmount = getLpQuoteBalance(getMsgSender());
      uint256 capital        = getQuoteCapitalBalanceOf(getMsgSender());

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawQuotePenalty(withdrawAmount);
      require(penalty <= withdrawAmount, "PENALTY_EXCEED");

      // settlement
      stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = sub(stores.store._TARGET_QUOTE_TOKEN_AMOUNT_, withdrawAmount);
      _burnQuoteCapital(getMsgSender(), capital);
      _quoteTokenTransferOut(to, extended_asset(sub(withdrawAmount, penalty), stores.store._QUOTE_TOKEN_));
      _donateQuoteToken(penalty);

      return sub(withdrawAmount, penalty);
   }

   uint256 withdrawAllBaseTo(address to) {
      preventReentrant();
      dodoNotClosed();
      uint256 withdrawAmount = getLpBaseBalance(getMsgSender());
      uint256 capital        = getBaseCapitalBalanceOf(getMsgSender());

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawBasePenalty(withdrawAmount);
      require(penalty <= withdrawAmount, "PENALTY_EXCEED");

      // settlement
      stores.store._TARGET_BASE_TOKEN_AMOUNT_ = sub(stores.store._TARGET_BASE_TOKEN_AMOUNT_, withdrawAmount);
      _burnBaseCapital(getMsgSender(), capital);
      _baseTokenTransferOut(to, extended_asset(sub(withdrawAmount, penalty), stores.store._BASE_TOKEN_));
      _donateBaseToken(penalty);

      return sub(withdrawAmount, penalty);
   }

   // ============ Helper Functions ============
   void _mintBaseCapital(address user, uint256 amount) {
      storage.get_lptoken(stores.store._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.mint(user, amount); });
   }

   void _mintQuoteCapital(address user, uint256 amount) {
      storage.get_lptoken(stores.store._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.mint(user, amount); });
   }

   void _burnBaseCapital(address user, uint256 amount) {
      storage.get_lptoken(stores.store._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(user, amount); });
   }

   void _burnQuoteCapital(address user, uint256 amount) {
      storage.get_lptoken(stores.store._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(user, amount); });
   }

   // ============ Getter Functions ============

   uint256 getLpBaseBalance(address lp) {
      uint256 lpBalance        = 0;
      uint256 totalBaseCapital = getTotalBaseCapital();
      uint256 baseTarget;
      std::tie(baseTarget, std::ignore) = getExpectedTarget();
      if (totalBaseCapital == 0) {
         return 0;
      }
      lpBalance = div(mul(getBaseCapitalBalanceOf(lp), baseTarget), totalBaseCapital);
      return lpBalance;
   }

   uint256 getLpQuoteBalance(address lp) {
      uint256 lpBalance                  = 0;
      uint256 totalQuoteCapital          = getTotalQuoteCapital();
      uint256 quoteTarget                = 0;
      std::tie(std::ignore, quoteTarget) = getExpectedTarget();
      if (totalQuoteCapital == 0) {
         return 0;
      }
      lpBalance = div(mul(getQuoteCapitalBalanceOf(lp), quoteTarget), totalQuoteCapital);
      return lpBalance;
   }

   uint256 getWithdrawQuotePenalty(uint256 amount) {
      require(amount <= stores.store._QUOTE_BALANCE_, "DODO_QUOTE_BALANCE_NOT_ENOUGH");
      if (stores.store._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 spareBase  = sub(stores.store._BASE_BALANCE_, stores.store._TARGET_BASE_TOKEN_AMOUNT_);
         uint256 price      = getOraclePrice();
         uint256 fairAmount = DecimalMath::mul(spareBase, price);
         uint256 targetQuote =
             DODOMath::_SolveQuadraticFunctionForTarget(stores.store._QUOTE_BALANCE_, stores.store._K_, fairAmount);
         // if amount = _QUOTE_BALANCE_, div error
         uint256 targetQuoteWithWithdraw = DODOMath::_SolveQuadraticFunctionForTarget(
             sub(stores.store._QUOTE_BALANCE_, amount), stores.store._K_, fairAmount);
         return sub(targetQuote, add(targetQuoteWithWithdraw, amount));
      }
      return 0;
   }

   uint256 getWithdrawBasePenalty(uint256 amount) {
      require(amount <= stores.store._BASE_BALANCE_, "DODO_BASE_BALANCE_NOT_ENOUGH");
      if (stores.store._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 spareQuote = sub(stores.store._QUOTE_BALANCE_, stores.store._TARGET_QUOTE_TOKEN_AMOUNT_);
         uint256 price      = getOraclePrice();
         uint256 fairAmount = DecimalMath::divFloor(spareQuote, price);
         uint256 targetBase =
             DODOMath::_SolveQuadraticFunctionForTarget(stores.store._BASE_BALANCE_, stores.store._K_, fairAmount);
         // if amount = _BASE_BALANCE_, div error
         uint256 targetBaseWithWithdraw = DODOMath::_SolveQuadraticFunctionForTarget(
             sub(stores.store._BASE_BALANCE_, amount), stores.store._K_, fairAmount);
         return sub(targetBase, add(targetBaseWithWithdraw, amount));
      }
      return 0;
   }
};
