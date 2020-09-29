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

class BPool : public BMath {

private:
  name self;
  name msg_sender;
  name pool_name;
  BToken &tokens;
  BPoolStorageSingleton pool_storage_singleton;
  BPoolStorage _pool_storage;

public:
  BPool(name _self, BToken &_token)
      : self(_self), tokens(_token),
        pool_storage_singleton(_self, _self.value) {
    _pool_storage = pool_storage_singleton.exists()
                        ? pool_storage_singleton.get()
                        : BPoolStorage{};
  }
  ~BPool() { pool_storage_singleton.set(_pool_storage, self); }

  class Lock {
    BPoolStore &pool_store;
    Lock(BPoolStore &_pool_store) : pool_store(_pool_store) {
      require(!pool_store.mutex, "ERR_REENTRY");
      pool_store.mutex = true;
    }
    ~Lock() { pool_store.mutex = false; }
  };

  BToken &getToken() { return tokens; }

  void auth(name _msg_sender, name _pool_name) {
    msg_sender = _msg_sender;
    pool_name = _pool_name;
    tokens.initToken(msg_sender, pool_name);
    auto p = _pool_storage.pools.find(pool_name);
    bool b = p != _pool_storage.pools.end();
    require(b, "NO_POOL");
  }

  void initBPool(name _msg_sender, name pool_name) {
    tokens.initToken(_msg_sender, pool_name);

    auto p = _pool_storage.pools.find(pool_name);
    if (p == _pool_storage.pools.end()) {
      _pool_storage.pools.insert(
          std::map<name, BPoolStore>::value_type(pool_name, BPoolStore()));
    }

    _pool_storage.pools[pool_name].controller = _msg_sender;
    _pool_storage.pools[pool_name].factory = self;
    _pool_storage.pools[pool_name].swapFee = MIN_FEE;
    _pool_storage.pools[pool_name].publicSwap = false;
    _pool_storage.pools[pool_name].finalized = false;
  }

  bool isPublicSwap() { return _pool_storage.pools[pool_name].publicSwap; }

  bool isFinalized() { return _pool_storage.pools[pool_name].finalized; }

  bool isBound(name t) {
    return _pool_storage.pools[pool_name].records[t].bound;
  }

  uint getNumTokens() { return _pool_storage.pools[pool_name].tokens.size(); }

  std::vector<name> getCurrentTokens() {
    return _pool_storage.pools[pool_name].tokens;
  }

  std::vector<name> getFinalTokens() {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");
    return _pool_storage.pools[pool_name].tokens;
  }

  uint getDenormalizedWeight(name token) {
    require(_pool_storage.pools[pool_name].records[token].bound,
            "ERR_NOT_BOUND");
    return _pool_storage.pools[pool_name].records[token].denorm;
  }

  uint getTotalDenormalizedWeight() {
    return _pool_storage.pools[pool_name].totalWeight;
  }

  uint getNormalizedWeight(name token) {

    require(_pool_storage.pools[pool_name].records[token].bound,
            "ERR_NOT_BOUND");
    uint denorm = _pool_storage.pools[pool_name].records[token].denorm;
    return BMath::bdiv(denorm, _pool_storage.pools[pool_name].totalWeight);
  }

  uint getBalance(name token) {
    require(_pool_storage.pools[pool_name].records[token].bound,
            "ERR_NOT_BOUND");
    return _pool_storage.pools[pool_name].records[token].balance;
  }

  uint getSwapFee() { return _pool_storage.pools[pool_name].swapFee; }

  name getController() { return _pool_storage.pools[pool_name].controller; }

  void setSwapFee(uint swapFee) {
    require(!_pool_storage.pools[pool_name].finalized, "ERR_IS_FINALIZED");
    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    require(swapFee >= MIN_FEE, "ERR_MIN_FEE");
    require(swapFee <= MAX_FEE, "ERR_MAX_FEE");
    _pool_storage.pools[pool_name].swapFee = swapFee;
  }

  void setController(name manager) {
    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    _pool_storage.pools[pool_name].controller = manager;
  }

  void setPublicSwap(bool public_) {
    require(!_pool_storage.pools[pool_name].finalized, "ERR_IS_FINALIZED");
    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    _pool_storage.pools[pool_name].publicSwap = public_;
  }

  void finalize(name msg_sender) {
    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    require(!_pool_storage.pools[pool_name].finalized, "ERR_IS_FINALIZED");
    require(_pool_storage.pools[pool_name].tokens.size() >= MIN_BOUND_TOKENS,
            "ERR_MIN_pool_storage.tokens");

    _pool_storage.pools[pool_name].finalized = true;
    _pool_storage.pools[pool_name].publicSwap = true;

    _mintPoolShare(INIT_POOL_SUPPLY);
    _pushPoolShare(msg_sender, INIT_POOL_SUPPLY);
  }

