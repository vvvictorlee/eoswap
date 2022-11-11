//! #  Auction
//!
//! This is an  NFT Token Auction implementation.
//!
//! ## Warning
//!
//! This contract is an *example*. It is neither audited nor endorsed for production use.
//! Do **not** rely on it to keep anything of value secure.
//!
//! ## Overview
//!
//!
//! What is an English Auction?
//! English auction refers to the process or method of the sale of a single quantity of a product where the bidding starts with the starting price,
//! which is set by the seller of the product and increases with the continuous bidding from the different buyers until the price is reached at a level above which there is no further bidding.
//! This price will be the selling price of the product under the auction.
//! English auction is the process under which one quantity of a product is listed for sale.
//! Under this method, all the bidders are aware of each other, and the bids are placed openly in front of everyone.
//!  The process starts with the declaration of the opening bid or the reserve price, which the product seller sets.
//! After this, the interested bidders start placing their respective bids in an ascending order, i.e., the next bid should be higher than the previous bidder’s price.
//! This process continues until there is a bid above which any other buyer is not interested in buying the item.
//! It is the highest bid and the selling price of the product.
//! reserve price Reserve Price Reserve price refers to the minimum price at which the seller of an item is ready to sell its item in an auction,
//!  below which he is not obliged to accept the deal.
//! Features
//! English auction is an open and transparent auction with different bidders, and each bidder’s value is known to others.
//! All the bids should be in ascending order, and the next bidder can place the bid with an amount higher than the previous bid amount.
//! The seller of the product sets the reserve price or the opening bid. So, the bid below such price is allowed.
//! The auction houses set the mechanism of the bid price increment.
//!  
//! Bid Price is the highest amount that a buyer quotes against the “ask price” (quoted by a seller) to buy particular security, stock, or any financial instrument.
//!
//! To list NFTs in an auction, a lister specifies —
//! Every auction listing obeys two 'buffers' to make it a fair auction:
//!
//! 1. **Time buffer**: this is measured in seconds (by default, 20 minutes or 1200 seconds).
//! If a winning bid is made within the buffer of the auction closing (e.g. 12 hours within the auction closing),
//! the auction's closing time is increased by the buffer to prevent buyers from making last minute winning bids, and to give time to other buyers to make a higher bid if they wish to.
//! 2. **Bid buffer**: this is a increment (by default, 10). A new bid is considered to be a winning bid only if its bid amount is at least the bid buffer (e.g. 10) greater than the previous winning bid.
//! This prevents buyers from making insignificantly higher bids to win the auctioned items.
//! A new bid is considered to be a winning bid only if its bid amount is at least the bid buffer (e.g. 5%) greater than the previous winning bid.
//! This prevents buyers from making insignificantly higher bids to win the auctioned items.
//
//! These buffer values are contract-wide, which means every auction conducted in the Marketplace obeys, at any given moment,
//! the same buffers. These buffers can be configured by contract owner .
//!
//! The NFTs to list in an auction *do* leave the wallet of the lister, and are escrowed in the market until the closing of the auction.
//!  Whenever a new winning bid is made by a buyer, the buyer deposits this bid amount into the market;
//! this bid amount is escrowed in the market until a new winning bid is made.
//! The previous winning bid amount is automatically refunded to the respective bidder.
//!
//! **Note:** As a result, the new winning bidder pays for the gas used in refunding the previous winning bidder.
//! This trade-off is made for better UX for bidders — a bidder that has been outbid is automatically refunded, and does not need to pull out their deposited bid manually.
//! This reduces bidding to a single action, instead of two actions — bidding, and pulling out the bid on being outbid.
//!
//! If the lister sets a `price_per_item`, the marketplace expects the `price_per_item` to be greater than or equal to the `rerserve_price` of the auction.
//! Once the auction window ends, the seller collects the highest bid, and the buyer collects the auctioned NFTs.
//! - **Auction ** are *high commitment*, low frequency listings. The seller and bidders respect the auction window,
//! recognize that their NFTs / bid amounts will be illiquid for the auction duration, and expect a guaranteed payout at auction closing — the auctioned items for the bidder,
//! and the winning bid amount for the seller. So, tokens listed for sale in an auction, and the highest bid at any given moment *are* escrowed in the market.
//- Thirdweb's customers
//!     - Deploy the marketplace contract like any other thirdweb contract.
//!     - Can set a % 'platform fee'. This % is collected on every sale  when a seller collects the highest bid on auction closing. This platform fee is distributed to the platform fee recipient (set by a contract owner).
//!     - Can set auction buffers. These auction buffers apply to all auctions being conducted in the market.
//! - End users of thirdweb customers
//!     - Can auction NFTs.
//!     - Can make bids to auctions.
//!     - Must pay gas fees to perform any actions, including the actions just listed.
//
//! ## Error Handling
//!
//!
//! Any error or invariant violation triggers a panic and therefore
//! rolls back the transaction.
//!
//! ## Interface
//!
//!  ### Auction Creation
//!
//!  Creates a new auction for a given item
//!  Only the owner of item can create an auction and must have approved the contract
//!  In addition to owning the item, the sender also has to have the owner role.
//!  End time for the auction must be in the future.
//!
//!  ### Bid Placement
//!
//!   Places a new bid, out bidding the existing bidder if found and criteria is reached
//!   Only callable when the auction is open
//!   Bids from smart contracts are prohibited to prevent griefing with always reverting receiver
//!
//!  ### Bid Withdrawing
//!   
//!   Allows the hightest bidder to withdraw the bid (after 12 hours post auction's end)
//!   Only callable by the existing top bidder
//!
//!  ### Auction Result
//!   
//!   Closes a finished auction and rewards the highest bidder
//!   Only admin or smart contract
//!   Auction can only be resulted if there has been a bidder and reserve met.
//!   If there have been no bids, the auction needs to be cancelled instead using `cancelAuction()`  
//!
//!  ### Auction Cancellation
//!   
//!   Cancels and inflight and un-resulted auctions, returning the funds to the top bidder if found
//!   ### Contract Parameters
//!  `update_min_bid_increment`  Update the amount by which bids have to increase, across all auctions
//!  `update_bid_withdrawal_lock_time`    Update the global bid withdrawal lockout time
//!  `update_auction_reserve_price ` Update the current reserve price for an auction
//!  `update_auction_start_time` Update the current start time for an auction
//!   `update_auction_end_time`Update the current end time for an auction
//!   `update_platform_fee`Method for updating platform fee
//!  `update_platform_fee_recipient`Method for updating platform fee address
//!   `update_address_registry`Update SubAddressRegistry contract

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_auction::{SubAuction, SubAuctionRef};

