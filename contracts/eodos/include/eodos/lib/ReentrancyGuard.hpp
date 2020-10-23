/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


/**
 * @title ReentrancyGuard
 * @author DODO Breeder
 *
 * @notice Protect functions from Reentrancy Attack
 */
class ReentrancyGuard { 
 public:

    // https://solidity.readthedocs.io/en/latest/control-structures.html?highlight=zero-state#scoping-and-declarations
    // zero-state of _ENTERED_ is false
    bool  _ENTERED_;

    void preventReentrant() {
        require(!_ENTERED_, "REENTRANT");
        _ENTERED_ = true;
        
        _ENTERED_ = false;
    }
}