  void bind(name token, uint balance, uint denorm)
  // _lock_  Bind does not lock because it jumps to `rebind`, which does
  {
    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    require(!_pool_storage.pools[pool_name].records[token].bound,
            "ERR_IS_BOUND");
    require(!_pool_storage.pools[pool_name].finalized, "ERR_IS_FINALIZED");

    require(_pool_storage.pools[pool_name].tokens.size() < MAX_BOUND_TOKENS,
            "ERR_MAX_TOKENS");

    _pool_storage.pools[pool_name].records[token] = Record({
        true, _pool_storage.pools[pool_name].tokens.size(),
        0, // balance and denorm will be validated
        0  // and set by `rebind`
    });
    _pool_storage.pools[pool_name].tokens.push_back(token);
    rebind(token, balance, denorm);
  }

  void rebind(name token, uint balance, uint denorm) {

    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    require(_pool_storage.pools[pool_name].records[token].bound,
            "ERR_NOT_BOUND");
    require(!_pool_storage.pools[pool_name].finalized, "ERR_IS_FINALIZED");

    require(denorm >= MIN_WEIGHT, "ERR_MIN_WEIGHT");
    require(denorm <= MAX_WEIGHT, "ERR_MAX_WEIGHT");
    print(balance, "==", MIN_BALANCE);
    require(balance >= MIN_BALANCE, "ERR_MIN_BALANCE");

    // Adjust the denorm and totalWeight
    uint oldWeight = _pool_storage.pools[pool_name].records[token].denorm;
    if (denorm > oldWeight) {
      _pool_storage.pools[pool_name].totalWeight =
          BMath::badd(_pool_storage.pools[pool_name].totalWeight,
                      BMath::bsub(denorm, oldWeight));
      require(_pool_storage.pools[pool_name].totalWeight <= MAX_TOTAL_WEIGHT,
              "ERR_MAX_TOTAL_WEIGHT");
    } else if (denorm < oldWeight) {
      _pool_storage.pools[pool_name].totalWeight =
          BMath::bsub(_pool_storage.pools[pool_name].totalWeight,
                      BMath::bsub(oldWeight, denorm));
    }
    _pool_storage.pools[pool_name].records[token].denorm = denorm;

    // Adjust the balance record and actual token balance
    uint oldBalance = _pool_storage.pools[pool_name].records[token].balance;
    _pool_storage.pools[pool_name].records[token].balance = balance;
    if (balance > oldBalance) {
      _pullUnderlying(token, msg_sender, BMath::bsub(balance, oldBalance));
    } else if (balance < oldBalance) {
      // In this case liquidity is being withdrawn, so charge EXIT_FEE
      uint tokenBalanceWithdrawn = BMath::bsub(oldBalance, balance);
      uint tokenExitFee = BMath::bmul(tokenBalanceWithdrawn, EXIT_FEE);
      _pushUnderlying(token, msg_sender,
                      BMath::bsub(tokenBalanceWithdrawn, tokenExitFee));
      _pushUnderlying(token, _pool_storage.pools[pool_name].factory,
                      tokenExitFee);
    }
  }

  void unbind(name token) {

    require(msg_sender == _pool_storage.pools[pool_name].controller,
            "ERR_NOT_CONTROLLER");
    require(_pool_storage.pools[pool_name].records[token].bound,
            "ERR_NOT_BOUND");
    require(!_pool_storage.pools[pool_name].finalized, "ERR_IS_FINALIZED");

    uint tokenBalance = _pool_storage.pools[pool_name].records[token].balance;
    uint tokenExitFee = BMath::bmul(tokenBalance, EXIT_FEE);

    _pool_storage.pools[pool_name].totalWeight =
        BMath::bsub(_pool_storage.pools[pool_name].totalWeight,
                    _pool_storage.pools[pool_name].records[token].denorm);

    // Swap the token-to-unbind with the last token,
    // then delete the last token
    uint index = _pool_storage.pools[pool_name].records[token].index;
    uint last = _pool_storage.pools[pool_name].tokens.size() - 1;
    _pool_storage.pools[pool_name].tokens[index] =
        _pool_storage.pools[pool_name].tokens[last];
    _pool_storage.pools[pool_name]
        .records[_pool_storage.pools[pool_name].tokens[index]]
        .index = index;
    _pool_storage.pools[pool_name].tokens.pop_back();
    _pool_storage.pools[pool_name].records[token] = Record({false, 0, 0, 0});

    _pushUnderlying(token, msg_sender, BMath::bsub(tokenBalance, tokenExitFee));
    _pushUnderlying(token, _pool_storage.pools[pool_name].factory,
                    tokenExitFee);
  }

