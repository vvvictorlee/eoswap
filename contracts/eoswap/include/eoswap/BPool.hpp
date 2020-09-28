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

class BPool : public BToken, public BMath {

private:
  name self;
  BPoolStorageSingleton pool_storage_singleton;
  BPoolStorage _pool_storage;
  bool testFlag;

public:
  BPool(name _self, bool test = false)
      : self(_self), pool_storage_singleton(_self, _self.value), BToken(_self),
        testFlag(test)

  {
    if (!testFlag) {
      require_auth(self);
    }
    _pool_storage = pool_storage_singleton.exists()
                        ? pool_storage_singleton.get()
                        : BPoolStorage{};
  }
  ~BPool() { pool_storage_singleton.set(_pool_storage, self); }

  class Lock {
    BPoolStorage &pool_storage;
    Lock(BPoolStorage &_pool_storage) : pool_storage(_pool_storage) {
      require(!pool_storage.mutex, "ERR_REENTRY");
      pool_storage.mutex = true;
    }
    ~Lock() { pool_storage.mutex = false; }
  };

  void initBPool(name self, name factory) {
    _pool_storage.controller = self;
    _pool_storage.factory = factory;
    _pool_storage.swapFee = MIN_FEE;
    _pool_storage.publicSwap = false;
    _pool_storage.finalized = false;
  }

  bool isPublicSwap() { return _pool_storage.publicSwap; }

  bool isFinalized() { return _pool_storage.finalized; }

  bool isBound(name t) { return _pool_storage.records[t].bound; }

  uint getNumTokens() { return _pool_storage.tokens.size(); }

  std::vector<name> getCurrentTokens() { return _pool_storage.tokens; }

  std::vector<name> getFinalTokens() {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");
    return _pool_storage.tokens;
  }

  uint getDenormalizedWeight(name token) {
    require(_pool_storage.records[token].bound, "ERR_NOT_BOUND");
    return _pool_storage.records[token].denorm;
  }

  uint getTotalDenormalizedWeight() { return _pool_storage.totalWeight; }

  uint getNormalizedWeight(name token) {

    require(_pool_storage.records[token].bound, "ERR_NOT_BOUND");
    uint denorm = _pool_storage.records[token].denorm;
    return BMath::bdiv(denorm, _pool_storage.totalWeight);
  }

  uint getBalance(name token) {
    require(_pool_storage.records[token].bound, "ERR_NOT_BOUND");
    return _pool_storage.records[token].balance;
  }

  uint getSwapFee() { return _pool_storage.swapFee; }

  name getController() { return _pool_storage.controller; }

  void setSwapFee(uint swapFee) {
    require(!_pool_storage.finalized, "ERR_IS_FINALIZED");
    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    require(swapFee >= MIN_FEE, "ERR_MIN_FEE");
    require(swapFee <= MAX_FEE, "ERR_MAX_FEE");
    _pool_storage.swapFee = swapFee;
  }

  void setController(name manager) {
    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    _pool_storage.controller = manager;
  }

  void setPublicSwap(bool public_) {
    require(!_pool_storage.finalized, "ERR_IS_FINALIZED");
    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    _pool_storage.publicSwap = public_;
  }

  void finalize(name self) {
    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    require(!_pool_storage.finalized, "ERR_IS_FINALIZED");
    require(_pool_storage.tokens.size() >= MIN_BOUND_TOKENS,
            "ERR_MIN_pool_storage.tokens");

    _pool_storage.finalized = true;
    _pool_storage.publicSwap = true;

    _mintPoolShare(INIT_POOL_SUPPLY);
    _pushPoolShare(self, INIT_POOL_SUPPLY);
  }

  void bind(name token, uint balance, uint denorm)
  // _lock_  Bind does not lock because it jumps to `rebind`, which does
  {
    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    require(!_pool_storage.records[token].bound, "ERR_IS_BOUND");
    require(!_pool_storage.finalized, "ERR_IS_FINALIZED");

    require(_pool_storage.tokens.size() < MAX_BOUND_TOKENS, "ERR_MAX_TOKENS");

    _pool_storage.records[token] = Record({
        true, _pool_storage.tokens.size(),
        0, // balance and denorm will be validated
        0  // and set by `rebind`
    });
    _pool_storage.tokens.push_back(token);
    rebind(token, balance, denorm);
  }

