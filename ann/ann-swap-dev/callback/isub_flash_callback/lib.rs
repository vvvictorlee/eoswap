#![cfg_attr(not(feature = "std"), no_std)]

//! Traits are extracted into a separate crate to show how the user can import
//! several foreign traits and implement those for the contract.

use ink_lang as ink;
use ink_prelude::vec::Vec;

/// @title Callback for IUniswapV3PoolActions#flash
/// @notice Any contract that calls IUniswapV3PoolActions#flash must implement this interface
#[ink::trait_definition]
pub trait ISubFlashCallback {
    /// @notice Called to `msg.sender` after transferring to the recipient from IUniswapV3Pool#flash.
    /// @dev In the implementation you must repay the pool the tokens sent by flash plus the computed fee amounts.
    /// The caller of this method must be checked to be a UniswapV3Pool deployed by the canonical UniswapV3Factory.
    /// @param fee0 The fee amount in token0 due to the pool by the end of the flash
    /// @param fee1 The fee amount in token1 due to the pool by the end of the flash
    /// @param data Any data passed through by the caller via the IUniswapV3PoolActions#flash call
    #[ink(message)]
    fn sub_flash_callback(
        fee0:U256,
        fee1:U256,
        data:Vec<u8>,
    ) ;
}
