// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <common/BType.hpp>

#include <eoswap/BNum.hpp>

class BMath : public BNum {
 public:
   /**********************************************************************************************
   // calcSpotPrice //
   // sP = spotPrice //
   // bI = tokenBalanceIn                ( bI / wI )         1 //
   // bO = tokenBalanceOut         sP =  -----------  *  ---------- //
   // wI = tokenWeightIn                 ( bO / wO )     ( 1 - sF ) //
   // wO = tokenWeightOut //
   // sF = swapFee //
   **********************************************************************************************/
   uint256m calcSpotPrice(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m tokenBalanceOut, uint256m tokenWeightOut,
       uint256m swapFee) {
      uint256m numer     = bdiv(tokenBalanceIn, tokenWeightIn);
      uint256m denom     = bdiv(tokenBalanceOut, tokenWeightOut);
      uint256m ratio     = bdiv(numer, denom);
      uint256m scale     = bdiv(BONE, bsub(BONE, swapFee));
      uint256m spotPrice = bmul(ratio, scale);
      //   my_print_f("=numer=%=,denom=%=,ratio=%=,scale=%=,spotPrice=%=", numer, denom, ratio, scale, spotPrice);

      return spotPrice;
   }

   uint256m dcalcSpotPrice(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m tokenBalanceOut, uint256m tokenWeightOut,
       uint256m swapFee) {
      //   uint256m numer     = bdiv(tokenBalanceIn, tokenWeightIn);
      double numer = double(tokenBalanceIn) / tokenWeightIn;
      //   uint256m denom     = bdiv(tokenBalanceOut, tokenWeightOut);
      double denom = double(tokenBalanceOut) / tokenWeightOut;
      //   uint256m ratio     = bdiv(numer, denom);
      double ratio = numer / denom;
      //   uint256m scale     = bdiv(BONE, bsub(BONE, swapFee));
      double scale = 1 / double(1 - double(swapFee) / BONE);
      //   uint256m spotPrice = bmul(ratio, scale);
      double spotPrice = ratio * scale * BONE;
      my_print_f("=numer=%=,denom=%=,ratio=%=,scale=%=,spotPrice=%=", numer, denom, ratio, scale, spotPrice);

      return spotPrice;
   }
   /**********************************************************************************************
   // calcOutGivenIn //
   // aO = tokenAmountOut //
   // bO = tokenBalanceOut //
   // bI = tokenBalanceIn              /      /            bI             \ (wI /
   wO) \      //
   // aI = tokenAmountIn    aO = bO * |  1 - | --------------------------  | ^ |
   //
   // wI = tokenWeightIn               \      \ ( bI + ( aI * ( 1 - sF )) / / //
   // wO = tokenWeightOut //
   // sF = swapFee //
   **********************************************************************************************/
   uint256m calcOutGivenIn(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m tokenBalanceOut, uint256m tokenWeightOut,
       uint256m tokenAmountIn, uint256m swapFee) {
      my_print_f("=calcOutGivenIn=tokenBalanceIn=%=,  tokenWeightIn=%=,  tokenBalanceOut=%=,  tokenWeightOut=%=,tokenAmountIn = %=,swapFee       = %= ", tokenBalanceIn,  tokenWeightIn,  tokenBalanceOut,  tokenWeightOut,
          tokenAmountIn,
          swapFee);
      uint256m weightRatio    = bdiv(tokenWeightIn, tokenWeightOut);
      uint256m adjustedIn0    = bsub(BONE, swapFee);
      uint256m adjustedIn     = bmul(tokenAmountIn, adjustedIn0);
      uint256m y              = bdiv(tokenBalanceIn, badd(tokenBalanceIn, adjustedIn));
      uint256m foo            = bpow(y, weightRatio);
      uint256m bar            = bsub(BONE, foo);
      uint256m tokenAmountOut = bmul(tokenBalanceOut, bar);
      my_print_f("=calcOutGivenIn=weightRatio=%=,adjustedIn0=%=,adjustedIn=%=,y=%=,  foo=%=,bar=%=,tokenAmountOut=%=", weightRatio,
          adjustedIn0, adjustedIn, y, foo, bar, tokenAmountOut);
      if (tokenAmountOut == 0) {
         tokenAmountOut =
             dcalcOutGivenIn(tokenBalanceIn, tokenWeightIn, tokenBalanceOut, tokenWeightOut, tokenAmountIn, swapFee);
         my_print_f("=calcOutGivenIn=y=%,tokenAmountOut == 0 tokenAmountOut=%, foo=%=", y, tokenAmountOut, foo);
      }

      return tokenAmountOut;
   }

