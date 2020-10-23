/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once



class IDODOLpToken { 
 public:

    virtual void  mint(address user, uint256 value) = 0;

    virtual uint256  burn(address user, uint256 value) = 0;

    virtual uint256  totalSupply() = 0;
}
