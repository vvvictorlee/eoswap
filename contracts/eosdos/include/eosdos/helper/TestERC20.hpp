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
//    TokenStore& stores;
extended_symbol esymbol;
 public:
   TestERC20(const extended_symbol& _esymbol)
       : esymbol(_esymbol) {}
   void init(const extended_asset& token) {
      //   stores.decimals = _decimals;
    //   stores.esymbol = token.get_extended_symbol();
      //   stores.originToken = esymbol;
      transfer_mgmt::static_create(msg_sender, token);
   }

   name getMsgSender() { return msg_sender; }
      void setMsgSender(name _msg_sender, bool flag = false) {
      if (flag) {
         require_auth(_msg_sender);
      }
      msg_sender = _msg_sender;
   }
   bool transfer(address to, uint64_t amount) {
      //   require(to != address(0), "TO_ADDRESS_IS_EMPTY");
      //   require(amount <= stores.balances[getMsgSender()], "BALANCE_NOT_ENOUGH");
      //   stores.balances[getMsgSender()] = sub(stores.balances[getMsgSender()], amount);
      //   stores.balances[to]             = add(stores.balances[to], amount);
      //   return true;

      return transferFrom(getMsgSender(), to, amount);
   }

   uint64_t balanceOf(address owner) {
      return transfer_mgmt::get_balance(owner, esymbol);
      // return stores.balances[owner];
   }

   bool transferFrom(address from, address to, uint64_t amount) {
      //   require(to != address(0), "TO_ADDRESS_IS_EMPTY");
      //   require(amount <= stores.balances[from], "BALANCE_NOT_ENOUGH");
      //   require(amount <= stores.allowed[from].dst2amt[getMsgSender()], "ALLOWANCE_NOT_ENOUGH");

      //   stores.balances[from]                        = sub(stores.balances[from], amount);
      //   stores.balances[to]                          = add(stores.balances[to], amount);
      //   stores.allowed[from].dst2amt[getMsgSender()] = sub(stores.allowed[from].dst2amt[getMsgSender()], amount);

      transfer_mgmt::static_transfer(from, to, extended_asset(amount, esymbol));

      return true;
   }

   bool approve(address spender, uint64_t amount) {
      //   stores.allowed[getMsgSender()].dst2amt[spender] = amount;

      return true;
   }

   uint64_t allowance(address owner, address spender) {
      return uint64_t(-1);
      // return stores.allowed[owner].dst2amt[spender];
   }

   void mint(address account, uint64_t amount) {
      // stores.balances[account] = add(stores.balances[account], amount);
      transfer_mgmt::static_issue(account, extended_asset(amount, esymbol));
   }
};
