/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
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
        balanceOf[getMsgSender()] += msg.value;
        
    }

    void  withdraw(uint256 wad) {
        require(balanceOf[getMsgSender()] >= wad);
        balanceOf[getMsgSender()] -= wad;
        getMsgSender().transfer(wad);
        
    }

    uint256  totalSupply() {
        return address(this).balance;
    }

    bool  approve(address guy, uint256 wad) {
        allowance[getMsgSender()][guy] = wad;
        
        return true;
    }

    bool  transfer(address dst, uint256 wad) {
        return transferFrom(getMsgSender(), dst, wad);
    }

    bool transferFrom(
        address src,
        address dst,
        uint256 wad
    ) {
        require(balanceOf[src] >= wad);

        if (src != getMsgSender() && allowance[src][getMsgSender()] != uint256(-1)) {
            require(allowance[src][getMsgSender()] >= wad);
            allowance[src][getMsgSender()] -= wad;
        }

        balanceOf[src] -= wad;
        balanceOf[dst] += wad;

        Transfer(src, dst, wad);

        return true;
    }
};
