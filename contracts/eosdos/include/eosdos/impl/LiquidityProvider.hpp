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
class LiquidityProvider : virtual public Storage, virtual public Pricing, virtual public Settlement {
 private:
   DODOStore& stores;
   IFactory&  factory;

 public:
   LiquidityProvider(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , factory(_factory)
       , Storage(_stores, _factory)
       , Pricing(_stores, _factory)
       , Settlement(_stores, _factory) {}
   // ============ Events ============

   void depositQuoteAllowed() { require(stores._DEPOSIT_QUOTE_ALLOWED_, "DEPOSIT_QUOTE_NOT_ALLOWED"); }

   void depositBaseAllowed() { require(stores._DEPOSIT_BASE_ALLOWED_, "DEPOSIT_BASE_NOT_ALLOWED"); }

   void dodoNotClosed() { require(!stores._CLOSED_, "DODO_CLOSED"); }

   // ============ Routine Functions ============
   uint64_t withdrawBase(uint64_t amount) { return withdrawBaseTo(getMsgSender(), amount); }

   uint64_t depositBase(uint64_t amount) { return depositBaseTo(getMsgSender(), amount); }

   uint64_t withdrawQuote(uint64_t amount) { return withdrawQuoteTo(getMsgSender(), amount); }

   virtual uint64_t depositQuote(uint64_t amount) { return depositQuoteTo(getMsgSender(), amount); }

   uint64_t withdrawAllBase() { return withdrawAllBaseTo(getMsgSender()); }

   uint64_t withdrawAllQuote() { return withdrawAllQuoteTo(getMsgSender()); }

   // ============ Deposit Functions ============
   virtual uint64_t depositQuoteTo(address to, uint64_t amount) {
      //preventReentrant();

      depositQuoteAllowed();

      uint256 quoteTarget                = 0;
      std::tie(std::ignore, quoteTarget) = getExpectedTarget();

      uint256 totalQuoteCapital = getTotalQuoteCapital();

      uint256 capital = amount;
      if (totalQuoteCapital == 0) {
         // give remaining quote token to lp as a gift
         capital = add(amount, quoteTarget);
      } else if (quoteTarget > 0) {
         capital = div(mul(amount, totalQuoteCapital), quoteTarget);
      }

      // settlement
      _quoteTokenTransferIn(getMsgSender(), extended_asset(amount, stores._QUOTE_TOKEN_));
      _mintQuoteCapital(to, capital);
      stores._TARGET_QUOTE_TOKEN_AMOUNT_ = add(stores._TARGET_QUOTE_TOKEN_AMOUNT_, amount);

      return capital;
   }

   uint64_t depositBaseTo(address to, uint64_t amount) {
      //preventReentrant();
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
      _baseTokenTransferIn(getMsgSender(), extended_asset(amount, stores._BASE_TOKEN_));

      _mintBaseCapital(to, capital);

      stores._TARGET_BASE_TOKEN_AMOUNT_ = add(stores._TARGET_BASE_TOKEN_AMOUNT_, amount);

      return capital;
   }

   // ============ Withdraw Functions ============

   uint64_t withdrawQuoteTo(address to, uint64_t amount) {
      //preventReentrant();
      dodoNotClosed();
      // calculate capital
      uint256 quoteTarget                = 0;
      std::tie(std::ignore, quoteTarget) = getExpectedTarget();
      uint256 totalQuoteCapital          = getTotalQuoteCapital();
      require(totalQuoteCapital > 0, "NO_QUOTE_LP");

      uint256 requireQuoteCapital = divCeil(mul(amount, totalQuoteCapital), quoteTarget);
      require(requireQuoteCapital <= getQuoteCapitalBalanceOf(getMsgSender()), "LP_QUOTE_CAPITAL_BALANCE_NOT_ENOUGH");

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawQuotePenalty(amount);
      require(penalty < amount, "PENALTY_EXCEED");

      // settlement
      stores._TARGET_QUOTE_TOKEN_AMOUNT_ = sub(stores._TARGET_QUOTE_TOKEN_AMOUNT_, amount);
      _burnQuoteCapital(getMsgSender(), requireQuoteCapital);
      auto amountx = extended_asset(sub(amount, penalty), stores._QUOTE_TOKEN_);
      _quoteTokenTransferOut(to, amountx,true);
      _donateQuoteToken(penalty);

      return sub(amount, penalty);
   }

   uint64_t withdrawBaseTo(address to, uint64_t amount) {
      //preventReentrant();
      dodoNotClosed();
      // calculate capital
      uint256 baseTarget                = 0;
      std::tie(baseTarget, std::ignore) = getExpectedTarget();
      uint256 totalBaseCapital          = getTotalBaseCapital();
      require(totalBaseCapital > 0, "NO_BASE_LP");
      uint256 requireBaseCapital = divCeil(mul(amount, totalBaseCapital), baseTarget);
      require(requireBaseCapital <= getBaseCapitalBalanceOf(getMsgSender()), "LP_BASE_CAPITAL_BALANCE_NOT_ENOUGH");

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawBasePenalty(amount);
      require(penalty <= amount, "PENALTY_EXCEED");

      // settlement
      stores._TARGET_BASE_TOKEN_AMOUNT_ = sub(stores._TARGET_BASE_TOKEN_AMOUNT_, amount);
      _burnBaseCapital(getMsgSender(), requireBaseCapital);
      auto amountx = extended_asset(sub(amount, penalty), stores._BASE_TOKEN_);
      _baseTokenTransferOut(to, amountx,true);
      _donateBaseToken(penalty);

      return sub(amount, penalty);
   }

   // ============ Withdraw all Functions ============

   uint64_t withdrawAllQuoteTo(address to) {
      //preventReentrant();
      dodoNotClosed();
      uint256 withdrawAmount = getLpQuoteBalance(getMsgSender());
      uint256 capital        = getQuoteCapitalBalanceOf(getMsgSender());

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawQuotePenalty(withdrawAmount);
      require(penalty <= withdrawAmount, "PENALTY_EXCEED");

      // settlement
      stores._TARGET_QUOTE_TOKEN_AMOUNT_ = sub(stores._TARGET_QUOTE_TOKEN_AMOUNT_, withdrawAmount);
      _burnQuoteCapital(getMsgSender(), capital);
      auto amountx = extended_asset(sub(withdrawAmount, penalty), stores._QUOTE_TOKEN_);
      _quoteTokenTransferOut(to, amountx,true);
      _donateQuoteToken(penalty);

      return sub(withdrawAmount, penalty);
   }

   uint64_t withdrawAllBaseTo(address to) {
      //preventReentrant();
      dodoNotClosed();
      uint256 withdrawAmount = getLpBaseBalance(getMsgSender());
      uint256 capital        = getBaseCapitalBalanceOf(getMsgSender());

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawBasePenalty(withdrawAmount);
      require(penalty <= withdrawAmount, "PENALTY_EXCEED");

      // settlement
      stores._TARGET_BASE_TOKEN_AMOUNT_ = sub(stores._TARGET_BASE_TOKEN_AMOUNT_, withdrawAmount);
      _burnBaseCapital(getMsgSender(), capital);
      auto amountx = extended_asset(sub(withdrawAmount, penalty), stores._BASE_TOKEN_);
      _baseTokenTransferOut(to, amountx,true);
      _donateBaseToken(penalty);

      return sub(withdrawAmount, penalty);
   }

   // ============ Helper Functions ============
   void _mintBaseCapital(address user, uint64_t amount) {
      factory.get_lptoken(getMsgSender(),stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.mint(user, amount); });
   }