  void rebind(name token, uint balance, uint denorm) {

    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    require(_pool_storage.records[token].bound, "ERR_NOT_BOUND");
    require(!_pool_storage.finalized, "ERR_IS_FINALIZED");

    require(denorm >= MIN_WEIGHT, "ERR_MIN_WEIGHT");
    require(denorm <= MAX_WEIGHT, "ERR_MAX_WEIGHT");
    require(balance >= MIN_BALANCE, "ERR_MIN_BALANCE");

    // Adjust the denorm and totalWeight
    uint oldWeight = _pool_storage.records[token].denorm;
    if (denorm > oldWeight) {
      _pool_storage.totalWeight = BMath::BMath::badd(
          _pool_storage.totalWeight, BMath::bsub(denorm, oldWeight));
      require(_pool_storage.totalWeight <= MAX_TOTAL_WEIGHT,
              "ERR_MAX_TOTAL_WEIGHT");
    } else if (denorm < oldWeight) {
      _pool_storage.totalWeight = BMath::bsub(_pool_storage.totalWeight,
                                              BMath::bsub(oldWeight, denorm));
    }
    _pool_storage.records[token].denorm = denorm;

    // Adjust the balance record and actual token balance
    uint oldBalance = _pool_storage.records[token].balance;
    _pool_storage.records[token].balance = balance;
    if (balance > oldBalance) {
      _pullUnderlying(token, self, BMath::bsub(balance, oldBalance));
    } else if (balance < oldBalance) {
      // In this case liquidity is being withdrawn, so charge EXIT_FEE
      uint tokenBalanceWithdrawn = BMath::bsub(oldBalance, balance);
      uint tokenExitFee = BMath::bmul(tokenBalanceWithdrawn, EXIT_FEE);
      _pushUnderlying(token, self,
                      BMath::bsub(tokenBalanceWithdrawn, tokenExitFee));
      _pushUnderlying(token, _pool_storage.factory, tokenExitFee);
    }
  }

  void unbind(name token) {

    require(self == _pool_storage.controller, "ERR_NOT_CONTROLLER");
    require(_pool_storage.records[token].bound, "ERR_NOT_BOUND");
    require(!_pool_storage.finalized, "ERR_IS_FINALIZED");

    uint tokenBalance = _pool_storage.records[token].balance;
    uint tokenExitFee = BMath::bmul(tokenBalance, EXIT_FEE);

    _pool_storage.totalWeight = BMath::bsub(
        _pool_storage.totalWeight, _pool_storage.records[token].denorm);

    // Swap the token-to-unbind with the last token,
    // then delete the last token
    uint index = _pool_storage.records[token].index;
    uint last = _pool_storage.tokens.size() - 1;
    _pool_storage.tokens[index] = _pool_storage.tokens[last];
    _pool_storage.records[_pool_storage.tokens[index]].index = index;
    _pool_storage.tokens.pop_back();
    _pool_storage.records[token] = Record({false, 0, 0, 0});

    _pushUnderlying(token, self, BMath::bsub(tokenBalance, tokenExitFee));
    _pushUnderlying(token, _pool_storage.factory, tokenExitFee);
  }

  // Absorb any tokens that have been sent to this contract into the pool
  void gulp(name token) {
    require(_pool_storage.records[token].bound, "ERR_NOT_BOUND");
    _pool_storage.records[token].balance = this->balanceOf(self);
  }

  uint getSpotPrice(name tokenIn, name tokenOut) {
    require(_pool_storage.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(_pool_storage.records[tokenOut].bound, "ERR_NOT_BOUND");
    Record inRecord = _pool_storage.records[tokenIn];
    Record outRecord = _pool_storage.records[tokenOut];
    return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                         outRecord.denorm, _pool_storage.swapFee);
  }

  uint getSpotPriceSansFee(name tokenIn, name tokenOut) {
    require(_pool_storage.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(_pool_storage.records[tokenOut].bound, "ERR_NOT_BOUND");
    Record inRecord = _pool_storage.records[tokenIn];
    Record outRecord = _pool_storage.records[tokenOut];
    return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                         outRecord.denorm, 0);
  }

