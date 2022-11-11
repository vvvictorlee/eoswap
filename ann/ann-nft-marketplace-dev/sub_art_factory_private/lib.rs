//! #  NFT Art Factory Private
//!
//! This is a NFT Art Factory Private implementation.
//!

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_art_factory_private::{SubArtFactoryPrivate, SubArtFactoryPrivateRef};

#[cfg_attr(test, allow(dead_code))]
const INTERFACE_ID_ERC1155: [u8; 4] = [0xD9, 0xB6, 0x7A, 0x26];
use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
mod sub_art_factory_private {
    use ink_lang::codegen::EmitEvent;
    use ink_prelude::string::String;
    use ink_prelude::vec::Vec;
    use ink_storage::{traits::SpreadAllocate, Mapping};
    use scale::{Decode, Encode};
    use sub_art_tradable_private::sub_art_tradable_private::{ContractCreated, ContractDisabled};

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubArtFactoryPrivate {
        ///  Marketplace contract
        marketplace: AccountId,
        ///  BundleMarketplace contract
        bundle_marketplace: AccountId,
        ///  NFT mint fee
        mint_fee: Balance,
        ///  Platform fee
        platform_fee: Balance,
        ///  Platform fee receipient
        fee_recipient: AccountId,
        endowment_amount: Balance,
        tokens: Vec<AccountId>,
        exists: Mapping<AccountId, bool>,
        /// NFT Art Tradable Private contract code hash
        code_hash: Hash,
        /// The contract owner
        owner: AccountId,
        test_instantiate_contract_failed: bool,
        test_support_interface: [u8; 4],
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        InsufficientFunds,
        TransferFailed,
        TransferOwnershipFailed,
        ArtContractAlreadyRegistered,
        NotAnERC1155Contract,
        ArtContractIsNotRegistered,
        TransactionFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubArtFactory result types.
    pub type Result<T> = core::result::Result<T, Error>;

    impl SubArtFactoryPrivate {
        /// Creates a new NFT Art Factory Private contract.
        #[ink(constructor)]
        pub fn new(
            marketplace: AccountId,
            bundle_marketplace: AccountId,
            mint_fee: Balance,
            platform_fee: Balance,
            fee_recipient: AccountId,
            code_hash: Hash,
        ) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.marketplace = marketplace;
                contract.bundle_marketplace = bundle_marketplace;
                contract.mint_fee = mint_fee;
                contract.platform_fee = platform_fee;
                contract.fee_recipient = fee_recipient;
                contract.code_hash = code_hash;
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
        /// Update Marketplace contract
        /// Only admin
        /// # Fields
        /// marketplace  new marketplace contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_marketplace(&mut self, marketplace: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.marketplace = marketplace;
            Ok(())
        }

        /// Update BundleMarketplace contract
        /// Only admin
        /// # Fields
        /// bundle_marketplace  new bundle marketplace contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_bundle_marketplace(&mut self, bundle_marketplace: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.bundle_marketplace = bundle_marketplace;
            Ok(())
        }

