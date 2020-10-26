/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
 #include <common/defines.hpp>

class IMinimumOracle {
 public:
   virtual uint256 getPrice() = 0;

   virtual void setPrice(uint256 newPrice) = 0;

   virtual void transferOwnership(address newOwner) = 0;
}

class MinimumOracle {
 public:


   void onlyOwner() { require(getMsgSender() == _OWNER_, "NOT_OWNER"); }

   // ============ Functions ============

   MinimumOracle() { _OWNER_ = getMsgSender(); }

   void transferOwnership(address newOwner) {
      onlyOwner();
      require(newOwner != address(0), "INVALID_OWNER");

      _OWNER_ = newOwner;
   }

   void setPrice(uint256 newPrice) {
      onlyOwner();
      tokenPrice = newPrice;
   }

   uint256 getPrice() { return tokenPrice; }
};
