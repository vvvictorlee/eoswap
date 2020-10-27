/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>


#include <eodos/lib/SafeMath.hpp>


class IChainlink { 
 public:

    virtual uint256  latestAnswer() = 0;
}


// for YFI-USDC(decimals=6) price convert

class ChainlinkYFIUSDCPriceOracleProxy { 
 public:
   address yfiEth;// = 0x7c5d4F8345e66f68099581Db340cd65B078C41f4;
   address EthUsd;// = 0x5f4eC3Df9cbd43714FE2740f5E3616155c5b8419;

    uint256  getPrice() {
        uint256 yfiEthPrice = IChainlink(yfiEth).latestAnswer();
        uint256 EthUsdPrice = IChainlink(EthUsd).latestAnswer();
        return yfiEthPrice.mul(EthUsdPrice).div(10**20);
    }
};
