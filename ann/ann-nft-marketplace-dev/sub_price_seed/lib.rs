//! # Price Seed Contract
//!
//! This is a Price Seed implementation.
//!

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_price_seed::{SubPriceSeed, SubPriceSeedRef};

use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
mod sub_price_seed {
    use ink_prelude::vec::Vec;
    use ink_storage::{traits::SpreadAllocate, Mapping};
    use scale::{Decode, Encode};
    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubPriceSeed {
        /// Keeps track of oracles for each tokens
        oracles: Mapping<AccountId, AccountId>,
        tokens: Vec<AccountId>,
        /// The address registry contract
        address_registry: AccountId,
        /// The wrapped SUB contract
        wrapped_token: AccountId,
        /// The contract owner
        owner: AccountId,
        test_enabled: Mapping<AccountId, bool>,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        OracleAlreadySet,
        InvalidPayToken,
        OracleNotSet,
        TransactionFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }
    pub type Result<T> = core::result::Result<T, Error>;
    impl SubPriceSeed {
        /// Creates a new price seed contract.
        #[ink(constructor)]
        pub fn new(address_registry: AccountId, wrapped_token: AccountId) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.address_registry = address_registry;
                contract.wrapped_token = wrapped_token;
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
        ///Register oracle contract to token
        /// Only owner can register oracle
        /// # Fields
        ///token ERC20 token address
        ///oracle Oracle address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        /// - If the `oracle`  already set.
        #[ink(message)]
        pub fn register_oracle(&mut self, token: AccountId, oracle: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.token_registry_enabled(self.get_token_registry()?, token)
                    .unwrap_or(false),
                Error::InvalidPayToken,
            );
            ensure!(
                self.oracles
                    .get(&token)
                    .unwrap_or(AccountId::from([0x0; 32]))
                    == AccountId::from([0x0; 32]),
                Error::OracleAlreadySet
            );
            self.oracles.insert(&token, &oracle);
            self.tokens.push(token);
            Ok(())
        }

