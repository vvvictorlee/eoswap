/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
#prama once 
 #include <common/defines.hpp>



class IWETH { 
 public:

    virtual uint256  totalSupply() = 0;

    virtual uint256  transfer(address recipient, uint256 amount) = 0;

    virtual bool  approve(address spender, uint256 amount) = 0;

    virtual bool transferFrom(
        address src,
        address dst,
        uint256 wad
    ) = 0;

    virtual void  deposit() = 0;

    virtual void  withdraw(uint256 wad) = 0;
};
