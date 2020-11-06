/**
 *Submitted for verification at Etherscan.io on 2020-05-05
 */

// File: contracts/interfaces/IUniswapV2Pair.sol
#pragma once
 #include <common/defines.hpp>

#ifdef UNISWAP
class IUniswapV2Pair {
 public:
   virtual string name() = 0;

   virtual uint64_t decimals() = 0;

   virtual uint64_t balanceOf(address owner) = 0;

   virtual bool approve(address spender, uint64_t value) = 0;

   virtual bool transferFrom(address from, address to, uint64_t value) = 0;

   virtual bytes32 DOMAIN_SEPARATOR() = 0;

   virtual uint64_t nonces(address owner) = 0;

   virtual void
   permit(address owner, address spender, uint64_t value, uint64_t deadline, uint8 v, bytes32 r, bytes32 s) = 0;

   virtual uint64_t price0CumulativeLast() = 0;

   virtual uint64_t liquidity kLast() = 0;

   virtual uint64_t amount0, uint64_t amount1 burn(address to) = 0;

   virtual void swap(uint64_t amount0Out, uint64_t amount1Out, address to, bytes data) = 0;

   virtual void skim(address to) = 0;

   virtual void sync() = 0;

   virtual void initialize(address, address) = 0;
};

    // File: contracts/interfaces/IUniswapV2ERC20.sol


class IUniswapV2ERC20 {
 public:
   virtual string name() = 0;

   virtual uint64_t decimals() = 0;

   virtual uint64_t balanceOf(address owner) = 0;

   virtual bool approve(address spender, uint64_t value) = 0;

   virtual bool transferFrom(address from, address to, uint64_t value) = 0;

   virtual bytes32 DOMAIN_SEPARATOR() = 0;

   virtual uint64_t nonces(address owner) = 0;

   virtual void
   permit(address owner, address spender, uint64_t value, uint64_t deadline, uint8 v, bytes32 r, bytes32 s) = 0;
};

// File: contracts/libraries/SafeMath.sol


// a library for performing overflow-safe math, courtesy of DappHub (https://github.com/dapphub/ds-math)

namespace SafeMath {
   uint64_t  add(uint64_t x, uint64_t y) { require((z = x + y) >= x, "ds-math-add-overflow"); }

   uint64_t  sub(uint64_t x, uint64_t y) { require((z = x - y) <= x, "ds-math-sub-underflow"); }

   uint64_t  mul(uint64_t x, uint64_t y) { require(y == 0 || (z = x * y) / y == x, "ds-math-mul-overflow"); }
}

// File: contracts/UniswapV2ERC20.sol


class UniswapV2ERC20 {
 public:
   // keccak256("Permit(address owner,address spender,uint64_t value,uint64_t nonce,uint64_t deadline)");
   bytes32 PERMIT_TYPEHASH = 0x6e71edae12b1b97f4d1f60370fef10105fa2faae0126114a169c64845d6126c9;

   void _mint(address to, uint64_t value) {
      totalSupply   = totalSupply.add(value);
      balanceOf[to] = balanceOf[to].add(value);
   }

   void _burn(address from, uint64_t value) {
      balanceOf[from] = balanceOf[from].sub(value);
      totalSupply     = totalSupply.sub(value);
   }

   void _approve(address owner, address spender, uint64_t value) { allowance[owner][spender] = value; }

   void _transfer(address from, address to, uint64_t value) {
      balanceOf[from] = balanceOf[from].sub(value);
      balanceOf[to]   = balanceOf[to].add(value);
   }

   bool approve(address spender, uint64_t value) {
      _approve(getMsgSender(), spender, value);
      return true;
   }

   bool transfer(address to, uint64_t value) {
      _transfer(getMsgSender(), to, value);
      return true;
   }

   bool transferFrom(address from, address to, uint64_t value) {
      if (allowance[from][getMsgSender()] != uint64_t(-1)) {
         allowance[from][getMsgSender()] = allowance[from][getMsgSender()].sub(value);
      }
      _transfer(from, to, value);
      return true;
   }

   void permit(address owner, address spender, uint64_t value, uint64_t deadline, uint8 v, bytes32 r, bytes32 s) {
      require(deadline >= block.timestamp, "UniswapV2: EXPIRED");
      bytes32 digest           = keccak256(abi.encodePacked(
          "\x19\x01", DOMAIN_SEPARATOR,
          keccak256(abi.encode(PERMIT_TYPEHASH, owner, spender, value, nonces[owner]++, deadline))));
      address recoveredAddress = ecrecover(digest, v, r, s);
      require(recoveredAddress != address(0) && recoveredAddress == owner, "UniswapV2: INVALID_SIGNATURE");
      _approve(owner, spender, value);
   }
};

// File: contracts/libraries/Math.sol


// a library for performing various math operations

namespace Math {
   uint64_t z min(uint64_t x, uint64_t y) { z = x < y ? x : y; }

   // babylonian method (https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Babylonian_method)
   uint64_t z sqrt(uint64_t y) {
      if (y > 3) {
         z         = y;
         uint64_t x = y / 2 + 1;
         while (x < z) {
            z = x;
            x = (y / x + x) / 2;
         }
      } else if (y != 0) {
         z = 1;
      }
   }
}

