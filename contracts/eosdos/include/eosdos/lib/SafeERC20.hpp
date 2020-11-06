/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0
    This is a simplified version of OpenZepplin's SafeERC20 library

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/intf/IERC20.hpp>

/**
 * @title SafeERC20
 * @dev Wrappers around ERC20 operations that throw on failure (when the token
 * contract returns false). Tokens that return no value (and instead revert or
 * throw on failure) are also supported, non-reverting calls are assumed to be
 * successful.
 * To use this library you can add a `` statement to your contract,
 * which allows you to call the safe operations as `token.safeTransfer(...)`, etc.
 */
namespace SafeERC20 {

/**
 * @dev Imitates a Solidity high-level call (i.e. a regular function call to a contract), relaxing the requirement
 * on the return value: the return value is optional (but if data is returned, it must not be false).
 * @param token The token targeted by the call.
 * @param data The call data (encoded using abi.encode or one of its variants).
 */
template <typename T>
void _callOptionalReturn(IERC20& token, T data) {
   // We need to perform a low level call here, to bypass Solidity's return data size checking mechanism, since
   // we're implementing it ourselves.

   // A Solidity high level call has three parts:
   //  1. The target address is checked to verify it contains contract code
   //  2. The call itself is made, and success asserted
   //  3. The return value is decoded, which in turn checks the size of the returned data.
   // solhint-disable-next-line max-line-length

   // solhint-disable-next-line avoid-low-level-calls
#ifdef _CALLOPTIONALRETURN
   (bool success, bytes returndata) = address(token).call(data);
   require(success, "SafeERC20: low-level call failed");

   if (returndata.length > 0) {
      // Return data is optional
      // solhint-disable-next-line max-line-length
      require(abi.decode(returndata, (bool)), "SafeERC20: ERC20 operation did not succeed");
   }
#endif
}

void safeTransfer(IERC20& token, address to, uint64_t value) {
   _callOptionalReturn(token, std::make_tuple("token.transfer.selector", to, value));
}

void safeTransferFrom(IERC20& token, address from, address to, uint64_t value) {
   _callOptionalReturn(token, std::make_tuple("token.transferFrom.selector", from, to, value));
}

void safeApprove(IERC20& token, name self,address spender, uint64_t value) {
   // safeApprove should only be called when setting an initial allowance,
   // or when resetting it to zero. To increase and decrease it, use
   // 'safeIncreaseAllowance' and 'safeDecreaseAllowance'
   // solhint-disable-next-line max-line-length
   require(
       (value == 0) || (token.allowance(self, spender) == 0),
       "SafeERC20: approve from non-zero to non-zero allowance");
   _callOptionalReturn(token, std::make_tuple("token.approve.selector", spender, value));
}

} // namespace SafeERC20
