//! # Token Registry
//!
//! This is a Token Registry implementation.
//!

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_token_registry::{SubTokenRegistry, SubTokenRegistryRef};

use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
mod sub_token_registry {
    use ink_prelude::{collections::BTreeSet, vec::Vec};
    use ink_storage::{traits::SpreadAllocate, Mapping};
    use scale::{Decode, Encode};

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubTokenRegistry {
        /// Mapping ERC20 Address to  Bool
        enabled: Mapping<AccountId, bool>,
        tokens: Mapping<bool, BTreeSet<AccountId>>,
        /// The contract owner
        owner: AccountId,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        TokenAlreadyAdded,
        TokenNotExist,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubTokenRegistry result types.
    pub type Result<T> = core::result::Result<T, Error>;
    /// Event emitted when a token added occurs.
    #[ink(event)]
    pub struct TokenAdded {
        token: AccountId,
    }
    /// Event emitted when a token removed occurs.
    #[ink(event)]
    pub struct TokenRemoved {
        token: AccountId,
    }

    impl SubTokenRegistry {
        /// Creates a new token registry contract.
        #[ink(constructor)]
        pub fn new() -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
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
        /// Method for adding payment token
        /// Only admin
        /// # Fields
        /// token ERC20 token address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        /// - If the `token`  already added.  
        #[ink(message)]
        pub fn add(&mut self, token: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                !self.enabled.get(&token).unwrap_or(false),
                Error::TokenAlreadyAdded
            );
            self.enabled.insert(&token, &true);
            let mut tokens = self.tokens.get(&true).unwrap_or_default();
            tokens.insert(token);
            self.tokens.insert(true, &tokens);
            self.env().emit_event(TokenAdded { token });
            Ok(())
        }

        /// Method for removing payment token
        /// Only admin
        /// # Fields
        /// token ERC20 token address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        /// - If the `token`  does not add.  
        #[ink(message)]
        pub fn remove(&mut self, token: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.enabled.get(&token).unwrap_or(false),
                Error::TokenNotExist
            );
            self.enabled.remove(&token);
            let mut tokens = self.tokens.get(&true).unwrap_or_default();
            tokens.remove(&token);
            self.env().emit_event(TokenRemoved { token });
            Ok(())
        }
        /// Get enabled of the specified token
        /// # Return
        ///  enabled  enabled
        #[ink(message)]
        pub fn enabled(&self, token: AccountId) -> bool {
            self.enabled.get(token).unwrap_or(false)
        }
        /// Get tokens of the enabled tokens
        /// # Return
        ///  tokens  tokens
        #[ink(message)]
        pub fn tokens(&self) -> Vec<AccountId> {
            self.tokens
                .get(&true)
                .unwrap_or_default()
                .iter()
                .cloned()
                .collect::<Vec<AccountId>>()
        }
        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
    }

    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        // use ink_env::Clear;
        use ink_lang as ink;
        type Event = <SubTokenRegistry as ::ink_lang::reflect::ContractEventBase>::Type;
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

        fn init_contract() -> SubTokenRegistry {
            let erc = SubTokenRegistry::new();

            erc
        }
        #[ink::test]
        fn add_token_works() {
            // Create a new contract instance.
            let mut token_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = charlie();
            assert!(token_registry.add(token).is_ok());

            assert_eq!(token_registry.enabled.get(&token), Some(true));
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_token_added_event(&emitted_events[0], token);
        }

        #[ink::test]
        fn add_token_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut token_registry = init_contract();
            let caller = bob();
            set_caller(caller);
            let token = charlie();
            assert_eq!(token_registry.add(token).unwrap_err(), Error::OnlyOwner);
        }

        #[ink::test]
        fn remove_token_works() {
            // Create a new contract instance.
            let mut token_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = charlie();
            token_registry.enabled.insert(&token, &true);

            assert!(token_registry.remove(token).is_ok());

            assert_eq!(token_registry.enabled.get(&token), None);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_token_removed_event(&emitted_events[0], token);
        }

        #[ink::test]
        fn remove_token_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut token_registry = init_contract();
            let caller = bob();
            set_caller(caller);
            let token = charlie();
            token_registry.enabled.insert(&token, &true);

            assert_eq!(token_registry.remove(token).unwrap_err(), Error::OnlyOwner);
        }

        #[ink::test]
        fn token_enabled_works() {
            // Create a new contract instance.
            let mut token_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let token = charlie();
            token_registry.enabled.insert(&token, &true);

            assert_eq!(token_registry.enabled(token), true);
            token_registry.enabled.insert(&token, &false);

            assert_eq!(token_registry.enabled(token), false);
        }

        fn assert_token_added_event(
            event: &ink_env::test::EmittedEvent,
            expected_token: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::TokenAdded(TokenAdded { token }) = decoded_event {
                assert_eq!(
                    token, expected_token,
                    "encountered invalid TokenAdded.token"
                );
            } else {
                panic!("encountered unexpected event kind: expected a TokenAdded event")
            }
        }

        fn assert_token_removed_event(
            event: &ink_env::test::EmittedEvent,
            expected_token: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::TokenRemoved(TokenRemoved { token }) = decoded_event {
                assert_eq!(
                    token, expected_token,
                    "encountered invalid TokenRemoved.token"
                );
            } else {
                panic!("encountered unexpected event kind: expected a TokenRemoved event")
            }
        }

        /// For calculating the event topic hash.
        struct PrefixedValue<'a, 'b, T> {
            pub prefix: &'a [u8],
            pub value: &'b T,
        }

        impl<X> scale::Encode for PrefixedValue<'_, '_, X>
        where
            X: scale::Encode,
        {
            #[inline]
            fn size_hint(&self) -> usize {
                self.prefix.size_hint() + self.value.size_hint()
            }

            #[inline]
            fn encode_to<T: scale::Output + ?Sized>(&self, dest: &mut T) {
                self.prefix.encode_to(dest);
                self.value.encode_to(dest);
            }
        }
    }
}
