/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
 #include <common/defines.hpp>



class IDODO { 
 public:

    virtual void init(
        address owner,
        address supervisor,
        address maintainer,
        address baseToken,
        address quoteToken,
        address oracle,
        uint256 lpFeeRate,
        uint256 mtFeeRate,
        uint256 k,
        uint256 gasPriceLimit
    ) = 0;

    virtual void  transferOwnership(address newOwner) = 0;

    virtual void  claimOwnership() = 0;

    virtual uint256 sellBaseToken(
        uint256 amount,
        uint256 minReceiveQuote,
        bytes  data
    ) = 0;

    virtual uint256 buyBaseToken(
        uint256 amount,
        uint256 maxPayQuote,
        bytes  data
    ) = 0;

    virtual uint256  querySellBaseToken(uint256 amount) = 0;

    virtual uint256  getExpectedTarget() = 0;

    virtual uint256  withdrawBase(uint256 amount) = 0;

    virtual uint256  depositQuoteTo(address to, uint256 amount) = 0;

    virtual address  withdrawAllQuote() = 0;

    virtual address  _QUOTE_CAPITAL_TOKEN_() = 0;

    virtual address  _QUOTE_TOKEN_() = 0;
};
