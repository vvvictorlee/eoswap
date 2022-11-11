#![cfg_attr(not(feature = "std"), no_std)]

//! Traits are extracted into a separate crate to show how the user can import
//! several foreign traits and implement those for the contract.

use ink_lang as ink;
type Balance = <ink_env::DefaultEnvironment as ink_env::Environment>::Balance;

/// Get the decimals and  the latest answer value.
#[ink::trait_definition]
pub trait IOracle {
    /// Returns the latest value of the oracle.
    #[ink(message)]
    fn latest_answer(&self) -> Balance;
    /// Returns the decimals of the oracle.
    #[ink(message)]
    fn decimals(&self) -> u32;
}
