/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
 #include <common/defines.hpp>


/**
 * @title ReentrancyGuard
 * @author DODO Breeder
 *
 * @notice Protect functions from Reentrancy Attack
 */
class ReentrancyGuard { 
 private:
   ReentrancyGuardStore& reentrancy_guard_store;

 public:
   ReentrancyGuard(ReentrancyGuardStore& _reentrancy_guard_store)
       : reentrancy_guard_store(_reentrancy_guard_store)

    // https://solidity.readthedocs.io/en/latest/control-structures.html?highlight=zero-state#scoping-and-declarations
    // zero-state of _ENTERED_ is false
    // bool  _ENTERED_;

    void preventReentrant() {
        require(!reentrancy_guard_store._ENTERED_, "REENTRANT");
        reentrancy_guard_store._ENTERED_ = true;
        
        reentrancy_guard_store._ENTERED_ = false;
    }
};
