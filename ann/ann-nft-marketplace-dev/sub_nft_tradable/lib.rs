//! # NFT Tradable
//!
//! This is an custom ERC-721 Token  implementation.
//!
//! ## Warning
//!
//! This contract is an *example*. It is neither audited nor endorsed for production use.
//! Do **not** rely on it to keep anything of value secure.
//!
//! ## Overview
//!
//! This contract demonstrates how to build non-fungible or unique tokens using ink!.
//!
//! ## Error Handling
//!
//! Any function that modifies the state returns a `Result` type and does not changes the state
//! if the `Error` occurs.
//! The errors are defined as an `enum` type. Any other error or invariant violation
//! triggers a panic and therefore rolls back the transaction.
//!
//! ## Token Management
//!
//! After creating a new token, the function caller becomes the owner.
//! A token can be created, transferred, or destroyed.
//!
//! Token owners can assign other accounts for transferring specific tokens on their behalf.
//! It is also possible to authorize an operator (higher rights) for another account to handle tokens.
//!
//! ### Token Creation
//!
//! Token creation start by calling the `mint(&mut self, id: u32)` function.
//! The token owner becomes the function caller. The Token ID needs to be specified
//! as the argument on this function call.
//!
//! ### Token Transfer
//!
//! Transfers may be initiated by:
//! - The owner of a token
//! - The approved address of a token
//! - An authorized operator of the current owner of a token
//!
//! The token owner can transfer a token by calling the `transfer` or `transfer_from` functions.
//! An approved address can make a token transfer by calling the `transfer_from` function.
//! Operators can transfer tokens on another account's behalf or can approve a token transfer
//! for a different account.
//!
//! ### Token Removal
//!
//! Tokens can be destroyed by burning them. Only the token owner is allowed to burn a token.

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_nft_tradable::{SubNFTTradable, SubNFTTradableRef};

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
pub mod sub_nft_tradable {
    #[cfg_attr(test, allow(dead_code))]
    const INTERFACE_ID_ERC721: [u8; 4] = [0x80, 0xAC, 0x58, 0xCD];

    use ink_lang as ink;
    use ink_prelude::string::String;
    use ink_storage::{traits::SpreadAllocate, Mapping};

    use scale::{Decode, Encode};

    /// A token ID.
    pub type TokenId = u128;

    #[ink::trait_definition]
    pub trait Erc721 {
        /// Returns the balance of the owner.
        ///
        /// This represents the amount of unique tokens the owner has.
        #[ink(message)]
        fn balance_of(&self, owner: AccountId) -> u32;

        /// Returns the owner of the token.
        #[ink(message)]
        fn owner_of(&self, id: TokenId) -> Option<AccountId>;

        /// Returns the approved account ID for this token if any.
        #[ink(message)]
        fn get_approved(&self, id: TokenId) -> Option<AccountId>;

        /// Returns `true` if the operator is approved by the owner.
        #[ink(message)]
        fn is_approved_for_all(&self, owner: AccountId, operator: AccountId) -> bool;

        /// Approves or disapproves the operator for all tokens of the caller.
        #[ink(message)]
        fn set_approval_for_all(&mut self, to: AccountId, approved: bool) -> Result<()>;

        /// Approves the account to transfer the specified token on behalf of the caller.
        #[ink(message)]
        fn approve(&mut self, to: AccountId, id: TokenId) -> Result<()>;

        /// Transfer approved or owned token.
        #[ink(message)]
        fn transfer_from(&mut self, from: AccountId, to: AccountId, id: TokenId) -> Result<()>;
        /// Transfers the token from the caller to the given destination.
        #[ink(message)]
        fn transfer(&mut self, destination: AccountId, id: TokenId) -> Result<()>;

        /// Creates a new token.
        #[ink(message)]
        fn mint(&mut self, id: TokenId) -> Result<()>;

