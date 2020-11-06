/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once 
 #include <common/defines.hpp>



class IWETH { 
 public:

    virtual uint64_t  totalSupply() = 0;

    virtual uint64_t  transfer(address recipient, uint64_t amount) = 0;

    virtual bool  approve(address spender, uint64_t amount) = 0;

    virtual bool transferFrom(
        address src,
        address dst,
        uint64_t wad
    ) = 0;

    virtual void  deposit() = 0;

    virtual void  withdraw(uint64_t wad) = 0;
};
