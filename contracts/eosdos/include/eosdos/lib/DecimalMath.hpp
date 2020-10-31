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

namespace DecimalMath {
    
    uint256  ONE = my_pow(10,4);//18;

    uint256  mul(uint256 target, uint256 d) {
        return SafeMath::mul(target,d) / ONE;
    }

    uint256  mulCeil(uint256 target, uint256 d) {
        return SafeMath::divCeil(SafeMath::mul(target,d),ONE);
    }

    uint256  divFloor(uint256 target, uint256 d) {
        return SafeMath::div(SafeMath::mul(target,ONE),d);
    }

    uint256  divCeil(uint256 target, uint256 d) {
        return SafeMath::divCeil(SafeMath::mul(target,ONE),d);
    }
}
