/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/lib/SafeMath.hpp>

class TestERC20 {
 private:
   name        msg_sender;
   TokenStore& stores;

 public:
   TestERC20(TokenStore& _stores)
       : stores(_stores) {}
   void init(string _name, uint8 _decimals, const extended_symbol& esymbol) {
      stores.names    = _name;
      stores.decimals = _decimals;
      stores.esymbol  = esymbol;
      //   stores.originToken = esymbol;
   }

   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender) {
      require_auth(_msg_sender);
      msg_sender = _msg_sender;
   }
   bool transfer(address to, uint256 amount) {
      require(to != address(0), "TO_ADDRESS_IS_EMPTY");
      require(amount <= stores.balances[getMsgSender()], "BALANCE_NOT_ENOUGH");
      stores.balances[getMsgSender()] = sub(stores.balances[getMsgSender()], amount);
      stores.balances[to]             = add(stores.balances[to], amount);
      return true;
   }

   uint256 balanceOf(address owner) { return stores.balances[owner]; }

   bool transferFrom(address from, address to, uint256 amount) {
      print("\n====testerc20============transferFrom====");
      from.print();
      print("\n=======testerc20=========transferFrom====");
      getMsgSender().print();
      print("\n=======testerc20=========transferFrom====");
      to.print();
      print("\n========testerc20========transferFrom====");
stores.esymbol.print();
 print("\n========testerc20========transferFrom====");
 print(amount,"\n========testerc20========transferFrom====",stores.allowed[from].dst2amt[getMsgSender()]);
      require(to != address(0), "TO_ADDRESS_IS_EMPTY");
      require(amount <= stores.balances[from], "BALANCE_NOT_ENOUGH");
      require(amount <= stores.allowed[from].dst2amt[getMsgSender()], "ALLOWANCE_NOT_ENOUGH");

      stores.balances[from]                        = sub(stores.balances[from], amount);
      stores.balances[to]                          = add(stores.balances[to], amount);
      stores.allowed[from].dst2amt[getMsgSender()] = sub(stores.allowed[from].dst2amt[getMsgSender()], amount);

      return true;
   }

   bool approve(address spender, uint256 amount) {
      stores.allowed[getMsgSender()].dst2amt[spender] = amount;

      return true;
   }

   uint256 allowance(address owner, address spender) { return stores.allowed[owner].dst2amt[spender]; }

   void mint(address account, uint256 amount) { stores.balances[account] = add(stores.balances[account], amount); }
};
