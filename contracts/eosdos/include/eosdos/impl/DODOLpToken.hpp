/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/Ownable.hpp>
#include <eosdos/lib/SafeMath.hpp>
using namespace SafeMath;
/**
 * @title DODOLpToken
 * @author DODO Breeder
 *
 * @notice Tokenize liquidity pool assets. An ordinary ERC20 contract with mint and burn functions
 */
class DODOLpToken : public Ownable {
 private:
   TokenStore& stores;
   TokenStore& ostores;
   IFactory&   factory;

 public:
   DODOLpToken(TokenStore& _stores, TokenStore& _ostores, IFactory& _factory)
       : stores(_stores)
       , ostores(_ostores)
       , factory(_factory)
       , Ownable(_stores.ownable) {}

   // ============ Functions ============

   void init(const extended_asset& token, const extended_symbol& _originToken) {
      stores.esymbol     = token.get_extended_symbol();
      stores.originToken = _originToken;
      transfer_mgmt::static_create(getMsgSender(), token);
   }

   const extended_symbol& get_esymbol() { return stores.esymbol; }
   std::string            names() {
      std::string lpTokenSuffix = "_DODO_LP_TOKEN_";
      //   return string(abi.encodePacked(IERC20(originToken).name(), lpTokenSuffix));
      return ostores.names + lpTokenSuffix;
      // std::to_string(sym.precision()) + sym.code().to_string() + "@" + exsym.get_contract().to_string();
   }

   std::string            symbol() { return stores.esymbol.get_symbol().code().to_string(); };
   const extended_symbol& originToken() { return stores.originToken; };

   uint8   decimals() { return stores.esymbol.get_symbol().precision(); }
   uint64_t totalSupply() {
      return transfer_mgmt::get_supply(stores.esymbol);
      // return stores.totalSupply;
   };

   /**
    * @dev transfer token for a specified address
    * @param to The address to transfer to.
    * @param amount The amount to be transferred.
    */
   bool transfer(address to, uint64_t amount) {
      //   require(amount <= stores.balances[getMsgSender()], "BALANCE_NOT_ENOUGH");

      //   stores.balances[getMsgSender()] = sub(stores.balances[getMsgSender()], amount);
      //   stores.balances[to]             = add(stores.balances[to], amount);
      transferFrom(getMsgSender(), to, amount);
      return true;
   }

   /**
    * @dev Gets the balance of the specified address.
    * @param owner The address to query the the balance of.
    * @return balance An uint64_t representing the amount owned by the passed address.
    */
   uint64_t balanceOf(address owner) {
      return transfer_mgmt::get_balance(owner, stores.esymbol);
      // return stores.balances[owner];
   }

   /**
    * @dev Transfer tokens from one address to another
    * @param from address The address which you want to send tokens from
    * @param to address The address which you want to transfer to
    * @param amount uint64_t the amount of tokens to be transferred
    */
   bool transferFrom(address from, address to, uint64_t amount) {
      //       require(amount <= stores.balances[from], "BALANCE_NOT_ENOUGH");
      //   require(amount <= stores.allowed[from].dst2amt[getMsgSender()], "ALLOWANCE_NOT_ENOUGH");

      //   stores.balances[from]                        = sub(stores.balances[from], amount);
      //   stores.balances[to]                          = sub(stores.balances[to], amount);
      //   stores.allowed[from].dst2amt[getMsgSender()] = sub(stores.allowed[from].dst2amt[getMsgSender()], amount);
      transfer_mgmt::static_transfer(from, to, extended_asset(amount, stores.esymbol));
      return true;
   }

   /**
    * @dev Approve the passed address to spend the specified amount of tokens on behalf of getMsgSender().
    * @param spender The address which will spend the funds.
    * @param amount The amount of tokens to be spent.
    */
   bool approve(address spender, uint64_t amount) {
      //   stores.allowed[getMsgSender()].dst2amt[spender] = amount;

      return true;
   }

   /**
    * @dev Function to check the amount of tokens that an owner allowed to a spender.
    * @param owner address The address which owns the funds.
    * @param spender address The address which will spend the funds.
    * @return A uint64_t specifying the amount of tokens still available for the spender.
    */
   uint64_t allowance(address owner, address spender) {
      return uint64_t(-1);
      // return stores.allowed[owner].dst2amt[spender];
   }

   void mint(address user, uint64_t value) {
      //   stores.balances[user] = add(stores.balances[user], value);
      //   stores.totalSupply    = add(stores.totalSupply, value);
      transfer_mgmt::static_issue(user, extended_asset(value, stores.esymbol));
   }

   void burn(address user, uint64_t value) {
      //   stores.balances[user] = sub(stores.balances[user], value);
      //   stores.totalSupply    = sub(stores.totalSupply, value);
      transfer_mgmt::static_burn(user, extended_asset(value, stores.esymbol));
   }
};
