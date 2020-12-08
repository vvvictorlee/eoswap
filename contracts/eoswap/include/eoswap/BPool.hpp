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
#include <common/IFactory.hpp>
#include <eoswap/BMath.hpp>
#include <eoswap/BToken.hpp>
#include <storage/BPoolTable.hpp>

class storage_mgmt;
class BPool : public BToken, public BMath {
 private:
   IFactory&  ifactory;
   BPoolStore pool_store;
   name       pool_name;

 public:
   BPool(name _self, const extended_symbol& tokenx, IFactory& _ifactory, name _pool_name, const BPoolStore& _pool_store)
       : ifactory(_ifactory)
       , pool_store(_pool_store)
       , pool_name(_pool_name)
       , BToken(_self, tokenx, false) {}

   ~BPool() { ifactory.get_storage_mgmt().savePool(pool_name, pool_store); }

   class Lock {
      BPoolStore& pool_store;

    public:
      Lock(BPoolStore& _pool_store)
          : pool_store(_pool_store) {
         require(!pool_store.mutex, "ERR_REENTRY");
         pool_store.mutex = true;
      }
      ~Lock() { pool_store.mutex = false; }
   };

   void init() {
      pool_store.controller = get_self();
      pool_store.factory    = get_self();
      pool_store.swapFee    = MIN_FEE;
      pool_store.publicSwap = false;
      pool_store.finalized  = false;
      createtoken();
   }

   bool isPublicSwap() { return pool_store.publicSwap; }

   bool isFinalized() { return pool_store.finalized; }

   bool isBound(namesym t) { return pool_store.records[t].bound; }

   uint64_t getNumTokens() { return pool_store.tokens.size(); }

   std::vector<namesym> getCurrentTokens() { return pool_store.tokens; }

   std::vector<namesym> getFinalTokens() {
      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      return pool_store.tokens;
   }

   uint64_t getDenormalizedWeight(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      return pool_store.records[token].denorm;
   }

   uint64_t getTotalDenormalizedWeight() { return pool_store.totalWeight; }

   uint64_t getNormalizedWeight(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      uint64_t denorm = pool_store.records[token].denorm;
      return BMath::bdiv(denorm, pool_store.totalWeight);
   }

   uint64_t getBalance(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      return pool_store.records[token].balance;
   }

   uint64_t getSwapFee() { return pool_store.swapFee; }

   name getController() { return pool_store.controller; }

   void setSwapFee(uint64_t swapFee) {
      require(!pool_store.finalized, "ERR_IS_FINALIZED");
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(swapFee >= MIN_FEE, "ERR_MIN_FEE");
      require(swapFee <= MAX_FEE, "ERR_MAX_FEE");
      pool_store.swapFee = swapFee;
   }

   void setController(name manager) {
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      pool_store.controller = manager;
   }

   void setPublicSwap(bool public_) {
      require(!pool_store.finalized, "ERR_IS_FINALIZED");
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      pool_store.publicSwap = public_;
   }

   void finalize() {
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");
      require(pool_store.tokens.size() >= MIN_BOUND_TOKENS, "ERR_MIN_TOKENS");

      pool_store.finalized  = true;
      pool_store.publicSwap = true;

      _mintPoolShare(INIT_POOL_SUPPLY);
      _pushPoolShare(get_msg_sender(), INIT_POOL_SUPPLY);
   }

   void bind(const extended_asset& balancex, uint64_t denorm)
   // _lock_  Bind does not lock because it jumps to `rebind`, which does
   {
      namesym  token   = to_namesym(balancex.get_extended_symbol());
      uint64_t balance = balancex.quantity.amount;
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(!pool_store.records[token].bound, "ERR_IS_BOUND");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");

      require(pool_store.tokens.size() < MAX_BOUND_TOKENS, "ERR_MAX_TOKENS");

      pool_store.records[token] = Record({
          true, pool_store.tokens.size(),
          0, // balance and denorm will be validated
          0  // and set by `rebind`
      });
      pool_store.tokens.push_back(token);
      rebind(balancex, denorm);
   }

