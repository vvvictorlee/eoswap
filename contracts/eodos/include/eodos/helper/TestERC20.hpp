/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
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
        require(amount <= balances[getMsgSender()], "BALANCE_NOT_ENOUGH");

        balances[getMsgSender()] = balances[getMsgSender()].sub(amount);
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
        require(amount <= allowed[from][getMsgSender()], "ALLOWANCE_NOT_ENOUGH");

        balances[from] = balances[from].sub(amount);
        balances[to] = balances[to].add(amount);
        allowed[from][getMsgSender()] = allowed[from][getMsgSender()].sub(amount);
        
        return true;
    }

    bool  approve(address spender, uint256 amount) {
        allowed[getMsgSender()][spender] = amount;
        
        return true;
    }

    uint256  allowance(address owner, address spender) {
        return allowed[owner][spender];
    }

    void  mint(address account, uint256 amount) {
        balances[account] = balances[account].add(amount);
    }
};
