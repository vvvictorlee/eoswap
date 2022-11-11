// Mint
// The user must input a name, description and upload a file (image) to mint his own NFT. Once minted,
// a representation of this NFT will be displayed in the marketplace and initially it will be owned by its creator.
// This is open for everyone, meaning everyone can participate in this NFT creation within this collection.

#![cfg_attr(not(feature = "std"), no_std)]

pub use self::sub_art_tradable_private::{SubArtTradablePrivate, SubArtTradablePrivateRef};
use ink_env::AccountId;
use ink_lang as ink;
use ink_prelude::vec::Vec;
// This is the return value that we expect if a smart contract supports receiving ERC-1155
// tokens.
//
// It is calculated with
// `bytes4(keccak256("onERC1155Received(address,address,uint256,uint256,bytes)"))`, and corresponds
// to 0xf23a6e61.
#[cfg_attr(test, allow(dead_code))]
const ON_ERC_1155_RECEIVED_SELECTOR: [u8; 4] = [0xF2, 0x3A, 0x6E, 0x61];

// This is the return value that we expect if a smart contract supports batch receiving ERC-1155
// tokens.
//
// It is calculated with
// `bytes4(keccak256("onERC1155BatchReceived(address,address,uint256[],uint256[],bytes)"))`, and
// corresponds to 0xbc197c81.
const _ON_ERC_1155_BATCH_RECEIVED_SELECTOR: [u8; 4] = [0xBC, 0x19, 0x7C, 0x81];

/// A type representing the unique IDs of tokens managed by this contract.
pub type TokenId = u128;

type Balance = <ink_env::DefaultEnvironment as ink_env::Environment>::Balance;

// The ERC-1155 error types.
#[derive(Debug, PartialEq, scale::Encode, scale::Decode)]
#[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
pub enum Error {
    OnlyOwner,
    /// This token ID has not yet been created by the contract.
    UnexistentToken,
    /// The caller tried to sending tokens to the zero-address (`0x00`).
    ZeroAddressTransfer,
    /// The caller is not approved to transfer tokens on behalf of the account.
    NotApproved,
    /// The account does not have enough funds to complete the transfer.
    InsufficientBalance,
    /// An account does not need to approve themselves to transfer tokens.
    SelfApproval,
    /// The number of tokens being transferred does not match the specified number of transfers.
    BatchTransferMismatch,
    TransferFailed,
    InsufficientFunds,
    NewOwnerIsTheZeroAddress,
}

// The ERC-1155 result types.
pub type Result<T> = core::result::Result<T, Error>;

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

/// The interface for an ERC-1155 compliant contract.
///
/// The interface is defined here: <https://eips.ethereum.org/EIPS/eip-1155>.
///
/// The goal of ERC-1155 is to allow a single contract to manage a variety of assets.
/// These assets can be fungible, non-fungible, or a combination.
///
/// By tracking multiple assets the ERC-1155 standard is able to support batch transfers, which
/// make it easy to transfer a mix of multiple tokens at once.
#[ink::trait_definition]
pub trait Erc1155 {
    /// Transfer a `value` amount of `token_id` tokens to the `to` account from the `from`
    /// account.
    ///
    /// Note that the call does not have to originate from the `from` account, and may originate
    /// from any account which is approved to transfer `from`'s tokens.
    #[ink(message)]
    fn safe_transfer_from(
        &mut self,
        from: AccountId,
        to: AccountId,
        token_id: TokenId,
        value: Balance,
        data: Vec<u8>,
    ) -> Result<()>;

    /// Perform a batch transfer of `token_ids` to the `to` account from the `from` account.
    ///
    /// The number of `values` specified to be transferred must match the number of `token_ids`,
    /// otherwise this call will revert.
    ///
    /// Note that the call does not have to originate from the `from` account, and may originate
    /// from any account which is approved to transfer `from`'s tokens.
    #[ink(message)]
    fn safe_batch_transfer_from(
        &mut self,
        from: AccountId,
        to: AccountId,
        token_ids: Vec<TokenId>,
        values: Vec<Balance>,
        data: Vec<u8>,
    ) -> Result<()>;

    /// Query the balance of a specific token for the provided account.
    #[ink(message)]
    fn balance_of(&self, owner: AccountId, token_id: TokenId) -> Balance;

    /// Query the balances for a set of tokens for a set of accounts.
    ///
    /// E.g use this call if you want to query what Alice and Bob's balances are for Tokens ID 1 and
    /// ID 2.
    ///
    /// This will return all the balances for a given owner before moving on to the next owner. In
    /// the example above this means that the return value should look like:
    ///
    /// [Alice Balance of Token ID 1, Alice Balance of Token ID 2, Bob Balance of Token ID 1, Bob Balance of Token ID 2]
    #[ink(message)]
    fn balance_of_batch(&self, owners: Vec<AccountId>, token_ids: Vec<TokenId>) -> Vec<Balance>;

    /// Enable or disable a third party, known as an `operator`, to control all tokens on behalf of
    /// the caller.
    #[ink(message)]
    fn set_approval_for_all(&mut self, operator: AccountId, approved: bool) -> Result<()>;

    /// Query if the given `operator` is allowed to control all of `owner`'s tokens.
    #[ink(message)]
    fn is_approved_for_all(&self, owner: AccountId, operator: AccountId) -> bool;
}