        /// Deletes an existing token. Only the owner can burn the token.
        #[ink(message)]
        fn burn(&mut self, id: TokenId) -> Result<()>;
    }
    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubNFTTradable {
        /// Mapping from token to owner.
        token_owner: Mapping<TokenId, AccountId>,
        /// Mapping from token to approvals users.
        token_approvals: Mapping<TokenId, AccountId>,
        /// Mapping from owner to number of owned token.
        owned_tokens_count: Mapping<AccountId, u32>,
        /// Mapping from owner to operator approvals.
        operator_approvals: Mapping<(AccountId, AccountId), ()>,
        ///  Current max tokenId
        token_id_nonce: TokenId,
        ///  Mapping from TokenID to  Uri
        token_uris: Mapping<TokenId, String>,
        /// Token name
        name: String,
        /// Token symbol
        symbol: String,
        ///  Auction contract
        auction: AccountId,
        ///  Marketplace contract
        marketplace: AccountId,
        ///  BundleMarketplace contract
        bundle_marketplace: AccountId,
        ///   Platform fee
        platform_fee: Balance,
        ///   Platform fee receipient
        fee_recipient: AccountId,
        /// The contract owner
        owner: AccountId,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        NotOwner,
        NotApproved,
        TokenExists,
        TokenNotFound,
        CannotInsert,
        CannotFetchValue,
        NotAllowed,
        InsufficientFundsToMint,
        OnlyOwnerOrApproved,
        OnlyOwner,
        TokenURIIsEmpty,
        DesignerIsZeroAddress,
        TokenShouldExist,
        OperatorQueryForNonExistentToken,
        TransferFailed,
        NewOwnerIsTheZeroAddress,
    }

    // The SubNFTTradable result types.
    pub type Result<T> = core::result::Result<T, Error>;
    //=============Sub  Begin============
    #[ink(event)]
    pub struct ContractCreated {
        pub creator: AccountId,
        pub nft_address: AccountId,
    }
    #[ink(event)]
    pub struct ContractDisabled {
        pub caller: AccountId,
        pub nft_address: AccountId,
    }
    //=============Sub  End============
    #[ink(event)]
    pub struct OwnershipTransferred {
        #[ink(topic)]
        previous_owner: AccountId,
        #[ink(topic)]
        new_owner: AccountId,
    }
    /// Event emitted when a token transfer occurs.
    #[ink(event)]
    pub struct Transfer {
        #[ink(topic)]
        from: Option<AccountId>,
        #[ink(topic)]
        to: Option<AccountId>,
        #[ink(topic)]
        id: TokenId,
    }

    /// Event emitted when a token approve occurs.
    #[ink(event)]
    pub struct Approval {
        #[ink(topic)]
        from: AccountId,
        #[ink(topic)]
        to: AccountId,
        #[ink(topic)]
        id: TokenId,
    }

    /// Event emitted when an operator is enabled or disabled for an owner.
    /// The operator can manage all NFTs of the owner.
    #[ink(event)]
    pub struct ApprovalForAll {
        #[ink(topic)]
        owner: AccountId,
        #[ink(topic)]
        operator: AccountId,
        approved: bool,
    }

    /// Event emitted when a token Minted occurs.
    #[ink(event)]
    pub struct Minted {
        token_id: TokenId,
        to: AccountId,
        token_uri: String,
        minter: AccountId,
    }

