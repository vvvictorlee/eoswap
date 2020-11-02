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
       : stores(_stores) {}

   void init(const extended_asset& token) {
      //   stores.decimals = _decimals;
      stores.esymbol = token.get_extended_symbol();
      //   stores.originToken = esymbol;
      transfer_mgmt::static_create(msg_sender, token);
   }

   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }
   //    void fallback() { deposit(); }

   //    void receive() { deposit(); }

   uint256 balanceOf(address owner) {
      // return stores.balanceOf[owner];
      return transfer_mgmt::get_balance(owner, stores.esymbol);
   }

   uint256 allowance(address owner, address spender) {
      return uint256(-1);
      // return stores.allowance[owner].dst2amt[spender];
   }

   void deposit(uint256 msg_value) {
      // stores.balanceOf[getMsgSender()] += msg_value;
      mint(getMsgSender(), msg_value);
   }

   void withdraw(uint256 wad) {
      require(balanceOf(getMsgSender()) >= wad, "withdraw amount couldn't be more than  the balance");
      //   stores.balanceOf[getMsgSender()] -= wad;
      burn(getMsgSender(), wad);
   }

   uint256 totalSupply() {
      return balanceOf(stores.esymbol.get_contract());
      // return stores.balanceOf[stores.esymbol.get_contract()];
   }

   bool approve(address guy, uint256 wad) {
      //   stores.allowance[getMsgSender()].dst2amt[guy] = wad;

      return true;
   }

   bool transfer(address dst, uint256 wad) { return transferFrom(getMsgSender(), dst, wad); }

   bool transferFrom(address src, address dst, uint256 wad) {
      require(balanceOf(src) >= wad, "The balance is less than the amount to be transfered");

      //   if (src != getMsgSender() && stores.allowance[src].dst2amt[getMsgSender()] != uint256(-1)) {
      //      require(
      //          stores.allowance[src].dst2amt[getMsgSender()] >= wad,
      //          "(src != getMsgSender() && stores.allowance[src].dst2amt[getMsgSender()] != uint256(-1))");
      //      stores.allowance[src].dst2amt[getMsgSender()] -= wad;
      //   }

      //   stores.balanceOf[src] -= wad;
      //   stores.balanceOf[dst] += wad;

      //   Transfer(src, dst, wad);
      transfer_mgmt::static_transfer(src, dst, extended_asset(wad, stores.esymbol));

      return true;
   }

   void mint(address account, uint256 amount) {
      transfer_mgmt::static_issue(account, extended_asset(amount, stores.esymbol));
      // stores.balanceOf[account] = add(stores.balanceOf[account], amount);
   }
   void burn(address account, uint256 amount) {
      transfer_mgmt::static_burn(account, extended_asset(amount, stores.esymbol));
      // stores.balanceOf[account] = add(stores.balanceOf[account], amount);
   }
};