/// The interface for an ERC-1155 Token Receiver contract.
///
/// The interface is defined here: <https://eips.ethereum.org/EIPS/eip-1155>.
///
/// Smart contracts which want to accept token transfers must implement this interface. By default
/// if a contract does not support this interface any transactions originating from an ERC-1155
/// compliant contract which attempt to transfer tokens directly to the contract's address must be
/// reverted.
#[ink::trait_definition]
pub trait Erc1155TokenReceiver {
    /// Handle the receipt of a single ERC-1155 token.
    ///
    /// This should be called by a compliant ERC-1155 contract if the intended recipient is a smart
    /// contract.
    ///
    /// If the smart contract implementing this interface accepts token transfers then it must
    /// return `ON_ERC_1155_RECEIVED_SELECTOR` from this function. To reject a transfer it must revert.
    ///
    /// Any callers must revert if they receive anything other than `ON_ERC_1155_RECEIVED_SELECTOR` as a return
    /// value.
    #[ink(message, selector = 0xF23A6E61)]
    fn on_received(
        &mut self,
        operator: AccountId,
        from: AccountId,
        token_id: TokenId,
        value: Balance,
        data: Vec<u8>,
    ) -> Vec<u8>;

    /// Handle the receipt of multiple ERC-1155 tokens.
    ///
    /// This should be called by a compliant ERC-1155 contract if the intended recipient is a smart
    /// contract.
    ///
    /// If the smart contract implementing this interface accepts token transfers then it must
    /// return `BATCH_ON_ERC_1155_RECEIVED_SELECTOR` from this function. To reject a transfer it must revert.
    ///
    /// Any callers must revert if they receive anything other than `BATCH_ON_ERC_1155_RECEIVED_SELECTOR` as a return
    /// value.
    #[ink(message, selector = 0xBC197C81)]
    fn on_batch_received(
        &mut self,
        operator: AccountId,
        from: AccountId,
        token_ids: Vec<TokenId>,
        values: Vec<Balance>,
        data: Vec<u8>,
    ) -> Vec<u8>;
}

//  @title SubArtTradable
//  SubArtTradable - ERC1155 contract that whitelists an operator address,
//  has mint functionality, and supports useful standards from OpenZeppelin,
//  like _exists(), name(), symbol(), and totalSupply()
#[ink::contract]
pub mod sub_art_tradable_private {
    #[cfg_attr(test, allow(dead_code))]
    const INTERFACE_ID_ERC1155: [u8; 4] = [0xD9, 0xB6, 0x7A, 0x26];

    use super::*;
    // use ink_lang as ink;
    use ink_prelude::string::String;
    use ink_prelude::vec::Vec;
    use ink_storage::{traits::SpreadAllocate, Mapping};

    type Owner = AccountId;
    type Operator = AccountId;

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

    #[ink(event)]
    pub struct OwnershipTransferred {
        #[ink(topic)]
        previous_owner: AccountId,
        #[ink(topic)]
        new_owner: AccountId,
    }

    /// Indicate that a token transfer has occured.
    ///
    /// This must be emitted even if a zero value transfer occurs.
    /// While mint (the follow action), parameter from is None, to is follower address, id is serialization of the address being followed.
    /// While burn (the unfollow action), parameter from is follower's address, to is None, id is serialization of the address being followed.
    /// eg: value always 1
    #[ink(event)]
    pub struct TransferSingle {
        /// operator: operator of transaction
        #[ink(topic)]
        operator: Option<AccountId>,
        /// from: Profile NFT old owner
        #[ink(topic)]
        from: Option<AccountId>,
        /// to: Profile NFT new owner
        #[ink(topic)]
        to: Option<AccountId>,
        /// token_id: Profile NFT ID
        token_id: TokenId,
        /// value: Profile NFT value
        value: Balance,
    }

    /// Indicate that a token transfer has occured.
    ///
    /// This must be emitted even if a zero value transfer occurs.
    /// ids is a set of serializations of addresses being followed.
    #[ink(event)]
    pub struct TransferBatch {
        /// operator: operator of transaction
        #[ink(topic)]
        operator: Option<AccountId>,
        /// from: Profile NFT old owner
        #[ink(topic)]
        from: Option<AccountId>,
        /// to: Profile NFT new owner
        #[ink(topic)]
        to: Option<AccountId>,
        /// token_ids: Profile NFT ID's list
        token_ids: Vec<TokenId>,
        /// values: Profile NFT ID value's list
        values: Vec<Balance>,
    }

    /// Indicate that an approval event has happened.
    #[ink(event)]
    pub struct ApprovalForAll {
        #[ink(topic)]
        owner: AccountId,
        #[ink(topic)]
        operator: AccountId,
        approved: bool,
    }

    /// Indicate that a token's URI has been updated.
    #[ink(event)]
    pub struct Uri {
        pub value: ink_prelude::string::String,
        #[ink(topic)]
        pub token_id: TokenId,
    }

    /// Indicate that a mint event has happened.
    #[ink(event)]
    pub struct Mint {
        /// account: Profile NFT new owner
        #[ink(topic)]
        pub account: AccountId,
        /// owner: operator of transaction
        #[ink(topic)]
        pub owner: AccountId,
        /// token_id: Profile NFT ID
        #[ink(topic)]
        pub token_id: u128,
    }

    /// Indicate that a mintbatch event has happened.
    #[ink(event)]
    pub struct MintBatch {
        /// accounts: Profile NFT new owners
        #[ink(topic)]
        pub accounts: Vec<AccountId>,
        /// owner: operator of transaction
        #[ink(topic)]
        pub owner: AccountId,
        /// token_ids: Profile NFT ID's list
        #[ink(topic)]
        pub token_ids: Vec<u128>,
    }

    /// Indicate that a mintbatch event has happened.
    #[ink(event)]
    pub struct Burn {
        /// account: Profile NFT new owner
        #[ink(topic)]
        pub account: AccountId,
        /// owner: operator of transaction
        #[ink(topic)]
        pub owner: AccountId,
        /// token_id: Profile NFT ID
        #[ink(topic)]
        pub token_id: TokenId,
    }

