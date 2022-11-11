//! #  Bundle marketplace contract
//!
//! This is an  NFT bundle marketplace implementation.

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_bundle_marketplace::{SubBundleMarketplace, SubBundleMarketplaceRef};

use ink_lang as ink;

#[cfg_attr(test, allow(dead_code))]
const INTERFACE_ID_ERC721: [u8; 4] = [0x80, 0xAC, 0x58, 0xCD];

const INTERFACE_ID_ERC1155: [u8; 4] = [0xD9, 0xB6, 0x7A, 0x26];

/// Evaluate `$x:expr` and if not true return `Err($y:expr)`.   
///
/// Used as `ensure!(expression_to_ensure, expression_to_return_on_false)`.
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
mod sub_bundle_marketplace {
    use ink_prelude::collections::BTreeSet;
    use ink_prelude::string::String;
    use ink_prelude::vec::Vec;
    use ink_storage::{
        traits::{PackedLayout, SpreadAllocate, SpreadLayout},
        Mapping,
    };

    use scale::{Decode, Encode};

    /// A token ID.
    pub type TokenId = u128;

    #[derive(Default, scale::Encode, scale::Decode, SpreadLayout, PackedLayout)]
    #[cfg_attr(
        feature = "std",
        derive(
            Debug,
            PartialEq,
            Eq,
            scale_info::TypeInfo,
            ink_storage::traits::StorageLayout
        )
    )]
    pub struct Listing {
        nft_addresses: Vec<AccountId>,
        token_ids: Vec<TokenId>,
        pub quantities: Vec<u128>,
        pub pay_token: AccountId,
        pub price: Balance,
        pub start_time: u128,
    }

    #[derive(Default, scale::Encode, scale::Decode, SpreadLayout, PackedLayout)]
    #[cfg_attr(
        feature = "std",
        derive(
            Debug,
            PartialEq,
            Eq,
            scale_info::TypeInfo,
            ink_storage::traits::StorageLayout
        )
    )]
    pub struct Offer {
        pub pay_token: AccountId,
        pub price: Balance,
        pub deadline: u128,
    }

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubBundleMarketplace {
        /// Mapping NftAddress , Bundle ID to   Listing item
        listings: Mapping<(AccountId, String), Listing>,
        /// Mapping Bundle ID to  owner
        owners: Mapping<String, AccountId>,
        /// Mapping NftAddress , token ID to  Bundle id list
        bundle_ids_per_item: Mapping<(AccountId, TokenId), BTreeSet<String>>,
        /// Mapping  Bundle id  NftAddress , token ID to array index of the item of the Listing
        nft_indices: Mapping<(String, AccountId, u128), u128>,
        /// Mapping Bundle ID to  Bundle ID hash
        bundle_ids: Mapping<String, String>,
        /// Mapping  Bundle id  NftAddress  to Offer  
        offers: Mapping<(String, AccountId), Offer>,
        ///  Address registry
        address_registry: AccountId,
        ///  Platform fee
        platform_fee: Balance,
        ///  Platform fee receipient
        fee_recipient: AccountId,
        ///  The contract owner
        owner: AccountId,
        /// The tick for test on dev node.
        tick: bool,
        test_token_owner: Mapping<TokenId, AccountId>,
        test_enabled: Mapping<AccountId, bool>,
        test_operator_approvals: Mapping<(AccountId, AccountId), ()>,
        test_transfer_fail: Mapping<(AccountId, AccountId, AccountId, TokenId, Balance), ()>,
        test_contract_id: AccountId,
        test_support_interface: [u8; 4],
        test_balances: Mapping<(AccountId, TokenId), Balance>,
        test_marketplace_validate_item_sold_failed: bool,
        test_auctions: Mapping<(AccountId, TokenId), (u128, bool)>,
        test_exists: Mapping<AccountId, bool>,
        test_marketplace: AccountId,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        InvalidPayToken,
        NotOwningItem,
        InvalidNFTAddress,
        AlreadyListed,
        ItemNotApproved,
        MustHoldEnoughNFTs,
        NotListedItem,
        ItemNotBuyable,
        InsufficientBalanceOrNotApproved,
        OnlyOwner,
        OfferAlreadyCreated,
        CannotPlaceAnOfferIfAuctionIsGoingOn,
        InvalidExpiration,
        OfferNotExistsOrExpired,
        InvalidRoyalty,
        RoyaltyAlreadySet,
        InvalidCreatorAddress,
        SenderMustBeAuctionOrMarketplace,
        InvalidData,
        InvalidId,
        InvalidPrice,
        InsufficientFunds,
        FeeTransferFailed,
        OwnerFeeTransferFailed,
        TransactionFailed,
        ERC20TransferFromFeeAmountFailed,
        ERC20TransferFromRoyaltyFeeFailed,
        ERC20TransferFromCollectionRoyaltyFeeFailed,
        ERC20TransferFromPayAmountFailed,
        ERC721TransferFromTokenIdFailed,
        ERC1155TransferFromTokenIdFailed,
        BundleMarketplaceValidateItemSoldFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubBundleMarketplace result types.
    pub type Result<T> = core::result::Result<T, Error>;
    /// Event emitted when an item listed occurs.
    #[ink(event)]
    pub struct ItemListed {
        #[ink(topic)]
        owner: AccountId,
        bundle_id: String,
        pay_token: AccountId,
        price: Balance,
        start_time: u128,
    }

    /// Event emitted when an item sold occurs.
    #[ink(event)]
    pub struct ItemSold {
        #[ink(topic)]
        seller: AccountId,
        #[ink(topic)]
        buyer: AccountId,
        bundle_id: String,
        pay_token: AccountId,
        unit_price: Balance,
        price: Balance,
    }

    /// Event emitted when an item updated occurs.
    #[ink(event)]
    pub struct ItemUpdated {
        #[ink(topic)]
        owner: AccountId,
        bundle_id: String,
        nft_addresses: Vec<AccountId>,
        token_ids: Vec<TokenId>,
        quantities: Vec<u128>,
        pay_token: AccountId,
        new_price: Balance,
    }

    /// Event emitted when an item canceled occurs.
    #[ink(event)]
    pub struct ItemCanceled {
        #[ink(topic)]
        owner: AccountId,
        bundle_id: String,
    }

    /// Event emitted when an offer created occurs.
    #[ink(event)]
    pub struct OfferCreated {
        #[ink(topic)]
        creator: AccountId,
        bundle_id: String,
        pay_token: AccountId,
        price: Balance,
        deadline: u128,
    }
    /// Event emitted when an offer canceled occurs.
    #[ink(event)]
    pub struct OfferCanceled {
        #[ink(topic)]
        creator: AccountId,
        bundle_id: String,
    }

    /// Event emitted when update platform fee occurs.
    #[ink(event)]
    pub struct UpdatePlatformFee {
        platform_fee: Balance,
    }
    /// Event emitted when update platform fee recipient occurs.
    #[ink(event)]
    pub struct UpdatePlatformFeeRecipient {
        fee_recipient: AccountId,
    }

    impl SubBundleMarketplace {
        /// Creates a new bundle marketplace contract.
        #[ink(constructor)]
        pub fn new(platform_fee: Balance, fee_recipient: AccountId) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.platform_fee = platform_fee;
                contract.fee_recipient = fee_recipient;
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
        ///  Method for listing NFT bundle
        ///  # Fields
        ///  bundle_id Bundle ID
        ///  nft_addresses Addresses of NFT contract
        ///  token_ids Token IDs of NFT
        ///  quantities token amounts to list (needed for ERC-1155 NFTs, set as 1 for ERC-721)
        ///  price sale price for bundle
        ///  start_time scheduling for a future sale
        ///
        /// # Errors
        ///
        /// - If the length of `nft_addresses` is not equal to the length of `token_ids` or the length of `quantities` is not equal to the length of `token_ids` .
        /// - If the owner of the specified bundle id  is not zero address  and the caller is not  the owner of the specified bundle id  
        ///      or the price of the listing of  the specified bundle id is greater than zero.
        /// - If  the `address_registry` is zero address
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        /// - If the `nft_address` support neither ERC-721  nor ERC-1155.
        /// - If the contract is not approved by the caller in the  contract in the following ERC-721 standard.
        /// - If the caller is not the owner of `nft_address` and the `token_id` in the  contract in the following ERC-721 standard.
        /// - If the contract is not approved by the caller in the  contract in the following ERC-1155 standard.
        /// - If the quantity is greater than  the balance  of the caller  in the `nft_address` contract in the following ERC-721 standard.
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn list_item(
            &mut self,
            bundle_id: String,
            nft_addresses: Vec<AccountId>,
            token_ids: Vec<TokenId>,
            quantities: Vec<u128>,
            pay_token: AccountId,
            price: Balance,
            start_time: u128,
        ) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);
            self.bundle_ids.insert(&_bundle_id, &bundle_id);
            ensure!(
                nft_addresses.len() == token_ids.len() && quantities.len() == token_ids.len(),
                Error::InvalidData
            );
            let owner = self.owner_impl(&_bundle_id);

            let mut listing = self.listing_impl(&_bundle_id,self.env().caller());
            ensure!(
                owner == AccountId::from([0x0; 32])
                    || (owner == self.env().caller() && listing.price == 0),
                Error::AlreadyListed
            );

            self.valid_pay_token(pay_token)?;
            listing.nft_addresses.clear();
            listing.token_ids.clear();
            listing.quantities.clear();
            for (i, &nft_address) in nft_addresses.iter().enumerate() {
                let token_id = token_ids[i];
                let quantity = quantities[i];

                if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                    ensure!(
                        Some(self.env().caller()) == self.erc721_owner_of(nft_address, token_id)?,
                        Error::NotOwningItem
                    );
                    ensure!(
                        self.erc721_is_approved_for_all(
                            nft_address,
                            self.env().caller(),
                            self.env().account_id()
                        )
                        .unwrap_or(false),
                        Error::ItemNotApproved
                    );
                    listing.quantities.push(1);
                } else if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155) {
                    ensure!(
                        quantity
                            <= self.erc1155_balance_of(
                                nft_address,
                                self.env().caller(),
                                token_id
                            )?,
                        Error::MustHoldEnoughNFTs
                    );
                    ensure!(
                        self.erc1155_is_approved_for_all(
                            nft_address,
                            self.env().caller(),
                            self.env().account_id()
                        )
                        .unwrap_or(false),
                        Error::ItemNotApproved
                    );
                    listing.quantities.push(quantity);
                } else {
                    ensure!(false, Error::InvalidNFTAddress);
                }
                listing.nft_addresses.push(nft_address);
                listing.token_ids.push(token_id);
                let mut items = self.items_impl(nft_address, token_id);
                items.insert(_bundle_id.clone());
                self.bundle_ids_per_item
                    .insert(&(nft_address, token_id), &items);
                self.nft_indices
                    .insert(&(_bundle_id.clone(), nft_address, token_id), &(i as u128));
            }
            listing.pay_token = pay_token;
            listing.price = price;
            listing.start_time = start_time;
            self.listings
                .insert(&(self.env().caller(), _bundle_id.clone()), &listing);
            self.owners.insert(&_bundle_id, &self.env().caller());

            self.env().emit_event(ItemListed {
                owner: self.env().caller(),
                bundle_id,
                pay_token,
                price,
                start_time,
            });
            Ok(())
        }

        ///  Method for canceling listed NFT bundle
        ///  # Fields
        ///  nft_address Address of NFT contract
        ///
        /// # Errors
        ///
        /// - If the price of the listing of  the specified bundle id is  zero.
        #[ink(message)]
        pub fn cancel_listing(&mut self, bundle_id: String) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);

            let listing = self.listing_impl(&_bundle_id,self.env().caller());
            ensure!(listing.price > 0, Error::NotListedItem);
            self._cancel_listing(self.env().caller(), bundle_id)?;
            Ok(())
        }

        ///  Method for updating listed NFT bundle
        ///  # Fields
        ///  bundle_id Bundle ID
        ///  pay_token payment token
        ///  new_price New sale price for bundle
        ///
        /// # Errors
        ///
        /// - If the price of the listing of  the specified bundle id is  zero.
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        #[ink(message)]
        pub fn update_listing(
            &mut self,
            bundle_id: String,
            pay_token: AccountId,
            new_price: Balance,
        ) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);
            let mut listing = self.listing_impl(&_bundle_id,self.env().caller());
            ensure!(listing.price > 0, Error::NotListedItem);

            self.valid_pay_token(pay_token)?;

            listing.pay_token = pay_token;
            listing.price = new_price;
            self.listings
                .insert(&(self.env().caller(), _bundle_id.clone()), &listing);
            self.env().emit_event(ItemUpdated {
                owner: self.env().caller(),
                bundle_id,
                nft_addresses: listing.nft_addresses,
                token_ids: listing.token_ids,
                quantities: listing.quantities,
                pay_token,
                new_price,
            });
            Ok(())
        }

        ///  Method for buying listed NFT bundle
        ///  # Fields
        ///  bundle_id Bundle ID
        ///  pay_token payment token
        ///
        /// # Errors
        ///
        /// - If the owner of the specified bundle id  is  zero address  .
        /// - If `pay_token` is not the `pay_token` of the listing of `nft_address` and `bundle_id`  .
        /// - If the price of the listing of  the specified bundle id is  zero.
        /// - If the `nft_address` support neither ERC-721  nor ERC-1155.
        /// - If the contract is not approved by the caller in the  contract in the following ERC-721 standard.
        /// - If the caller is not the owner of `nft_address` and the `token_id` in the  contract in the following ERC-721 standard.
        /// - If the quantity is greater than  the balance  of the caller  in the `nft_address` contract in the following ERC-721 standard.
        /// - If the start time  of the listing of  the specified bundle id is  greater than the current time.
        /// - If it failed when the contract transfer platform fee to the `fee_recipient`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer platform fee to the `fee_recipient`  in the native token .
        /// - If it failed when the contract transfer pay amount to the owner in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer pay amount to the owner in the native token .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer  token id  to the caller in the `nft_address` erc721 contract .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer  token id  to the caller  in the `nft_address` erc1155 contract .
        #[ink(message)]
        pub fn buy_item(&mut self, bundle_id: String, pay_token: AccountId) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&&bundle_id);

            let owner = self.owner_impl(&_bundle_id);
            ensure!(owner != AccountId::from([0x0; 32]), Error::InvalidId);

            let listing = self.listing_impl( &_bundle_id,owner);
            ensure!(listing.pay_token == pay_token, Error::InvalidPayToken);

            self._buy_item(bundle_id, pay_token)?;
            Ok(())
        }

        ///  Method for offering bundle item
        ///  # Fields
        ///  bundle_id Bundle ID
        ///  pay_token Paying token
        ///  price Price
        ///  deadline Offer expiration
        ///
        /// # Errors
        ///
        /// - If the owner of the specified bundle id  is  zero address  .
        /// - If `deadline` is less than or equal to the current time .
        /// - If the price of the listing of  the specified bundle id is  zero.
        /// - If `deadline` of the listing of `bundle_id` and the caller is greater than  the current time .
        #[ink(message)]
        pub fn create_offer(
            &mut self,
            bundle_id: String,
            pay_token: AccountId,
            price: Balance,
            deadline: u128,
        ) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);
            let owner = self.owner_impl(&_bundle_id);
            ensure!(AccountId::from([0x0; 32]) != owner, Error::InvalidId);
            ensure!(deadline > self.get_now(), Error::InvalidExpiration);
            ensure!(price > 0, Error::InvalidPrice);
            let offer = self.offer_impl(&_bundle_id, self.env().caller());
            ensure!(offer.deadline <= self.get_now(), Error::OfferAlreadyCreated);

            self.offers.insert(
                &(_bundle_id.clone(), self.env().caller()),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            self.env().emit_event(OfferCreated {
                creator: self.env().caller(),
                bundle_id,
                pay_token,
                price,
                deadline,
            });
            Ok(())
        }
        ///  Method for canceling the bundle offer
        ///  # Fields
        ///  bundle_id Bundle ID
        ///
        /// # Errors
        ///
        /// - If `deadline` of the listing  of `bundle_id` and the caller is less than or equal to  the current time .
        #[ink(message)]
        pub fn cancel_offer(&mut self, bundle_id: String) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);

            let offer = self.offer_impl(&_bundle_id, self.env().caller());
            ensure!(
                offer.deadline > self.get_now(),
                Error::OfferNotExistsOrExpired
            );
            self.offers
                .remove(&(_bundle_id.clone(), self.env().caller()));
            self.env().emit_event(OfferCanceled {
                creator: self.env().caller(),
                bundle_id,
            });
            Ok(())
        }

        ///  Method for accepting the bundle offer
        ///  # Fields
        ///  bundle_id Bundle ID
        ///  creator Offer creator address
        ///
        /// # Errors
        ///
        /// - If the caller is not the owner of the specified bundle id   .
        /// - If `deadline` of the listing  of `bundle_id` and the creator is less than or equal to  the current time .
        /// - If it failed when the caller transfer platform fee to the `fee_recipient`  in the `pay_token` erc20 contract .
        /// - If it failed when the caller transfer  pay amount to the owner in the `pay_token` erc20 contract .
        /// - If it failed when the caller transfer  token id  to the creator in the `nft_address` erc721 contract .
        /// - If it failed when the caller transfer  token id  to the creator  in the `nft_address` erc1155 contract .
        /// - If the `validate_item_sold` executes failed.
        #[ink(message)]
        pub fn accept_offer(&mut self, bundle_id: String, creator: AccountId) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);
            let owner = self.owner_impl(&_bundle_id);
            ensure!(owner == self.env().caller(), Error::NotOwningItem);
            let offer = self.offer_impl(&_bundle_id, creator);
            ensure!(
                offer.deadline > self.get_now(),
                Error::OfferNotExistsOrExpired
            );

            let price = offer.price;
            let fee_amount = price * self.platform_fee / 1000;
            ensure!(
                self.erc20_transfer_from(offer.pay_token, creator, self.fee_recipient, fee_amount)
                    .is_ok(),
                Error::ERC20TransferFromFeeAmountFailed
            );
            ensure!(
                self.erc20_transfer_from(
                    offer.pay_token,
                    self.env().caller(),
                    owner,
                    price - fee_amount,
                )
                .is_ok(),
                Error::ERC20TransferFromPayAmountFailed
            );

            let mut listing = self.listing_impl(&_bundle_id,self.env().caller());

            for (i, &nft_address) in listing.nft_addresses.iter().enumerate() {
                let token_id = listing.token_ids[i];
                let quantity = listing.quantities[i];
                // Transfer NFT to buyer
                if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                    ensure!(
                        self.erc721_transfer_from(
                            nft_address,
                            self.env().caller(),
                            creator,
                            token_id
                        )
                        .is_ok(),
                        Error::ERC721TransferFromTokenIdFailed
                    );
                } else if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155) {
                    ensure!(
                        self.erc1155_transfer_from(
                            nft_address,
                            self.env().caller(),
                            creator,
                            token_id,
                            quantity,
                        )
                        .is_ok(),
                        Error::ERC1155TransferFromTokenIdFailed
                    );
                }
                ensure!(
                    self.marketplace_validate_item_sold(nft_address, token_id, owner, creator)
                        .is_ok(),
                    Error::BundleMarketplaceValidateItemSoldFailed
                );
            }
            self.listings
                .remove(&(self.env().caller(), _bundle_id.clone()));
            listing.price = 0;
            self.listings
                .insert(&(creator, _bundle_id.clone()), &listing);
            self.owners.insert(&_bundle_id, &creator);
            self.offers.remove(&(_bundle_id.clone(), creator));

            self.env().emit_event(ItemSold {
                seller: self.env().caller(),
                buyer: creator,
                bundle_id: bundle_id.clone(),
                pay_token: offer.pay_token,
                unit_price: self.marketplace_get_price(offer.pay_token)?,
                price: offer.price,
            });
            self.env().emit_event(OfferCanceled { creator, bundle_id });
            Ok(())
        }

        ///  Method for updating platform fee
        ///  # Fields
        ///  Only admin
        ///  platform_fee the platform fee to set
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn update_platform_fee(&mut self, platform_fee: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.platform_fee = platform_fee;
            self.env().emit_event(UpdatePlatformFee { platform_fee });
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
            self.env()
                .emit_event(UpdatePlatformFeeRecipient { fee_recipient });
            Ok(())
        }

        /// Update SubAddressRegistry contract
        /// Only admin
        /// # Fields
        /// address_registry the address of address registry contract
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

        /// Validate and cancel listing
        /// Only marketplace can access
        /// # Fields
        /// nft_address NFT contract address
        /// token_id Token Id
        /// quantity The quantity of NFTs
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the address of the marketplace contract .
        #[ink(message)]
        pub fn validate_item_sold(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            quantity: u128,
        ) -> Result<()> {
            //onlyContract
            #[cfg(test)]
            {
                ensure!(
                    self.env().caller() == self.test_marketplace,
                    Error::SenderMustBeAuctionOrMarketplace
                );
            }
            ink_env::debug_println!("bundle_marketplace_validate_item_sold= {:?}", 1);
            #[cfg(not(test))]
            {
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);

                ensure!(
                    self.env().caller() == address_registry_instance.auction()
                        || self.env().caller() == address_registry_instance.marketplace(),
                    Error::SenderMustBeAuctionOrMarketplace
                );
            }
            ink_env::debug_println!("bundle_marketplace_validate_item_sold= {:?}", 2);
            let items = self.items_impl(nft_address, token_id);
            for _bundle_id in &items {
                let owner = self.owner_impl(&_bundle_id);
                if owner == AccountId::from([0x0; 32]) {
                    continue;
                }
                let mut listing = self.listing_impl( &_bundle_id,owner);
                let bundle_id = self.bundle_ids_impl(&_bundle_id);
                let index = self.indices_impl(&_bundle_id, nft_address, token_id) as usize;
                if listing.quantities[index] > quantity {
                    listing.quantities[index] -= quantity;
                    self.listings.insert(&(owner, _bundle_id.clone()), &listing);
                } else {
                    self.nft_indices
                        .remove(&(_bundle_id.clone(), nft_address, token_id));
                    if listing.nft_addresses.len() == 1 {
                        self.listings.remove(&(owner, _bundle_id.clone()));
                        self.owners.remove(&_bundle_id);
                        self.bundle_ids.remove(&_bundle_id);
                        self.env().emit_event(ItemUpdated {
                            owner: self.env().caller(),
                            bundle_id,
                            nft_addresses: Vec::new(),
                            token_ids: Vec::new(),
                            quantities: Vec::new(),
                            pay_token: AccountId::from([0x0; 32]),
                            new_price: 0,
                        });
                        continue;
                    } else {
                        let indexu = index as u128;
                        let last = index < listing.nft_addresses.len() - 1;
                        listing.nft_addresses.swap_remove(index);
                        listing.token_ids.swap_remove(index);
                        listing.quantities.swap_remove(index);
                        if last {
                            self.nft_indices.insert(
                                &(
                                    _bundle_id.clone(),
                                    listing.nft_addresses[index],
                                    listing.token_ids[index],
                                ),
                                &indexu,
                            );
                        }
                        self.listings.insert(&(owner, _bundle_id.clone()), &listing);
                    }
                }
                self.env().emit_event(ItemUpdated {
                    owner: self.env().caller(),
                    bundle_id,
                    nft_addresses: listing.nft_addresses,
                    token_ids: listing.token_ids,
                    quantities: listing.quantities,
                    pay_token: listing.pay_token,
                    new_price: listing.price,
                });
            }
            ink_env::debug_println!("bundle_marketplace_validate_item_sold= {:?}", 3);
            self.bundle_ids_per_item.remove(&(nft_address, token_id));
            Ok(())
        }

        ///  Method for get NFT bundle listing
        /// # Fields
        ///  owner Owner address
        ///  bundle_id Bundle ID
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn get_listing(
            &self,
            bundle_id: String,
            owner: AccountId,
        ) -> Listing {
            let _bundle_id = self.get_bundle_id(&bundle_id);
            self.listing_impl( &_bundle_id,owner)
        }

        /// Get owner of the specified bundle id
        /// # Fields
        ///  bundle_id Bundle ID
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner_of(&self, bundle_id: String) -> AccountId {
            self.owner_impl(&bundle_id)
        }
        /// Get items of the specified bundle id and token id
        /// # Fields
        ///  bundle_id Bundle ID
        /// token_id Token Id
        /// # Return
        ///  items  items
        #[ink(message)]
        pub fn items_of(&self, nft_address: AccountId, token_id: TokenId) -> BTreeSet<String> {
            self.items_impl(nft_address, token_id)
        }
        /// Get indices of the specified bundle id and token id
        /// # Fields
        ///  bundle_id Bundle ID
        /// nft_address NFT contract address
        /// token_id Token Id
        /// # Return
        ///  indices  indices
        #[ink(message)]
        pub fn indices_of(
            &self,
            bundle_id: String,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> u128 {
            self.indices_impl(&bundle_id, nft_address, token_id)
        }
        /// Method for getting the offer of the specified nft token contract address .
        /// # Fields
        ///  bundle_id Bundle ID
        ///  creator creator address
        #[ink(message)]
        pub fn offer_of(&self, bundle_id: String, creator: AccountId) -> Offer {
            self.offer_impl(&bundle_id, creator)
        }
        /// Get bundle_ids of the specified bundle id
        /// # Fields
        ///  bundle_id Bundle ID
        /// # Return
        ///  bundle_ids  bundle_ids
        #[ink(message)]
        pub fn bundle_ids_of(&self, bundle_id: String) -> String {
            self.bundle_ids_impl(&bundle_id)
        }
        #[ink(message)]
        pub fn get_bundle_ids(&self, bundle_id: String) -> String {
            self.get_bundle_id(&bundle_id)
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
        pub fn test_erc20_transfer_from(
            &mut self,
            token_contract: AccountId,
            bid_amount: Balance,
        ) -> Result<()> {
            self.erc20_transfer_from(
                token_contract,
                self.env().caller(),
                self.env().account_id(),
                bid_amount,
            )
        }
        #[ink(message)]
        pub fn test_marketplace_validate_item_sold(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            seller: AccountId,
            buyer: AccountId,
        ) -> Result<()> {
            self.marketplace_validate_item_sold(nft_address, token_id, seller, buyer)
        }
        #[ink(message)]
        pub fn test_marketplace_get_price(&self, pay_token: AccountId) -> Result<Balance> {
            self.marketplace_get_price(pay_token)
        }
        #[ink(message)]
        pub fn test_erc1155_transfer_from(
            &mut self,
            token: AccountId,
            from: AccountId,
            to: AccountId,
            token_id: TokenId,
            value: Balance,
        ) -> Result<()> {
            self.erc1155_transfer_from(token, from, to, token_id, value)
        }
        ///  tick the  flag
        ///  Only admin
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn tick(&mut self) -> Result<()> {
            ensure!(self.owner == self.env().caller(), Error::OnlyOwner);
            self.tick = !self.tick;
            Ok(())
        }
        #[ink(message)]
        pub fn test_erc1155_balance_of(
            &self,
            token: AccountId,
            owner: AccountId,
            token_id: TokenId,
        ) -> Result<Balance> {
            self.erc1155_balance_of(token, owner, token_id)
        }
        #[ink(message)]
        pub fn test_erc1155_is_approved_for_all(
            &self,
            token: AccountId,
            owner: AccountId,
            operator: AccountId,
        ) -> Result<bool> {
            self.erc1155_is_approved_for_all(token, owner, operator)
        }
        /// Get current timestamp
        /// # Return
        ///  current time  timestamp (s)
        #[ink(message)]
        pub fn now_timestamp(&self) -> u128 {
            self.get_now()
        }
    }
    #[ink(impl)]
    impl SubBundleMarketplace {
        /// Returns the listing for the specified `nft_address` and `token_id` and  owner`.
        ///
        /// Returns `default listing  ` if the listing is non-existent.
        #[inline]
        fn listing_impl(&self, bundle_id: &String, owner: AccountId) -> Listing {
            self.listings
                .get(&(owner, bundle_id.clone()))
                .unwrap_or_default()
        }

        /// Returns the offer for the specified `nft_address` and `token_id` and  owner`.
        ///
        /// Returns `default offer  ` if the offer is non-existent.
        #[inline]
        fn offer_impl(&self, bundle_id: &String, creator: AccountId) -> Offer {
            self.offers
                .get(&(bundle_id.clone(), creator))
                .unwrap_or_default()
        }
        /// Get owner of the specified bundle id
        /// # Fields
        ///  bundle_id Bundle ID
        /// # Return
        ///  owner  owner
        #[inline]
        fn owner_impl(&self, bundle_id: &String) -> AccountId {
            self.owners.get(bundle_id).unwrap_or_default()
        }
        /// Get items of the specified bundle id and token id
        /// # Fields
        ///  bundle_id Bundle ID
        /// token_id Token Id
        /// # Return
        ///  items  items
        #[inline]
        fn items_impl(&self, nft_address: AccountId, token_id: TokenId) -> BTreeSet<String> {
            self.bundle_ids_per_item
                .get(&(nft_address, token_id))
                .unwrap_or_default()
        }
        /// Get bundle_ids of the specified bundle id
        /// # Fields
        ///  bundle_id Bundle ID
        /// # Return
        ///  bundle_ids  bundle_ids
        #[inline]
        fn bundle_ids_impl(&self, bundle_id: &String) -> String {
            self.bundle_ids.get(bundle_id).unwrap_or_default()
        }
        /// Get indices of the specified bundle id and token id
        /// # Fields
        ///  bundle_id Bundle ID
        /// nft_address NFT contract address
        /// token_id Token Id
        /// # Return
        ///  indices  indices
        #[inline]
        fn indices_impl(
            &self,
            bundle_id: &String,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> u128 {
            self.nft_indices
                .get(&(bundle_id.clone(), nft_address, token_id))
                .unwrap_or_default()
        }
        fn _cancel_listing(&mut self, owner: AccountId, bundle_id: String) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);

            let listing = self.listing_impl( &_bundle_id,owner);
            for (i, &nft_address) in listing.nft_addresses.iter().enumerate() {
                let token_id = listing.token_ids[i];
                let mut items = self.items_impl(nft_address, token_id);
                items.remove(&_bundle_id);
                self.bundle_ids_per_item
                    .insert(&(nft_address, token_id), &items);
                self.nft_indices
                    .remove(&(_bundle_id.clone(), nft_address, token_id));
            }

            self.listings.remove(&(owner, _bundle_id.clone()));
            self.owners.remove(&_bundle_id);
            self.bundle_ids.remove(&_bundle_id);
            self.env().emit_event(ItemCanceled { owner, bundle_id });
            Ok(())
        }
        fn _buy_item(&mut self, bundle_id: String, pay_token: AccountId) -> Result<()> {
            let _bundle_id = self.get_bundle_id(&bundle_id);
            let owner = self.owner_impl(&_bundle_id);
            let mut listing = self.listing_impl( &_bundle_id,owner);
            ensure!(listing.price > 0, Error::NotListedItem);

            for (i, &nft_address) in listing.nft_addresses.iter().enumerate() {
                let token_id = listing.token_ids[i];
                let quantity = listing.quantities[i];
                if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                    ensure!(
                        Some(owner) == self.erc721_owner_of(nft_address, token_id)?,
                        Error::NotOwningItem
                    );
                } else if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155) {
                    ensure!(
                        quantity <= self.erc1155_balance_of(nft_address, owner, token_id)?,
                        Error::MustHoldEnoughNFTs
                    );
                } else {
                    ensure!(false, Error::InvalidNFTAddress);
                }
            }

            ensure!(self.get_now() >= listing.start_time, Error::ItemNotBuyable);

            let price = listing.price;
            let fee_amount = price * self.platform_fee / 1000;
            if pay_token == AccountId::from([0x0; 32]) {
                // Send platform fee
                ensure!(fee_amount <= self.env().balance(), Error::FeeTransferFailed);
                ensure!(
                    self.env().transfer(self.fee_recipient, fee_amount).is_ok(),
                    Error::FeeTransferFailed
                );
                ensure!(
                    price - fee_amount <= self.env().balance(),
                    Error::OwnerFeeTransferFailed
                );
                ensure!(
                    self.env().transfer(owner, price - fee_amount).is_ok(),
                    Error::OwnerFeeTransferFailed
                );
            } else {
                ensure!(
                    self.erc20_transfer_from(
                        pay_token,
                        self.env().caller(),
                        self.fee_recipient,
                        fee_amount,
                    )
                    .is_ok(),
                    Error::ERC20TransferFromFeeAmountFailed
                );
                ensure!(
                    self.erc20_transfer_from(
                        pay_token,
                        self.env().caller(),
                        owner,
                        price - fee_amount
                    )
                    .is_ok(),
                    Error::ERC20TransferFromPayAmountFailed
                );
            }

            for (i, &nft_address) in listing.nft_addresses.iter().enumerate() {
                let token_id = listing.token_ids[i];
                let quantity = listing.quantities[i];
                if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                    ensure!(
                        self.erc721_transfer_from(
                            nft_address,
                            owner,
                            self.env().caller(),
                            token_id
                        )
                        .is_ok(),
                        Error::ERC721TransferFromTokenIdFailed
                    );
                } else if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155) {
                    ensure!(
                        self.erc1155_transfer_from(
                            nft_address,
                            owner,
                            self.env().caller(),
                            token_id,
                            quantity,
                        )
                        .is_ok(),
                        Error::ERC1155TransferFromTokenIdFailed
                    );
                }
                ensure!(
                    self.marketplace_validate_item_sold(
                        nft_address,
                        token_id,
                        owner,
                        self.env().caller(),
                    )
                    .is_ok(),
                    Error::BundleMarketplaceValidateItemSoldFailed
                );
            }
            self.listings.remove(&(owner, _bundle_id.clone()));
            listing.price = 0;
            self.listings
                .insert(&(self.env().caller(), _bundle_id.clone()), &listing);
            self.owners.insert(&_bundle_id, &self.env().caller());
            self.offers.remove(&(_bundle_id, self.env().caller()));
            self.env().emit_event(ItemSold {
                seller: owner,
                buyer: self.env().caller(),
                bundle_id: bundle_id.clone(),
                pay_token,
                unit_price: self.marketplace_get_price(pay_token)?,
                price,
            });
            self.env().emit_event(OfferCanceled {
                creator: self.env().caller(),
                bundle_id,
            });
            Ok(())
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn get_marketplace(&self) -> Result<AccountId> {
            #[cfg(test)]
            {
                Ok(AccountId::from([0x0; 32]))
            }
            #[cfg(not(test))]
            {
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);

                ensure!(
                    AccountId::from([0x0; 32]) != address_registry_instance.marketplace(),
                    Error::InvalidPayToken
                );
                Ok(address_registry_instance.marketplace())
            }
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn marketplace_validate_item_sold(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            seller: AccountId,
            buyer: AccountId,
        ) -> Result<()> {
            let marketplace = self.get_marketplace()?;
            #[cfg(test)]
            {
                ensure!(
                    !self.test_marketplace_validate_item_sold_failed,
                    Error::TransactionFailed
                );
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x5E, 0x38, 0x31, 0x94]; //_marketplace_validate_item_sold
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(marketplace)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(nft_address)
                            .push_arg(token_id)
                            .push_arg(seller)
                            .push_arg(buyer),
                    )
                    .returns::<Result<()>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("marketplace_validate_item_sold= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!("marketplace_validate_item_sold= {:?}", e);
                            Error::TransactionFailed
                        })
                    })
            }
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn marketplace_get_price(&self, pay_token: AccountId) -> Result<Balance> {
            let marketplace = self.get_marketplace()?;
            #[cfg(test)]
            {
                Ok(1)
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0xF2, 0x3D, 0x4B, 0x6C];
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(marketplace)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(pay_token))
                    .returns::<Result<Balance>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("marketplace_get_price= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!("marketplace_get_price= {:?}", e);
                            Error::TransactionFailed
                        })
                    })
            }
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
        #[cfg_attr(test, allow(unused_variables))]
        fn valid_pay_token(&self, pay_token: AccountId) -> Result<()> {
            if AccountId::from([0x0; 32]) != pay_token {
                ensure!(
                    self.token_registry_enabled(self.get_token_registry()?, pay_token)
                        .unwrap_or(false),
                    Error::InvalidPayToken,
                );
            }
            Ok(())
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
                let selector: [u8; 4] = [0x14, 0x14, 0x63, 0x1C]; // token_registry_enabled
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
                        ink_env::debug_println!("token_registry_enabled= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc20_transfer_from(
            &mut self,
            token: AccountId,
            from: AccountId,
            to: AccountId,
            value: Balance,
        ) -> Result<()> {
            #[cfg(test)]
            {
                ensure!(
                    self.test_transfer_fail
                        .get(&(token, from, to, 0, value))
                        .is_none(),
                    Error::TransactionFailed
                );
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x0B, 0x39, 0x6F, 0x18]; //erc20 transfer_from
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(from)
                            .push_arg(to)
                            .push_arg(value),
                    )
                    .returns::<Result<()>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc20_transfer_from= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!("erc20_transfer_from= {:?}", e);
                            Error::TransactionFailed
                        })
                    })
            }
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn erc721_transfer_from(
            &mut self,
            token: AccountId,
            from: AccountId,
            to: AccountId,
            token_id: TokenId,
        ) -> Result<()> {
            #[cfg(test)]
            {
                ensure!(
                    self.test_transfer_fail
                        .get(&(token, from, to, token_id, 0))
                        .is_none(),
                    Error::TransactionFailed
                );
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x25, 0xF5, 0xB2, 0xD1]; //Erc721::transfer_from
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(from)
                            .push_arg(to)
                            .push_arg(token_id),
                    )
                    .returns::<Result<()>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc721_transfer_from= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!("erc721_transfer_from= {:?}", e);
                            Error::TransactionFailed
                        })
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc721_owner_of(
            &self,
            token: AccountId,
            token_id: TokenId,
        ) -> Result<Option<AccountId>> {
            #[cfg(test)]
            {
                Ok(self.test_token_owner.get(&token_id))
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x48, 0x39, 0x17, 0x41]; //auction Erc721::owner_of
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(token_id))
                    .returns::<Option<AccountId>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc721_owner_of= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc721_is_approved_for_all(
            &self,
            token: AccountId,
            owner: AccountId,
            operator: AccountId,
        ) -> Result<bool> {
            #[cfg(test)]
            {
                Ok(self
                    .test_operator_approvals
                    .get(&(owner, operator))
                    .is_some())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x65, 0x11, 0x56, 0xC8]; //auction is_approved_for_all_nft
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(owner)
                            .push_arg(operator),
                    )
                    .returns::<bool>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc721_is_approved_for_all= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc1155_transfer_from(
            &mut self,
            token: AccountId,
            from: AccountId,
            to: AccountId,
            token_id: TokenId,
            value: Balance,
        ) -> Result<()> {
            #[cfg(test)]
            {
                ensure!(
                    self.test_transfer_fail
                        .get(&(token, from, to, token_id, value))
                        .is_none(),
                    Error::TransactionFailed
                );
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x53, 0x24, 0xD5, 0x56]; //erc1155 safe_transfer_from
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(from)
                            .push_arg(to)
                            .push_arg(token_id)
                            .push_arg(value)
                            .push_arg(Vec::<u8>::new()),
                    )
                    .returns::<Result<()>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc1155_transfer_from= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!("erc1155_transfer_from= {:?}", e);
                            Error::TransactionFailed
                        })
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc1155_balance_of(
            &self,
            token: AccountId,
            owner: AccountId,
            token_id: TokenId,
        ) -> Result<Balance> {
            #[cfg(test)]
            {
                Ok(self
                    .test_balances
                    .get(&(owner, token_id))
                    .unwrap_or_default())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x16, 0x4B, 0x9B, 0xA0]; //erc1155 balance_of
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(owner)
                            .push_arg(token_id),
                    )
                    .returns::<Balance>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc1155_balance_of= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc1155_is_approved_for_all(
            &self,
            token: AccountId,
            owner: AccountId,
            operator: AccountId,
        ) -> Result<bool> {
            #[cfg(test)]
            {
                Ok(self
                    .test_operator_approvals
                    .get(&(owner, operator))
                    .is_some())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x84, 0x3D, 0x8A, 0xAC]; //is_approved_for_all_art
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(owner)
                            .push_arg(operator),
                    )
                    .returns::<bool>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc1155_is_approved_for_all= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        fn get_now(&self) -> u128 {
            self.env().block_timestamp() as u128 / 1000
        }
        fn get_bundle_id(&self, bundle_id: &String) -> String {
            use ink_env::hash;

            let uncompressed = bundle_id.as_bytes();

            // Hash the uncompressed public key by Keccak256 algorithm.
            let mut hash = <hash::Keccak256 as hash::HashOutput>::Type::default();
            // The first byte indicates that the public key is uncompressed.
            // Let's skip it for hashing the public key directly.
            ink_env::hash_bytes::<hash::Keccak256>(&uncompressed[1..], &mut hash);
            bundle_id.clone()
        }
    }
    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_env::Clear;
        use ink_lang as ink;
        type Event = <SubBundleMarketplace as ::ink_lang::reflect::ContractEventBase>::Type;
        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<ink_env::DefaultEnvironment>(sender);
        }
        fn default_accounts() -> ink_env::test::DefaultAccounts<Environment> {
            ink_env::test::default_accounts::<Environment>()
        }
        fn contract_id() -> AccountId {
            ink_env::test::callee::<ink_env::DefaultEnvironment>()
        }
        fn set_balance(account_id: AccountId, balance: Balance) {
            ink_env::test::set_account_balance::<ink_env::DefaultEnvironment>(account_id, balance)
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
        fn init_contract() -> SubBundleMarketplace {
            let erc = SubBundleMarketplace::new(10, fee_recipient());

            erc
        }
        #[ink::test]
        fn list_item_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            //   assert_eq!( bundle_marketplace.list_item(
            //                     bundle_id.clone(),
            //                     nft_addresses.clone(),
            //                     token_ids.clone(),
            //                     quantities.clone(),
            //                     pay_token,
            //                     price,
            //                     start_time,
            //             ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .list_item(
                    bundle_id.clone(),
                    nft_addresses.clone(),
                    token_ids.clone(),
                    quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                )
                .is_ok());
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            assert_eq!(
                bundle_marketplace.listings.get(&(caller, _bundle_id)),
                Some(Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                })
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_listed_event(
                &emitted_events[0],
                caller,
                bundle_id,
                pay_token,
                price,
                start_time,
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_length_of_nft_addresses_is_not_equal_to_the_length_of_token_ids_or_the_length_of_quantities_is_not_equal_to_the_length_of_token_ids(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1, 2];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::InvalidData
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_owner_of_the_specified_bundle_id_is_not_zero_address_and_the_caller_is_not_the_owner_of_the_specified_bundle_id_or_the_price_of_the_listing_of_the_specified_bundle_id_is_greater_than_zero(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.owners.insert(&_bundle_id, &eve());
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::AlreadyListed
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_pay_token_is_not_enabled_in_the_token_registry_contract_when_the_pay_token_is_not_zero_address(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_contract_is_not_approved_by_the_caller_in_the_contract_in_the_following_erc_721_standard(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
            }
            bundle_marketplace.test_enabled.insert(&pay_token, &true);

            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::ItemNotApproved
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::MustHoldEnoughNFTs
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_contract_is_not_approved_by_the_caller_in_the_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            for (&token_id, &quantity) in token_ids.iter().zip(&quantities) {
                bundle_marketplace
                    .test_balances
                    .insert(&(caller, token_id), &quantity);
            }
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::ItemNotApproved
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![charlie()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                bundle_marketplace
                    .list_item(
                        bundle_id.clone(),
                        nft_addresses.clone(),
                        token_ids.clone(),
                        quantities.clone(),
                        pay_token,
                        price,
                        start_time,
                    )
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn cancel_listing_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            bundle_marketplace.listings.insert(
                &(caller, _bundle_id),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            // assert_eq!( bundle_marketplace.cancel_listing(
            //   nft_address,
            //         token_id,
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace.cancel_listing(bundle_id.clone()).is_ok());
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            for (i, &nft_address) in nft_addresses.iter().enumerate() {
                let token_id = token_ids[i];
                assert!(bundle_marketplace
                    .items_impl(nft_address, token_id)
                    .get(&_bundle_id)
                    .is_none());

                assert!(bundle_marketplace
                    .nft_indices
                    .get(&(_bundle_id.clone(), nft_address, token_id))
                    .is_none());
            }
            assert_eq!(bundle_marketplace.owners.get(&_bundle_id), None);
            assert_eq!(bundle_marketplace.bundle_ids.get(&_bundle_id), None);
            assert_eq!(bundle_marketplace.listings.get(&(caller, _bundle_id)), None);

            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_canceled_event(&emitted_events[0], caller, bundle_id);
        }

        #[ink::test]
        fn cancel_listing_failed_if_the_price_of_the_listing_of_the_specified_bundle_id_is_zero() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            // let bundle_id = String::from("1");
            // let nft_addresses = vec![alice()];
            // let token_ids = vec![1];
            // let quantities = vec![1];
            // let pay_token = alice();
            // let price = 10;
            // let start_time = 10;
            // let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            // bundle_marketplace.listings.insert(
            //     &(caller, _bundle_id),
            //     &Listing {
            //         nft_addresses,
            //         token_ids,
            //         quantities,
            //         pay_token,
            //         price,
            //         start_time,
            //     },
            // );
            let bundle_id = String::from("1");
            // let nft_addresses = vec![alice()];
            // let token_ids = vec![1];
            assert_eq!(
                bundle_marketplace
                    .cancel_listing(bundle_id.clone())
                    .unwrap_err(),
                Error::NotListedItem
            );
        }

        #[ink::test]
        fn update_listing_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(caller, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let new_pay_token = eve();
            let new_price = 11;
            bundle_marketplace
                .test_enabled
                .insert(&new_pay_token, &true);
            // assert_eq!( bundle_marketplace.update_listing(
            //  bundle_id.clone(), new_pay_token, new_price
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .update_listing(bundle_id.clone(), new_pay_token, new_price)
                .is_ok());

            assert_eq!(
                bundle_marketplace.listings.get(&(caller, _bundle_id)),
                Some(Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token: new_pay_token,
                    price: new_price,
                    start_time,
                })
            );
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_updated_event(
                &emitted_events[0],
                caller,
                bundle_id,
                nft_addresses,
                token_ids,
                quantities,
                new_pay_token,
                new_price,
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_price_of_the_listing_of_the_specified_bundle_id_is_zero() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            // let bundle_id = String::from("1");
            // let nft_addresses = vec![alice()];
            // let token_ids = vec![1];
            // let quantities = vec![1];
            // let pay_token = alice();
            // let price = 10;
            // let start_time = 10;
            // let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            // bundle_marketplace.listings.insert(
            //     &(caller, _bundle_id.clone()),
            //     &Listing {
            //         nft_addresses,
            //         token_ids,
            //         quantities,
            //         pay_token,
            //         price,
            //         start_time,
            //     },
            // );
            let bundle_id = String::from("1");
            // let nft_addresses = vec![alice()];
            // let token_ids = vec![1];
            // let quantities = vec![1];
            let new_pay_token = eve();
            let new_price = 11;
            assert_eq!(
                bundle_marketplace
                    .update_listing(bundle_id.clone(), new_pay_token, new_price)
                    .unwrap_err(),
                Error::NotListedItem
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_pay_token_is_not_enabled_in_the_token_registry_contract_when_the_pay_token_is_not_zero_address(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = 10;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(caller, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            let new_pay_token = eve();
            let new_price = 11;
            assert_eq!(
                bundle_marketplace
                    .update_listing(bundle_id.clone(), new_pay_token, new_price)
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn buy_item_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );

            // assert_eq!( bundle_marketplace.buy_item(
            //   nft_address,
            //         token_id, pay_token, owner
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .buy_item(bundle_id.clone(), pay_token)
                .is_ok());

            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(owner, _bundle_id.clone())),
                None
            );
            assert_eq!(bundle_marketplace.owners.get(&_bundle_id), Some(caller));
            assert_eq!(
                bundle_marketplace.offers.get(&(_bundle_id.clone(), caller)),
                None
            );
            let unit_price = 1;
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_item_sold_event(
                &emitted_events[0],
                owner,
                caller,
                bundle_id.clone(),
                pay_token,
                unit_price,
                price,
            );
            assert_offer_canceled_event(&emitted_events[1], caller, bundle_id);
        }

        #[ink::test]
        fn buy_item_failed_if_the_owner_of_the_specified_bundle_id_is_zero_address() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            // let nft_addresses = vec![alice()];
            // let token_ids = vec![1];
            // let quantities = vec![1];
            // let price = 10;
            // let start_time = bundle_marketplace.get_now();
            // let owner = bob();
            // let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            // bundle_marketplace.listings.insert(
            //     &(owner, _bundle_id.clone()),
            //     &Listing {
            //         nft_addresses,
            //         token_ids,
            //         quantities,
            //         pay_token,
            //         price,
            //         start_time,
            //     },
            // );
            // bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::InvalidId
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_pay_token_is_not_the_pay_token_of_the_listing_of_nft_address_and_bundle_id(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), frank())
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_price_of_the_listing_of_the_specified_bundle_id_is_zero() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 0;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::NotListedItem
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            // bundle_marketplace.test_enabled.insert(&pay_token, &true);
            // for &token_id in &token_ids {
            //     bundle_marketplace
            //         .test_token_owner
            //         .insert(&token_id, &frank());
            // }
            // bundle_marketplace
            //     .test_operator_approvals
            //     .insert(&(caller, contract_id()), &());

            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_contract_is_not_approved_by_the_caller_in_the_contract_in_the_following_erc_721_standard(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            // bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            // bundle_marketplace
            //     .test_operator_approvals
            //     .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::ItemNotApproved
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::MustHoldEnoughNFTs
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_start_time_of_the_listing_of_nft_address_and_token_id_and_owner_is_greater_than_the_current_time(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now() + 1;
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::ItemNotBuyable
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_platform_fee_to_the_fee_recipient_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = AccountId::from([0x0; 32]);
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10 * 1000;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            set_balance(contract_id(), 0);
            bundle_marketplace.platform_fee = 1;
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::FeeTransferFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_owner_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = AccountId::from([0x0; 32]);
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10 * 1000;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.platform_fee = 1;
            let fee_amount = price * bundle_marketplace.platform_fee / 1000;
            set_balance(contract_id(), fee_amount);
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::OwnerFeeTransferFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_platform_fee_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            let fee_amount = price * bundle_marketplace.platform_fee / 1000;
            bundle_marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, fee_recipient(), 0, fee_amount), &());
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::ERC20TransferFromFeeAmountFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_owner_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            let fee_amount = price * bundle_marketplace.platform_fee / 1000;
            bundle_marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, owner, 0, price - fee_amount), &());
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::ERC20TransferFromPayAmountFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for (i, &token_id) in token_ids.iter().enumerate() {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
                bundle_marketplace
                    .test_transfer_fail
                    .insert(&(nft_addresses[i], owner, caller, token_id, 0), &());
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);

            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::ERC721TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc1155_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for (i, &token_id) in token_ids.iter().enumerate() {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
                bundle_marketplace.test_transfer_fail.insert(
                    &(nft_addresses[i], owner, caller, token_id, quantities[i]),
                    &(),
                );
                bundle_marketplace
                    .test_balances
                    .insert(&(owner, token_id), &quantities[i]);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::ERC1155TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_bundle_marketplace_validate_item_sold_failed() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &owner);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.test_marketplace_validate_item_sold_failed = true;
            assert_eq!(
                bundle_marketplace
                    .buy_item(bundle_id.clone(), pay_token)
                    .unwrap_err(),
                Error::BundleMarketplaceValidateItemSoldFailed
            );
        }

        #[ink::test]
        fn create_offer_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");

            let pay_token = alice();
            let price = 1;
            let deadline = bundle_marketplace.get_now() + 1;
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            // assert_eq!( bundle_marketplace.create_offer(
            //               bundle_id.clone(),
            //         pay_token,
            //         price,
            //         deadline
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .create_offer(bundle_id.clone(), pay_token, price, deadline)
                .is_ok());
            assert_eq!(
                bundle_marketplace.offers.get(&(_bundle_id, caller)),
                Some(Offer {
                    pay_token,
                    price,
                    deadline,
                }),
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_offer_created_event(
                &emitted_events[0],
                caller,
                bundle_id,
                pay_token,
                price,
                deadline,
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_owner_of_the_specified_bundle_id_is_zero_address() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");

            let pay_token = alice();
            let price = 1;
            let deadline = bundle_marketplace.get_now() + 1;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            // let owner = bob();
            // bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .create_offer(bundle_id.clone(), pay_token, price, deadline)
                    .unwrap_err(),
                Error::InvalidId
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_deadline_is_less_than_or_equal_to_the_current_time() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");

            let pay_token = alice();
            let price = 1;
            let deadline = bundle_marketplace.get_now();
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .create_offer(bundle_id.clone(), pay_token, price, deadline)
                    .unwrap_err(),
                Error::InvalidExpiration
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_price_of_the_listing_of_the_specified_bundle_id_is_zero() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");

            let pay_token = alice();
            let price = 0;
            let deadline = bundle_marketplace.get_now() + 1;
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            assert_eq!(
                bundle_marketplace
                    .create_offer(bundle_id.clone(), pay_token, price, deadline)
                    .unwrap_err(),
                Error::InvalidPrice
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_deadline_of_the_listing_of_bundle_id_and_the_caller_is_greater_than_the_current_time(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");

            let pay_token = alice();
            let price = 1;
            let deadline = bundle_marketplace.get_now() + 1;
            let owner = bob();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), caller),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            assert_eq!(
                bundle_marketplace
                    .create_offer(bundle_id.clone(), pay_token, price, deadline)
                    .unwrap_err(),
                Error::OfferAlreadyCreated
            );
        }

        #[ink::test]
        fn cancel_offer_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let price = 1;
            let deadline = bundle_marketplace.get_now() + 1;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), caller),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );

            // assert_eq!( bundle_marketplace.cancel_offer(
            // nft_address, token_id
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace.cancel_offer(bundle_id.clone()).is_ok());
            assert_eq!(bundle_marketplace.offers.get(&(_bundle_id, caller)), None);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_offer_canceled_event(&emitted_events[0], caller, bundle_id);
        }

        #[ink::test]
        fn cancel_offer_failed_if_the_deadline_of_the_listing_of_bundle_id_and_the_caller_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let pay_token = alice();
            let price = 1;
            let deadline = bundle_marketplace.get_now();
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), caller),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );

            assert_eq!(
                bundle_marketplace.cancel_offer(bundle_id).unwrap_err(),
                Error::OfferNotExistsOrExpired
            );
        }

        #[ink::test]
        fn accept_offer_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            //     assert_eq!( bundle_marketplace.accept_offer(
            //    bundle_id.clone(), creator
            //     ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .accept_offer(bundle_id.clone(), creator)
                .is_ok());
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(caller, _bundle_id.clone())),
                None,
            );
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(caller, _bundle_id.clone())),
                None
            );
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(creator, _bundle_id.clone())),
                Some(Listing {
                    nft_addresses,
                    token_ids,
                    quantities,
                    pay_token,
                    price: 0,
                    start_time,
                })
            );
            assert_eq!(
                bundle_marketplace
                    .offers
                    .get(&(_bundle_id.clone(), creator)),
                None
            );
            assert_eq!(bundle_marketplace.owners.get(&_bundle_id), Some(creator));
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_item_sold_event(
                &emitted_events[0],
                caller,
                creator,
                bundle_id.clone(),
                pay_token,
                unit_price,
                price,
            );
            assert_offer_canceled_event(&emitted_events[1], creator, bundle_id);
        }

        #[ink::test]
        fn accept_offer_failed_if_the_caller_is_not_the_owner_of_the_specified_bundle_id() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            // bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_the_deadline_of_the_listing_of_bundle_id_and_the_creator_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::OfferNotExistsOrExpired
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_contract_transfer_platform_fee_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            let fee_amount = price * bundle_marketplace.platform_fee / 1000;
            bundle_marketplace
                .test_transfer_fail
                .insert(&(pay_token, creator, fee_recipient(), 0, fee_amount), &());
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::ERC20TransferFromFeeAmountFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_owner_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            let fee_amount = price * bundle_marketplace.platform_fee / 1000;
            bundle_marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, owner, 0, price - fee_amount), &());
            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::ERC20TransferFromPayAmountFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for (i, &token_id) in token_ids.iter().enumerate() {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
                bundle_marketplace
                    .test_transfer_fail
                    .insert(&(nft_addresses[i], caller, creator, token_id, 0), &());
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::ERC721TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc1155_contract(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for (i, &token_id) in token_ids.iter().enumerate() {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
                bundle_marketplace.test_transfer_fail.insert(
                    &(nft_addresses[i], caller, creator, token_id, quantities[i]),
                    &(),
                );
                bundle_marketplace
                    .test_balances
                    .insert(&(owner, token_id), &quantities[i]);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::ERC1155TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_the_bundle_marketplace_validate_item_sold_failed() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let deadline = bundle_marketplace.get_now() + 1;
            let creator = bob();
            //let unit_price = 1;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = alice();
            let price = 10;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            bundle_marketplace.test_enabled.insert(&pay_token, &true);
            for &token_id in &token_ids {
                bundle_marketplace
                    .test_token_owner
                    .insert(&token_id, &caller);
            }
            bundle_marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.offers.insert(
                &(_bundle_id.clone(), creator),
                &Offer {
                    pay_token,
                    price,
                    deadline,
                },
            );
            bundle_marketplace.test_marketplace_validate_item_sold_failed = true;

            assert_eq!(
                bundle_marketplace
                    .accept_offer(bundle_id.clone(), creator)
                    .unwrap_err(),
                Error::BundleMarketplaceValidateItemSoldFailed
            );
        }

        #[ink::test]
        fn update_platform_fee_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let platform_fee = 10;
            assert!(bundle_marketplace.update_platform_fee(platform_fee).is_ok());

            assert_eq!(bundle_marketplace.platform_fee, platform_fee);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_update_platform_fee_event(&emitted_events[0], platform_fee);
        }

        #[ink::test]
        fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = frank();
            set_caller(caller);
            let platform_fee = 10;
            assert_eq!(
                bundle_marketplace
                    .update_platform_fee(platform_fee)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_recipient_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let fee_recipient = bob();
            assert!(bundle_marketplace
                .update_platform_fee_recipient(fee_recipient)
                .is_ok());

            assert_eq!(bundle_marketplace.fee_recipient, fee_recipient);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_update_platform_fee_recipient_event(&emitted_events[0], fee_recipient);
        }

        #[ink::test]
        fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = frank();
            set_caller(caller);
            let fee_recipient = bob();
            assert_eq!(
                bundle_marketplace
                    .update_platform_fee_recipient(fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_address_registry_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let address_registry = bob();
            assert!(bundle_marketplace
                .update_address_registry(address_registry)
                .is_ok());

            assert_eq!(bundle_marketplace.address_registry, address_registry);
        }

        #[ink::test]
        fn update_address_registry_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = frank();
            set_caller(caller);
            let address_registry = bob();
            assert_eq!(
                bundle_marketplace
                    .update_address_registry(address_registry)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn validate_item_sold_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price = 1;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![quantity];
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            let mut items = bundle_marketplace.items_impl(nft_address, token_id);
            items.insert(_bundle_id.clone());
            bundle_marketplace
                .bundle_ids_per_item
                .insert(&(nft_address, token_id), &items);

            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace
                .bundle_ids
                .insert(&_bundle_id, &bundle_id);

            bundle_marketplace
                .nft_indices
                .insert(&(_bundle_id.clone(), nft_address, token_id), &0);

            bundle_marketplace.test_marketplace = caller;

            // assert_eq!( bundle_marketplace.validate_item_sold(
            // nft_address, token_id, quantity - 10
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .validate_item_sold(nft_address, token_id, quantity - 10)
                .is_ok());
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(owner, _bundle_id.clone())),
                Some(Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: vec![10],
                    pay_token,
                    price,
                    start_time,
                }),
            );
            assert_eq!(
                bundle_marketplace
                    .bundle_ids_per_item
                    .get(&(nft_address, token_id)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_updated_event(
                &emitted_events[0],
                caller,
                bundle_id,
                nft_addresses,
                token_ids,
                vec![10],
                pay_token,
                price,
            );
        }

        #[ink::test]
        fn validate_item_sold_works_when_the_quantity_equal_to_the_quantity_of_the_listing_and_the_length_of_nft_addresses_equal_to_1(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price = 1;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![quantity];
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            let mut items = bundle_marketplace.items_impl(nft_address, token_id);
            items.insert(_bundle_id.clone());
            bundle_marketplace
                .bundle_ids_per_item
                .insert(&(nft_address, token_id), &items);

            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace
                .bundle_ids
                .insert(&_bundle_id, &bundle_id);

            bundle_marketplace
                .nft_indices
                .insert(&(_bundle_id.clone(), nft_address, token_id), &0);

            bundle_marketplace.test_marketplace = caller;

            // assert_eq!( bundle_marketplace.validate_item_sold(
            // nft_address, token_id
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .validate_item_sold(nft_address, token_id, quantity)
                .is_ok());
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(owner, _bundle_id.clone())),
                None,
            );
            assert_eq!(bundle_marketplace.owners.get(&_bundle_id.clone()), None);
            assert_eq!(bundle_marketplace.bundle_ids.get(&_bundle_id.clone()), None);
            assert_eq!(
                bundle_marketplace
                    .nft_indices
                    .get(&(_bundle_id.clone(), nft_address, token_id)),
                None,
            );
            assert_eq!(
                bundle_marketplace
                    .bundle_ids_per_item
                    .get(&(nft_address, token_id)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_updated_event(
                &emitted_events[0],
                caller,
                bundle_id,
                Vec::new(),
                Vec::new(),
                Vec::new(),
                AccountId::from([0x0; 32]),
                0,
            );
        }

        #[ink::test]
        fn validate_item_sold_workswhen_the_quantity_equal_to_the_quantity_of_the_listing_and_the_length_of_nft_addresses_is_greater_than_1(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price = 1;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice(), eve()];
            let token_ids = vec![1, 2];
            let quantities = vec![quantity, 400];
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            let mut items = bundle_marketplace.items_impl(nft_address, token_id);
            items.insert(_bundle_id.clone());
            bundle_marketplace
                .bundle_ids_per_item
                .insert(&(nft_address, token_id), &items);

            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace
                .bundle_ids
                .insert(&_bundle_id, &bundle_id);

            for (i, (&nft_address, &token_id)) in nft_addresses.iter().zip(&token_ids).enumerate() {
                bundle_marketplace
                    .nft_indices
                    .insert(&(_bundle_id.clone(), nft_address, token_id), &(i as u128));
            }

            bundle_marketplace.test_marketplace = caller;

            // assert_eq!( bundle_marketplace.validate_item_sold(
            // nft_address, token_id
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .validate_item_sold(nft_address, token_id, quantity)
                .is_ok());
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(owner, _bundle_id.clone())),
                Some(Listing {
                    nft_addresses: vec![nft_addresses[1]],
                    token_ids: vec![token_ids[1]],
                    quantities: vec![quantities[1]],
                    pay_token,
                    price,
                    start_time,
                }),
            );
            assert_eq!(
                bundle_marketplace
                    .nft_indices
                    .get(&(_bundle_id.clone(), nft_address, token_id)),
                None,
            );
            assert_eq!(
                bundle_marketplace.nft_indices.get(&(
                    _bundle_id.clone(),
                    nft_addresses[1],
                    token_ids[1]
                )),
                Some(0),
            );
            assert_eq!(
                bundle_marketplace
                    .bundle_ids_per_item
                    .get(&(nft_address, token_id)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_updated_event(
                &emitted_events[0],
                caller,
                bundle_id,
                vec![nft_addresses[1]],
                vec![token_ids[1]],
                vec![quantities[1]],
                pay_token,
                price,
            );
        }

        #[ink::test]
        fn validate_item_sold_workswhen_the_quantity_equal_to_the_quantity_of_the_listing_and_the_length_of_nft_addresses_is_greater_than_1_and_the_index_is_the_last(
        ) {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price = 1;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let bundle_id = String::from("1");
            let nft_addresses = vec![eve(), alice()];
            let token_ids = vec![2, 1];
            let quantities = vec![400, quantity];
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            let mut items = bundle_marketplace.items_impl(nft_address, token_id);
            items.insert(_bundle_id.clone());
            bundle_marketplace
                .bundle_ids_per_item
                .insert(&(nft_address, token_id), &items);

            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace
                .bundle_ids
                .insert(&_bundle_id, &bundle_id);

            for (i, (&nft_address, &token_id)) in nft_addresses.iter().zip(&token_ids).enumerate() {
                bundle_marketplace
                    .nft_indices
                    .insert(&(_bundle_id.clone(), nft_address, token_id), &(i as u128));
            }

            bundle_marketplace.test_marketplace = caller;

            // assert_eq!( bundle_marketplace.validate_item_sold(
            // nft_address, token_id
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(bundle_marketplace
                .validate_item_sold(nft_address, token_id, quantity)
                .is_ok());
            assert_eq!(
                bundle_marketplace
                    .listings
                    .get(&(owner, _bundle_id.clone())),
                Some(Listing {
                    nft_addresses: vec![nft_addresses[0]],
                    token_ids: vec![token_ids[0]],
                    quantities: vec![quantities[0]],
                    pay_token,
                    price,
                    start_time,
                }),
            );
            assert_eq!(
                bundle_marketplace
                    .nft_indices
                    .get(&(_bundle_id.clone(), nft_address, token_id)),
                None,
            );
            assert_eq!(
                bundle_marketplace.nft_indices.get(&(
                    _bundle_id.clone(),
                    nft_addresses[0],
                    token_ids[0]
                )),
                Some(0),
            );
            assert_eq!(
                bundle_marketplace
                    .bundle_ids_per_item
                    .get(&(nft_address, token_id)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_updated_event(
                &emitted_events[0],
                caller,
                bundle_id,
                vec![nft_addresses[0]],
                vec![token_ids[0]],
                vec![quantities[0]],
                pay_token,
                price,
            );
        }

        #[ink::test]
        fn validate_item_sold_failed_if_the_caller_is_not_the_address_of_the_marketplace_contract()
        {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price = 1;
            let start_time = bundle_marketplace.get_now();
            let owner = caller;
            let bundle_id = String::from("1");
            let nft_addresses = vec![alice()];
            let token_ids = vec![1];
            let quantities = vec![quantity];
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);

            let mut items = bundle_marketplace.items_impl(nft_address, token_id);
            items.insert(_bundle_id.clone());
            bundle_marketplace
                .bundle_ids_per_item
                .insert(&(nft_address, token_id), &items);

            bundle_marketplace.owners.insert(&_bundle_id, &owner);
            bundle_marketplace.listings.insert(
                &(owner, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            bundle_marketplace
                .bundle_ids
                .insert(&_bundle_id, &bundle_id);

            bundle_marketplace
                .nft_indices
                .insert(&(_bundle_id.clone(), nft_address, token_id), &0);
            bundle_marketplace.test_marketplace = frank();
            assert_eq!(
                bundle_marketplace
                    .validate_item_sold(nft_address, token_id, quantity - 10)
                    .unwrap_err(),
                Error::SenderMustBeAuctionOrMarketplace
            );
        }

        #[ink::test]
        fn get_listing_works() {
            // Create a new contract instance.
            let mut bundle_marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let bundle_id = String::from("1");
            let nft_addresses = vec![frank()];
            let token_ids = vec![1];
            let quantities = vec![1];
            let pay_token = django();
            let price = 10;
            let start_time = 10;
            let _bundle_id = bundle_marketplace.get_bundle_id(&bundle_id);
            bundle_marketplace.listings.insert(
                &(caller, _bundle_id.clone()),
                &Listing {
                    nft_addresses: nft_addresses.clone(),
                    token_ids: token_ids.clone(),
                    quantities: quantities.clone(),
                    pay_token,
                    price,
                    start_time,
                },
            );
            // assert_eq!( bundle_marketplace.list_item(
            //   nft_address,
            //         token_id,
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         start_time,
            // ).unwrap_err(),Error::NotOwningItem);
            assert_eq!(
                bundle_marketplace.get_listing(caller, bundle_id),
                (nft_addresses, token_ids, quantities, price, start_time)
            );
        }

        fn assert_item_listed_event(
            event: &ink_env::test::EmittedEvent,
            expected_owner: AccountId,
            expected_bundle_id: String,
            expected_pay_token: AccountId,
            expected_price: Balance,
            expected_starting_time: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemListed(ItemListed {
                owner,
                bundle_id,
                pay_token,
                price,
                start_time,
            }) = decoded_event
            {
                assert_eq!(
                    owner, expected_owner,
                    "encountered invalid ItemListed.owner"
                );
                assert_eq!(
                    bundle_id, expected_bundle_id,
                    "encountered invalid ItemListed.bundle_id"
                );

                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid ItemListed.pay_token"
                );
                assert_eq!(
                    price, expected_price,
                    "encountered invalid ItemListed.price"
                );

                assert_eq!(
                    start_time, expected_starting_time,
                    "encountered invalid ItemListed.start_time"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ItemListed event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubBundleMarketplace::ItemListed",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::ItemListed::owner",
                    value: &expected_owner,
                }),
            ];

            let topics = event.topics.clone();
            for (n, (actual_topic, expected_topic)) in
                topics.iter().zip(expected_topics).enumerate()
            {
                let mut topic_hash = Hash::clear();
                let len = actual_topic.len();
                topic_hash.as_mut()[0..len].copy_from_slice(&actual_topic[0..len]);

                assert_eq!(
                    topic_hash, expected_topic,
                    "encountered invalid topic at {}",
                    n
                );
            }
        }

        fn assert_item_sold_event(
            event: &ink_env::test::EmittedEvent,
            expected_seller: AccountId,
            expected_buyer: AccountId,
            expected_bundle_id: String,
            expected_pay_token: AccountId,
            expected_unit_price: Balance,
            expected_price: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemSold(ItemSold {
                seller,
                buyer,
                bundle_id,
                pay_token,
                unit_price,
                price,
            }) = decoded_event
            {
                assert_eq!(
                    seller, expected_seller,
                    "encountered invalid ItemSold.seller"
                );
                assert_eq!(buyer, expected_buyer, "encountered invalid ItemSold.buyer");
                assert_eq!(
                    bundle_id, expected_bundle_id,
                    "encountered invalid ItemSold.bundle_id"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid ItemSold.pay_token"
                );

                assert_eq!(
                    unit_price, expected_unit_price,
                    "encountered invalid ItemSold.unit_price"
                );

                assert_eq!(price, expected_price, "encountered invalid ItemSold.price");
            } else {
                panic!("encountered unexpected event kind: expected a ItemSold event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubBundleMarketplace::ItemSold",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::ItemSold::seller",
                    value: &expected_seller,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::ItemSold::buyer",
                    value: &expected_buyer,
                }),
            ];

            let topics = event.topics.clone();
            for (n, (actual_topic, expected_topic)) in
                topics.iter().zip(expected_topics).enumerate()
            {
                let mut topic_hash = Hash::clear();
                let len = actual_topic.len();
                topic_hash.as_mut()[0..len].copy_from_slice(&actual_topic[0..len]);

                assert_eq!(
                    topic_hash, expected_topic,
                    "encountered invalid topic at {}",
                    n
                );
            }
        }

        fn assert_item_updated_event(
            event: &ink_env::test::EmittedEvent,
            expected_owner: AccountId,
            expected_bundle_id: String,
            expected_nft_addresses: Vec<AccountId>,
            expected_token_ids: Vec<TokenId>,
            expected_quantities: Vec<u128>,
            expected_pay_token: AccountId,
            expected_new_price: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemUpdated(ItemUpdated {
                owner,
                bundle_id,
                nft_addresses,
                token_ids,
                quantities,
                pay_token,
                new_price,
            }) = decoded_event
            {
                assert_eq!(
                    owner, expected_owner,
                    "encountered invalid ItemUpdated.owner"
                );
                assert_eq!(
                    bundle_id, expected_bundle_id,
                    "encountered invalid ItemUpdated.bundle_id"
                );
                assert_eq!(
                    nft_addresses, expected_nft_addresses,
                    "encountered invalid ItemUpdated.nft_addresses"
                );
                assert_eq!(
                    token_ids, expected_token_ids,
                    "encountered invalid ItemUpdated.token_ids"
                );
                assert_eq!(
                    quantities, expected_quantities,
                    "encountered invalid ItemUpdated.quantities"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid ItemUpdated.pay_token"
                );

                assert_eq!(
                    new_price, expected_new_price,
                    "encountered invalid ItemUpdated.new_price"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ItemUpdated event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubBundleMarketplace::ItemUpdated",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::ItemUpdated::owner",
                    value: &expected_owner,
                }),
            ];

            let topics = event.topics.clone();
            for (n, (actual_topic, expected_topic)) in
                topics.iter().zip(expected_topics).enumerate()
            {
                let mut topic_hash = Hash::clear();
                let len = actual_topic.len();
                topic_hash.as_mut()[0..len].copy_from_slice(&actual_topic[0..len]);

                assert_eq!(
                    topic_hash, expected_topic,
                    "encountered invalid topic at {}",
                    n
                );
            }
        }

        fn assert_item_canceled_event(
            event: &ink_env::test::EmittedEvent,
            expected_owner: AccountId,
            expected_bundle_id: String,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemCanceled(ItemCanceled { owner, bundle_id }) = decoded_event {
                assert_eq!(
                    owner, expected_owner,
                    "encountered invalid ItemCanceled.owner"
                );
                assert_eq!(
                    bundle_id, expected_bundle_id,
                    "encountered invalid ItemCanceled.bundle_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ItemCanceled event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubBundleMarketplace::ItemCanceled",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::ItemCanceled::owner",
                    value: &expected_owner,
                }),
            ];

            let topics = event.topics.clone();
            for (n, (actual_topic, expected_topic)) in
                topics.iter().zip(expected_topics).enumerate()
            {
                let mut topic_hash = Hash::clear();
                let len = actual_topic.len();
                topic_hash.as_mut()[0..len].copy_from_slice(&actual_topic[0..len]);

                assert_eq!(
                    topic_hash, expected_topic,
                    "encountered invalid topic at {}",
                    n
                );
            }
        }

        fn assert_offer_created_event(
            event: &ink_env::test::EmittedEvent,
            expected_creator: AccountId,
            expected_bundle_id: String,
            expected_pay_token: AccountId,
            expected_price: Balance,
            expected_deadline: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::OfferCreated(OfferCreated {
                creator,
                bundle_id,
                pay_token,
                price,
                deadline,
            }) = decoded_event
            {
                assert_eq!(
                    creator, expected_creator,
                    "encountered invalid OfferCreated.creator"
                );
                assert_eq!(
                    bundle_id, expected_bundle_id,
                    "encountered invalid OfferCreated.bundle_id"
                );

                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid OfferCreated.pay_token"
                );
                assert_eq!(
                    price, expected_price,
                    "encountered invalid OfferCreated.price"
                );

                assert_eq!(
                    deadline, expected_deadline,
                    "encountered invalid OfferCreated.deadline"
                );
            } else {
                panic!("encountered unexpected event kind: expected a OfferCreated event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubBundleMarketplace::OfferCreated",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::OfferCreated::creator",
                    value: &expected_creator,
                }),
            ];

            let topics = event.topics.clone();
            for (n, (actual_topic, expected_topic)) in
                topics.iter().zip(expected_topics).enumerate()
            {
                let mut topic_hash = Hash::clear();
                let len = actual_topic.len();
                topic_hash.as_mut()[0..len].copy_from_slice(&actual_topic[0..len]);

                assert_eq!(
                    topic_hash, expected_topic,
                    "encountered invalid topic at {}",
                    n
                );
            }
        }

        fn assert_offer_canceled_event(
            event: &ink_env::test::EmittedEvent,
            expected_creator: AccountId,
            expected_bundle_id: String,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::OfferCanceled(OfferCanceled { creator, bundle_id }) = decoded_event {
                assert_eq!(
                    creator, expected_creator,
                    "encountered invalid OfferCanceled.creator"
                );
                assert_eq!(
                    bundle_id, expected_bundle_id,
                    "encountered invalid OfferCanceled.bundle_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a OfferCanceled event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubBundleMarketplace::OfferCanceled",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubBundleMarketplace::OfferCanceled::creator",
                    value: &expected_creator,
                }),
            ];

            let topics = event.topics.clone();
            for (n, (actual_topic, expected_topic)) in
                topics.iter().zip(expected_topics).enumerate()
            {
                let mut topic_hash = Hash::clear();
                let len = actual_topic.len();
                topic_hash.as_mut()[0..len].copy_from_slice(&actual_topic[0..len]);

                assert_eq!(
                    topic_hash, expected_topic,
                    "encountered invalid topic at {}",
                    n
                );
            }
        }
        fn assert_update_platform_fee_event(
            event: &ink_env::test::EmittedEvent,
            expected_platform_fee: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdatePlatformFee(UpdatePlatformFee { platform_fee }) = decoded_event {
                assert_eq!(
                    platform_fee, expected_platform_fee,
                    "encountered invalid UpdatePlatformFee.platform_fee"
                );
            } else {
                panic!("encountered unexpected event kind: expected a UpdatePlatformFee event")
            }
        }

        fn assert_update_platform_fee_recipient_event(
            event: &ink_env::test::EmittedEvent,
            expected_fee_recipient: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdatePlatformFeeRecipient(UpdatePlatformFeeRecipient { fee_recipient }) =
                decoded_event
            {
                assert_eq!(
                    fee_recipient, expected_fee_recipient,
                    "encountered invalid UpdatePlatformFeeRecipient.fee_recipient"
                );
            } else {
                panic!("encountered unexpected event kind: expected a UpdatePlatformFeeRecipient event")
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

        fn encoded_into_hash<T>(entity: &T) -> Hash
        where
            T: scale::Encode,
        {
            use ink_env::{
                hash::{Blake2x256, CryptoHash, HashOutput},
                Clear,
            };
            let mut result = Hash::clear();
            let len_result = result.as_ref().len();
            let encoded = entity.encode();
            let len_encoded = encoded.len();
            if len_encoded <= len_result {
                result.as_mut()[..len_encoded].copy_from_slice(&encoded);
                return result;
            }
            let mut hash_output = <<Blake2x256 as HashOutput>::Type as Default>::default();
            <Blake2x256 as CryptoHash>::hash(&encoded, &mut hash_output);
            let copy_len = core::cmp::min(hash_output.len(), len_result);
            result.as_mut()[0..copy_len].copy_from_slice(&hash_output[0..copy_len]);
            result
        }
    }
}
