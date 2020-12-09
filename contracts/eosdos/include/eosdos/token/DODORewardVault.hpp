/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>


#include <eosdos/lib/SafeERC20.hpp>
#include <eosdos/intf/IERC20.hpp>


class IDODORewardVault { 
 public:

    virtual void  reward(address to, uint64_t amount) = 0;
}


class DODORewardVault  {
 public:
    

   address dodoToken;

    DODORewardVault(address _dodoToken) {
        dodoToken = _dodoToken;
    }

    void  reward(address to, uint64_t amount) {
        IERC20(dodoToken).safeTransfer(to, amount);
    }
};
