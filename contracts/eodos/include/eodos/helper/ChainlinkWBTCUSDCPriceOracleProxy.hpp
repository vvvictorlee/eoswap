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


// for WBTC(decimals=8)-USDC(decimals=6) price convert

class ChainlinkWBTCUSDCPriceOracleProxy { 
 public:

   address chainlink;// = 0xF4030086522a5bEEa4988F8cA5B36dbC97BeE88c;

    uint256  getPrice() {
        return IChainlink(chainlink).latestAnswer() * (10**8);
    }
};
