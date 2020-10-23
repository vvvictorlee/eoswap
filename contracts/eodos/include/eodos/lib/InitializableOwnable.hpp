/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

/**
 * @title Ownable
 * @author DODO Breeder
 *
 * @notice Ownership related functions
 */
class InitializableOwnable {
 public:

   // ============ Modifiers ============

   void onlyOwner() { require(msg.sender == _OWNER_, "NOT_OWNER"); }

   // ============ Functions ============

   void transferOwnership(address newOwner) {
      onlyOwner();
      require(newOwner != address(0), "INVALID_OWNER");

      _NEW_OWNER_ = newOwner;
   }

   void claimOwnership() {
      require(msg.sender == _NEW_OWNER_, "INVALID_CLAIM");

      _OWNER_     = _NEW_OWNER_;
      _NEW_OWNER_ = address(0);
   }
}
