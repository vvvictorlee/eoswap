/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/impl/Storage.hpp>
#include <eosdos/lib/DODOMath.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/lib/Types.hpp>

/**
 * @title Pricing
 * @author DODO Breeder
 *
 * @notice DODO Pricing model
 */
class Pricing : virtual public Storage {
 private:
   DODOStore& stores;

 public:
   Pricing(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , Storage(_stores, _factory) {}

   // ============ R = 1 cases ============
   uint256 _ROneSellBaseToken(uint256 amount, uint256 targetQuoteTokenAmount) {
      // uint256 receiveQuoteToken ;
      uint256 i  = getOraclePrice();
      uint256 Q2 = DODOMath::_SolveQuadraticFunctionForTrade(
          targetQuoteTokenAmount, targetQuoteTokenAmount, DecimalMath::mul(i, amount), false, stores._K_);
      // in theory Q2 <= targetQuoteTokenAmount
      // however when amount is close to 0, precision problems may cause Q2 > targetQuoteTokenAmount
      DODO_DEBUG("                   _ROneSellBaseToken:i=%,Q2=%,", i, Q2 );
      return sub(targetQuoteTokenAmount, Q2);
   }

   uint256 _ROneBuyBaseToken(uint256 amount, uint256 targetBaseTokenAmount) {
      uint256 payQuoteToken = 0;
      require(amount < targetBaseTokenAmount, "DODO_BASE_BALANCE_NOT_ENOUGH");
      uint256 B2    = sub(targetBaseTokenAmount, amount);
      payQuoteToken = _RAboveIntegrate(targetBaseTokenAmount, targetBaseTokenAmount, B2);

      DODO_DEBUG("                   _ROneBuyBaseToken:amount=%,targetBaseTokenAmount=%,B2=%,payQuoteToken=%;", amount, targetBaseTokenAmount, B2,
          payQuoteToken);

      return payQuoteToken;
   }

   // ============ R < 1 cases ============

   uint256 _RBelowSellBaseToken(uint256 amount, uint256 quoteBalance, uint256 targetQuoteAmount) {
      uint256 i  = getOraclePrice();
      uint256 Q2 = DODOMath::_SolveQuadraticFunctionForTrade(
          targetQuoteAmount, quoteBalance, DecimalMath::mul(i, amount), false, stores._K_);
      DODO_DEBUG("                   _RBelowSellBaseToken:i=%,Q2=%,", i, Q2 );

      return sub(quoteBalance, Q2);
   }

   uint256 _RBelowBuyBaseToken(uint256 amount, uint256 quoteBalance, uint256 targetQuoteAmount) {
      // Here we don't require amount less than some value
      // Because it is limited at upper function
      // See Trader.queryBuyBaseToken
      uint256 i  = getOraclePrice();
      uint256 Q2 = DODOMath::_SolveQuadraticFunctionForTrade(
          targetQuoteAmount, quoteBalance, DecimalMath::mulCeil(i, amount), true, stores._K_);
      DODO_DEBUG("                   $$$$$$$$$$$$$$$$$$$$$$_RBelowBuyBaseToken:i=%,Q2=%,", i, Q2);
      return sub(Q2, quoteBalance);
   }

   uint256 _RBelowBackToOne() {
      // important: carefully design the system to make sure spareBase always greater than or equal to 0
      uint256 spareBase  = sub(stores._BASE_BALANCE_, stores._TARGET_BASE_TOKEN_AMOUNT_);
      uint256 price      = getOraclePrice();
      uint256 fairAmount = DecimalMath::mul(spareBase, price);
      DODO_DEBUG("                   *********_RBelowBackToOne:spareBase=%=,price=%=,fairAmount=%=", spareBase, price, fairAmount);

      uint256 newTargetQuote =
          DODOMath::_SolveQuadraticFunctionForTarget(stores._QUOTE_BALANCE_, stores._K_, fairAmount);

      DODO_DEBUG("                   *********_RBelowBackToOne:_QUOTE_BALANCE_=%=,=newTargetQuote=%,",stores._QUOTE_BALANCE_, newTargetQuote);

      return sub(newTargetQuote, stores._QUOTE_BALANCE_);
   }

   // ============ R > 1 cases ============

   uint256 _RAboveBuyBaseToken(uint256 amount, uint256 baseBalance, uint256 targetBaseAmount) {
      require(amount < baseBalance, "DODO_BASE_BALANCE_NOT_ENOUGH");
      uint256 B2 = sub(baseBalance, amount);
      DODO_DEBUG("                   $$$$$$$$$$$$$$$$_RAboveBuyBaseToken:amount=%,targetBaseTokenAmount=%,B2=%,;",
          amount, targetBaseAmount, B2);

      return _RAboveIntegrate(targetBaseAmount, baseBalance, B2);
   }

   uint256 _RAboveSellBaseToken(uint256 amount, uint256 baseBalance, uint256 targetBaseAmount) {
      // here we don't require B1 <= targetBaseAmount
      // Because it is limited at upper function
      // See Trader.querySellBaseToken
      uint256 B1 = add(baseBalance, amount);
      DODO_DEBUG("                   _RAboveSellBaseToken:amount=%,targetBaseAmount=%,B1=%,;",
          amount, targetBaseAmount, B1 );
      return _RAboveIntegrate(targetBaseAmount, B1, baseBalance);
   }

   uint256 _RAboveBackToOne() {
      // important: carefully design the system to make sure spareBase always greater than or equal to 0
      uint256 spareQuote = sub(stores._QUOTE_BALANCE_, stores._TARGET_QUOTE_TOKEN_AMOUNT_);
      uint256 price      = getOraclePrice();
      uint256 fairAmount = DecimalMath::divFloor(spareQuote, price);
      DODO_DEBUG("                   *********_RAboveBackToOne:spareQuote=%=,price=%=,fairAmount=%=", spareQuote, price, fairAmount);
      uint256 newTargetBase = DODOMath::_SolveQuadraticFunctionForTarget(stores._BASE_BALANCE_, stores._K_, fairAmount);
      DODO_DEBUG("                   *********_RAboveBackToOne:newTargetBase=%=,", newTargetBase);

      return sub(newTargetBase, stores._BASE_BALANCE_);
   }

   // ============ Helper functions ============
   std::tuple<uint256, uint256> getExpectedTarget() {
      uint256 baseTarget  = 0;
      uint256 quoteTarget = 0;
      uint256 Q           = stores._QUOTE_BALANCE_;
      uint256 B           = stores._BASE_BALANCE_;
      if (stores._R_STATUS_ == Types::RStatus::ONE) {
         DODO_DEBUG("                   1 getExpectedTarget:stores._TARGET_BASE_TOKEN_AMOUNT_=%=, stores._TARGET_QUOTE_TOKEN_AMOUNT_=%=,",
             stores._TARGET_BASE_TOKEN_AMOUNT_, stores._TARGET_QUOTE_TOKEN_AMOUNT_);
         return std::make_tuple(stores._TARGET_BASE_TOKEN_AMOUNT_, stores._TARGET_QUOTE_TOKEN_AMOUNT_);
      } else if (stores._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 payQuoteToken = _RBelowBackToOne();
         DODO_DEBUG("                   2 getExpectedTarget:stores._TARGET_BASE_TOKEN_AMOUNT_=%=, Q=%, payQuoteToken=%,add(Q, payQuoteToken)=%=,",
             stores._TARGET_BASE_TOKEN_AMOUNT_, Q, payQuoteToken, add(Q, payQuoteToken));
         return std::make_tuple(stores._TARGET_BASE_TOKEN_AMOUNT_, add(Q, payQuoteToken));
      } else if (stores._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 payBaseToken = _RAboveBackToOne();
         DODO_DEBUG("                   3 getExpectedTarget:stores._TARGET_QUOTE_TOKEN_AMOUNT_=%=, B=%, payBaseToken=%,add(B, payBaseToken)=%=,",
             stores._TARGET_QUOTE_TOKEN_AMOUNT_, B, payBaseToken, add(B, payBaseToken));

         return std::make_tuple(add(B, payBaseToken), stores._TARGET_QUOTE_TOKEN_AMOUNT_);
      }
      return std::make_tuple(baseTarget, quoteTarget);
   }

   uint256 getMidPrice() {
      uint256 baseTarget;
      uint256 quoteTarget;
      std::tie(baseTarget, quoteTarget) = getExpectedTarget();
      if (stores._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 R =
             DecimalMath::divFloor(div(mul(quoteTarget, quoteTarget), stores._QUOTE_BALANCE_), stores._QUOTE_BALANCE_);
         R = add(sub(DecimalMath::ONE, stores._K_), DecimalMath::mul(stores._K_, R));
         return DecimalMath::divFloor(getOraclePrice(), R);
      } else {
         uint256 R =
             DecimalMath::divFloor(div(mul(baseTarget, baseTarget), stores._BASE_BALANCE_), stores._BASE_BALANCE_);
         R = add(sub(DecimalMath::ONE, stores._K_), DecimalMath::mul(stores._K_, R));
         return DecimalMath::mul(getOraclePrice(), R);
      }
   }

   uint256 _RAboveIntegrate(uint256 B0, uint256 B1, uint256 B2) {
      uint256 i = getOraclePrice();
      DODO_DEBUG("                   3 _RAboveIntegrate:B0=%, B1=%, B2=%, i=%, stores._K_=%,",
          B0, B1, B2, i, stores._K_);
      return DODOMath::_GeneralIntegrate(B0, B1, B2, i, stores._K_);
   }

   // function _RBelowIntegrate(
   //     uint256 Q0,
   //     uint256 Q1,
   //     uint256 Q2
   // )  view returns (uint256) {
   //     uint256 i = getOraclePrice();
   //     i = DecimalMath::divFloor(DecimalMath::ONE, i); // 1/i
   //     return DODOMath::_GeneralIntegrate(Q0, Q1, Q2, i, stores._K_);
   // }
};
