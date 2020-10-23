/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


#include <eodos/lib/Ownable.hpp>
#include <eodos/lib/SafeERC20.hpp>
#include <eodos/intf/IERC20.hpp>


class IDODORewardVault { 
 public:

    virtual void  reward(address to, uint256 amount) = 0;
}


class DODORewardVault : public  Ownable {
 public:
    

   address dodoToken;

    DODORewardVault(address _dodoToken) {
        dodoToken = _dodoToken;
    }

    void  reward(address to, uint256 amount) {
        IERC20(dodoToken).safeTransfer(to, amount);
    }
}