   void _mintQuoteCapital(address user, uint64_t amount) {
      factory.get_lptoken(getMsgSender(),stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.mint(user, amount); });
   }

   void _burnBaseCapital(address user, uint64_t amount) {
      factory.get_lptoken(getMsgSender(),stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(user, amount); });
   }

   void _burnQuoteCapital(address user, uint64_t amount) {
      factory.get_lptoken(getMsgSender(),stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { lptoken.burn(user, amount); });
   }

   // ============ Getter Functions ============
   uint64_t getLpBaseBalance(address lp) {
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

   uint64_t getLpQuoteBalance(address lp) {
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

   uint64_t getWithdrawQuotePenalty(uint64_t amount) {
      require(amount <= stores._QUOTE_BALANCE_, "DODO_QUOTE_BALANCE_NOT_ENOUGH");
      if (stores._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 spareBase  = sub(stores._BASE_BALANCE_, stores._TARGET_BASE_TOKEN_AMOUNT_);
         uint256 price      = getOraclePrice();
         uint256 fairAmount = DecimalMath::mul(spareBase, price);
         uint256 targetQuote =
             DODOMath::_SolveQuadraticFunctionForTarget(stores._QUOTE_BALANCE_, stores._K_, fairAmount);
         // if amount = _QUOTE_BALANCE_, div error
         uint256 targetQuoteWithWithdraw =
             DODOMath::_SolveQuadraticFunctionForTarget(sub(stores._QUOTE_BALANCE_, amount), stores._K_, fairAmount);
        my_print_f("targetQuote=%, targetQuoteWithWithdraw=%, amount=%",targetQuote, targetQuoteWithWithdraw, amount);
         return sub(targetQuote, add(targetQuoteWithWithdraw, amount));
      }
      return 0;
   }

   uint64_t getWithdrawBasePenalty(uint64_t amount) {
      require(amount <= stores._BASE_BALANCE_, "DODO_BASE_BALANCE_NOT_ENOUGH");
      if (stores._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 spareQuote = sub(stores._QUOTE_BALANCE_, stores._TARGET_QUOTE_TOKEN_AMOUNT_);
         uint256 price      = getOraclePrice();
         uint256 fairAmount = DecimalMath::divFloor(spareQuote, price);
         uint256 targetBase = DODOMath::_SolveQuadraticFunctionForTarget(stores._BASE_BALANCE_, stores._K_, fairAmount);
         // if amount = _BASE_BALANCE_, div error
         uint256 targetBaseWithWithdraw =
             DODOMath::_SolveQuadraticFunctionForTarget(sub(stores._BASE_BALANCE_, amount), stores._K_, fairAmount);
         return sub(targetBase, add(targetBaseWithWithdraw, amount));
      }
      return 0;
   }
};
