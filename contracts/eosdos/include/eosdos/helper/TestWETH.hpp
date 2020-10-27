/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

class WETH9 {
 private:
   name         msg_sender;
   TokenStore& stores;

 public:
   TestERC20(TokenStore& _stores)
       : stores(_stores) {}
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }
   void fallback() { deposit(); }

   void receive() { deposit(); }

   void deposit(uint256 msg_value) { stores.balanceOf[getMsgSender()] += msg_value; }

   void withdraw(uint256 wad) {
      require(stores.balanceOf[getMsgSender()] >= wad);
      stores.balanceOf[getMsgSender()] -= wad;
      getMsgSender().transfer(wad);
   }

   uint256 totalSupply() { return address(this).balance; }

   bool approve(address guy, uint256 wad) {
      stores.allowance[getMsgSender()][guy] = wad;

      return true;
   }

   bool transfer(address dst, uint256 wad) { return transferFrom(getMsgSender(), dst, wad); }

   bool transferFrom(address src, address dst, uint256 wad) {
      require(stores.balanceOf[src] >= wad);

      if (src != getMsgSender() && stores.allowance[src][getMsgSender()] != uint256(-1)) {
         require(stores.allowance[src][getMsgSender()] >= wad);
         stores.allowance[src][getMsgSender()] -= wad;
      }

      stores.balanceOf[src] -= wad;
      stores.balanceOf[dst] += wad;

      Transfer(src, dst, wad);

      return true;
   }
};
