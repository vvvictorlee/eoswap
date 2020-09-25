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

class BMath :  public BNum {
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
  uint calcSpotPrice(uint tokenBalanceIn, uint tokenWeightIn,
                     uint tokenBalanceOut, uint tokenWeightOut, uint swapFee) {
    uint numer = bdiv(tokenBalanceIn, tokenWeightIn);
    uint denom = bdiv(tokenBalanceOut, tokenWeightOut);
    uint ratio = bdiv(numer, denom);
    uint scale = bdiv(BONE, bsub(BONE, swapFee));
    uint spotPrice = bmul(ratio, scale);
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
  uint calcOutGivenIn(uint tokenBalanceIn, uint tokenWeightIn,
                      uint tokenBalanceOut, uint tokenWeightOut,
                      uint tokenAmountIn, uint swapFee) {
    uint weightRatio = bdiv(tokenWeightIn, tokenWeightOut);
    uint adjustedIn = bsub(BONE, swapFee);
    adjustedIn = bmul(tokenAmountIn, adjustedIn);
    uint y = bdiv(tokenBalanceIn, badd(tokenBalanceIn, adjustedIn));
    uint foo = bpow(y, weightRatio);
    uint bar = bsub(BONE, foo);
    uint tokenAmountOut = bmul(tokenBalanceOut, bar);
    return tokenAmountOut;
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
  uint calcInGivenOut(uint tokenBalanceIn, uint tokenWeightIn,
                      uint tokenBalanceOut, uint tokenWeightOut,
                      uint tokenAmountOut, uint swapFee) {
    uint weightRatio = bdiv(tokenWeightOut, tokenWeightIn);
    uint diff = bsub(tokenBalanceOut, tokenAmountOut);
    uint y = bdiv(tokenBalanceOut, diff);
    uint foo = bpow(y, weightRatio);
    foo = bsub(foo, BONE);
    uint tokenAmountIn = bsub(BONE, swapFee);
    tokenAmountIn = bdiv(bmul(tokenBalanceIn, foo), tokenAmountIn);
    return tokenAmountIn;
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
  uint calcPoolOutGivenSingleIn(uint tokenBalanceIn, uint tokenWeightIn,
                                uint poolSupply, uint totalWeight,
                                uint tokenAmountIn, uint swapFee) {
    // Charge the trading fee for the proportion of tokenAi
    ///  which is implicitly traded to the other pool tokens.
    // That proportion is (1- weightTokenIn)
    // tokenAiAfterFee = tAi * (1 - (1-weightTi) * poolFee);
    uint normalizedWeight = bdiv(tokenWeightIn, totalWeight);
    uint zaz = bmul(bsub(BONE, normalizedWeight), swapFee);
    uint tokenAmountInAfterFee = bmul(tokenAmountIn, bsub(BONE, zaz));

    uint newTokenBalanceIn = badd(tokenBalanceIn, tokenAmountInAfterFee);
    uint tokenInRatio = bdiv(newTokenBalanceIn, tokenBalanceIn);

    // uint newPoolSupply = (ratioTi ^ weightTi) * poolSupply;
    uint poolRatio = bpow(tokenInRatio, normalizedWeight);
    uint newPoolSupply = bmul(poolRatio, poolSupply);
    uint poolAmountOut = bsub(newPoolSupply, poolSupply);
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
  uint calcSingleInGivenPoolOut(uint tokenBalanceIn, uint tokenWeightIn,
                                uint poolSupply, uint totalWeight,
                                uint poolAmountOut, uint swapFee) {
    uint normalizedWeight = bdiv(tokenWeightIn, totalWeight);
    uint newPoolSupply = badd(poolSupply, poolAmountOut);
    uint poolRatio = bdiv(newPoolSupply, poolSupply);

    // uint newBalTi = poolRatio^(1/weightTi) * balTi;
    uint boo = bdiv(BONE, normalizedWeight);
    uint tokenInRatio = bpow(poolRatio, boo);
    uint newTokenBalanceIn = bmul(tokenInRatio, tokenBalanceIn);
    uint tokenAmountInAfterFee = bsub(newTokenBalanceIn, tokenBalanceIn);
    // Do reverse order of fees charged in joinswap_ExternAmountIn, this way
    //     ``` pAo == joinswap_ExternAmountIn(Ti, joinswap_PoolAmountOut(pAo,
    //     Ti)) ```
    // uint tAi = tAiAfterFee / (1 - (1-weightTi) * swapFee) ;
    uint zar = bmul(bsub(BONE, normalizedWeight), swapFee);
    uint tokenAmountIn = bdiv(tokenAmountInAfterFee, bsub(BONE, zar));
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
  uint calcSingleOutGivenPoolIn(uint tokenBalanceOut, uint tokenWeightOut,
                                uint poolSupply, uint totalWeight,
                                uint poolAmountIn, uint swapFee) {
    uint normalizedWeight = bdiv(tokenWeightOut, totalWeight);
    // charge exit fee on the pool token side
    // pAiAfterExitFee = pAi*(1-exitFee)
    uint poolAmountInAfterExitFee = bmul(poolAmountIn, bsub(BONE, EXIT_FEE));
    uint newPoolSupply = bsub(poolSupply, poolAmountInAfterExitFee);
    uint poolRatio = bdiv(newPoolSupply, poolSupply);

    // newBalTo = poolRatio^(1/weightTo) * balTo;
    uint tokenOutRatio = bpow(poolRatio, bdiv(BONE, normalizedWeight));
    uint newTokenBalanceOut = bmul(tokenOutRatio, tokenBalanceOut);

    uint tokenAmountOutBeforeSwapFee =
        bsub(tokenBalanceOut, newTokenBalanceOut);

    // charge swap fee on the output token side
    // uint tAo = tAoBeforeSwapFee * (1 - (1-weightTo) * swapFee)
    uint zaz = bmul(bsub(BONE, normalizedWeight), swapFee);
    uint tokenAmountOut = bmul(tokenAmountOutBeforeSwapFee, bsub(BONE, zaz));
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
  uint calcPoolInGivenSingleOut(uint tokenBalanceOut, uint tokenWeightOut,
                                uint poolSupply, uint totalWeight,
                                uint tokenAmountOut, uint swapFee) {

    // charge swap fee on the output token side
    uint normalizedWeight = bdiv(tokenWeightOut, totalWeight);
    // uint tAoBeforeSwapFee = tAo / (1 - (1-weightTo) * swapFee) ;
    uint zoo = bsub(BONE, normalizedWeight);
    uint zar = bmul(zoo, swapFee);
    uint tokenAmountOutBeforeSwapFee = bdiv(tokenAmountOut, bsub(BONE, zar));

    uint newTokenBalanceOut =
        bsub(tokenBalanceOut, tokenAmountOutBeforeSwapFee);
    uint tokenOutRatio = bdiv(newTokenBalanceOut, tokenBalanceOut);

    // uint newPoolSupply = (ratioTo ^ weightTo) * poolSupply;
    uint poolRatio = bpow(tokenOutRatio, normalizedWeight);
    uint newPoolSupply = bmul(poolRatio, poolSupply);
    uint poolAmountInAfterExitFee = bsub(poolSupply, newPoolSupply);

    // charge exit fee on the pool token side
    // pAi = pAiAfterExitFee/(1-exitFee)
    uint poolAmountIn = bdiv(poolAmountInAfterExitFee, bsub(BONE, EXIT_FEE));
    return poolAmountIn;
  }
};
