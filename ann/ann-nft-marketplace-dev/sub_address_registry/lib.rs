//! # Address registry Contract
//!
//! This is an Address registry implementation.
//!

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_address_registry::{SubAddressRegistry, SubAddressRegistryRef};

use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
pub mod sub_address_registry {
    #[cfg_attr(test, allow(dead_code))]
    const INTERFACE_ID_ERC721: [u8; 4] = [0x80, 0xAC, 0x58, 0xCD];
    pub type TokenId = u128;

    use ink_storage::traits::SpreadAllocate;
    use scale::{Decode, Encode};

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubAddressRegistry {
        ///  Artion contract
        artion: AccountId,
        ///  Auction contract
        auction: AccountId,
        ///  Marketplace contract
        marketplace: AccountId,
        ///  BundleMarketplace contract
        bundle_marketplace: AccountId,
        ///  NFTFactory contract
        factory: AccountId,
        ///  NFTFactoryPrivate contract
        private_factory: AccountId,
        ///  ArtFactory contract
        art_factory: AccountId,
        ///  ArtFactoryPrivate contract
        private_art_factory: AccountId,
        ///  TokenRegistry contract
        token_registry: AccountId,
        ///  PriceFeed contract
        price_seed: AccountId,
        /// The contract owner
        owner: AccountId,
        test_support_interface: [u8; 4],
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        NotERC721,
        TransactionFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubAddressRegistry result types.
    pub type Result<T> = core::result::Result<T, Error>;

    impl SubAddressRegistry {
        /// Creates a new Address registry contract.
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
        /// Update artion contract address
        /// Only admin
        /// # Fields
        /// artion  new artion contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        /// - If  the artion does not  support ERC-721 .     
        #[ink(message)]
        pub fn update_artion(&mut self, artion: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.supports_interface_check(artion, INTERFACE_ID_ERC721),
                Error::NotERC721
            );
            self.artion = artion;

            Ok(())
        }

        /// Update auction contract address
        /// Only admin
        /// # Fields
        /// auction  new auction contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_auction(&mut self, auction: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.auction = auction;
            Ok(())
        }

        /// Update Marketplace contract address
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

        /// Update BundleMarketplace contract address
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

