/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
 #include <common/defines.hpp>


#include <eodos/SafeMath.hpp>


/**
 * @title DecimalMath
 * @author DODO Breeder
 *
 * @notice Functions for fixed point number with 18 decimals
 */
namespace DecimalMath {
    

    uint256 constant ONE = 10**18;

    uint256  mul(uint256 target, uint256 d) {
        return target.mul(d) / ONE;
    }

    uint256  mulCeil(uint256 target, uint256 d) {
        return target.mul(d).divCeil(ONE);
    }

    uint256  divFloor(uint256 target, uint256 d) {
        return target.mul(ONE).div(d);
    }

    uint256  divCeil(uint256 target, uint256 d) {
        return target.mul(ONE).divCeil(d);
    }
}
