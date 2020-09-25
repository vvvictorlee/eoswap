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
  uint btoi(uint a) { return a / BONE; }

  uint bfloor(uint a) { return btoi(a) * BONE; }

  uint badd(uint a, uint b) {
    uint c = a + b;
    require(c >= a, "ERR_ADD_OVERFLOW");
    return c;
  }

  uint bsub(uint a, uint b) {
    uint c;
    bool flag;
    std::tie(c, flag) = bsubSign(a, b);
    require(!flag, "ERR_SUB_UNDERFLOW");
    return c;
  }

  std::tuple<uint, bool> bsubSign(uint a, uint b) {
    if (a >= b) {
      return std::make_tuple(a - b, false);
    } else {
      return std::make_tuple(b - a, true);
    }
  }

  uint bmul(uint a, uint b) {
    uint c0 = a * b;
    require(a == 0 || c0 / a == b, "ERR_MUL_OVERFLOW");
    uint c1 = c0 + (BONE / 2);
    require(c1 >= c0, "ERR_MUL_OVERFLOW");
    uint c2 = c1 / BONE;
    return c2;
  }

  uint bdiv(uint a, uint b) {
    require(b != 0, "ERR_DIV_ZERO");
    uint c0 = a * BONE;
    require(a == 0 || c0 / a == BONE, "ERR_DIV_INTERNAL"); // bmul overflow
    uint c1 = c0 + (b / 2);
    require(c1 >= c0, "ERR_DIV_INTERNAL"); //  badd require
    uint c2 = c1 / b;
    return c2;
  }

  // DSMath.wpow
  uint bpowi(uint a, uint n) {
    uint z = n % 2 != 0 ? a : BONE;

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
  uint bpow(uint base, uint exp) {
    require(base >= MIN_BPOW_BASE, "ERR_BPOW_BASE_TOO_LOW");
    require(base <= MAX_BPOW_BASE, "ERR_BPOW_BASE_TOO_HIGH");

    uint whole = bfloor(exp);
    uint remain = bsub(exp, whole);

    uint wholePow = bpowi(base, btoi(whole));

    if (remain == 0) {
      return wholePow;
    }

    uint partialResult = bpowApprox(base, remain, BPOW_PRECISION);
    return bmul(wholePow, partialResult);
  }

  uint bpowApprox(uint base, uint exp, uint precision) {
    // term 0:
    uint a = exp;
    uint x;
    bool xneg;
    std::tie(x, xneg) = bsubSign(base, BONE);
    uint term = BONE;
    uint sum = term;
    bool negative = false;

    // term(k) = numer / denom
    //         = (product(a - i - 1, i=1-->k) * x^k) / (k!)
    // each iteration, multiply previous term by (a-(k-1)) * x / k
    // continue until term is less than precision
    for (uint i = 1; term >= precision; i++) {
      uint bigK = i * BONE;
      uint c;
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
