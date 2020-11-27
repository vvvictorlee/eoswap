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
#include <eoswap/BMath.hpp>
#include <eoswap/BToken.hpp>
#include <storage/BPoolTable.hpp>

class storage_mgmt;
template <typename FactoryType, typename PoolStoreType, typename TokenStoreType>
class BPool : public BToken<TokenStoreType>, public BMath {

 private:
   FactoryType& factory;
   BPoolStore   pool_store;
   name pool_name;
 public:
   BPool(name _self, FactoryType& _factory,name _pool_name,const PoolStoreType& _pool_store, TokenStoreType& _tokenStore)
       : factory(_factory)
       , pool_store(_pool_store)
       , pool_name(_pool_name)
       , BToken<TokenStoreType>(_self, _tokenStore) {}

   ~BPool() { factory.get_storage_mgmt().savePool(pool_name, pool_store ); }

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
      pool_store.controller = BToken<TokenStoreType>::get_self();
      pool_store.factory    = BToken<TokenStoreType>::get_self();
      pool_store.swapFee    = MIN_FEE;
      pool_store.publicSwap = false;
      pool_store.finalized  = false;
   }

   bool isPublicSwap() { return pool_store.publicSwap; }

   bool isFinalized() { return pool_store.finalized; }

   bool isBound(namesym t) { return pool_store.records[t].bound; }

   uint getNumTokens() { return pool_store.tokens.size(); }

   std::vector<namesym> getCurrentTokens() { return pool_store.tokens; }

   std::vector<namesym> getFinalTokens() {
      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      return pool_store.tokens;
   }

   uint getDenormalizedWeight(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      return pool_store.records[token].denorm;
   }

   uint getTotalDenormalizedWeight() { return pool_store.totalWeight; }

   uint getNormalizedWeight(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      uint denorm = pool_store.records[token].denorm;
      return BMath::bdiv(denorm, pool_store.totalWeight);
   }

   uint getBalance(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      return pool_store.records[token].balance;
   }

   uint getSwapFee() { return pool_store.swapFee; }

   name getController() { return pool_store.controller; }

   void setSwapFee(uint swapFee) {
      require(!pool_store.finalized, "ERR_IS_FINALIZED");
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(swapFee >= MIN_FEE, "ERR_MIN_FEE");
      require(swapFee <= MAX_FEE, "ERR_MAX_FEE");
      pool_store.swapFee = swapFee;
   }

   void setController(name manager) {
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      pool_store.controller = manager;
   }

   void setPublicSwap(bool public_) {
      require(!pool_store.finalized, "ERR_IS_FINALIZED");
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      pool_store.publicSwap = public_;
   }

   void finalize() {
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");
      require(pool_store.tokens.size() >= MIN_BOUND_TOKENS, "ERR_MIN_TOKENS");

      pool_store.finalized  = true;
      pool_store.publicSwap = true;

      _mintPoolShare(INIT_POOL_SUPPLY);
      _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), INIT_POOL_SUPPLY);
   }

   void bind(const extended_asset& balancex, uint denorm)
   // _lock_  Bind does not lock because it jumps to `rebind`, which does
   {
      namesym token   = to_namesym(balancex.get_extended_symbol());
      uint    balance = balancex.quantity.amount;
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
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

   void rebind(const extended_asset& balancex, uint denorm) {
      namesym token   = to_namesym(balancex.get_extended_symbol());
      uint    balance = balancex.quantity.amount;
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");

      require(denorm >= MIN_WEIGHT, "ERR_MIN_WEIGHT");
      require(denorm <= MAX_WEIGHT, "ERR_MAX_WEIGHT");
      require(balance >= MIN_BALANCE, "ERR_MIN_BALANCE");

      // Adjust the denorm and totalWeight
      uint oldWeight = pool_store.records[token].denorm;
      if (denorm > oldWeight) {
         pool_store.totalWeight = BMath::badd(pool_store.totalWeight, BMath::bsub(denorm, oldWeight));
         require(pool_store.totalWeight <= MAX_TOTAL_WEIGHT, "ERR_MAX_TOTAL_WEIGHT");
      } else if (denorm < oldWeight) {
         pool_store.totalWeight = BMath::bsub(pool_store.totalWeight, BMath::bsub(oldWeight, denorm));
      }
      pool_store.records[token].denorm = denorm;

      // Adjust the balance record and actual token balance
      uint oldBalance                   = pool_store.records[token].balance;
      pool_store.records[token].balance = balance;
      pool_store.records[token].exsym   = balancex.get_extended_symbol();
      const extended_symbol& exsym      = pool_store.records[token].exsym;
      if (balance > oldBalance) {
         _pullUnderlying(
             BToken<TokenStoreType>::get_msg_sender(), extended_asset(BMath::bsub(balance, oldBalance), exsym));
      } else if (balance < oldBalance) {
         // In this case liquidity is being withdrawn, so charge EXIT_FEE
         uint tokenBalanceWithdrawn = BMath::bsub(oldBalance, balance);
         uint tokenExitFee          = BMath::bmul(tokenBalanceWithdrawn, EXIT_FEE);
         _pushUnderlying(
             BToken<TokenStoreType>::get_msg_sender(),
             extended_asset(BMath::bsub(tokenBalanceWithdrawn, tokenExitFee), exsym));
         _pushUnderlying(pool_store.factory, extended_asset(tokenExitFee, exsym));
      }
   }

   void unbind(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller, "ERR_NOT_CONTROLLER");
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      require(!pool_store.finalized, "ERR_IS_FINALIZED");

      uint tokenBalance = pool_store.records[token].balance;
      uint tokenExitFee = BMath::bmul(tokenBalance, EXIT_FEE);

      pool_store.totalWeight = BMath::bsub(pool_store.totalWeight, pool_store.records[token].denorm);

      // Swap the token-to-unbind with the last token,
      // then delete the last token
      uint index                                         = pool_store.records[token].index;
      uint last                                          = pool_store.tokens.size() - 1;
      pool_store.tokens[index]                           = pool_store.tokens[last];
      pool_store.records[pool_store.tokens[index]].index = index;
      pool_store.tokens.pop_back();
      pool_store.records[token]    = Record({false, 0, 0, 0});
      const extended_symbol& exsym = pool_store.records[token].exsym;
      _pushUnderlying(
          BToken<TokenStoreType>::get_msg_sender(), extended_asset(BMath::bsub(tokenBalance, tokenExitFee), exsym));
      _pushUnderlying(pool_store.factory, extended_asset(tokenExitFee, exsym));
   }

   // Absorb any tokens that have been sent to this contract into the pool
   void gulp(const extended_symbol& tokenx) {
      namesym token = to_namesym(tokenx);
      require(pool_store.records[token].bound, "ERR_NOT_BOUND");
      factory.setMsgSender(BToken<TokenStoreType>::get_msg_sender());
      factory.token(token, [&](auto& _token_) {
         pool_store.records[token].balance = _token_.balanceOf(BToken<TokenStoreType>::get_msg_sender());
         auto tokenBalance =
             transfer_mgmt::get_balance(BToken<TokenStoreType>::get_msg_sender(), pool_store.records[token].exsym);
         require(pool_store.records[token].balance <= tokenBalance.amount, "ERR_BALANCE_FATAL");
      });
   }

   uint getSpotPrice(const extended_symbol& tokenInx, const extended_symbol& tokenOutx) {
      namesym tokenIn  = to_namesym(tokenInx);
      namesym tokenOut = to_namesym(tokenOutx);
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      Record inRecord  = pool_store.records[tokenIn];
      Record outRecord = pool_store.records[tokenOut];
      return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
   }

   uint getSpotPriceSansFee(const extended_symbol& tokenInx, const extended_symbol& tokenOutx) {
      namesym tokenIn  = to_namesym(tokenInx);
      namesym tokenOut = to_namesym(tokenOutx);
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      Record inRecord  = pool_store.records[tokenIn];
      Record outRecord = pool_store.records[tokenOut];
      return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, 0);
   }

   void joinPool(uint poolAmountOut, std::vector<uint> maxAmountsIn) {
      require(pool_store.finalized, "ERR_NOT_FINALIZED");

      uint poolTotal = BToken<TokenStoreType>::totalSupply();
      uint ratio     = BMath::bdiv(poolAmountOut, poolTotal);
      require(ratio != 0, "ERR_MATH_APPROX");

      for (uint i = 0; i < pool_store.tokens.size(); i++) {
         namesym t             = pool_store.tokens[i];
         uint    bal           = pool_store.records[t].balance;
         uint    tokenAmountIn = BMath::bmul(ratio, bal);

         require(tokenAmountIn != 0, "ERR_MATH_APPROX");
         require(tokenAmountIn <= maxAmountsIn[i], "ERR_LIMIT_IN joinPool");
         pool_store.records[t].balance = BMath::badd(pool_store.records[t].balance, tokenAmountIn);
         _pullUnderlying(
             BToken<TokenStoreType>::get_msg_sender(), extended_asset(tokenAmountIn, pool_store.records[t].exsym));
      }
      _mintPoolShare(poolAmountOut);
      _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountOut);
   }

   void exitPool(uint poolAmountIn, std::vector<uint> minAmountsOut) {
      require(pool_store.finalized, "ERR_NOT_FINALIZED");

      uint poolTotal       = BToken<TokenStoreType>::totalSupply();
      uint exitFee         = BMath::bmul(poolAmountIn, EXIT_FEE);
      uint pAiAfterExitFee = BMath::bsub(poolAmountIn, exitFee);
      uint ratio           = BMath::bdiv(pAiAfterExitFee, poolTotal);
      require(ratio != 0, "ERR_MATH_APPROX");

      _pullPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountIn);
      _pushPoolShare(pool_store.factory, exitFee);
      _burnPoolShare(pAiAfterExitFee);

      for (uint i = 0; i < pool_store.tokens.size(); i++) {
         namesym t              = pool_store.tokens[i];
         uint    bal            = pool_store.records[t].balance;
         uint    tokenAmountOut = BMath::bmul(ratio, bal);
         require(tokenAmountOut != 0, "ERR_MATH_APPROX");
         require(tokenAmountOut >= minAmountsOut[i], "ERR_LIMIT_OUT");
         pool_store.records[t].balance = BMath::bsub(pool_store.records[t].balance, tokenAmountOut);
         _pushUnderlying(
             BToken<TokenStoreType>::get_msg_sender(), extended_asset(tokenAmountOut, pool_store.records[t].exsym));
      }
   }

   std::pair<uint, uint>
   swapExactAmountIn(const extended_asset& tokenAmountInx, const extended_asset& minAmountOutx, uint maxPrice) {

      namesym tokenIn       = to_namesym(tokenAmountInx.get_extended_symbol());
      uint    tokenAmountIn = tokenAmountInx.quantity.amount;
      namesym tokenOut      = to_namesym(minAmountOutx.get_extended_symbol());
      uint    minAmountOut  = minAmountOutx.quantity.amount;

      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      require(pool_store.publicSwap, "ERR_SWAP_NOT_PUBLIC");

      Record inRecord  = pool_store.records[tokenIn];
      Record outRecord = pool_store.records[tokenOut];

      require(tokenAmountIn <= BMath::bmul(inRecord.balance, MAX_IN_RATIO), "ERR_MAX_IN_RATIO");

      uint spotPriceBefore =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

      uint tokenAmountOut = calcOutGivenIn(
          inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, tokenAmountIn, pool_store.swapFee);
      print("===tokenAmountOut===", tokenAmountOut, "==minAmountOut===", minAmountOut);
      require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

      inRecord.balance  = BMath::badd(inRecord.balance, tokenAmountIn);
      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint spotPriceAfter =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
      require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
      require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut), "ERR_MATH_APPROX");

      _pullUnderlying(BToken<TokenStoreType>::get_msg_sender(), tokenAmountInx);
      _pushUnderlying(
          BToken<TokenStoreType>::get_msg_sender(),
          extended_asset(tokenAmountOut, minAmountOutx.get_extended_symbol()));

      return std::make_pair(tokenAmountOut, spotPriceAfter);
   }

   std::pair<uint, uint>
   swapExactAmountOut(const extended_asset& maxAmountInx, const extended_asset& tokenAmountOutx, uint maxPrice) {
      namesym tokenIn        = to_namesym(maxAmountInx.get_extended_symbol());
      uint    maxAmountIn    = maxAmountInx.quantity.amount;
      namesym tokenOut       = to_namesym(tokenAmountOutx.get_extended_symbol());
      uint    tokenAmountOut = tokenAmountOutx.quantity.amount;

      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      require(pool_store.publicSwap, "ERR_SWAP_NOT_PUBLIC");

      Record inRecord  = pool_store.records[tokenIn];
      Record outRecord = pool_store.records[tokenOut];

      require(tokenAmountOut <= BMath::bmul(outRecord.balance, MAX_OUT_RATIO), "ERR_MAX_OUT_RATIO");

      uint spotPriceBefore =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

      uint tokenAmountIn = calcInGivenOut(
          inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, tokenAmountOut, pool_store.swapFee);
      require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

      inRecord.balance  = BMath::badd(inRecord.balance, tokenAmountIn);
      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint spotPriceAfter =
          calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm, pool_store.swapFee);
      require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
      require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
      require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut), "ERR_MATH_APPROX");

      _pullUnderlying(
          BToken<TokenStoreType>::get_msg_sender(), extended_asset(tokenAmountIn, maxAmountInx.get_extended_symbol()));
      _pushUnderlying(BToken<TokenStoreType>::get_msg_sender(), tokenAmountOutx);

      return std::make_pair(tokenAmountIn, spotPriceAfter);
   }

   uint joinswapExternAmountIn(const extended_asset& tokenAmountInx, uint minPoolAmountOut) {
      namesym tokenIn       = to_namesym(tokenAmountInx.get_extended_symbol());
      uint    tokenAmountIn = tokenAmountInx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
      require(tokenAmountIn <= BMath::bmul(pool_store.records[tokenIn].balance, MAX_IN_RATIO), "ERR_MAX_IN_RATIO");

      Record inRecord = pool_store.records[tokenIn];

      uint poolAmountOut = calcPoolOutGivenSingleIn(
          inRecord.balance, inRecord.denorm, BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
          tokenAmountIn, pool_store.swapFee);

      require(poolAmountOut >= minPoolAmountOut, "ERR_LIMIT_OUT");

      inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

      _mintPoolShare(poolAmountOut);
      _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountOut);
      _pullUnderlying(BToken<TokenStoreType>::get_msg_sender(), tokenAmountInx);

      return poolAmountOut;
   }

   uint joinswapPoolAmountOut(uint poolAmountOut, const extended_asset& maxAmountInx) {
      namesym tokenIn     = to_namesym(maxAmountInx.get_extended_symbol());
      uint    maxAmountIn = maxAmountInx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");

      Record inRecord = pool_store.records[tokenIn];

      uint tokenAmountIn = calcSingleInGivenPoolOut(
          inRecord.balance, inRecord.denorm, BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
          poolAmountOut, pool_store.swapFee);

      require(tokenAmountIn != 0, "ERR_MATH_APPROX");
      require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

      require(tokenAmountIn <= BMath::bmul(pool_store.records[tokenIn].balance, MAX_IN_RATIO), "ERR_MAX_IN_RATIO");

      inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

      _mintPoolShare(poolAmountOut);
      _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountOut);
      _pullUnderlying(
          BToken<TokenStoreType>::get_msg_sender(), extended_asset(tokenAmountIn, maxAmountInx.get_extended_symbol()));

      return tokenAmountIn;
   }

   uint exitswapPoolAmountIn(uint poolAmountIn, const extended_asset& minAmountOutx) {
      namesym tokenOut     = to_namesym(minAmountOutx.get_extended_symbol());
      uint    minAmountOut = minAmountOutx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");

      Record outRecord = pool_store.records[tokenOut];

      uint tokenAmountOut = calcSingleOutGivenPoolIn(
          outRecord.balance, outRecord.denorm, BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
          poolAmountIn, pool_store.swapFee);

      require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

      require(tokenAmountOut <= BMath::bmul(pool_store.records[tokenOut].balance, MAX_OUT_RATIO), "ERR_MAX_OUT_RATIO");

      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

      _pullPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountIn);
      _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
      _pushPoolShare(pool_store.factory, exitFee);

      _pushUnderlying(
          BToken<TokenStoreType>::get_msg_sender(),
          extended_asset(tokenAmountOut, minAmountOutx.get_extended_symbol()));

      return tokenAmountOut;
   }

   uint exitswapExternAmountOut(const extended_asset& tokenAmountOutx, uint maxPoolAmountIn) {
      namesym tokenOut       = to_namesym(tokenAmountOutx.get_extended_symbol());
      uint    tokenAmountOut = tokenAmountOutx.quantity.amount;

      require(pool_store.finalized, "ERR_NOT_FINALIZED");
      require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
      require(tokenAmountOut <= BMath::bmul(pool_store.records[tokenOut].balance, MAX_OUT_RATIO), "ERR_MAX_OUT_RATIO");

      Record outRecord = pool_store.records[tokenOut];

      uint poolAmountIn = calcPoolInGivenSingleOut(
          outRecord.balance, outRecord.denorm, BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
          tokenAmountOut, pool_store.swapFee);

      require(poolAmountIn != 0, "ERR_MATH_APPROX");
      require(poolAmountIn <= maxPoolAmountIn, "ERR_LIMIT_IN");

      outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

      uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

      _pullPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountIn);
      _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
      _pushPoolShare(pool_store.factory, exitFee);
      _pushUnderlying(BToken<TokenStoreType>::get_msg_sender(), tokenAmountOutx);

      return poolAmountIn;
   }

   // ==
   // 'Underlying' token-manipulation functions make external calls but are NOT
   // locked You must `_lock_` or otherwise ensure reentry-safety
   void _pullUnderlying(name from, const extended_asset& amountx) {
      namesym token  = to_namesym(amountx.get_extended_symbol());
      uint    amount = amountx.quantity.amount;
      /// transfer memo implementation
      factory.setMsgSender(BToken<TokenStoreType>::get_msg_sender());
      factory.token(token, [&](auto& _token_) {
         factory.get_transfer_mgmt().transfer(from, BToken<TokenStoreType>::get_self(), amountx, "");
         bool xfer = _token_.transferFrom(from, BToken<TokenStoreType>::get_self(), amount);
         require(xfer, "ERR_ERC20_FALSE");
      });
   }

   void _pushUnderlying(name to, const extended_asset& amountx) {
      namesym token  = to_namesym(amountx.get_extended_symbol());
      uint    amount = amountx.quantity.amount;
      factory.setMsgSender(BToken<TokenStoreType>::get_msg_sender());
      factory.token(token, [&](auto& _token_) {
         factory.get_transfer_mgmt().transfer(BToken<TokenStoreType>::get_self(), to, amountx, "");
         bool xfer = _token_.transfer(to, amount);
         require(xfer, "ERR_ERC20_FALSE");
      });
   }

   void _pullPoolShare(name from, uint amount) { BToken<TokenStoreType>::_pull(from, amount); }

   void _pushPoolShare(name to, uint amount) { BToken<TokenStoreType>::_push(to, amount); }

   void _mintPoolShare(uint amount) { BToken<TokenStoreType>::_mint(amount); }

   void _burnPoolShare(uint amount) { BToken<TokenStoreType>::_burn(amount); }
};
