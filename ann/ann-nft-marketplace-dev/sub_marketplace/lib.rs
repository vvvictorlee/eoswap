//! # NFT Marketplace contract
//!
//! This is NFT Marketplace implementation.
//! The Marketplace is a market where where people can sell NFTs
//! — NFT Tradable/NFT Tradable Private Token (custom ERC 721 ) or Art Tradable/Art Tradable Private Token (custom ERC 1155 )tokens
//! at a fixed price.
//!
//! This is an open decentralized NFT Marketplace built with smart contracts powered by Substrate.
//! It basically consists in an open platform where each user can mint his own NFT and expose it on a marketplace.
//! This project consists in an open platform where each user can mint his own NFT and expose it on a marketplace by making an offer or buying NFT from others.
//!
//! ## Warning
//!
//! This contract is an *example*. It is neither audited nor endorsed for production use.
//! Do **not** rely on it to keep anything of value secure.
//!
//! ## Overview
//!
//!
//! There are two groups of users — (1) 's customers who'll set up the marketplace, and (2) the end users of customers' marketplaces.
//! To customers, the `Marketplace` can be set up like any of the other contract (e.g. 'NFT Collection') ,
//! or by directly consuming the open sourced marketplace smart contract.
//! To the end users of customers, the experience of using the marketplace will feel familiar to popular marketplace platforms .
//! The biggest difference in user experience will be that performing any action on the marketplace requires gas fees.
//! - customers
//!   - Deploy the marketplace contract like any other contract.
//!   - Can set 'platform fee'. This is collected on every sale — when a buyer buys tokens from a listing.
//! This platform fee is distributed to the platform fee recipient (set by a contract admin).
//! - End users of customers
//!   - Can list NFTs for sale at a fixed price.
//!   - Can edit an existing listing's parameters.
//!   - Can make offers to NFTs listed for a fixed price.
//!   - Must pay gas fees to perform any actions, including the actions just listed.
//!
//! The `Marketplace` contract supports both ERC20 currencies, and a chain's native token .
//! This means that any action that involves transferring token (e.g. buying a token from a  listing) can be performed with either an ERC20 token or the chain's native token.
//!
//!
//!
//! ## Error Handling
//!
//! Any function that modifies the state returns a `Result` type and does not changes the state
//! if the `Error` occurs.
//! The errors are defined as an `enum` type. Any other error or invariant violation
//! triggers a panic and therefore rolls back the transaction.
//!
//! ## NFT Marketplace
//!
//! After creating a new Marketplace contract instance , the function caller becomes the owner.
//!
//! Contract owners can registry collection royalty `register_collection_royalty` and update parameters `update_platform_fee`,
//! `update_platform_fee_recipient`,`update_address_registry`.
//!
//! ### Item Listings
//!
//! An NFT owner (or 'lister') can list their NFTs for sale at a fixed price.
//! A potential buyer can buy the NFT for the specified price, or make an offer to buy the listed NFTs for a different price or token,
//! which the lister can choose to accept.
//! The listed NFTs do not leave the wallet of the lister until a sale is executed with the seller and buyer's consent.
//! To list NFTs for sale, the lister must own the NFTs being listed, and approve the market to transfer the NFTs.
//! The latter lets the market transfer NFTs to a buyer who buys the NFTs for the accepted price.
//- **listings** are *low commitment*, high frequency listings; people constantly list and de-list their NFTs based on market trends.
//! So, the listed NFTs and offer amounts are *not* escrowed in the Marketplace to keep the seller's NFTs and the buyer's token liquid.
//! Allowing users to list NFTs for sale just by approvals gives them the freedom to list the same NFT in multiple marketplaces,
//!
//! ### Item Listings Cancellation
//!   canceling listed NFT
//!
//! ### Item Listings Updating
//!   updating listed NFT
//!
//! ### NFT Offer Creation
//!
//! The user can offer his NFT by specifying its price (in native token). If someone fulfills this offer,
//! then the ownership is transferred to a new owner.
//!
//! ### Offer Cancellation
//!
//! The user can cancel an offer he did in the past if in the end he does not want to sell his NFT or wants to adjust the price.
//!
//! ### Offer Acceptation
//!
//! The user can accept an offer .
//!
//! ### Item Buying
//! A user can buy those NFT which someone else offered. This will require paying the requested price (the native token will be transferred to the smart contract to be claimed later on).
//!
//! ### Royalty registeration
//! setting royalty
//!
//! ### Collection Royalty registeration
//!
//! setting collection royalty

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_marketplace::{SubMarketplace, SubMarketplaceRef};

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
mod sub_marketplace {
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
        /// The amount of NFTs of the given 'assetContract' and 'tokenId' to list for sale. For ERC721 NFTs, this is always 1.
        pub quantity: u128,
        // The address of the currency accepted by the listing. Either an ERC20 token or the chain's native token (e.g. ether on Ethereum mainnet).
        pub pay_token: AccountId,
        // The price per unit of NFT listed for sale.
        pub price_per_item: Balance,
        /// The unix timestamp after which NFTs can be bought from the listing.
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
        // The token in which the offer is made.
        pub pay_token: AccountId,
        ///The quantity of NFTs from the listing for which the offer is made. For ERC721 NFTs, this is always 1.
        pub quantity: u128,
        // The offered price per item.
        pub price_per_item: Balance,
        /// The unix timestamp after which the offer expires.
        pub deadline: u128,
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
    pub struct CollectionRoyalty {
        ///A royalty is a legally binding payment made to an individual or company for the ongoing use of their assets.
        pub royalty: u128,
        /// The creator of the Collection.
        pub creator: AccountId,
        /// The fee recipient of the Collection.
        pub fee_recipient: AccountId,
    }

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubMarketplace {
        /// NftAddress -> Token ID -> Minter
        minters: Mapping<(AccountId, TokenId), AccountId>,
        /// NftAddress -> Token ID -> Royalty
        royalties: Mapping<(AccountId, TokenId), u128>,
        /// NftAddress -> Token ID -> Owner -> Listing item
        listings: Mapping<(AccountId, TokenId, AccountId), Listing>,
        /// NftAddress -> Token ID -> Offerer -> Offer
        offers: Mapping<(AccountId, TokenId, AccountId), Offer>,
        /// NftAddress -> Royalty
        collection_royalties: Mapping<AccountId, CollectionRoyalty>,
        /// Address registry
        address_registry: AccountId,
        /// Platform fee
        platform_fee: Balance,
        /// Platform fee receipient
        fee_recipient: AccountId,
        /// The contract owner
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
        test_bundle_marketplace_validate_item_sold_failed: bool,
        test_auctions: Mapping<(AccountId, TokenId), (u128, bool)>,
        test_exists: Mapping<AccountId, bool>,
        test_bundle_marketplace: AccountId,
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
        SenderMustBeBundleMarketplace,
        NotOwneAndOrContractNotApproved,
        TransactionFailed,
        InvalidFeeRecipientAddress,
        ERC20TransferFromFeeAmountFailed,
        ERC20TransferFromRoyaltyFeeFailed,
        ERC20TransferFromCollectionRoyaltyFeeFailed,
        ERC20TransferFromPayAmountFailed,
        ERC721TransferFromTokenIdFailed,
        ERC1155TransferFromTokenIdFailed,
        BundleMarketplaceValidateItemSoldFailed,
        NoneOfNFTFactoryAddress,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubMarketplace result types.
    pub type Result<T> = core::result::Result<T, Error>;

    /// Event emitted when an item listed occurs.
    #[ink(event)]
    pub struct ItemListed {
        #[ink(topic)]
        pub owner: AccountId,
        #[ink(topic)]
        pub nft_address: AccountId,
        pub token_id: TokenId,
        pub quantity: u128,
        pub pay_token: AccountId,
        pub price_per_item: Balance,
        pub start_time: u128,
    }

    /// Event emitted when an item sold occurs.
    #[ink(event)]
    pub struct ItemSold {
        #[ink(topic)]
        pub seller: AccountId,
        #[ink(topic)]
        pub buyer: AccountId,
        #[ink(topic)]
        pub nft_address: AccountId,
        pub token_id: TokenId,
        pub quantity: u128,
        pub pay_token: AccountId,
        pub unit_price: Balance,
        pub price_per_item: Balance,
    }

    /// Event emitted when an item updated occurs.
    #[ink(event)]
    pub struct ItemUpdated {
        #[ink(topic)]
        pub owner: AccountId,
        #[ink(topic)]
        pub nft_address: AccountId,
        pub token_id: TokenId,
        pub pay_token: AccountId,
        pub new_price: Balance,
    }

    /// Event emitted when an item canceled occurs.
    #[ink(event)]
    pub struct ItemCanceled {
        #[ink(topic)]
        pub owner: AccountId,
        #[ink(topic)]
        pub nft_address: AccountId,
        pub token_id: TokenId,
    }

    /// Event emitted when an offer created occurs.
    #[ink(event)]
    pub struct OfferCreated {
        #[ink(topic)]
        pub creator: AccountId,
        #[ink(topic)]
        pub nft_address: AccountId,
        pub token_id: TokenId,
        pub quantity: u128,
        pub pay_token: AccountId,
        pub price_per_item: Balance,
        pub deadline: u128,
    }
    /// Event emitted when an offer canceled occurs.
    #[ink(event)]
    pub struct OfferCanceled {
        #[ink(topic)]
        pub creator: AccountId,
        #[ink(topic)]
        pub nft_address: AccountId,
        pub token_id: TokenId,
    }

    /// Event emitted when update platform fee occurs.
    #[ink(event)]
    pub struct UpdatePlatformFee {
        pub platform_fee: Balance,
    }
    /// Event emitted when update platform fee recipient occurs.
    #[ink(event)]
    pub struct UpdatePlatformFeeRecipient {
        pub fee_recipient: AccountId,
    }

    impl SubMarketplace {
        /// Creates a new marketplace contract.
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
        /// Method for listing NFT
        /// # Fields
        /// nft_address Address of NFT contract
        /// token_id Token ID of NFT
        /// quantity token amount to list (needed for ERC-1155 NFTs, set as 1 for ERC-721)
        /// pay_token Paying token
        /// price_per_item sale price for each iteam
        /// start_time scheduling for a future sale
        ///
        /// # Errors
        ///
        /// - If the quantity of the listing of `nft_address` and `token_id` and the caller is greater than zero.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If the caller is not the owner of the specified nft address and token id in the erc721 contract.
        /// - If the contract is not approved by the caller in the contract in the following ERC-721 standard.
        /// - If the contract is not approved by the caller in the contract in the following ERC-1155 standard.
        /// - If the quantity is greater than the balance of the contract in the `nft_address` contract in the following ERC-1155 standard.
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn list_item(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            quantity: u128,
            pay_token: AccountId,
            price_per_item: Balance,
            start_time: u128,
        ) -> Result<()> {
            let listing = self.listing_impl(nft_address, token_id, self.env().caller());
            ensure!(listing.quantity == 0, Error::AlreadyListed);

            if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                ensure!(
                    Some(self.env().caller()) == self.erc721_owner_of(nft_address, token_id)?,
                    Error::NotOwningItem
                );

                ensure!(
                    self.erc721_is_approved_for_all(
                        nft_address,
                        self.env().caller(),
                        self.env().account_id(),
                    )
                    .unwrap_or(false),
                    Error::ItemNotApproved
                );
            } else if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155) {
                ensure!(
                    quantity
                        <= self.erc1155_balance_of(nft_address, self.env().caller(), token_id)?,
                    Error::MustHoldEnoughNFTs
                );
                ensure!(
                    self.erc1155_is_approved_for_all(
                        nft_address,
                        self.env().caller(),
                        self.env().account_id(),
                    )
                    .unwrap_or(false),
                    Error::ItemNotApproved
                );
            } else {
                ensure!(false, Error::InvalidNFTAddress);
            }
            self.valid_pay_token(pay_token)?;
            self.listings.insert(
                (nft_address, token_id, self.env().caller()),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            self.env().emit_event(ItemListed {
                owner: self.env().caller(),
                nft_address,
                token_id,
                quantity,
                pay_token,
                price_per_item,
                start_time,
            });
            Ok(())
        }

        /// Method for canceling listed NFT
        /// # Fields
        /// nft_address Address of NFT contract
        /// token_id Token ID of NFT
        ///
        /// # Errors
        ///
        /// - If the quantity of the listing of `nft_address` and `token_id` and the caller is zero.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If the caller is not the owner of the specified nft contract address and token id where 'nft_address' is following erc721 standard.
        /// - If the quantity is greater than the balance of the caller in the `nft_address` contract in the following ERC-721 standard.
        #[ink(message)]
        pub fn cancel_listing(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> {
            let listing = self.listing_impl(nft_address, token_id, self.env().caller());
            ensure!(listing.quantity > 0, Error::NotListedItem);
            self._cancel_listing(nft_address, token_id, self.env().caller())?;
            Ok(())
        }

        /// Method for updating listed NFT
        /// # Fields
        /// nft_address Address of NFT contract
        /// token_id Token ID of NFT
        /// pay_token payment token
        /// new_price New sale price for each iteam
        ///
        /// # Errors
        ///
        /// - If the quantity of the listing of `nft_address` and `token_id` and the caller is zero.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If the caller is not the owner of the specified nft contract address and token id where 'nft_address ' is following erc721 standard.
        /// - If the quantity is greater than the balance of the caller in the `nft_address` contract in the following ERC-721 standard.
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        #[ink(message)]
        pub fn update_listing(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            pay_token: AccountId,
            new_price: Balance,
        ) -> Result<()> {
            let mut listing = self.listing_impl(nft_address, token_id, self.env().caller());
            ensure!(listing.quantity > 0, Error::NotListedItem);
            self.valid_owner(nft_address, token_id, self.env().caller(), listing.quantity)?;
            self.valid_pay_token(pay_token)?;
            listing.pay_token = pay_token;
            listing.price_per_item = new_price;
            self.listings
                .insert(&(nft_address, token_id, self.env().caller()), &listing);
            self.env().emit_event(ItemUpdated {
                owner: self.env().caller(),
                nft_address,
                token_id,
                pay_token,
                new_price,
            });
            Ok(())
        }

        /// Method for buying listed NFT
        /// # Fields
        /// nft_address NFT contract address
        /// token_id TokenId
        /// pay_token payment token
        /// owner the owner of the nft address and the token id
        ///
        /// # Errors
        ///
        /// - If the quantity of the listing of `nft_address` and `token_id` and `owner` is zero.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If the caller is not the owner of the specified nft contract address and token id where 'nft_address ' is following erc721 standard.
        /// - If the quantity is greater than the balance of the caller in the `nft_address` contract in the following ERC-721 standard.
        /// - If the `start_time` of the listing of `nft_address` and `token_id` and `owner` is greater than the current time.
        /// - If the `pay_token` is not the `pay_token` of the listing of `nft_address` and `token_id` and `owner` .
        /// - If it failed when the contract transfer platform fee to the `fee_recipient` in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer royalty fee to the `minter` in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer collectiion royalty fee to the `minter` in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer pay amount to the `owner` in the `pay_token` erc20 contract .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer token id to the `caller` in the `nft_address` erc721 contract .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer token id to the `caller` in the `nft_address` erc1155 contract .
        /// - If `bundle_marketplace_validate_item_sold_failed` execute failed .
        #[ink(message)]
        pub fn buy_item(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            pay_token: AccountId,
            owner: AccountId,
        ) -> Result<()> {
            let listing = self.listing_impl(nft_address, token_id, owner);
            ensure!(listing.quantity > 0, Error::NotListedItem);
            self.valid_owner(nft_address, token_id, owner, listing.quantity)?;

            ensure!(self.get_now() >= listing.start_time, Error::ItemNotBuyable);

            ensure!(listing.pay_token == pay_token, Error::InvalidPayToken);

            self._buy_item(nft_address, token_id, pay_token, owner)?;
            Ok(())
        }

        /// Method for offering item
        /// # Fields
        /// nft_address NFT contract address
        /// token_id TokenId
        /// quantity Quantity of items
        /// pay_token Paying token
        /// price_per_item Price per item
        /// deadline Offer expiration
        ///
        /// # Errors
        ///
        /// - If the quantity of the offer of `nft_address` and `token_id` and the caller is greater than zero.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If `start_time` of the specified nft contract address and token id where `nft_address` is greater than zero and `resulted` of the specified nft contract address and token id where `nft_address` is false.
        /// - If the `deadline` of the offer of `nft_address` and `token_id` and the caller is less than or equal to the current time.
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        #[ink(message)]
        pub fn create_offer(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            quantity: u128,
            pay_token: AccountId,
            price_per_item: Balance,
            deadline: u128,
        ) -> Result<()> {
            let offer = self.offer_impl(nft_address, token_id, self.env().caller());
            ensure!(
                offer.quantity == 0 || offer.deadline <= self.get_now(),
                Error::OfferAlreadyCreated
            );

            ensure!(
                self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721)
                    || self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155),
                Error::InvalidNFTAddress
            );

            let (start_time, resulted) = self.auction_start_time_resulted(nft_address, token_id)?;
            ensure!(
                0 == start_time || resulted,
                Error::CannotPlaceAnOfferIfAuctionIsGoingOn
            );

            ensure!(deadline > self.get_now(), Error::InvalidExpiration);
            self.valid_pay_token(pay_token)?;
            self.offers.insert(
                &(nft_address, token_id, self.env().caller()),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            self.env().emit_event(OfferCreated {
                creator: self.env().caller(),
                nft_address,
                token_id,
                quantity,
                pay_token,
                price_per_item,
                deadline,
            });
            Ok(())
        }
        /// Method for canceling the offer
        /// # Fields
        /// nft_address NFT contract address
        /// token_id TokenId
        ///
        /// # Errors
        ///
        /// - If the quantity of the offer of `nft_address` and `token_id` and the caller is zero and the `deadline` of the offer of `nft_address` and `token_id` and the caller is less than or equal to the current time.
        #[ink(message)]
        pub fn cancel_offer(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> {
            let offer = self.offer_impl(nft_address, token_id, self.env().caller());
            ensure!(
                offer.quantity > 0 || offer.deadline > self.get_now(),
                Error::OfferNotExistsOrExpired
            );
            self.offers
                .remove(&(nft_address, token_id, self.env().caller()));
            self.env().emit_event(OfferCanceled {
                creator: self.env().caller(),
                nft_address,
                token_id,
            });
            Ok(())
        }

        /// Method for accepting the offer
        /// # Fields
        /// nft_address NFT contract address
        /// token_id TokenId
        /// creator Offer creator address
        ///
        /// # Errors
        ///
        /// - If the quantity of the offer of `nft_address` and `token_id` and the caller is zero and the `deadline` of the offer of `nft_address` and `token_id` and the caller is less than or equal to the current time.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If the caller is not the owner of the specified nft contract address and token id where 'nft_address ' is following erc721 standard.
        /// - If the quantity is greater than the balance of the caller in the `nft_address` contract in the following ERC-721 standard.
        /// - If it failed when the contract transfer platform fee to the `fee_recipient` in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer royalty fee to the `minter` in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer pay amount to the `owner` in the `pay_token` erc20 contract .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer token id to the `caller` in the `nft_address` erc721 contract .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer token id to the `caller` in the `nft_address` erc1155 contract .
        #[ink(message)]
        pub fn accept_offer(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            creator: AccountId,
        ) -> Result<()> {
            let offer = self.offer_impl(nft_address, token_id, creator);
            ensure!(
                offer.quantity > 0 || offer.deadline > self.get_now(),
                Error::OfferNotExistsOrExpired
            );
            self.valid_owner(nft_address, token_id, self.env().caller(), offer.quantity)?;
            let price = offer.price_per_item * offer.quantity;
            let mut fee_amount = price * self.platform_fee / 1000;
            let minter = self.minter_of_impl(nft_address, token_id);
            let royalty = self.royalty_of_impl(nft_address, token_id);
            if minter != AccountId::from([0x0; 32]) && royalty != 0 {
                let royalty_fee = (price - fee_amount) * royalty / 10000;
                ensure!(
                    self.erc20_transfer_from(offer.pay_token, creator, minter, royalty_fee)
                        .is_ok(),
                    Error::ERC20TransferFromRoyaltyFeeFailed
                );
                fee_amount += royalty_fee;
            } else {
                let collection_royalty = self.collection_royalty_impl(nft_address);
                let minter = collection_royalty.fee_recipient;
                let royalty = collection_royalty.royalty;
                if minter != AccountId::from([0x0; 32]) && royalty != 0 {
                    let royalty_fee = (price - fee_amount) * royalty / 10000;
                    ensure!(
                        self.erc20_transfer_from(offer.pay_token, creator, minter, royalty_fee)
                            .is_ok(),
                        Error::ERC20TransferFromCollectionRoyaltyFeeFailed
                    );
                    fee_amount += royalty_fee;
                }
            }
            ensure!(
                self.erc20_transfer_from(
                    offer.pay_token,
                    creator,
                    self.env().caller(),
                    price - fee_amount,
                )
                .is_ok(),
                Error::ERC20TransferFromPayAmountFailed
            );

            // Transfer NFT to buyer
            if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                ensure!(
                    self.erc721_transfer_from(nft_address, self.env().caller(), creator, token_id)
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
                        offer.quantity,
                    )
                    .is_ok(),
                    Error::ERC1155TransferFromTokenIdFailed
                );
            }
            ensure!(
                self.bundle_marketplace_validate_item_sold(nft_address, token_id, offer.quantity)
                    .is_ok(),
                Error::BundleMarketplaceValidateItemSoldFailed
            );

            self.env().emit_event(ItemSold {
                seller: self.env().caller(),
                buyer: creator,
                nft_address,
                token_id,
                quantity: offer.quantity,
                pay_token: offer.pay_token,
                unit_price: self.get_price(offer.pay_token)?,
                price_per_item: offer.price_per_item,
            });
            self.env().emit_event(OfferCanceled {
                creator,
                nft_address,
                token_id,
            });
            self.listings
                .remove(&(nft_address, token_id, self.env().caller()));
            self.offers.remove(&(nft_address, token_id, creator));
            Ok(())
        }
        /// Method for register royalty
        /// # Fields
        /// nft_address NFT contract address
        /// token_id TokenId
        /// royalty Royalty
        ///
        /// # Errors
        ///
        /// - If `royalty` is greater than 10000.
        /// - If the `nft_address` is none of `nft_factory`,`nft_factory_private`,`art_factory`,`art_factory_private`.
        /// - If the `nft_address` support neither ERC-721 nor ERC-1155.
        /// - If the caller is not the owner of the specified nft contract address and token id where `nft_address ` is following erc721 standard.
        /// - If the quantity is greater than the balance of the caller in the `nft_address` contract in the following ERC-721 standard.
        /// - If the minter of `nft_address`, `token_id` exists in minters.
        #[ink(message)]
        pub fn register_royalty(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            royalty: u128,
        ) -> Result<()> {
            ensure!(royalty <= 10000, Error::InvalidRoyalty);
            ensure!(self.is_nft(nft_address), Error::NoneOfNFTFactoryAddress);
            self.valid_owner(nft_address, token_id, self.env().caller(), 1)?;

            ensure!(
                self.minters.get(&(nft_address, token_id)).is_none(),
                Error::RoyaltyAlreadySet
            );
            self.minters
                .insert(&(nft_address, token_id), &self.env().caller());
            self.royalties.insert(&(nft_address, token_id), &royalty);
            Ok(())
        }
        /// Method for setting collection royalty
        /// # Fields
        /// nft_address NFT contract address
        /// creator  creator
        /// royalty Royalty
        /// fee_recipient the fee recipient
        ///
        /// # Errors
        ///
        /// - If the caller is not the owner of the contract .
        /// - If `creator` is zero address.
        /// - If `royalty` is greater than 10000.
        /// - If `royalty` is greater than 0 and fee_recipient is zero address.
        /// - If the `nft_address` is none of 'nft_factory','nft_factory_private','art_factory','art_factory_private'.
        #[ink(message)]
        pub fn register_collection_royalty(
            &mut self,
            nft_address: AccountId,
            creator: AccountId,
            royalty: u128,
            fee_recipient: AccountId,
        ) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                AccountId::from([0x0; 32]) != creator,
                Error::InvalidCreatorAddress
            );
            ensure!(royalty <= 10000, Error::InvalidRoyalty);
            ensure!(
                royalty == 0 || AccountId::from([0x0; 32]) != fee_recipient,
                Error::InvalidFeeRecipientAddress
            );
            ensure!(self.is_nft(nft_address), Error::NoneOfNFTFactoryAddress);
            self.collection_royalties.insert(
                &nft_address,
                &CollectionRoyalty {
                    royalty,
                    creator,
                    fee_recipient,
                },
            );

            Ok(())
        }

        /// Method for updating platform fee
        /// # Fields
        /// Only admin
        /// platform_fee the platform fee to set
        ///
        /// # Errors
        ///
        /// - If the caller is not the owner of the contract .
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
        /// - If the caller is not the owner of the contract .
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
        /// - If the caller is not the owner of the contract .
        #[ink(message)]
        pub fn update_address_registry(&mut self, address_registry: AccountId) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.address_registry = address_registry;

            Ok(())
        }

        /// Validate and cancel listing
        /// Only bundle marketplace can access
        /// # Fields
        /// nft_address NFT contract address
        /// token_id Token Id
        /// seller the seller address of the item
        /// buyer the buyer address of the item
        ///
        /// # Errors
        ///
        /// - If the caller is not the address of the bundle_marketplace contract .
        /// - If `_cancel_listing` executes failed .
        #[ink(message)]
        pub fn validate_item_sold(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            seller: AccountId,
            buyer: AccountId,
        ) -> Result<()> {
            //onlyMarketplace
            #[cfg(test)]
            {
                ensure!(
                    self.env().caller() == self.test_bundle_marketplace,
                    Error::SenderMustBeBundleMarketplace
                );
            }
            #[cfg(not(test))]
            {
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);
                ensure!(
                    self.env().caller() == address_registry_instance.bundle_marketplace(),
                    Error::SenderMustBeBundleMarketplace
                );
            }
            let listing = self.listing_impl(nft_address, token_id, seller);
            if listing.quantity > 0 {
                self._cancel_listing(nft_address, token_id, seller)?;
            }
            self.offers.remove(&(nft_address, token_id, buyer));
            self.env().emit_event(OfferCanceled {
                creator: buyer,
                nft_address,
                token_id,
            });
            Ok(())
        }
        /// Method for getting the price of payment token from oracle price seed .
        /// # Fields
        /// pay_token Address of payment token
        #[ink(message)]
        #[cfg_attr(test, allow(unused_variables))]
        pub fn get_price(&self, pay_token: AccountId) -> Result<Balance> {
            #[cfg(test)]
            {
                Ok(1)
            }
            #[cfg(not(test))]
            {
                ensure!(
                    AccountId::from([0x0; 32]) != self.address_registry,
                    Error::InvalidPayToken
                );
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);

                let price_seed_instance: sub_price_seed::SubPriceSeedRef =
                    ink_env::call::FromAccountId::from_account_id(
                        address_registry_instance.price_seed(),
                    );
                let (mut unit_price, decimals) = if AccountId::from([0x0; 32]) == pay_token {
                    price_seed_instance.get_price(price_seed_instance.wrapped_token())
                } else {
                    price_seed_instance.get_price(pay_token)
                };
                if decimals < 12 {
                    unit_price *= 10u128.pow(12 - decimals);
                } else {
                    unit_price /= 10u128.pow(decimals - 12);
                }
                Ok(unit_price)
            }
        }
        /// Method for getting the minter of the specified owner and token id .
        /// # Fields
        /// owner the address of the token
        /// token_id the token id of the token
        #[ink(message)]
        pub fn minter_of(&self, owner: AccountId, token_id: TokenId) -> AccountId {
            self.minter_of_impl(owner, token_id)
        }
        /// Method for getting the royalty of the specified nft token contract address and token id .
        /// # Fields
        /// nft_address the address of the nft token contract
        /// token_id the token id of the token
        #[ink(message)]
        pub fn royalty_of(&self, nft_address: AccountId, token_id: TokenId) -> u128 {
            self.royalty_of_impl(nft_address, token_id)
        }
        /// Method for getting the collection royalty of the specified nft token contract address .
        /// # Fields
        /// nft_address the address of the nft token contract
        #[ink(message)]
        pub fn collection_royalty_of(&self, nft_address: AccountId) -> (AccountId, Balance) {
            let collection_royalty = self.collection_royalty_impl(nft_address);
            (collection_royalty.fee_recipient, collection_royalty.royalty)
        }
        /// Method for getting the listing of the specified nft token contract address .
        /// # Fields
        /// nft_address the address of the nft token contract
        /// token_id the token id of the token
        /// owner the address of the token
        #[ink(message)]
        pub fn listing_of(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
            owner: AccountId,
        ) -> Listing {
            self.listing_impl(nft_address, token_id, owner)
        }
        /// Method for getting the offer of the specified nft token contract address .
        /// # Fields
        /// nft_address the address of the nft token contract
        /// token_id the token id of the token
        /// owner the address of the token
        #[ink(message)]
        pub fn offer_of(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
            owner: AccountId,
        ) -> Offer {
            self.offer_impl(nft_address, token_id, owner)
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
        #[cfg_attr(test, allow(unused_variables))]
        pub fn test_bundle_marketplace_validate_item_sold(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            quantity: u128,
        ) -> Result<()> {
            self.bundle_marketplace_validate_item_sold(nft_address, token_id, quantity)
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
            )?;
            Ok(())
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
        /// Get current timestamp
        /// # Return
        ///  current time  timestamp (s)
        #[ink(message)]
        pub fn now_timestamp(&self) -> u128 {
            self.get_now()
        }
    }
    #[ink(impl)]
    impl SubMarketplace {
        /// Returns the minter for the specified `owner` and `token_id`.
        ///
        /// Returns `zero address` if the minter is non-existent.
        ///
        /// # Note
        ///
        /// Prefer to call this method over `minter_of` since this
        /// works using references which are more efficient in Wasm.
        #[inline]
        fn minter_of_impl(&self, owner: AccountId, token_id: TokenId) -> AccountId {
            self.minters.get(&(owner, token_id)).unwrap_or_default()
        }

        /// Returns the royalty for the specified `nft_address` and `token_id`.
        ///
        /// Returns `0` if the royalty is non-existent.
        ///
        /// # Note
        ///
        /// Prefer to call this method over `royalty_of` since this
        /// works using references which are more efficient in Wasm.
        #[inline]
        fn royalty_of_impl(&self, nft_address: AccountId, token_id: TokenId) -> u128 {
            self.royalties
                .get(&(nft_address, token_id))
                .unwrap_or_default()
        }
        /// Returns the listing for the specified `nft_address` and `token_id` and owner`.
        ///
        /// Returns `default listing ` if the listing is non-existent.
        ///
        /// # Note
        ///
        /// Prefer to call this method over `listing_of` since this
        /// works using references which are more efficient in Wasm.
        #[inline]
        fn listing_impl(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
            owner: AccountId,
        ) -> Listing {
            self.listings
                .get(&(nft_address, token_id, owner))
                .unwrap_or_default()
        }

        /// Returns the offer for the specified `nft_address` and `token_id` and owner`.
        ///
        /// Returns `default offer ` if the offer is non-existent.
        ///
        /// # Note
        ///
        /// Prefer to call this method over `offer_of` since this
        /// works using references which are more efficient in Wasm.
        #[inline]
        fn offer_impl(&self, nft_address: AccountId, token_id: TokenId, owner: AccountId) -> Offer {
            self.offers
                .get(&(nft_address, token_id, owner))
                .unwrap_or_default()
        }

        /// Returns the collection royalty for the specified `nft_address` .
        ///
        /// Returns `default collection royalty ` if the collection royalty is non-existent.
        ///
        /// # Note
        ///
        /// Prefer to call this method over `collection_royalty_of` since this
        /// works using references which are more efficient in Wasm.
        #[inline]
        fn collection_royalty_impl(&self, nft_address: AccountId) -> CollectionRoyalty {
            self.collection_royalties
                .get(&nft_address)
                .unwrap_or_default()
        }
        fn _cancel_listing(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            owner: AccountId,
        ) -> Result<()> {
            let listing = self.listing_impl(nft_address, token_id, owner);
            self.valid_owner(nft_address, token_id, owner, listing.quantity)?;
            self.listings.remove(&(nft_address, token_id, owner));
            self.env().emit_event(ItemCanceled {
                owner,
                nft_address,
                token_id,
            });
            Ok(())
        }
        fn _buy_item(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            pay_token: AccountId,
            owner: AccountId,
        ) -> Result<()> {
            let listing = self.listing_impl(nft_address, token_id, owner);
            let price = listing.price_per_item * listing.quantity;
            let mut fee_amount = price * self.platform_fee / 1000;
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
            let minter = self.minter_of_impl(nft_address, token_id);
            let royalty = self.royalty_of_impl(nft_address, token_id);
            if minter != AccountId::from([0x0; 32]) && royalty != 0 {
                let royalty_fee = (price - fee_amount) * royalty / 10000;
                ensure!(
                    self.erc20_transfer_from(pay_token, self.env().caller(), minter, royalty_fee)
                        .is_ok(),
                    Error::ERC20TransferFromRoyaltyFeeFailed
                );
                fee_amount += royalty_fee;
            } else {
                let collection_royalty = self.collection_royalty_impl(nft_address);
                let minter = collection_royalty.fee_recipient;
                let royalty = collection_royalty.royalty;
                if minter != AccountId::from([0x0; 32]) && royalty != 0 {
                    let royalty_fee = (price - fee_amount) * royalty / 10000;
                    ensure!(
                        self.erc20_transfer_from(
                            pay_token,
                            self.env().caller(),
                            minter,
                            royalty_fee
                        )
                        .is_ok(),
                        Error::ERC20TransferFromCollectionRoyaltyFeeFailed
                    );
                    fee_amount += royalty_fee;
                }
            }

            ensure!(
                self.erc20_transfer_from(pay_token, self.env().caller(), owner, price - fee_amount)
                    .is_ok(),
                Error::ERC20TransferFromPayAmountFailed
            );

            if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                ensure!(
                    self.erc721_transfer_from(nft_address, owner, self.env().caller(), token_id)
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
                        listing.quantity,
                    )
                    .is_ok(),
                    Error::ERC1155TransferFromTokenIdFailed
                );
            }
            ensure!(
                self.bundle_marketplace_validate_item_sold(nft_address, token_id, listing.quantity)
                    .is_ok(),
                Error::BundleMarketplaceValidateItemSoldFailed
            );

            self.env().emit_event(ItemSold {
                seller: owner,
                buyer: self.env().caller(),
                nft_address,
                token_id,
                quantity: listing.quantity,
                pay_token,
                unit_price: self.get_price(pay_token)?,
                price_per_item: price / listing.quantity,
            });
            self.listings.remove(&(nft_address, token_id, owner));
            Ok(())
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn auction_start_time_resulted(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> Result<(u128, bool)> {
            #[cfg(test)]
            {
                Ok(self
                    .test_auctions
                    .get(&(nft_address, token_id))
                    .unwrap_or_default())
            }
            #[cfg(not(test))]
            {
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x39, 0xF0, 0xAB, 0x3E]; //auction get_auction_start_time_resulted
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(address_registry_instance.auction())
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(nft_address)
                            .push_arg(token_id),
                    )
                    .returns::<(u128, bool)>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("auction_start_time_resulted= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn bundle_marketplace_validate_item_sold(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
            quantity: u128,
        ) -> Result<()> {
            let get_bundle_marketplace = || {
                #[cfg(test)]
                {
                    Ok(AccountId::from([0x0; 32]))
                }
                #[cfg(not(test))]
                {
                    let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);

                    ensure!(
                        AccountId::from([0x0; 32])
                            != address_registry_instance.bundle_marketplace(),
                        Error::InvalidPayToken
                    );
                    Ok(address_registry_instance.bundle_marketplace())
                }
            };
            let bundle_marketplace = get_bundle_marketplace()?;
            #[cfg(test)]
            {
                ensure!(
                    !self.test_bundle_marketplace_validate_item_sold_failed,
                    Error::TransactionFailed
                );
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x5E, 0x38, 0x31, 0x94]; //_bundle_marketplace_validate_item_sold
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(bundle_marketplace)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(nft_address)
                            .push_arg(token_id)
                            .push_arg(quantity),
                    )
                    .returns::<Result<()>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("_bundle_marketplace_validate_item_sold= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!(
                                "_bundle_marketplace_validate_item_sold= {:?}",
                                e
                            );
                            Error::TransactionFailed
                        })
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn valid_owner(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
            owner: AccountId,
            quantity: u128,
        ) -> Result<()> {
            if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC721) {
                ensure!(
                    Some(owner) == self.erc721_owner_of(nft_address, token_id)?,
                    Error::NotOwningItem
                );
            } else if self.supports_interface_check(nft_address, crate::INTERFACE_ID_ERC1155) {
                ensure!(
                    quantity <= self.erc1155_balance_of(nft_address, owner, token_id)?,
                    Error::NotOwningItem
                );
            } else {
                ensure!(false, Error::InvalidNFTAddress);
            }
            Ok(())
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
                let selector: [u8; 4] = [0x14, 0x14, 0x63, 0x1C]; //token_registry_enabled
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
        fn factory_exists(&self, callee: AccountId, token: AccountId) -> Result<bool> {
            #[cfg(test)]
            {
                Ok(self.test_exists.get(&token).unwrap_or_default())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0xCA, 0x94, 0x23, 0x1F]; //factory_exists
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
                        ink_env::debug_println!("factory_exists= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn is_nft(&self, nft_address: AccountId) -> bool {
            let get_addresses = || {
                #[cfg(test)]
                {
                    (
                        AccountId::from([0x0; 32]),
                        AccountId::from([0x0; 32]),
                        AccountId::from([0x0; 32]),
                        AccountId::from([0x0; 32]),
                        AccountId::from([0x0; 32]),
                    )
                }
                #[cfg(not(test))]
                {
                    let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);
                    (
                        address_registry_instance.artion(),
                        address_registry_instance.nft_factory(),
                        address_registry_instance.nft_factory_private(),
                        address_registry_instance.art_factory(),
                        address_registry_instance.art_factory_private(),
                    )
                }
            };
            let (artion, nft_factory, nft_factory_private, art_factory, art_factory_private) =
                get_addresses();
            nft_address == artion
                || self
                    .factory_exists(nft_factory, nft_address)
                    .unwrap_or(false)
                || self
                    .factory_exists(nft_factory_private, nft_address)
                    .unwrap_or(false)
                || self
                    .factory_exists(art_factory, nft_address)
                    .unwrap_or(false)
                || self
                    .factory_exists(art_factory_private, nft_address)
                    .unwrap_or(false)
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
    }
    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_env::Clear;
        use ink_lang as ink;
        type Event = <SubMarketplace as ::ink_lang::reflect::ContractEventBase>::Type;

        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<ink_env::DefaultEnvironment>(sender);
        }
        fn default_accounts() -> ink_env::test::DefaultAccounts<Environment> {
            ink_env::test::default_accounts::<Environment>()
        }
        fn contract_id() -> AccountId {
            ink_env::test::callee::<ink_env::DefaultEnvironment>()
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
        fn init_contract() -> SubMarketplace {
            let erc = SubMarketplace::new(10, fee_recipient());

            erc
        }
        #[ink::test]
        fn list_item_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = eve();
            let token_id = 1;
            let quantity = 300;
            let pay_token = frank();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            marketplace.test_enabled.insert(&pay_token, &true);
            marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            // assert_eq!( marketplace.list_item(
            //  nft_address,
            //     token_id,
            //     quantity,
            //     pay_token,
            //     price_per_item,
            //     start_time,
            // ).unwrap_err(),Error::NotOwningItem);

            assert!(marketplace
                .list_item(
                    nft_address,
                    token_id,
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                )
                .is_ok());

            assert_eq!(
                marketplace.listings.get(&(nft_address, token_id, caller)),
                Some(Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                })
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_listed_event(
                &emitted_events[0],
                caller,
                nft_address,
                token_id,
                quantity,
                pay_token,
                price_per_item,
                start_time,
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_quantity_of_the_listing_of_nft_address_and_token_id_and_the_caller_is_greater_than_zero(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = django();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        start_time,
                    )
                    .unwrap_err(),
                Error::AlreadyListed
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        start_time,
                    )
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        start_time,
                    )
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn list_item_failed_if_contract_is_not_approved_by_the_caller_in_the_contract_in_the_following_erc_721_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
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
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        start_time,
                    )
                    .unwrap_err(),
                Error::MustHoldEnoughNFTs
            );
        }

        #[ink::test]
        fn list_item_failed_if_contract_is_not_approved_by_the_caller_in_the_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace
                .test_balances
                .insert(&(nft_address, token_id), &quantity);
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        start_time,
                    )
                    .unwrap_err(),
                Error::ItemNotApproved
            );
        }

        #[ink::test]
        fn list_item_failed_if_the_pay_token_is_not_enabled_in_the_token_registry_contract_when_the_pay_token_is_not_zero_address(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            marketplace
                .test_balances
                .insert(&(nft_address, token_id), &quantity);
            assert_eq!(
                marketplace
                    .list_item(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        start_time,
                    )
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn cancel_listing_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);

            // assert_eq!( marketplace.cancel_listing(
            //  nft_address,
            //     token_id,
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace.cancel_listing(nft_address, token_id).is_ok());

            assert_eq!(
                marketplace.listings.get(&(nft_address, token_id, caller)),
                None
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_canceled_event(&emitted_events[0], caller, nft_address, token_id);
        }

        #[ink::test]
        fn cancel_listing_failed_if_the_quantity_of_the_listing_of_nft_address_and_token_id_and_the_caller_is_zero(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            // let quantity = 300;
            // let pay_token = alice();
            // let price_per_item = 10;
            // let start_time = 10;
            // marketplace.listings.insert(
            //     &(nft_address, token_id, caller),
            //     &Listing {
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         start_time,
            //     },
            // );
            assert_eq!(
                marketplace
                    .cancel_listing(nft_address, token_id)
                    .unwrap_err(),
                Error::NotListedItem
            );
        }

        #[ink::test]
        fn cancel_listing_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            assert_eq!(
                marketplace
                    .cancel_listing(nft_address, token_id)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn cancel_listing_failed_if_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            assert_eq!(
                marketplace
                    .cancel_listing(nft_address, token_id)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn cancel_listing_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            assert_eq!(
                marketplace
                    .cancel_listing(nft_address, token_id)
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn update_listing_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            let new_price = 11;
            let new_pay_token = bob();
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            marketplace.test_enabled.insert(&new_pay_token, &true);
            // assert_eq!( marketplace.update_listing(
            //  nft_address,
            //     token_id, new_pay_token, new_price
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace
                .update_listing(nft_address, token_id, new_pay_token, new_price)
                .is_ok());

            assert_eq!(
                marketplace.listings.get(&(nft_address, token_id, caller)),
                Some(Listing {
                    quantity,
                    pay_token: new_pay_token,
                    price_per_item: new_price,
                    start_time,
                })
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_updated_event(
                &emitted_events[0],
                caller,
                nft_address,
                token_id,
                new_pay_token,
                new_price,
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_quantity_of_the_listing_of_nft_address_and_token_id_and_the_caller_is_zero(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            // let quantity = 300;
            // let pay_token = alice();
            // let price_per_item = 10;
            // let start_time = 10;
            let new_price = 11;
            let new_pay_token = bob();
            // marketplace.listings.insert(
            //     &(nft_address, token_id, caller),
            //     &Listing {
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         start_time,
            //     },
            // );

            assert_eq!(
                marketplace
                    .update_listing(nft_address, token_id, new_pay_token, new_price)
                    .unwrap_err(),
                Error::NotListedItem
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            let new_price = 11;
            let new_pay_token = bob();
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;

            assert_eq!(
                marketplace
                    .update_listing(nft_address, token_id, new_pay_token, new_price)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            let new_price = 11;
            let new_pay_token = bob();
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;

            assert_eq!(
                marketplace
                    .update_listing(nft_address, token_id, new_pay_token, new_price)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            let new_price = 11;
            let new_pay_token = bob();
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );

            assert_eq!(
                marketplace
                    .update_listing(nft_address, token_id, new_pay_token, new_price)
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn update_listing_failed_if_the_pay_token_is_not_enabled_in_the_token_registry_contract_when_the_pay_token_is_not_zero_address(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 10;
            let start_time = 10;
            let new_price = 11;
            let new_pay_token = bob();
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            marketplace
                .test_balances
                .insert(&(nft_address, token_id), &quantity);
            assert_eq!(
                marketplace
                    .update_listing(nft_address, token_id, new_pay_token, new_price)
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn buy_item_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            // assert_eq!( marketplace.buy_item(
            //  nft_address,
            //     token_id, pay_token, owner
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace
                .buy_item(nft_address, token_id, pay_token, owner)
                .is_ok());

            assert_eq!(
                marketplace.listings.get(&(nft_address, token_id, owner)),
                None
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_item_sold_event(
                &emitted_events[0],
                owner,
                caller,
                nft_address,
                token_id,
                quantity,
                pay_token,
                unit_price,
                price_per_item,
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_quantity_of_the_listing_of_nft_address_and_token_id_and_the_caller_is_zero(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let pay_token = alice();
            let owner = bob();
            // let quantity = 300;
            //      let price_per_item = 1;
            // let start_time = marketplace.get_now();
            // let unit_price = 1;
            // marketplace.listings.insert(
            //     &(nft_address, token_id, owner),
            //     &Listing {
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         start_time,
            //     },
            // );

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::NotListedItem
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_start_time_of_the_listing_of_nft_address_and_token_id_and_owner_is_greater_than_the_current_time(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now() + 1;
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ItemNotBuyable
            );
        }

        #[ink::test]
        fn buy_item_failed_if_the_pay_token_is_not_the_pay_token_of_the_listing_of_nft_address_and_token_id_and_owner(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, frank(), owner)
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_platform_fee_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, fee_recipient(), 0, fee_amount), &());
            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ERC20TransferFromFeeAmountFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_royalty_fee_to_the_minter_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );

            let minter = eve();
            let royalty = 1;
            marketplace
                .minters
                .insert(&(nft_address, token_id), &minter);
            marketplace
                .royalties
                .insert(&(nft_address, token_id), &royalty);

            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            let royalty_fee = (price - fee_amount) * royalty / 10000;
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, minter, 0, royalty_fee), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ERC20TransferFromRoyaltyFeeFailed
            );
        }
        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_collection_royalty_fee_to_the_minter_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            let (creator, fee_recipient, royalty) = (eve(), eve(), 1);
            marketplace.collection_royalties.insert(
                &nft_address,
                &CollectionRoyalty {
                    royalty,
                    creator,
                    fee_recipient,
                },
            );

            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            let royalty_fee = (price - fee_amount) * royalty / 10000;
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, fee_recipient, 0, royalty_fee), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ERC20TransferFromCollectionRoyaltyFeeFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_owner_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            let minter = eve();
            let royalty = 1;
            marketplace
                .minters
                .insert(&(nft_address, token_id), &minter);
            marketplace
                .royalties
                .insert(&(nft_address, token_id), &royalty);

            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            // let royalty_fee = (price - fee_amount) * royalty / 10000;

            marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, owner, 0, price - fee_amount), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ERC20TransferFromPayAmountFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );

            marketplace
                .test_transfer_fail
                .insert(&(pay_token, owner, caller, token_id, 0), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ERC721TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_it_failed_when_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc1155_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );

            marketplace
                .test_transfer_fail
                .insert(&(pay_token, owner, caller, token_id, quantity), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            marketplace
                .test_balances
                .insert(&(owner, token_id), &quantity);

            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::ERC1155TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn buy_item_failed_if_bundle_marketplace_validate_item_sold_failed() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let start_time = marketplace.get_now();
            let owner = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, owner),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.test_bundle_marketplace_validate_item_sold_failed = true;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &owner);
            assert_eq!(
                marketplace
                    .buy_item(nft_address, token_id, pay_token, owner)
                    .unwrap_err(),
                Error::BundleMarketplaceValidateItemSoldFailed
            );
        }

        #[ink::test]
        fn create_offer_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_enabled.insert(&pay_token, &true);
            // let owner = bob();
            // let unit_price = 1;

            // assert_eq!( marketplace.create_offer(
            // nft_address, token_id, pay_token, quantity,price_per_item,deadline
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace
                .create_offer(
                    nft_address,
                    token_id,
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline
                )
                .is_ok());
            assert_eq!(
                marketplace.offers.get(&(nft_address, token_id, caller)),
                Some(Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                }),
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_offer_created_event(
                &emitted_events[0],
                caller,
                nft_address,
                token_id,
                quantity,
                pay_token,
                price_per_item,
                deadline,
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_quantity_of_the_offer_of_nft_address_and_token_id_and_the_caller_is_greater_than_zero(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            // let owner = bob();
            // let unit_price = 1;
            marketplace.offers.insert(
                &(nft_address, token_id, caller),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            assert_eq!(
                marketplace
                    .create_offer(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        deadline
                    )
                    .unwrap_err(),
                Error::OfferAlreadyCreated
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            // let owner = bob();
            // let unit_price = 1;
            assert_eq!(
                marketplace
                    .create_offer(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        deadline
                    )
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn create_offer_failed_if_start_time_of_the_specified_nft_contract_address_and_token_id_where_nft_address_is_greater_than_zero_and_resulted_of_the_specified_nft_contract_address_and_token_id_where_nft_address_is_false(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            // let owner = bob();
            // let unit_price = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace
                .test_auctions
                .insert(&(nft_address, token_id), &(1, false));
            assert_eq!(
                marketplace
                    .create_offer(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        deadline
                    )
                    .unwrap_err(),
                Error::CannotPlaceAnOfferIfAuctionIsGoingOn
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_deadline_of_the_offer_of_nft_address_and_token_id_and_the_caller_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now();
            // let owner = bob();
            // let unit_price = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;

            assert_eq!(
                marketplace
                    .create_offer(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        deadline
                    )
                    .unwrap_err(),
                Error::InvalidExpiration
            );
        }

        #[ink::test]
        fn create_offer_failed_if_the_pay_token_is_not_the_pay_token_of_the_listing_of_nft_address_and_token_id_and_owner(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            // let owner = bob();
            // let unit_price = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            assert_eq!(
                marketplace
                    .create_offer(
                        nft_address,
                        token_id,
                        quantity,
                        pay_token,
                        price_per_item,
                        deadline
                    )
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn cancel_offer_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            // let owner = bob();
            // let unit_price = 1;
            marketplace.offers.insert(
                &(nft_address, token_id, caller),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            // assert_eq!( marketplace.cancel_offer(
            // nft_address, token_id
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace.cancel_offer(nft_address, token_id).is_ok());
            assert_eq!(
                marketplace.offers.get(&(nft_address, token_id, caller)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_offer_canceled_event(&emitted_events[0], caller, nft_address, token_id);
        }

        #[ink::test]
        fn cancel_offer_failed_if_the_quantity_of_the_offer_of_nft_address_and_token_id_and_the_caller_is_zero_and_the_deadline_of_the_offer_of_nft_address_and_token_id_and_the_caller_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            // let quantity = 300;
            // let pay_token = alice();
            // let price_per_item = 1;
            // let deadline = marketplace.get_now() + 1;
            // let owner = bob();
            // let unit_price = 1;
            // marketplace.offers.insert(
            //     &(nft_address, token_id, caller),
            //     &Offer {
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         deadline,
            //     },
            // );
            assert_eq!(
                marketplace.cancel_offer(nft_address, token_id).unwrap_err(),
                Error::OfferNotExistsOrExpired
            );
        }

        #[ink::test]
        fn accept_offer_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            // assert_eq!( marketplace.accept_offer(
            // nft_address, token_id
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace
                .accept_offer(nft_address, token_id, creator)
                .is_ok());
            assert_eq!(
                marketplace.listings.get(&(nft_address, token_id, caller)),
                None,
            );
            assert_eq!(
                marketplace.offers.get(&(nft_address, token_id, creator)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_item_sold_event(
                &emitted_events[0],
                caller,
                creator,
                nft_address,
                token_id,
                quantity,
                pay_token,
                unit_price,
                price_per_item,
            );
            assert_offer_canceled_event(&emitted_events[1], creator, nft_address, token_id);
        }

        #[ink::test]
        fn accept_offer_failed_if_quantity_of_the_offer_of_nft_address_and_token_id_and_the_caller_is_zero_and_the_deadline_of_the_offer_of_nft_address_and_token_id_and_the_caller_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let creator = bob();
            // let quantity = 300;
            // let pay_token = alice();
            // let price_per_item = 1;
            // let deadline = marketplace.get_now() + 1;
            // let start_time = marketplace.get_now();
            //let unit_price = 1;
            // marketplace.listings.insert(
            //     &(nft_address, token_id, caller),
            //     &Listing {
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         start_time,
            //     },
            // );
            // marketplace.offers.insert(
            //     &(nft_address, token_id, creator),
            //     &Offer {
            //         quantity,
            //         pay_token,
            //         price_per_item,
            //         deadline,
            //     },
            // );
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::OfferNotExistsOrExpired
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;

            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_the_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;

            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );

            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_contract_transfer_royalty_fee_to_the_minter_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            let minter = eve();
            let royalty = 1;
            marketplace
                .minters
                .insert(&(nft_address, token_id), &minter);
            marketplace
                .royalties
                .insert(&(nft_address, token_id), &royalty);

            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            let royalty_fee = (price - fee_amount) * royalty / 10000;
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, creator, minter, 0, royalty_fee), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::ERC20TransferFromRoyaltyFeeFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_contract_transfer_collection_royalty_fee_to_the_minter_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            let (fee_recipient, royalty) = (eve(), 1);
            marketplace.collection_royalties.insert(
                &nft_address,
                &CollectionRoyalty {
                    royalty,
                    creator,
                    fee_recipient,
                },
            );

            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            let royalty_fee = (price - fee_amount) * royalty / 10000;
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, creator, fee_recipient, 0, royalty_fee), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::ERC20TransferFromCollectionRoyaltyFeeFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_owner_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            let minter = eve();
            let royalty = 1;
            marketplace
                .minters
                .insert(&(nft_address, token_id), &minter);
            marketplace
                .royalties
                .insert(&(nft_address, token_id), &royalty);

            let price = price_per_item * quantity;
            let fee_amount = price * marketplace.platform_fee / 1000;
            // let royalty_fee = (price - fee_amount) * royalty / 10000;

            marketplace
                .test_transfer_fail
                .insert(&(pay_token, creator, caller, 0, price - fee_amount), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::ERC20TransferFromPayAmountFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, creator, token_id, 0), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::ERC721TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_it_failed_when_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_caller_in_the_nft_address_erc1155_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace
                .test_transfer_fail
                .insert(&(pay_token, caller, creator, token_id, quantity), &());
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            marketplace
                .test_balances
                .insert(&(caller, token_id), &quantity);
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::ERC1155TransferFromTokenIdFailed
            );
        }

        #[ink::test]
        fn accept_offer_failed_if_bundle_marketplace_validate_item_sold_failed() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let creator = bob();
            //let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, caller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, creator),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace.test_bundle_marketplace_validate_item_sold_failed = true;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                marketplace
                    .accept_offer(nft_address, token_id, creator)
                    .unwrap_err(),
                Error::BundleMarketplaceValidateItemSoldFailed
            );
        }

        #[ink::test]
        fn register_royalty_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            marketplace.test_exists.insert(&nft_address, &true);
            assert!(marketplace
                .register_royalty(nft_address, token_id, royalty)
                .is_ok());
            assert_eq!(
                marketplace.minters.get(&(nft_address, token_id)),
                Some(caller),
            );
            assert_eq!(
                marketplace.royalties.get(&(nft_address, token_id)),
                Some(royalty),
            );
        }

        #[ink::test]
        fn register_royalty_failed_if_the_royalty_is_greater_than_10000() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 100001;

            assert_eq!(
                marketplace
                    .register_royalty(nft_address, token_id, royalty)
                    .unwrap_err(),
                Error::InvalidRoyalty
            );
        }

        #[ink::test]
        fn register_royalty_failed_if_the_nft_address_is_none_of_nft_factory_nft_factory_private_art_factory_art_factory_private(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 1;

            assert_eq!(
                marketplace
                    .register_royalty(nft_address, token_id, royalty)
                    .unwrap_err(),
                Error::NoneOfNFTFactoryAddress
            );
        }
        #[ink::test]
        fn register_royalty_failed_if_the_caller_is_not_the_owner_of_the_specified_nft_address_and_token_id_in_the_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_exists.insert(&nft_address, &true);

            assert_eq!(
                marketplace
                    .register_royalty(nft_address, token_id, royalty)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }
        #[ink::test]
        fn register_royalty_failed_if_the_the_quantity_is_greater_than_the_balance_of_the_contract_in_the_nft_address_contract_in_the_following_erc_1155_standard(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC1155;
            marketplace.test_exists.insert(&nft_address, &true);

            assert_eq!(
                marketplace
                    .register_royalty(nft_address, token_id, royalty)
                    .unwrap_err(),
                Error::NotOwningItem
            );
        }
        #[ink::test]
        fn register_royalty_failed_if_the_nft_address_support_neither_erc_721_nor_erc_1155() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 1;
            marketplace.test_exists.insert(&nft_address, &true);

            assert_eq!(
                marketplace
                    .register_royalty(nft_address, token_id, royalty)
                    .unwrap_err(),
                Error::InvalidNFTAddress
            );
        }
        #[ink::test]
        fn register_royalty_failed_if_the_minter_of_nft_address_token_id_exists_in_minters() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let royalty = 1;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &caller);
            marketplace.test_exists.insert(&nft_address, &true);
            marketplace
                .minters
                .insert(&(nft_address, token_id), &caller);
            assert_eq!(
                marketplace
                    .register_royalty(nft_address, token_id, royalty)
                    .unwrap_err(),
                Error::RoyaltyAlreadySet
            );
        }

        #[ink::test]
        fn register_collection_royalty_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let creator = bob();
            let fee_recipient = charlie();
            let royalty = 1;
            marketplace.test_exists.insert(&nft_address, &true);
            assert!(marketplace
                .register_collection_royalty(nft_address, creator, royalty, fee_recipient)
                .is_ok());
            assert_eq!(
                marketplace.collection_royalties.get(&nft_address),
                Some(CollectionRoyalty {
                    royalty,
                    creator,
                    fee_recipient,
                }),
            );
        }

        #[ink::test]
        fn register_collection_royalty_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = bob();
            set_caller(caller);
            let nft_address = alice();
            let creator = bob();
            let fee_recipient = charlie();
            let royalty = 1;

            assert_eq!(
                marketplace
                    .register_collection_royalty(nft_address, creator, royalty, fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn register_collection_royalty_failed_if_the_creator_is_zero_address() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let creator = AccountId::from([0x0; 32]);
            let fee_recipient = charlie();
            let royalty = 1;

            assert_eq!(
                marketplace
                    .register_collection_royalty(nft_address, creator, royalty, fee_recipient)
                    .unwrap_err(),
                Error::InvalidCreatorAddress
            );
        }

        #[ink::test]
        fn register_collection_royalty_failed_if_the_royalty_is_greater_than_10000() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let creator = bob();
            let fee_recipient = charlie();
            let royalty = 100001;

            assert_eq!(
                marketplace
                    .register_collection_royalty(nft_address, creator, royalty, fee_recipient)
                    .unwrap_err(),
                Error::InvalidRoyalty
            );
        }

        #[ink::test]
        fn register_collection_royalty_failed_if_the_oyalty_is_greater_than_0_and_fee_recipient_is_zero_address(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let creator = bob();
            let fee_recipient = AccountId::from([0x0; 32]);
            let royalty = 1;

            assert_eq!(
                marketplace
                    .register_collection_royalty(nft_address, creator, royalty, fee_recipient)
                    .unwrap_err(),
                Error::InvalidFeeRecipientAddress
            );
        }

        #[ink::test]
        fn register_collection_royalty_failed_if_the_nft_address_is_none_of_nft_factory_nft_factory_private_art_factory_art_factory_private(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let creator = bob();
            let fee_recipient = charlie();
            let royalty = 1;

            assert_eq!(
                marketplace
                    .register_collection_royalty(nft_address, creator, royalty, fee_recipient)
                    .unwrap_err(),
                Error::NoneOfNFTFactoryAddress
            );
        }

        #[ink::test]
        fn update_platform_fee_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let platform_fee = 10;
            assert!(marketplace.update_platform_fee(platform_fee).is_ok());

            assert_eq!(marketplace.platform_fee, platform_fee);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_update_platform_fee_event(&emitted_events[0], platform_fee);
        }

        #[ink::test]
        fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = bob();
            set_caller(caller);
            let platform_fee = 10;
            assert_eq!(
                marketplace.update_platform_fee(platform_fee).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_recipient_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let fee_recipient = bob();
            assert!(marketplace
                .update_platform_fee_recipient(fee_recipient)
                .is_ok());

            assert_eq!(marketplace.fee_recipient, fee_recipient);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_update_platform_fee_recipient_event(&emitted_events[0], fee_recipient);
        }

        #[ink::test]
        fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = eve();
            set_caller(caller);
            let fee_recipient = bob();
            assert_eq!(
                marketplace
                    .update_platform_fee_recipient(fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_address_registry_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let address_registry = bob();
            assert!(marketplace
                .update_address_registry(address_registry)
                .is_ok());

            assert_eq!(marketplace.address_registry, address_registry);
        }

        #[ink::test]
        fn update_address_registry_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = eve();
            set_caller(caller);
            let address_registry = bob();
            assert_eq!(
                marketplace
                    .update_address_registry(address_registry)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn validate_item_sold_works() {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let buyer = bob();
            let seller = charlie();
            // let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, seller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, buyer),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace.test_bundle_marketplace = caller;
            marketplace.test_support_interface = crate::INTERFACE_ID_ERC721;
            marketplace.test_token_owner.insert(&token_id, &seller);
            // assert_eq!( marketplace.validate_item_sold(
            // nft_address, token_id, seller, buyer
            // ).unwrap_err(),Error::NotOwningItem);
            assert!(marketplace
                .validate_item_sold(nft_address, token_id, seller, buyer)
                .is_ok());
            assert_eq!(
                marketplace.listings.get(&(nft_address, token_id, seller)),
                None,
            );
            assert_eq!(
                marketplace.offers.get(&(nft_address, token_id, buyer)),
                None,
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_item_canceled_event(&emitted_events[0], seller, nft_address, token_id);
            assert_offer_canceled_event(&emitted_events[1], buyer, nft_address, token_id);
        }

        #[ink::test]
        fn validate_item_sold_failed_if_the_caller_is_not_the_address_of_the_bundle_marketplace_contract(
        ) {
            // Create a new contract instance.
            let mut marketplace = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address = alice();
            let token_id = 1;
            let quantity = 300;
            let pay_token = alice();
            let price_per_item = 1;
            let deadline = marketplace.get_now() + 1;
            let start_time = marketplace.get_now();
            let buyer = bob();
            let seller = charlie();
            // let unit_price = 1;
            marketplace.listings.insert(
                &(nft_address, token_id, seller),
                &Listing {
                    quantity,
                    pay_token,
                    price_per_item,
                    start_time,
                },
            );
            marketplace.offers.insert(
                &(nft_address, token_id, buyer),
                &Offer {
                    quantity,
                    pay_token,
                    price_per_item,
                    deadline,
                },
            );
            marketplace.test_bundle_marketplace = frank();
            assert_eq!(
                marketplace
                    .validate_item_sold(nft_address, token_id, seller, buyer)
                    .unwrap_err(),
                Error::SenderMustBeBundleMarketplace
            );
        }

        fn assert_item_listed_event(
            event: &ink_env::test::EmittedEvent,
            expected_owner: AccountId,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_quantity: u128,
            expected_pay_token: AccountId,
            expected_price_per_item: Balance,
            expected_starting_time: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemListed(ItemListed {
                owner,
                nft_address,
                token_id,
                quantity,
                pay_token,
                price_per_item,
                start_time,
            }) = decoded_event
            {
                assert_eq!(
                    owner, expected_owner,
                    "encountered invalid ItemListed.owner"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid ItemListed.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid ItemListed.token_id"
                );
                assert_eq!(
                    quantity, expected_quantity,
                    "encountered invalid ItemListed.quantity"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid ItemListed.pay_token"
                );
                assert_eq!(
                    price_per_item, expected_price_per_item,
                    "encountered invalid ItemListed.price_per_item"
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
                    value: b"SubMarketplace::ItemListed",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemListed::owner",
                    value: &expected_owner,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemListed::nft_address",
                    value: &expected_nft_address,
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
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_quantity: u128,
            expected_pay_token: AccountId,
            expected_unit_price: Balance,
            expected_price_per_item: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemSold(ItemSold {
                seller,
                buyer,
                nft_address,
                token_id,
                quantity,
                pay_token,
                unit_price,
                price_per_item,
            }) = decoded_event
            {
                assert_eq!(
                    seller, expected_seller,
                    "encountered invalid ItemSold.seller"
                );
                assert_eq!(buyer, expected_buyer, "encountered invalid ItemSold.buyer");
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid ItemSold.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid ItemSold.token_id"
                );
                assert_eq!(
                    quantity, expected_quantity,
                    "encountered invalid ItemSold.quantity"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid ItemSold.pay_token"
                );
                assert_eq!(
                    unit_price, expected_unit_price,
                    "encountered invalid ItemSold.unit_price"
                );

                assert_eq!(
                    price_per_item, expected_price_per_item,
                    "encountered invalid ItemSold.price_per_item"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ItemSold event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubMarketplace::ItemSold",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemSold::seller",
                    value: &expected_seller,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemSold::buyer",
                    value: &expected_buyer,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemSold::nft_address",
                    value: &expected_nft_address,
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
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_pay_token: AccountId,
            expected_new_price: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemUpdated(ItemUpdated {
                owner,
                nft_address,
                token_id,
                pay_token,
                new_price,
            }) = decoded_event
            {
                assert_eq!(
                    owner, expected_owner,
                    "encountered invalid ItemUpdated.owner"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid ItemUpdated.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid ItemUpdated.token_id"
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
                    value: b"SubMarketplace::ItemUpdated",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemUpdated::owner",
                    value: &expected_owner,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemUpdated::nft_address",
                    value: &expected_nft_address,
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
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::ItemCanceled(ItemCanceled {
                owner,
                nft_address,
                token_id,
            }) = decoded_event
            {
                assert_eq!(
                    owner, expected_owner,
                    "encountered invalid ItemCanceled.owner"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid ItemCanceled.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid ItemCanceled.token_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a ItemCanceled event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubMarketplace::ItemCanceled",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemCanceled::owner",
                    value: &expected_owner,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::ItemCanceled::nft_address",
                    value: &expected_nft_address,
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
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_quantity: u128,
            expected_pay_token: AccountId,
            expected_price_per_item: Balance,
            expected_deadline: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::OfferCreated(OfferCreated {
                creator,
                nft_address,
                token_id,
                quantity,
                pay_token,
                price_per_item,
                deadline,
            }) = decoded_event
            {
                assert_eq!(
                    creator, expected_creator,
                    "encountered invalid OfferCreated.creator"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid OfferCreated.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid OfferCreated.token_id"
                );
                assert_eq!(
                    quantity, expected_quantity,
                    "encountered invalid OfferCreated.quantity"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid OfferCreated.pay_token"
                );
                assert_eq!(
                    price_per_item, expected_price_per_item,
                    "encountered invalid OfferCreated.price_per_item"
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
                    value: b"SubMarketplace::OfferCreated",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::OfferCreated::creator",
                    value: &expected_creator,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::OfferCreated::nft_address",
                    value: &expected_nft_address,
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
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::OfferCanceled(OfferCanceled {
                creator,
                nft_address,
                token_id,
            }) = decoded_event
            {
                assert_eq!(
                    creator, expected_creator,
                    "encountered invalid OfferCanceled.creator"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid OfferCanceled.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid OfferCanceled.token_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a OfferCanceled event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubMarketplace::OfferCanceled",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::OfferCanceled::creator",
                    value: &expected_creator,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubMarketplace::OfferCanceled::nft_address",
                    value: &expected_nft_address,
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