  void joinPool(uint poolAmountOut, std::vector<uint> maxAmountsIn) {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");

    uint poolTotal = totalSupply();
    uint ratio = BMath::bdiv(poolAmountOut, poolTotal);
    require(ratio != 0, "ERR_MATH_APPROX");

    for (uint i = 0; i < _pool_storage.tokens.size(); i++) {
      name t = _pool_storage.tokens[i];
      uint bal = _pool_storage.records[t].balance;
      uint tokenAmountIn = BMath::bmul(ratio, bal);
      require(tokenAmountIn != 0, "ERR_MATH_APPROX");
      require(tokenAmountIn <= maxAmountsIn[i], "ERR_LIMIT_IN");
      _pool_storage.records[t].balance =
          BMath::badd(_pool_storage.records[t].balance, tokenAmountIn);
      _pullUnderlying(t, self, tokenAmountIn);
    }
    _mintPoolShare(poolAmountOut);
    _pushPoolShare(self, poolAmountOut);
  }

  void exitPool(uint poolAmountIn, std::vector<uint> minAmountsOut) {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");

    uint poolTotal = totalSupply();
    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);
    uint pAiAfterExitFee = BMath::bsub(poolAmountIn, exitFee);
    uint ratio = BMath::bdiv(pAiAfterExitFee, poolTotal);
    require(ratio != 0, "ERR_MATH_APPROX");

    _pullPoolShare(self, poolAmountIn);
    _pushPoolShare(_pool_storage.factory, exitFee);
    _burnPoolShare(pAiAfterExitFee);

