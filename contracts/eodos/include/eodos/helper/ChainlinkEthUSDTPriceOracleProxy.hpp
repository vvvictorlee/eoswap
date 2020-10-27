/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>



class IChainlink { 
 public:

    virtual uint256  latestAnswer() = 0;
}


// for WETH-USDT(decimals=6) price convert

class ChainlinkETHUSDTPriceOracleProxy { 
 public:

   address chainlink;//= 0xEe9F2375b4bdF6387aa8265dD4FB8F16512A1d46;

    uint256  getPrice() {
        return 10**24 / IChainlink(chainlink).latestAnswer();
    }
};
