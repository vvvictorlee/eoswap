/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
 #include <common/defines.hpp>



class ConstOracle { 
 public:

   uint256 tokenPrice;

    ConstOracle(uint256 _price) {
        tokenPrice = _price;
    }

    uint256  getPrice() {
        return tokenPrice;
    }
};