   void rebind(const extended_asset& balancex, uint64_t denorm) {
      namesym  token   = to_namesym(balancex.get_extended_symbol());
      uint64_t balance = balancex.quantity.amount;
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");

      require(denorm >= MIN_WEIGHT, "ERR_MIN_WEIGHT");
      require(denorm <= MAX_WEIGHT, "ERR_MAX_WEIGHT");
      require(balance >= MIN_BALANCE, "ERR_MIN_BALANCE");

      // Adjust the denorm and totalWeight
      uint64_t oldWeight = pool_store.records[token].denorm;
      if (denorm > oldWeight) {
         pool_store.totalWeight = BMath::badd(pool_store.totalWeight, BMath::bsub(denorm, oldWeight));
         require(pool_store.totalWeight <= MAX_TOTAL_WEIGHT, "ERR_MAX_TOTAL_WEIGHT");
      } else if (denorm < oldWeight) {
         pool_store.totalWeight = BMath::bsub(pool_store.totalWeight, BMath::bsub(oldWeight, denorm));
      }
      pool_store.records[token].denorm = denorm;

      // Adjust the balance record and actual token balance
      uint64_t oldBalance               = pool_store.records[token].balance;
      pool_store.records[token].balance = balance;
      pool_store.records[token].exsym   = balancex.get_extended_symbol();
      const extended_symbol& exsym      = pool_store.records[token].exsym;
      if (balance > oldBalance) {
         _pullUnderlying(get_msg_sender(), extended_asset(BMath::bsub(balance, oldBalance), exsym));
      } else if (balance < oldBalance) {
         // In this case liquidity is being withdrawn, so charge EXIT_FEE
         uint64_t tokenBalanceWithdrawn = BMath::bsub(oldBalance, balance);
         uint64_t tokenExitFee          = BMath::bmul(tokenBalanceWithdrawn, EXIT_FEE);
         _pushUnderlying(get_msg_sender(), extended_asset(BMath::bsub(tokenBalanceWithdrawn, tokenExitFee), exsym));
         _pushUnderlying(pool_store.factory, extended_asset(tokenExitFee, exsym));
      }
   }

   void unbind(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");

      uint64_t tokenBalance = pool_store.records[token].balance;
      uint64_t tokenExitFee = BMath::bmul(tokenBalance, EXIT_FEE);

      pool_store.totalWeight = BMath::bsub(pool_store.totalWeight, pool_store.records[token].denorm);

      // Swap the token-to-unbind with the last token,
      // then delete the last token
      uint64_t index                                     = pool_store.records[token].index;
      uint64_t last                                      = pool_store.tokens.size() - 1;
      pool_store.tokens[index]                           = pool_store.tokens[last];
      pool_store.records[pool_store.tokens[index]].index = index;
      pool_store.tokens.pop_back();
      pool_store.records[token]    = Record({false, 0, 0, 0});
      const extended_symbol& exsym = pool_store.records[token].exsym;
      _pushUnderlying(get_msg_sender(), extended_asset(BMath::bsub(tokenBalance, tokenExitFee), exsym));
      _pushUnderlying(pool_store.factory, extended_asset(tokenExitFee, exsym));
   }

   // Absorb any tokens that have been sent to this contract into the pool
   void gulp(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      pool_store.records[token].balance = transfer_mgmt::get_balance_one(pool_name, pool_store.records[token].exsym);
      ;
   }

   uint64_t getSpotPrice(const extended_symbol& tokenInx, const extended_symbol& tokenOutx) {
      namesym tokenIn  = to_namesym(tokenInx);
      namesym tokenOut = to_namesym(tokenOutx);
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      Record& inRecord  = pool_store.records[tokenIn];
      Record& outRecord = pool_store.records[tokenOut];
      return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
   }

