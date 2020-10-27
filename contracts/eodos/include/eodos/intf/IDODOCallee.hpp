/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
#pragma once 
 #include <common/defines.hpp>


class IDODOCallee { 
 public:

    virtual void dodoCall(
        bool isBuyBaseToken,
        uint256 baseAmount,
        uint256 quoteAmount,
        bytes  data
    ) = 0;
};
