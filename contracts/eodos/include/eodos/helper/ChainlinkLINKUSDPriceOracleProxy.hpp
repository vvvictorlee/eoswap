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


// for LINK-USDC(decimals=6) price convert

class ChainlinkLINKUSDCPriceOracleProxy { 
 public:

   address chainlink;// = 0x2c1d072e956AFFC0D435Cb7AC38EF18d24d9127c;

    uint256  getPrice() {
        return IChainlink(chainlink).latestAnswer() / 100;
    }
};
