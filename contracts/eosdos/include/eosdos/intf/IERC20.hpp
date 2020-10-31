// This is a file copied from https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/token/ERC20/IERC20.sol
// SPDX-License-Identifier: MIT
#pragma once
#pragma once 
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

    virtual std::string   name() = 0;

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
*/
    virtual bool  transfer(address recipient, uint256 amount) = 0;

    /**
     * @dev Returns the remaining number of tokens that `spender` will be
     * allowed to spend on behalf of `owner` through {transferFrom}. This is
     * zero by default.
     *
     * This value changes when {approve} or {transferFrom} are called.
     */
    virtual uint256  allowance(address owner, address spender) = 0;

    /**
     * @dev Sets `amount` as the allowance of `spender` over the caller's tokens.
     *
     * Returns a boolean value indicating whether the operation succeeded.
     *
     * IMPORTANT: Beware that changing an allowance with this method brings the risk
     * that someone may use both the old and the new allowance by unfortunate
     * transaction ordering. One possible solution to mitigate this race
     * condition is to first reduce the spender's allowance to 0 and set the
     * desired value afterwards:
     * https://github.com/ethereum/EIPs/issues/20#issuecomment-263524729
     *
     * Emits an {Approval} event.
     */
    virtual bool  approve(address spender, uint256 amount) = 0;

    /**
     * @dev Moves `amount` tokens from `sender` to `recipient` using the
     * allowance mechanism. `amount` is then deducted from the caller's
     * allowance.
     *
     * Returns a boolean value indicating whether the operation succeeded.
     *
     * Emits a {Transfer} event.
     */
    virtual bool  transferFrom(
        address sender,
        address recipient,
        uint256 amount
    ) = 0;
};
