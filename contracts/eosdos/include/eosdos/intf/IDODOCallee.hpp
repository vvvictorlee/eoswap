/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
 #include <common/defines.hpp>


class IDODOCallee { 
 public:

    virtual void dodoCall(
        bool isBuyBaseToken,
        uint64_t baseAmount,
        uint64_t quoteAmount,
        bytes  data
    ) = 0;
};