  // Absorb any tokens that have been sent to this contract into the pool
  void gulp(name token) {
    require(_pool_storage.pools[pool_name].records[token].bound,
            "ERR_NOT_BOUND");
    _pool_storage.pools[pool_name].records[token].balance =
        tokens.balanceOf(msg_sender);
  }

  uint getSpotPrice(name tokenIn, name tokenOut) {
    require(_pool_storage.pools[pool_name].records[tokenIn].bound,
            "ERR_NOT_BOUND");
    require(_pool_storage.pools[pool_name].records[tokenOut].bound,
            "ERR_NOT_BOUND");
    Record inRecord = _pool_storage.pools[pool_name].records[tokenIn];
    Record outRecord = _pool_storage.pools[pool_name].records[tokenOut];
    return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                         outRecord.denorm,
                         _pool_storage.pools[pool_name].swapFee);
  }

  uint getSpotPriceSansFee(name tokenIn, name tokenOut) {
    require(_pool_storage.pools[pool_name].records[tokenIn].bound,
            "ERR_NOT_BOUND");
    require(_pool_storage.pools[pool_name].records[tokenOut].bound,
            "ERR_NOT_BOUND");
    Record inRecord = _pool_storage.pools[pool_name].records[tokenIn];
    Record outRecord = _pool_storage.pools[pool_name].records[tokenOut];
    return calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                         outRecord.denorm, 0);
  }

  void joinPool(uint poolAmountOut, std::vector<uint> maxAmountsIn) {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");

    uint poolTotal = tokens.totalSupply();
    uint ratio = BMath::bdiv(poolAmountOut, poolTotal);
    require(ratio != 0, "ERR_MATH_APPROX");

    for (uint i = 0; i < _pool_storage.pools[pool_name].tokens.size(); i++) {
      name t = _pool_storage.pools[pool_name].tokens[i];
      uint bal = _pool_storage.pools[pool_name].records[t].balance;
      uint tokenAmountIn = BMath::bmul(ratio, bal);

      require(tokenAmountIn != 0, "ERR_MATH_APPROX");
      require(tokenAmountIn <= maxAmountsIn[i], "ERR_LIMIT_IN joinPool");
      _pool_storage.pools[pool_name].records[t].balance = BMath::badd(
          _pool_storage.pools[pool_name].records[t].balance, tokenAmountIn);
      _pullUnderlying(t, msg_sender, tokenAmountIn);
    }
    _mintPoolShare(poolAmountOut);
    _pushPoolShare(msg_sender, poolAmountOut);
  }

  void exitPool(uint poolAmountIn, std::vector<uint> minAmountsOut) {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");

    uint poolTotal = tokens.totalSupply();
    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);
    uint pAiAfterExitFee = BMath::bsub(poolAmountIn, exitFee);
    uint ratio = BMath::bdiv(pAiAfterExitFee, poolTotal);
    require(ratio != 0, "ERR_MATH_APPROX");

    _pullPoolShare(msg_sender, poolAmountIn);
    _pushPoolShare(_pool_storage.pools[pool_name].factory, exitFee);
    _burnPoolShare(pAiAfterExitFee);

    for (uint i = 0; i < _pool_storage.pools[pool_name].tokens.size(); i++) {
      name t = _pool_storage.pools[pool_name].tokens[i];
      uint bal = _pool_storage.pools[pool_name].records[t].balance;
      uint tokenAmountOut = BMath::bmul(ratio, bal);
      require(tokenAmountOut != 0, "ERR_MATH_APPROX");
      require(tokenAmountOut >= minAmountsOut[i], "ERR_LIMIT_OUT");
      _pool_storage.pools[pool_name].records[t].balance = BMath::bsub(
          _pool_storage.pools[pool_name].records[t].balance, tokenAmountOut);
      _pushUnderlying(t, msg_sender, tokenAmountOut);
    }
  }

  std::pair<uint, uint> swapExactAmountIn(name tokenIn, uint tokenAmountIn,
                                          name tokenOut, uint minAmountOut,
                                          uint maxPrice) {

    require(_pool_storage.pools[pool_name].records[tokenIn].bound,
            "ERR_NOT_BOUND");
    require(_pool_storage.pools[pool_name].records[tokenOut].bound,
            "ERR_NOT_BOUND");
    require(_pool_storage.pools[pool_name].publicSwap, "ERR_SWAP_NOT_PUBLIC");

    Record inRecord = _pool_storage.pools[pool_name].records[name(tokenIn)];
    Record outRecord = _pool_storage.pools[pool_name].records[name(tokenOut)];

    require(tokenAmountIn <= BMath::bmul(inRecord.balance, MAX_IN_RATIO),
            "ERR_MAX_IN_RATIO");

    uint spotPriceBefore =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.pools[pool_name].swapFee);
    require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

    uint tokenAmountOut = calcOutGivenIn(
        inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm,
        tokenAmountIn, _pool_storage.pools[pool_name].swapFee);
    require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);
    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint spotPriceAfter =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.pools[pool_name].swapFee);
    require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
    require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
    require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
            "ERR_MATH_APPROX");

    _pullUnderlying(tokenIn, msg_sender, tokenAmountIn);
    _pushUnderlying(tokenOut, msg_sender, tokenAmountOut);

    return std::make_pair(tokenAmountOut, spotPriceAfter);
  }

  std::pair<uint, uint> swapExactAmountOut(name tokenIn, uint maxAmountIn,
                                           name tokenOut, uint tokenAmountOut,
                                           uint maxPrice) {
    require(_pool_storage.pools[pool_name].records[tokenIn].bound,
            "ERR_NOT_BOUND");
    require(_pool_storage.pools[pool_name].records[tokenOut].bound,
            "ERR_NOT_BOUND");
    require(_pool_storage.pools[pool_name].publicSwap, "ERR_SWAP_NOT_PUBLIC");

    Record inRecord = _pool_storage.pools[pool_name].records[name(tokenIn)];
    Record outRecord = _pool_storage.pools[pool_name].records[name(tokenOut)];

    require(tokenAmountOut <= BMath::bmul(outRecord.balance, MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    uint spotPriceBefore =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.pools[pool_name].swapFee);
    require(spotPriceBefore <= maxPrice, "ERR_BAD_LIMIT_PRICE");

    uint tokenAmountIn = calcInGivenOut(
        inRecord.balance, inRecord.denorm, outRecord.balance, outRecord.denorm,
        tokenAmountOut, _pool_storage.pools[pool_name].swapFee);
    require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);
    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint spotPriceAfter =
        calcSpotPrice(inRecord.balance, inRecord.denorm, outRecord.balance,
                      outRecord.denorm, _pool_storage.pools[pool_name].swapFee);
    require(spotPriceAfter >= spotPriceBefore, "ERR_MATH_APPROX");
    require(spotPriceAfter <= maxPrice, "ERR_LIMIT_PRICE");
    require(spotPriceBefore <= BMath::bdiv(tokenAmountIn, tokenAmountOut),
            "ERR_MATH_APPROX");

    _pullUnderlying(tokenIn, msg_sender, tokenAmountIn);
    _pushUnderlying(tokenOut, msg_sender, tokenAmountOut);

    return std::make_pair(tokenAmountIn, spotPriceAfter);
  }

  uint joinswapExternAmountIn(name tokenIn, uint tokenAmountIn,
                              uint minPoolAmountOut) {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.pools[pool_name].records[tokenIn].bound,
            "ERR_NOT_BOUND");
    require(
        tokenAmountIn <=
            BMath::bmul(_pool_storage.pools[pool_name].records[tokenIn].balance,
                        MAX_IN_RATIO),
        "ERR_MAX_IN_RATIO");

    Record inRecord = _pool_storage.pools[pool_name].records[tokenIn];

    uint poolAmountOut = calcPoolOutGivenSingleIn(
        inRecord.balance, inRecord.denorm, tokens.totalSupply(),
        _pool_storage.pools[pool_name].totalWeight, tokenAmountIn,
        _pool_storage.pools[pool_name].swapFee);

    require(poolAmountOut >= minPoolAmountOut, "ERR_LIMIT_OUT");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

    _mintPoolShare(poolAmountOut);
    _pushPoolShare(msg_sender, poolAmountOut);
    _pullUnderlying(tokenIn, msg_sender, tokenAmountIn);

    return poolAmountOut;
  }

  uint joinswapPoolAmountOut(name tokenIn, uint poolAmountOut,
                             uint maxAmountIn) {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.pools[pool_name].records[tokenIn].bound,
            "ERR_NOT_BOUND");

    Record inRecord = _pool_storage.pools[pool_name].records[tokenIn];

    uint tokenAmountIn = calcSingleInGivenPoolOut(
        inRecord.balance, inRecord.denorm, tokens.totalSupply(),
        _pool_storage.pools[pool_name].totalWeight, poolAmountOut,
        _pool_storage.pools[pool_name].swapFee);

    require(tokenAmountIn != 0, "ERR_MATH_APPROX");
    require(tokenAmountIn <= maxAmountIn, "ERR_LIMIT_IN");

    require(
        tokenAmountIn <=
            BMath::bmul(_pool_storage.pools[pool_name].records[tokenIn].balance,
                        MAX_IN_RATIO),
        "ERR_MAX_IN_RATIO");

    inRecord.balance = BMath::badd(inRecord.balance, tokenAmountIn);

    _mintPoolShare(poolAmountOut);
    _pushPoolShare(msg_sender, poolAmountOut);
    _pullUnderlying(tokenIn, msg_sender, tokenAmountIn);

    return tokenAmountIn;
  }

  uint exitswapPoolAmountIn(name tokenOut, uint poolAmountIn,
                            uint minAmountOut) {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.pools[pool_name].records[tokenOut].bound,
            "ERR_NOT_BOUND");

    Record outRecord = _pool_storage.pools[pool_name].records[tokenOut];

    uint tokenAmountOut = calcSingleOutGivenPoolIn(
        outRecord.balance, outRecord.denorm, tokens.totalSupply(),
        _pool_storage.pools[pool_name].totalWeight, poolAmountIn,
        _pool_storage.pools[pool_name].swapFee);

    require(tokenAmountOut >= minAmountOut, "ERR_LIMIT_OUT");

    require(tokenAmountOut <=
                BMath::bmul(
                    _pool_storage.pools[pool_name].records[tokenOut].balance,
                    MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

    _pullPoolShare(msg_sender, poolAmountIn);
    _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
    _pushPoolShare(_pool_storage.pools[pool_name].factory, exitFee);
    _pushUnderlying(tokenOut, msg_sender, tokenAmountOut);

    return tokenAmountOut;
  }

  uint exitswapExternAmountOut(name tokenOut, uint tokenAmountOut,
                               uint maxPoolAmountIn) {
    require(_pool_storage.pools[pool_name].finalized, "ERR_NOT_FINALIZED");
    require(_pool_storage.pools[pool_name].records[tokenOut].bound,
            "ERR_NOT_BOUND");
    require(tokenAmountOut <=
                BMath::bmul(
                    _pool_storage.pools[pool_name].records[tokenOut].balance,
                    MAX_OUT_RATIO),
            "ERR_MAX_OUT_RATIO");

    Record outRecord = _pool_storage.pools[pool_name].records[tokenOut];

    uint poolAmountIn = calcPoolInGivenSingleOut(
        outRecord.balance, outRecord.denorm, tokens.totalSupply(),
        _pool_storage.pools[pool_name].totalWeight, tokenAmountOut,
        _pool_storage.pools[pool_name].swapFee);

    require(poolAmountIn != 0, "ERR_MATH_APPROX");
    require(poolAmountIn <= maxPoolAmountIn, "ERR_LIMIT_IN");

    outRecord.balance = BMath::bsub(outRecord.balance, tokenAmountOut);

    uint exitFee = BMath::bmul(poolAmountIn, EXIT_FEE);

    _pullPoolShare(msg_sender, poolAmountIn);
    _burnPoolShare(BMath::bsub(poolAmountIn, exitFee));
    _pushPoolShare(_pool_storage.pools[pool_name].factory, exitFee);
    _pushUnderlying(tokenOut, msg_sender, tokenAmountOut);

    return poolAmountIn;
  }

  // ==
  // 'Underlying' token-manipulation functions make external calls but are NOT
  // locked You must `_lock_` or otherwise ensure reentry-safety

  void _pullUnderlying(name erc20, name from, uint amount) {
    /// transfer memo implementation
    // bool xfer = tokens.transferFrom(from, from, msg_sender, amount);
    // require(xfer, "ERR_ERC20_FALSE");
  }

  void _pushUnderlying(name erc20, name to, uint amount) {
    bool xfer = tokens.transfer(msg_sender, to, amount);
    require(xfer, "ERR_ERC20_FALSE");
  }

  void _pullPoolShare(name from, uint amount) { tokens._pull(from, amount); }

  void _pushPoolShare(name to, uint amount) { tokens._push(to, amount); }

  void _mintPoolShare(uint amount) { tokens._mint(amount); }

  void _burnPoolShare(uint amount) { tokens._burn(amount); }
};
