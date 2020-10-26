/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
 #include <common/defines.hpp>



class IChainlink { 
 public:

    virtual uint256  latestAnswer() = 0;
}


// for WETH-USDC(decimals=6) price convert

class ChainlinkETHPriceOracleProxy { 
 public:

   address chainlink ;//= 0x5f4eC3Df9cbd43714FE2740f5E3616155c5b8419;

    uint256  getPrice() {
        return IChainlink(chainlink).latestAnswer() / 100;
    }
};
