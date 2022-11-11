//! # Oracle Contract
//!
//! This is a Oracle implementation.
//!

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_oracle::{SubOracle, SubOracleRef};
pub use ioracle::IOracle;

use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
mod sub_oracle {
    use ink_storage::traits::SpreadAllocate;
    use ioracle::IOracle;
    use scale::{Decode, Encode};
    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubOracle {
        /// Keeps track of oracles for each tokens
        token: AccountId,
        price: Balance,
        decimals: u32,
        /// The contract owner
        owner: AccountId,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        TransactionFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }
    pub type Result<T> = core::result::Result<T, Error>;
    impl SubOracle {
        /// Creates a new Oracle contract.
        #[ink(constructor)]
        pub fn new(token: AccountId, decimals: u32) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.token = token;
                contract.decimals = decimals;
                contract.owner = Self::env().caller();
            })
        }
        #[ink(message)]
        pub fn transfer_ownership(&mut self, new_owner: AccountId) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                AccountId::default() != new_owner,
                Error::NewOwnerIsTheZeroAddress
            );
            self.owner = new_owner;
            Ok(())
        }
        ///Update price
        /// Only owner can  Update price
        /// # Fields
        ///price balance
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update(&mut self, price: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);

            self.price = price;
            Ok(())
        }
        ///Update decimals
        /// Only owner can Update decimals
        /// # Fields
        ///decimals decimals
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .  
        #[ink(message)]
        pub fn update_decimals(&mut self, decimals: u32) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.decimals = decimals;
            Ok(())
        }
        /// Get token
        /// # Return
        ///  token  token
        #[ink(message)]
        pub fn token(&self) -> AccountId {
            self.token
        }

        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
    }
    impl IOracle for SubOracle {
        ///Get price from the oracle  contract
        /// # Fields
        /// token token contract address
        #[ink(message)]
        fn latest_answer(&self) -> Balance {
            self.price
        }
        ///  decimals of the token
        /// #return  decimals
        #[ink(message)]
        fn decimals(&self) -> u32 {
            self.decimals
        }
    }
    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_lang as ink;

        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<ink_env::DefaultEnvironment>(sender);
        }
        fn default_accounts() -> ink_env::test::DefaultAccounts<Environment> {
            ink_env::test::default_accounts::<Environment>()
        }

        fn alice() -> AccountId {
            default_accounts().alice
        }

        fn bob() -> AccountId {
            default_accounts().bob
        }

        fn charlie() -> AccountId {
            default_accounts().charlie
        }

        fn token() -> AccountId {
            default_accounts().charlie
        }

        fn decimals() -> u32 {
            12
        }

        fn init_contract() -> SubOracle {
            let erc = SubOracle::new(token(), decimals());

            erc
        }
        #[ink::test]
        fn update_price_works() {
            // Create a new contract instance.
            let mut oracle = init_contract();
            let caller = alice();
            set_caller(caller);
            assert!(oracle.update(1).is_ok());

            assert_eq!(oracle.latest_answer(), 1);
        }
        #[ink::test]
        fn update_price_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut oracle = init_contract();
            let caller = charlie();
            set_caller(caller);
            assert_eq!(oracle.update(1).unwrap_err(), Error::OnlyOwner);
        }

        #[ink::test]
        fn update_decimals_works() {
            // Create a new contract instance.
            let mut oracle = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            oracle.oracles.insert(&token, &charlie());
            assert!(oracle.update_decimals(11).is_ok());

            assert_eq!(oracle.decimals(), 11);
        }

        #[ink::test]
        fn update_decimals_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut oracle = init_contract();
            let caller = charlie();
            set_caller(caller);
            assert_eq!(oracle.update_decimals(11).unwrap_err(), Error::OnlyOwner);
        }

        #[ink::test]
        fn latest_answer_works() {
            // Create a new contract instance.
            let oracle = init_contract();
            let caller = alice();
            set_caller(caller);
            oracle.price = 1;
            assert_eq!(oracle.latest_answer(), 1);
        }
        #[ink::test]
        fn decimals_works() {
            // Create a new contract instance.
            let mut oracle = init_contract();
            let caller = alice();
            set_caller(caller);
            oracle.decimals = 11;
            assert_eq!(oracle.decimals(), 11);
        }
    }
}