    /// Event emitted when a token UpdatePlatformFee occurs.
    #[ink(event)]
    pub struct UpdatePlatformFee {
        platform_fee: Balance,
    }
    /// Event emitted when a token UpdatePlatformFeeRecipient occurs.
    #[ink(event)]
    pub struct UpdatePlatformFeeRecipient {
        fee_recipient: AccountId,
    }
    impl SubNFTTradable {
        /// Creates a new SubNFTTradable contract.
        /// # Fields
        /// name token name
        /// symbol token symbol
        /// auction auction contract address
        /// marketplace  marketplace contract address
        /// bundle_marketplace bundle marketplace contract address
        /// platform_fee the platform fee to set
        /// fee_recipient payable address the address to sends the funds to
        #[ink(constructor)]
        pub fn new(
            name: String,
            symbol: String,
            auction: AccountId,
            marketplace: AccountId,
            bundle_marketplace: AccountId,
            platform_fee: Balance,
            fee_recipient: AccountId,
        ) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.name = name;
                contract.symbol = symbol;
                contract.auction = auction;
                contract.marketplace = marketplace;
                contract.bundle_marketplace = bundle_marketplace;
                contract.platform_fee = platform_fee;
                contract.fee_recipient = fee_recipient;
            })
        }
        /// Returns whether the specified interface supports
        /// # Fields
        /// interface_id interface id hash prefix 4 bytes
        #[ink(message)]
        pub fn supports_interface(&self, interface_id: [u8; 4]) -> bool {
            INTERFACE_ID_ERC721 == interface_id
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

        /// Mints a token to an address with a tokenURI.
        /// # Fields
        /// to address of the future owner of the token
        /// token_uri uri of the future owner of the token
        ///
        /// # Errors
        ///
        /// - If the transferred value is less than the `platform_fee`.
        /// - If it failed when the contract trasfer to  fee_recipient in native token.
        /// - If the `to` is zero address.
        #[ink(message, payable)]
        pub fn mint_nft(&mut self, to: AccountId, token_uri: String) -> Result<TokenId> {
            ensure!(
                self.env().transferred_value() >= self.platform_fee,
                Error::InsufficientFundsToMint
            );
            let minter = self.env().caller();

            self._assert_minting_params_valid(&token_uri, minter)?;
            let token_id = self.get_next_token_id();
            // Mint token and set token URI
            self.add_token_to(&to, token_id)?;
            self.set_token_uri(token_id, &token_uri)?;
            self.increment_token_type_id();
            // Send NativeToken fee to fee recipient
            ensure!(
                self.env()
                    .transfer(self.fee_recipient, self.env().transferred_value())
                    .is_ok(),
                Error::TransferFailed
            );
            self.env().emit_event(Minted {
                token_id,
                to: to,
                token_uri,
                minter,
            });
            Ok(token_id)
        }

        /// Deletes an existing token. Only the owner can burn the token.
        /// # Fields
        /// token_id address of the token
        ///
        /// # Errors
        ///
        /// - If th caller is not the owner of the token id  and the caller is not approved by the owner of the token id .
        #[ink(message)]
        pub fn burn_nft(&mut self, token_id: TokenId) -> Result<()> {
            let operator = self.env().caller();
            ensure!(
                self.owner_of(token_id) == Some(operator) || self.is_approved(token_id, operator),
                Error::OnlyOwnerOrApproved
            );

            self.burn(token_id)?;

            Ok(())
        }
        ///checks the specified operator is approved either for all  token ID  by the specified owner
        /// # Fields
        /// owner address of the owner of the token
        /// operator address of the operator of the token
        #[ink(message)]
        pub fn is_approved_for_all_nft(&self, owner: AccountId, operator: AccountId) -> bool {
            if self.auction == operator
                || self.marketplace == operator
                || self.bundle_marketplace == operator
            {
                return true;
            }
            self.is_approved_for_all(owner, operator)
        }
        /// Returns whether the specified token exists by checking to see if it has a creator
        /// id ID of the token to query the existence of
        /// #return bool whether the token exists
        #[ink(message)]
        pub fn exists(&self, id: TokenId) -> bool {
            if let Some(addr) = self.token_owner.get(&id) {
                addr != AccountId::from([0x0; 32])
            } else {
                false
            }
        }
        /// the current token ID based on value of token_id_nonce
        /// for the current token ID
        #[ink(message)]
        pub fn get_current_token_id(&self) -> TokenId {
            self.token_id_nonce
        }
        /// calculates the next token ID based on value of token_id_nonce
        /// for the next token ID
        fn get_next_token_id(&self) -> TokenId {
            self.token_id_nonce + 1
        }

        /// increments the value of token_id_nonce
        fn increment_token_type_id(&mut self) {
            self.token_id_nonce += 1;
        }

        ///checks the given token ID is approved either for all or the single token ID
        fn is_approved(&self, token_id: TokenId, operator: AccountId) -> bool {
            self.is_approved_for_all(
                self.owner_of(token_id)
                    .unwrap_or(AccountId::from([0x0; 32])),
                operator,
            ) || self.get_approved(token_id) == Some(operator)
        }

        pub fn is_approved_or_owner(
            &mut self,
            spender: AccountId,
            token_id: TokenId,
        ) -> Result<bool> {
            ensure!(
                self.exists(token_id),
                Error::OperatorQueryForNonExistentToken
            );
            let owner = self.owner_of(token_id).unwrap();
            if self.is_approved_for_all_nft(owner, spender) {
                return Ok(true);
            }

            Ok(self.approved_or_owner(Some(spender), token_id))
        }
        pub fn set_token_uri(&mut self, token_id: TokenId, uri: &String) -> Result<()> {
            ensure!(self.exists(token_id), Error::TokenShouldExist);
            self.token_uris.insert(&token_id, uri);

            Ok(())
        }

        /// Checks that the URI is not empty and the designer is a real address
        /// token_uri URI supplied on minting
        /// designer Address supplied on minting
        fn _assert_minting_params_valid(
            &self,
            token_uri: &String,
            designer: AccountId,
        ) -> Result<()> {
            ensure!(!token_uri.is_empty(), Error::TokenURIIsEmpty);
            ensure!(
                designer != AccountId::from([0x0; 32]),
                Error::DesignerIsZeroAddress
            );
            Ok(())
        }

        // ===========ERC721====================================
        #[ink(message)]
        pub fn transfer_ownership(&mut self, new_owner: AccountId) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);
            ensure!(
                AccountId::default() != new_owner,
                Error::NewOwnerIsTheZeroAddress
            );
            let previous_owner = self.owner;
            self.owner = new_owner;
            self.env().emit_event(OwnershipTransferred {
                previous_owner,
                new_owner,
            });
            Ok(())
        }
        /// Transfers token `id` `from` the sender to the `to` `AccountId`.
        fn transfer_token_from(
            &mut self,
            from: &AccountId,
            to: &AccountId,
            id: TokenId,
        ) -> Result<()> {
            let caller = self.env().caller();
            if !self.exists(id) {
                return Err(Error::TokenNotFound);
            };
            if !self.approved_or_owner(Some(caller), id) {
                return Err(Error::NotApproved);
            };
            self.clear_approval(id);
            self.remove_token_from(from, id)?;
            self.add_token_to(to, id)?;
            self.env().emit_event(Transfer {
                from: Some(*from),
                to: Some(*to),
                id,
            });
            Ok(())
        }

        /// Removes token `id` from the owner.
        fn remove_token_from(&mut self, from: &AccountId, id: TokenId) -> Result<()> {
            let Self {
                token_owner,
                owned_tokens_count,
                ..
            } = self;

            if token_owner.get(&id).is_none() {
                return Err(Error::TokenNotFound);
            }

            let count = owned_tokens_count
                .get(&from)
                .map(|c| c - 1)
                .ok_or(Error::CannotFetchValue)?;
            owned_tokens_count.insert(&from, &count);
            token_owner.remove(&id);

            Ok(())
        }

        /// Adds the token `id` to the `to` AccountID.
        fn add_token_to(&mut self, to: &AccountId, id: TokenId) -> Result<()> {
            let Self {
                token_owner,
                owned_tokens_count,
                ..
            } = self;

            if token_owner.get(&id).is_some() {
                return Err(Error::TokenExists);
            }

            if *to == AccountId::from([0x0; 32]) {
                return Err(Error::NotAllowed);
            };

            let count = owned_tokens_count.get(to).map(|c| c + 1).unwrap_or(1);

            owned_tokens_count.insert(to, &count);
            token_owner.insert(&id, to);

            Ok(())
        }

        /// Approves or disapproves the operator to transfer all tokens of the caller.
        fn approve_for_all(&mut self, to: AccountId, approved: bool) -> Result<()> {
            let caller = self.env().caller();
            if to == caller {
                return Err(Error::NotAllowed);
            }
            self.env().emit_event(ApprovalForAll {
                owner: caller,
                operator: to,
                approved,
            });

            if approved {
                self.operator_approvals.insert((&caller, &to), &());
            } else {
                self.operator_approvals.remove((&caller, &to));
            }

            Ok(())
        }

        /// Approve the passed `AccountId` to transfer the specified token on behalf of the message's sender.
        fn approve_for(&mut self, to: &AccountId, id: TokenId) -> Result<()> {
            let caller = self.env().caller();
            let owner = self.owner_of(id);
            if !(owner == Some(caller)
                || self.approved_for_all(owner.expect("Error with AccountId"), caller))
            {
                return Err(Error::NotAllowed);
            };

            if *to == AccountId::from([0x0; 32]) {
                return Err(Error::NotAllowed);
            };

            if self.token_approvals.get(&id).is_some() {
                return Err(Error::CannotInsert);
            } else {
                self.token_approvals.insert(&id, to);
            }

            self.env().emit_event(Approval {
                from: caller,
                to: *to,
                id,
            });

            Ok(())
        }

        /// Removes existing approval from token `id`.
        fn clear_approval(&mut self, id: TokenId) {
            self.token_approvals.remove(&id);
        }

        // Returns the total number of tokens from an account.
        fn balance_of_or_zero(&self, of: &AccountId) -> u32 {
            self.owned_tokens_count.get(of).unwrap_or(0)
        }

        /// Gets an operator on other Account's behalf.
        fn approved_for_all(&self, owner: AccountId, operator: AccountId) -> bool {
            self.operator_approvals.get((&owner, &operator)).is_some()
        }

        /// Returns true if the `AccountId` `from` is the owner of token `id`
        /// or it has been approved on behalf of the token `id` owner.
        fn approved_or_owner(&self, from: Option<AccountId>, id: TokenId) -> bool {
            let owner = self.owner_of(id);
            from != Some(AccountId::from([0x0; 32]))
                && (from == owner
                    || from == self.token_approvals.get(&id)
                    || self.approved_for_all(
                        owner.expect("Error with AccountId"),
                        from.expect("Error with AccountId"),
                    ))
        }

        // /// Returns true if token `id` exists or false if it does not.
        // fn exists(&self, id: TokenId) -> bool {
        //     self.token_owner.get(&id).is_some()
        // }
    }

    impl Erc721 for SubNFTTradable {
        /// Returns the balance of the owner.
        ///
        /// This represents the amount of unique tokens the owner has.
        #[ink(message)]
        fn balance_of(&self, owner: AccountId) -> u32 {
            self.balance_of_or_zero(&owner)
        }

        /// Returns the owner of the token.
        #[ink(message)]
        fn owner_of(&self, id: TokenId) -> Option<AccountId> {
            self.token_owner.get(&id)
        }

        /// Returns the approved account ID for this token if any.
        #[ink(message)]
        fn get_approved(&self, id: TokenId) -> Option<AccountId> {
            self.token_approvals.get(&id)
        }

        /// Returns `true` if the operator is approved by the owner.
        #[ink(message)]
        fn is_approved_for_all(&self, owner: AccountId, operator: AccountId) -> bool {
            self.approved_for_all(owner, operator)
        }

        /// Approves or disapproves the operator for all tokens of the caller.
        #[ink(message)]
        fn set_approval_for_all(&mut self, to: AccountId, approved: bool) -> Result<()> {
            self.approve_for_all(to, approved)
        }

        /// Approves the account to transfer the specified token on behalf of the caller.
        #[ink(message)]
        fn approve(&mut self, to: AccountId, id: TokenId) -> Result<()> {
            self.approve_for(&to, id)
        }

        /// Transfer approved or owned token.
        #[ink(message)]
        fn transfer_from(&mut self, from: AccountId, to: AccountId, id: TokenId) -> Result<()> {
            self.transfer_token_from(&from, &to, id)
        }
        /// Transfers the token from the caller to the given destination.
        #[ink(message)]
        fn transfer(&mut self, destination: AccountId, id: TokenId) -> Result<()> {
            self.transfer_token_from(&self.env().caller(), &destination, id)
        }

        /// Creates a new token.
        #[ink(message)]
        fn mint(&mut self, id: TokenId) -> Result<()> {
            let caller = self.env().caller();
            self.add_token_to(&caller, id)?;
            self.env().emit_event(Transfer {
                from: Some(AccountId::from([0x0; 32])),
                to: Some(caller),
                id,
            });
            Ok(())
        }

        /// Deletes an existing token. Only the owner can burn the token.
        #[ink(message)]
        fn burn(&mut self, id: TokenId) -> Result<()> {
            let caller = self.env().caller();

            let owner = self.token_owner.get(&id).ok_or(Error::TokenNotFound)?;
            if owner != caller {
                if !self.is_approved(id, caller) {
                    return Err(Error::NotOwner);
                }
            };

            let count = self
                .owned_tokens_count
                .get(&caller)
                .map(|c| c - 1)
                .ok_or(Error::CannotFetchValue)?;
            self.owned_tokens_count.insert(&caller, &count);
            self.token_owner.remove(&id);

            self.env().emit_event(Transfer {
                from: Some(caller),
                to: Some(AccountId::from([0x0; 32])),
                id,
            });

            Ok(())
        }
    }

    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_env::Clear;
        use ink_lang as ink;
        type Event = <SubNFTTradable as ::ink_lang::reflect::ContractEventBase>::Type;

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
        fn set_balance(account_id: AccountId, balance: Balance) {
            ink_env::test::set_account_balance::<ink_env::DefaultEnvironment>(account_id, balance)
        }
        fn get_balance(account_id: AccountId) -> Balance {
            ink_env::test::get_account_balance::<ink_env::DefaultEnvironment>(account_id)
                .expect("Cannot get account balance")
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
            django()
        }
        fn init_contract() -> SubNFTTradable {
            let erc = SubNFTTradable::new(
                String::from("test"),
                String::from("TEST"),
                charlie(),
                eve(),
                frank(),
                10,
                fee_recipient(),
            );

            erc
        }
        #[ink::test]
        fn mint_nft_works() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = bob();
            let token_uri = String::from("token_uri:bob");
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

            let token_id_result = nft_tradable.mint_nft(to, token_uri.clone());
            assert!(token_id_result.is_ok());
            assert_eq!(get_balance(fee_recipient()), 10);

            assert_eq!(nft_tradable.owned_tokens_count.get(to), Some(1));
            assert_eq!(
                nft_tradable.token_owner.get(&token_id_result.unwrap()),
                Some(to)
            );
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_minted_event(
                &emitted_events[0],
                token_id_result.unwrap(),
                to,
                token_uri,
                caller,
            );
        }

        #[ink::test]
        fn mint_nft_failed_if_the_transferred_value_is_less_than_the_platform_fee() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = frank();
            let token_uri = String::from("token_uri:frank");
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(1);

            assert_eq!(
                nft_tradable.mint_nft(to, token_uri.clone()).unwrap_err(),
                Error::InsufficientFundsToMint
            );
        }

        #[ink::test]
        fn mint_nft_failed_if_the_token_uri_is_empty() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = eve();
            let token_uri = String::from("");
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

            assert_eq!(
                nft_tradable.mint_nft(to, token_uri.clone()).unwrap_err(),
                Error::TokenURIIsEmpty
            );
        }

        #[ink::test]
        fn mint_nft_failed_if_the_to_is_zero_address() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = AccountId::from([0x0; 32]);
            let token_uri = String::from("token_uri:bob");
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

            assert_eq!(
                nft_tradable.mint_nft(to, token_uri.clone()).unwrap_err(),
                Error::NotAllowed
            );
        }

        #[ink::test]
        fn burn_nft_works() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = caller;
            nft_tradable.token_id_nonce += 1;
            let token_id = nft_tradable.token_id_nonce;
            nft_tradable.owned_tokens_count.insert(to, &1);
            nft_tradable.token_owner.insert(&token_id, &to);
            assert!(nft_tradable.burn_nft(token_id).is_ok());

            assert_eq!(nft_tradable.owned_tokens_count.get(to), Some(0));
            assert_eq!(nft_tradable.token_owner.get(&token_id), None);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_transfer_event(
                &emitted_events[0],
                Some(caller),
                Some(AccountId::from([0x0; 32])),
                token_id,
            );
        }

        #[ink::test]
        fn burn_nft_failed_if_the_caller_is_not_the_owner_of_the_token_id_and_the_caller_is_not_approved_by_the_owner_of_the_token_id(
        ) {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = caller;
            nft_tradable.token_id_nonce += 1;
            let token_id = nft_tradable.token_id_nonce;
            nft_tradable.owned_tokens_count.insert(to, &1);

            assert_eq!(
                nft_tradable.burn_nft(token_id).unwrap_err(),
                Error::OnlyOwnerOrApproved
            );
        }

        #[ink::test]
        fn update_platform_fee_works() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let platform_fee = 10;
            assert!(nft_tradable.update_platform_fee(platform_fee).is_ok());

            assert_eq!(nft_tradable.platform_fee, platform_fee);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_update_platform_fee_event(&emitted_events[0], platform_fee);
        }

        #[ink::test]
        fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = charlie();
            set_caller(caller);
            let platform_fee = 10;
            assert_eq!(
                nft_tradable.update_platform_fee(platform_fee).unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn update_platform_fee_recipient_works() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let fee_recipient = bob();
            assert!(nft_tradable
                .update_platform_fee_recipient(fee_recipient)
                .is_ok());

            assert_eq!(nft_tradable.fee_recipient, fee_recipient);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_update_platform_fee_recipient_event(&emitted_events[0], fee_recipient);
        }

        #[ink::test]
        fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut nft_tradable = init_contract();
            let caller = charlie();
            set_caller(caller);
            let fee_recipient = bob();
            assert_eq!(
                nft_tradable
                    .update_platform_fee_recipient(fee_recipient)
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        fn assert_minted_event(
            event: &ink_env::test::EmittedEvent,
            expected_token_id: TokenId,
            expected_to: AccountId,
            expected_token_uri: String,
            expected_minter: AccountId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::Minted(Minted {
                token_id,
                to,
                token_uri,
                minter,
            }) = decoded_event
            {
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid Minted.token_id"
                );
                assert_eq!(to, expected_to, "encountered invalid Minted.to");
                assert_eq!(
                    token_uri, expected_token_uri,
                    "encountered invalid Minted.token_uri"
                );
                assert_eq!(minter, expected_minter, "encountered invalid Minted.minter");
            } else {
                panic!("encountered unexpected event kind: expected a Minted event")
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
        fn assert_transfer_event(
            event: &ink_env::test::EmittedEvent,
            expected_from: Option<AccountId>,
            expected_to: Option<AccountId>,
            expected_token_id: TokenId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::Transfer(Transfer { from, to, id }) = decoded_event {
                assert_eq!(from, expected_from, "encountered invalid Transfer.from");
                assert_eq!(to, expected_to, "encountered invalid Transfer.to");
                assert_eq!(
                    id, expected_token_id,
                    "encountered invalid Transfer.token_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a Transfer event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubNFTTradable::Transfer",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubNFTTradable::Transfer::from",
                    value: &expected_from,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubNFTTradable::Transfer::to",
                    value: &expected_to,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubNFTTradable::Transfer::id",
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
        //==================================ERC721=============
        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<ink_env::DefaultEnvironment>(sender);
        }
        #[ink::test]
        fn mint_works() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Token 1 does not exists.
            assert_eq!(erc721.owner_of(1), None);
            // Alice does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.alice), 0);
            // Create token Id 1.
            assert_eq!(erc721.mint(1), Ok(()));
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
        }

        #[ink::test]
        fn mint_existing_should_fail() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1.
            assert_eq!(erc721.mint(1), Ok(()));
            // The first Transfer event takes place
            assert_eq!(1, ink_env::test::recorded_events().count());
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Alice owns token Id 1.
            assert_eq!(erc721.owner_of(1), Some(accounts.alice));
            // Cannot create  token Id if it exists.
            // Bob cannot own token Id 1.
            assert_eq!(erc721.mint(1), Err(Error::TokenExists));
        }

        #[ink::test]
        fn transfer_works() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1 for Alice
            assert_eq!(erc721.mint(1), Ok(()));
            // Alice owns token 1
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Bob does not owns any token
            assert_eq!(erc721.balance_of(accounts.bob), 0);
            // The first Transfer event takes place
            assert_eq!(1, ink_env::test::recorded_events().count());
            // Alice transfers token 1 to Bob
            assert_eq!(erc721.transfer(accounts.bob, 1), Ok(()));
            // The second Transfer event takes place
            assert_eq!(2, ink_env::test::recorded_events().count());
            // Bob owns token 1
            assert_eq!(erc721.balance_of(accounts.bob), 1);
        }

        #[ink::test]
        fn invalid_transfer_should_fail() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Transfer token fails if it does not exists.
            assert_eq!(erc721.transfer(accounts.bob, 2), Err(Error::TokenNotFound));
            // Token Id 2 does not exists.
            assert_eq!(erc721.owner_of(2), None);
            // Create token Id 2.
            assert_eq!(erc721.mint(2), Ok(()));
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Token Id 2 is owned by Alice.
            assert_eq!(erc721.owner_of(2), Some(accounts.alice));
            // Set Bob as caller
            set_caller(accounts.bob);
            // Bob cannot transfer not owned tokens.
            assert_eq!(erc721.transfer(accounts.eve, 2), Err(Error::NotApproved));
        }

        #[ink::test]
        fn approved_transfer_works() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1.
            assert_eq!(erc721.mint(1), Ok(()));
            // Token Id 1 is owned by Alice.
            assert_eq!(erc721.owner_of(1), Some(accounts.alice));
            // Approve token Id 1 transfer for Bob on behalf of Alice.
            assert_eq!(erc721.approve(accounts.bob, 1), Ok(()));
            // Set Bob as caller
            set_caller(accounts.bob);
            // Bob transfers token Id 1 from Alice to Eve.
            assert_eq!(
                erc721.transfer_from(accounts.alice, accounts.eve, 1),
                Ok(())
            );
            // TokenId 3 is owned by Eve.
            assert_eq!(erc721.owner_of(1), Some(accounts.eve));
            // Alice does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.alice), 0);
            // Bob does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.bob), 0);
            // Eve owns 1 token.
            assert_eq!(erc721.balance_of(accounts.eve), 1);
        }

        #[ink::test]
        fn approved_for_all_works() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1.
            assert_eq!(erc721.mint(1), Ok(()));
            // Create token Id 2.
            assert_eq!(erc721.mint(2), Ok(()));
            // Alice owns 2 tokens.
            assert_eq!(erc721.balance_of(accounts.alice), 2);
            // Approve token Id 1 transfer for Bob on behalf of Alice.
            assert_eq!(erc721.set_approval_for_all(accounts.bob, true), Ok(()));
            // Bob is an approved operator for Alice
            assert!(erc721.is_approved_for_all(accounts.alice, accounts.bob));
            // Set Bob as caller
            set_caller(accounts.bob);
            // Bob transfers token Id 1 from Alice to Eve.
            assert_eq!(
                erc721.transfer_from(accounts.alice, accounts.eve, 1),
                Ok(())
            );
            // TokenId 1 is owned by Eve.
            assert_eq!(erc721.owner_of(1), Some(accounts.eve));
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Bob transfers token Id 2 from Alice to Eve.
            assert_eq!(
                erc721.transfer_from(accounts.alice, accounts.eve, 2),
                Ok(())
            );
            // Bob does not own tokens.
            assert_eq!(erc721.balance_of(accounts.bob), 0);
            // Eve owns 2 tokens.
            assert_eq!(erc721.balance_of(accounts.eve), 2);
            // Remove operator approval for Bob on behalf of Alice.
            set_caller(accounts.alice);
            assert_eq!(erc721.set_approval_for_all(accounts.bob, false), Ok(()));
            // Bob is not an approved operator for Alice.
            assert!(!erc721.is_approved_for_all(accounts.alice, accounts.bob));
        }

        #[ink::test]
        fn not_approved_transfer_should_fail() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1.
            assert_eq!(erc721.mint(1), Ok(()));
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Bob does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.bob), 0);
            // Eve does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.eve), 0);
            // Set Eve as caller
            set_caller(accounts.eve);
            // Eve is not an approved operator by Alice.
            assert_eq!(
                erc721.transfer_from(accounts.alice, accounts.frank, 1),
                Err(Error::NotApproved)
            );
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Bob does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.bob), 0);
            // Eve does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.eve), 0);
        }

        #[ink::test]
        fn burn_works() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1 for Alice
            assert_eq!(erc721.mint(1), Ok(()));
            // Alice owns 1 token.
            assert_eq!(erc721.balance_of(accounts.alice), 1);
            // Alice owns token Id 1.
            assert_eq!(erc721.owner_of(1), Some(accounts.alice));
            // Destroy token Id 1.
            assert_eq!(erc721.burn(1), Ok(()));
            // Alice does not owns tokens.
            assert_eq!(erc721.balance_of(accounts.alice), 0);
            // Token Id 1 does not exists
            assert_eq!(erc721.owner_of(1), None);
        }

        #[ink::test]
        fn burn_fails_token_not_found() {
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Try burning a non existent token
            assert_eq!(erc721.burn(1), Err(Error::TokenNotFound));
        }

        #[ink::test]
        fn burn_fails_not_owner() {
            let accounts = ink_env::test::default_accounts::<ink_env::DefaultEnvironment>();
            // Create a new contract instance.
            let mut erc721 = init_contract();
            // Create token Id 1 for Alice
            assert_eq!(erc721.mint(1), Ok(()));
            // Try burning this token with a different account
            set_caller(accounts.eve);
            assert_eq!(erc721.burn(1), Err(Error::NotOwner));
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