    /// Indicate that a mintbatch event has happened.
    #[ink(event)]
    pub struct BurnBatch {
        /// accounts: Profile NFT owners
        #[ink(topic)]
        pub accounts: Vec<AccountId>,
        /// owner: operator of transaction
        #[ink(topic)]
        pub owner: AccountId,
        /// token_ids: Profile NFT ID's list
        #[ink(topic)]
        pub token_ids: Vec<TokenId>,
    }

    /// An ERC-1155 contract.
    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubArtTradablePrivate {
        /// Tracks the balances of accounts across the different tokens that they might be holding.
        balances: Mapping<(AccountId, TokenId), Balance>,
        /// Which accounts (called operators) have been approved to spend funds on behalf of an owner.
        approvals: Mapping<(Owner, Operator), ()>,
        /// A unique identifier for the tokens which have been minted (and are therefore supported)
        /// by this contract.
        token_id_nonce: TokenId,
        /// Mapping from token id to  the uri
        token_uris: Mapping<TokenId, String>,
        /// Mapping from token id to  the creator
        creators: Mapping<TokenId, AccountId>,
        /// the token supply of the token
        token_supply: Mapping<TokenId, Balance>,
        /// the name of the token
        name: String,
        /// the symbol of the token
        symbol: String,
        ///  Marketplace contract
        marketplace: AccountId,
        ///  BundleMarketplace contract
        bundle_marketplace: AccountId,
        ///  Platform fee
        platform_fee: Balance,
        ///  Platform fee receipient
        fee_recipient: AccountId,
        ///the contract owner
        owner: AccountId,
    }