        /// Method for updating mint fee
        /// Only admin
        /// # Fields
        /// mint_fee the mint fee to set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn update_mint_fee(&mut self, mint_fee: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.mint_fee = mint_fee;
            Ok(())
        }

        /// Method for updating platform fee
        /// Only admin
        /// # Fields
        /// platform_fee the platform fee to set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_platform_fee(&mut self, platform_fee: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.platform_fee = platform_fee;
            Ok(())
        }

        /// Method for updating platform fee address
        /// Only admin
        /// # Fields
        /// fee_recipient payable address the address to sends the funds to  
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_platform_fee_recipient(&mut self, fee_recipient: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.fee_recipient = fee_recipient;
            Ok(())
        }

        /// Method for updating instantiate contract endowment amount
        /// Only admin
        /// # Fields
        /// endowment_amount the endowment amountto set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_endowment_amount(&mut self, endowment_amount: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.endowment_amount = endowment_amount;
            Ok(())
        }
        ///  Method for deploying new SubArtTradable contract
        /// # Fields
        ///  name Name of NFT contract
        ///  symbol Symbol of NFT contract
        ///
        /// # Errors
        ///
        /// - If the transferred value is less than the `platform_fee`.
        /// - If it failed when the contract trasfer to  fee_recipient in native token.
        /// - If `instantiate_contract` failed .
        #[ink(message, payable)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn create_nft_contract(&mut self, name: String, symbol: String) -> Result<AccountId> {
            ensure!(
                self.env().transferred_value() >= self.platform_fee,
                Error::InsufficientFunds
            );
            ensure!(
                self.env()
                    .transfer(self.fee_recipient, self.env().transferred_value())
                    .is_ok(),
                Error::TransferFailed
            );
            let instantiate_contract = || -> Result<AccountId> {
                #[cfg(test)]
                {
                    ensure!(
                        !self.test_instantiate_contract_failed,
                        Error::TransferOwnershipFailed
                    );
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_art_tradable_private::SubArtTradablePrivateRef;
                    let mut version: Vec<u8> = name.as_bytes().to_vec();
                    version.extend(symbol.as_bytes());
                    let salt = version;
                    let instance_params = SubArtTradablePrivateRef::new(
                        name,
                        symbol,
                        self.marketplace,
                        self.bundle_marketplace,
                        self.mint_fee,
                        self.fee_recipient,
                    )
                    .endowment(self.endowment_amount)
                    .code_hash(self.code_hash)
                    .salt_bytes(salt)
                    .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr = init_result
                        .expect("failed at instantiating the `art tradable private` contract");
                    let mut sub_art_tradable_private_instance: SubArtTradablePrivateRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    let _r =
                        sub_art_tradable_private_instance.transfer_ownership(self.env().caller());
                    ensure!(_r.is_ok(), Error::TransferOwnershipFailed);

                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.exists.insert(&ans_contract_addr, &true);
            self.tokens.push(ans_contract_addr);
            self.env().emit_event(ContractCreated {
                creator: self.env().caller(),
                nft_address: ans_contract_addr,
            });
            Ok(ans_contract_addr)
        }

        ///  Method for registering existing SubArtTradable contract
        /// # Fields
        ///   token_contract Address of NFT contract
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        /// - If  the artion does not  support ERC-1155 .     
        /// - If  the `token_contract` exists in the contract .    
        #[ink(message)]
        pub fn register_token_contract(&mut self, token_contract: AccountId) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                !self.exists.get(&token_contract).unwrap_or(false),
                Error::ArtContractAlreadyRegistered
            );
            ensure!(
                self.supports_interface_check(token_contract, crate::INTERFACE_ID_ERC1155),
                Error::NotAnERC1155Contract
            );
            if self.exists.get(&token_contract).is_none() {
                self.tokens.push(token_contract);
            }
            self.exists.insert(&token_contract, &true);
            self.env().emit_event(ContractCreated {
                creator: self.env().caller(),
                nft_address: token_contract,
            });
            Ok(())
        }

        ///  Method for disabling existing SubArtTradablePrivate contract
        /// # Fields
        ///   token_contract Address of NFT contract
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        /// - If  the `token_contract` does not exist in the contract .    
        #[ink(message)]
        pub fn disable_token_contract(&mut self, token_contract: AccountId) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);

            ensure!(
                self.exists.get(&token_contract).unwrap_or(false),
                Error::ArtContractIsNotRegistered
            );

            self.exists.insert(&token_contract, &false);
            self.env().emit_event(ContractDisabled {
                caller: self.env().caller(),
                nft_address: token_contract,
            });
            Ok(())
        }
        /// Returns whether the specified token exists by checking to see if it has a creator
        /// # Fields
        /// token   the address of the token to query the existence of
        /// #return bool whether the token exists
        #[ink(message)]
        pub fn exists(&self, token: AccountId) -> bool {
            self.exists.get(&token).unwrap_or(false)
        }
        #[ink(message)]
        pub fn tokens(&self) -> Vec<AccountId> {
            self.tokens
                .iter()
                .filter(|token| self.exists.get(token).unwrap_or(false))
                .cloned()
                .collect()
        }
        /// Querying marketplace contract address
        /// # return marketplace contract address
        #[ink(message)]
        pub fn marketplace(&self) -> AccountId {
            self.marketplace
        }
        /// Querying bundle marketplace contract address
        /// # return bundle marketplace contract address
        #[ink(message)]
        pub fn bundle_marketplace(&self) -> AccountId {
            self.bundle_marketplace
        }
        /// Get mint_fee
        /// # Return
        ///  mint_fee  mint_fee
        #[ink(message)]
        pub fn mint_fee(&self) -> Balance {
            self.mint_fee
        }
        /// Get platform_fee
        /// # Return
        ///  platform_fee  platform_fee
        #[ink(message)]
        pub fn platform_fee(&self) -> Balance {
            self.platform_fee
        }
        /// Get fee_recipient
        /// # Return
        ///  fee_recipient  fee_recipient
        #[ink(message)]
        pub fn fee_recipient(&self) -> AccountId {
            self.fee_recipient
        }
        /// Get code_hash
        /// # Return
        ///  code_hash  code_hash
        #[ink(message)]
        pub fn code_hash(&self) -> Hash {
            self.code_hash
        }
        /// Get endowment_amount
        /// # Return
        ///  endowment_amount  endowment_amount
        #[ink(message)]
        pub fn endowment_amount(&self) -> Balance {
            self.endowment_amount
        }
        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn supports_interface_check(&self, callee: AccountId, data: [u8; 4]) -> bool {
            #[cfg(test)]
            {
                self.test_support_interface == data
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0xE6, 0x11, 0x3A, 0x8A]; //supports_interface_check
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(callee)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(data))
                    .returns::<bool>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("supports_interface_check= {:?}", e);
                        Error::TransactionFailed
                    })
                    .unwrap_or(false)
            }
        }
    }

    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_lang as ink;
        type Event = <sub_art_tradable_private::SubArtTradablePrivate as ::ink_lang::reflect::ContractEventBase>::Type;
        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<ink_env::DefaultEnvironment>(sender);
        }
        fn default_accounts() -> ink_env::test::DefaultAccounts<Environment> {
            ink_env::test::default_accounts::<Environment>()
        }
        fn set_balance(account_id: AccountId, balance: Balance) {
            ink_env::test::set_account_balance::<ink_env::DefaultEnvironment>(account_id, balance)
        }

        fn get_balance(account_id: AccountId) -> Balance {
            ink_env::test::get_account_balance::<ink_env::DefaultEnvironment>(account_id)
                .expect("Cannot get account balance")
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
        fn django() -> AccountId {
            default_accounts().django
        }

        fn eve() -> AccountId {
            default_accounts().eve
        }

        fn frank() -> AccountId {
            default_accounts().frank
        }
        fn fee_recipient() -> AccountId {
            default_accounts().django
        }
        fn init_contract() -> SubArtFactoryPrivate {
            let erc = SubArtFactoryPrivate::new(
                frank(),
                eve(),
                1,
                10,
                fee_recipient(),
                Hash::from([0x99; 32]),
            );

            erc
        }
        #[ink::test]
        fn update_marketplace_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            let marketplace = bob();
            assert!(art_factory.update_marketplace(marketplace).is_ok());

            assert_eq!(art_factory.marketplace, marketplace);
        }

        #[ink::test]
        fn update_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = charlie();
            set_caller(caller);
            let marketplace = bob();
            assert_eq!(
                art_factory.update_marketplace(marketplace).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_bundle_marketplace_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_marketplace = bob();
            assert!(art_factory
                .update_bundle_marketplace(bundle_marketplace)
                .is_ok());

            assert_eq!(art_factory.bundle_marketplace, bundle_marketplace);
        }

        #[ink::test]
        fn update_bundle_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = charlie();
            set_caller(caller);
            let bundle_marketplace = bob();
            assert_eq!(
                art_factory
                    .update_bundle_marketplace(bundle_marketplace)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_mint_fee_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            let mint_fee = 10;
            assert!(art_factory.update_mint_fee(mint_fee).is_ok());

            assert_eq!(art_factory.mint_fee, mint_fee);
        }

        #[ink::test]
        fn update_mint_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = charlie();
            set_caller(caller);
            let mint_fee = 10;
            assert_eq!(
                art_factory.update_mint_fee(mint_fee).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            let platform_fee = 10;
            assert!(art_factory.update_platform_fee(platform_fee).is_ok());

            assert_eq!(art_factory.platform_fee, platform_fee);
        }

        #[ink::test]
        fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = bob();
            set_caller(caller);
            let platform_fee = 10;
            assert_eq!(
                art_factory.update_platform_fee(platform_fee).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_recipient_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            let fee_recipient = bob();
            assert!(art_factory
                .update_platform_fee_recipient(fee_recipient)
                .is_ok());

            assert_eq!(art_factory.fee_recipient, fee_recipient);
        }

        #[ink::test]
        fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = bob();
            set_caller(caller);
            let fee_recipient = bob();
            assert_eq!(
                art_factory
                    .update_platform_fee_recipient(fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn create_nft_contract_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            let fee = 10;
            set_caller(caller);
            set_balance(caller, fee);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(fee);

            let contract_addr =
                art_factory.create_nft_contract(String::from("test"), String::from("TEST"));
            // assert_eq!(contract_addr.unwrap_err(),Error::TransferOwnershipFailed);
            assert!(contract_addr.is_ok());

            // // Token 1 does not exists.
            assert_eq!(art_factory.exists.get(&contract_addr.unwrap()), Some(true));
            assert_eq!(get_balance(fee_recipient()), fee);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_contract_created_event(&emitted_events[0], caller, contract_addr.unwrap());
        }

        #[ink::test]
        fn create_nft_contract_failed_if_the_transferred_value_is_less_than_the_platform_fee() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(1);

            let contract_addr =
                art_factory.create_nft_contract(String::from("test"), String::from("TEST"));
            assert_eq!(contract_addr.unwrap_err(), Error::InsufficientFunds);
        }

        #[ink::test]
        fn create_nft_contract_failed_if_instantiate_contract_failed() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            art_factory.test_instantiate_contract_failed = true;
            let contract_addr =
                art_factory.create_nft_contract(String::from("test"), String::from("TEST"));
            assert_eq!(contract_addr.unwrap_err(), Error::TransferOwnershipFailed);
        }

        #[ink::test]
        fn register_token_contract_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            art_factory.test_support_interface = crate::INTERFACE_ID_ERC1155;

            // assert_eq!(art_factory
            //     .register_token_contract(
            //       token_contract,
            //     ).unwrap_err(),Error::TransferOwnershipFailed);
            assert!(art_factory.register_token_contract(token_contract,).is_ok());

            // // Token 1 does not exists.
            assert_eq!(art_factory.exists.get(&token_contract), Some(true));
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_contract_created_event(&emitted_events[0], caller, token_contract);
        }

        #[ink::test]
        fn register_token_contract_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = charlie();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            assert_eq!(
                art_factory
                    .register_token_contract(token_contract,)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn register_token_contract_failed_if_the_token_contract_exists_in_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            art_factory.exists.insert(&token_contract, &true);
            assert_eq!(
                art_factory
                    .register_token_contract(token_contract,)
                    .unwrap_err(),
                Error::ArtContractAlreadyRegistered
            );
        }

        #[ink::test]
        fn register_token_contract_failed_if_the_artion_does_not_support_erc_1155() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            assert_eq!(
                art_factory
                    .register_token_contract(token_contract,)
                    .unwrap_err(),
                Error::NotAnERC1155Contract
            );
        }

        #[ink::test]
        fn disable_token_contract_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            art_factory.exists.insert(&token_contract, &true);
            // assert_eq!(contract_addr.unwrap_err(),Error::TransferOwnershipFailed);
            assert!(art_factory.disable_token_contract(token_contract,).is_ok());

            // // Token 1 does not exists.
            assert_eq!(art_factory.exists.get(&token_contract), Some(false));
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_contract_disabled_event(&emitted_events[0], caller, token_contract);
        }

        #[ink::test]
        fn disable_token_contract_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = charlie();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            art_factory.exists.insert(&token_contract, &true);
            assert_eq!(
                art_factory
                    .disable_token_contract(token_contract,)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn disable_token_contract_failed_if_the_token_contract_does_not_exist_in_the_contract() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = django();
            assert_eq!(
                art_factory
                    .disable_token_contract(token_contract,)
                    .unwrap_err(),
                Error::ArtContractIsNotRegistered
            );
        }

        #[ink::test]
        fn exists_contract_works() {
            // Create a new contract instance.
            let mut art_factory = init_contract();
            let caller = alice();
            set_caller(caller);
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            let token_contract = charlie();
            art_factory.exists.insert(&token_contract, &true);

            // // Token 1 does not exists.
            assert!(art_factory.exists(token_contract));
        }

        fn assert_contract_created_event(
            event: &ink_env::test::EmittedEvent,
            expected_creator: AccountId,
            expected_nft_address: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ContractCreated(ContractCreated {
                creator,
                nft_address,
            }) = decoded_event
            {
                assert_eq!(
                    creator, expected_creator,
                    "encountered invalid ContractCreated.creator"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid ContractCreated.nft_address"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ContractCreated event")
            }
        }

        fn assert_contract_disabled_event(
            event: &ink_env::test::EmittedEvent,
            expected_caller: AccountId,
            expected_nft_address: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ContractDisabled(ContractDisabled {
                caller,
                nft_address,
            }) = decoded_event
            {
                assert_eq!(
                    caller, expected_caller,
                    "encountered invalid ContractDisabled.caller"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid ContractDisabled.nft_address"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ContractDisabled event")
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
