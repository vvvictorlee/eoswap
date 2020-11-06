/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once



class IDODOLpToken { 
 public:

    virtual void  mint(address user, uint64_t value) = 0;

    virtual uint64_t  burn(address user, uint64_t value) = 0;

    virtual uint64_t  totalSupply() = 0;
};
