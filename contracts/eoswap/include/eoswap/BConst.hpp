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
#include <eoswap/BColor.hpp>

class BConst : public  BBronze {
public:
    static  constexpr uint  BONE              = my_pow(10,6);

    static constexpr uint  MIN_BOUND_TOKENS  = 2;
    static constexpr uint  MAX_BOUND_TOKENS  = 8;

    static constexpr uint  MIN_FEE           = BONE / my_pow(10,3);//6
    static constexpr uint  MAX_FEE           = BONE / 10;
    static constexpr uint  EXIT_FEE          = 0;

    static constexpr uint  MIN_WEIGHT        = BONE;
    static constexpr uint  MAX_WEIGHT        = BONE * 50;
    static constexpr uint  MAX_TOTAL_WEIGHT  = BONE * 50;
    static constexpr uint  MIN_BALANCE       = BONE / my_pow(10,4);//12

    static constexpr uint  INIT_POOL_SUPPLY  = BONE * 100;

    static constexpr uint  MIN_BPOW_BASE     = 1 ;
    static constexpr uint  MAX_BPOW_BASE     = (2 * BONE) - 1 ;
    static constexpr uint  BPOW_PRECISION    = BONE / my_pow(10,3);//10

    static constexpr uint  MAX_IN_RATIO      = BONE / 2;
    static constexpr uint  MAX_OUT_RATIO     = (BONE / 3) + 1 ;
};
