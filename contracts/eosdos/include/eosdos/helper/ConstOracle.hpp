/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>



class ConstOracle { 
private:
   OracleStore& stores;

 public:
   ConstOracle(OracleStore& _stores,const extended_asset& _price)
       : stores(_stores)
    {
        stores.tokenPrice = _price;
    }

    uint64_t  getPrice() {
        return stores.tokenPrice.quantity.amount;
    }
};
