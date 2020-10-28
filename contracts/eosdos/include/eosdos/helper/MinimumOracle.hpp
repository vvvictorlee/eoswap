/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

class IMinimumOracle {
 public:
   virtual uint256 getPrice() = 0;

   virtual void setPrice(uint256 newPrice) = 0;

   virtual void transferOwnership(address newOwner) = 0;
};

class MinimumOracle {
 private:
   name         msg_sender;
   OracleStore& stores;

 public:
   MinimumOracle(OracleStore& _stores)
       : stores(_stores) {}

   // ============ Functions ============
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) { require_auth(_msg_sender);
msg_sender = _msg_sender; }
   // ============ Modifiers ============

   void onlyOwner() { require(msg_sender == stores._OWNER_, "NOT_OWNER"); }

   // ============ Functions ============

   void init() { stores._OWNER_ = msg_sender; }

   void transferOwnership(address newOwner) {
      onlyOwner();
      require(newOwner != address(0), "INVALID_OWNER");

      stores._OWNER_ = newOwner;
   }

   void setPrice(const extended_asset& newPrice) {
      onlyOwner();
      stores.tokenPrice = newPrice;
   }

   uint256 getPrice() { return stores.tokenPrice.quantity.amount; }
};
