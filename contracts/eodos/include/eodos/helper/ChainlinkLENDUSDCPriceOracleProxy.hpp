/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>



class IChainlink { 
 public:

    virtual uint256  latestAnswer() = 0;
}


// for LEND-USDC(decimals=6) price convert

class ChainlinkLENDUSDCPriceOracleProxy { 
 public:

   address chainlink = 0x4aB81192BB75474Cf203B56c36D6a13623270A67;

    uint256  getPrice() {
        return IChainlink(chainlink).latestAnswer() / 100;
    }
}
