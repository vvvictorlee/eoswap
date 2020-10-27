/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>


#include <eosdos/lib/SafeMath.hpp>


/**
 * @title DecimalMath
 * @author DODO Breeder
 *
 * @notice Functions for fixed point number with 18 decimals
 */
using namespace SafeMath;
namespace DecimalMath {
    

    uint256  ONE = my_pow(10,6);//18;

    uint256  mul(uint256 target, uint256 d) {
        return mul(target,d) / ONE;
    }

    uint256  mulCeil(uint256 target, uint256 d) {
        return divCeil(mul(target,d),ONE);
    }

    uint256  divFloor(uint256 target, uint256 d) {
        return SafeMath::div(mul(target,ONE),d);
    }

    uint256  divCeil(uint256 target, uint256 d) {
        return divCeil(mul(target,ONE),d);
    }
}
