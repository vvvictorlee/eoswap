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


// for COMP-USDC(decimals=6) price convert

class ChainlinkCOMPUSDCPriceOracleProxy { 
 public:

   address chainlink;// = 0xdbd020CAeF83eFd542f4De03e3cF0C28A4428bd5;

    uint256  getPrice() {
        return IChainlink(chainlink).latestAnswer() / 100;
    }
};