// File: contracts/libraries/UQ112x112.sol


// a library for handling binary fixed point numbers (https://en.wikipedia.org/wiki/Q_(number_format))

// range: [0, 2**112 - 1]
// resolution: 1 / 2**112

namespace UQ112x112 {
uint224 constant Q112 = 2 * *112;

// encode a uint112 as a UQ112x112
uint224 z encode(uint112 y) {
   z = uint224(y) * Q112; // never overflows
}

// divide a UQ112x112 by a uint112, returning a UQ112x112
uint224 z uqdiv(uint224 x, uint112 y) { z = x / uint224(y); }
} // namespace UQ112x112

// File: contracts/interfaces/IERC20.sol

class IERC20 {
 public:
   virtual string name() = 0;

   virtual uint64_t decimals() = 0;

   virtual uint64_t balanceOf(address owner) = 0;

   virtual bool approve(address spender, uint64_t value) = 0;

   virtual bool transferFrom(address from, address to, uint64_t value) = 0;
};

// File: contracts/interfaces/IUniswapV2Factory.sol

class IUniswapV2Factory {
 public:
   void lock() {
      require(unlocked == 1, "UniswapV2: LOCKED");
      unlocked = 0;

      unlocked = 1;
   }

   std::tuple<uint112, uint112, uint32> getReserves() {
      uint112 _reserve0;
      uint112 _reserve1;
      uint32  _blockTimestampLast;

      _reserve0           = reserve0;
      _reserve1           = reserve1;
      _blockTimestampLast = blockTimestampLast;
   }

   void _safeTransfer(address token, address to, uint64_t value) {
      (bool success, bytes data) = token.call(abi.encodeWithSelector(SELECTOR, to, value));
      require(success && (data.length == 0 || abi.decode(data, (bool))), "UniswapV2: TRANSFER_FAILED");
   }

   IUniswapV2Factory() { factory = getMsgSender(); }

   // called once by the factory at time of deployment
   void initialize(address _token0, address _token1) {
      require(getMsgSender() == factory, "UniswapV2: FORBIDDEN"); // sufficient check
      token0 = _token0;
      token1 = _token1;
   }

   // update reserves and, on the first call per block, price accumulators
   void _update(uint64_t balance0, uint64_t balance1, uint112 _reserve0, uint112 _reserve1) {
      require(balance0 <= uint112(-1) && balance1 <= uint112(-1), "UniswapV2: OVERFLOW");
      uint32 blockTimestamp = uint32(block.timestamp % 2 * *32);
      uint32 timeElapsed    = blockTimestamp - blockTimestampLast; // overflow is desired
      if (timeElapsed > 0 && _reserve0 != 0 && _reserve1 != 0) {
         // * never overflows, and + overflow is desired
         price0CumulativeLast += uint64_t(UQ112x112.encode(_reserve1).uqdiv(_reserve0)) * timeElapsed;
         price1CumulativeLast += uint64_t(UQ112x112.encode(_reserve0).uqdiv(_reserve1)) * timeElapsed;
      }
      reserve0           = uint112(balance0);
      reserve1           = uint112(balance1);
      blockTimestampLast = blockTimestamp;
   }

   // if fee is on, mint liquidity equivalent to 1/6th of the growth in sqrt(k)
   bool _mintFee(uint112 _reserve0, uint112 _reserve1) {
      address feeTo  = IUniswapV2Factory(factory).feeTo();
      bool    feeOn  = feeTo != address(0);
      uint64_t _kLast = kLast; // gas savings
      if (feeOn) {
         if (_kLast != 0) {
            uint64_t rootK     = Math.sqrt(uint64_t(_reserve0).mul(_reserve1));
            uint64_t rootKLast = Math.sqrt(_kLast);
            if (rootK > rootKLast) {
               uint64_t numerator   = totalSupply.mul(rootK.sub(rootKLast));
               uint64_t denominator = rootK.mul(5).add(rootKLast);
               uint64_t liquidity   = numerator / denominator;
               if (liquidity > 0)
                  _mint(feeTo, liquidity);
            }
         }
      } else if (_kLast != 0) {
         kLast = 0;
      }

      return feeOn;
   }

   // this low-level function should be called from a contract which performs important safety checks
   uint64_t liquidity mint(address to) {
      uint64_t liquidity;
      (uint112 _reserve0, uint112 _reserve1, ) = getReserves(); // gas savings
      uint64_t balance0                         = IERC20(token0).balanceOf(address(this));
      uint64_t balance1                         = IERC20(token1).balanceOf(address(this));
      uint64_t amount0                          = balance0.sub(_reserve0);
      uint64_t amount1                          = balance1.sub(_reserve1);

      bool    feeOn        = _mintFee(_reserve0, _reserve1);
      uint64_t _totalSupply = totalSupply; // gas savings, must be defined here since totalSupply can update in _mintFee
      if (_totalSupply == 0) {
         liquidity = Math.sqrt(amount0.mul(amount1)).sub(MINIMUM_LIQUIDITY);
         _mint(address(0), MINIMUM_LIQUIDITY); // permanently lock the first MINIMUM_LIQUIDITY tokens
      } else {
         liquidity = Math.min(amount0.mul(_totalSupply) / _reserve0, amount1.mul(_totalSupply) / _reserve1);
      }
      require(liquidity > 0, "UniswapV2: INSUFFICIENT_LIQUIDITY_MINTED");
      _mint(to, liquidity);

      _update(balance0, balance1, _reserve0, _reserve1);
      if (feeOn)
         kLast = uint64_t(reserve0).mul(reserve1); // reserve0 and reserve1 are up-to-date
      return liquidity;
   }