use ink_lang as ink;

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
pub mod sub_auction {
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
    pub struct Auction {
        /// The `owner` of the contract
        pub owner: AccountId,
        /// Pay token. The address of the token accepted by the listing. Either an ERC20 token or the chain's native token
        pub pay_token: AccountId,
        /// min bid amount
        pub min_bid: Balance,
        /// The  reserve price. All bids made to this auction must be at least as great as the reserve price per unit of NFTs auctioned, times the total number of NFTs put up for auction.
        pub reserve_price: Balance,
        /// The start time of the auction.The unix timestamp after which NFTs can be bought from the listing.
        pub start_time: u128,
        /// The end time of the auction. No. of seconds after `startTime`, after which NFTs can no longer be bought from the listing.
        pub end_time: u128,
        /// The resulted flag of the auction.
        pub resulted: bool,
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
    pub struct HighestBid {
        /// The `AccountId` of the bidder.
        pub bidder: AccountId,
        /// The bid amount.
        pub bid: Balance,
        /// The last bid time of the bid.
        pub last_bid_time: u128,
    }

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubAuction {
        ///  ERC721 Address -> Token ID -> Auction Parameters.
        auctions: Mapping<(AccountId, TokenId), Auction>,
        ///  ERC721 Address -> Token ID -> HighestBid Parameters.
        highest_bids: Mapping<(AccountId, TokenId), HighestBid>,
        ///  globally and across all auctions, the amount by which a bid has to increase.
        min_bid_increment: Balance,
        ///  global bid withdrawal lock time20 minutes.
        bid_withdrawal_lock_time: u128,
        ///  Platform fee.
        platform_fee: Balance,
        ///  Platform fee receipient.
        fee_recipient: AccountId,
        ///  Platform fee receipient.
        address_registry: AccountId,
        /// The paused of the contract.
        is_paused: bool,
        /// The tick for test on dev node.
        tick: bool,
        ///  The contract owner
        owner: AccountId,
        test_token_owner: Mapping<TokenId, AccountId>,
        test_enabled: Mapping<AccountId, bool>,
        test_operator_approvals: Mapping<(AccountId, AccountId), ()>,
        test_transfer_fail: Mapping<(AccountId, AccountId, AccountId, TokenId, Balance), ()>,
        test_contract_id: AccountId,
        test_minters_royalties: Mapping<(AccountId, TokenId), (AccountId, Balance)>,
        test_collection_royalties: Mapping<AccountId, (AccountId, Balance)>,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        ContractPaused,
        InvalidPayToken,
        AuctionAlreadyStarted,
        EndTimeMustBeGreaterThanStartBy5Minutes,
        InvalidStartTime,
        TransferFailed,
        NoContractsPermitted,
        BiddingOutsideOfTheAuctionWindow,
        ERC20MethodUsedForAuction,
        BidCannotBeLowerThanReservePrice,
        FailedToOutbidHighestBidder,
        YouAreNotTheHighestBidder,
        CanWithdrawOnlyAfter12HoursAfterAuctionEnded,
        SenderMustBeItemOwner,
        AuctioncNotApproved,
        NoAuctionExists,
        AuctionNotEnded,
        AuctionAlreadyResulted,
        NoOpenBids,
        HighestBidIsBelowReservePrice,
        InsufficientFunds,
        FailedToSendPlatformFee,
        FailedToSendTheOwnerTheirRoyalties,
        FailedToSendTheRoyalties,
        FailedToSendTheOwnerTheAuctionBalance,
        NotOwneAndOrContractNotApproved,
        StartTimeShouldBeLessThanEndTimeBy5Minutes,
        AuctionAlreadyEnded,
        EndTimeMustBeGreaterThanStart,
        AuctionShouldEndAfter5Minutes,
        FailedToRefundPreviousBidder,
        InvalidAddress,
        TransactionFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubAuction result types.
    pub type Result<T> = core::result::Result<T, Error>;
    /// Event emitted when a contract deployed occurs.
    #[ink(event)]
    pub struct SubAuctionContractDeployed {}
    #[ink(event)]
    pub struct PauseToggled {
        is_paused: bool,
    }
    /// Event emitted when a auction created occurs.
    #[ink(event)]
    pub struct AuctionCreated {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        pay_token: AccountId,
    }

    /// Event emitted when update auction end time occurs.
    #[ink(event)]
    pub struct UpdateAuctionEndTime {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        end_time: u128,
    }

    /// Event emitted when update auction start time occurs.
    #[ink(event)]
    pub struct UpdateAuctionStartTime {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        start_time: u128,
    }

    /// Event emitted when update auction reserve price occurs.
    #[ink(event)]
    pub struct UpdateAuctionReservePrice {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        pay_token: AccountId,
        reserve_price: Balance,
    }

    /// Event emitted when  update platform fee occurs.
    #[ink(event)]
    pub struct UpdatePlatformFee {
        platform_fee: Balance,
    }
    /// Event emitted when  update platform fee recipient occurs.
    #[ink(event)]
    pub struct UpdatePlatformFeeRecipient {
        fee_recipient: AccountId,
    }

    /// Event emitted when update bid withdrawal lock time occurs.
    #[ink(event)]
    pub struct UpdateMinBidIncrement {
        min_bid_increment: Balance,
    }
    /// Event emitted when update bid withdrawal lock time occurs.
    #[ink(event)]
    pub struct UpdateBidWithdrawalLockTime {
        bid_withdrawal_lock_time: u128,
    }
    /// Event emitted when a bid is placed.
    #[ink(event)]
    pub struct BidPlaced {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        #[ink(topic)]
        bidder: AccountId,
        bid: Balance,
    }

    /// Event emitted when bid withdrawn occurs.
    #[ink(event)]
    pub struct BidWithdrawn {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        #[ink(topic)]
        bidder: AccountId,
        bid: Balance,
    }

    /// Event emitted when a bid refunded occurs.
    #[ink(event)]
    pub struct BidRefunded {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        #[ink(topic)]
        bidder: AccountId,
        bid: Balance,
    }
    /// Event emitted when auction resulted occurs.
    #[ink(event)]
    pub struct AuctionResulted {
        old_owner: AccountId,
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        #[ink(topic)]
        winner: AccountId,
        pay_token: AccountId,
        unit_price: Balance,
        winning_bid: Balance,
    }

    /// Event emitted when  auction cancelled occurs.
    #[ink(event)]
    pub struct AuctionCancelled {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
    }
    impl SubAuction {
        /// Creates a new auction contract.
        #[ink(constructor)]
        pub fn new(platform_fee: Balance, fee_recipient: AccountId) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                Self::initialize(contract, platform_fee, fee_recipient)
            })
        }
        ///  Contract initializer
        fn initialize(&mut self, platform_fee: Balance, fee_recipient: AccountId) {
            assert!(fee_recipient != AccountId::from([0x0; 32]));
            self.owner = Self::env().caller();
            self.fee_recipient = fee_recipient;
            self.platform_fee = platform_fee;
            self.min_bid_increment = 1;
            self.bid_withdrawal_lock_time = 1200;
            Self::env().emit_event(SubAuctionContractDeployed {});
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

        ///  Creates a new auction for a given item
        ///  Only the owner of item can create an auction and must have approved the contract
        ///  In addition to owning the item, the sender also has to have the owner role.
        ///  End time for the auction must be in the future.
        ///
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the item being auctioned
        ///  pay_token Paying token
        ///  reserve_price Item cannot be sold for less than this or minBidIncrement, whichever is higher
        ///  start_time Unix epoch in seconds for the auction start time
        ///  min_bid_reserve Unix epoch in seconds for the auction end time.
        ///
        /// # Errors
        ///
        /// - If the contract is paused.
        /// - If  the caller is not  the owner of the specified nft contract address and token id  .
        /// - If the contract is not approved by the caller in the  contract in the following ERC-721 standard.
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        /// - If  the end time of the auction  of the specified nft contract address and token id is not zero.
        /// - If the specified `end_time`  is less than 'start_time'+300.
        /// - If the specified `start_time`  is greater than the current time.
        #[ink(message)]
        pub fn create_auction(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            pay_token: AccountId,
            reserve_price: Balance,
            start_time: u128,
            min_bid_reserve: bool,
            end_time: u128,
        ) -> Result<()> {
            ink_env::debug_println!(
                "  erc721_owner_of=11====  {:?}, =erc721_is_approved_for_all=== {:?}",
                self.erc721_owner_of(nft_address, token_id)?,
                self.erc721_is_approved_for_all(
                    nft_address,
                    self.env().caller(),
                    self.env().account_id(),
                )?,
            );
            //whenNotPaused
            ensure!(!self.is_paused, Error::ContractPaused);
            ensure!(
                Some(self.env().caller()) == self.erc721_owner_of(nft_address, token_id)?
                    && self.erc721_is_approved_for_all(
                        nft_address,
                        self.env().caller(),
                        self.env().account_id(),
                    )?,
                Error::NotOwneAndOrContractNotApproved
            );
            ink_env::debug_println!(" ==484========");
            if AccountId::from([0x0; 32]) != pay_token {
                #[cfg(test)]
                {
                    ensure!(
                        self.token_registry_enabled(AccountId::from([0x0; 32]), pay_token)
                            .unwrap_or(false),
                        Error::InvalidPayToken,
                    );
                }
                #[cfg(not(test))]
                {
                    let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                        ink_env::call::FromAccountId::from_account_id(self.address_registry);

                    ensure!(
                        AccountId::from([0x0; 32]) != address_registry_instance.token_registry(),
                        Error::InvalidPayToken
                    );
                    ensure!(
                        self.token_registry_enabled(
                            address_registry_instance.token_registry(),
                            pay_token
                        )
                        .unwrap_or(false),
                        Error::InvalidPayToken,
                    );
                }
            }
            ink_env::debug_println!("   ==521======== ");
            self._create_auction(
                nft_address,
                token_id,
                pay_token,
                reserve_price,
                start_time,
                min_bid_reserve,
                end_time,
            )?;
            Ok(())
        }

        ///  Places a new bid, out bidding the existing bidder if found and criteria is reached
        ///  Only callable when the auction is open
        ///  Bids from smart contracts are prohibited to prevent griefing with always reverting receiver
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the item being auctioned
        ///
        /// # Errors
        ///
        /// - If  the caller is a contract.
        /// - If  the start time  of  the auction (the specified `nft_address` and `token_id`) is greater than the current time.
        /// - If  the end time  of  the auction (the specified `nft_address` and `token_id`) is less than the current time.
        /// - If  the pay token of the auction (the specified `nft_address` and `token_id`)  is not zero address.
        /// - If the contract is paused.
        /// - If  the `bid_amount` is  less than the reserve price of the the auction (the specified `nft_address` and `token_id`) .
        /// - If  the `bid_amount` is  less than the sum of the minimum bid increment and the  bid of the the highest bid (the specified `nft_address` and `token_id`) .
        /// - If  it failed when the caller transfer to the contract  in the `pay_token` erc20 contract .
        /// - If the `current_highest_bid`  is greater than the balance of  the contract .
        /// - If it failed when the contract transfer  to the `current_highest_bidder`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer  to the `current_highest_bidder`  in the native token .
        #[ink(message, payable)]
        pub fn place_bid_native(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> Result<()> {
            ensure!(!self.is_paused, Error::ContractPaused);
            #[cfg(test)]
            {
                ensure!(
                    self.test_contract_id != self.env().caller(),
                    Error::NoContractsPermitted
                );
            }
            #[cfg(not(test))]
            {
                ensure!(
                    !self.env().is_contract(&self.env().caller()),
                    Error::NoContractsPermitted
                );
            }
            let auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                auction.start_time <= self.get_now() && auction.end_time >= self.get_now(),
                Error::BiddingOutsideOfTheAuctionWindow
            );
            ensure!(
                auction.pay_token == AccountId::from([0x0; 32]),
                Error::InvalidPayToken
            );
            self._place_bid(nft_address, token_id, self.env().transferred_value())?;
            Ok(())
        }

        ///  Places a new bid, out bidding the existing bidder if found and criteria is reached
        ///  Only callable when the auction is open
        ///  Bids from smart contracts are prohibited to prevent griefing with always reverting receiver
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the item being auctioned
        ///  bid_amount Bid amount
        ///
        /// # Errors
        ///
        /// - If  the caller is a contract.
        /// - If  the start time  of  the auction (the specified `nft_address` and `token_id`) is greater than the current time.
        /// - If  the end time  of  the auction (the specified `nft_address` and `token_id`) is less than the current time.
        /// - If  the pay token of the auction (the specified `nft_address` and `token_id`)  is zero address.
        /// - If the contract is paused.
        /// - If  the `bid_amount` is  less than the reserve price of the the auction (the specified `nft_address` and `token_id`) .
        /// - If  the `bid_amount` is  less than the sum of the minimum bid increment and the  bid of the the highest bid (the specified `nft_address` and `token_id`) .
        /// - If  it failed when the caller transfer to the contract  in the `pay_token` erc20 contract .
        /// - If the `current_highest_bid`  is greater than the balance of  the contract .
        /// - If it failed when the contract transfer  to the `current_highest_bidder`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer  to the `current_highest_bidder`  in the native token .
        #[ink(message)]
        pub fn place_bid(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            bid_amount: Balance,
        ) -> Result<()> {
            ensure!(!self.is_paused, Error::ContractPaused);
            #[cfg(test)]
            {
                ensure!(
                    self.test_contract_id != self.env().caller(),
                    Error::NoContractsPermitted
                );
            }
            #[cfg(not(test))]
            {
                ensure!(
                    !self.env().is_contract(&self.env().caller()),
                    Error::NoContractsPermitted
                );
            }
            let auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                auction.start_time <= self.get_now() && auction.end_time >= self.get_now(),
                Error::BiddingOutsideOfTheAuctionWindow
            );
            ensure!(
                auction.pay_token != AccountId::from([0x0; 32]),
                Error::ERC20MethodUsedForAuction
            );
            self._place_bid(nft_address, token_id, bid_amount)?;
            Ok(())
        }

        ///  Allows the hightest bidder to withdraw the bid (after 12 hours post auction's end)
        ///  Only callable by the existing top bidder
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the item being auctioned
        ///
        /// # Errors
        ///
        /// - If the contract is paused.
        /// - If  the caller is not the bidder of  the highest bid (the specified `nft_address` and `token_id`) .
        /// - If  the sum of `bid_withdrawal_lock_time` and the end time  of  the auction (the specified `nft_address` and `token_id`) is greater than the current time.
        /// - If the `current_highest_bid`  is greater than the balance of  the contract .
        /// - If it failed when  the contract transfer to the `current_highest_bidder` in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer to the `current_highest_bidder` in the native token .
        #[ink(message)]
        pub fn withdraw_bid(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> {
            ensure!(!self.is_paused, Error::ContractPaused);
            let highest_bid = self.highest_bid_impl(nft_address, token_id);
            ensure!(
                highest_bid.bidder == self.env().caller(),
                Error::YouAreNotTheHighestBidder
            );

            let auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                auction.end_time + self.bid_withdrawal_lock_time <= self.get_now(),
                Error::CanWithdrawOnlyAfter12HoursAfterAuctionEnded
            );
            let previous_bid = highest_bid.bid;
            self.highest_bids.remove(&(nft_address, token_id));
            self._refund_highest_bidder(nft_address, token_id, self.env().caller(), previous_bid)?;
            self.env().emit_event(BidWithdrawn {
                nft_address,
                token_id,
                bidder: self.env().caller(),
                bid: previous_bid,
            });

            Ok(())
        }

