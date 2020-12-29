/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/SafeMath.hpp>
using namespace SafeMath;

/**
 * @title DODOMath
 * @author DODO Breeder
 *
 * @notice Functions for complex calculating. Including ONE Integration and TWO Quadratic solutions
 */
namespace DODOMath {

/*
    Integrate dodo curve fron V1 to V2
    require V0>=V1>=V2>0
    res = (1-k)i(V1-V2)+ikV0*V0(1/V2-1/V1)
    let V1-V2=delta
    res = i*delta*(1-k+k(V0^2/V1/V2))
*/
uint256 _GeneralIntegrate(uint256 V0, uint256 V1, uint256 V2, uint256 i, uint256 k) {
   my_print_f(">>>>>>3 _GeneralIntegrate:V0=%=,  V1=%=,  V2=%=,  i=%=,  k=%=,", V0, V1, V2, i, k);

   uint256 fairAmount = DecimalMath::mul(i, sub(V1, V2)); // i*delta
   uint256 V0V0V1V2   = DecimalMath::divCeil(div(SafeMath::mul(V0, V0), V1), V2);
   uint256 penalty    = DecimalMath::mul(k, V0V0V1V2); // k(V0^2/V1/V2)
   my_print_f(
       ">>>>>>3 _GeneralIntegrate:fairAmount=%, V0V0V1V2=%, penalty=%,DODOMath::_GeneralIntegrate(B0, B1, B2, i, "
       "stores._K_)=%=,",
       fairAmount, V0V0V1V2, penalty, DecimalMath::mul(fairAmount, add(sub(DecimalMath::ONE, k), penalty)));
   my_print_f(
       ">>>>>>3 _GeneralIntegrate:sub(DecimalMath::ONE, k)=%, add(sub(DecimalMath::ONE, k), penalty)=%=,",
       sub(DecimalMath::ONE, k), add(sub(DecimalMath::ONE, k), penalty));

   return DecimalMath::mul(fairAmount, add(sub(DecimalMath::ONE, k), penalty));
}

/*
    The same with integration expression above, we have:
    i*deltaB = (Q2-Q1)*(1-k+kQ0^2/Q1/Q2)
    Given Q1 and deltaB, solve Q2
    This is a quadratic function and the standard version is
    aQ2^2 + bQ2 + c = 0, where
    a=1-k
    -b=(1-k)Q1-kQ0^2/Q1+i*deltaB
    c=-kQ0^2
    and Q2=(-b+sqrt(b^2+4(1-k)kQ0^2))/2(1-k)
    note: another root is negative, abondan
    if deltaBSig=true, then Q2>Q1
    if deltaBSig=false, then Q2<Q1
*/
uint256 _SolveQuadraticFunctionForTrade(uint256 Q0, uint256 Q1, uint256 ideltaB, bool deltaBSig, uint256 k) {
   my_print_f(
       ">>>>>>> _SolveQuadraticFunctionForTrade: Q0=%,  Q1=%,  ideltaB=%,  deltaBSig=%,  k=%", Q0, Q1, ideltaB,
       deltaBSig, k);

   // calculate -b value and sig
   // -b = (1-k)Q1-kQ0^2/Q1+i*deltaB
   uint256 kQ02Q1 = div(SafeMath::mul(DecimalMath::mul(k, Q0), Q0), Q1); // kQ0^2/Q1
   uint256 b      = DecimalMath::mul(sub(DecimalMath::ONE, k), Q1);      // (1-k)Q1

   my_print_f(">>>>>>1 _SolveQuadraticFunctionForTrade: b=%, kQ02Q1=%", b, kQ02Q1);

   bool minusbSig = true;
   if (deltaBSig) {
      b = add(b, ideltaB); // (1-k)Q1+i*deltaB
   } else {
      kQ02Q1 = add(kQ02Q1, ideltaB); // i*deltaB+kQ0^2/Q1
   }

   my_print_f(">>>>>>2 _SolveQuadraticFunctionForTrade: b=%, kQ02Q1=%", b, kQ02Q1);

   if (b >= kQ02Q1) {
      b         = sub(b, kQ02Q1);
      minusbSig = true;
   } else {
      b         = sub(kQ02Q1, b);
      minusbSig = false;
   }

   // calculate sqrt
   uint256 squareRoot = DecimalMath::mul(
       SafeMath::mul(sub(DecimalMath::ONE, k), 4), SafeMath::mul(DecimalMath::mul(k, Q0), Q0)); // 4(1-k)kQ0^2
   squareRoot = SafeMath::sqrt(add(SafeMath::mul(b, b), squareRoot)); // sqrt(b*b+4(1-k)kQ0*Q0)

   my_print_f(
       ">>>>>>3 _SolveQuadraticFunctionForTrade:squareRoot0, "
       "SafeMath::mul(DecimalMath::mul(k, Q0), Q0))=%, squareRoot=%",
       DecimalMath::mul(SafeMath::mul(sub(DecimalMath::ONE, k), 4), SafeMath::mul(DecimalMath::mul(k, Q0), Q0)),
       squareRoot);

   // final res
   uint256 denominator = SafeMath::mul(sub(DecimalMath::ONE, k), 2); // 2(1-k)
   uint256 numerator;
   if (minusbSig) {
      numerator = add(b, squareRoot);
   } else {
      numerator = sub(squareRoot, b);
   }

   my_print_f(
       ">>>>>>4 _SolveQuadraticFunctionForTrade:minusbSig=%, numerator=%, denominator=%", minusbSig, numerator,
       denominator);
   if (deltaBSig) {
      my_print_f(
          ">>>>>>5 _SolveQuadraticFunctionForTrade DecimalMath::divFloor(numerator, denominator)=%",
          DecimalMath::divFloor(numerator, denominator));
      return DecimalMath::divFloor(numerator, denominator);
   } else {
      my_print_f(
          ">>>>>>6 _SolveQuadraticFunctionForTrade:DecimalMath::divCeil(numerator, denominator=%",
          DecimalMath::divCeil(numerator, denominator));
      return DecimalMath::divCeil(numerator, denominator);
   }
}

/*
    Start from the integration function
    i*deltaB = (Q2-Q1)*(1-k+kQ0^2/Q1/Q2)
    Assume Q2=Q0, Given Q1 and deltaB, solve Q0
    let fairAmount = i*deltaB
*/
uint256 _SolveQuadraticFunctionForTarget(uint256 V1, uint256 k, uint256 fairAmount) {
   my_print_f(">>>>>>> _SolveQuadraticFunctionForTarget: V1=%,  k=%,  fairAmount=%", V1, k, fairAmount);

   // V0 = V1+V1*(sqrt-1)/2k
//    uint256 sqrtv0 = mul(DecimalMath::mul(k, fairAmount), 4);
//    uint256 sqrtv1   = DecimalMath::divCeil(sqrtv0, V1);
//    uint256 sqrtv           = SafeMath::sqrt(mul(add(sqrtv1, DecimalMath::ONE), DecimalMath::ONE));
//    uint256 premium = DecimalMath::divCeil(sub(sqrtv, DecimalMath::ONE), mul(k, 2));
   double sq = DecimalMath::mul(k, fairAmount);
   double sqrtv0 =  DSafeMath::mul(sq, 4);
   double sqrtv1   = sqrtv0*DecimalMath::ONE/V1;//DecimalMath::ddivCeil(sqrtv0, V1);
   double sqrtv          = std::sqrt((sqrtv1+DecimalMath::ONE)*DecimalMath::ONE);//DSafeMath::sqrt(DSafeMath::mul(DSafeMath::add(sqrtv1, DecimalMath::ONE), DecimalMath::ONE));
   uint256 premium =   DecimalMath::ddivCeil(DSafeMath::sub(sqrtv, DecimalMath::ONE), DSafeMath::mul(k, 2));

   // V0 is greater than or equal to V1 according to the solution
   my_print_f(
       "\n>>>>>>_SolveQuadraticFunctionForTarget:sq=%,sqrtv0=%,sqrtv1=%, "
       "sqrtv=%, premium=%,sqrtv0/V1=% ",
       sq,sqrtv0,sqrtv1,sqrtv, premium,sqrtv0/V1);

   return DecimalMath::mul(V1, add(DecimalMath::ONE, premium));
}
} // namespace DODOMath
