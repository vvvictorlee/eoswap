//! # Contract Management Contract
//!
//! This is a Contract Management implementation.
//!

#![cfg_attr(not(feature = "std"), no_std)]

use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
pub mod sub_contract_management {
    #[cfg_attr(test, allow(dead_code))]
    use ink_storage::{traits::SpreadAllocate, Mapping};
    use scale::{Decode, Encode};
    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubContractManagement {
        /// Mapping Hash to  Address
        hash_address: Mapping<Hash, AccountId>,
        address_registry: AccountId,
        auction: AccountId,
        marketplace: AccountId,
        bundle_marketplace: AccountId,
        art_tradable_hash: Hash,
        art_tradable_private_hash: Hash,
        nft_tradable_hash: Hash,
        nft_tradable_private_hash: Hash,
        /// erc20 initial_supply
        initial_supply: Balance,
        wrapped_token: AccountId,
        /// NFT mint fee
        mint_fee: Balance,
        /// Platform fee
        platform_fee: Balance,
        token: AccountId,
        decimals: u32,
        /// Platform fee receipient
        fee_recipient: AccountId,
        endowment_amount: Balance,
        version: u32,
        /// The contract owner
        owner: AccountId,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        TransactionFailed,
        TransferOwnershipFailed,
        UpdateFailed,
        GetAuctionFailed,
        GetMarketplaceFailed,
        GetBundleMarketplaceFailed,
        GetFeeRecipientFailed,
        GetArtTradableFailed,
        GetArtTradablePrivateFailed,
        GetNFTTradableFailed,
        GetNFTTradablePrivateFailed,
        GetWrappedTokenFailed,
        GeAddressRegistryFailed,
        GetInitialSupplyFailed,
        GetTokenFailed,
        GetDecimalsFailed,
    }

    // The SubContractManagement result types.
    pub type Result<T> = core::result::Result<T, Error>;

