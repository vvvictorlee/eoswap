/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

#include <eodos/intf/IERC20.hpp>
#include <eodos/lib/Ownable.hpp>
#include <eodos/lib/SafeMath.hpp>

/**
 * @title DODOLpToken
 * @author DODO Breeder
 *
 * @notice Tokenize liquidity pool assets. An ordinary ERC20 contract with mint and burn functions
 */
class DODOLpToken : public Ownable {
 public:

   // ============ Functions ============

   DODOLpToken(address _originToken) { originToken = _originToken; }

   string name() {
      string lpTokenSuffix = "_DODO_LP_TOKEN_";
      return string(abi.encodePacked(IERC20(originToken).name(), lpTokenSuffix));
   }

   uint8 decimals() { return IERC20(originToken).decimals(); }

   /**
    * @dev transfer token for a specified address
    * @param to The address to transfer to.
    * @param amount The amount to be transferred.
    */
   bool transfer(address to, uint256 amount) {
      require(amount <= balances[msg.sender], "BALANCE_NOT_ENOUGH");

      balances[msg.sender] = balances[msg.sender].sub(amount);
      balances[to]         = balances[to].add(amount);

      return true;
   }

   /**
    * @dev Gets the balance of the specified address.
    * @param owner The address to query the the balance of.
    * @return balance An uint256 representing the amount owned by the passed address.
    */
   uint256 balance balanceOf(address owner) { return balances[owner]; }

   /**
    * @dev Transfer tokens from one address to another
    * @param from address The address which you want to send tokens from
    * @param to address The address which you want to transfer to
    * @param amount uint256 the amount of tokens to be transferred
    */
   bool transferFrom(address from, address to, uint256 amount) {
      require(amount <= balances[from], "BALANCE_NOT_ENOUGH");
      require(amount <= allowed[from][msg.sender], "ALLOWANCE_NOT_ENOUGH");

      balances[from]            = balances[from].sub(amount);
      balances[to]              = balances[to].add(amount);
      allowed[from][msg.sender] = allowed[from][msg.sender].sub(amount);

      return true;
   }

   /**
    * @dev Approve the passed address to spend the specified amount of tokens on behalf of msg.sender.
    * @param spender The address which will spend the funds.
    * @param amount The amount of tokens to be spent.
    */
   bool approve(address spender, uint256 amount) {
      allowed[msg.sender][spender] = amount;

      return true;
   }

   /**
    * @dev Function to check the amount of tokens that an owner allowed to a spender.
    * @param owner address The address which owns the funds.
    * @param spender address The address which will spend the funds.
    * @return A uint256 specifying the amount of tokens still available for the spender.
    */
   uint256 allowance(address owner, address spender) { return allowed[owner][spender]; }

   void mint(address user, uint256 value) {
      balances[user] = balances[user].add(value);
      totalSupply    = totalSupply.add(value);
   }

   void burn(address user, uint256 value) {
      balances[user] = balances[user].sub(value);
      totalSupply    = totalSupply.sub(value);
   }
}
