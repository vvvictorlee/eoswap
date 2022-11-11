#![cfg_attr(not(feature = "std"), no_std)]

//! Traits are extracted into a separate crate to show how the user can import
//! several foreign traits and implement those for the contract.

use ink_lang as ink;
use ink_prelude::vec::Vec;
/// @title Callback for IUniswapV3PoolActions#swap
/// @notice Any contract that calls IUniswapV3PoolActions#swap must implement this interface
#[ink::trait_definition]
pub trait ISubSwapCallback {
    /// @notice Called to `msg.sender` after executing a swap via IUniswapV3Pool#swap.
    /// @dev In the implementation you must pay the pool tokens owed for the swap.
    /// The caller of this method must be checked to be a UniswapV3Pool deployed by the canonical UniswapV3Factory.
    /// amount0Delta and amount1Delta can both be 0 if no tokens were swapped.
    /// @param amount0Delta The amount of token0 that was sent (negative) or must be received (positive) by the pool by
    /// the end of the swap. If positive, the callback must send that amount of token0 to the pool.
    /// @param amount1Delta The amount of token1 that was sent (negative) or must be received (positive) by the pool by
    /// the end of the swap. If positive, the callback must send that amount of token1 to the pool.
    /// @param data Any data passed through by the caller via the IUniswapV3PoolActions#swap call
    #[ink(message)]
    fn sub_swap_callback(
         amount0_delta:I256,
         amount1_delta:I256,
         data:Vec<u8>,
    ) ;
}
