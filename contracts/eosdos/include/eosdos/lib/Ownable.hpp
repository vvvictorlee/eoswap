/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>
#include <common/storage_mgmt.hpp>
/**
 * @title Ownable
 * @author DODO Breeder
 *
 * @notice Ownership related functions
 */
class Ownable {
 private:
   name          msg_sender;
   OwnableStore& ownable_store;

 public:
   Ownable(OwnableStore& _ownable_store)
       : ownable_store(_ownable_store){
}

       //    address _OWNER_;
       //    address _NEW_OWNER_;
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }
       // ============ Modifiers ============

       void onlyOwner() {
      require(msg_sender == ownable_store._OWNER_, "NOT_OWNER");
   }

   // ============ Functions ============

   void init() { ownable_store._OWNER_ = msg_sender; }

   void transferOwnership(address newOwner) {
      onlyOwner();
      require(newOwner != address(0), "INVALID_OWNER");

      ownable_store._NEW_OWNER_ = newOwner;
   }

   void claimOwnership() {
      require(msg_sender == ownable_store._NEW_OWNER_, "INVALID_CLAIM");

      ownable_store._OWNER_     = ownable_store._NEW_OWNER_;
      ownable_store._NEW_OWNER_ = address(0);
   }
};
