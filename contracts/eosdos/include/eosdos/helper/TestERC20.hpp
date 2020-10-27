/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/lib/SafeMath.hpp>

class TestERC20 {
 private:
   name         msg_sender;
   TokenStore& stores;

 public:
   TestERC20(TokenStore& _stores)
       : stores(_stores){}
   void init(string _name, uint8 _decimals) {
      stores.names     = _name;
      stores.decimals = _decimals;
   }
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }
   bool transfer(address to, uint256 amount) {
      require(to != address(0), "TO_ADDRESS_IS_EMPTY");
      require(amount <= stores.balances[getMsgSender()], "BALANCE_NOT_ENOUGH");

      stores.balances[getMsgSender()] = stores.balances[getMsgSender()].sub(amount);
      stores.balances[to]             = stores.balances[to].add(amount);

      return true;
   }

   uint256 balance balanceOf(address owner) { return stores.balances[owner]; }

   bool transferFrom(address from, address to, uint256 amount) {
      require(to != address(0), "TO_ADDRESS_IS_EMPTY");
      require(amount <= stores.balances[from], "BALANCE_NOT_ENOUGH");
      require(amount <= stores.allowed[from][getMsgSender()], "ALLOWANCE_NOT_ENOUGH");

      stores.balances[from]                = stores.balances[from].sub(amount);
      stores.balances[to]                  = stores.balances[to].add(amount);
      stores.allowed[from][getMsgSender()] = stores.allowed[from][getMsgSender()].sub(amount);

      return true;
   }

   bool approve(address spender, uint256 amount) {
      stores.allowed[getMsgSender()].dst2amt[spender] = amount;

      return true;
   }

   uint256 allowance(address owner, address spender) { return stores.allowed[owner].dst2amt[spender]; }

   void mint(address account, uint256 amount) { stores.balances[account] = stores.balances[account].add(amount); }
};
