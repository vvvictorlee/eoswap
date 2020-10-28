/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

/**
 * @title Ownable
 * @author DODO Breeder
 *
 * @notice Ownership related functions
 */
class InitializableOwnable {
 private:
   name          msg_sender;
   OwnableStore& ownable_store;

 public:
   InitializableOwnable(OwnableStore& _ownable_store)
       : ownable_store(_ownable_store) {
   }

   // ============ Modifiers ============
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) {
      require_auth(_msg_sender);
      msg_sender = _msg_sender;
   }
   void onlyOwner() { require(getMsgSender() == ownable_store._OWNER_, "NOT_OWNER"); }

   // ============ Functions ============

   void transferOwnership(address newOwner) {
      onlyOwner();
      require(newOwner != address(0), "INVALID_OWNER");

      ownable_store._NEW_OWNER_ = newOwner;
   }

   void claimOwnership() {
      require(getMsgSender() == ownable_store._NEW_OWNER_, "INVALID_CLAIM");

      ownable_store._OWNER_     = ownable_store._NEW_OWNER_;
      ownable_store._NEW_OWNER_ = address(0);
   }
};
