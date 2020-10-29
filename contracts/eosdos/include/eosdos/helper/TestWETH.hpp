/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

class WETH9 {
 private:
   name        msg_sender;
   TokenStore& stores;

 public:
   WETH9(TokenStore& _stores)
       :stores(_stores) {}

   void init(name contract_self, const extended_symbol& esymbol) {
      stores.contract_self = contract_self;
      stores.esymbol       = esymbol;
stores.originToken = esymbol;
   }

   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }
//    void fallback() { deposit(); }

//    void receive() { deposit(); }

   void deposit(uint256 msg_value) { stores.balanceOf[getMsgSender()] += msg_value; }

   void withdraw(uint256 wad) {
      require(stores.balanceOf[getMsgSender()] >= wad,"withdraw amount couldn't be more than  the balance");
      stores.balanceOf[getMsgSender()] -= wad;
      transferFrom(stores.contract_self,getMsgSender(),wad);
   }

   uint256 totalSupply() { return stores.balanceOf[stores.contract_self]; }

   bool approve(address guy, uint256 wad) {
      stores.allowance[getMsgSender()].dst2amt[guy] = wad;

      return true;
   }

   bool transfer(address dst, uint256 wad) { return transferFrom(getMsgSender(), dst, wad); }

   bool transferFrom(address src, address dst, uint256 wad) {
      require(stores.balanceOf[src] >= wad,"The balance is less than the amount to be transfered");

      if (src != getMsgSender() && stores.allowance[src].dst2amt[getMsgSender()] != uint256(-1)) {
         require(stores.allowance[src].dst2amt[getMsgSender()] >= wad,"if (src != getMsgSender() && stores.allowance[src].dst2amt[getMsgSender()] != uint256(-1))");
         stores.allowance[src].dst2amt[getMsgSender()] -= wad;
      }

      stores.balanceOf[src] -= wad;
      stores.balanceOf[dst] += wad;

      //   Transfer(src, dst, wad);

      return true;
   }

   void mint(address account, uint256 amount) { stores.balanceOf[account] = add(stores.balanceOf[account],amount); }


};