        ///  Closes a finished auction and rewards the highest bidder
        ///  Only admin or smart contract
        ///  Auction can only be resulted if there has been a bidder and reserve met.
        ///  If there have been no bids, the auction needs to be cancelled instead using `cancelAuction()`
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the item being auctioned
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the specified nft contract address and token id  .
        /// - If the caller is not the owner  by the auction of the specified nft contract address and token id .
        /// - If the `pay_token` is not enabled in the token registry contract when the `pay_token` is not zero address.
        /// - If  the end time of the auction  of the specified nft contract address and token id is zero.
        /// - If  the end time of the auction  of the specified nft contract address and token id is greater than or equal to the current time.
        /// - If  the resulted of the auction  of the specified nft contract address and token id is true.
        /// - If  the bidder of  the highest bid (the specified `nft_address` and `token_id`) is zero address .
        /// - If  the reserve price of the auction of the specified nft contract address and token id is greater than the bid of the the highest bid (the specified `nft_address` and `token_id`) .
        /// - If the contract is not approved by the caller in the  contract in the following ERC-721 standard.
        /// - If it failed when the contract transfer platform fee to the `fee_recipient`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer platform fee to the `fee_recipient`  in the native token .
        /// - If it failed when the contract transfer  royalty fee to the `minter`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer  royalty fee to the `minter`  in the native token .
        /// - If it failed when the contract transfer collection royalty fee to the `minter`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer collection royalty fee to the `minter`  in the native token .
        /// - If it failed when the contract transfer  pay amount to the `auction.owner`  in the `pay_token` erc20 contract .
        /// - If it failed when the contract transfer  pay amount to the `auction.owner`  in the native token .
        /// - If it failed when the owner of `nft_address` and `token_id` transfer  token id  to the `winner`  in the `nft_address` erc721 contract .
        #[ink(message)]
        pub fn result_auction(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> {
            // Check the auction to see if it can be resulted
            let mut auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                Some(self.env().caller()) == self.erc721_owner_of(nft_address, token_id)?
                    && self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );

            // Check the auction real
            ensure!(auction.end_time > 0, Error::NoAuctionExists);
            // Check the auction has ended
            ensure!(auction.end_time < self.get_now(), Error::AuctionNotEnded);
            // Ensure auction not already resulted

            ensure!(!auction.resulted, Error::AuctionAlreadyResulted);
            let highest_bid = self.highest_bid_impl(nft_address, token_id);

            let winner = highest_bid.bidder;
            let winning_bid = highest_bid.bid;
            ensure!(winner != AccountId::from([0x0; 32]), Error::NoOpenBids);
            ensure!(
                winning_bid >= auction.reserve_price,
                Error::HighestBidIsBelowReservePrice
            );
            ensure!(
                self.erc721_is_approved_for_all(
                    nft_address,
                    self.env().caller(),
                    self.env().account_id(),
                )
                .unwrap_or(false),
                Error::AuctioncNotApproved
            );
            auction.resulted = true;
            self.auctions.insert(&(nft_address, token_id), &auction);
            self.highest_bids.remove(&(nft_address, token_id));
            let mut pay_amount = winning_bid;
            if winning_bid > auction.reserve_price {
                // Work out total above the reserve
                let above_reserve_price = winning_bid - auction.reserve_price;

                // Work out platform fee from above reserve amount
                let platform_fee_above_reserve = above_reserve_price * self.platform_fee / 1000;

                if auction.pay_token == AccountId::from([0x0; 32]) {
                    // Send platform fee
                    ensure!(
                        platform_fee_above_reserve <= self.env().balance(),
                        Error::FailedToSendPlatformFee
                    );
                    ensure!(
                        self.env()
                            .transfer(self.fee_recipient, platform_fee_above_reserve)
                            .is_ok(),
                        Error::FailedToSendPlatformFee
                    );
                } else {
                    ensure!(
                        self.erc20_transfer(
                            auction.pay_token,
                            self.fee_recipient,
                            platform_fee_above_reserve
                        )
                        .is_ok(),
                        Error::FailedToSendPlatformFee
                    );
                }

                // Send remaining to designer
                pay_amount -= platform_fee_above_reserve;
            }
            let (minter, royalty) = (
                self.marketplace_minters(nft_address, token_id)?,
                self.marketplace_royalties(nft_address, token_id)?,
            );
            if minter != AccountId::from([0x0; 32]) && royalty != 0 {
                let royalty_fee = pay_amount * royalty / 10000;
                if auction.pay_token == AccountId::from([0x0; 32]) {
                    // Send royalty fee
                    ensure!(
                        royalty_fee <= self.env().balance(),
                        Error::FailedToSendTheOwnerTheirRoyalties
                    );
                    ensure!(
                        self.env().transfer(minter, royalty_fee).is_ok(),
                        Error::FailedToSendTheOwnerTheirRoyalties
                    );
                } else {
                    ensure!(
                        self.erc20_transfer(auction.pay_token, minter, royalty_fee)
                            .is_ok(),
                        Error::FailedToSendTheOwnerTheirRoyalties
                    );
                }
                pay_amount -= royalty_fee;
            } else {
                let (minter, royalty) = self.marketplace_collection_royalties(nft_address)?;
                if minter != AccountId::from([0x0; 32]) && royalty != 0 {
                    let royalty_fee = pay_amount * royalty / 10000;
                    if auction.pay_token == AccountId::from([0x0; 32]) {
                        // Send royalty fee
                        ensure!(
                            royalty_fee <= self.env().balance(),
                            Error::FailedToSendTheRoyalties
                        );
                        ensure!(
                            self.env().transfer(minter, royalty_fee).is_ok(),
                            Error::FailedToSendTheRoyalties
                        );
                    } else {
                        ensure!(
                            self.erc20_transfer(auction.pay_token, minter, royalty_fee)
                                .is_ok(),
                            Error::FailedToSendTheRoyalties
                        );
                    }
                    pay_amount -= royalty_fee;
                }
            }
            if pay_amount > 0 {
                if auction.pay_token == AccountId::from([0x0; 32]) {
                    // Send pay amount
                    ensure!(
                        pay_amount <= self.env().balance(),
                        Error::FailedToSendTheOwnerTheAuctionBalance
                    );
                    ensure!(
                        self.env().transfer(auction.owner, pay_amount).is_ok(),
                        Error::FailedToSendTheOwnerTheAuctionBalance
                    );
                } else {
                    ensure!(
                        self.erc20_transfer(auction.pay_token, auction.owner, pay_amount)
                            .is_ok(),
                        Error::FailedToSendTheOwnerTheAuctionBalance
                    );
                }
            }
            ensure!(
                self.erc721_transfer_from(
                    nft_address,
                    self.erc721_owner_of(nft_address, token_id)?.unwrap(),
                    winner,
                    token_id
                )
                .is_ok(),
                Error::NotOwneAndOrContractNotApproved
            );

            self.bundle_marketplace_validate_item_sold(nft_address, token_id, 1)?;
            let unit_price = self.marketplace_get_price(nft_address)?;

            self.env().emit_event(AuctionResulted {
                old_owner: self.env().caller(),
                nft_address,
                token_id,
                winner,
                pay_token: auction.pay_token,
                unit_price,
                winning_bid,
            });
            self.auctions.remove(&(nft_address, token_id));
            Ok(())
        }

        ///  Cancels and inflight and un-resulted auctions, returning the funds to the top bidder if found
        ///  Only item owner
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the specified nft contract address and token id  .
        /// - If the caller is not the owner  by the auction of the specified nft contract address and token id .
        /// - If  the end time of the auction  of the specified nft contract address and token id is zero.
        /// - If  the resulted of the auction  of the specified nft contract address and token id is true.
        /// - If it failed when the contract transfer `current_highest_bid` to the `current_highest_bidder`  in the `pay_token` erc20 contract  , the bidder of  the highest bid (the specified `nft_address` and `token_id`) is not zero address  and `pay_token`  is not zero address.
        /// - If it failed when the contract transfer `current_highest_bid`  to the `current_highest_bidder`  in the native token , the bidder of  the highest bid (the specified `nft_address` and `token_id`) is not zero address and `pay_token`  is  zero address .
        #[ink(message)]
        pub fn cancel_auction(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> {
            // Check valid and not resulted
            let auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                Some(self.env().caller()) == self.erc721_owner_of(nft_address, token_id)?
                    && self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );
            // Check the auction real
            ensure!(auction.end_time > 0, Error::NoAuctionExists);
            // Ensure auction not already resulted
            ensure!(!auction.resulted, Error::AuctionAlreadyResulted);
            self._cancel_auction(nft_address, token_id)?;
            Ok(())
        }

        ///  Toggling the pause flag
        ///  Only admin
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn toggle_is_paused(&mut self) -> Result<()> {
            ensure!(self.owner == self.env().caller(), Error::OnlyOwner);
            self.is_paused = !self.is_paused;
            self.env().emit_event(PauseToggled {
                is_paused: self.is_paused,
            });

            Ok(())
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
        ///  Update the amount by which bids have to increase, across all auctions
        ///  Only admin
        ///  # Fields
        ///  min_bid_increment New bid step in smallest unit
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn update_min_bid_increment(&mut self, min_bid_increment: Balance) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.min_bid_increment = min_bid_increment;
            self.env()
                .emit_event(UpdateMinBidIncrement { min_bid_increment });
            Ok(())
        }
        ///  Update the global bid withdrawal lockout time
        ///  Only admin
        ///  # Fields
        ///  bid_withdrawal_lock_time New bid withdrawal lock time
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn update_bid_withdrawal_lock_time(
            &mut self,
            bid_withdrawal_lock_time: u128,
        ) -> Result<()> {
            //onlyOwner
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            self.bid_withdrawal_lock_time = bid_withdrawal_lock_time;
            self.env().emit_event(UpdateBidWithdrawalLockTime {
                bid_withdrawal_lock_time,
            });
            Ok(())
        }
        ///  Update the current reserve price for an auction
        ///  Only admin
        ///  Auction must exist
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        ///  reserve_price New reserve price ( the smallest unit )
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the auction of the `nft_address` and `token_id` .
        /// - If  the end time of the auction  of the specified nft contract address and token id is zero.
        /// - If  the resulted of the auction  of the specified nft contract address and token id is true.
        #[ink(message)]
        pub fn update_auction_reserve_price(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            reserve_price: Balance,
        ) -> Result<()> {
            let mut auction = self.auction_of_impl(nft_address, token_id);

            ensure!(
                self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );

            // Ensure auction not already resulted
            ensure!(!auction.resulted, Error::AuctionAlreadyResulted);
            // Check the auction real
            ensure!(auction.end_time > 0, Error::NoAuctionExists);
            auction.reserve_price = reserve_price;
            self.auctions.insert(&(nft_address, token_id), &auction);
            self.env().emit_event(UpdateAuctionReservePrice {
                nft_address,
                token_id,
                pay_token: auction.pay_token,
                reserve_price,
            });
            Ok(())
        }