        ///Update oracle address for token
        /// Only owner can update oracle
        /// # Fields
        ///token ERC20 token address
        ///oracle Oracle address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .  
        /// - If the `oracle`  does not set.  
        #[ink(message)]
        pub fn update_oracle(&mut self, token: AccountId, oracle: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.oracles
                    .get(&token)
                    .unwrap_or(AccountId::from([0x0; 32]))
                    != AccountId::from([0x0; 32]),
                Error::OracleNotSet
            );
            self.oracles.insert(&token, &oracle);
            Ok(())
        }

        ///Update address registry contract
        /// Only admin
        /// # Fields
        /// address_registry new address registry contract address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_address_registry(&mut self, address_registry: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.address_registry = address_registry;
            Ok(())
        }

        ///Get oracle from the token  
        /// # Fields
        /// token token contract address
        #[ink(message)]
        pub fn oracle_of(&self, token: AccountId) -> AccountId {
            self.oracles.get(&token).unwrap_or_default()
        }

        ///Get price from the oracle  contract
        /// # Fields
        /// token token contract address
        #[ink(message)]
        pub fn get_price(&self, token: AccountId) -> (Balance, u32) {
            if let Some(oracle) = self.oracles.get(&token) {
                self.get_price_from_oracle(oracle)
            } else {
                (1, 12)
            }
            // IOracle oracle = IOracle(oracles[token]);
            // return (oracle.latestAnswer(), oracle.decimals());
        }
        ///  ERC20 contract  address  of Wrapped_ native token
        /// #return token contract address
        #[ink(message)]
        pub fn wrapped_token(&self) -> AccountId {
            self.wrapped_token
        }
        /// Get address_registry
        /// # Return
        ///  address_registry  address_registry
        #[ink(message)]
        pub fn address_registry(&self) -> AccountId {
            self.address_registry
        }

        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
        #[ink(message)]
        pub fn oracles(&self) -> Vec<AccountId> {
            self.tokens
                .iter()
                .map(|t| self.oracles.get(t).unwrap())
                .collect()
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn get_price_from_oracle(&self, oracle: AccountId) -> (Balance, u32) {
            #[cfg(test)]
            {
                (1, 12)
            }
            #[cfg(not(test))]
            {
                use sub_oracle::IOracle;
                let oracle_instance: sub_oracle::SubOracleRef =
                    ink_env::call::FromAccountId::from_account_id(oracle);
                (oracle_instance.latest_answer(), oracle_instance.decimals())
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn get_token_registry(&self) -> Result<AccountId> {
            #[cfg(test)]
            {
                Ok(AccountId::from([0x1; 32]))
            }
            #[cfg(not(test))]
            {
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);
                ensure!(
                    AccountId::from([0x0; 32]) != address_registry_instance.token_registry(),
                    Error::InvalidPayToken
                );
                Ok(address_registry_instance.token_registry())
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn token_registry_enabled(&self, callee: AccountId, token: AccountId) -> Result<bool> {
            #[cfg(test)]
            {
                Ok(self.test_enabled.get(&token).unwrap_or(false))
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x14, 0x14, 0x63, 0x1C]; //0x1414631c enabled
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(callee)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(token))
                    .returns::<bool>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("supports_interface_check= {:?}", e);
                        Error::TransactionFailed
                    })
            }
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

        fn init_contract() -> SubPriceSeed {
            let erc = SubPriceSeed::new(bob(), charlie());

            erc
        }
        #[ink::test]
        fn register_oracle_works() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            price_seed.test_enabled.insert(&token, &true);

            assert!(price_seed.register_oracle(token, oracle).is_ok());

            assert_eq!(price_seed.oracles.get(&token), Some(oracle));
        }
        #[ink::test]
        fn register_oracle_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = charlie();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            price_seed.test_enabled.insert(&token, &true);
            assert_eq!(
                price_seed.register_oracle(token, oracle).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn register_oracle_failed_if_the_pay_token_is_not_enabled_in_the_token_registry_contract_when_the_pay_token_is_not_zero_address(
        ) {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            price_seed.oracles.insert(&token, &charlie());
            assert_eq!(
                price_seed.register_oracle(token, oracle).unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn register_oracle_failed_if_the_oracle_already_set() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            price_seed.oracles.insert(&token, &charlie());
            price_seed.test_enabled.insert(&token, &true);
            assert_eq!(
                price_seed.register_oracle(token, oracle).unwrap_err(),
                Error::OracleAlreadySet
            );
        }

        #[ink::test]
        fn update_oracle_works() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            price_seed.oracles.insert(&token, &charlie());
            assert!(price_seed.update_oracle(token, oracle).is_ok());

            assert_eq!(price_seed.oracles.get(&token), Some(oracle));
        }

        #[ink::test]
        fn update_oracle_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = charlie();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            price_seed.oracles.insert(&token, &charlie());
            assert_eq!(
                price_seed.update_oracle(token, oracle).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_oracle_failed_if_the_oracle_does_not_set() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            let oracle = bob();
            // price_seed.oracles.insert(&token, &charlie());
            assert_eq!(
                price_seed.update_oracle(token, oracle).unwrap_err(),
                Error::OracleNotSet
            );
        }

        #[ink::test]
        fn update_address_registry_works() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let address_registry = bob();

            assert!(price_seed.update_address_registry(address_registry).is_ok());

            assert_eq!(price_seed.address_registry, address_registry);
        }

        #[ink::test]
        fn update_address_registry_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = charlie();
            set_caller(caller);
            let address_registry = bob();
            assert_eq!(
                price_seed
                    .update_address_registry(address_registry)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }
        #[ink::test]
        fn get_price_works() {
            // Create a new contract instance.
            let price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = bob();
            assert_eq!(price_seed.get_price(token), (0, 0));
        }
        #[ink::test]
        fn wrapped_token_works() {
            // Create a new contract instance.
            let mut price_seed = init_contract();
            let caller = alice();
            set_caller(caller);
            let wrapped_token = bob();
            price_seed.wrapped_token = wrapped_token;

            assert_eq!(price_seed.wrapped_token(), wrapped_token);
        }
    }
}
