/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

#include <eodos/lib/SafeMath.hpp>

class TestERC20 { 
 public:

    


    

    TestERC20(string  _name, uint8 _decimals) {
        name = _name;
        decimals = _decimals;
    }

    bool  transfer(address to, uint256 amount) {
        require(to != address(0), "TO_ADDRESS_IS_EMPTY");
        require(amount <= balances[msg.sender], "BALANCE_NOT_ENOUGH");

        balances[msg.sender] = balances[msg.sender].sub(amount);
        balances[to] = balances[to].add(amount);
        
        return true;
    }

    uint256 balance  balanceOf(address owner) {
        return balances[owner];
    }

    bool transferFrom(
        address from,
        address to,
        uint256 amount
    ) {
        require(to != address(0), "TO_ADDRESS_IS_EMPTY");
        require(amount <= balances[from], "BALANCE_NOT_ENOUGH");
        require(amount <= allowed[from][msg.sender], "ALLOWANCE_NOT_ENOUGH");

        balances[from] = balances[from].sub(amount);
        balances[to] = balances[to].add(amount);
        allowed[from][msg.sender] = allowed[from][msg.sender].sub(amount);
        
        return true;
    }

    bool  approve(address spender, uint256 amount) {
        allowed[msg.sender][spender] = amount;
        
        return true;
    }

    uint256  allowance(address owner, address spender) {
        return allowed[owner][spender];
    }

    void  mint(address account, uint256 amount) {
        balances[account] = balances[account].add(amount);
    }
}
