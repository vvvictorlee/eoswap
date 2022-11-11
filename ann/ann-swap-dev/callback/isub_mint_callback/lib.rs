#![cfg_attr(not(feature = "std"), no_std)]

//! Traits are extracted into a separate crate to show how the user can import
//! several foreign traits and implement those for the contract.

use ink_lang as ink;
use ink_prelude::vec::Vec;

/// @title Callback for IUniswapV3PoolActions#mint
/// @notice Any contract that calls IUniswapV3PoolActions#mint must implement this interface
#[ink::trait_definition]
pub trait ISubMintCallback {
    /// @notice Called to `msg.sender` after minting liquidity to a position from IUniswapV3Pool#mint.
    /// @dev In the implementation you must pay the pool tokens owed for the minted liquidity.
    /// The caller of this method must be checked to be a UniswapV3Pool deployed by the canonical UniswapV3Factory.
    /// @param amount0Owed The amount of token0 due to the pool for the minted liquidity
    /// @param amount1Owed The amount of token1 due to the pool for the minted liquidity
    /// @param data Any data passed through by the caller via the IUniswapV3PoolActions#mint call
    #[ink(message)]    
    fn sub_mint_callback(
        amount0_owed:U256,
        amount1_owed:U256,
        data:Vec<u8>,
    ) ;
}