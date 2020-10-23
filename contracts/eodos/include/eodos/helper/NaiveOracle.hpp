/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


#include <eodos/lib/Ownable.hpp>


// Oracle only for test
class NaiveOracle : public  Ownable {
   uint256 tokenPrice;

    void  setPrice(uint256 newPrice) {
        tokenPrice = newPrice;
    }

    uint256  getPrice() {
        return tokenPrice;
    }
}
