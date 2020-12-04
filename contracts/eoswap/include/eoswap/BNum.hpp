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
#include <eoswap/BConst.hpp>

class BNum : public BConst {
public:
  uint256m btoi(uint256m a) { return a / BONE; }

  uint256m bfloor(uint256m a) { return btoi(a) * BONE; }

  uint256m badd(uint256m a, uint256m b) {
    uint256m c = a + b;
    require(c >= a, "ERR_ADD_OVERFLOW");
    return c;
  }

  uint256m bsub(uint256m a, uint256m b) {
    uint256m c;
    bool flag;
    std::tie(c, flag) = bsubSign(a, b);
    require(!flag, "ERR_SUB_UNDERFLOW");
    return c;
  }

  std::tuple<uint256m, bool> bsubSign(uint256m a, uint256m b) {
    if (a >= b) {
      return std::make_tuple(a - b, false);
    } else {
      return std::make_tuple(b - a, true);
    }
  }

  uint256m bmul(uint256m a, uint256m b) {
    uint256m c0 = a * b;
    require(a == 0 || c0 / a == b, "ERR_MUL_OVERFLOW");
    uint256m c1 = c0 + (BONE / 2);
    require(c1 >= c0, "ERR_MUL_OVERFLOW");
    uint256m c2 = c1 / BONE;
    return c2;
  }

  uint256m bdiv(uint256m a, uint256m b) {
    require(b != 0, "ERR_DIV_ZERO");
    uint256m c0 = a * BONE;
    require(a == 0 || c0 / a == BONE, "ERR_DIV_INTERNAL"); // bmul overflow
    uint256m c1 = c0 + (b / 2);
    require(c1 >= c0, "ERR_DIV_INTERNAL"); //  badd require
    uint256m c2 = c1 / b;
    return c2;
  }

  // DSMath.wpow
  uint256m bpowi(uint256m a, uint256m n) {
    uint256m z = n % 2 != 0 ? a : BONE;

    for (n /= 2; n != 0; n /= 2) {
      a = bmul(a, a);

      if (n % 2 != 0) {
        z = bmul(z, a);
      }
    }
    return z;
  }

  // Compute b^(e.w) by splitting it into (b^e)*(b^0.w).
  // Use `bpowi` for `b^e` and `bpowK` for k iterations
  // of approximation of b^0.w
  uint256m bpow(uint256m base, uint256m exp) {
    require(base >= MIN_BPOW_BASE, "ERR_BPOW_BASE_TOO_LOW");
    require(base <= MAX_BPOW_BASE, "ERR_BPOW_BASE_TOO_HIGH");

    uint256m whole = bfloor(exp);
    uint256m remain = bsub(exp, whole);

    uint256m wholePow = bpowi(base, btoi(whole));

    if (remain == 0) {
      return wholePow;
    }

    uint256m partialResult = bpowApprox(base, remain, BPOW_PRECISION);
    return bmul(wholePow, partialResult);
  }

  uint256m bpowApprox(uint256m base, uint256m exp, uint256m precision) {
    // term 0:
    uint256m a = exp;
    uint256m x;
    bool xneg;
    std::tie(x, xneg) = bsubSign(base, BONE);
    uint256m term = BONE;
    uint256m sum = term;
    bool negative = false;

    // term(k) = numer / denom
    //         = (product(a - i - 1, i=1-->k) * x^k) / (k!)
    // each iteration, multiply previous term by (a-(k-1)) * x / k
    // continue until term is less than precision
    for (uint256m i = 1; term >= precision; i++) {
      uint256m bigK = i * BONE;
      uint256m c;
      bool cneg;
      std::tie(c, cneg) = bsubSign(a, bsub(bigK, BONE));
      term = bmul(term, bmul(c, x));
      term = bdiv(term, bigK);
      if (term == 0)
        break;

      if (xneg)
        negative = !negative;
      if (cneg)
        negative = !negative;
      if (negative) {
        sum = bsub(sum, term);
      } else {
        sum = badd(sum, term);
      }
    }

    return sum;
  }
};
