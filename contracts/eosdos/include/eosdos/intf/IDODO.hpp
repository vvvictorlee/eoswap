/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
 #include <common/defines.hpp>



class IDODO { 
 public:

    virtual void init(name dodo_name,
        address owner,
        address supervisor,
        address maintainer,
        address baseToken,
        address quoteToken,
        address oracle,
        uint64_t lpFeeRate,
        uint64_t mtFeeRate,
        uint64_t k,
        uint64_t gasPriceLimit
    ) = 0;

    virtual void  transferOwnership(address newOwner) = 0;

    virtual void  claimOwnership() = 0;

    virtual uint64_t sellBaseToken(
        uint64_t amount,
        uint64_t minReceiveQuote,
        bytes  data
    ) = 0;

    virtual uint64_t buyBaseToken(
        uint64_t amount,
        uint64_t maxPayQuote,
        bytes  data
    ) = 0;

    virtual uint64_t  querySellBaseToken(uint64_t amount) = 0;

    virtual uint64_t  getExpectedTarget() = 0;

    virtual uint64_t  withdrawBase(uint64_t amount) = 0;

    virtual uint64_t  depositQuoteTo(address to, uint64_t amount) = 0;

    virtual address  withdrawAllQuote() = 0;

    virtual address  _QUOTE_CAPITAL_TOKEN_() = 0;

    virtual address  _QUOTE_TOKEN_() = 0;
};