   uint64_t getSpotPriceSansFee(const extended_symbol& tokenInx, const extended_symbol& tokenOutx) {
      namesym tokenIn  = to_namesym(tokenInx);
      namesym tokenOut = to_namesym(tokenOutx);
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      Record& inRecord  = pool_store.records[tokenIn];
      Record& outRecord = pool_store.records[tokenOut];
      return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, 0);
   }

   void joinPool(uint64_t poolAmountOut, std::vector<uint64_t> maxAmountsIn) {
      require(pool_store.finalized, "ERR_NOT_FINALIZED");

      uint64_t poolTotal = totalSupply();
      uint64_t ratio     = BMath::bdiv(poolAmountOut, poolTotal);
      check(
          ratio != 0, "ERR_MATH_APPROX joinPool: poolAmountOut:" + std::to_string(poolAmountOut) +
                          "poolTotal:" + std::to_string(poolTotal));

      for (uint64_t i = 0; i < pool_store.tokens.size(); i++) {
         namesym  t             = pool_store.tokens[i];
         uint64_t bal           = pool_store.records[t].balance;
         uint64_t tokenAmountIn = BMath::bmul(ratio, bal);

         check(
             tokenAmountIn != 0, "ERR_MATH_APPROX joinPool: ratio:" + std::to_string(ratio) +
                                     "bal:" + std::to_string(bal) + " i:" + std::to_string(i));
         check(
             tokenAmountIn <= maxAmountsIn[i], "ERR_LIMIT_IN joinPool: tokenAmountIn:" + std::to_string(tokenAmountIn));
         pool_store.records[t].balance = BMath::badd(pool_store.records[t].balance, tokenAmountIn);
         _pullUnderlying(get_msg_sender(), extended_asset(tokenAmountIn, pool_store.records[t].exsym));
      }
      _mintPoolShare(poolAmountOut);
      _pushPoolShare(get_msg_sender(), poolAmountOut);
   }

   void exitPool(uint64_t poolAmountIn, std::vector<uint64_t> minAmountsOut) {
      require(pool_store.finalized, "ERR_NOT_FINALIZED");

      uint64_t poolTotal       = totalSupply();
      uint64_t exitFee         = BMath::bmul(poolAmountIn, EXIT_FEE);
      uint64_t pAiAfterExitFee = BMath::bsub(poolAmountIn, exitFee);
      uint64_t ratio           = BMath::bdiv(pAiAfterExitFee, poolTotal);
      require(ratio != 0, "ERR_MATH_APPROX");

      _pullPoolShare(get_msg_sender(), poolAmountIn);
      _pushPoolShare(pool_store.factory, exitFee);
      _burnPoolShare(pAiAfterExitFee);

      for (uint64_t i = 0; i < pool_store.tokens.size(); i++) {
         namesym  t              = pool_store.tokens[i];
         uint64_t bal            = pool_store.records[t].balance;
         uint64_t tokenAmountOut = BMath::bmul(ratio, bal);
         require(tokenAmountOut != 0, "ERR_MATH_APPROX");
         require(tokenAmountOut >= minAmountsOut[i], "ERR_LIMIT_OUT");
         pool_store.records[t].balance = BMath::bsub(pool_store.records[t].balance, tokenAmountOut);
         _pushUnderlying(get_msg_sender(), extended_asset(tokenAmountOut, pool_store.records[t].exsym));
      }
   }

   std::pair<uint64_t, uint64_t>
   swapExactAmountIn(const extended_asset& tokenAmountInx, const extended_asset& minAmountOutx, uint64_t maxPrice) {
      namesym  tokenIn       = to_namesym(tokenAmountInx.get_extended_symbol());
      uint64_t tokenAmountIn = tokenAmountInx.quantity.amount;
      namesym  tokenOut      = to_namesym(minAmountOutx.get_extended_symbol());
      uint64_t minAmountOut  = minAmountOutx.quantity.amount;
      my_print_f("===minAmountOut==%==", minAmountOut);
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      require(pool_store.publicSwap, "ERR_SWAP_NOT_PUBLIC");

      Record& inRecord  = pool_store.records[tokenIn];
      Record& outRecord = pool_store.records[tokenOut];

      check(
          tokenAmountIn <= BMath::bmul(inRecord.balance, MAX_IN_RATIO),
          "ERR_MAX_IN_RATIO:MAX_IN_RATIO=" + std::to_string(static_cast<uint64_t>(MAX_IN_RATIO)) +
              ":inRecord.balance=" + std::to_string(inRecord.balance) +
              ":BMath::bmul(inRecord.balance, MAX_IN_RATIO)=" +
              std::to_string(static_cast<uint64_t>(BMath::bmul(inRecord.balance, MAX_IN_RATIO))));

      uint64_t spotPriceBefore =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      check(
          spotPriceBefore <= maxPrice,
          std::to_string(maxPrice) + "ERR_BAD_LIMIT_PRICE:spotPriceBefore=" + std::to_string(spotPriceBefore));

      uint64_t tokenAmountOut = calcOutGivenIn(
          inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, tokenAmountIn, pool_store.swapFee);
      check(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT:tokenAmountOut=" + std::to_string(tokenAmountOut));

      inRecord.balance  = BMath::badd(inRecord.balance, tokenAmountIn);
      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint64_t spotPriceAfter =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      check(
          spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX:spotPriceAfter=" + std::to_string(spotPriceAfter) +
                                                 ":spotPriceBefore=" + std::to_string(spotPriceBefore));
      check(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE:spotPriceAfter=" + std::to_string(spotPriceAfter));
      check(
          spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
          "ERR_MATH_APPROX:tokenAmountIn=" + std::to_string(tokenAmountIn) + ":tokenAmountOut=" +
              std::to_string(tokenAmountOut) + ":spotPriceBefore=" + std::to_string(spotPriceBefore));

      _pullUnderlying(get_msg_sender(), tokenAmountInx);
      _pushUnderlying(get_msg_sender(), extended_asset(tokenAmountOut, minAmountOutx.get_extended_symbol()));

      return std::make_pair(tokenAmountOut, spotPriceAfter);
   }

   std::pair<uint64_t, uint64_t>
   swapExactAmountOut(const extended_asset& maxAmountInx, const extended_asset& tokenAmountOutx, uint64_t maxPrice) {
      namesym  tokenIn        = to_namesym(maxAmountInx.get_extended_symbol());
      uint64_t maxAmountIn    = maxAmountInx.quantity.amount;
      namesym  tokenOut       = to_namesym(tokenAmountOutx.get_extended_symbol());
      uint64_t tokenAmountOut = tokenAmountOutx.quantity.amount;

      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      require(pool_store.publicSwap, "ERR_SWAP_NOT_PUBLIC");

      Record& inRecord  = pool_store.records[tokenIn];
      Record& outRecord = pool_store.records[tokenOut];

      check(
          tokenAmountOut <= BMath::bmul(outRecord.balance, MAX_OUT_RATIO),
          "ERR_MAX_OUT_RATIO:MAX_OUT_RATIO=" + std::to_string(static_cast<uint64_t>(MAX_OUT_RATIO)) +
              ":outRecord.balance=" + std::to_string(outRecord.balance) +
              ":BMath::bmul(outRecord.balance, MAX_OUT_RATIO)=" +
              std::to_string(static_cast<uint64_t>(BMath::bmul(outRecord.balance, MAX_OUT_RATIO))));

      uint64_t spotPriceBefore =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      check(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE:spotPriceBefore=" + std::to_string(spotPriceBefore));

      uint64_t tokenAmountIn = calcInGivenOut(
          inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, tokenAmountOut, pool_store.swapFee);
      check(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN:tokenAmountIn=" + std::to_string(tokenAmountIn));

      inRecord.balance  = BMath::badd(inRecord.balance, tokenAmountIn);
      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint64_t spotPriceAfter =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      check(
          spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX:spotPriceAfter=" + std::to_string(spotPriceAfter) +
                                                 ":spotPriceBefore=" + std::to_string(spotPriceBefore));
      check(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE:spotPriceAfter=" + std::to_string(spotPriceAfter));
      check(
          spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
          "ERR_MATH_APPROX:tokenAmountIn=" + std::to_string(tokenAmountIn) + ":tokenAmountOut=" +
              std::to_string(tokenAmountOut) + ":spotPriceBefore=" + std::to_string(spotPriceBefore));

      _pullUnderlying(get_msg_sender(), extended_asset(tokenAmountIn, maxAmountInx.get_extended_symbol()));
      _pushUnderlying(get_msg_sender(), tokenAmountOutx);

      return std::make_pair(tokenAmountIn, spotPriceAfter);
   }

   uint64_t joinswapExternAmountIn(const extended_asset& tokenAmountInx, uint64_t minPoolAmountOut) {
      namesym  tokenIn       = to_namesym(tokenAmountInx.get_extended_symbol());
      uint64_t tokenAmountIn = tokenAmountInx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(tokenAmountIn <= BMath::bmul(pool_store.records[tokenIn].balance, MAX_IN_RATIO), "ERR_MAX_IN_RATIO");

      Record& inRecord = pool_store.records[tokenIn];

      uint64_t poolAmountOut = calcPoolOutGivenSingleIn(
          inRecord.balance, inRecord.denorm, totalSupply(), pool_store.totalWeight, tokenAmountIn, pool_store.swapFee);

      require(poolAmountOut >= minPoolAmountOut, "ERR_LIMIT_OUT");

      inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

      _mintPoolShare(poolAmountOut);
      _pushPoolShare(get_msg_sender(), poolAmountOut);
      _pullUnderlying(get_msg_sender(), tokenAmountInx);

      return poolAmountOut;
   }

   uint64_t joinswapPoolAmountOut(uint64_t poolAmountOut, const extended_asset& maxAmountInx) {
      namesym  tokenIn     = to_namesym(maxAmountInx.get_extended_symbol());
      uint64_t maxAmountIn = maxAmountInx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");

      Record& inRecord = pool_store.records[tokenIn];

      uint64_t tokenAmountIn = calcSingleInGivenPoolOut(
          inRecord.balance, inRecord.denorm, totalSupply(), pool_store.totalWeight, poolAmountOut, pool_store.swapFee);

      require(tokenAmountIn != 0, "ERR_MATH_APPROX");
      require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

      require(tokenAmountIn <= BMath::bmul(pool_store.records[tokenIn].balance, MAX_IN_RATIO), "ERR_MAX_IN_RATIO");

      inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

      _mintPoolShare(poolAmountOut);
      _pushPoolShare(get_msg_sender(), poolAmountOut);
      _pullUnderlying(get_msg_sender(), extended_asset(tokenAmountIn, maxAmountInx.get_extended_symbol()));

      return tokenAmountIn;
   }

   uint64_t exitswapPoolAmountIn(uint64_t poolAmountIn, const extended_asset& minAmountOutx) {
      namesym  tokenOut     = to_namesym(minAmountOutx.get_extended_symbol());
      uint64_t minAmountOut = minAmountOutx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");

      Record& outRecord = pool_store.records[tokenOut];

      uint64_t tokenAmountOut = calcSingleOutGivenPoolIn(
          outRecord.balance, outRecord.denorm, totalSupply(), pool_store.totalWeight, poolAmountIn, pool_store.swapFee);

      require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

      require(tokenAmountOut <= BMath::bmul(pool_store.records[tokenOut].balance, MAX_OUT_RATIO), "ERR_MAX_OUT_RATIO");

      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint64_t exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

      _pullPoolShare(get_msg_sender(), poolAmountIn);
      _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
      _pushPoolShare(pool_store.factory, exitFee);

      _pushUnderlying(get_msg_sender(), extended_asset(tokenAmountOut, minAmountOutx.get_extended_symbol()));

      return tokenAmountOut;
   }

   uint64_t exitswapExternAmountOut(const extended_asset& tokenAmountOutx, uint64_t maxPoolAmountIn) {
      namesym  tokenOut       = to_namesym(tokenAmountOutx.get_extended_symbol());
      uint64_t tokenAmountOut = tokenAmountOutx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      require(tokenAmountOut <= BMath::bmul(pool_store.records[tokenOut].balance, MAX_OUT_RATIO), "ERR_MAX_OUT_RATIO");

      Record& outRecord = pool_store.records[tokenOut];

      uint64_t poolAmountIn = calcPoolInGivenSingleOut(
          outRecord.balance, outRecord.denorm, totalSupply(), pool_store.totalWeight, tokenAmountOut,
          pool_store.swapFee);

      require(poolAmountIn != 0, "ERR_MATH_APPROX");
      require(poolAmountIn <= maxPoolAmountIn, "ERR_LIMIT_IN");

      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint64_t exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

      _pullPoolShare(get_msg_sender(), poolAmountIn);
      _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
      _pushPoolShare(pool_store.factory, exitFee);
      _pushUnderlying(get_msg_sender(), tokenAmountOutx);

      return poolAmountIn;
   }

   // ==
   // 'Underlying' token-manipulation functions make external calls but are NOT
   // locked You must `_lock_` or otherwise ensure reentry-safety
   void _pullUnderlying(name from, const extended_asset& amount) {
      extended_asset amountx = convert_one_decimals(amount, -1);
      /// transfer memo implementation
      my_print_f("_pullUnderlying======= %,%,%==", from, pool_name, amountx);
      if (0 == amountx.quantity.amount) {
         return;
      }
      ifactory.get_transfer_mgmt().transfer(from, pool_name, amountx, "");
   }

   void _pushUnderlying(name to, const extended_asset& amount) {
      extended_asset amountx = convert_one_decimals(amount, -1);
      my_print_f("_pushUnderlying=======%,%,%==", pool_name, to, amountx);
      if (0 == amountx.quantity.amount) {
         return;
      }
      ifactory.get_transfer_mgmt().transfer(pool_name, to, amountx, "");
   }

   void _pullPoolShare(name from, uint64_t amount) {
      my_print_f("_pullPoolShare======= %,%,%==", from, pool_name, amount);
      if (0 == amount) {
         return;
      }
      set_caller(pool_name);
      _pull(from, amount);
   }

   void _pushPoolShare(name to, uint64_t amount) {
      my_print_f("_pushPoolShare======= %,%,%==", to, pool_name, amount);
      if (0 == amount) {
         return;
      }
      set_caller(pool_name);
      _push(to, amount);
   }

   void _mintPoolShare(uint64_t amount) {
      my_print_f("_mintPoolShare======= %,%==", pool_name, amount);
      if (0 == amount) {
         return;
      }
      set_caller(pool_name);
      _mint(amount);
   }

   void _burnPoolShare(uint64_t amount) {
      my_print_f("_burnPoolShare=======%, %,==", pool_name, amount);
      if (0 == amount) {
         return;
      }
      set_caller(pool_name);
      _burn(amount);
   }
};
