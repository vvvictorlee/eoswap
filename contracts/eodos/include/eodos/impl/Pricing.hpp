/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once
#include <common/defines.hpp>

#include <eodos/Storage.hpp>
#include <eodos/lib/DODOMath.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/Types.hpp>

/**
 * @title Pricing
 * @author DODO Breeder
 *
 * @notice DODO Pricing model
 */
class Pricing : public Storage {
 private:
   DODOStore& stores;

 public:
   Pricing(DODOStore& _stores)
       : stores(_stores)
       , Storage(_stores) {}

   // ============ R = 1 cases ============

   uint256 receiveQuoteToken _ROneSellBaseToken(uint256 amount, uint256 targetQuoteTokenAmount) {
      uint256 i  = getOraclePrice();
      uint256 Q2 = DODOMath._SolveQuadraticFunctionForTrade(
          targetQuoteTokenAmount, targetQuoteTokenAmount, DecimalMath.mul(i, amount), false, stores.store._K_);
      // in theory Q2 <= targetQuoteTokenAmount
      // however when amount is close to 0, precision problems may cause Q2 > targetQuoteTokenAmount
      return targetQuoteTokenAmount.sub(Q2);
   }

   uint256 payQuoteToken _ROneBuyBaseToken(uint256 amount, uint256 targetBaseTokenAmount) {
      require(amount < targetBaseTokenAmount, "DODO_BASE_BALANCE_NOT_ENOUGH");
      uint256 B2    = targetBaseTokenAmount.sub(amount);
      payQuoteToken = _RAboveIntegrate(targetBaseTokenAmount, targetBaseTokenAmount, B2);
      return payQuoteToken;
   }

   // ============ R < 1 cases ============

   uint256 _RBelowSellBaseToken(uint256 amount, uint256 quoteBalance, uint256 targetQuoteAmount) {
      uint256 i  = getOraclePrice();
      uint256 Q2 = DODOMath._SolveQuadraticFunctionForTrade(
          targetQuoteAmount, quoteBalance, DecimalMath.mul(i, amount), false, stores.store._K_);
      return quoteBalance.sub(Q2);
   }

   uint256 _RBelowBuyBaseToken(uint256 amount, uint256 quoteBalance, uint256 targetQuoteAmount) {
      // Here we don't require amount less than some value
      // Because it is limited at upper function
      // See Trader.queryBuyBaseToken
      uint256 i  = getOraclePrice();
      uint256 Q2 = DODOMath._SolveQuadraticFunctionForTrade(
          targetQuoteAmount, quoteBalance, DecimalMath.mulCeil(i, amount), true, stores.store._K_);
      return Q2.sub(quoteBalance);
   }

   uint256 payQuoteToken _RBelowBackToOne() {
      // important: carefully design the system to make sure spareBase always greater than or equal to 0
      uint256 spareBase      = stores.store._BASE_BALANCE_.sub(_TARGET_BASE_TOKEN_AMOUNT_);
      uint256 price          = getOraclePrice();
      uint256 fairAmount     = DecimalMath.mul(spareBase, price);
      uint256 newTargetQuote = DODOMath._SolveQuadraticFunctionForTarget(_QUOTE_BALANCE_, stores.store._K_, fairAmount);
      return newTargetQuote.sub(_QUOTE_BALANCE_);
   }

   // ============ R > 1 cases ============

   uint256 _RAboveBuyBaseToken(uint256 amount, uint256 baseBalance, uint256 targetBaseAmount) {
      require(amount < baseBalance, "DODO_BASE_BALANCE_NOT_ENOUGH");
      uint256 B2 = baseBalance.sub(amount);
      return _RAboveIntegrate(targetBaseAmount, baseBalance, B2);
   }

   uint256 _RAboveSellBaseToken(uint256 amount, uint256 baseBalance, uint256 targetBaseAmount) {
      // here we don't require B1 <= targetBaseAmount
      // Because it is limited at upper function
      // See Trader.querySellBaseToken
      uint256 B1 = baseBalance.add(amount);
      return _RAboveIntegrate(targetBaseAmount, B1, baseBalance);
   }

   uint256 payBaseToken _RAboveBackToOne() {
      // important: carefully design the system to make sure spareBase always greater than or equal to 0
      uint256 spareQuote    = _QUOTE_BALANCE_.sub(_TARGET_QUOTE_TOKEN_AMOUNT_);
      uint256 price         = getOraclePrice();
      uint256 fairAmount    = DecimalMath.divFloor(spareQuote, price);
      uint256 newTargetBase = DODOMath._SolveQuadraticFunctionForTarget(stores.store._BASE_BALANCE_, stores.store._K_, fairAmount);
      return newTargetBase.sub(stores.store._BASE_BALANCE_);
   }

   // ============ Helper functions ============

   uint256 baseTarget, uint256 quoteTarget getExpectedTarget() {
      uint256 Q = _QUOTE_BALANCE_;
      uint256 B = stores.store._BASE_BALANCE_;
      if (_R_STATUS_ == Types::RStatus::ONE) {
         return (_TARGET_BASE_TOKEN_AMOUNT_, _TARGET_QUOTE_TOKEN_AMOUNT_);
      } else if (_R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 payQuoteToken = _RBelowBackToOne();
         return (_TARGET_BASE_TOKEN_AMOUNT_, Q.add(payQuoteToken));
      } else if (_R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 payBaseToken = _RAboveBackToOne();
         return (B.add(payBaseToken), _TARGET_QUOTE_TOKEN_AMOUNT_);
      }
   }

   uint256 midPrice getMidPrice() {
      (uint256 baseTarget, uint256 quoteTarget) = getExpectedTarget();
      if (_R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 R = DecimalMath.divFloor(quoteTarget.mul(quoteTarget).div(_QUOTE_BALANCE_), _QUOTE_BALANCE_);
         R         = DecimalMath.ONE.sub(stores.store._K_).add(DecimalMath.mul(stores.store._K_, R));
         return DecimalMath.divFloor(getOraclePrice(), R);
      } else {
         uint256 R = DecimalMath.divFloor(baseTarget.mul(baseTarget).div(stores.store._BASE_BALANCE_), stores.store._BASE_BALANCE_);
         R         = DecimalMath.ONE.sub(stores.store._K_).add(DecimalMath.mul(stores.store._K_, R));
         return DecimalMath.mul(getOraclePrice(), R);
      }
   }

   uint256 _RAboveIntegrate(uint256 B0, uint256 B1, uint256 B2) {
      uint256 i = getOraclePrice();
      return DODOMath._GeneralIntegrate(B0, B1, B2, i, stores.store._K_);
   }

   // function _RBelowIntegrate(
   //     uint256 Q0,
   //     uint256 Q1,
   //     uint256 Q2
   // )  view returns (uint256) {
   //     uint256 i = getOraclePrice();
   //     i = DecimalMath.divFloor(DecimalMath.ONE, i); // 1/i
   //     return DODOMath._GeneralIntegrate(Q0, Q1, Q2, i, stores.store._K_);
   // }
};