    for (uint i = 0; i < _pool_storage.tokens.size(); i++) {
      name t = _pool_storage.tokens[i];
      uint bal = _pool_storage.records[t].balance;
      uint tokenAmountOut = BMath::bmul(ratio, bal);
      require(tokenAmountOut != 0, "ERR_MATH_APPROX");
      require(tokenAmountOut >= minAmountsOut[i], "ERR_LIMIT_OUT");
      _pool_storage.records[t].balance =
          BMath::bsub(_pool_storage.records[t].balance, tokenAmountOut);
      _pushUnderlying(t, self, tokenAmountOut);
    }
  }

  std::pair<uint, uint> swapExactAmountIn(name tokenIn, uint tokenAmountIn,
                                          name tokenOut, uint minAmountOut,
                                          uint maxPrice) {

    require(_pool_storage.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(_pool_storage.records[tokenOut].bound, "ERR_NOT_BOUND");
    require(_pool_storage.publicSwap, "ERR_SWAP_NOT_PUBLIC");

    Record inRecord = _pool_storage.records[name(tokenIn)];
    Record outRecord = _pool_storage.records[name(tokenOut)];

    require(tokenAmountIn <= BMath::bmul(inRecord.balance, MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    uint spotPriceBefore =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.swapFee);
    require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

    uint tokenAmountOut =
        calcOutGivenIn(inRecord.balance, inRecord.denorm, outRecord.balance,
                       outRecord.denorm, tokenAmountIn, _pool_storage.swapFee);
    require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);
    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint spotPriceAfter =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.swapFee);
    require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
    require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
    require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
            "ERR_MATH_APPROX");

    _pullUnderlying(tokenIn, self, tokenAmountIn);
    _pushUnderlying(tokenOut, self, tokenAmountOut);

    return std::make_pair(tokenAmountOut, spotPriceAfter);
  }

  std::pair<uint, uint> swapExactAmountOut(name tokenIn, uint maxAmountIn,
                                           name tokenOut, uint tokenAmountOut,
                                           uint maxPrice) {
    require(_pool_storage.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(_pool_storage.records[tokenOut].bound, "ERR_NOT_BOUND");
    require(_pool_storage.publicSwap, "ERR_SWAP_NOT_PUBLIC");

    Record inRecord = _pool_storage.records[name(tokenIn)];
    Record outRecord = _pool_storage.records[name(tokenOut)];

    require(tokenAmountOut <= BMath::bmul(outRecord.balance, MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    uint spotPriceBefore =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.swapFee);
    require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

    uint tokenAmountIn =
        calcInGivenOut(inRecord.balance, inRecord.denorm, outRecord.balance,
                       outRecord.denorm, tokenAmountOut, _pool_storage.swapFee);
    require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);
    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint spotPriceAfter =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.swapFee);
    require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
    require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
    require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
            "ERR_MATH_APPROX");

    _pullUnderlying(tokenIn, self, tokenAmountIn);
    _pushUnderlying(tokenOut, self, tokenAmountOut);

    return std::make_pair(tokenAmountIn, spotPriceAfter);
  }

  uint joinswapExternAmountIn(name tokenIn, uint tokenAmountIn,
                              uint minPoolAmountOut) {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.records[tokenIn].bound, "ERR_NOT_BOUND");
    require(tokenAmountIn <= BMath::bmul(_pool_storage.records[tokenIn].balance,
                                         MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    Record inRecord = _pool_storage.records[tokenIn];

    uint poolAmountOut = calcPoolOutGivenSingleIn(
        inRecord.balance, inRecord.denorm, _token_storage.totalSupply,
        _pool_storage.totalWeight, tokenAmountIn, _pool_storage.swapFee);

    require(poolAmountOut >= minPoolAmountOut, "ERR_LIMIT_OUT");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

    _mintPoolShare(poolAmountOut);
    _pushPoolShare(self, poolAmountOut);
    _pullUnderlying(tokenIn, self, tokenAmountIn);

    return poolAmountOut;
  }

  uint joinswapPoolAmountOut(name tokenIn, uint poolAmountOut,
                             uint maxAmountIn) {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.records[tokenIn].bound, "ERR_NOT_BOUND");

    Record inRecord = _pool_storage.records[tokenIn];

    uint tokenAmountIn = calcSingleInGivenPoolOut(
        inRecord.balance, inRecord.denorm, _token_storage.totalSupply,
        _pool_storage.totalWeight, poolAmountOut, _pool_storage.swapFee);

    require(tokenAmountIn != 0, "ERR_MATH_APPROX");
    require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

    require(tokenAmountIn <= BMath::bmul(_pool_storage.records[tokenIn].balance,
                                         MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

    _mintPoolShare(poolAmountOut);
    _pushPoolShare(self, poolAmountOut);
    _pullUnderlying(tokenIn, self, tokenAmountIn);

    return tokenAmountIn;
  }

  uint exitswapPoolAmountIn(name tokenOut, uint poolAmountIn,
                            uint minAmountOut) {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.records[tokenOut].bound, "ERR_NOT_BOUND");

    Record outRecord = _pool_storage.records[tokenOut];

    uint tokenAmountOut = calcSingleOutGivenPoolIn(
        outRecord.balance, outRecord.denorm, _token_storage.totalSupply,
        _pool_storage.totalWeight, poolAmountIn, _pool_storage.swapFee);

    require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

    require(
        tokenAmountOut <=
            BMath::bmul(_pool_storage.records[tokenOut].balance, MAX_OUT_RATIO),
        "ERR_MAX_OUT_RATIO");

    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

    _pullPoolShare(self, poolAmountIn);
    _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
    _pushPoolShare(_pool_storage.factory, exitFee);
    _pushUnderlying(tokenOut, self, tokenAmountOut);

    return tokenAmountOut;
  }

  uint exitswapExternAmountOut(name tokenOut, uint tokenAmountOut,
                               uint maxPoolAmountIn) {
    require(_pool_storage.finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.records[tokenOut].bound, "ERR_NOT_BOUND");
    require(
        tokenAmountOut <=
            BMath::bmul(_pool_storage.records[tokenOut].balance, MAX_OUT_RATIO),
        "ERR_MAX_OUT_RATIO");

    Record outRecord = _pool_storage.records[tokenOut];

    uint poolAmountIn = calcPoolInGivenSingleOut(
        outRecord.balance, outRecord.denorm, _token_storage.totalSupply,
        _pool_storage.totalWeight, tokenAmountOut, _pool_storage.swapFee);

    require(poolAmountIn != 0, "ERR_MATH_APPROX");
    require(poolAmountIn <= maxPoolAmountIn, "ERR_LIMIT_IN");

    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

    _pullPoolShare(self, poolAmountIn);
    _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
    _pushPoolShare(_pool_storage.factory, exitFee);
    _pushUnderlying(tokenOut, self, tokenAmountOut);

    return poolAmountIn;
  }

  // ==
  // 'Underlying' token-manipulation functions make external calls but are NOT
  // locked You must `_lock_` or otherwise ensure reentry-safety

  void _pullUnderlying(name erc20, name from, uint amount) {
    bool xfer = this->transferFrom(from, from, self, amount);
    require(xfer, "ERR_ERC20_FALSE");
  }

  void _pushUnderlying(name erc20, name to, uint amount) {
    bool xfer = this->transfer(self, to, amount);
    require(xfer, "ERR_ERC20_FALSE");
  }

  void _pullPoolShare(name from, uint amount) { _pull(from, amount); }

  void _pushPoolShare(name to, uint amount) { _push(to, amount); }

  void _mintPoolShare(uint amount) { _mint(amount); }

  void _burnPoolShare(uint amount) { _burn(amount); }
};
