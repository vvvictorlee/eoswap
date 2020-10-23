/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>



class IChainlink { 
 public:

    virtual uint256  latestAnswer() = 0;
}


// for SNX-USDC(decimals=6) price convert

class ChainlinkSNXUSDCPriceOracleProxy { 
 public:

   address chainlink = 0xDC3EA94CD0AC27d9A86C180091e7f78C683d3699;

    uint256  getPrice() {
        return IChainlink(chainlink).latestAnswer() / 100;
    }
}
