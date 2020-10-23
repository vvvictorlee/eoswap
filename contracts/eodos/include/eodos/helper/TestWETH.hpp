/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


class WETH9 { 
 public:

 
   void fallback()  {
        deposit();
    }

    void receive()  {
        deposit();
    }

    void  deposit() {
        balanceOf[msg.sender] += msg.value;
        
    }

    void  withdraw(uint256 wad) {
        require(balanceOf[msg.sender] >= wad);
        balanceOf[msg.sender] -= wad;
        msg.sender.transfer(wad);
        
    }

    uint256  totalSupply() {
        return address(this).balance;
    }

    bool  approve(address guy, uint256 wad) {
        allowance[msg.sender][guy] = wad;
        
        return true;
    }

    bool  transfer(address dst, uint256 wad) {
        return transferFrom(msg.sender, dst, wad);
    }

    bool transferFrom(
        address src,
        address dst,
        uint256 wad
    ) {
        require(balanceOf[src] >= wad);

        if (src != msg.sender && allowance[src][msg.sender] != uint256(-1)) {
            require(allowance[src][msg.sender] >= wad);
            allowance[src][msg.sender] -= wad;
        }

        balanceOf[src] -= wad;
        balanceOf[dst] += wad;

        Transfer(src, dst, wad);

        return true;
    }
}