        ///  Update the current start time for an auction
        ///  Only admin
        ///  Auction must exist
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        /// start_time New start time (unix epoch in seconds)
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the auction of the `nft_address` and `token_id` .
        /// - If  the start time   is zero.
        /// - If  the sum of 60 seconds and the start time  of  the auction (the specified `nft_address` and `token_id`) is less than or equal to the current time.
        /// - If  the sum of 300 seconds and the start time   is greater than or equal to the end time of the auction  of the specified nft contract address and token id .
        /// - If  the resulted of the auction  of the specified nft contract address and token id is true.
        #[ink(message)]
        pub fn update_auction_start_time(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            start_time: u128,
        ) -> Result<()> {
            let mut auction = self.auction_of_impl(nft_address, token_id);

            ensure!(
                self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );
            ensure!(start_time > 0, Error::InvalidStartTime);
            ensure!(
                auction.start_time + 60 > self.get_now(),
                Error::AuctionAlreadyStarted
            );
            ensure!(
                start_time + 300 < auction.end_time,
                Error::StartTimeShouldBeLessThanEndTimeBy5Minutes
            );
            // Ensure auction not already resulted
            ensure!(!auction.resulted, Error::AuctionAlreadyResulted);
            // Check the auction real
            auction.start_time = start_time;
            self.auctions.insert(&(nft_address, token_id), &auction);
            self.env().emit_event(UpdateAuctionStartTime {
                nft_address,
                token_id,
                start_time,
            });
            Ok(())
        }

        ///  Update the current end time for an auction
        ///  Only admin
        ///  Auction must exist
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        ///  end_time New end time (unix epoch in seconds)
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the auction of the `nft_address` and `token_id` .
        /// - If  the end time  of  the auction (the specified `nft_address` and `token_id`) is less than or equal to the current time.
        /// - If  the start time of the auction  of the specified nft contract address and token id is greater than or equal to the end time.
        /// - If  the sum of 300 seconds and  the current time  is greater than or equal to the end time  .
        #[ink(message)]
        pub fn update_auction_end_time(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            end_time: u128,
        ) -> Result<()> {
            let mut auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );

            ensure!(
                auction.end_time > self.get_now(),
                Error::AuctionAlreadyEnded
            );
            // Check the auction real

            ensure!(
                auction.start_time < end_time,
                Error::EndTimeMustBeGreaterThanStart
            );
            ensure!(
                end_time > self.get_now() + 300,
                Error::AuctionShouldEndAfter5Minutes
            );