   double dcalcOutGivenIn(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m tokenBalanceOut, uint256m tokenWeightOut,
       uint256m tokenAmountIn, uint256m swapFee) {

      //   uint256m weightRatio = bdiv(tokenWeightIn, tokenWeightOut);
      double weightRatio = double(tokenWeightIn) / tokenWeightOut;
      //   uint256m adjustedIn = bsub(BONE, swapFee);
      double adjustedIn = 1 - double(swapFee) / BONE;
      //   adjustedIn              = bmul(tokenAmountIn, adjustedIn);
      adjustedIn = double(tokenAmountIn) / BONE * adjustedIn;
      //   uint256m y              = bdiv(tokenBalanceIn, badd(tokenBalanceIn, adjustedIn));
      double dtokenBalanceIn = double(tokenBalanceIn) / BONE;
      double y               = dtokenBalanceIn / (dtokenBalanceIn + adjustedIn);
      //   uint256m foo             = bpow(y, weightRatio);
      double foo = std::pow(y, weightRatio);
      //   uint256m bar             = bsub(BONE, foo);
      double bar = 1 - foo;
      //   uint256m tokenAmountOut  = bmul(tokenBalanceOut, bar);
      double tokenAmountOut = double(tokenBalanceOut) / BONE * bar;
      my_print_f(
          "=weightRatio=%=,adjustedIn=%=,dtokenBalanceIn=%=, y=%=, foo=%=,bar=%=, tokenAmountOut=%=", weightRatio,
          adjustedIn, dtokenBalanceIn, y, foo, bar, tokenAmountOut);

      return tokenAmountOut * BONE;
   }

   /**********************************************************************************************
   // calcInGivenOut //
   // aI = tokenAmountIn //
   // bO = tokenBalanceOut               /  /     bO      \    (wO / wI)      \
   //
   // bI = tokenBalanceIn          bI * |  | ------------  | ^            - 1  |
   //
   // aO = tokenAmountOut    aI =        \  \ ( bO - aO ) /                   /
   //
   // wI = tokenWeightIn           --------------------------------------------
   //
   // wO = tokenWeightOut                          ( 1 - sF ) //
   // sF = swapFee //
   **********************************************************************************************/
   uint256m calcInGivenOut(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m tokenBalanceOut, uint256m tokenWeightOut,
       uint256m tokenAmountOut, uint256m swapFee) {
      my_print_f("=calcInGivenOut=calcInGivenOut=tokenBalanceIn=%=,  tokenWeightIn=%=,  tokenBalanceOut=%=,  tokenWeightOut=%=,tokenAmountOut = %=,swapFee        = %= ", tokenBalanceIn,  tokenWeightIn,  tokenBalanceOut,  tokenWeightOut,
          tokenAmountOut,
          swapFee);
      uint256m weightRatio    = bdiv(tokenWeightOut, tokenWeightIn);
      uint256m diff           = bsub(tokenBalanceOut, tokenAmountOut);
      uint256m y              = bdiv(tokenBalanceOut, diff);
      uint256m foo0           = bpow(y, weightRatio);
      uint256m foo            = bsub(foo0, BONE);
      uint256m tokenAmountIn0 = bsub(BONE, swapFee);
      uint256m tokenAmountIn  = bdiv(bmul(tokenBalanceIn, foo), tokenAmountIn0);
      my_print_f("=calcInGivenOut=weightRatio=%=,diff=%=,y=%=,  foo0=%=, foo=%=,tokenAmountIn0=%=,tokenAmountIn=%=", weightRatio, diff, y,
          foo0, foo, tokenAmountIn0, tokenAmountIn);

      if (tokenAmountIn == 0) {
         tokenAmountIn =
             dcalcInGivenOut(tokenBalanceIn, tokenWeightIn, tokenBalanceOut, tokenWeightOut, tokenAmountOut, swapFee);
         //   my_print_f(
         //       "=weightRatio=%=,diff=%=,dtokenBalanceIn=%=, y=%=, foo=%=, tokenAmountIn=%=", weightRatio,
         //       diff, tokenBalanceIn, y, foo,  tokenAmountIn);
      }

      return tokenAmountIn;
   }

