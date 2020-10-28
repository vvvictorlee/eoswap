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

 public:
   DODOLpToken(TokenStore& _stores, TokenStore& _ostores)
       : stores(_stores),ostores(_ostores)
       , Ownable(_stores.ownable) {}

   // ============ Functions ============

  void init(const extended_symbol& _originToken) { stores.originToken = _originToken; }
   const extended_symbol&  get_esymbol() { return stores.esymbol; }
   string name() {
      std::string lpTokenSuffix = "_DODO_LP_TOKEN_";
      //   return string(abi.encodePacked(IERC20(originToken).name(), lpTokenSuffix));
      return ostores.names + lpTokenSuffix;
   }

   uint8 decimals() { return ostores.decimals; }

   /**
    * @dev transfer token for a specified address
    * @param to The address to transfer to.
    * @param amount The amount to be transferred.
    */
   bool transfer(address to, uint256 amount) {
      require(amount <= stores.balances[getMsgSender()], "BALANCE_NOT_ENOUGH");

      stores.balances[getMsgSender()] = sub(stores.balances[getMsgSender()],amount);
      stores.balances[to]             =  add(stores.balances[to],amount);

      return true;
   }

   /**
    * @dev Gets the balance of the specified address.
    * @param owner The address to query the the balance of.
    * @return balance An uint256 representing the amount owned by the passed address.
    */
   uint256 balanceOf(address owner) { return stores.balances[owner]; }

   /**
    * @dev Transfer tokens from one address to another
    * @param from address The address which you want to send tokens from
    * @param to address The address which you want to transfer to
    * @param amount uint256 the amount of tokens to be transferred
    */
   bool transferFrom(address from, address to, uint256 amount) {
      require(amount <= stores.balances[from], "BALANCE_NOT_ENOUGH");
      require(amount <= stores.allowed[from].dst2amt[getMsgSender()], "ALLOWANCE_NOT_ENOUGH");

      stores.balances[from]                = sub(stores.balances[from],amount);
      stores.balances[to]                  = sub(stores.balances[to],amount);
      stores.allowed[from].dst2amt[getMsgSender()] = sub(stores.allowed[from].dst2amt[getMsgSender()],amount);

      return true;
   }

   /**
    * @dev Approve the passed address to spend the specified amount of tokens on behalf of getMsgSender().
    * @param spender The address which will spend the funds.
    * @param amount The amount of tokens to be spent.
    */
   bool approve(address spender, uint256 amount) {
      stores.allowed[getMsgSender()].dst2amt[spender] = amount;

      return true;
   }

   /**
    * @dev Function to check the amount of tokens that an owner allowed to a spender.
    * @param owner address The address which owns the funds.
    * @param spender address The address which will spend the funds.
    * @return A uint256 specifying the amount of tokens still available for the spender.
    */
   uint256 allowance(address owner, address spender) { return stores.allowed[owner].dst2amt[spender]; }

   void mint(address user, uint256 value) {
      stores.balances[user] = add(stores.balances[user],value);
      stores.totalSupply    = add(stores.totalSupply,value);
   }

   void burn(address user, uint256 value) {
      stores.balances[user] = sub(stores.balances[user],value);
      stores.totalSupply    = sub(stores.totalSupply,value);
   }
};
