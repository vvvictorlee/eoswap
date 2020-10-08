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
  FactoryType &factory;
  BPoolStore &pool_store;

public:
  BPool(name _self, FactoryType &_factory, PoolStoreType &_pool_store,
        TokenStoreType &_tokenStore)
      : factory(_factory),
        pool_store(_pool_store), BToken<TokenStoreType>(_self, _tokenStore) {}

  ~BPool() {}

  class Lock {
    BPoolStore &pool_store;

  public:
    Lock(BPoolStore &_pool_store) : pool_store(_pool_store) {
      require(!pool_store.mutex, "ERR_REENTRY");
      pool_store.mutex = true;
    }
    ~Lock() { pool_store.mutex = false; }
  };

  void init() {
    pool_store.controller = BToken<TokenStoreType>::get_self();
    pool_store.factory = BToken<TokenStoreType>::get_self();
    pool_store.swapFee = MIN_FEE;
    pool_store.publicSwap = false;
    pool_store.finalized = false;
  }

  bool isPublicSwap() { return pool_store.publicSwap; }

  bool isFinalized() { return pool_store.finalized; }

  bool isBound(name t) { return pool_store.records[t].bound; }

  uint getNumTokens() { return pool_store.tokens.size(); }

  std::vector<name> getCurrentTokens() { return pool_store.tokens; }

  std::vector<name> getFinalTokens() {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");
    return pool_store.tokens;
  }

  uint getDenormalizedWeight(name token) {
    require(pool_store.records[token].bound, "ERR_NOT_BOUND");
    return pool_store.records[token].denorm;
  }

  uint getTotalDenormalizedWeight() { return pool_store.totalWeight; }

  uint getNormalizedWeight(name token) {

    require(pool_store.records[token].bound, "ERR_NOT_BOUND");
    uint denorm = pool_store.records[token].denorm;
    return BMath::bdiv(denorm, pool_store.totalWeight);
  }

  uint getBalance(name token) {
    require(pool_store.records[token].bound, "ERR_NOT_BOUND");
    return pool_store.records[token].balance;
  }

  uint getSwapFee() { return pool_store.swapFee; }

  name getController() { return pool_store.controller; }

  void setSwapFee(uint swapFee) {
    require(!pool_store.finalized, "ERR_IS_FINALIZED");
    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    require(swapFee >= MIN_FEE, "ERR_MIN_FEE");
    require(swapFee <= MAX_FEE, "ERR_MAX_FEE");
    pool_store.swapFee = swapFee;
  }

  void setController(name manager) {
    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    pool_store.controller = manager;
  }

  void setPublicSwap(bool public_) {
    require(!pool_store.finalized, "ERR_IS_FINALIZED");
    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    pool_store.publicSwap = public_;
  }

  void finalize() {
    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    require(!pool_store.finalized, "ERR_IS_FINALIZED");
    require(pool_store.tokens.size() >= MIN_BOUND_TOKENS, "ERR_MIN_TOKENS");

    pool_store.finalized = true;
    pool_store.publicSwap = true;

    _mintPoolShare(INIT_POOL_SUPPLY);
    _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), INIT_POOL_SUPPLY);
  }

  void bind(name token, uint balance, uint denorm)
  // _lock_  Bind does not lock because it jumps to `rebind`, which does
  {
    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    require(!pool_store.records[token].bound, "ERR_IS_BOUND");
    require(!pool_store.finalized, "ERR_IS_FINALIZED");

    require(pool_store.tokens.size() < MAX_BOUND_TOKENS, "ERR_MAX_TOKENS");

    pool_store.records[token] = Record({
        true, pool_store.tokens.size(),
        0, // balance and denorm will be validated
        0  // and set by `rebind`
    });
    pool_store.tokens.push_back(token);
    rebind(token, balance, denorm);
  }

  void rebind(name token, uint balance, uint denorm) {

    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    require(pool_store.records[token].bound, "ERR_NOT_BOUND");
    require(!pool_store.finalized, "ERR_IS_FINALIZED");

    require(denorm >= MIN_WEIGHT, "ERR_MIN_WEIGHT");
    require(denorm <= MAX_WEIGHT, "ERR_MAX_WEIGHT");
    require(balance >= MIN_BALANCE, "ERR_MIN_BALANCE");

    // Adjust the denorm and totalWeight
    uint oldWeight = pool_store.records[token].denorm;
    if (denorm > oldWeight) {
      pool_store.totalWeight =
          BMath::badd(pool_store.totalWeight, BMath::bsub(denorm, oldWeight));
      require(pool_store.totalWeight <= MAX_TOTAL_WEIGHT,
              "ERR_MAX_TOTAL_WEIGHT");
    } else if (denorm < oldWeight) {
      pool_store.totalWeight =
          BMath::bsub(pool_store.totalWeight, BMath::bsub(oldWeight, denorm));
    }
    pool_store.records[token].denorm = denorm;

    // Adjust the balance record and actual token balance
    uint oldBalance = pool_store.records[token].balance;
    pool_store.records[token].balance = balance;
    if (balance > oldBalance) {
      _pullUnderlying(token, BToken<TokenStoreType>::get_msg_sender(),
                      BMath::bsub(balance, oldBalance));
    } else if (balance < oldBalance) {
      // In this case liquidity is being withdrawn, so charge EXIT_FEE
      uint tokenBalanceWithdrawn = BMath::bsub(oldBalance, balance);
      uint tokenExitFee = BMath::bmul(tokenBalanceWithdrawn, EXIT_FEE);
      _pushUnderlying(token, BToken<TokenStoreType>::get_msg_sender(),
                      BMath::bsub(tokenBalanceWithdrawn, tokenExitFee));
      _pushUnderlying(token, pool_store.factory, tokenExitFee);
    }
  }

  void unbind(name token) {

    require(BToken<TokenStoreType>::get_msg_sender() == pool_store.controller,
            "ERR_NOT_CONTROLLER");
    require(pool_store.records[token].bound, "ERR_NOT_BOUND");
    require(!pool_store.finalized, "ERR_IS_FINALIZED");

    uint tokenBalance = pool_store.records[token].balance;
    uint tokenExitFee = BMath::bmul(tokenBalance, EXIT_FEE);

    pool_store.totalWeight =
        BMath::bsub(pool_store.totalWeight, pool_store.records[token].denorm);

    // Swap the token-to-unbind with the last token,
    // then delete the last token
    uint index = pool_store.records[token].index;
    uint last = pool_store.tokens.size() - 1;
    pool_store.tokens[index] = pool_store.tokens[last];
    pool_store.records[pool_store.tokens[index]].index = index;
    pool_store.tokens.pop_back();
    pool_store.records[token] = Record({false, 0, 0, 0});

    _pushUnderlying(token, BToken<TokenStoreType>::get_msg_sender(),
                    BMath::bsub(tokenBalance, tokenExitFee));
    _pushUnderlying(token, pool_store.factory, tokenExitFee);
  }

  // Absorb any tokens that have been sent to this contract into the pool
  void gulp(name token) {
    require(pool_store.records[token].bound, "ERR_NOT_BOUND");
    factory.setMsgSender(BToken<TokenStoreType>::get_msg_sender());
    factory.token(token, [&](auto &_token_) {
      pool_store.records[token].balance =
          _token_.balanceOf(BToken<TokenStoreType>::get_msg_sender());
    });
  }

  uint getSpotPrice(name tokenIn, name tokenOut) {
    require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
    Record inRecord = pool_store.records[tokenIn];
    Record outRecord = pool_store.records[tokenOut];
    return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                         outRecord.denorm, pool_store.swapFee);
  }

  uint getSpotPriceSansFee(name tokenIn, name tokenOut) {
    require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
    Record inRecord = pool_store.records[tokenIn];
    Record outRecord = pool_store.records[tokenOut];
    return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                         outRecord.denorm, 0);
  }

  void joinPool(uint poolAmountOut, std::vector<uint> maxAmountsIn) {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");

    uint poolTotal = BToken<TokenStoreType>::totalSupply();
    uint ratio = BMath::bdiv(poolAmountOut, poolTotal);
    require(ratio != 0, "ERR_MATH_APPROX");

    for (uint i = 0; i < pool_store.tokens.size(); i++) {
      name t = pool_store.tokens[i];
      uint bal = pool_store.records[t].balance;
      uint tokenAmountIn = BMath::bmul(ratio, bal);

      require(tokenAmountIn != 0, "ERR_MATH_APPROX");
      require(tokenAmountIn <= maxAmountsIn[i], "ERR_LIMIT_IN joinPool");
      pool_store.records[t].balance =
          BMath::badd(pool_store.records[t].balance, tokenAmountIn);
      _pullUnderlying(t, BToken<TokenStoreType>::get_msg_sender(),
                      tokenAmountIn);
    }
    _mintPoolShare(poolAmountOut);
    _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountOut);
  }

  void exitPool(uint poolAmountIn, std::vector<uint> minAmountsOut) {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");

    uint poolTotal = BToken<TokenStoreType>::totalSupply();
    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);
    uint pAiAfterExitFee = BMath::bsub(poolAmountIn, exitFee);
    uint ratio = BMath::bdiv(pAiAfterExitFee, poolTotal);
    require(ratio != 0, "ERR_MATH_APPROX");

    _pullPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountIn);
    _pushPoolShare(pool_store.factory, exitFee);
    _burnPoolShare(pAiAfterExitFee);

    for (uint i = 0; i < pool_store.tokens.size(); i++) {
      name t = pool_store.tokens[i];
      uint bal = pool_store.records[t].balance;
      uint tokenAmountOut = BMath::bmul(ratio, bal);
      require(tokenAmountOut != 0, "ERR_MATH_APPROX");
      require(tokenAmountOut >= minAmountsOut[i], "ERR_LIMIT_OUT");
      pool_store.records[t].balance =
          BMath::bsub(pool_store.records[t].balance, tokenAmountOut);
      _pushUnderlying(t, BToken<TokenStoreType>::get_msg_sender(),
                      tokenAmountOut);
    }
  }

  std::pair<uint, uint> swapExactAmountIn(name tokenIn, uint tokenAmountIn,
                                          name tokenOut, uint minAmountOut,
                                          uint maxPrice) {

    require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
    require(pool_store.publicSwap, "ERR_SWAP_NOT_PUBLIC");

    Record inRecord = pool_store.records[tokenIn];
    Record outRecord = pool_store.records[tokenOut];

    require(tokenAmountIn <= BMath::bmul(inRecord.balance, MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    uint spotPriceBefore =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, pool_store.swapFee);
    require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

    uint tokenAmountOut =
        calcOutGivenIn(inRecord.balance, inRecord.denorm, outRecord.balance,
                       outRecord.denorm, tokenAmountIn, pool_store.swapFee);
    require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);
    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint spotPriceAfter =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, pool_store.swapFee);
    require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
    require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
    require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
            "ERR_MATH_APPROX");

    _pullUnderlying(tokenIn, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountIn);
    _pushUnderlying(tokenOut, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountOut);

    return std::make_pair(tokenAmountOut, spotPriceAfter);
  }

  std::pair<uint, uint> swapExactAmountOut(name tokenIn, uint maxAmountIn,
                                           name tokenOut, uint tokenAmountOut,
                                           uint maxPrice) {
    require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
    require(pool_store.publicSwap, "ERR_SWAP_NOT_PUBLIC");

    Record inRecord = pool_store.records[tokenIn];
    Record outRecord = pool_store.records[tokenOut];

    require(tokenAmountOut <= BMath::bmul(outRecord.balance, MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    uint spotPriceBefore =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, pool_store.swapFee);
    require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

    uint tokenAmountIn =
        calcInGivenOut(inRecord.balance, inRecord.denorm, outRecord.balance,
                       outRecord.denorm, tokenAmountOut, pool_store.swapFee);
    require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);
    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint spotPriceAfter =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, pool_store.swapFee);
    require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
    require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
    require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
            "ERR_MATH_APPROX");

    _pullUnderlying(tokenIn, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountIn);
    _pushUnderlying(tokenOut, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountOut);

    return std::make_pair(tokenAmountIn, spotPriceAfter);
  }

  uint joinswapExternAmountIn(name tokenIn, uint tokenAmountIn,
                              uint minPoolAmountOut) {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");
    require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(tokenAmountIn <=
                BMath::bmul(pool_store.records[tokenIn].balance, MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    Record inRecord = pool_store.records[tokenIn];

    uint poolAmountOut = calcPoolOutGivenSingleIn(
        inRecord.balance, inRecord.denorm,
        BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
        tokenAmountIn, pool_store.swapFee);

    require(poolAmountOut >= minPoolAmountOut, "ERR_LIMIT_OUT");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

    _mintPoolShare(poolAmountOut);
    _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountOut);
    _pullUnderlying(tokenIn, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountIn);

    return poolAmountOut;
  }

  uint joinswapPoolAmountOut(name tokenIn, uint poolAmountOut,
                             uint maxAmountIn) {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");
    require(pool_store.records[tokenIn].bound, "ERR_NOT_BOUND");

    Record inRecord = pool_store.records[tokenIn];

    uint tokenAmountIn = calcSingleInGivenPoolOut(
        inRecord.balance, inRecord.denorm,
        BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
        poolAmountOut, pool_store.swapFee);

    require(tokenAmountIn != 0, "ERR_MATH_APPROX");
    require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

    require(tokenAmountIn <=
                BMath::bmul(pool_store.records[tokenIn].balance, MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

    _mintPoolShare(poolAmountOut);
    _pushPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountOut);
    _pullUnderlying(tokenIn, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountIn);

    return tokenAmountIn;
  }

  uint exitswapPoolAmountIn(name tokenOut, uint poolAmountIn,
                            uint minAmountOut) {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");
    require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");

    Record outRecord = pool_store.records[tokenOut];

    uint tokenAmountOut = calcSingleOutGivenPoolIn(
        outRecord.balance, outRecord.denorm,
        BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
        poolAmountIn, pool_store.swapFee);

    require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

    require(tokenAmountOut <= BMath::bmul(pool_store.records[tokenOut].balance,
                                          MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

    _pullPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountIn);
    _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
    _pushPoolShare(pool_store.factory, exitFee);
    _pushUnderlying(tokenOut, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountOut);

    return tokenAmountOut;
  }

  uint exitswapExternAmountOut(name tokenOut, uint tokenAmountOut,
                               uint maxPoolAmountIn) {
    require(pool_store.finalized, "ERR_NOT_FINALIZED");
    require(pool_store.records[tokenOut].bound, "ERR_NOT_BOUND");
    require(tokenAmountOut <= BMath::bmul(pool_store.records[tokenOut].balance,
                                          MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    Record outRecord = pool_store.records[tokenOut];

    uint poolAmountIn = calcPoolInGivenSingleOut(
        outRecord.balance, outRecord.denorm,
        BToken<TokenStoreType>::totalSupply(), pool_store.totalWeight,
        tokenAmountOut, pool_store.swapFee);

    require(poolAmountIn != 0, "ERR_MATH_APPROX");
    require(poolAmountIn <= maxPoolAmountIn, "ERR_LIMIT_IN");

    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

    _pullPoolShare(BToken<TokenStoreType>::get_msg_sender(), poolAmountIn);
    _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
    _pushPoolShare(pool_store.factory, exitFee);
    _pushUnderlying(tokenOut, BToken<TokenStoreType>::get_msg_sender(),
                    tokenAmountOut);

    return poolAmountIn;
  }

  // ==
  // 'Underlying' token-manipulation functions make external calls but are NOT
  // locked You must `_lock_` or otherwise ensure reentry-safety
  void _pullUnderlying(name token, name from, uint amount) {
    /// transfer memo implementation
    factory.setMsgSender(BToken<TokenStoreType>::get_msg_sender());
    factory.token(token, [&](auto &_token_) {
      bool xfer = _token_.transferFrom(from, BToken<TokenStoreType>::get_self(),
                                       amount);
      require(xfer, "ERR_ERC20_FALSE");
    });
  }

  void _pushUnderlying(name token, name to, uint amount) {
    factory.setMsgSender(BToken<TokenStoreType>::get_msg_sender());
    factory.token(token, [&](auto &_token_) {
      bool xfer = _token_.transfer(to, amount);
      require(xfer, "ERR_ERC20_FALSE");
    });
  }

  void _pullPoolShare(name from, uint amount) {
    BToken<TokenStoreType>::_pull(from, amount);
  }

  void _pushPoolShare(name to, uint amount) {
    BToken<TokenStoreType>::_push(to, amount);
  }

  void _mintPoolShare(uint amount) { BToken<TokenStoreType>::_mint(amount); }

  void _burnPoolShare(uint amount) { BToken<TokenStoreType>::_burn(amount); }
};