        /// Update nft factory contract address
        /// Only admin
        /// # Fields
        /// factory  new factory contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_nft_factory(&mut self, factory: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.factory = factory;
            Ok(())
        }

        /// Update NFT factory private contract address
        /// Only admin
        /// # Fields
        /// private_factory  new  NFTfactory private  contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_nft_factory_private(&mut self, private_factory: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.private_factory = private_factory;
            Ok(())
        }

        /// Update Art factory  contractaddress
        /// Only admin
        /// # Fields
        /// art_factory  new  Art factory   contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_art_factory(&mut self, art_factory: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.art_factory = art_factory;
            Ok(())
        }

        /// Update Art factory private contract address
        /// Only admin
        /// # Fields
        /// private_art_factory  new  Art factory private  contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_art_factory_private(&mut self, private_art_factory: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.private_art_factory = private_art_factory;
            Ok(())
        }

        /// Update token registry contract address
        /// Only admin
        /// # Fields
        /// token_registry  new  token registry contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_token_registry(&mut self, token_registry: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.token_registry = token_registry;
            Ok(())
        }

        /// Update price seed contract address
        /// Only admin
        /// # Fields
        /// price_seed  new  price seed contract  address
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_price_seed(&mut self, price_seed: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.price_seed = price_seed;
            Ok(())
        }

        /// Querying artion contract address
        /// # return artion contract address
        #[ink(message)]
        pub fn artion(&self) -> AccountId {
            self.artion
        }
        /// Querying auction contract address
        /// # return auction contract address
        #[ink(message)]
        pub fn auction(&self) -> AccountId {
            self.auction
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

        /// Querying nft factory contract address
        /// # return nft factory contract address
        #[ink(message)]
        pub fn nft_factory(&self) -> AccountId {
            self.factory
        }

        /// Querying nft factory private contract address
        /// # return nft factory private contract address
        #[ink(message)]
        pub fn nft_factory_private(&self) -> AccountId {
            self.private_factory
        }

        /// Querying art factory contract address
        /// # return art factory contract address
        #[ink(message)]
        pub fn art_factory(&self) -> AccountId {
            self.art_factory
        }

        /// Querying art factory private contract address
        /// # return art factory private contract address
        #[ink(message)]
        pub fn art_factory_private(&self) -> AccountId {
            self.private_art_factory
        }

        /// Querying token registry contract address
        /// # return token registry contract address
        #[ink(message)]
        pub fn token_registry(&self) -> AccountId {
            self.token_registry
        }
        /// Querying price seed contract address
        /// # return price seed contract address
        #[ink(message)]
        pub fn price_seed(&self) -> AccountId {
            self.price_seed
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
        fn django() -> AccountId {
            default_accounts().django
        }

        fn eve() -> AccountId {
            default_accounts().eve
        }

        fn frank() -> AccountId {
            default_accounts().frank
        }
        fn init_contract() -> SubAddressRegistry {
            let erc = SubAddressRegistry::new();

            erc
        }
        #[ink::test]
        fn update_artion_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let artion = bob();
            address_registry.test_support_interface = INTERFACE_ID_ERC721;

            assert!(address_registry.update_artion(artion).is_ok());

            assert_eq!(address_registry.artion, artion);
        }

        #[ink::test]
        fn update_artion_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = frank();
            set_caller(caller);
            let artion = bob();
            assert_eq!(
                address_registry.update_artion(artion).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_artion_failed_if_the_artion_does_not_support_erc721() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let artion = bob();
            address_registry.test_support_interface = [0x0; 4];
            assert_eq!(
                address_registry.update_artion(artion).unwrap_err(),
                Error::NotERC721
            );
        }

        #[ink::test]
        fn update_auction_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let auction = bob();
            assert!(address_registry.update_auction(auction).is_ok());

            assert_eq!(address_registry.auction, auction);
        }

        #[ink::test]
        fn update_auction_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = eve();
            set_caller(caller);
            let auction = bob();
            assert_eq!(
                address_registry.update_auction(auction).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_marketplace_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let marketplace = bob();
            assert!(address_registry.update_marketplace(marketplace).is_ok());

            assert_eq!(address_registry.marketplace, marketplace);
        }

        #[ink::test]
        fn update_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = django();
            set_caller(caller);
            let marketplace = bob();
            assert_eq!(
                address_registry
                    .update_marketplace(marketplace)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_bundle_marketplace_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_marketplace = bob();
            assert!(address_registry
                .update_bundle_marketplace(bundle_marketplace)
                .is_ok());

            assert_eq!(address_registry.bundle_marketplace, bundle_marketplace);
        }

        #[ink::test]
        fn update_bundle_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = charlie();
            set_caller(caller);
            let bundle_marketplace = bob();
            assert_eq!(
                address_registry
                    .update_bundle_marketplace(bundle_marketplace)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_nft_factory_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let factory = bob();
            assert!(address_registry.update_nft_factory(factory).is_ok());

            assert_eq!(address_registry.factory, factory);
        }

        #[ink::test]
        fn update_nft_factory_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = frank();
            set_caller(caller);
            let factory = bob();
            assert_eq!(
                address_registry.update_nft_factory(factory).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_nft_factory_private_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let private_factory = bob();
            assert!(address_registry
                .update_nft_factory_private(private_factory)
                .is_ok());

            assert_eq!(address_registry.private_factory, private_factory);
        }

        #[ink::test]
        fn update_nft_factory_private_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = eve();
            set_caller(caller);
            let private_factory = bob();
            assert_eq!(
                address_registry
                    .update_nft_factory_private(private_factory)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_art_factory_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let art_factory = bob();
            assert!(address_registry.update_art_factory(art_factory).is_ok());

            assert_eq!(address_registry.art_factory, art_factory);
        }

        #[ink::test]
        fn update_art_factory_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = eve();
            set_caller(caller);
            let art_factory = bob();
            assert_eq!(
                address_registry
                    .update_art_factory(art_factory)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_art_factory_private_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let private_art_factory = bob();
            assert!(address_registry
                .update_art_factory_private(private_art_factory)
                .is_ok());

            assert_eq!(address_registry.private_art_factory, private_art_factory);
        }

        #[ink::test]
        fn update_art_factory_private_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = eve();
            set_caller(caller);
            let private_art_factory = bob();

            assert_eq!(
                address_registry
                    .update_art_factory_private(private_art_factory)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_token_registry_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let token_registry = bob();
            assert!(address_registry
                .update_token_registry(token_registry)
                .is_ok());

            assert_eq!(address_registry.token_registry, token_registry);
        }

        #[ink::test]
        fn update_token_registry_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = eve();
            set_caller(caller);
            let token_registry = bob();

            assert_eq!(
                address_registry
                    .update_token_registry(token_registry)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_price_seed_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let price_seed = bob();
            assert!(address_registry.update_price_seed(price_seed).is_ok());

            assert_eq!(address_registry.price_seed, price_seed);
        }

        #[ink::test]
        fn update_price_seed_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = eve();
            set_caller(caller);
            let price_seed = bob();
            assert_eq!(
                address_registry.update_price_seed(price_seed).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn artion_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let artion = bob();
            address_registry.artion = artion;

            assert_eq!(address_registry.artion(), artion);
        }

        #[ink::test]
        fn auction_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let auction = bob();
            address_registry.auction = auction;

            assert_eq!(address_registry.auction(), auction);
        }
        #[ink::test]
        fn marketplace_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let marketplace = bob();
            address_registry.marketplace = marketplace;

            assert_eq!(address_registry.marketplace(), marketplace);
        }

        #[ink::test]
        fn bundle_marketplace_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_marketplace = bob();
            address_registry.bundle_marketplace = bundle_marketplace;

            assert_eq!(address_registry.bundle_marketplace(), bundle_marketplace);
        }

        #[ink::test]
        fn nft_factory_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let factory = bob();
            address_registry.factory = factory;

            assert_eq!(address_registry.nft_factory(), factory);
        }

        #[ink::test]
        fn nft_factory_private_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let private_factory = bob();
            address_registry.private_factory = private_factory;

            assert_eq!(address_registry.nft_factory_private(), private_factory);
        }

        #[ink::test]
        fn art_factory_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let art_factory = bob();
            address_registry.art_factory = art_factory;

            assert_eq!(address_registry.art_factory(), art_factory);
        }

        #[ink::test]
        fn art_factory_private_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let private_art_factory = bob();
            address_registry.private_art_factory = private_art_factory;

            assert_eq!(address_registry.art_factory_private(), private_art_factory);
        }

        #[ink::test]
        fn token_registry_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let token_registry = bob();
            address_registry.token_registry = token_registry;

            assert_eq!(address_registry.token_registry(), token_registry);
        }

        #[ink::test]
        fn price_seed_works() {
            // Create a new contract instance.
            let mut address_registry = init_contract();
            let caller = alice();
            set_caller(caller);
            let price_seed = bob();
            address_registry.price_seed = price_seed;

            assert_eq!(address_registry.price_seed(), price_seed);
        }
    }
}