   double dcalcInGivenOut(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m tokenBalanceOut, uint256m tokenWeightOut,
       uint256m tokenAmountOut, uint256m swapFee) {

      //   uint256m weightRatio   = bdiv(tokenWeightOut, tokenWeightIn);
      double weightRatio = double(tokenWeightOut) / tokenWeightIn;
      //   uint256m diff          = bsub(tokenBalanceOut, tokenAmountOut);
      double diff = double(tokenBalanceOut - tokenAmountOut) / BONE;
      //   uint256m y             = bdiv(tokenBalanceOut, diff);
      double y = double(tokenBalanceOut) / BONE / diff;
      //   uint256m foo           = bpow(y, weightRatio);
      double foo1 = std::pow(y, weightRatio);
      //   foo                    = bsub(foo, BONE);
      double foo = foo1 - 1;
      //   uint256m tokenAmountIn = bsub(BONE, swapFee);
      double tokenAmountIn = 1 - double(swapFee) / BONE;
      //   tokenAmountIn          = bdiv(bmul(tokenBalanceIn, foo), tokenAmountIn);
      tokenAmountIn = double(tokenBalanceIn) / BONE * foo / tokenAmountIn;

      //   my_print_f(
      //       "=dweightRatio=%=,diff=%=,dtokenBalanceIn=%=, y=%=, foo=%=,foo1=%=, tokenAmountIn=%=", weightRatio,
      //       diff, tokenBalanceIn, y, foo,foo1,  tokenAmountIn);

      return tokenAmountIn * BONE;
   }

   /**********************************************************************************************
   // calcPoolOutGivenSingleIn //
   // pAo = poolAmountOut         / \              //
   // tAi = tokenAmountIn        ///      /     //    wI \      \\       \     wI
   \             //
   // wI = tokenWeightIn        //| tAi *| 1 - || 1 - --  | * sF || + tBi \    --
   \            //
   // tW = totalWeight     pAo=||  \      \     \\    tW /      //         | ^ tW
   | * pS - pS //
   // tBi = tokenBalanceIn      \\  ------------------------------------- / / //
   // pS = poolSupply            \\                    tBi               / / //
   // sF = swapFee                \ /              //
   **********************************************************************************************/
   uint256m calcPoolOutGivenSingleIn(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m poolSupply, uint256m totalWeight,
       uint256m tokenAmountIn, uint256m swapFee) {
      // Charge the trading fee for the proportion of tokenAi
      ///  which is implicitly traded to the other pool tokens.
      // That proportion is (1- weightTokenIn)
      // tokenAiAfterFee = tAi * (1 - (1-weightTi) * poolFee);
      uint256m normalizedWeight      = bdiv(tokenWeightIn, totalWeight);
      uint256m zaz                   = bmul(bsub(BONE, normalizedWeight), swapFee);
      uint256m tokenAmountInAfterFee = bmul(tokenAmountIn, bsub(BONE, zaz));

      uint256m newTokenBalanceIn = badd(tokenBalanceIn, tokenAmountInAfterFee);
      uint256m tokenInRatio      = bdiv(newTokenBalanceIn, tokenBalanceIn);

      // uint256m newPoolSupply = (ratioTi ^ weightTi) * poolSupply;
      uint256m poolRatio     = bpow(tokenInRatio, normalizedWeight);
      uint256m newPoolSupply = bmul(poolRatio, poolSupply);
      uint256m poolAmountOut = bsub(newPoolSupply, poolSupply);
      return poolAmountOut;
   }

   /**********************************************************************************************
   // calcSingleInGivenPoolOut //
   // tAi = tokenAmountIn              //(pS + pAo)\     /    1    \\ //
   // pS = poolSupply                 || ---------  | ^ | --------- || * bI - bI
   //
   // pAo = poolAmountOut              \\    pS    /     \(wI / tW)// //
   // bI = balanceIn          tAi =  --------------------------------------------
   //
   // wI = weightIn                              /      wI  \ //
   // tW = totalWeight                          |  1 - ----  |  * sF //
   // sF = swapFee                               \      tW  / //
   **********************************************************************************************/
   uint256m calcSingleInGivenPoolOut(
       uint256m tokenBalanceIn, uint256m tokenWeightIn, uint256m poolSupply, uint256m totalWeight,
       uint256m poolAmountOut, uint256m swapFee) {
      uint256m normalizedWeight = bdiv(tokenWeightIn, totalWeight);
      uint256m newPoolSupply    = badd(poolSupply, poolAmountOut);
      uint256m poolRatio        = bdiv(newPoolSupply, poolSupply);

      // uint256m newBalTi = poolRatio^(1/weightTi) * balTi;
      uint256m boo                   = bdiv(BONE, normalizedWeight);
      uint256m tokenInRatio          = bpow(poolRatio, boo);
      uint256m newTokenBalanceIn     = bmul(tokenInRatio, tokenBalanceIn);
      uint256m tokenAmountInAfterFee = bsub(newTokenBalanceIn, tokenBalanceIn);
      // Do reverse order of fees charged in joinswap_ExternAmountIn, this way
      //     ``` pAo == joinswap_ExternAmountIn(Ti, joinswap_PoolAmountOut(pAo,
      //     Ti)) ```
      // uint256m tAi = tAiAfterFee / (1 - (1-weightTi) * swapFee) ;
      uint256m zar           = bmul(bsub(BONE, normalizedWeight), swapFee);
      uint256m tokenAmountIn = bdiv(tokenAmountInAfterFee, bsub(BONE, zar));
      return tokenAmountIn;
   }

