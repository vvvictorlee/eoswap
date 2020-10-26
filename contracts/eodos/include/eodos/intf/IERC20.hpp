// This is a file copied from https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/IERC20.sol
// SPDX-License-Identifier: MIT
#pragma once
#prama once 
 #include <common/defines.hpp>


/**
 * @dev Interface of the ERC20 standard as defined in the EIP.
 */
class IERC20 { 
 public:

    /**
     * @dev Returns the amount of tokens in existence.
     */
    virtual uint8  totalSupply() = 0;

    virtual string   name() = 0;

    /**
     * @dev Returns the amount of tokens owned by `account`.
     */
    virtual uint256  balanceOf(address account) = 0;

    /**
     * @dev Moves `amount` tokens from the caller's account to `recipient`.
     *
     * Returns a boolean value indicating whether the operation succeeded.
     *
     * Emits a {Transfer} 
};
