/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/intf/IDODO.hpp>
#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/SafeERC20.hpp>
#include <eosdos/lib/SafeMath.hpp>

class IUniswapV2Pair {
 public:
   virtual address token0() = 0;

   virtual std::tuple<uint112, uint112, uint32> getReserves() = 0;

   virtual void swap(uint64_t amount0Out, uint64_t amount1Out, address to, bytes data) = 0;
};
#ifdef  UNISWAPARBITRAGEUR
class UniswapArbitrageur {
 public:
   UniswapArbitrageur(address _uniswap, address _dodo) {
      _UNISWAP_ = _uniswap;
      _DODO_    = _dodo;

      _BASE_  = IDODO(_DODO_)._BASE_TOKEN_();
      _QUOTE_ = IDODO(_DODO_)._QUOTE_TOKEN_();

      address token0 = IUniswapV2Pair(_UNISWAP_).token0();
      address token1 = IUniswapV2Pair(_UNISWAP_).token1();

      if (token0 == _BASE_ && token1 == _QUOTE_) {
         _REVERSE_ = false;
      } else if (token0 == _QUOTE_ && token1 == _BASE_) {
         _REVERSE_ = true;
      } else {
         require(true, "DODO_UNISWAP_NOT_MATCH");
      }

      IERC20(_BASE_).approve(_DODO_, uint64_t(-1));
      IERC20(_QUOTE_).approve(_DODO_, uint64_t(-1));
   }

   uint64_t quoteProfit executeBuyArbitrage(uint64_t baseAmount) {
      IDODO(_DODO_).buyBaseToken(baseAmount, uint64_t(-1), "0xd");
      quoteProfit = IERC20(_QUOTE_).balanceOf(address(this));
      IERC20(_QUOTE_).transfer(getMsgSender(), quoteProfit);
      return quoteProfit;
   }

   uint64_t baseProfit executeSellArbitrage(uint64_t baseAmount) {
      IDODO(_DODO_).sellBaseToken(baseAmount, 0, "0xd");
      baseProfit = IERC20(_BASE_).balanceOf(address(this));
      IERC20(_BASE_).transfer(getMsgSender(), baseProfit);
      return baseProfit;
   }

   void dodoCall(bool isDODOBuy, uint64_t baseAmount, uint64_t quoteAmount, bytes) {
      require(getMsgSender() == _DODO_, "WRONG_DODO");
      if (_REVERSE_) {
         _inverseArbitrage(isDODOBuy, baseAmount, quoteAmount);
      } else {
         _arbitrage(isDODOBuy, baseAmount, quoteAmount);
      }
   }

   void _inverseArbitrage(bool isDODOBuy, uint64_t baseAmount, uint64_t quoteAmount) {
      (uint112 _reserve0, uint112 _reserve1, ) = IUniswapV2Pair(_UNISWAP_).getReserves();
      uint64_t token0Balance                    = uint64_t(_reserve0);
      uint64_t token1Balance                    = uint64_t(_reserve1);
      uint64_t token0Amount;
      uint64_t token1Amount;
      if (isDODOBuy) {
         IERC20(_BASE_).transfer(_UNISWAP_, baseAmount);
         // transfer token1 into uniswap
         uint64_t newToken0Balance = token0Balance.mul(token1Balance).div(token1Balance.add(baseAmount));
         token0Amount             = token0Balance.sub(newToken0Balance).mul(9969).div(10000); // mul 0.9969
         require(token0Amount > quoteAmount, "NOT_PROFITABLE");
         IUniswapV2Pair(_UNISWAP_).swap(token0Amount, token1Amount, address(this), "");
      } else {
         IERC20(_QUOTE_).transfer(_UNISWAP_, quoteAmount);
         // transfer token0 into uniswap
         uint64_t newToken1Balance = token0Balance.mul(token1Balance).div(token0Balance.add(quoteAmount));
         token1Amount             = token1Balance.sub(newToken1Balance).mul(9969).div(10000); // mul 0.9969
         require(token1Amount > baseAmount, "NOT_PROFITABLE");
         IUniswapV2Pair(_UNISWAP_).swap(token0Amount, token1Amount, address(this), "");
      }
   }

   void _arbitrage(bool isDODOBuy, uint64_t baseAmount, uint64_t quoteAmount) {
      (uint112 _reserve0, uint112 _reserve1, ) = IUniswapV2Pair(_UNISWAP_).getReserves();
      uint64_t token0Balance                    = uint64_t(_reserve0);
      uint64_t token1Balance                    = uint64_t(_reserve1);
      uint64_t token0Amount;
      uint64_t token1Amount;
      if (isDODOBuy) {
         IERC20(_BASE_).transfer(_UNISWAP_, baseAmount);
         // transfer token0 into uniswap
         uint64_t newToken1Balance = token1Balance.mul(token0Balance).div(token0Balance.add(baseAmount));
         token1Amount             = token1Balance.sub(newToken1Balance).mul(9969).div(10000); // mul 0.9969
         require(token1Amount > quoteAmount, "NOT_PROFITABLE");
         IUniswapV2Pair(_UNISWAP_).swap(token0Amount, token1Amount, address(this), "");
      } else {
         IERC20(_QUOTE_).transfer(_UNISWAP_, quoteAmount);
         // transfer token1 into uniswap
         uint64_t newToken0Balance = token1Balance.mul(token0Balance).div(token1Balance.add(quoteAmount));
         token0Amount             = token0Balance.sub(newToken0Balance).mul(9969).div(10000); // mul 0.9969
         require(token0Amount > baseAmount, "NOT_PROFITABLE");
         IUniswapV2Pair(_UNISWAP_).swap(token0Amount, token1Amount, address(this), "");
      }
   }

   void retrieve(address token, uint64_t amount) { IERC20(token).safeTransfer(getMsgSender(), amount); }
};
#endif