    impl SubContractManagement {
        /// Creates a new Address registry contract.
        #[ink(constructor)]
        pub fn new() -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.mint_fee = 10;
                contract.platform_fee = 10;
                contract.fee_recipient = Self::env().caller();
                contract.initial_supply = 1000_000;
                contract.version = 1;
                contract.decimals = 12;
            })
        }
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_address_registry_contract(
            &mut self,
            code_hash: Hash,
        ) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);

            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    let salt = self.version.to_le_bytes();
                    let instance_params = sub_address_registry::SubAddressRegistryRef::new()
                        .endowment(self.endowment_amount)
                        .code_hash(code_hash)
                        .salt_bytes(salt)
                        .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr = init_result
                        .expect("failed at instantiating the `address_registry` contract");

                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.address_registry = ans_contract_addr;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        ///  Method for deploying new SubArtTradablePrivate contract
        /// # Fields
        ///  name Name of NFT contract
        ///  symbol Symbol of NFT contract
        ///
        /// # Errors
        ///
        /// - If the transferred value is less than the `platform_fee`.
        /// - If it failed when the contract trasfer to  fee_recipient in native token.
        /// - If `instantiate_contract` failed .
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_art_factory_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.marketplace != AccountId::from([0x0; 32]),
                Error::GetMarketplaceFailed
            );
            ensure!(
                self.bundle_marketplace != AccountId::from([0x0; 32]),
                Error::GetBundleMarketplaceFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );
            ensure!(
                self.art_tradable_hash != Hash::default(),
                Error::GetArtTradableFailed
            );
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_art_factory::SubArtFactoryRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubArtFactoryRef::new(
                        self.marketplace,
                        self.bundle_marketplace,
                        self.mint_fee,
                        self.platform_fee,
                        self.fee_recipient,
                        self.art_tradable_hash,
                    )
                    .endowment(self.endowment_amount)
                    .code_hash(code_hash)
                    .salt_bytes(salt)
                    .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `art factory ` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_art_factory(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_art_factory_instance: SubArtFactoryRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_art_factory_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );

                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_art_factory_private_contract(
            &mut self,
            code_hash: Hash,
        ) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.marketplace != AccountId::from([0x0; 32]),
                Error::GetMarketplaceFailed
            );
            ensure!(
                self.bundle_marketplace != AccountId::from([0x0; 32]),
                Error::GetBundleMarketplaceFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );
            ensure!(
                self.art_tradable_private_hash != Hash::default(),
                Error::GetArtTradablePrivateFailed
            );
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_art_factory_private::SubArtFactoryPrivateRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubArtFactoryPrivateRef::new(
                        self.marketplace,
                        self.bundle_marketplace,
                        self.mint_fee,
                        self.platform_fee,
                        self.fee_recipient,
                        self.art_tradable_private_hash,
                    )
                    .endowment(self.endowment_amount)
                    .code_hash(code_hash)
                    .salt_bytes(salt)
                    .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr = init_result
                        .expect("failed at instantiating the `art factory private` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_art_factory_private(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_art_factory_private_instance: SubArtFactoryPrivateRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_art_factory_private_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;

            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_artion_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );

            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_artion::SubArtionRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubArtionRef::new(self.platform_fee, self.fee_recipient)
                        .endowment(self.endowment_amount)
                        .code_hash(code_hash)
                        .salt_bytes(salt)
                        .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `artion` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_artion(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_artion_instance: SubArtionRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_artion_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;

            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_auction_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );

            let address_registry = self.address_registry;

            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_auction::SubAuctionRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubAuctionRef::new(self.platform_fee, self.fee_recipient)
                        .endowment(self.endowment_amount)
                        .code_hash(code_hash)
                        .salt_bytes(salt)
                        .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `auction` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_auction(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_auction_instance: SubAuctionRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_auction_instance
                            .update_address_registry(address_registry)
                            .is_ok(),
                        Error::UpdateFailed
                    );
                    ensure!(
                        sub_auction_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.auction = ans_contract_addr;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_bundle_marketplace_contract(
            &mut self,
            code_hash: Hash,
        ) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );

            let address_registry = self.address_registry;

            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_bundle_marketplace::SubBundleMarketplaceRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params =
                        SubBundleMarketplaceRef::new(self.platform_fee, self.fee_recipient)
                            .endowment(self.endowment_amount)
                            .code_hash(code_hash)
                            .salt_bytes(salt)
                            .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr = init_result
                        .expect("failed at instantiating the `bundle_marketplace` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_bundle_marketplace(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_bundle_marketplace_instance: SubBundleMarketplaceRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_bundle_marketplace_instance
                            .update_address_registry(address_registry)
                            .is_ok(),
                        Error::UpdateFailed
                    );
                    ensure!(
                        sub_bundle_marketplace_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.bundle_marketplace = ans_contract_addr;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_marketplace_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );

            let address_registry = self.address_registry;
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_marketplace::SubMarketplaceRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params =
                        SubMarketplaceRef::new(self.platform_fee, self.fee_recipient)
                            .endowment(self.endowment_amount)
                            .code_hash(code_hash)
                            .salt_bytes(salt)
                            .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `marketplace` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_marketplace(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_marketplace_instance: SubMarketplaceRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_marketplace_instance
                            .update_address_registry(address_registry)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    ensure!(
                        sub_marketplace_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.marketplace = ans_contract_addr;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_nft_factory_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.auction != AccountId::from([0x0; 32]),
                Error::GetAuctionFailed
            );
            ensure!(
                self.marketplace != AccountId::from([0x0; 32]),
                Error::GetMarketplaceFailed
            );
            ensure!(
                self.bundle_marketplace != AccountId::from([0x0; 32]),
                Error::GetBundleMarketplaceFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );
            ensure!(
                self.nft_tradable_hash != Hash::default(),
                Error::GetNFTTradableFailed
            );
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_nft_factory::SubNFTFactoryRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubNFTFactoryRef::new(
                        self.auction,
                        self.marketplace,
                        self.bundle_marketplace,
                        self.mint_fee,
                        self.platform_fee,
                        self.fee_recipient,
                        self.nft_tradable_hash,
                    )
                    .endowment(self.endowment_amount)
                    .code_hash(code_hash)
                    .salt_bytes(salt)
                    .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `nft factory ` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_nft_factory(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_nft_factory_instance: SubNFTFactoryRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_nft_factory_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;

            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_nft_factory_private_contract(
            &mut self,
            code_hash: Hash,
        ) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.auction != AccountId::from([0x0; 32]),
                Error::GetAuctionFailed
            );
            ensure!(
                self.marketplace != AccountId::from([0x0; 32]),
                Error::GetMarketplaceFailed
            );
            ensure!(
                self.bundle_marketplace != AccountId::from([0x0; 32]),
                Error::GetBundleMarketplaceFailed
            );
            ensure!(
                self.fee_recipient != AccountId::from([0x0; 32]),
                Error::GetFeeRecipientFailed
            );
            ensure!(
                self.nft_tradable_private_hash != Hash::default(),
                Error::GetNFTTradablePrivateFailed
            );
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_nft_factory_private::SubNFTFactoryPrivateRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubNFTFactoryPrivateRef::new(
                        self.auction,
                        self.marketplace,
                        self.bundle_marketplace,
                        self.mint_fee,
                        self.platform_fee,
                        self.fee_recipient,
                        self.nft_tradable_private_hash,
                    )
                    .endowment(self.endowment_amount)
                    .code_hash(code_hash)
                    .salt_bytes(salt)
                    .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr = init_result
                        .expect("failed at instantiating the `nft factory private` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_nft_factory_private(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_nft_factory_private_instance: SubNFTFactoryPrivateRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);
                    ensure!(
                        sub_nft_factory_private_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_price_seed_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            ensure!(
                self.wrapped_token != AccountId::from([0x0; 32]),
                Error::GetWrappedTokenFailed
            );

            let address_registry = self.address_registry;

            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_price_seed::SubPriceSeedRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params =
                        SubPriceSeedRef::new(address_registry, self.wrapped_token)
                            .endowment(self.endowment_amount)
                            .code_hash(code_hash)
                            .salt_bytes(salt)
                            .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `price_seed` contract");

                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_price_seed(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_price_seed_instance: SubPriceSeedRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);

                    ensure!(
                        sub_price_seed_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );

                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;

            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_token_registry_contract(
            &mut self,
            code_hash: Hash,
        ) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.address_registry != AccountId::from([0x0; 32]),
                Error::GeAddressRegistryFailed
            );
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_token_registry::SubTokenRegistryRef;

                    let salt = self.version.to_le_bytes();
                    let instance_params = SubTokenRegistryRef::new()
                        .endowment(self.endowment_amount)
                        .code_hash(code_hash)
                        .salt_bytes(salt)
                        .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `token_registry` contract");
                    let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    ensure!(
                        sub_address_registry_instance
                            .update_token_registry(contract_addr)
                            .is_ok(),
                        Error::UpdateFailed
                    );

                    let mut sub_token_registry_instance: SubTokenRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);

                    ensure!(
                        sub_token_registry_instance.add(self.token).is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    ensure!(
                        sub_token_registry_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );

                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;

            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }

        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_erc20_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(self.initial_supply > 0, Error::GetInitialSupplyFailed);
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use erc20::Erc20Ref;
                    let salt = self.version.to_le_bytes();
                    let instance_params = Erc20Ref::new(self.owner, self.initial_supply)
                        .endowment(self.endowment_amount)
                        .code_hash(code_hash)
                        .salt_bytes(salt)
                        .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `erc20` contract");

                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.token = ans_contract_addr;
            self.wrapped_token = ans_contract_addr;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_oracle_contract(&mut self, code_hash: Hash) -> Result<AccountId> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                self.token != AccountId::from([0x0; 32]),
                Error::GetTokenFailed
            );
            ensure!(self.decimals > 0, Error::GetDecimalsFailed);
            let instantiate_contract = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    use sub_oracle::SubOracleRef;
                    let salt = self.version.to_le_bytes();
                    let instance_params = SubOracleRef::new(self.token, self.decimals)
                        .endowment(self.endowment_amount)
                        .code_hash(code_hash)
                        .salt_bytes(salt)
                        .params();
                    let init_result = ink_env::instantiate_contract(&instance_params);
                    let contract_addr =
                        init_result.expect("failed at instantiating the `oracle` contract");
                    let mut sub_oracle_instance: SubOracleRef =
                        ink_env::call::FromAccountId::from_account_id(contract_addr);

                    ensure!(
                        sub_oracle_instance
                            .transfer_ownership(self.env().caller())
                            .is_ok(),
                        Error::TransferOwnershipFailed
                    );
                    Ok(contract_addr)
                }
            };
            let ans_contract_addr = instantiate_contract()?;
            self.hash_address.insert(&code_hash, &ans_contract_addr);
            Ok(ans_contract_addr)
        }
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn transfer_ownership_of_address_registry(&mut self) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);

            #[cfg(not(test))]
            {
                let mut sub_address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);
                ensure!(
                    sub_address_registry_instance
                        .transfer_ownership(self.env().caller())
                        .is_ok(),
                    Error::TransferOwnershipFailed
                );
            }

            Ok(())
        }

        #[ink(message)]
        pub fn update_address_registry(&mut self, address_registry: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.address_registry = address_registry;

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
        /// Method for updating wrapped_token
        /// Only admin
        /// # Fields
        /// wrapped_token wrapped_token
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_wrapped_token(&mut self, wrapped_token: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.wrapped_token = wrapped_token;
            Ok(())
        }
        /// Method for updating oracle contract  token
        /// Only admin
        /// # Fields
        /// token the token to set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_token(&mut self, token: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.token = token;
            Ok(())
        }

        /// Method for updating art_tradable_hash
        /// Only admin
        /// # Fields
        /// art_tradable_hash art_tradable_hash
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_art_tradable_hash(&mut self, art_tradable_hash: Hash) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.art_tradable_hash = art_tradable_hash;
            Ok(())
        }

        /// Method for updating art_tradable_private_hash
        /// Only admin
        /// # Fields
        /// art_tradable_private_hash art_tradable_private_hash
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_art_tradable_private_hash(
            &mut self,
            art_tradable_private_hash: Hash,
        ) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.art_tradable_private_hash = art_tradable_private_hash;
            Ok(())
        }

        /// Method for updating nft_tradable_hash
        /// Only admin
        /// # Fields
        /// nft_tradable_hash nft_tradable_hash
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_nft_tradable_hash(&mut self, nft_tradable_hash: Hash) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.nft_tradable_hash = nft_tradable_hash;
            Ok(())
        }

        /// Method for updating nft_tradable_private_hash
        /// Only admin
        /// # Fields
        /// nft_tradable_private_hash nft_tradable_private_hash
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .     
        #[ink(message)]
        pub fn update_nft_tradable_private_hash(
            &mut self,
            nft_tradable_private_hash: Hash,
        ) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.nft_tradable_private_hash = nft_tradable_private_hash;
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

        /// Method for updating initial_supply
        /// Only admin
        /// # Fields
        /// initial_supply the initial_supply to set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn update_initial_supply(&mut self, initial_supply: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.initial_supply = initial_supply;
            Ok(())
        }

        /// Method for updating instantiate contract version
        /// Only admin
        /// # Fields
        /// version the version to set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .    
        #[ink(message)]
        pub fn update_version(&mut self, version: u32) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.version = version;
            Ok(())
        }

        /// Method for updating oracle contract  decimals
        /// Only admin
        /// # Fields
        /// decimals the decimals to set
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
        #[ink(message)]
        pub fn update_parameters(
            &mut self,
            art_tradable_hash: Hash,
            art_tradable_private_hash: Hash,
            nft_tradable_hash: Hash,
            nft_tradable_private_hash: Hash,
        ) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.art_tradable_hash = art_tradable_hash;
            self.art_tradable_private_hash = art_tradable_private_hash;
            self.nft_tradable_hash = nft_tradable_hash;
            self.nft_tradable_private_hash = nft_tradable_private_hash;
            Ok(())
        }
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn instantiate_contracts(
            &mut self,
            address_registry_hash: Hash,
            art_factory_hash: Hash,
            art_factory_private_hash: Hash,
            artion_hash: Hash,
            auction_hash: Hash,
            bundle_marketplace_hash: Hash,
            marketplace_hash: Hash,
            nft_factory_hash: Hash,
            nft_factory_private_hash: Hash,
            price_seed_hash: Hash,
            token_registry_hash: Hash,
            erc20_hash: Hash,
            oracle_hash: Hash,
        ) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.instantiate_address_registry_contract(address_registry_hash)?;
            self.instantiate_auction_contract(auction_hash)?;
            self.instantiate_bundle_marketplace_contract(bundle_marketplace_hash)?;
            self.instantiate_marketplace_contract(marketplace_hash)?;
            self.instantiate_art_factory_contract(art_factory_hash)?;
            self.instantiate_art_factory_private_contract(art_factory_private_hash)?;
            self.instantiate_artion_contract(artion_hash)?;
            self.instantiate_nft_factory_contract(nft_factory_hash)?;
            self.instantiate_nft_factory_private_contract(nft_factory_private_hash)?;
            self.instantiate_erc20_contract(erc20_hash)?;
            self.instantiate_price_seed_contract(price_seed_hash)?;
            self.instantiate_token_registry_contract(token_registry_hash)?;
            self.instantiate_oracle_contract(oracle_hash)?;
            Ok(())
        }

        /// Querying artion contract address
        /// # return artion contract address
        #[ink(message)]
        pub fn address_registry(&self) -> AccountId {
            self.address_registry
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

        /// Querying art_tradable_hash
        /// # return bundle art_tradable_hash
        #[ink(message)]
        pub fn art_tradable_hash(&self) -> Hash {
            self.art_tradable_hash
        }

        /// Querying art_tradable_private_hash
        /// # return bundle art_tradable_private_hash
        #[ink(message)]
        pub fn art_tradable_private_hash(&self) -> Hash {
            self.art_tradable_private_hash
        }

        /// Querying nft_tradable_hash
        /// # return bundle nft_tradable_hash
        #[ink(message)]
        pub fn nft_tradable_hash(&self) -> Hash {
            self.nft_tradable_hash
        }

        /// Querying nft_tradable_private_hash
        /// # return bundle nft_tradable_private_hash
        #[ink(message)]
        pub fn nft_tradable_private_hash(&self) -> Hash {
            self.nft_tradable_private_hash
        }

        /// Querying wrapped_token contract address
        /// # return wrapped_token contract address
        #[ink(message)]
        pub fn wrapped_token(&self) -> AccountId {
            self.wrapped_token
        }

        #[ink(message)]
        pub fn initial_supply(&self) -> Balance {
            self.initial_supply
        }
        #[ink(message)]
        pub fn mint_fee(&self) -> Balance {
            self.mint_fee
        }
        #[ink(message)]
        pub fn platform_fee(&self) -> Balance {
            self.platform_fee
        }
        #[ink(message)]
        pub fn fee_recipient(&self) -> AccountId {
            self.fee_recipient
        }
        #[ink(message)]
        pub fn address_by_code_hash(&self, code_hash: Hash) -> AccountId {
            self.hash_address.get(&code_hash).unwrap_or_default()
        }
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
        #[ink(message)]
        pub fn token(&self) -> AccountId {
            self.token
        }
        #[ink(message)]
        pub fn decimals(&self) -> u32 {
            self.decimals
        }
        #[ink(message)]
        pub fn version(&self) -> u32 {
            self.version
        }
        #[ink(message)]
        pub fn endowment_amount(&self) -> Balance {
            self.endowment_amount
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

        fn init_contract() -> SubContractManagement {
            let erc = SubContractManagement::new();

            erc
        }
        #[ink::test]
        fn update_auction_works() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = alice();
            set_caller(caller);
            let auction = bob();
            assert!(contract_management.update_auction(auction).is_ok());

            assert_eq!(contract_management.auction, auction);
        }

        #[ink::test]
        fn update_auction_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = charlie();
            set_caller(caller);
            let auction = bob();
            assert_eq!(
                contract_management.update_auction(auction).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_marketplace_works() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = alice();
            set_caller(caller);
            let marketplace = bob();
            assert!(contract_management.update_marketplace(marketplace).is_ok());

            assert_eq!(contract_management.marketplace, marketplace);
        }

        #[ink::test]
        fn update_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = charlie();
            set_caller(caller);
            let marketplace = bob();
            assert_eq!(
                contract_management
                    .update_marketplace(marketplace)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_bundle_marketplace_works() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_marketplace = bob();
            assert!(contract_management
                .update_bundle_marketplace(bundle_marketplace)
                .is_ok());

            assert_eq!(contract_management.bundle_marketplace, bundle_marketplace);
        }

        #[ink::test]
        fn update_bundle_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = charlie();
            set_caller(caller);
            let bundle_marketplace = bob();
            assert_eq!(
                contract_management
                    .update_bundle_marketplace(bundle_marketplace)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_mint_fee_works() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = alice();
            set_caller(caller);
            let mint_fee = 10;
            assert!(contract_management.update_mint_fee(mint_fee).is_ok());

            assert_eq!(contract_management.mint_fee, mint_fee);
        }

        #[ink::test]
        fn update_mint_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = charlie();
            set_caller(caller);
            let mint_fee = 10;
            assert_eq!(
                contract_management.update_mint_fee(mint_fee).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_works() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = alice();
            set_caller(caller);
            let platform_fee = 10;
            assert!(contract_management
                .update_platform_fee(platform_fee)
                .is_ok());

            assert_eq!(contract_management.platform_fee, platform_fee);
        }

        #[ink::test]
        fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = bob();
            set_caller(caller);
            let platform_fee = 10;
            assert_eq!(
                contract_management
                    .update_platform_fee(platform_fee)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_recipient_works() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = alice();
            set_caller(caller);
            let fee_recipient = bob();
            assert!(contract_management
                .update_platform_fee_recipient(fee_recipient)
                .is_ok());

            assert_eq!(contract_management.fee_recipient, fee_recipient);
        }
        #[ink::test]
        fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut contract_management = init_contract();
            let caller = bob();
            set_caller(caller);
            let fee_recipient = bob();
            assert_eq!(
                contract_management
                    .update_platform_fee_recipient(fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }
    }
}
