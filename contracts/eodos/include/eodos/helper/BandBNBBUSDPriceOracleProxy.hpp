/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>



class IBandOracleAggregator { 
 public:

    virtual uint256  getReferenceData(string  base, string  quote) = 0;
}


class BandBNBBUSDPriceOracleProxy { 
 public:

    IBandOracleAggregator  aggregator;

    BandBNBBUSDPriceOracleProxy(IBandOracleAggregator _aggregator) {
        aggregator = _aggregator;
    }

    uint256  getPrice() {
        return aggregator.getReferenceData("BNB", "USD");
    }
}