    impl SubArtTradablePrivate {
        /// Initialize a default instance of this ERC-1155 implementation.
        #[ink(constructor)]
        pub fn new(
            name: String,
            symbol: String,
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
                contract.marketplace = marketplace;
                contract.bundle_marketplace = bundle_marketplace;
                contract.fee_recipient = fee_recipient;
                contract.platform_fee = platform_fee;
            })
        }
        /// Returns whether the specified interface supports
        /// # Fields
        /// interface_id interface id hash prefix 4 bytes
        #[ink(message)]
        pub fn supports_interface(&self, interface_id: [u8; 4]) -> bool {
            INTERFACE_ID_ERC1155 == interface_id
        }
        /// Returns the uri for a token ID
        /// # Fields
        /// id the ID of the token to query
        /// #return uri of token in existence
        #[ink(message)]
        pub fn uri(&self, id: TokenId) -> Result<String> {
            ensure!(self.exists(id), Error::UnexistentToken);
            Ok(self.token_uris.get(&id).unwrap_or(String::new()))
        }

        /// Returns the total quantity for a token ID
        /// # Fields
        /// id the ID of the token to query
        /// #return amount of token in existence
        #[ink(message)]
        pub fn token_supply(&self, id: TokenId) -> Balance {
            self.token_supply.get(&id).unwrap_or(Balance::default())
        }

        /// Method for minting  token with specified  owner , suplly and uri.
        /// # Fields
        /// to  the owner of the token
        /// supply  the value of the token
        /// uri  the uri of the token
        ///
        /// # Errors
        ///
        /// - If the transferred value is less than the `platform_fee`.
        /// - If it failed when the contract trasfer to  fee_recipient in native token.
        /// - If the `to` is zero address.
        /// - If the caller is not the owner of the contract.
        #[ink(message, payable)]
        pub fn mint_art(&mut self, to: AccountId, supply: Balance, uri: String) -> Result<()> {
            ensure!(self.env().caller() == self.owner, Error::OnlyOwner);

            ensure!(
                self.env().transferred_value() >= self.platform_fee,
                Error::InsufficientFunds
            );
            let id = self.get_next_token_id();
            self.increment_token_type_id();
            self.creators.insert(&id, &self.env().caller());
            self.set_token_uri(id, &uri)?;
            if !uri.is_empty() {
                self.env().emit_event(Uri {
                    value: uri,
                    token_id: id,
                });
            }
            self._mint_to(to, id, supply, Vec::new())?;
            self.token_supply.insert(&id, &supply);

            ensure!(
                self.env()
                    .transfer(self.fee_recipient, self.env().transferred_value())
                    .is_ok(),
                Error::TransferFailed
            );
            Ok(())
        }

        ///  is_approved_for_all_art to whitelist Sub contracts to enable gas-less listings.
        /// # Fields
        /// owner address of the owner of the token
        /// operator address of the operator of the token
        #[ink(message)]
        pub fn is_approved_for_all_art(&self, owner: AccountId, operator: AccountId) -> bool {
            if self.marketplace == operator || self.bundle_marketplace == operator {
                return true;
            }
            self.is_approved_for_all(owner, operator)
        }

        /// Returns whether the specified token exists by checking to see if it has a creator
        /// # Fields
        ///  id ID of the token to query the existence of
        /// #return bool whether the token exists
        #[ink(message)]
        pub fn exists(&self, id: TokenId) -> bool {
            if let Some(addr) = self.creators.get(&id) {
                addr != AccountId::from([0x0; 32])
            } else {
                false
            }
        }

        #[ink(message)]
        pub fn get_current_token_id(&self) -> TokenId {
            self.token_id_nonce
        }
        /// calculates the next token ID based on value of token_id_nonce
        /// #return for the next token ID
        fn get_next_token_id(&self) -> TokenId {
            self.token_id_nonce + 1
        }

        /// increments the value of token_id_nonce
        fn increment_token_type_id(&mut self) {
            self.token_id_nonce += 1;
        }

        /// Internal function to set the token URI for a given token.
        /// Reverts if the token ID does not exist.
        ///  id ID of the token to set its URI
        /// uri string URI to assign
        fn set_token_uri(&mut self, id: TokenId, uri: &String) -> Result<()> {
            ensure!(self.exists(id), Error::UnexistentToken);
            self.token_uris.insert(&id, uri);
            Ok(())
        }
        //================================ERC1155=========================
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
        #[ink(message)]
        pub fn _mint_to(
            &mut self,
            to: AccountId,
            token_id: TokenId,
            value: Balance,
            _data: Vec<u8>,
        ) -> Result<()> {
            let caller = self.env().caller();
            ensure!(to != AccountId::default(), Error::ZeroAddressTransfer);
            self.balances.insert(&(to, token_id), &value);

            // Emit transfer event but with mint semantics
            self.env().emit_event(TransferSingle {
                operator: Some(caller),
                from: None,
                to: Some(to),
                token_id,
                value,
            });

            Ok(())
        }

        /// Create the initial supply for a token.
        ///
        /// The initial supply will be provided to the caller (a.k.a the minter), and the
        /// `token_id` will be assigned by the smart contract.
        ///
        /// Note that as implemented anyone can create tokens. If you were to instantiate
        /// this contract in a production environment you'd probably want to lock down
        /// the addresses that are allowed to create tokens.
        #[ink(message)]
        pub fn create(&mut self, value: Balance) -> TokenId {
            let caller = self.env().caller();

            // Given that TokenId is a `u128` the likelihood of this overflowing is pretty slim.
            self.token_id_nonce += 1;
            self.balances.insert(&(caller, self.token_id_nonce), &value);

            // Emit transfer event but with mint semantics
            self.env().emit_event(TransferSingle {
                operator: Some(caller),
                from: None,
                to: if value == 0 { None } else { Some(caller) },
                token_id: self.token_id_nonce,
                value,
            });

            self.token_id_nonce
        }

        /// Mint a `value` amount of `token_id` tokens.
        ///
        /// It is assumed that the token has already been `create`-ed. The newly minted supply will
        /// be assigned to the caller (a.k.a the minter).
        ///
        /// Note that as implemented anyone can mint tokens. If you were to instantiate
        /// this contract in a production environment you'd probably want to lock down
        /// the addresses that are allowed to mint tokens.
        #[ink(message)]
        pub fn mint(&mut self, token_id: TokenId, value: Balance) -> Result<()> {
            ensure!(token_id <= self.token_id_nonce, Error::UnexistentToken);

            let caller = self.env().caller();
            self.balances.insert(&(caller, token_id), &value);

            // Emit transfer event but with mint semantics
            self.env().emit_event(TransferSingle {
                operator: Some(caller),
                from: None,
                to: Some(caller),
                token_id,
                value,
            });

            Ok(())
        }

        /// Mint a `value` amount of `token_id` tokens to the given account.
        ///
        /// The newly minted supply will
        /// be assigned to the given account.
        ///
        /// Note that as implemented anyone can mint tokens. If you were to instantiate
        /// this contract in a production environment you'd probably want to lock down
        /// the addresses that are allowed to mint tokens.
        #[ink(message)]
        pub fn mint_to(&mut self, to: AccountId, token_id: TokenId, value: Balance) -> Result<()> {
            let caller = self.env().caller();
            ensure!(to != AccountId::default(), Error::ZeroAddressTransfer);
            self.balances.insert(&(to, token_id), &value);

            // Emit transfer event but with mint semantics
            self.env().emit_event(TransferSingle {
                operator: Some(caller),
                from: None,
                to: Some(to),
                token_id,
                value,
            });

            Ok(())
        }

        /// Batch mint these `values` amount of `token_ids` tokens to the given account.
        ///
        /// The newly minted supply will
        /// be assigned to the given account.
        ///
        /// Note that as implemented anyone can mint tokens. If you were to instantiate
        /// this contract in a production environment you'd probably want to lock down
        /// the addresses that are allowed to mint tokens.
        #[ink(message)]
        pub fn mint_to_batch(
            &mut self,
            to: AccountId,
            token_ids: Vec<TokenId>,
            values: Vec<Balance>,
        ) -> Result<()> {
            let caller = self.env().caller();
            ensure!(to != AccountId::default(), Error::ZeroAddressTransfer);

            let transfers = token_ids.iter().zip(values.iter());
            for (&token_id, &value) in transfers {
                self.balances.insert(&(to, token_id), &value);
            }
            // Emit transfer event but with mint semantics
            self.env().emit_event(TransferBatch {
                operator: Some(caller),
                from: None,
                to: Some(to),
                token_ids,
                values,
            });

            Ok(())
        }
        /// Deletes an existing token.
        /// Deletes `value` tokens on the behalf of `from`.
        ///
        /// This can be used to allow a contract to delete tokens on ones behalf and/or
        /// to charge fees in sub-currencies.
        #[ink(message)]
        pub fn burn(&mut self, from: AccountId, token_id: TokenId, value: Balance) -> Result<()> {
            ensure!(from != AccountId::default(), Error::ZeroAddressTransfer);

            let caller = self.env().caller();
            if caller != from {
                ensure!(self.is_approved_for_all(from, caller), Error::NotApproved);
            }
            let mut sender_balance = self
                .balances
                .get(&(from, token_id))
                .expect("Caller should have ensured that `from` holds `token_id`.");
            sender_balance -= value;
            self.balances.insert(&(from, token_id), &sender_balance);

            let caller = self.env().caller();
            self.env().emit_event(TransferSingle {
                operator: Some(caller),
                from: Some(from),
                to: None,
                token_id,
                value,
            });
            Ok(())
        }

        /// Deletes the existing tokens.
        /// Deletes `values` tokens on the behalf of `from`.
        ///
        /// This can be used to allow a contract to delete tokens on ones behalf and/or
        /// to charge fees in sub-currencies.
        #[ink(message)]
        pub fn burn_batch(
            &mut self,
            from: AccountId,
            token_ids: Vec<TokenId>,
            values: Vec<Balance>,
        ) -> Result<()> {
            ensure!(from != AccountId::default(), Error::ZeroAddressTransfer);

            let caller = self.env().caller();
            if caller != from {
                ensure!(self.is_approved_for_all(from, caller), Error::NotApproved);
            }

            let transfers = token_ids.iter().zip(values.iter());
            for (&token_id, &value) in transfers {
                let mut sender_balance = self
                    .balances
                    .get(&(from, token_id))
                    .expect("Caller should have ensured that `from` holds `token_id`.");
                sender_balance -= value;
                self.balances.insert(&(from, token_id), &sender_balance);
            }

            let caller = self.env().caller();
            self.env().emit_event(TransferBatch {
                operator: Some(caller),
                from: Some(from),
                to: None,
                token_ids,
                values,
            });
            Ok(())
        }

        // Helper function for performing single token transfers.
        //
        // Should not be used directly since it's missing certain checks which are important to the
        // ERC-1155 standard (it is expected that the caller has already performed these).
        //
        // # Panics
        //
        // If `from` does not hold any `token_id` tokens.
        fn perform_transfer(
            &mut self,
            from: AccountId,
            to: AccountId,
            token_id: TokenId,
            value: Balance,
        ) {
            let mut sender_balance = self
                .balances
                .get(&(from, token_id))
                .expect("Caller should have ensured that `from` holds `token_id`.");
            sender_balance -= value;
            self.balances.insert(&(from, token_id), &sender_balance);

            let mut recipient_balance = self.balances.get(&(to, token_id)).unwrap_or(0);
            recipient_balance += value;
            self.balances.insert(&(to, token_id), &recipient_balance);

            let caller = self.env().caller();
            self.env().emit_event(TransferSingle {
                operator: Some(caller),
                from: Some(from),
                to: Some(from),
                token_id,
                value,
            });
        }

        // Check if the address at `to` is a smart contract which accepts ERC-1155 token transfers.
        //
        // If they're a smart contract which **doesn't** accept tokens transfers this call will
        // revert. Otherwise we risk locking user funds at in that contract with no chance of
        // recovery.
        #[cfg_attr(test, allow(unused_variables))]
        fn transfer_acceptance_check(
            &mut self,
            caller: AccountId,
            from: AccountId,
            to: AccountId,
            token_id: TokenId,
            value: Balance,
            data: Vec<u8>,
        ) {
            // This is disabled during tests due to the use of `invoke_contract()` not being
            // supported (tests end up panicking).
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput, Selector};

                // If our recipient is a smart contract we need to see if they accept or
                // reject this transfer. If they reject it we need to revert the call.
                let params = build_call::<Environment>()
                    .call_type(Call::new().callee(to).gas_limit(5000))
                    .exec_input(
                        ExecutionInput::new(Selector::new(ON_ERC_1155_RECEIVED_SELECTOR))
                            .push_arg(caller)
                            .push_arg(from)
                            .push_arg(token_id)
                            .push_arg(value)
                            .push_arg(data),
                    )
                    .returns::<Vec<u8>>()
                    .params();

                match ink_env::invoke_contract(&params) {
                    Ok(v) => {
                        ink_env::debug_println!(
                            "Received return value \"{:?}\" from contract {:?}",
                            v,
                            from
                        );
                        assert_eq!(
                            v,
                            &ON_ERC_1155_RECEIVED_SELECTOR[..],
                            "The recipient contract at {:?} does not accept token transfers.\n
                            Expected: {:?}, Got {:?}",
                            to,
                            ON_ERC_1155_RECEIVED_SELECTOR,
                            v
                        )
                    }
                    Err(e) => {
                        match e {
                            ink_env::Error::CodeNotFound | ink_env::Error::NotCallable => {
                                // Our recipient wasn't a smart contract, so there's nothing more for
                                // us to do
                                ink_env::debug_println!(
                                    "Recipient at {:?} from is not a smart contract ({:?})",
                                    from,
                                    e
                                );
                            }
                            _ => {
                                // We got some sort of error from the call to our recipient smart
                                // contract, and as such we must revert this call
                                panic!("Got error \"{:?}\" while trying to call {:?}", e, from)
                            }
                        }
                    }
                }
            }
        }
    }

    impl super::Erc1155 for SubArtTradablePrivate {
        #[ink(message)]
        fn safe_transfer_from(
            &mut self,
            from: AccountId,
            to: AccountId,
            token_id: TokenId,
            value: Balance,
            data: Vec<u8>,
        ) -> Result<()> {
            let caller = self.env().caller();
            if caller != from {
                ensure!(self.is_approved_for_all(from, caller), Error::NotApproved);
            }

            ensure!(to != AccountId::default(), Error::ZeroAddressTransfer);

            let balance = self.balance_of(from, token_id);
            ensure!(balance >= value, Error::InsufficientBalance);

            self.perform_transfer(from, to, token_id, value);
            self.transfer_acceptance_check(caller, from, to, token_id, value, data);

            Ok(())
        }

        #[ink(message)]
        fn safe_batch_transfer_from(
            &mut self,
            from: AccountId,
            to: AccountId,
            token_ids: Vec<TokenId>,
            values: Vec<Balance>,
            data: Vec<u8>,
        ) -> Result<()> {
            let caller = self.env().caller();
            if caller != from {
                ensure!(self.is_approved_for_all(from, caller), Error::NotApproved);
            }

            ensure!(to != AccountId::default(), Error::ZeroAddressTransfer);
            ensure!(!token_ids.is_empty(), Error::BatchTransferMismatch);
            ensure!(
                token_ids.len() == values.len(),
                Error::BatchTransferMismatch,
            );

            let transfers = token_ids.iter().zip(values.iter());
            for (&id, &v) in transfers.clone() {
                let balance = self.balance_of(from, id);
                ensure!(balance >= v, Error::InsufficientBalance);
            }

            for (&id, &v) in transfers {
                self.perform_transfer(from, to, id, v);
            }

            // Can use the any token ID/value here, we really just care about knowing if `to` is a
            // smart contract which accepts transfers
            self.transfer_acceptance_check(caller, from, to, token_ids[0], values[0], data);

            Ok(())
        }

        #[ink(message)]
        fn balance_of(&self, owner: AccountId, token_id: TokenId) -> Balance {
            self.balances.get(&(owner, token_id)).unwrap_or(0)
        }

        #[ink(message)]
        fn balance_of_batch(
            &self,
            owners: Vec<AccountId>,
            token_ids: Vec<TokenId>,
        ) -> Vec<Balance> {
            let mut output = Vec::new();
            for o in &owners {
                for t in &token_ids {
                    let amount = self.balance_of(*o, *t);
                    output.push(amount);
                }
            }
            output
        }

        #[ink(message)]
        fn set_approval_for_all(&mut self, operator: AccountId, approved: bool) -> Result<()> {
            let caller = self.env().caller();
            ensure!(operator != caller, Error::SelfApproval);

            if approved {
                self.approvals.insert((&caller, &operator), &());
            } else {
                self.approvals.remove((&caller, &operator));
            }

            self.env().emit_event(ApprovalForAll {
                owner: caller,
                operator,
                approved,
            });

            Ok(())
        }

        #[ink(message)]
        fn is_approved_for_all(&self, owner: AccountId, operator: AccountId) -> bool {
            self.approvals.get((&owner, &operator)).is_some()
        }
    }

    impl super::Erc1155TokenReceiver for SubArtTradablePrivate {
        #[ink(message, selector = 0xF23A6E61)]
        fn on_received(
            &mut self,
            _operator: AccountId,
            _from: AccountId,
            _token_id: TokenId,
            _value: Balance,
            _data: Vec<u8>,
        ) -> Vec<u8> {
            // The ERC-1155 standard dictates that if a contract does not accept token transfers
            // directly to the contract, then the contract must revert.
            //
            // This prevents a user from unintentionally transferring tokens to a smart contract
            // and getting their funds stuck without any sort of recovery mechanism.
            //
            // Note that the choice of whether or not to accept tokens is implementation specific,
            // and we've decided to not accept them in this implementation.
            unimplemented!("This smart contract does not accept token transfer.")
        }

        #[ink(message, selector = 0xBC197C81)]
        fn on_batch_received(
            &mut self,
            _operator: AccountId,
            _from: AccountId,
            _token_ids: Vec<TokenId>,
            _values: Vec<Balance>,
            _data: Vec<u8>,
        ) -> Vec<u8> {
            // The ERC-1155 standard dictates that if a contract does not accept token transfers
            // directly to the contract, then the contract must revert.
            //
            // This prevents a user from unintentionally transferring tokens to a smart contract
            // and getting their funds stuck without any sort of recovery mechanism.
            //
            // Note that the choice of whether or not to accept tokens is implementation specific,
            // and we've decided to not accept them in this implementation.
            unimplemented!("This smart contract does not accept batch token transfers.")
        }
    }

    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use crate::Erc1155;

        use ink_lang as ink;

        fn set_caller(sender: AccountId) {
            ink_env::test::set_caller::<Environment>(sender);
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
        fn init_contract() -> SubArtTradablePrivate {
            let mut erc = SubArtTradablePrivate::new(
                String::from("test"),
                String::from("TEST"),
                frank(),
                eve(),
                10,
                fee_recipient(),
            );
            erc.balances.insert((alice(), 1), &10);
            erc.balances.insert((alice(), 2), &20);
            erc.balances.insert((bob(), 1), &10);

            erc
        }

        #[ink::test]
        fn mint_art_works() {
            // Create a new contract instance.
            let mut art_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = charlie();
            let token_uri = String::from("token_uri:bob");
            let supply = 10;
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

            assert!(art_tradable.mint_art(to, supply, token_uri.clone()).is_ok());
            assert_eq!(get_balance(fee_recipient()), 10);
            let token_id = art_tradable.get_current_token_id();
            assert_eq!(art_tradable.token_supply.get(&token_id), Some(supply));
            assert_eq!(art_tradable.creators.get(&token_id), Some(caller));
            assert_eq!(
                art_tradable.token_uris.get(&token_id),
                Some(token_uri.clone())
            );
            assert_eq!(art_tradable.balances.get(&(to, token_id)), Some(supply));
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 2);
            assert_uri_event(&emitted_events[0], token_uri, token_id);
            assert_transfer_single_event(
                &emitted_events[1],
                Some(caller),
                None,
                Some(to),
                token_id,
                supply,
            );
        }

        #[ink::test]
        fn mint_art_failed_if_the_caller_is_not_the_owner_of_the_contract() {
            // Create a new contract instance.
            let mut art_tradable = init_contract();
            let caller = bob();
            set_caller(caller);
            let to = charlie();
            let token_uri = String::from("token_uri:bob");
            let supply = 10;
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

            assert_eq!(
                art_tradable
                    .mint_art(to, supply, token_uri.clone())
                    .unwrap_err(),
                Error::OnlyOwner
            );
        }

        #[ink::test]
        fn mint_art_failed_if_the_transferred_value_is_less_than_the_platform_fee() {
            // Create a new contract instance.
            let mut art_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = charlie();
            let token_uri = String::from("token_uri:bob");
            let supply = 10;
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(1);

            assert_eq!(
                art_tradable
                    .mint_art(to, supply, token_uri.clone())
                    .unwrap_err(),
                Error::InsufficientFunds
            );
        }

        #[ink::test]
        fn mint_art_failed_if_the_to_is_zero_address() {
            // Create a new contract instance.
            let mut art_tradable = init_contract();
            let caller = alice();
            set_caller(caller);
            let to = AccountId::from([0x0; 32]);
            let token_uri = String::from("token_uri:bob");
            let supply = 10;
            set_balance(caller, 10);
            set_balance(fee_recipient(), 0);
            ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

            assert_eq!(
                art_tradable
                    .mint_art(to, supply, token_uri.clone())
                    .unwrap_err(),
                Error::ZeroAddressTransfer
            );
        }

        fn assert_uri_event(
            event: &ink_env::test::EmittedEvent,
            expected_value: ink_prelude::string::String,
            expected_token_id: TokenId,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::Uri(Uri { value, token_id }) = decoded_event {
                assert_eq!(value, expected_value, "encountered invalid Uri.value");
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid Uri.token_id"
                );
            } else {
                panic!("encountered unexpected event kind: expected a Uri event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubArtTradablePrivate::Uri",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::Uri::token_id",
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

        //==========================ERC1155===========================
        #[ink::test]
        fn can_get_correct_balance_of() {
            let erc = init_contract();

            assert_eq!(erc.balance_of(alice(), 1), 10);
            assert_eq!(erc.balance_of(alice(), 2), 20);
            assert_eq!(erc.balance_of(alice(), 3), 0);
            assert_eq!(erc.balance_of(bob(), 2), 0);
        }

        #[ink::test]
        fn can_get_correct_batch_balance_of() {
            let erc = init_contract();

            assert_eq!(
                erc.balance_of_batch(vec![alice()], vec![1, 2, 3]),
                vec![10, 20, 0]
            );
            assert_eq!(
                erc.balance_of_batch(vec![alice(), bob()], vec![1]),
                vec![10, 10]
            );

            assert_eq!(
                erc.balance_of_batch(vec![alice(), bob(), charlie()], vec![1, 2]),
                vec![10, 20, 10, 0, 0, 0]
            );
        }

        #[ink::test]
        fn can_send_tokens_between_accounts() {
            let mut erc = init_contract();

            assert!(erc.safe_transfer_from(alice(), bob(), 1, 5, vec![]).is_ok());
            assert_eq!(erc.balance_of(alice(), 1), 5);
            assert_eq!(erc.balance_of(bob(), 1), 15);

            assert!(erc.safe_transfer_from(alice(), bob(), 2, 5, vec![]).is_ok());
            assert_eq!(erc.balance_of(alice(), 2), 15);
            assert_eq!(erc.balance_of(bob(), 2), 5);
        }

        #[ink::test]
        fn sending_too_many_tokens_fails() {
            let mut erc = init_contract();
            let res = erc.safe_transfer_from(alice(), bob(), 1, 99, vec![]);
            assert_eq!(res.unwrap_err(), Error::InsufficientBalance);
        }

        #[ink::test]
        fn sending_tokens_to_zero_address_fails() {
            let burn: AccountId = [0; 32].into();

            let mut erc = init_contract();
            let res = erc.safe_transfer_from(alice(), burn, 1, 10, vec![]);
            assert_eq!(res.unwrap_err(), Error::ZeroAddressTransfer);
        }

        #[ink::test]
        fn can_send_batch_tokens() {
            let mut erc = init_contract();
            assert!(erc
                .safe_batch_transfer_from(alice(), bob(), vec![1, 2], vec![5, 10], vec![])
                .is_ok());

            let balances = erc.balance_of_batch(vec![alice(), bob()], vec![1, 2]);
            assert_eq!(balances, vec![5, 10, 15, 10])
        }

        #[ink::test]
        fn rejects_batch_if_lengths_dont_match() {
            let mut erc = init_contract();
            let res = erc.safe_batch_transfer_from(alice(), bob(), vec![1, 2, 3], vec![5], vec![]);
            assert_eq!(res.unwrap_err(), Error::BatchTransferMismatch);
        }

        #[ink::test]
        fn batch_transfers_fail_if_len_is_zero() {
            let mut erc = init_contract();
            let res = erc.safe_batch_transfer_from(alice(), bob(), vec![], vec![], vec![]);
            assert_eq!(res.unwrap_err(), Error::BatchTransferMismatch);
        }

        #[ink::test]
        fn operator_can_send_tokens() {
            let mut erc = init_contract();

            let owner = alice();
            let operator = bob();

            set_caller(owner);
            assert!(erc.set_approval_for_all(operator, true).is_ok());

            set_caller(operator);
            assert!(erc
                .safe_transfer_from(owner, charlie(), 1, 5, vec![])
                .is_ok());
            assert_eq!(erc.balance_of(alice(), 1), 5);
            assert_eq!(erc.balance_of(charlie(), 1), 5);
        }

        #[ink::test]
        fn approvals_work() {
            let mut erc = init_contract();
            let owner = alice();
            let operator = bob();
            let another_operator = charlie();

            // Note: All of these tests are from the context of the owner who is either allowing or
            // disallowing an operator to control their funds.
            set_caller(owner);
            assert!(!erc.is_approved_for_all(owner, operator));

            assert!(erc.set_approval_for_all(operator, true).is_ok());
            assert!(erc.is_approved_for_all(owner, operator));

            assert!(erc.set_approval_for_all(another_operator, true).is_ok());
            assert!(erc.is_approved_for_all(owner, another_operator));

            assert!(erc.set_approval_for_all(operator, false).is_ok());
            assert!(!erc.is_approved_for_all(owner, operator));
        }

        #[ink::test]
        fn minting_tokens_works() {
            let mut erc = init_contract();

            set_caller(alice());
            assert_eq!(erc.create(0), 1);
            assert_eq!(erc.balance_of(alice(), 1), 0);

            assert!(erc.mint(1, 123).is_ok());
            assert_eq!(erc.balance_of(alice(), 1), 123);
        }

        #[ink::test]
        fn minting_not_allowed_for_nonexistent_tokens() {
            let mut erc = init_contract();

            let res = erc.mint(1, 123);
            assert_eq!(res.unwrap_err(), Error::UnexistentToken);
        }
        use ink_env::Clear;

        type Event = <SubArtTradablePrivate as ::ink_lang::reflect::ContractEventBase>::Type;

        fn assert_transfer_single_event(
            event: &ink_env::test::EmittedEvent,
            expected_operator: Option<AccountId>,
            expected_from: Option<AccountId>,
            expected_to: Option<AccountId>,
            expected_token_id: TokenId,
            expected_value: Balance,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::TransferSingle(TransferSingle {
                operator,
                from,
                to,
                token_id,
                value,
            }) = decoded_event
            {
                assert_eq!(
                    operator, expected_operator,
                    "encountered invalid TransferSingle.operator"
                );
                assert_eq!(
                    from, expected_from,
                    "encountered invalid TransferSingle.from"
                );
                assert_eq!(to, expected_to, "encountered invalid TransferSingle.to");
                assert_eq!(
                    token_id, expected_token_id,
                    "encountered invalid TransferSingle.token_id"
                );
                assert_eq!(
                    value, expected_value,
                    "encountered invalid TransferSingle.value"
                );
            } else {
                panic!("encountered unexpected event kind: expected a TransferSingle event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubArtTradablePrivate::TransferSingle",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::TransferSingle::operator",
                    value: &expected_operator,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::TransferSingle::from",
                    value: &expected_from,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::TransferSingle::to",
                    value: &expected_to,
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

        fn assert_transfer_batch_event(
            event: &ink_env::test::EmittedEvent,
            expected_operator: Option<AccountId>,
            expected_from: Option<AccountId>,
            expected_to: Option<AccountId>,
            expected_token_ids: Vec<TokenId>,
            expected_values: Vec<Balance>,
        ) {
            let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
                .expect("encountered invalid contract event data buffer");
            if let Event::TransferBatch(TransferBatch {
                operator,
                from,
                to,
                token_ids,
                values,
            }) = decoded_event
            {
                assert_eq!(
                    operator, expected_operator,
                    "encountered invalid TransferBatch.operator"
                );
                assert_eq!(
                    from, expected_from,
                    "encountered invalid TransferBatch.from"
                );
                assert_eq!(to, expected_to, "encountered invalid TransferBatch.to");
                assert_eq!(
                    token_ids, expected_token_ids,
                    "encountered invalid TransferBatch.token_ids"
                );
                assert_eq!(
                    values, expected_values,
                    "encountered invalid TransferBatch.values"
                );
            } else {
                panic!("encountered unexpected event kind: expected a TransferBatch event")
            }
            let expected_topics = vec![
                encoded_into_hash(&PrefixedValue {
                    value: b"SubArtTradablePrivate::TransferBatch",
                    prefix: b"",
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::TransferBatch::operator",
                    value: &expected_operator,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::TransferBatch::from",
                    value: &expected_from,
                }),
                encoded_into_hash(&PrefixedValue {
                    prefix: b"SubArtTradablePrivate::TransferBatch::to",
                    value: &expected_to,
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

        #[ink::test]
        fn minting_to_tokens_works() {
            let mut erc = init_contract();

            set_caller(alice());

            assert!(erc.mint_to(bob(), 1, 123).is_ok());
            assert_eq!(erc.balance_of(bob(), 1), 123);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_transfer_single_event(
                &emitted_events[0],
                Some(alice()),
                None,
                Some(bob()),
                1,
                123,
            );
        }
        #[ink::test]
        fn can_mint_batch_tokens() {
            let mut erc = init_contract();
            assert!(erc.mint_to_batch(bob(), vec![1, 2], vec![5, 10]).is_ok());

            let balances = erc.balance_of_batch(vec![bob()], vec![1, 2]);
            assert_eq!(balances, vec![5, 10]);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_transfer_batch_event(
                &emitted_events[0],
                Some(alice()),
                None,
                Some(bob()),
                vec![1, 2],
                vec![5, 10],
            );
        }

        #[ink::test]
        fn burning_to_tokens_works() {
            let mut erc = init_contract();

            set_caller(alice());
            erc.balances.insert((bob(), 1), &123);
            erc.approvals.insert((&bob(), &alice()), &());
            assert!(erc.burn(bob(), 1, 123).is_ok());
            assert_eq!(erc.balance_of(bob(), 1), 0);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_transfer_single_event(
                &emitted_events[0],
                Some(alice()),
                Some(bob()),
                None,
                1,
                123,
            );
        }
        #[ink::test]
        fn can_burn_batch_tokens() {
            let mut erc = init_contract();
            erc.balances.insert((bob(), 1), &5);
            erc.balances.insert((bob(), 2), &10);
            set_caller(alice());
            erc.approvals.insert((&bob(), &alice()), &());
            assert!(erc.burn_batch(bob(), vec![1, 2], vec![5, 10]).is_ok());
            let balances = erc.balance_of_batch(vec![bob()], vec![1, 2]);
            assert_eq!(balances, vec![0, 0]);
            let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
            assert_eq!(emitted_events.len(), 1);
            assert_transfer_batch_event(
                &emitted_events[0],
                Some(alice()),
                Some(bob()),
                None,
                vec![1, 2],
                vec![5, 10],
            );
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