   // this low-level function should be called from a contract which performs important safety checks
   std::tuple<uint64_t, uint64_t> burn(address to) {
      uint64_t amount0;
      uint64_t amount1;
      (uint112 _reserve0, uint112 _reserve1, ) = getReserves(); // gas savings
      address _token0                          = token0;        // gas savings
      address _token1                          = token1;        // gas savings
      uint64_t balance0                         = IERC20(_token0).balanceOf(address(this));
      uint64_t balance1                         = IERC20(_token1).balanceOf(address(this));
      uint64_t liquidity                        = balanceOf[address(this)];

      bool    feeOn        = _mintFee(_reserve0, _reserve1);
      uint64_t _totalSupply = totalSupply; // gas savings, must be defined here since totalSupply can update in _mintFee
      amount0              = liquidity.mul(balance0) / _totalSupply; // using balances ensures pro-rata distribution
      amount1              = liquidity.mul(balance1) / _totalSupply; // using balances ensures pro-rata distribution
      require(amount0 > 0 && amount1 > 0, "UniswapV2: INSUFFICIENT_LIQUIDITY_BURNED");
      _burn(address(this), liquidity);
      _safeTransfer(_token0, to, amount0);
      _safeTransfer(_token1, to, amount1);
      balance0 = IERC20(_token0).balanceOf(address(this));
      balance1 = IERC20(_token1).balanceOf(address(this));

      _update(balance0, balance1, _reserve0, _reserve1);
      if (feeOn)
         kLast = uint64_t(reserve0).mul(reserve1); // reserve0 and reserve1 are up-to-date
      return std::make_tuple(amount0, amount1);
   }

   // this low-level function should be called from a contract which performs important safety checks
   void swap(uint64_t amount0Out, uint64_t amount1Out, address to, bytes data) {
      lock();
      require(amount0Out > 0 || amount1Out > 0, "UniswapV2: INSUFFICIENT_OUTPUT_AMOUNT");
      (uint112 _reserve0, uint112 _reserve1, ) = getReserves(); // gas savings
      require(amount0Out < _reserve0 && amount1Out < _reserve1, "UniswapV2: INSUFFICIENT_LIQUIDITY");

      uint64_t balance0;
      uint64_t balance1;
      {
         // scope for _token{0,1}, avoids stack too deep errors
         address _token0 = token0;
         address _token1 = token1;
         require(to != _token0 && to != _token1, "UniswapV2: INVALID_TO");
         if (amount0Out > 0)
            _safeTransfer(_token0, to, amount0Out); // optimistically transfer tokens
         if (amount1Out > 0)
            _safeTransfer(_token1, to, amount1Out); // optimistically transfer tokens
         if (data.length > 0)
            IUniswapV2Callee(to).uniswapV2Call(getMsgSender(), amount0Out, amount1Out, data);
         balance0 = IERC20(_token0).balanceOf(address(this));
         balance1 = IERC20(_token1).balanceOf(address(this));
      }
      uint64_t amount0In = balance0 > _reserve0 - amount0Out ? balance0 - (_reserve0 - amount0Out) : 0;
      uint64_t amount1In = balance1 > _reserve1 - amount1Out ? balance1 - (_reserve1 - amount1Out) : 0;
      require(amount0In > 0 || amount1In > 0, "UniswapV2: INSUFFICIENT_INPUT_AMOUNT");
      {
         // scope for reserve{0,1}Adjusted, avoids stack too deep errors
         uint64_t balance0Adjusted = balance0.mul(1000).sub(amount0In.mul(3));
         uint64_t balance1Adjusted = balance1.mul(1000).sub(amount1In.mul(3));
         require(
             balance0Adjusted.mul(balance1Adjusted) >= uint64_t(_reserve0).mul(_reserve1).mul(1000 * *2),
             "UniswapV2: K");
      }

      _update(balance0, balance1, _reserve0, _reserve1);
   }

   // force balances to match reserves
   void skim(address to) {
      lock();
      address _token0 = token0; // gas savings
      address _token1 = token1; // gas savings
      _safeTransfer(_token0, to, IERC20(_token0).balanceOf(address(this)).sub(reserve0));
      _safeTransfer(_token1, to, IERC20(_token1).balanceOf(address(this)).sub(reserve1));
   }

   // force reserves to match balances
   void sync() {
      lock();
      _update(IERC20(token0).balanceOf(address(this)), IERC20(token1).balanceOf(address(this)), reserve0, reserve1);
   }
};
#endif