            auction.end_time = end_time;
            self.auctions.insert(&(nft_address, token_id), &auction);
            self.env().emit_event(UpdateAuctionEndTime {
                nft_address,
                token_id,
                end_time,
            });
            Ok(())
        }

        ///  Method for updating platform fee
        ///  Only admin
        ///  # Fields
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
        /// fee_recipient  address the address to sends the funds to    
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

        ///  Update SubAddressRegistry contract
        ///  Only admin
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

        ///  Method for getting all info about the auction
        /// # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        #[ink(message)]
        pub fn get_auction(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> (AccountId, AccountId, Balance, Balance, u128, u128, bool) {
            // Check valid and not resulted

            let auction = self.auction_of_impl(nft_address, token_id);
            (
                auction.owner,
                auction.pay_token,
                auction.min_bid,
                auction.reserve_price,
                auction.start_time,
                auction.end_time,
                auction.resulted,
            )
        }
        ///  Method for getting start time and resulted flag about the auction
        /// # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        #[ink(message)]
        pub fn get_auction_start_time_resulted(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> (u128, bool) {
            // Check valid and not resulted
            let auction = self.auction_of_impl(nft_address, token_id);
            (auction.start_time, auction.resulted)
        }

        ///  Method for getting all info about the highest bidder
        /// # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        #[ink(message)]
        pub fn get_highest_bid(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> (AccountId, Balance, u128) {
            let highest_bid = self.highest_bid_impl(nft_address, token_id);
            (
                highest_bid.bidder,
                highest_bid.bid,
                highest_bid.last_bid_time,
            )
        }
        /// Get min_bid_increment
        /// # Return
        ///  min_bid_increment  min_bid_increment
        #[ink(message)]
        pub fn min_bid_increment(&self) -> Balance {
            self.min_bid_increment
        }
        /// Get bid_withdrawal_lock_time
        /// # Return
        ///  bid_withdrawal_lock_time  bid_withdrawal_lock_time
        #[ink(message)]
        pub fn bid_withdrawal_lock_time(&self) -> u128 {
            self.bid_withdrawal_lock_time
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
        /// Get is_paused
        /// # Return
        ///  is_paused  is_paused
        #[ink(message)]
        pub fn is_paused(&self) -> bool {
            self.is_paused
        }
        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
        ///  Reclaims ERC20 Compatible tokens for entire balance
        ///  Only access controls admin
        /// # Fields
        ///  token_contract The address of the token contract
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the contract .
        #[ink(message)]
        pub fn reclaim_erc20(&mut self, token_contract: AccountId) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                token_contract != AccountId::from([0x0; 32]),
                Error::InvalidAddress
            );
            let balance = self.erc20_balance_of(token_contract, self.env().account_id())?;
            self.erc20_transfer(token_contract, self.env().caller(), balance)?;
            Ok(())
        }
 ///  Update the current start time for an auction  for Test
        ///  Only admin
        ///  Auction must exist
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        /// start_time New start time (unix epoch in seconds)
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the auction of the `nft_address` and `token_id` .
        /// - If  the start time   is zero.
        /// - If  the resulted of the auction  of the specified nft contract address and token id is true.
        #[ink(message)]
        pub fn update_auction_start_time_for_test(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            start_time: u128,
        ) -> Result<()> {
            let mut auction = self.auction_of_impl(nft_address, token_id);

            ensure!(
                self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );
            ensure!(start_time > 0, Error::InvalidStartTime);
           
            // Ensure auction not already resulted
            ensure!(!auction.resulted, Error::AuctionAlreadyResulted);
            // Check the auction real
            auction.start_time = start_time;
            self.auctions.insert(&(nft_address, token_id), &auction);

            Ok(())
        }

        ///  Update the current end time for an auction  for Test
        ///  Only admin
        ///  Auction must exist
        ///  # Fields
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        ///  end_time New end time (unix epoch in seconds)
        ///
        /// # Errors
        ///
        /// - If  the caller is not  the owner of the auction of the `nft_address` and `token_id` .
        #[ink(message)]
        pub fn update_auction_end_time_for_test(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            end_time: u128,
        ) -> Result<()> {
            let mut auction = self.auction_of_impl(nft_address, token_id);
            ensure!(
                self.env().caller() == auction.owner,
                Error::SenderMustBeItemOwner
            );
            auction.end_time = end_time;
            self.auctions.insert(&(nft_address, token_id), &auction);

            Ok(())
        }
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
        pub fn test_erc20(&mut self, token_contract: AccountId, bid_amount: Balance) -> Result<()> {
            self.erc20_transfer_from(
                token_contract,
                self.env().caller(),
                self.env().account_id(),
                bid_amount,
            )
        }
        #[ink(message)]
        pub fn test_marketplace_get_price(&self, pay_token: AccountId) -> Result<Balance> {
            self.marketplace_get_price(pay_token)
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
    impl SubAuction {
        /// Returns the auction for the specified `nft_address`  and `token_id`.
        ///
        /// Returns `default auction ` if the auction is non-existent.
        #[inline]
        fn auction_of_impl(&self, nft_address: AccountId, token_id: TokenId) -> Auction {
            self.auctions
                .get((nft_address, token_id))
                .unwrap_or_default()
        }

        /// Returns the highest bid for the specified `nft_address`  and `token_id`.
        ///
        /// Returns `default highest_bid`  if the highest bid  is non-existent.
        /// works using references which are more efficient in Wasm.
        #[inline]
        fn highest_bid_impl(&self, nft_address: AccountId, token_id: TokenId) -> HighestBid {
            self.highest_bids
                .get((nft_address, token_id))
                .unwrap_or_default()
        }

        ///  Private method doing the heavy lifting of creating an auction
        ///  nft_address ERC 721 Address
        ///  token_id Token ID of the NFT being auctioned
        ///  pay_token Paying token
        ///  reserve_price Item cannot be sold for less than this or minBidIncrement, whichever is higher
        ///  start_time Unix epoch in seconds for the auction start time
        ///  end_time Unix epoch in seconds for the auction end time.
        fn _create_auction(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            pay_token: AccountId,
            reserve_price: Balance,
            start_time: u128,
            min_bid_reserve: bool,
            end_time: u128,
        ) -> Result<()> {
            let auction = self.auction_of_impl(nft_address, token_id);
            ink_env::debug_println!(
                "  ==auction.end_time == 0=====  {:?}, =end_time >= start_time + 300== {:?},",
                auction.end_time == 0,
                end_time >= start_time + 300,
            );
            ensure!(auction.end_time == 0, Error::AuctionAlreadyStarted);
            ensure!(
                end_time >= start_time + 300,
                Error::EndTimeMustBeGreaterThanStartBy5Minutes
            );
            ink_env::debug_println!(
                "  ==start_time > self.get_now()======  {:?},",
                start_time > self.get_now()
            );
            ensure!(start_time > self.get_now(), Error::InvalidStartTime);
            let mut min_bid = 0;
            if min_bid_reserve {
                min_bid = reserve_price;
            }
            self.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: self.env().caller(),
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            ink_env::debug_println!("==11==6666=====");
            self.env().emit_event(AuctionCreated {
                nft_address,
                token_id,
                pay_token,
            });
            Ok(())
        }

        fn _place_bid(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            bid_amount: Balance,
        ) -> Result<()> {
            ensure!(!self.is_paused, Error::ContractPaused);

            let auction = self.auction_of_impl(nft_address, token_id);
            if auction.min_bid == auction.reserve_price {
                ensure!(
                    auction.reserve_price <= bid_amount,
                    Error::BidCannotBeLowerThanReservePrice
                );
            }
            let mut highest_bid = self.highest_bid_impl(nft_address, token_id);
            let min_bid_required = highest_bid.bid + self.min_bid_increment;
            ensure!(
                min_bid_required <= bid_amount,
                Error::FailedToOutbidHighestBidder
            );
            if auction.pay_token != AccountId::from([0x0; 32]) {
                self.erc20_transfer_from(
                    auction.pay_token,
                    self.env().caller(),
                    self.env().account_id(),
                    bid_amount,
                )?;
            }
            if highest_bid.bidder != AccountId::from([0x0; 32]) {
                self._refund_highest_bidder(
                    nft_address,
                    token_id,
                    highest_bid.bidder,
                    highest_bid.bid,
                )?;
            }
            highest_bid.bidder = self.env().caller();
            highest_bid.bid = bid_amount;
            highest_bid.last_bid_time = self.get_now();
            self.highest_bids
                .insert((nft_address, token_id), &highest_bid);
            self.env().emit_event(BidPlaced {
                nft_address,
                token_id,
                bidder: self.env().caller(),
                bid: bid_amount,
            });

            Ok(())
        }

        fn _cancel_auction(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> {
            let highest_bid = self.highest_bid_impl(nft_address, token_id);
            if highest_bid.bidder != AccountId::from([0x0; 32]) {
                self._refund_highest_bidder(
                    nft_address,
                    token_id,
                    highest_bid.bidder,
                    highest_bid.bid,
                )?;
                self.highest_bids.remove(&(nft_address, token_id))
            }

            self.auctions.remove(&(nft_address, token_id));

            self.env().emit_event(AuctionCancelled {
                nft_address,
                token_id,
            });

            Ok(())
        }

        ///  Used for sending back escrowed funds from a previous bid
        /// nft_address NFT contract address
        /// token_id Token Id
        ///  current_highest_bidder Address of the last highest bidder
        ///  current_highest_bid  the  amount in the smallest unit that the bidder sent when placing their bid
        fn _refund_highest_bidder(
            &mut self,
            nft_address: AccountId,
            token_id: TokenId,
            current_highest_bidder: AccountId,
            current_highest_bid: Balance,
        ) -> Result<()> {
            let auction = self.auction_of_impl(nft_address, token_id);
            if auction.pay_token == AccountId::from([0x0; 32]) {
                // Send platform fee
                ensure!(
                    current_highest_bid <= self.env().balance(),
                    Error::InsufficientFunds
                );
                ensure!(
                    self.env()
                        .transfer(current_highest_bidder, current_highest_bid)
                        .is_ok(),
                    Error::FailedToRefundPreviousBidder
                );
            } else {
                ensure!(
                    self.erc20_transfer(
                        auction.pay_token,
                        current_highest_bidder,
                        current_highest_bid
                    )
                    .is_ok(),
                    Error::FailedToRefundPreviousBidder
                );
            }
            self.env().emit_event(BidRefunded {
                nft_address,
                token_id,
                bidder: current_highest_bidder,
                bid: current_highest_bid,
            });
            Ok(())
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn marketplace_minters(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> Result<AccountId> {
            let marketplace = self.get_marketplace()?;

            #[cfg(test)]
            {
                Ok(self
                    .test_minters_royalties
                    .get(&(nft_address, token_id))
                    .unwrap_or_default()
                    .0)
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x4B, 0x4C, 0x7E, 0xC9]; //marketplace_minter_of
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
                            .push_arg(token_id),
                    )
                    .returns::<AccountId>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("marketplace_minters= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn marketplace_royalties(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
        ) -> Result<Balance> {
            let marketplace = self.get_marketplace()?;

            #[cfg(test)]
            {
                Ok(self
                    .test_minters_royalties
                    .get(&(nft_address, token_id))
                    .unwrap_or_default()
                    .1)
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x13, 0x5B, 0x72, 0xF2]; //marketplace royalty_of
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
                            .push_arg(token_id),
                    )
                    .returns::<Balance>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("marketplace_royalties= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn marketplace_collection_royalties(
            &self,
            nft_address: AccountId,
        ) -> Result<(AccountId, Balance)> {
            let marketplace = self.get_marketplace()?;
            #[cfg(test)]
            {
                Ok(self
                    .test_collection_royalties
                    .get(&nft_address)
                    .unwrap_or_default())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0xAA, 0xFC, 0xD7, 0xEA]; //marketplace_collection_royalties
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(marketplace)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(nft_address))
                    .returns::<(AccountId, Balance)>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("marketplace_collection_royalties= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn get_bundle_marketplace(&self) -> Result<AccountId> {
            #[cfg(test)]
            {
                Ok(AccountId::default())
            }
            #[cfg(not(test))]
            {
                let address_registry_instance: sub_address_registry::SubAddressRegistryRef =
                    ink_env::call::FromAccountId::from_account_id(self.address_registry);
                ensure!(
                    AccountId::from([0x0; 32]) != address_registry_instance.bundle_marketplace(),
                    Error::InvalidPayToken
                );
                Ok(address_registry_instance.bundle_marketplace())
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn get_marketplace(&self) -> Result<AccountId> {
            #[cfg(test)]
            {
                Ok(AccountId::default())
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
        fn bundle_marketplace_validate_item_sold(
            &self,
            nft_address: AccountId,
            token_id: TokenId,
            quantity: u128,
        ) -> Result<()> {
            let bundle_marketplace = self.get_bundle_marketplace()?;
            #[cfg(test)]
            {
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x5E, 0x38, 0x31, 0x94];
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
                        ink_env::debug_println!("bundle_marketplace_validate_item_sold= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!(
                                "bundle_marketplace_validate_item_sold= {:?}",
                                e
                            );
                            Error::TransactionFailed
                        })
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn marketplace_get_price(&self, nft_address: AccountId) -> Result<Balance> {
            let marketplace = self.get_marketplace()?;
            #[cfg(test)]
            {
                Ok(Balance::default())
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
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(nft_address))
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
        fn erc20_transfer(
            &mut self,
            token: AccountId,
            to: AccountId,
            value: Balance,
        ) -> Result<()> {
            #[cfg(test)]
            {
                ensure!(
                    self.test_transfer_fail
                        .get(&(token, to, to, 0, value))
                        .is_none(),
                    Error::TransactionFailed
                );
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x84, 0xA1, 0x5D, 0xA1]; //erc20 transfer
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
                            .push_arg(to)
                            .push_arg(value),
                    )
                    .returns::<Result<()>>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc20_transfer= {:?}", e);
                        Error::TransactionFailed
                    })
                    .and_then(|v| {
                        v.map_err(|e| {
                            ink_env::debug_println!("erc20_transfer= {:?}", e);
                            Error::TransactionFailed
                        })
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
                let params = build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
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
                    .params();
                ink_env::invoke_contract(&params)
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
        fn erc20_balance_of(&self, token: AccountId, owner: AccountId) -> Result<Balance> {
            #[cfg(test)]
            {
                Ok(Balance::default())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x0F, 0x75, 0x5A, 0x56]; //erc20 balance_of
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(owner))
                    .returns::<Balance>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc20_balance_of= {:?}", e);
                        Error::TransactionFailed
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
        type Event = <SubAuction as ::ink_lang::reflect::ContractEventBase>::Type;

        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<ink_env::DefaultEnvironment>(sender);
        }
        fn default_accounts() -> ink_env::test::DefaultAccounts<Environment> {
            ink_env::test::default_accounts::<Environment>()
        }
        fn contract_id() -> AccountId {
            ink_env::test::callee::<ink_env::DefaultEnvironment>()
        }
        fn advance_blocks(increment_block_timestamps: u128) {
            for _ in (0..increment_block_timestamps).step_by(6) {
                ink_env::test::advance_block::<ink_env::DefaultEnvironment>();
            }
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
        fn init_contract() -> SubAuction {
            let erc = SubAuction::new(10, fee_recipient());

            erc
        }

        #[ink::test]
        fn create_auction_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);

            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            auction.test_enabled.insert(&pay_token, &true);
            assert!(auction
                .create_auction(
                    nft_address,
                    token_id,
                    pay_token,
                    reserve_price,
                    start_time,
                    min_bid_reserve,
                    end_time,
                )
                .is_ok());
            // assert_eq!(auction.create_auction(nft_address,
            //     token_id,
            //     pay_token,
            //     reserve_price,
            //     start_time,
            //     min_bid_reserve,
            //     end_time,
            // ).unwrap_err(),Error::AuctionAlreadyStarted);
            // // Token 1 does not exists.
            assert_eq!(
                auction.auctions.get(&(nft_address, token_id)),
                Some(Auction {
                    owner: caller,
                    pay_token,
                    min_bid: reserve_price,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                })
            );
            // assert_eq!(get_balance(fee_recipient()), 10);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_auction_created_event(&emitted_events[1], nft_address, token_id, pay_token);
        }

        #[ink::test]
        fn create_auction_failed_if_the_contract_is_paused() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);
            auction.is_paused = true;
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::ContractPaused
            );
        }

        #[ink::test]
        fn create_auction_failed_if_the_caller_is_not_owner_of_token_id_in_erc721_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);
            // auction.test_token_owner.insert(&token_id,&caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            auction.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::NotOwneAndOrContractNotApproved
            );
        }

        #[ink::test]
        fn create_auction_failed_if_the_contract_is_not_approved_by_the_caller_in_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = frank();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);
            auction.test_token_owner.insert(&token_id, &caller);
            //   auction.test_operator_approvals.insert(&(caller,contract_id()),&());
            auction.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::NotOwneAndOrContractNotApproved
            );
        }

        #[ink::test]
        fn create_auction_failed_if_pay_token_is_not_enabled_in_the_token_registry_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = eve();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            // auction.test_enabled.insert(&pay_token,&true);
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::InvalidPayToken
            );
        }

        #[ink::test]
        fn create_auction_failed_if_the_end_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_not_zero(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = django();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            auction.test_enabled.insert(&pay_token, &true);
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::AuctionAlreadyStarted
            );
        }

        #[ink::test]
        fn create_auction_failed_if_end_time_is_less_than_start_time_plus_300() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = charlie();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 10;
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 299;
            set_caller(caller);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            auction.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::EndTimeMustBeGreaterThanStartBy5Minutes
            );
        }

        #[ink::test]
        fn create_auction_failed_if_start_time_is_greater_than_the_current_time() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            set_caller(caller);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            auction.test_enabled.insert(&pay_token, &true);
            assert_eq!(
                auction
                    .create_auction(
                        nft_address,
                        token_id,
                        pay_token,
                        reserve_price,
                        start_time,
                        min_bid_reserve,
                        end_time,
                    )
                    .unwrap_err(),
                Error::InvalidStartTime
            );
        }

        #[ink::test]
        fn place_bid_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert!(auction.place_bid(nft_address, token_id, bid_amount).is_ok());
            // assert_eq!(auction.place_bid(nft_address,
            //     token_id,
            //     bid_amount,
            // ).unwrap_err(),Error::AuctionAlreadyStarted);
            // // Token 1 does not exists.

            // assert_eq!(get_balance(fee_recipient()), 10);
            assert_eq!(
                auction.highest_bids.get((nft_address, token_id)),
                Some(HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                })
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_bid_placed_event(
                &emitted_events[1],
                nft_address,
                token_id,
                caller,
                bid_amount,
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_caller_is_a_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(contract_id());

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.test_contract_id = contract_id();
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::NoContractsPermitted
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_start_time_of_the_auction_the_specified_nft_address_and_token_id_is_greater_than_the_current_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 1;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::BiddingOutsideOfTheAuctionWindow
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_end_time_of_the_auction_the_specified_nft_address_and_token_id_is_less_than_the_current_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(auction.bid_withdrawal_lock_time);
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::BiddingOutsideOfTheAuctionWindow
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_pay_token_of_the_auction_the_specified_nft_address_and_token_id_is_zero_address(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::ERC20MethodUsedForAuction
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_contract_is_paused() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = django();
            let reserve_price: Balance = 20;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.is_paused = true;
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::ContractPaused
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_bid_amount_is_less_than_the_reserve_price_of_the_the_auction_the_specified_nft_address_and_token_id(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = django();
            let reserve_price: Balance = 20;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::BidCannotBeLowerThanReservePrice
            );
        }

        #[ink::test]
        fn place_bid_failed_if_the_bid_amount_is_less_than_the_sum_of_the_minimum_bid_increment_and_the_bid_of_the_the_highest_bid_the_specified_nft_address_and_token_id(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            auction.min_bid_increment = 20;
            let pay_token: AccountId = frank();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::FailedToOutbidHighestBidder
            );
        }

        #[ink::test]
        fn place_bid_failed_if_it_failed_when_the_caller_transfer_to_the_contract_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = frank();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction
                .test_transfer_fail
                .insert(&(pay_token, caller, contract_id(), 0, bid_amount), &());
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::TransactionFailed
            );
        }

        #[ink::test]
        fn place_bid_failed_if_it_failed_when_the_contract_transfer_to_the_current_highest_bidder_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = frank();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: eve(),
                    bid: 1,
                    last_bid_time: auction.get_now(),
                },
            );
            auction
                .test_transfer_fail
                .insert(&(pay_token, eve(), eve(), 0, 1), &());
            assert_eq!(
                auction
                    .place_bid(nft_address, token_id, bid_amount)
                    .unwrap_err(),
                Error::FailedToRefundPreviousBidder
            );
        }
        #[ink::test]
        fn place_bid_native_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 21;

            set_caller(caller);

            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 1;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: django(),
                    bid: 11,
                    last_bid_time: auction.get_now(),
                },
            );
            set_balance(caller, bid_amount);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(bid_amount);
            // assert_eq!(
            //     auction
            //         .place_bid_native(nft_address, token_id )
            //         .unwrap_err(),
            //     Error::InsufficientFunds
            // );
            assert!(auction.place_bid_native(nft_address, token_id).is_ok());
            assert_eq!(get_balance(contract_id()), bid_amount - 11);

            assert_eq!(
                auction.highest_bids.get((nft_address, token_id)),
                Some(HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                })
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 3);
            assert_bid_placed_event(
                &emitted_events[2],
                nft_address,
                token_id,
                caller,
                bid_amount,
            );
        }

        #[ink::test]
        fn withdraw_bid_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(auction.bid_withdrawal_lock_time);
            assert_eq!(auction.get_now(), auction.bid_withdrawal_lock_time);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert!(auction.withdraw_bid(nft_address, token_id).is_ok());
            // assert_eq!(auction.withdraw_bid(nft_address,
            //     token_id,
            // ).unwrap_err(),Error::AuctionAlreadyStarted);
            // // Token 1 does not exists.

            // assert_eq!(get_balance(fee_recipient()), 10);
            assert_eq!(auction.highest_bids.get((nft_address, token_id)), None);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 3);
            assert_bid_refunded_event(
                &emitted_events[1],
                nft_address,
                token_id,
                caller,
                bid_amount,
            );
            assert_bid_withdrawn_event(
                &emitted_events[2],
                nft_address,
                token_id,
                caller,
                bid_amount,
            );
        }

        #[ink::test]
        fn withdraw_bid_failed_if_the_contract_is_paused() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(auction.bid_withdrawal_lock_time);
            assert_eq!(auction.get_now(), auction.bid_withdrawal_lock_time);
            auction.is_paused = true;
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert_eq!(
                auction.withdraw_bid(nft_address, token_id).unwrap_err(),
                Error::ContractPaused
            );
        }

        #[ink::test]
        fn withdraw_bid_failed_if_the_caller_is_not_the_bidder_of_the_highest_bid() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(auction.bid_withdrawal_lock_time);
            assert_eq!(auction.get_now(), auction.bid_withdrawal_lock_time);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: eve(),
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert_eq!(
                auction.withdraw_bid(nft_address, token_id).unwrap_err(),
                Error::YouAreNotTheHighestBidder
            );
        }

        #[ink::test]
        fn withdraw_bid_failed_if_the_sum_of_43200_seconds_and_the_end_time_of_the_auction_is_greater_than_the_current_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(42000);
            assert_eq!(auction.get_now(), 42000);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert_eq!(
                auction.withdraw_bid(nft_address, token_id).unwrap_err(),
                Error::CanWithdrawOnlyAfter12HoursAfterAuctionEnded
            );
        }

        #[ink::test]
        fn withdraw_bid_failed_if_it_failed_when_the_contract_transfer_to_the_current_highest_bidder_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(auction.bid_withdrawal_lock_time);
            assert_eq!(auction.get_now(), auction.bid_withdrawal_lock_time);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            //  assert_eq!(get_balance(contract_id()), 0);
            set_balance(contract_id(), 0);
            assert_eq!(
                auction.withdraw_bid(nft_address, token_id).unwrap_err(),
                Error::InsufficientFunds
            );
        }

        #[ink::test]
        fn result_auction_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            //  assert_eq!(auction.result_auction(nft_address,
            //         token_id,
            //     ).unwrap_err(),Error::AuctionAlreadyStarted);
            assert!(auction.result_auction(nft_address, token_id).is_ok());

            // assert_eq!(get_balance(fee_recipient()), 10);
            assert_eq!(auction.auctions.get((nft_address, token_id)), None);
            assert!(auction.highest_bids.get((nft_address, token_id)).is_none());
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            let unit_price = 0;
            assert_auction_resulted_event(
                &emitted_events[1],
                caller,
                nft_address,
                token_id,
                caller,
                pay_token,
                unit_price,
                bid_amount,
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_caller_is_not_owner_of_token_id_in_erc721_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_caller_is_not_the_owner_of_the_auction() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: eve(),
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_end_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_zero(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::NoAuctionExists
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_end_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_greater_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);
            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::AuctionNotEnded
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_resulted_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_true(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: true,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            advance_blocks(12);

            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::AuctionAlreadyResulted
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_bidder_of_the_highest_bid_the_specified_nft_address_and_token_id_is_zero_address(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: AccountId::from([0x0; 32]),
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            advance_blocks(12);
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::NoOpenBids
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_reserve_price_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_greater_than_the_bid_of_the_the_highest_bid(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);

            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount - 1,
                    last_bid_time: auction.get_now(),
                },
            );
            advance_blocks(12);
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::HighestBidIsBelowReservePrice
            );
        }

        #[ink::test]
        fn result_auction_failed_if_the_contract_is_not_approved_by_the_caller_in_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::AuctioncNotApproved
            );
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_platform_fee_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 11;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;

            auction.test_transfer_fail.insert(
                &(
                    pay_token,
                    auction.fee_recipient,
                    auction.fee_recipient,
                    0,
                    platform_fee_above_reserve,
                ),
                &(),
            );
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendPlatformFee
            ); // Send platform fee
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_royalty_fee_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );

            let royalty = 1;
            let minter = eve();
            auction
                .test_minters_royalties
                .insert(&(nft_address, token_id), &(minter, royalty));

            auction.platform_fee = 1;
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;
            let pay_amount = bid_amount - platform_fee_above_reserve;
            let royalty_fee = pay_amount * royalty / 10000;

            auction
                .test_transfer_fail
                .insert(&(pay_token, minter, minter, 0, royalty_fee), &());
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendTheOwnerTheirRoyalties
            ); // Send royalty fee
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_collection_royalty_fee_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            let royalty = 1;
            let minter = eve();
            auction
                .test_collection_royalties
                .insert(&nft_address, &(minter, royalty));

            auction.platform_fee = 1;
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;
            let pay_amount = bid_amount - platform_fee_above_reserve;
            let royalty_fee = pay_amount * royalty / 10000;

            auction
                .test_transfer_fail
                .insert(&(pay_token, minter, minter, 0, royalty_fee), &());
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendTheRoyalties
            ); // Send collection royalty fee
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_fee_recipient_in_the_pay_token_erc20_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            let royalty = 1;
            auction
                .test_minters_royalties
                .insert(&(nft_address, token_id), &(eve(), royalty));

            auction.platform_fee = 1;
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;
            let pay_amount = bid_amount - platform_fee_above_reserve;
            let royalty_fee = pay_amount * royalty / 10000;
            let pay_amount = pay_amount - royalty_fee;
            auction
                .test_transfer_fail
                .insert(&(pay_token, caller, caller, 0, pay_amount), &());
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendTheOwnerTheAuctionBalance
            ); //
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_platform_fee_to_the_fee_recipient_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 11 * 1000;

            set_caller(caller);

            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            set_balance(contract_id(), 0);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendPlatformFee
            ); // Send platform fee
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_royalty_fee_to_the_fee_recipient_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10 * 10000;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            let royalty = 1;
            auction
                .test_minters_royalties
                .insert(&(nft_address, token_id), &(eve(), royalty));

            auction.platform_fee = 1;
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;
            // let pay_amount = bid_amount - platform_fee_above_reserve;
            // let royalty_fee = pay_amount * royalty / 10000;
            //let pay_amount = pay_amount - royalty_fee;
            set_balance(contract_id(), platform_fee_above_reserve);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendTheOwnerTheirRoyalties
            ); // Send royalty fee
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_collection_royalty_fee_to_the_fee_recipient_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10 * 10000;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            let royalty = 1;
            auction
                .test_collection_royalties
                .insert(&nft_address, &(eve(), royalty));

            auction.platform_fee = 1;
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;
            // let pay_amount = bid_amount - platform_fee_above_reserve;
            // let royalty_fee = pay_amount * royalty / 10000;
            //let pay_amount = pay_amount - royalty_fee;
            set_balance(contract_id(), platform_fee_above_reserve);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendTheRoyalties
            ); // Send collection royalty fee
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_contract_transfer_pay_amount_to_the_fee_recipient_in_the_native_token(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = AccountId::from([0x0; 32]);
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            let royalty = 1;
            auction
                .test_minters_royalties
                .insert(&(nft_address, token_id), &(eve(), royalty));

            auction.platform_fee = 1;
            let platform_fee_above_reserve =
                (bid_amount - reserve_price) * auction.platform_fee / 1000;
            // let pay_amount = bid_amount - platform_fee_above_reserve;
            // let royalty_fee = pay_amount * royalty / 10000;
            //let pay_amount = pay_amount - royalty_fee;
            set_balance(contract_id(), platform_fee_above_reserve);
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());

            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::FailedToSendTheOwnerTheAuctionBalance
            ); //
        }

        #[ink::test]
        fn result_auction_failed_if_it_failed_when_the_owner_of_nft_address_and_token_id_transfer_token_id_to_the_winner_in_the_nft_address_erc721_contract(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            advance_blocks(43212);
            assert_eq!(auction.get_now(), 43212);
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);
            auction
                .test_operator_approvals
                .insert(&(caller, contract_id()), &());
            auction
                .test_transfer_fail
                .insert(&(nft_address, caller, caller, token_id, 0), &());
            assert_eq!(
                auction.result_auction(nft_address, token_id).unwrap_err(),
                Error::NotOwneAndOrContractNotApproved
            ); //
        }

        #[ink::test]
        fn cancel_auction_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            // set_balance(caller,10);
            // set_balance(fee_recipient(),0);
            // ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);

            assert!(auction.cancel_auction(nft_address, token_id).is_ok());

            // assert_eq!(get_balance(fee_recipient()), 10);
            assert_eq!(auction.auctions.get((nft_address, token_id)), None);
            assert!(auction.highest_bids.get((nft_address, token_id)).is_none());
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 3);
            // let unit_price = 0;
            assert_auction_cancelled_event(&emitted_events[2], nft_address, token_id);
        }

        #[ink::test]
        fn cancel_auction_failed_if_the_caller_is_not_owner_of_token_id_in_erc721_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert_eq!(
                auction.cancel_auction(nft_address, token_id).unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn cancel_auction_failed_if_the_caller_is_not_the_owner_of_the_auction() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: eve(),
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.cancel_auction(nft_address, token_id).unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn cancel_auction_failed_if_the_end_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_zero(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.cancel_auction(nft_address, token_id).unwrap_err(),
                Error::NoAuctionExists
            );
        }

        #[ink::test]
        fn cancel_auction_failed_if_the_resulted_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_true(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            let bid_amount: Balance = 10;

            set_caller(caller);
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            let end_time: u128 = start_time + 1;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: true,
                },
            );
            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            auction.test_token_owner.insert(&token_id, &caller);

            assert_eq!(
                auction.cancel_auction(nft_address, token_id).unwrap_err(),
                Error::AuctionAlreadyResulted
            );
        }

        #[ink::test]
        fn toggle_is_paused_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let is_paused = auction.is_paused;
            assert!(auction.toggle_is_paused().is_ok());

            assert_eq!(auction.is_paused, !is_paused);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_pause_toggled_event(&emitted_events[1], !is_paused);
        }

        #[ink::test]
        fn toggle_is_paused_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            assert_eq!(auction.toggle_is_paused().unwrap_err(), Error::OnlyOwner);
        }

        #[ink::test]
        fn update_min_bid_increment_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let min_bid_increment = 10;
            assert!(auction.update_min_bid_increment(min_bid_increment).is_ok());

            assert_eq!(auction.min_bid_increment, min_bid_increment);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_min_bid_increment_event(&emitted_events[1], min_bid_increment);
        }

        #[ink::test]
        fn update_min_bid_increment_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let min_bid_increment = 10;
            assert_eq!(
                auction
                    .update_min_bid_increment(min_bid_increment)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_bid_withdrawal_lock_time_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let bid_withdrawal_lock_time = 10;
            assert!(auction
                .update_bid_withdrawal_lock_time(bid_withdrawal_lock_time)
                .is_ok());

            assert_eq!(auction.bid_withdrawal_lock_time, bid_withdrawal_lock_time);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_bid_withdrawal_lock_time_event(
                &emitted_events[1],
                bid_withdrawal_lock_time,
            );
        }

        #[ink::test]
        fn update_bid_withdrawal_lock_time_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let bid_withdrawal_lock_time = 10;
            assert_eq!(
                auction
                    .update_bid_withdrawal_lock_time(bid_withdrawal_lock_time)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_auction_reserve_price_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_reserve_price = 10;
            assert!(auction
                .update_auction_reserve_price(nft_address, token_id, auction_reserve_price)
                .is_ok());

            assert_eq!(
                auction
                    .auctions
                    .get(&(nft_address, token_id))
                    .unwrap()
                    .reserve_price,
                auction_reserve_price
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_auction_reserve_price_event(
                &emitted_events[1],
                nft_address,
                token_id,
                pay_token,
                auction_reserve_price,
            );
        }

        #[ink::test]
        fn update_auction_reserve_price_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: eve(),
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_reserve_price = 10;

            assert_eq!(
                auction
                    .update_auction_reserve_price(nft_address, token_id, auction_reserve_price)
                    .unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn update_auction_reserve_price_failed_if_the_resulted_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_true(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: true,
                },
            );
            let auction_reserve_price = 10;

            assert_eq!(
                auction
                    .update_auction_reserve_price(nft_address, token_id, auction_reserve_price)
                    .unwrap_err(),
                Error::AuctionAlreadyResulted
            );
        }

        #[ink::test]
        fn update_auction_reserve_price_failed_if_the_end_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_zero(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_reserve_price = 10;

            assert_eq!(
                auction
                    .update_auction_reserve_price(nft_address, token_id, auction_reserve_price)
                    .unwrap_err(),
                Error::NoAuctionExists
            );
        }

        #[ink::test]
        fn update_auction_start_time_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_start_time = 10;
            // assert_eq!(auction.update_auction_start_time(nft_address, token_id,auction_start_time
            // ).unwrap_err(),Error::AuctionAlreadyStarted);
            assert!(auction
                .update_auction_start_time(nft_address, token_id, auction_start_time)
                .is_ok());
            assert_eq!(
                auction
                    .auctions
                    .get(&(nft_address, token_id))
                    .unwrap()
                    .start_time,
                auction_start_time
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_auction_start_time_event(
                &emitted_events[1],
                nft_address,
                token_id,
                auction_start_time,
            );
        }

        #[ink::test]
        fn update_auction_start_time_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: eve(),
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_start_time = 10;
            assert_eq!(
                auction
                    .update_auction_start_time(nft_address, token_id, auction_start_time)
                    .unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn update_auction_start_time_failed_if_the_start_time_is_zero() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_start_time = 0;
            assert_eq!(
                auction
                    .update_auction_start_time(nft_address, token_id, auction_start_time)
                    .unwrap_err(),
                Error::InvalidStartTime
            );
        }

        #[ink::test]
        fn update_auction_start_time_failed_if_the_sum_of_60_seconds_and_the_start_time_of_the_auction_the_specified_nft_address_and_token_id_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 62;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_start_time = 100;
            advance_blocks(420);
            assert_eq!(
                auction
                    .update_auction_start_time(nft_address, token_id, auction_start_time)
                    .unwrap_err(),
                Error::AuctionAlreadyStarted
            );
        }

        #[ink::test]
        fn update_auction_start_time_failed_if_the_sum_of_300_seconds_and_the_start_time_is_greater_than_or_equal_to_the_end_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 300;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_start_time = 100;
            assert_eq!(
                auction
                    .update_auction_start_time(nft_address, token_id, auction_start_time)
                    .unwrap_err(),
                Error::StartTimeShouldBeLessThanEndTimeBy5Minutes
            );
        }

        #[ink::test]
        fn update_auction_start_time_failed_if_the_resulted_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_true(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: true,
                },
            );
            let auction_start_time = 10;
            assert_eq!(
                auction
                    .update_auction_start_time(nft_address, token_id, auction_start_time)
                    .unwrap_err(),
                Error::AuctionAlreadyResulted
            );
        }

        #[ink::test]
        fn update_auction_end_time_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_end_time = end_time + 10;
            // assert_eq!(auction.update_auction_end_time(nft_address, token_id,auction_end_time
            // ).unwrap_err(),Error::AuctionAlreadyStarted);
            assert!(auction
                .update_auction_end_time(nft_address, token_id, auction_end_time)
                .is_ok());
            assert_eq!(
                auction
                    .auctions
                    .get(&(nft_address, token_id))
                    .unwrap()
                    .end_time,
                auction_end_time
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_auction_end_time_event(
                &emitted_events[1],
                nft_address,
                token_id,
                auction_end_time,
            );
        }

        #[ink::test]
        fn update_auction_end_time_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: eve(),
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_end_time = end_time + 10;
            assert_eq!(
                auction
                    .update_auction_end_time(nft_address, token_id, auction_end_time)
                    .unwrap_err(),
                Error::SenderMustBeItemOwner
            );
        }

        #[ink::test]
        fn update_auction_end_time_failed_if_the_end_time_of_the_auction_the_specified_nft_address_and_token_id_is_less_than_or_equal_to_the_current_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = auction.get_now();
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_end_time = end_time + 10;
            assert_eq!(
                auction
                    .update_auction_end_time(nft_address, token_id, auction_end_time)
                    .unwrap_err(),
                Error::AuctionAlreadyEnded
            );
        }

        #[ink::test]
        fn update_auction_end_time_failed_if_the_start_time_of_the_auction_of_the_specified_nft_contract_address_and_token_id_is_greater_than_or_equal_to_the_end_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 61;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_end_time = start_time;
            assert_eq!(
                auction
                    .update_auction_end_time(nft_address, token_id, auction_end_time)
                    .unwrap_err(),
                Error::EndTimeMustBeGreaterThanStart
            );
        }

        #[ink::test]
        fn update_auction_end_time_failed_if_the_sum_of_300_seconds_and_the_current_time_is_greater_than_or_equal_to_the_end_time(
        ) {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 0;
            // let bid_amount: Balance = 10;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now() + 60;
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time + 302;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            let auction_end_time = auction.get_now() + 61;
            assert_eq!(
                auction
                    .update_auction_end_time(nft_address, token_id, auction_end_time)
                    .unwrap_err(),
                Error::AuctionShouldEndAfter5Minutes
            );
        }

        #[ink::test]
        fn update_platform_fee_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let platform_fee = 10;
            assert!(auction.update_platform_fee(platform_fee).is_ok());

            assert_eq!(auction.platform_fee, platform_fee);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_platform_fee_event(&emitted_events[1], platform_fee);
        }

        #[ink::test]
        fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let platform_fee = 10;
            assert_eq!(
                auction.update_platform_fee(platform_fee).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_recipient_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let fee_recipient = bob();
            assert!(auction.update_platform_fee_recipient(fee_recipient).is_ok());

            assert_eq!(auction.fee_recipient, fee_recipient);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_update_platform_fee_recipient_event(&emitted_events[1], fee_recipient);
        }

        #[ink::test]
        fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let fee_recipient = bob();
            assert_eq!(
                auction
                    .update_platform_fee_recipient(fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }
        #[ink::test]
        fn update_address_registry_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let address_registry = bob();
            assert!(auction.update_address_registry(address_registry).is_ok());

            assert_eq!(auction.address_registry, address_registry);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_sub_auction_contract_deployed_event(&emitted_events[0]);
        }

        #[ink::test]
        fn update_address_registry_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let address_registry = bob();
            assert_eq!(
                auction
                    .update_address_registry(address_registry)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }
        #[ink::test]
        fn get_auction_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 1;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let end_time: u128 = start_time;
            let min_bid = reserve_price;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );
            assert_eq!(
                auction.get_auction(nft_address, token_id),
                (
                    caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    false
                )
            );
        }
        #[ink::test]
        fn get_auction_start_time_resulted_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let nft_address: AccountId = alice();
            let token_id: TokenId = 1;
            let pay_token: AccountId = alice();
            let reserve_price: Balance = 10;
            let start_time: u128 = auction.get_now();
            // let min_bid_reserve: bool = true;
            let min_bid = 0;
            let end_time: u128 = start_time;
            auction.auctions.insert(
                &(nft_address, token_id),
                &Auction {
                    owner: caller,
                    pay_token,
                    min_bid,
                    reserve_price,
                    start_time,
                    end_time,
                    resulted: false,
                },
            );

            assert_eq!(
                auction.get_auction_start_time_resulted(nft_address, token_id),
                (start_time, false)
            );
        }

        #[ink::test]
        fn get_highest_bid_works() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            let nft_address: AccountId = alice();
            let token_id: TokenId = 1;
            let bid_amount: Balance = 10;

            set_caller(caller);

            auction.highest_bids.insert(
                &(nft_address, token_id),
                &HighestBid {
                    bidder: caller,
                    bid: bid_amount,
                    last_bid_time: auction.get_now(),
                },
            );
            assert_eq!(
                auction.get_highest_bid(nft_address, token_id),
                (caller, bid_amount, auction.get_now())
            );
        }

        #[ink::test]
        fn reclaim_erc20_fails() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = alice();
            set_caller(caller);
            let token_contract: AccountId = AccountId::default();

            assert_eq!(
                auction.reclaim_erc20(token_contract).unwrap_err(),
                Error::InvalidAddress
            );
        }

        #[ink::test]
        fn reclaim_erc20_fails_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut auction = init_contract();
            let caller = bob();
            set_caller(caller);
            let token_contract: AccountId = AccountId::default();

            assert_eq!(
                auction.reclaim_erc20(token_contract).unwrap_err(),
                Error::OnlyOwner
            );
        }
        fn assert_sub_auction_contract_deployed_event(event: &ink_env::test::EmittedEvent) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::SubAuctionContractDeployed(SubAuctionContractDeployed {}) = decoded_event
            {
            } else {
                panic!("encountered unexpected event kind: expected a SubAuctionContractDeployed event")
            }
        }

        fn assert_pause_toggled_event(
            event: &ink_env::test::EmittedEvent,
            expected_is_paused: bool,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::PauseToggled(PauseToggled { is_paused }) = decoded_event {
                assert_eq!(
                    is_paused, expected_is_paused,
                    "encountered invalid PauseToggled.is_paused"
                );
            } else {
                panic!("encountered unexpected event kind: expected a PauseToggled event")
            }
        }

        fn assert_auction_created_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_pay_token: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::AuctionCreated(AuctionCreated {
                nft_address,
                token_id,
                pay_token,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid AuctionCreated.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid AuctionCreated.token_id"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid AuctionCreated.pay_token"
                );
            } else {
                panic!("encountered unexpected event kind: expected a AuctionCreated event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::AuctionCreated",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionCreated::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionCreated::token_id",
                    value: &expected_token_id,
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
        fn assert_update_auction_end_time_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_end_time: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdateAuctionEndTime(UpdateAuctionEndTime {
                nft_address,
                token_id,
                end_time,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid UpdateAuctionEndTime.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid UpdateAuctionEndTime.token_id"
                );
                assert_eq!(
                    end_time, expected_end_time,
                    "encountered invalid UpdateAuctionEndTime.end_time"
                );
            } else {
                panic!("encountered unexpected event kind: expected a UpdateAuctionEndTime event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::UpdateAuctionEndTime",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::UpdateAuctionEndTime::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::UpdateAuctionEndTime::token_id",
                    value: &expected_token_id,
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

        fn assert_update_auction_start_time_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_start_time: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdateAuctionStartTime(UpdateAuctionStartTime {
                nft_address,
                token_id,
                start_time,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid UpdateAuctionStartTime.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid UpdateAuctionStartTime.token_id"
                );
                assert_eq!(
                    start_time, expected_start_time,
                    "encountered invalid UpdateAuctionStartTime.start_time"
                );
            } else {
                panic!("encountered unexpected event kind: expected a UpdateAuctionStartTime event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::UpdateAuctionStartTime",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::UpdateAuctionStartTime::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::UpdateAuctionStartTime::token_id",
                    value: &expected_token_id,
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

        fn assert_update_auction_reserve_price_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_pay_token: AccountId,
            expected_reserve_price: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdateAuctionReservePrice(UpdateAuctionReservePrice {
                nft_address,
                token_id,
                pay_token,
                reserve_price,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid UpdateAuctionReservePrice.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid UpdateAuctionReservePrice.token_id"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid UpdateAuctionReservePrice.pay_token"
                );
                assert_eq!(
                    reserve_price, expected_reserve_price,
                    "encountered invalid UpdateAuctionReservePrice.reserve_price"
                );
            } else {
                panic!(
                    "encountered unexpected event kind: expected a UpdateAuctionReservePrice event"
                )
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::UpdateAuctionReservePrice",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::UpdateAuctionReservePrice::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::UpdateAuctionReservePrice::token_id",
                    value: &expected_token_id,
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

        fn assert_update_min_bid_increment_event(
            event: &ink_env::test::EmittedEvent,
            expected_min_bid_increment: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdateMinBidIncrement(UpdateMinBidIncrement { min_bid_increment }) =
                decoded_event
            {
                assert_eq!(
                    min_bid_increment, expected_min_bid_increment,
                    "encountered invalid UpdateMinBidIncrement.min_bid_increment"
                );
            } else {
                panic!("encountered unexpected event kind: expected a UpdateMinBidIncrement event")
            }
        }

        fn assert_update_bid_withdrawal_lock_time_event(
            event: &ink_env::test::EmittedEvent,
            expected_bid_withdrawal_lock_time: u128,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::UpdateBidWithdrawalLockTime(UpdateBidWithdrawalLockTime {
                bid_withdrawal_lock_time,
            }) = decoded_event
            {
                assert_eq!(
                    bid_withdrawal_lock_time, expected_bid_withdrawal_lock_time,
                    "encountered invalid UpdateBidWithdrawalLockTime.bid_withdrawal_lock_time"
                );
            } else {
                panic!("encountered unexpected event kind: expected a UpdateBidWithdrawalLockTime event")
            }
        }

        fn assert_bid_placed_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_bidder: AccountId,
            expected_bid: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::BidPlaced(BidPlaced {
                nft_address,
                token_id,
                bidder,
                bid,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid BidPlaced.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid BidPlaced.token_id"
                );
                assert_eq!(
                    bidder, expected_bidder,
                    "encountered invalid BidPlaced.bidder"
                );
                assert_eq!(bid, expected_bid, "encountered invalid BidPlaced.bid");
            } else {
                panic!("encountered unexpected event kind: expected a BidPlaced event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::BidPlaced",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::BidPlaced::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::BidPlaced::token_id",
                    value: &expected_token_id,
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

        fn assert_bid_withdrawn_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_bidder: AccountId,
            expected_bid: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::BidWithdrawn(BidWithdrawn {
                nft_address,
                token_id,
                bidder,
                bid,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid BidWithdrawn.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid BidWithdrawn.token_id"
                );
                assert_eq!(
                    bidder, expected_bidder,
                    "encountered invalid BidWithdrawn.bidder"
                );
                assert_eq!(bid, expected_bid, "encountered invalid BidWithdrawn.bid");
            } else {
                panic!("encountered unexpected event kind: expected a BidWithdrawn event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::BidWithdrawn",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::BidWithdrawn::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::BidWithdrawn::token_id",
                    value: &expected_token_id,
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

        fn assert_bid_refunded_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_bidder: AccountId,
            expected_bid: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::BidRefunded(BidRefunded {
                nft_address,
                token_id,
                bidder,
                bid,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid BidRefunded.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid BidRefunded.token_id"
                );
                assert_eq!(
                    bidder, expected_bidder,
                    "encountered invalid BidRefunded.bidder"
                );
                assert_eq!(bid, expected_bid, "encountered invalid BidRefunded.bid");
            } else {
                panic!("encountered unexpected event kind: expected a BidRefunded event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::BidRefunded",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::BidRefunded::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::BidRefunded::token_id",
                    value: &expected_token_id,
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

        fn assert_auction_resulted_event(
            event: &ink_env::test::EmittedEvent,
            expected_old_owner: AccountId,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
            expected_winner: AccountId,
            expected_pay_token: AccountId,
            expected_unit_price: Balance,
            expected_winning_bid: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::AuctionResulted(AuctionResulted {
                old_owner,
                nft_address,
                token_id,
                winner,
                pay_token,
                unit_price,
                winning_bid,
            }) = decoded_event
            {
                assert_eq!(
                    old_owner, expected_old_owner,
                    "encountered invalid AuctionResulted.old_owner"
                );
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid AuctionResulted.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid AuctionResulted.token_id"
                );
                assert_eq!(
                    winner, expected_winner,
                    "encountered invalid AuctionResulted.winner"
                );
                assert_eq!(
                    pay_token, expected_pay_token,
                    "encountered invalid AuctionResulted.pay_token"
                );
                assert_eq!(
                    unit_price, expected_unit_price,
                    "encountered invalid AuctionResulted.unit_price"
                );
                assert_eq!(
                    winning_bid, expected_winning_bid,
                    "encountered invalid AuctionResulted.winning_bid"
                );
            } else {
                panic!("encountered unexpected event kind: expected a AuctionResulted event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::AuctionResulted",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionResulted::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionResulted::token_id",
                    value: &expected_token_id,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionResulted::winner",
                    value: &expected_winner,
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
        fn assert_auction_cancelled_event(
            event: &ink_env::test::EmittedEvent,
            expected_nft_address: AccountId,
            expected_token_id: TokenId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::AuctionCancelled(AuctionCancelled {
                nft_address,
                token_id,
            }) = decoded_event
            {
                assert_eq!(
                    nft_address, expected_nft_address,
                    "encountered invalid AuctionCancelled.nft_address"
                );
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid AuctionCancelled.token_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a AuctionCancelled event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubAuction::AuctionCancelled",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionCancelled::nft_address",
                    value: &expected_nft_address,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubAuction::AuctionCancelled::token_id",
                    value: &expected_token_id,
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
