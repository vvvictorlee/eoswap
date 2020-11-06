/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/lib/Ownable.hpp>

// Oracle only for test
class NaiveOracle : public Ownable {
 private:
   OracleStore& stores;
 public:
   NaiveOracle(OracleStore& _stores)
       : stores(_stores)
       , Ownable(_stores.ownable) {}
   void setPrice(const extended_asset& newPrice) {
      onlyOwner();
      stores.tokenPrice = newPrice;
   }

   uint64_t getPrice() { return stores.tokenPrice.quantity.amount; }
};
