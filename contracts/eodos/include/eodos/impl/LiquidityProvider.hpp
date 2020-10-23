/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

#include <eodos/Pricing.hpp>
#include <eodos/Settlement.hpp>
#include <eodos/Storage.hpp>
#include <eodos/intf/IDODOLpToken.hpp>
#include <eodos/lib/DODOMath.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/Types.hpp>

/**
 * @title LiquidityProvider
 * @author DODO Breeder
 *
 * @notice Functions for liquidity provider operations
 */
class LiquidityProvider : public Storage, public Pricing, public Settlement {

   // ============ Events ============

   void depositQuoteAllowed() { require(_DEPOSIT_QUOTE_ALLOWED_, "DEPOSIT_QUOTE_NOT_ALLOWED"); }

   void depositBaseAllowed() { require(_DEPOSIT_BASE_ALLOWED_, "DEPOSIT_BASE_NOT_ALLOWED"); }

   void dodoNotClosed() { require(!_CLOSED_, "DODO_CLOSED"); }

   // ============ Routine Functions ============

   uint256 withdrawBase(uint256 amount) { return withdrawBaseTo(msg.sender, amount); }

   uint256 depositBase(uint256 amount) { return depositBaseTo(msg.sender, amount); }

   uint256 withdrawQuote(uint256 amount) { return withdrawQuoteTo(msg.sender, amount); }

   uint256 depositQuote(uint256 amount) { return depositQuoteTo(msg.sender, amount); }

   uint256 withdrawAllBase() { return withdrawAllBaseTo(msg.sender); }

   uint256 withdrawAllQuote() { return withdrawAllQuoteTo(msg.sender); }

   // ============ Deposit Functions ============

   uint256 depositQuoteTo(address to, uint256 amount) {
      preventReentrant();
      depositQuoteAllowed();
      (, uint256 quoteTarget) = getExpectedTarget();
      uint256 totalQuoteCapital = getTotalQuoteCapital();
      uint256 capital           = amount;
      if (totalQuoteCapital == 0) {
         // give remaining quote token to lp as a gift
         capital = amount.add(quoteTarget);
      } else if (quoteTarget > 0) {
         capital = amount.mul(totalQuoteCapital).div(quoteTarget);
      }

      // settlement
      _quoteTokenTransferIn(msg.sender, amount);
      _mintQuoteCapital(to, capital);
      _TARGET_QUOTE_TOKEN_AMOUNT_ = _TARGET_QUOTE_TOKEN_AMOUNT_.add(amount);

      return capital;
   }

   uint256 depositBaseTo(address to, uint256 amount) {
      preventReentrant();
      depositBaseAllowed()
          : (uint256 baseTarget, ) = getExpectedTarget();
      uint256 totalBaseCapital     = getTotalBaseCapital();
      uint256 capital              = amount;
      if (totalBaseCapital == 0) {
         // give remaining base token to lp as a gift
         capital = amount.add(baseTarget);
      } else if (baseTarget > 0) {
         capital = amount.mul(totalBaseCapital).div(baseTarget);
      }

      // settlement
      _baseTokenTransferIn(msg.sender, amount);
      _mintBaseCapital(to, capital);
      _TARGET_BASE_TOKEN_AMOUNT_ = _TARGET_BASE_TOKEN_AMOUNT_.add(amount);

      return capital;
   }

   // ============ Withdraw Functions ============

   uint256 withdrawQuoteTo(address to, uint256 amount) {
      preventReentrant();
      dodoNotClosed();
      // calculate capital
      (, uint256 quoteTarget) = getExpectedTarget();
      uint256 totalQuoteCapital = getTotalQuoteCapital();
      require(totalQuoteCapital > 0, "NO_QUOTE_LP");

      uint256 requireQuoteCapital = amount.mul(totalQuoteCapital).divCeil(quoteTarget);
      require(requireQuoteCapital <= getQuoteCapitalBalanceOf(msg.sender), "LP_QUOTE_CAPITAL_BALANCE_NOT_ENOUGH");

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawQuotePenalty(amount);
      require(penalty < amount, "PENALTY_EXCEED");

      // settlement
      _TARGET_QUOTE_TOKEN_AMOUNT_ = _TARGET_QUOTE_TOKEN_AMOUNT_.sub(amount);
      _burnQuoteCapital(msg.sender, requireQuoteCapital);
      _quoteTokenTransferOut(to, amount.sub(penalty));
      _donateQuoteToken(penalty);

      return amount.sub(penalty);
   }

   uint256 withdrawBaseTo(address to, uint256 amount) {
      preventReentrant();
      dodoNotClosed();
      // calculate capital
      (uint256 baseTarget, )   = getExpectedTarget();
      uint256 totalBaseCapital = getTotalBaseCapital();
      require(totalBaseCapital > 0, "NO_BASE_LP");

      uint256 requireBaseCapital = amount.mul(totalBaseCapital).divCeil(baseTarget);
      require(requireBaseCapital <= getBaseCapitalBalanceOf(msg.sender), "LP_BASE_CAPITAL_BALANCE_NOT_ENOUGH");

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawBasePenalty(amount);
      require(penalty <= amount, "PENALTY_EXCEED");

      // settlement
      _TARGET_BASE_TOKEN_AMOUNT_ = _TARGET_BASE_TOKEN_AMOUNT_.sub(amount);
      _burnBaseCapital(msg.sender, requireBaseCapital);
      _baseTokenTransferOut(to, amount.sub(penalty));
      _donateBaseToken(penalty);

      return amount.sub(penalty);
   }

