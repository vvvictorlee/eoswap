# Sub Art NFT Marketplace

## Introduce

## Overview


### Auction contract

#### Write Functions

* `create_auction(&mut self,nft_address: AccountId,token_id: TokenId,pay_token: AccountId,reserve_price: Balance,start_time: u128,min_bid_reserve: bool,end_time: u128,) -> Result<()> ` : Creates a new auction for a given item.
* `place_bid_native( &mut self, nft_address: AccountId, token_id: TokenId, ) -> Result<()>`: Places a new bid, out bidding the existing bidder if found and criteria is reached.
* `place_bid( &mut self, nft_address: AccountId, token_id: TokenId, bid_amount: Balance, ) -> Result<()> `: Places a new bid, out bidding the existing bidder if found and criteria is reached.
* `withdraw_bid(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()>`: Allows the hightest bidder to withdraw the bid (after 12 hours post auction's end).
* `result_auction(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()> `: Closes a finished auction and rewards the highest bidder.
* `cancel_auction(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()>`: Cancels and inflight and un-resulted auctions, returning the funds to the top bidder if found; 
* `toggle_is_paused(&mut self) -> Result<()> `: Toggling the pause flag.
* `update_min_bid_increment(&mut self, min_bid_increment: Balance) -> Result<()> `:update the amount by which bids have to increase, across all auctions.
* `update_bid_withdrawal_lock_time( &mut self, bid_withdrawal_lock_time: u128, ) -> Result<()> `: Update the global bid withdrawal lockout time.
* `update_auction_reserve_price( &mut self, nft_address: AccountId, token_id: TokenId, reserve_price: Balance, ) -> Result<()> `: Update the current reserve price for an auction.
* ` update_auction_start_time( &mut self, nft_address: AccountId, token_id: TokenId, start_time: u128, ) -> Result<()>`: Update the current start time for an auction.
* `update_auction_end_time( &mut self, nft_address: AccountId, token_id: TokenId, end_time: u128, ) -> Result<()>`: Update the current end time for an auction.
* `update_platform_fee(&mut self, platform_fee: Balance) -> Result<()>`: Method for updating platform fee.
* `update_platform_fee_recipient(&mut self, fee_recipient: AccountId) -> Result<()> `: Method for updating platform fee address.
* `update_address_registry(&mut self, address_registry: AccountId) -> Result<()> `: Update SubAddressRegistry contract.
* `eclaim_erc20(&mut self, token_contract: AccountId) -> Result<()> `: Reclaims ERC20 Compatible tokens for entire balance

#### Read Functions

* `get_auction( &self, nft_address: AccountId, token_id: TokenId, ) -> (AccountId, AccountId, Balance, Balance, u128, u128, bool)`: Method for getting all info about the auction.
* `get_auction_start_time_resulted( &self, nft_address: AccountId, token_id: TokenId, ) -> (u128, bool)`: Method for getting start time and resulted flag about the auction.
* `get_highest_bid( &self, nft_address: AccountId, token_id: TokenId, ) -> (AccountId, Balance, u128) `: Method for getting all info about the highest bidder.

#### Storage Define

```rust
       ///ERC721 Address -> Token ID -> Auction Parameters.
        auctions: Mapping<(AccountId, TokenId), Auction>,
       ///ERC721 Address -> Token ID -> HighestBid Parameters.
        highest_bids: Mapping<(AccountId, TokenId), HighestBid>,
       ///globally and across all auctions, the amount by which a bid has to increase.
        min_bid_increment: Balance,
       ///global bid withdrawal lock time20 minutes.
        bid_withdrawal_lock_time: u128,
       ///Platform fee.
        platform_fee: Balance,
       ///Platform fee receipient.
        fee_recipient: AccountId,
       ///Platform fee receipient.
        address_registry: AccountId,
       /// The paused of the contract.
        is_paused: bool,
       ///The contract owner
        owner: AccountId,

```

#### Event Define

```rust
    ///   Event emitted when a contract deployed occurs.
    #[ink(event)]
    pub struct SubAuctionContractDeployed {}
    #[ink(event)]
    pub struct PauseToggled {
        is_paused: bool,
    }
    ///   Event emitted when a auction created occurs.
    #[ink(event)]
    pub struct AuctionCreated {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        pay_token: AccountId,
    }

    ///   Event emitted when update auction end time occurs.
    #[ink(event)]
    pub struct UpdateAuctionEndTime {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        end_time: u128,
    }

    ///   Event emitted when update auction start time occurs.
    #[ink(event)]
    pub struct UpdateAuctionStartTime {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        start_time: u128,
    }

    ///   Event emitted when update auction reserve price occurs.
    #[ink(event)]
    pub struct UpdateAuctionReservePrice {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
        pay_token: AccountId,
        reserve_price: Balance,
    }

    ///   Event emitted when  update platform fee occurs.
    #[ink(event)]
    pub struct UpdatePlatformFee {
        platform_fee: Balance,
    }
    ///   Event emitted when  update platform fee recipient occurs.
    #[ink(event)]
    pub struct UpdatePlatformFeeRecipient {
        fee_recipient: AccountId,
    }

    ///   Event emitted when update bid withdrawal lock time occurs.
    #[ink(event)]
    pub struct UpdateMinBidIncrement {
        min_bid_increment: Balance,
    }
    ///   Event emitted when update bid withdrawal lock time occurs.
    #[ink(event)]
    pub struct UpdateBidWithdrawalLockTime {
        bid_withdrawal_lock_time: u128,
    }
    ///   Event emitted when a bid is placed.
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

    ///   Event emitted when bid withdrawn occurs.
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

    ///   Event emitted when a bid refunded occurs.
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
    ///   Event emitted when auction resulted occurs.
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

    ///   Event emitted when  auction cancelled occurs.
    #[ink(event)]
    pub struct AuctionCancelled {
        #[ink(topic)]
        nft_address: AccountId,
        #[ink(topic)]
        token_id: TokenId,
    }
```

#### Error Define

```rust
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
   -  Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
```

### Usage

#### create_auction

Creates a new auction for a given item.
Only the owner of item can create an auction and must have approved the contract.
In addition to owning the item, the sender also has to have the owner role.
End time for the auction must be in the future.
  + `nft_address` ERC 721 Address
  + `token_id` Token ID of the item being auctioned
  + `pay_token` Paying token
  + `reserve_price` Item cannot be sold for less than this or minBidIncrement, whichever is higher
  + `start_time` Unix epoch in seconds for the auction start time
  + `min_bid_reserve` Unix epoch in seconds for the auction end time.

#### place_bid_native

Places a new bid, out bidding the existing bidder if found and criteria is reached.
Only callable when the auction is open.
Bids from smart contracts are prohibited to  prevent griefing with always reverting receiver.
  + `nft_address` ERC 721 Address
  + `token_id` Token ID of the item being auctioned

#### place_bid

Places a new bid, out bidding the existing bidder if found and criteria is reached.
Only callable when the auction is open.
Bids from smart contracts are prohibited to prevent griefing with always reverting receiver.
  + `nft_address` ERC 721 Address
  + `token_id` Token ID of the item being auctioned
  + `bid_amount` Bid amount

#### withdraw_bid

Allows the hightest bidder to withdraw the bid (after 12 hours post auction's end).
Only callable by the existing top bidder.
  + `nft_address` ERC 721 Address
  + `token_id` Token ID of the item being auctioned

#### result_auction

Closes a finished auction and rewards the highest bidder.
Only admin or smart contract.
Auction can only be resulted if there has been a bidder and reserve met.
If there have been no bids, the auction needs to be cancelled instead using `cancelAuction()` .
  + `nft_address` ERC 721 Address
  + `token_id` Token ID of the item being auctioned

#### cancel_auction

Cancels and inflight and un-resulted auctions, returning the funds to the top bidder if found.
Only item owner.
  + `nft_address` ERC 721 Address.
  + `token_id` Token ID of the NFT being auctioned.

#### toggle_is_paused

Toggling the pause flag.

#### update_min_bid_increment

Update the amount by which bids have to increase, across all auctions.
  + `min_bid_increment` New bid step in smallest unit.

#### update_bid_withdrawal_lock_time

Update the global bid withdrawal lockout time.
  + `bid_withdrawal_lock_time` New bid withdrawal lock time.

#### update_auction_reserve_price

Update the current reserve price for an auction.
Auction must exist.
  + `nft_address` ERC 721 Address
  + `token_id` Token ID of the NFT being auctioned
  + `reserve_price` New reserve price ( the smallest unit )

#### update_auction_start_time

Update the current start time for an auction.
Auction must exist.
  + `nft_address` ERC 721 Address.
  + `token_id` Token ID of the NFT being auctioned.
  + `start_time` New start time (unix epoch in seconds).

#### update_auction_end_time

 Update the current end time for an auction.
  + `nft_address` ERC 721 Address.
  + `token_id` Token ID of the NFT being auctioned.
  + `end_time` New end time (unix epoch in seconds).

#### update_platform_fee

Method for updating platform fee.
  + `platform_fee` the platform fee to set.

#### update_platform_fee_recipient

Method for updating platform fee address.
  + `fee_recipient` address the address to sends the funds to.

#### update_address_registry

Update SubAddressRegistry contract.
  + `address_registry` the address of address registry contract.

#### get_auction

Method for getting all info about the auction.
  + `nft_address` ERC 721 Address 
  + `token_id` Token ID of the NFT being auctioned

#### get_auction_start_time_resulted

Method for getting start time and resulted flag about the auction.
   - `nft_address` ERC 721 Address
   - `token_id` Token ID of the NFT being auctioned

#### get_highest_bid

Method for getting all info about the highest bidder.
   - `nft_address` ERC 721 Address.
   - `token_id` Token ID of the NFT being auctioned.

#### reclaim_erc20

 Reclaims ERC20 Compatible tokens for entire balance.
 Only access controls admin.
  + `token_contract` The address of the token contract

### Marketplace contract

#### Write Functions

* `list_item(&mut self,nft_address: AccountId,token_id: TokenId,quantity: u128,pay_token: AccountId,price_per_item: Balance,start_time: u128,) -> Result<()>` : Method for listing NFT.

* `cancel_listing(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()>` : Method for canceling listed NFT.
* `update_listing(&mut self,nft_address: AccountId,token_id: TokenId,pay_token: AccountId,new_price: Balance,) -> Result<()>`:   Method for updating listed NFT.

* `buy_item(&mut self,nft_address: AccountId,token_id: TokenId,pay_token: AccountId,owner: AccountId,) -> Result<()>`:Method for buying listed NFT.

`create_offer(&mut self,nft_address: AccountId,token_id: TokenId,pay_token: AccountId,quantity: u128,price_per_item: Balance,deadline: u128,) -> Result<()> `:Method for offering item.

`cancel_offer(&mut self, nft_address: AccountId, token_id: TokenId) -> Result<()>` : Method for canceling the offer.
`accept_offer(&mut self,nft_address: AccountId,token_id: TokenId,creator: AccountId,) -> Result<()> `:Method for accepting the offer.

 `register_royalty(&mut self,nft_address: AccountId,token_id: TokenId,royalty: u128,) -> Result<()> `:Method for register royalty

`register_collection_royalty(&mut self,nft_address: AccountId,creator: AccountId,royalty: u128,fee_recipient: AccountId,) -> Result<()> `:Method for setting collection royalty.

`update_platform_fee(&mut self, platform_fee: Balance) -> Result<()>` : Method for updating platform fee.
`update_platform_fee_recipient(&mut self, fee_recipient: AccountId) -> Result<()>` : Method for updating platform fee address.
`update_address_registry(&mut self, address_registry: AccountId) -> Result<()>` : Update SubAddressRegistry contract.
`validate_item_sold(&mut self,nft_address: AccountId,token_id: TokenId,seller: AccountId,buyer: AccountId,) -> Result<()>`:Validate and cancel listing.

#### Read Functions

* `get_price(&self, pay_token: AccountId) -> Result<Balance>` : Method for getting the price of payment token from oracle price seed .
* ` minter_of(&self, owner: AccountId, token_id: TokenId) -> AccountId` : Method for getting the minter of the specified owner and token id .
* `royalty_of(&self, nft_address: AccountId, token_id: TokenId) -> u128`: Method for getting the royalty of the specified nft token contract address and token id.
*  `collection_royalty_of(&self, nft_address: AccountId) -> (AccountId, Balance)` : Method for getting the collection royalty of the specified nft token contract .
* `listing_of(&self,nft_address: AccountId,token_id: TokenId,owner: AccountId,) -> Listing ` :  Method for getting the offer of the specified nft token contract address .

* `offer_of(&self,nft_address: AccountId,token_id: TokenId,owner: AccountId,) -> Offer` : Method for getting the offer of the specified nft token contract address .

#### Storage Define

```rust

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

        

```

#### Event Define

```rust
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
```

#### Error Define

```rust
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
```


### Usage

#### list_item

Method for listing NFT.
        *  `nft_address` Address of NFT contract
        *  `token_id` Token ID of NFT
        *  `quantity` token amount to list (needed for ERC-1155 NFTs, set as 1 for ERC-721)
        *  `pay_token` Paying token
        *  `price_per_item` sale price for each iteam
        *  `start_time` scheduling for a future sale

#### cancel_listing

Method for canceling listed NFT.
        *  `nft_address` Address of NFT contract
        *  `token_id` Token ID of NFT

#### update_listing

Method for updating listed NFT.
        *  `nft_address` Address of NFT contract
        *  `token_id` Token ID of NFT
        * `pay_token` payment token
        * `new_price` New sale price for each iteam

#### buy_item

Method for buying listed NFT.
        *  `nft_address` NFT contract address
        *  `token_id` TokenId
        *  `pay_token` payment token
        *  `owner` the owner of the nft address and the token id

#### create_offer

 Method for offering item.
        *  `nft_address` NFT contract address
        *  `token_id` TokenId
        *  `pay_token` Paying token
        *  `quantity` Quantity of items
        *  `price_per_item` Price per item
        *  `deadline` Offer expiration

#### cancel_offer

Method for canceling the offer.
        *  `nft_address` NFT contract address
        *  `token_id` TokenId

#### accept_offer

Method for accepting the offer.
        *  `nft_address` NFT contract address
        *  `token_id` TokenId
        * `creator` Offer creator address

#### register_royalty

Method for register royalty.
        *  `nft_address` NFT contract address
        *  `token_id` TokenId
        * `royalty` Royalty

#### register_collection_royalty

Method for setting collection royalty.
        *  `nft_address` NFT contract address
        *  `creator`  creator
        *  `royalty` Royalty
        *  `fee_recipient` the fee recipient

#### update_platform_fee

Method for updating platform fee.
        *  `platform_fee` the platform fee to set

#### update_platform_fee_recipient

 Method for updating platform fee address.
        *  `fee_recipient` payable address the address to sends the funds to

#### update_address_registry

Update SubAddressRegistry contract.
        *  `address_registry` the address of address registry contract

#### validate_item_sold

Validate and cancel listing.
Only bundle marketplace can access.
        *  `nft_address` NFT contract address
        *  `token_id` Token Id
        *  `seller` the seller address of the item
        *  `buyer` the buyer address of the item

#### get_price

Method for getting the price of payment token from oracle price seed .
        *  `pay_token` Address of payment token

#### minter_of

Method for getting the minter of the specified owner and token id .
        *  owner the address of the token
        *  `token_id` the token id of the token

#### royalty_of

Method for getting the royalty of the specified nft token contract address and token id .
        *  `nft_address` the address of the nft token contract
        *  `token_id` the token id of the token

#### collection_royalty_of

Method for getting the collection royalty of the specified nft token contract address .
        *  `nft_address` the address of the nft token contract

#### listing_of

Method for getting the offer of the specified nft token contract address .
        *  `nft_address` the address of the nft token contract
        *  `token_id` the token id of the token
        *  `owner` the address of the token

#### offer_of

Method for getting the offer of the specified nft token contract address .
        *  `nft_address` the address of the nft token contract
        *  `token_id` the token id of the token
        *  `owner` the address of the token

### BundleMarketplace Contract

#### Write Functions

* `list_item(&mut self,bundle_id: String,nft_addresses: Vec<AccountId>,token_ids: Vec<TokenId>,quantities: Vec<u128>,pay_token: AccountId,price: Balance,start_time: u128,) -> Result<()>`: Method for listing NFT bundle.
* `cancel_listing(&mut self, bundle_id: String) -> Result<()>`: Method for canceling listed NFT bundle.
* `update_listing(&mut self,bundle_id: String,pay_token: AccountId,new_price: Balance,) -> Result<()>`: Method for updating listed NFT bundle.
`buy_item(&mut self, bundle_id: String, pay_token: AccountId) -> Result<()> `:Method for buying listed NFT bundle.

`create_offer(&mut self,bundle_id: String,pay_token: AccountId,price: Balance,deadline: u128,) -> Result<()>`:Method for offering bundle item.

`cancel_offer(&mut self, bundle_id: String) -> Result<()> `:Method for canceling the bundle offer.
`accept_offer(&mut self, bundle_id: String, creator: AccountId) -> Result<()>`:Method for accepting the bundle offer.
`update_platform_fee(&mut self, platform_fee: Balance) -> Result<()>` : Method for updating platform fee.
`update_platform_fee_recipient(&mut self, fee_recipient: AccountId) -> Result<()>` : Method for updating platform fee address.
`update_address_registry(&mut self, address_registry: AccountId) -> Result<()>` : Update SubAddressRegistry contract.
`validate_item_sold(&mut self,nft_address: AccountId,token_id: TokenId,quantity: u128,) -> Result<()> `:Validate and cancel listing.

#### Read Functions

* `get_listing(&self,owner: AccountId,bundle_id: String,) -> (Vec<AccountId>, Vec<TokenId>, Vec<u128>, Balance, u128) ` : Method for get NFT bundle listing.
* ` offer_of(&self, bundle_id: String, creator: AccountId) -> Offer` : Method for getting the offer of the specified nft token contract address.

#### Storage Define

```rust

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

        

```

#### Event Define

```rust
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
```

#### Error Define

```rust
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
```

### Usage

#### list_item

Method for listing NFT bundle.
       *  `bundle_id` Bundle ID
       *  `nft_addresses` Addresses of NFT contract
       *  `token_ids` Token IDs of NFT
       *  `quantities` token amounts to list (needed for ERC-1155 NFTs, set as 1 for ERC-721)
       *  `price` sale price for bundle
       *  `start_time` scheduling for a future sale

#### cancel_listing

Method for canceling listed NFT bundle.
       *   `nft_address`  Address of NFT contract

#### update_listing

Method for updating listed NFT bundle.
       *  `bundle_id` Bundle ID
       *  `pay_token`  payment token
       *  `new_price` New sale price for bundle

#### buy_item

Method for buying listed NFT bundle.
       *  `bundle_id` Bundle ID
       *  `pay_token`  payment token

#### create_offer

Method for offering bundle item.
       *  `bundle_id` Bundle ID
       *  `pay_token`  Paying token
       *  price Price
       *  deadline Offer expiration

#### cancel_offer

Method for canceling the bundle offer.
       *  `bundle_id` Bundle ID

#### accept_offer

Method for accepting the bundle offer.
       *  `bundle_id` Bundle ID
       *  `creator` Offer creator address


#### update_platform_fee

Method for updating platform fee.
        *  `platform_fee` the platform fee to set

#### update_platform_fee_recipient

 Method for updating platform fee address.
        *  `fee_recipient` payable address the address to sends the funds to

#### update_address_registry

Update SubAddressRegistry contract.
        *  `address_registry` the address of address registry contract

#### validate_item_sold
Validate and cancel listing.
Only marketplace can access.
        - `nft_address`  NFT contract address
        - `token_id` Token Id
        - `quantity` The quantity of NFTs

#### get_listing

Method for get NFT bundle listing
        
       *  `owner` Owner address
       *  `bundle_id` Bundle ID

#### offer_of

Method for getting the offer of the specified nft token contract address .
        
       *  `bundle_id` Bundle ID
       *  `creator` creator address
## Acknowledgements

It is inspired by existing projects & standards:

* [Artion](https://github.com/Fantom-foundation/Artion-Contracts.git)