   /**********************************************************************************************
   // calcSingleOutGivenPoolIn //
   // tAo = tokenAmountOut            /      / \\   //
   // bO = tokenBalanceOut           /      // pS - (pAi * (1 - eF)) \     /    1
   \      \\  //
   // pAi = poolAmountIn            | bO - || ----------------------- | ^ |
   --------- | * b0 || //
   // ps = poolSupply                \      \\          pS           /     \(wO /
   tW)/      //  //
   // wI = tokenWeightIn      tAo =   \      \ //   //
   // tW = totalWeight                    /     /      wO \       \ //
   // sF = swapFee                    *  | 1 - |  1 - ---- | * sF  | //
   // eF = exitFee                        \     \      tW /       / //
   **********************************************************************************************/
   uint256m calcSingleOutGivenPoolIn(
       uint256m tokenBalanceOut, uint256m tokenWeightOut, uint256m poolSupply, uint256m totalWeight,
       uint256m poolAmountIn, uint256m swapFee) {
      uint256m normalizedWeight = bdiv(tokenWeightOut, totalWeight);
      // charge exit fee on the pool token side
      // pAiAfterExitFee = pAi*(1-exitFee)
      uint256m poolAmountInAfterExitFee = bmul(poolAmountIn, bsub(BONE, EXIT_FEE));
      uint256m newPoolSupply            = bsub(poolSupply, poolAmountInAfterExitFee);
      uint256m poolRatio                = bdiv(newPoolSupply, poolSupply);

      // newBalTo = poolRatio^(1/weightTo) * balTo;
      uint256m tokenOutRatio      = bpow(poolRatio, bdiv(BONE, normalizedWeight));
      uint256m newTokenBalanceOut = bmul(tokenOutRatio, tokenBalanceOut);

      uint256m tokenAmountOutBeforeSwapFee = bsub(tokenBalanceOut, newTokenBalanceOut);

      // charge swap fee on the output token side
      // uint256m tAo = tAoBeforeSwapFee * (1 - (1-weightTo) * swapFee)
      uint256m zaz            = bmul(bsub(BONE, normalizedWeight), swapFee);
      uint256m tokenAmountOut = bmul(tokenAmountOutBeforeSwapFee, bsub(BONE, zaz));
      return tokenAmountOut;
   }

   /**********************************************************************************************
   // calcPoolInGivenSingleOut //
   // pAi = poolAmountIn               // /               tAo             \\ / wO
   \     \   //
   // bO = tokenBalanceOut            // | bO - -------------------------- |\   |
   ---- |     \  //
   // tAo = tokenAmountOut      pS - ||   \     1 - ((1 - (tO / tW)) * sF)/  | ^
   \ tW /  * pS | //
   // ps = poolSupply                 \\ -----------------------------------/ /
   //
   // wO = tokenWeightOut  pAi =       \\               bO                 / / //
   // tW = totalWeight
   -------------------------------------------------------------  //
   // sF = swapFee                                        ( 1 - eF ) //
   // eF = exitFee //
   **********************************************************************************************/
   uint256m calcPoolInGivenSingleOut(
       uint256m tokenBalanceOut, uint256m tokenWeightOut, uint256m poolSupply, uint256m totalWeight,
       uint256m tokenAmountOut, uint256m swapFee) {

      // charge swap fee on the output token side
      uint256m normalizedWeight = bdiv(tokenWeightOut, totalWeight);
      // uint256m tAoBeforeSwapFee = tAo / (1 - (1-weightTo) * swapFee) ;
      uint256m zoo                         = bsub(BONE, normalizedWeight);
      uint256m zar                         = bmul(zoo, swapFee);
      uint256m tokenAmountOutBeforeSwapFee = bdiv(tokenAmountOut, bsub(BONE, zar));

      uint256m newTokenBalanceOut = bsub(tokenBalanceOut, tokenAmountOutBeforeSwapFee);
      uint256m tokenOutRatio      = bdiv(newTokenBalanceOut, tokenBalanceOut);

      // uint256m newPoolSupply = (ratioTo ^ weightTo) * poolSupply;
      uint256m poolRatio                = bpow(tokenOutRatio, normalizedWeight);
      uint256m newPoolSupply            = bmul(poolRatio, poolSupply);
      uint256m poolAmountInAfterExitFee = bsub(poolSupply, newPoolSupply);

      // charge exit fee on the pool token side
      // pAi = pAiAfterExitFee/(1-exitFee)
      uint256m poolAmountIn = bdiv(poolAmountInAfterExitFee, bsub(BONE, EXIT_FEE));
      return poolAmountIn;
   }
};