   // ============ Withdraw all Functions ============

   uint256 withdrawAllQuoteTo(address to) {
      preventReentrant();
      dodoNotClosed();
      uint256 withdrawAmount = getLpQuoteBalance(msg.sender);
      uint256 capital        = getQuoteCapitalBalanceOf(msg.sender);

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawQuotePenalty(withdrawAmount);
      require(penalty <= withdrawAmount, "PENALTY_EXCEED");

      // settlement
      _TARGET_QUOTE_TOKEN_AMOUNT_ = _TARGET_QUOTE_TOKEN_AMOUNT_.sub(withdrawAmount);
      _burnQuoteCapital(msg.sender, capital);
      _quoteTokenTransferOut(to, withdrawAmount.sub(penalty));
      _donateQuoteToken(penalty);

      return withdrawAmount.sub(penalty);
   }

   uint256 withdrawAllBaseTo(address to) {
      preventReentrant();
      dodoNotClosed();
      uint256 withdrawAmount = getLpBaseBalance(msg.sender);
      uint256 capital        = getBaseCapitalBalanceOf(msg.sender);

      // handle penalty, penalty may exceed amount
      uint256 penalty = getWithdrawBasePenalty(withdrawAmount);
      require(penalty <= withdrawAmount, "PENALTY_EXCEED");

      // settlement
      _TARGET_BASE_TOKEN_AMOUNT_ = _TARGET_BASE_TOKEN_AMOUNT_.sub(withdrawAmount);
      _burnBaseCapital(msg.sender, capital);
      _baseTokenTransferOut(to, withdrawAmount.sub(penalty));
      _donateBaseToken(penalty);

      return withdrawAmount.sub(penalty);
   }

   // ============ Helper Functions ============

   void _mintBaseCapital(address user, uint256 amount) { IDODOLpToken(_BASE_CAPITAL_TOKEN_).mint(user, amount); }

   void _mintQuoteCapital(address user, uint256 amount) { IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).mint(user, amount); }

   void _burnBaseCapital(address user, uint256 amount) { IDODOLpToken(_BASE_CAPITAL_TOKEN_).burn(user, amount); }

   void _burnQuoteCapital(address user, uint256 amount) { IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).burn(user, amount); }

   // ============ Getter Functions ============

   uint256 lpBalance getLpBaseBalance(address lp) {
      uint256 totalBaseCapital = getTotalBaseCapital();
      (uint256 baseTarget, )   = getExpectedTarget();
      if (totalBaseCapital == 0) {
         return 0;
      }
      lpBalance = getBaseCapitalBalanceOf(lp).mul(baseTarget).div(totalBaseCapital);
      return lpBalance;
   }

   uint256 lpBalance getLpQuoteBalance(address lp) {
      uint256 totalQuoteCapital = getTotalQuoteCapital();
      (, uint256 quoteTarget) = getExpectedTarget();
      if (totalQuoteCapital == 0) {
         return 0;
      }
      lpBalance = getQuoteCapitalBalanceOf(lp).mul(quoteTarget).div(totalQuoteCapital);
      return lpBalance;
   }

   uint256 penalty getWithdrawQuotePenalty(uint256 amount) {
      require(amount <= _QUOTE_BALANCE_, "DODO_QUOTE_BALANCE_NOT_ENOUGH");
      if (_R_STATUS_ == Types.RStatus.BELOW_ONE) {
         uint256 spareBase   = _BASE_BALANCE_.sub(_TARGET_BASE_TOKEN_AMOUNT_);
         uint256 price       = getOraclePrice();
         uint256 fairAmount  = DecimalMath.mul(spareBase, price);
         uint256 targetQuote = DODOMath._SolveQuadraticFunctionForTarget(_QUOTE_BALANCE_, _K_, fairAmount);
         // if amount = _QUOTE_BALANCE_, div error
         uint256 targetQuoteWithWithdraw =
             DODOMath._SolveQuadraticFunctionForTarget(_QUOTE_BALANCE_.sub(amount), _K_, fairAmount);
         return targetQuote.sub(targetQuoteWithWithdraw.add(amount));
      } else {
         return 0;
      }
   }

   uint256 penalty getWithdrawBasePenalty(uint256 amount) {
      require(amount <= _BASE_BALANCE_, "DODO_BASE_BALANCE_NOT_ENOUGH");
      if (_R_STATUS_ == Types.RStatus.ABOVE_ONE) {
         uint256 spareQuote = _QUOTE_BALANCE_.sub(_TARGET_QUOTE_TOKEN_AMOUNT_);
         uint256 price      = getOraclePrice();
         uint256 fairAmount = DecimalMath.divFloor(spareQuote, price);
         uint256 targetBase = DODOMath._SolveQuadraticFunctionForTarget(_BASE_BALANCE_, _K_, fairAmount);
         // if amount = _BASE_BALANCE_, div error
         uint256 targetBaseWithWithdraw =
             DODOMath._SolveQuadraticFunctionForTarget(_BASE_BALANCE_.sub(amount), _K_, fairAmount);
         return targetBase.sub(targetBaseWithWithdraw.add(amount));
      } else {
         return 0;
      }
   }
}
