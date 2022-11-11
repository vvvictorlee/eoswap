//! # NFT Factory Private
//!
//! This is a NFT Factory implementation.
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
//! Any fn that modifies the state returns a `Result` type and does not changes the state
//! if the `Error` occurs.
//! The errors are defined as an `enum` type. Any other error or invariant violation
//! triggers a panic and therefore rolls back the transaction.
//!
//! ## NFT Tradable Private Token Factory
//!
//! After creating a new factory contract instance , the fn caller becomes the owner.
//!
//! Token owners can assign other accounts for transferring specific tokens on their behalf.
//! It is also possible to authorize an operator (higher rights) for another account to handle tokens.
//!
//! ### NFT Token Creation
//!
//! Token creation start by calling the `mint(&mut self, id: u32)` fn.
//! The token owner becomes the fn caller. The Token ID needs to be specified
//! as the argument on this fn call.
//!
//! ### Token Registry
//!
//! Transfers may be initiated by:
//! - The owner of a token
//! - The approved AccountId of a token
//! - An authorized operator of the current owner of a token
//!
//! The token owner can transfer a token by calling the `transfer` or `transfer_from` functions.
//! An approved AccountId can make a token transfer by calling the `transfer_from` fn.
//! Operators can transfer tokens on another account's behalf or can approve a token transfer
//! for a different account.
//!
//! ### Token Disabled
//!
//! Tokens can be destroyed by burning them. Only the token owner is allowed to burn a token.

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_swap_router::{SubSwapRouter, SubSwapRouterRef};
mod bytes_lib;
mod i256;
mod path;
mod primitives;
mod safe_cast;
mod tick_math;
use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
pub mod sub_swap_router {
 use crate::path::Path;
    use crate::primitives::{I256, U160, U256};
    use crate::safe_cast::SafeCast;
    use crate::tick_math::TickMath;
    use ink_prelude::{vec, vec::Vec};
    use ink_primitives::Key;
    use ink_storage::{
        traits::{PackedAllocate, PackedLayout, SpreadAllocate, SpreadLayout},
        Mapping,
    };
    use scale::{Decode, Encode};

    #[derive(
        Default,
        scale::Encode,
        scale::Decode,
        Clone,
        SpreadAllocate,
        SpreadLayout,
        PackedLayout,
    )]
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
    pub struct SwapCallbackData {
        path: Vec<u8>,
        payer: AccountId,
    }

    #[derive(
        Default,
        scale::Encode,
        scale::Decode,
        Copy,
        Clone,
        SpreadAllocate,
        SpreadLayout,
        PackedLayout,
    )]
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
    /// @notice The identifying key of the pool
    struct PoolKey {
        token0: AccountId,
        token1: AccountId,
        fee: u32,
    }
 #[derive(
        Default,
        scale::Encode,
        scale::Decode,
        Copy,
        Clone,
        SpreadAllocate,
        SpreadLayout,
        PackedLayout,
    )]
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
 pub struct ExactInputSingleParams {
        token_in:AccountId,
        token_out:AccountId,
        fee:u32,
        recipient:AccountId,
        deadline:U256,
        amount_in:U256,
        amount_out_minimum:U256,
        sqrt_price_limit_x96:U160,
    }

 #[derive(
        Default,
        scale::Encode,
        scale::Decode,
        Clone,
        SpreadAllocate,
        SpreadLayout,
        PackedLayout,
    )]
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

    pub struct ExactInputParams {
        path:Vec<u8>,
        recipient:AccountId,
        deadline:U256,
        amount_in:U256,
        amount_out_minimum:U256,
    }

   #[derive(
        Default,
        scale::Encode,
        scale::Decode,
        Copy,
        Clone,
        SpreadAllocate,
        SpreadLayout,
        PackedLayout,
    )]
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
    pub struct ExactOutputSingleParams {
        token_in:AccountId,
        token_out:AccountId,
        fee:u32,
        recipient:AccountId,
        deadline:U256,
        amount_out:U256,
        amount_in_maximum:U256,
        sqrt_price_limit_x96:U160,
    }

   #[derive(
        Default,
        scale::Encode,
        scale::Decode,
        Clone,
        SpreadAllocate,
        SpreadLayout,
        PackedLayout,
    )]
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
    pub struct ExactOutputParams {
        path:Vec<u8>,
        recipient:AccountId,
        deadline:U256,
        amount_out:U256,
        amount_in_maximum:U256,
    }

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubSwapRouter {
        /// @dev Transient storage variable used for returning the computed amount in for an exact output swap.
        amount_in_cached: U256,
        //// Auction contract
        factory: AccountId,
        ////  wrapped_native_token contract
        wrapped_native_token: AccountId,
        ////  contract
        token0: AccountId,
        ////  contract
        token1: AccountId,
        /// NFT mint fee
        fee: u32,
        /// Platform fee
        tick_spacing: i32,
        ///  The contract original
        original: AccountId,
        ///  The contract owner
        owner: AccountId,
    }
    #[derive(Encode, Decode, Debug, PartialEq, Eq, Copy, Clone)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum Error {
        OnlyOwner,
        OnlyOriginal,
        TLU,
        TLM,
        TUM,
        AI,
        OLD,
        M0,
        M1,
        AS,
        LOK,
        SPL,
        IIA,
        F0,
        F1,
        I,
        L,
        NP,
        LO,
CallerIsNotPool,
        TransactionTooOld,
        TooLittleReceived,
        AmountOutReceivedDoesNotEqualAmountOut,
        TooMuchRequested,
        FeeBipsIsOutOfRange,
        InsufficientToken,
        NotWrappedNativeToken,
        InsufficientWrappedNativeToken,
        AmountIsZero,
        FeeProocolWrong,
        TickIsNotSpaced,
        UninitializedLower,
        UninitializedUpper,
        TransactionFailed,
        /// Returned if new owner  is the zero AccountId.
        NewOwnerIsTheZeroAddress,
    }

    // The SubSwapRouter result types.
    pub type Result<T> = core::result::Result<T, Error>;

    impl SubSwapRouter {
        /// Creates a new SubSwapRouter contract.
        #[ink(constructor)]
        pub fn new(factory: AccountId, wrapped_native_token: AccountId) -> Self {
            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.factory = factory;
                contract.wrapped_native_token = wrapped_native_token;
            })
        }
        /// @dev Used as the placeholder value amount_in_cached:for, because the computed amount in for an exact output swap
        /// can never actually be this value
        fn default_amount_in_cached() -> U256 {
            U256::MAX
        }
        fn check_deadline(&self,deadline: U256)->Result<()> {
            ensure!(self.block_timestamp() <= deadline.as_u128() as u32, Error::TransactionTooOld);
            Ok(())
        }

        fn get_pool(&self,token_a: AccountId, token_b: AccountId, fee: u32) -> AccountId {
           
                self
                    .compute_address(self.factory, self.get_pool_key(token_a, token_b, fee)).unwrap()
          
        }

        /// @inheritdoc IUniswapV3SwapCallback
        #[ink(message)]
        pub fn sub_swap_callback(&mut self,
            amount0_delta: I256,
            amount1_delta: I256,
            _data: Vec<u8>,
        ) -> Result<()> {
            ensure!(amount0_delta > I256::zero() || amount1_delta >  I256::zero(),Error::TransactionFailed); // swaps entirely within 0-liquidity regions are not supported
            let mut data=SwapCallbackData::decode(&mut &_data[..]).unwrap();
            let (mut token_in, token_out, fee) = Path::decode_first_pool(&data.path);
            self.verify_callback(self.factory, token_in, token_out, fee);

            let (is_exact_input, amount_to_pay) = if amount0_delta > I256::zero() {
                (token_in < token_out, U256::from(amount0_delta))
            } else {
                (token_out < token_in, U256::from(amount1_delta))
            };
            if is_exact_input {
                self.pay(token_in, data.payer, self.env().caller(), amount_to_pay);
            } else {
                // either initiate the next swap or pay
                if Path::has_multiple_pools(&data.path) {
                    data.path = Path::skip_token(&data.path);
                    self.exact_output_internal(amount_to_pay, self.env().caller(), U256::zero(), data);
                } else {
                    self.amount_in_cached = amount_to_pay;
                    token_in = token_out; // swap in/out because exact output swaps are reversed
                    self.pay(token_in, data.payer, self.env().caller(), amount_to_pay);
                }
            }
            Ok(())
        }

        /// @dev Performs a single exact input swap
        fn exact_input_internal(&mut self,
            amount_in: U256,
            mut recipient: AccountId,
            sqrt_price_limit_x96: U160,
            data: SwapCallbackData,
        ) -> U256 {
            // allow swapping to the router AccountId with AccountId 0
            if recipient == AccountId::from([0x0; 32]) {
                recipient = self.env().account_id();
            }

            let (token_in, token_out, fee) = Path::decode_first_pool(&data.path);

            let zero_for_one = token_in < token_out;

            let (amount0, amount1) =self.pool_swap( self.get_pool(token_in, token_out, fee),
                recipient,
                zero_for_one,
                SafeCast::to_int256(amount_in),
                if sqrt_price_limit_x96 == U256::zero() {
                    if zero_for_one {
                        TickMath::min_sqrt_ratio() + U256::one()
                    } else {
                        TickMath::max_sqrt_ratio() -  U256::one()
                    }
                } else {
                    sqrt_price_limit_x96
                },
                data.encode(),
            ).unwrap();

            return U256::from(-(if zero_for_one { amount1 } else { amount0 }));
        }

        /// @inheritdoc ISwapRouter
        #[ink(message,payable)]
        pub fn exact_input_single(&mut self,params: ExactInputSingleParams) -> Result<U256> {
               self.check_deadline(params.deadline);

            let amount_out = self.exact_input_internal(
                params.amount_in,
                params.recipient,
                params.sqrt_price_limit_x96,
                SwapCallbackData {
                    path: (params.token_in, params.fee, params.token_out).encode(),
                    payer: self.env().caller(),
                },
            );
            ensure!(
                amount_out >= params.amount_out_minimum,
                Error::TooLittleReceived
            );
            Ok(amount_out)
        }

        /// @inheritdoc ISwapRouter
        #[ink(message, payable)]
        pub fn exact_input(&mut self, params: ExactInputParams) -> Result<U256> {
            self.check_deadline(params.deadline);
            let  mut params=params.clone();
            let mut payer = self.env().caller(); // self.env().caller()pays for the first hop
            let mut amount_out;
            loop{
                let has_multiple_pools = Path::has_multiple_pools(&params.path);

                // the outputs of prior swaps become the inputs to subsequent ones
                params.amount_in = self.exact_input_internal(
                    params.amount_in,
                    if has_multiple_pools {
                        self.env().account_id()
                    } else {
                        params.recipient
                    }, // for intermediate swaps, this contract custodies
                    U256::zero(),
                    SwapCallbackData {
                        path: Path::get_first_pool(&params.path), // only the first pool in the path is necessary
                        payer,
                    },
                );

                // decide whether to continue or terminate
                if has_multiple_pools {
                    payer = self.env().account_id(); // at this point, the caller has paid
                    params.path = Path::skip_token(&params.path);
                } else {
                    amount_out = params.amount_in;
                    break;
                }
            }

            ensure!(
                amount_out >= params.amount_out_minimum,
                Error::TooLittleReceived
            );
            Ok(amount_out)
        }

        /// @dev Performs a single exact output swap
        fn exact_output_internal(&mut self,
            amount_out: U256,
            mut recipient: AccountId,
            sqrt_price_limit_x96: U160,
            data: SwapCallbackData,
        ) -> Result<U256> {
            // allow swapping to the router AccountId with AccountId 0
            if recipient == AccountId::from([0x0; 32]) {
                recipient = self.env().account_id();
            }

            let (token_out, token_in, fee) = Path::decode_first_pool(&data.path);

            let zero_for_one = token_in < token_out;

            let (amount0_delta, amount1_delta) = self.pool_swap(self.get_pool(token_in, token_out, fee),
                recipient,
                zero_for_one,
                -SafeCast::to_int256(amount_out),
                if sqrt_price_limit_x96 == U256::zero(){
                    if zero_for_one {
                        TickMath::min_sqrt_ratio() + U256::one()
                    } else {
                        TickMath::max_sqrt_ratio() - U256::one()
                    }
                } else {
                    sqrt_price_limit_x96
                },
                data.encode(),
            )?;

            let (amount_in, amount_out_received) = if zero_for_one {
                (U256::from(amount0_delta), U256::from(-amount1_delta))
            } else {
                (U256::from(amount1_delta), U256::from(-amount0_delta))
            };
            // it's technically possible to not receive the full output amount,
            // so if no price limit has been specified, ensure! this possibility away
            if sqrt_price_limit_x96 == U256::zero() {
                ensure!(
                    amount_out_received == amount_out,
                    Error::AmountOutReceivedDoesNotEqualAmountOut
                );
            }
            Ok(amount_in)
        }

        /// @inheritdoc ISwap_router
        #[ink(message, payable)]
        pub fn exact_output_single(&mut self,params: ExactOutputSingleParams) -> Result<U256> {
            self.check_deadline(params.deadline);

            // avoid an SLOAD by using the swap return data
            let amount_in = self.exact_output_internal(
                params.amount_out,
                params.recipient,
                params.sqrt_price_limit_x96,
                SwapCallbackData {
                    path: (params.token_out, params.fee, params.token_in).encode(),
                    payer: self.env().caller(),
                },
            );

            ensure!(
                amount_in.unwrap() <=params.amount_in_maximum,
                Error::TooMuchRequested
            );
            // has to be reset even though we don't use it in the single hop case
            self.amount_in_cached = Self::default_amount_in_cached();
            amount_in
        }

        /// @inheritdoc ISwap_router
        #[ink(message, payable)]
        pub fn exact_output(&mut self,params: ExactOutputParams) -> Result<U256> {
            self.check_deadline(params.deadline);

            // it's okay that the payer is fixed to self.env().caller()here, as they're only paying for the "final" exact output
            // swap, which happens first, and subsequent swaps are paid for within nested callback frames
            self.exact_output_internal(
                params.amount_out,
                params.recipient,
                U256::zero(),
                SwapCallbackData {
                    path: params.path,
                    payer: self.env().caller(),
                },
            );

            let amount_in = self.amount_in_cached;
            ensure!(
                amount_in <= params.amount_in_maximum,
                Error::TooMuchRequested
            );
            self.amount_in_cached = Self::default_amount_in_cached();
            Ok(amount_in)
        }

        /// @inheritdoc IPeriphery_payments_with_fee
        #[ink(message, payable)]
        pub fn unwrap_wrapped_native_token_with_fee(&mut self,
            amount_minimum: U256,
            recipient: AccountId,
            fee_bips: U256,
            fee_recipient: AccountId,
        )->Result<()>{
            ensure!(fee_bips > U256::zero() && fee_bips <= U256::from(100), Error::FeeBipsIsOutOfRange);

            let balance_wrapped_native_token =
                self.erc20_balance_of(self.wrapped_native_token,self.env().account_id()).unwrap();
            ensure!(
                balance_wrapped_native_token >= amount_minimum,
                Error::InsufficientWrappedNativeToken
            );

            if balance_wrapped_native_token > U256::zero() {
                self.erc20_withdraw(self.wrapped_native_token,balance_wrapped_native_token);
                let fee_amount = balance_wrapped_native_token*fee_bips / U256::from(10_000);
                if fee_amount > U256::zero() {
                    self.safe_transfer_native_token(fee_recipient, fee_amount);
                }
                self.safe_transfer_native_token(
                    recipient,
                    balance_wrapped_native_token - fee_amount,
                );
            }
            Ok(())
        }

        /// @inheritdoc IPeriphery_payments_with_fee
        #[ink(message, payable)]
        pub fn sweep_token_with_fee(&mut self,
            token: AccountId,
            amount_minimum: U256,
            recipient: AccountId,
            fee_bips: U256,
            fee_recipient: AccountId,
        ) ->Result<()>{
            ensure!(fee_bips >  U256::zero() && fee_bips <= U256::from(100), Error::FeeBipsIsOutOfRange);

            let balance_token = self.erc20_balance_of(token, self.env().account_id()).unwrap();
            ensure!(balance_token >= amount_minimum, Error::InsufficientToken);

            if balance_token > U256::zero() {
                let fee_amount = balance_token*fee_bips / U256::from(10_000);
                if fee_amount > U256::zero() {
                    self.erc20_transfer(token, fee_recipient, fee_amount);
                }
                self.erc20_transfer(token, recipient, balance_token - fee_amount);
            }
    Ok(())
        }
        #[ink(message, payable)]
        pub fn receive(&mut self) -> Result<()> {
            ensure!(
                self.env().caller() == self.wrapped_native_token,
                Error::NotWrappedNativeToken
            );
            Ok(())
        }

        /// @inheritdoc IPeriphery_payments
        #[ink(message, payable)]
        pub fn unwrap_wrapped_native_token(&mut self,amount_minimum: U256, recipient: AccountId) ->Result<()>{
            let balance_wrapped_native_token =
                self.erc20_balance_of(self.wrapped_native_token,self.env().account_id()).unwrap();
            ensure!(
                balance_wrapped_native_token >= amount_minimum,
                Error::InsufficientWrappedNativeToken
            );

            if balance_wrapped_native_token > U256::zero() {
                self.erc20_withdraw(self.wrapped_native_token,balance_wrapped_native_token);
                self.safe_transfer_native_token(recipient, balance_wrapped_native_token);
            }
Ok(())
        }

        /// @inheritdoc IPeriphery_payments
        #[ink(message, payable)]
        pub fn sweep_token(&mut self,token: AccountId, amount_minimum: U256, recipient: AccountId) ->Result<()>{
            let balance_token = self.erc20_balance_of(token, self.env().account_id()).unwrap();
            ensure!(balance_token >= amount_minimum, Error::InsufficientToken);

            if balance_token > U256::zero() {
                self.erc20_transfer(token, recipient, balance_token);
            }
Ok(())
        }

        /// @inheritdoc IPeriphery_payments
        #[ink(message, payable)]
        pub fn refund_native_token(&mut self) ->Result<()>{
            if self.env().balance() > 0 {
                self.safe_transfer_native_token(self.env().caller(), U256::from(self.env().balance()));
            }
            Ok(())
        }

        /// @param token The token to pay
        /// @param payer The entity that must pay
        /// @param recipient The entity that will receive payment
        /// @param value The amount to pay
        fn pay(&mut self,token: AccountId, payer: AccountId, recipient: AccountId, value: U256)->Result<()>{
            if token == self.wrapped_native_token && self.env().balance() >= value.as_u128() {
                // pay with wrapped_native_token
                self.erc20_deposit(self.wrapped_native_token, value); // wrap only what is needed to pay
                self.erc20_transfer(self.wrapped_native_token, recipient, value);
            } else if payer == self.env().account_id() {
                // pay with tokens already in the contract (for the exact input multihop case)
                self.erc20_transfer(token, recipient, value);
            } else {
                // pull payment
                self.erc20_transfer_from(token, payer, recipient, value);
            }
Ok(())
        }

        #[ink(message, payable)]
        pub fn multicall(&mut self,data: Vec<Vec<u8>>) -> Result<Vec<Vec<u8>>> {
            let mut results = vec![Vec::new(); data.len()];
            for (i, d) in data.iter().enumerate() {
                let result = self.delegatecall(self.env().account_id(), d);

                if let Err(e) = result {
                    // Next 5 lines from https://ethereum.stackexchange.com/a/83577
                    if e==Error::TransactionFailed {
                        assert!(false);
                    }

                    assert!(false, "{:?}",e.encode());
                }

                results[i] = result.unwrap().encode();
            }
            Ok(results)
        }
        /// @inheritdoc ISelf_permit
        #[ink(message, payable)]
        pub fn self_permit(&mut self,
            token: AccountId,
            value: U256,
            deadline: U256,
            v: u8,
            r: Vec<u8>,
            s: Vec<u8>,
        )->Result<()> {
            self.erc20_permit_permit(
                token,
                self.env().caller(),
                self.env().account_id(),
                value,
                deadline,
                v,
                r,
                s,
            )
        }

        /// @inheritdoc ISelf_permit
        #[ink(message, payable)]
        pub fn self_permit_if_necessary(&mut self,
            token: AccountId,
            value: U256,
            deadline: U256,
            v: u8,
            r: Vec<u8>,
            s: Vec<u8>,
        )->Result<()> {
            if self.erc20_allowance(token, self.env().caller(), self.env().account_id()).unwrap() < value {
                self.self_permit(token, value, deadline, v, r, s);
            }
            Ok(())
        }

        /// @inheritdoc ISelf_permit
        #[ink(message, payable)]
        pub fn self_permit_allowed(&mut self,
            token: AccountId,
            nonce: U256,
            expiry: U256,
            v: u8,
            r: Vec<u8>,
            s: Vec<u8>,
        )->Result<()> {
            self.erc20_permit_allowed_permit(
                token,
                self.env().caller(),
                self.env().account_id(),
                nonce,
                expiry,
                true,
                v,
                r,
                s,
            )
        }

        /// @inheritdoc ISelf_permit
        #[ink(message, payable)]
        pub fn self_permit_allowed_if_necessary(&mut self,
            token: AccountId,
            nonce: U256,
            expiry: U256,
            v: u8,
            r: Vec<u8>,
            s: Vec<u8>,
        ) ->Result<()>{
            if self.erc20_allowance(token, self.env().caller(), self.env().account_id()).unwrap()
                < U256::MAX
            {
                self.self_permit_allowed(token, nonce, expiry, v, r, s);
            }
Ok(())
        }

        /// @dev Common checks for valid tick inputs.
        fn check_ticks(&self, tick_lower: i32, tick_upper: i32) -> Result<()> {
            ensure!(tick_lower < tick_upper, Error::TLU);
            ensure!(tick_lower >= TickMath::MIN_TICK, Error::TLM);
            ensure!(tick_upper <= TickMath::MAX_TICK, Error::TUM);
            Ok(())
        }

        /// @dev Returns the block timestamp truncated to 32 bits, i.e. mod 2**32. This method is overridden in tests.
        /// # Return
        ///  current time  timestamp (s)
        #[ink(message)]
        pub fn now_timestamp(&self) -> u32 {
            self.block_timestamp()
        }
        fn block_timestamp(&self) -> u32 {
            self.env().block_timestamp() as _
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn pool_deployer_parameters(
            pool_deployer: AccountId,
        ) -> Result<(AccountId, AccountId, AccountId, u32, i32)> {
            #[cfg(test)]
            {
                Ok((
                    AccountId::from([0x0; 32]),
                    AccountId::from([0x0; 32]),
                    AccountId::from([0x0; 32]),
                    0,
                    0,
                ))
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = ink_lang::selector_bytes!("parameters");
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(pool_deployer)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()))
                    .returns::<(AccountId, AccountId, AccountId, u32, i32)>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc20_balance_of= {:?}", e);
                        Error::TransactionFailed
                    })
            }
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

        /// Querying factory contract AccountId
        /// # return factory contract AccountId
        #[ink(message)]
        pub fn factory(&self) -> AccountId {
            self.factory
        }
        /// Querying token0 contract AccountId
        /// # return token0 contract AccountId
        #[ink(message)]
        pub fn token0(&self) -> AccountId {
            self.token0
        }

        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
    
   
    }

    #[ink(impl)]
    impl SubSwapRouter {
        // Vec<u8>  constant POOL_INIT_CODE_HASH = 0xe34f199b19b2b4f47f68442619d555527d244f78a3297ea89325f843f87b8b54;
        /// @notice Returns PoolKey: the ordered tokens with the matched fee levels
        /// @param tokenA The first token of a pool, unsorted
        /// @param tokenB The second token of a pool, unsorted
        /// @param fee The fee level of the pool
        /// @return Poolkey The pool details with ordered token0 and token1 assignments
        fn get_pool_key(&self,token_a: AccountId, token_b: AccountId, fee: u32) -> PoolKey {
           let  (token_a, token_b) = if token_a > token_b {
               (token_b, token_a)
            }else{ (token_a, token_b) };
            PoolKey {
                token0: token_a,
                token1: token_b,
                fee,
            }
        }

        /// @notice Deterministically computes the pool AccountId given the factory and PoolKey
        /// @param factory The Uniswap V3 factory contract AccountId
        /// @param key The PoolKey
        /// @return pool The contract AccountId of the V3 pool
        fn compute_address(&self,factory: AccountId, key: PoolKey) -> Result<AccountId> {
            ensure!(key.token0 < key.token1,Error::TransactionFailed);
            let pool = AccountId::from([0x0; 32]);
            //    U256::from(
            //                 keccak256(
            //                     abi.encodePacked(
            //                         hex'ff',
            //                         factory,
            //                         keccak256(abi.encode(key.token0, key.token1, key.fee)),
            //                         POOL_INIT_CODE_HASH
            //                     )
            //                 )
            //             )
            Ok(pool)
        }
    }

    /// @notice Provides validation for callbacks from Uniswap V3 Pools
    #[ink(impl)]
    impl SubSwapRouter {
        /// @notice Returns the AccountId of a valid Uniswap V3 Pool
        /// @param factory The contract AccountId of the Uniswap V3 factory
        /// @param token_a The contract AccountId of either token0 or token1
        /// @param token_b The contract AccountId of the other token
        /// @param fee The fee collected upon every swap in the pool, denominated in hundredths of a bip
        /// @return pool The V3 pool contract AccountId
        fn verify_callback(&self,
            factory: AccountId,
            token_a: AccountId,
            token_b: AccountId,
            fee: u32,
        ) -> Result<AccountId> {
            self.verify_callback_internal(factory, self.get_pool_key(token_a, token_b, fee))
        }

        /// @notice Returns the AccountId of a valid Uniswap V3 Pool
        /// @param factory The contract AccountId of the Uniswap V3 factory
        /// @param pool_key The identifying key of the V3 pool
        /// @return pool The V3 pool contract AccountId
        fn verify_callback_internal(&self, factory: AccountId, pool_key: PoolKey) -> Result<AccountId> {
            let pool = self.compute_address(factory, pool_key)?;
            ensure!(self.env().caller() == pool, Error::CallerIsNotPool);
            Ok(pool)
        }
    }

    /// @title TransferHelper
    /// @notice Contains helper methods for interacting with ERC20 tokens that do not consistently return true/false
    #[ink(impl)]
    impl SubSwapRouter {
#[cfg_attr(test, allow(unused_variables))]
        fn pool_swap(&mut self, token:AccountId,       recipient: AccountId,
            zero_for_one: bool,
            amount_specified: I256,
            sqrt_price_limit_x96: U160,
            data: Vec<u8>,
        ) -> Result<(I256, I256)>  {
   
            Ok((I256::zero(),I256::zero()))
        }

 #[cfg_attr(test, allow(unused_variables))]
        fn safe_transfer_native_token(&mut self,  to: AccountId, value: U256) -> Result<()> {
                 ensure!(
                        self.env().transfer(to, value.as_u128()).is_ok(),
                        Error::TransactionFailed
                    );
            Ok(())
        }

        /// @notice Transfers tokens from self.env().caller()to a recipient
        /// @dev Calls transfer on token contract, errors with TF if transfer fails
        /// @param token The contract AccountId of the token which will be transferred
        /// @param to The recipient of the transfer
        /// @param value The value of the transfer
        #[cfg_attr(test, allow(unused_variables))]
        fn erc20_transfer(&mut self, token: AccountId, to: AccountId, value: U256) -> Result<()> {
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
                            .push_arg(value.as_u128()),
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
        fn delegatecall(&mut self, token: AccountId,data:&Vec<u8>) -> Result<()> {
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
        fn erc20_withdraw(&mut self, token: AccountId, value: U256) -> Result<()> {
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
                            .push_arg(value.as_u128()),
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
        fn erc20_deposit(&mut self, token: AccountId, value: U256) -> Result<()> {
            let from=self.env().account_id();
            let to=token;
           let value=value.as_u128();
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
        fn erc20_transfer_from(
            &mut self,
            token: AccountId,
            from: AccountId,
            to: AccountId,
            value: U256,
        ) -> Result<()> {
            let value=value.as_u128();
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
        fn erc20_balance_of(&self, token: AccountId, owner: AccountId) -> Result<U256> {
            #[cfg(test)]
            {
                Ok(U256::zero())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x0F, 0x75, 0x5A, 0x56]; //erc20 balance_of
                let (gas_limit, transferred_value) = (0, 0);
                Ok(U256::from(build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
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
                    }).unwrap()))
            }
        }

        #[cfg_attr(test, allow(unused_variables))]
        fn erc20_allowance(&self, token: AccountId, owner: AccountId,spender:AccountId) -> Result<U256> {
            #[cfg(test)]
            {
                Ok(U256::zero())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = [0x0F, 0x75, 0x5A, 0x56]; //erc20 balance_of
                let (gas_limit, transferred_value) = (0, 0);
                 Ok(U256::from(build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
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
                    }).unwrap()))
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn erc20_permit_permit(&mut self,
            token: AccountId,
            owner: AccountId,
            spender: AccountId,
            value: U256,
            deadline: U256,
            v: u8,
            r: Vec<u8>,
            s: Vec<u8>,
        ) ->Result<()>{
Ok(())
        }
 #[cfg_attr(test, allow(unused_variables))]
        fn erc20_permit_allowed_permit(&mut self,
            token: AccountId,
            owner: AccountId,
            spender: AccountId,
            nonce: U256,
            expiry: U256,
            allowed:bool,
            v: u8,
            r: Vec<u8>,
            s: Vec<u8>,
        ) ->Result<()>{
Ok(())
        }
    }

    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_lang as ink;
        type Event = <SubSwapRouter as ::ink_lang::reflect::ContractEventBase>::Type;
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
            django()
        }
        // fn init_contract() -> SubSwapRouter {
        //     let erc = SubSwapRouter::new(
        //         django(),
        //         eve(),
        //         frank(),
        //         1,
        //         10,
        //         fee_recipient(),
        //         Hash::from([0x99; 32]),
        //     );

        //     erc
        // }

        // #[ink::test]
        // fn update_auction_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     let auction = bob();
        //     assert!(art_factory.update_auction(auction).is_ok());

        //     assert_eq!(art_factory.auction, auction);
        // }

        // #[ink::test]
        // fn update_auction_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = charlie();
        //     set_caller(caller);
        //     let auction = bob();
        //     assert_eq!(
        //         art_factory.update_auction(auction).unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn update_marketplace_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     let marketplace = bob();
        //     assert!(art_factory.update_marketplace(marketplace).is_ok());

        //     assert_eq!(art_factory.marketplace, marketplace);
        // }

        // #[ink::test]
        // fn update_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = charlie();
        //     set_caller(caller);
        //     let marketplace = bob();
        //     assert_eq!(
        //         art_factory.update_marketplace(marketplace).unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn update_bundle_marketplace_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     let bundle_marketplace = bob();
        //     assert!(art_factory
        //         .update_bundle_marketplace(bundle_marketplace)
        //         .is_ok());

        //     assert_eq!(art_factory.bundle_marketplace, bundle_marketplace);
        // }

        // #[ink::test]
        // fn update_bundle_marketplace_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = charlie();
        //     set_caller(caller);
        //     let bundle_marketplace = bob();
        //     assert_eq!(
        //         art_factory
        //             .update_bundle_marketplace(bundle_marketplace)
        //             .unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn update_mint_fee_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     let mint_fee = 10;
        //     assert!(art_factory.update_mint_fee(mint_fee).is_ok());

        //     assert_eq!(art_factory.mint_fee, mint_fee);
        // }

        // #[ink::test]
        // fn update_mint_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = charlie();
        //     set_caller(caller);
        //     let mint_fee = 10;
        //     assert_eq!(
        //         art_factory.update_mint_fee(mint_fee).unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn update_platform_fee_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     let platform_fee = 10;
        //     assert!(art_factory.update_platform_fee(platform_fee).is_ok());

        //     assert_eq!(art_factory.platform_fee, platform_fee);
        // }

        // #[ink::test]
        // fn update_platform_fee_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = bob();
        //     set_caller(caller);
        //     let platform_fee = 10;
        //     assert_eq!(
        //         art_factory.update_platform_fee(platform_fee).unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn update_platform_fee_recipient_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     let fee_recipient = bob();
        //     assert!(art_factory
        //         .update_platform_fee_recipient(fee_recipient)
        //         .is_ok());

        //     assert_eq!(art_factory.fee_recipient, fee_recipient);
        // }
        // #[ink::test]
        // fn update_platform_fee_recipient_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = bob();
        //     set_caller(caller);
        //     let fee_recipient = bob();
        //     assert_eq!(
        //         art_factory
        //             .update_platform_fee_recipient(fee_recipient)
        //             .unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn create_nft_contract_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);

        //     let contract_addr =
        //         art_factory.create_nft_contract(String::from("test"), String::from("TEST"));
        //     // assert_eq!(contract_addr.unwrap_err(),Error::TransferOwnershipFailed);
        //     assert!(contract_addr.is_ok());

        //     // // Token 1 does not exists.
        //     assert_eq!(art_factory.exists.get(&contract_addr.unwrap()), Some(true));
        //     assert_eq!(get_balance(fee_recipient()), 10);
        //     let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
        //     assert_eq!(emitted_events.len(), 1);
        //     assert_contract_created_event(&emitted_events[0], caller, contract_addr.unwrap());
        // }

        // #[ink::test]
        // fn create_nft_contract_failed_if_the_transferred_value_is_less_than_the_platform_fee() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(1);

        //     let contract_addr =
        //         art_factory.create_nft_contract(String::from("test"), String::from("TEST"));
        //     assert_eq!(contract_addr.unwrap_err(), Error::InsufficientFunds);
        // }

        // #[ink::test]
        // fn create_nft_contract_failed_if_instantiate_contract_failed() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     ink_env::test::set_value_transferred::<ink_env::DefaultEnvironment>(10);
        //     art_factory.test_instantiate_contract_failed = true;
        //     let contract_addr =
        //         art_factory.create_nft_contract(String::from("test"), String::from("TEST"));
        //     assert_eq!(contract_addr.unwrap_err(), Error::TransferOwnershipFailed);
        // }
        // #[ink::test]
        // fn register_token_contract_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = eve();

        //     art_factory.test_support_interface = crate::INTERFACE_ID_ERC721;
        //     // assert_eq!(art_factory
        //     //     .register_token_contract(
        //     //       token_contract,
        //     //     ).unwrap_err(),Error::TransferOwnershipFailed);
        //     assert!(art_factory.register_token_contract(token_contract,).is_ok());

        //     assert_eq!(art_factory.exists.get(&token_contract), Some(true));
        //     let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
        //     assert_eq!(emitted_events.len(), 1);
        //     assert_contract_created_event(&emitted_events[0], caller, token_contract);
        // }

        // #[ink::test]
        // fn register_token_contract_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = charlie();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = django();
        //     assert_eq!(
        //         art_factory
        //             .register_token_contract(token_contract,)
        //             .unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn register_token_contract_failed_if_the_token_contract_exists_in_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = django();
        //     art_factory.exists.insert(&token_contract, &true);
        //     assert_eq!(
        //         art_factory
        //             .register_token_contract(token_contract,)
        //             .unwrap_err(),
        //         Error::NFTContractAlreadyRegistered
        //     );
        // }

        // #[ink::test]
        // fn register_token_contract_failed_if_the_artion_does_not_support_erc_1155() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = django();
        //     assert_eq!(
        //         art_factory
        //             .register_token_contract(token_contract,)
        //             .unwrap_err(),
        //         Error::NotAnERC721Contract
        //     );
        // }

        // #[ink::test]
        // fn disable_token_contract_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = frank();
        //     art_factory.exists.insert(&token_contract, &true);
        //     // assert_eq!(contract_addr.unwrap_err(),Error::TransferOwnershipFailed);
        //     assert!(art_factory.disable_token_contract(token_contract,).is_ok());

        //     // // Token 1 does not exists.
        //     assert_eq!(art_factory.exists.get(&token_contract), Some(false));
        //     let emitted_events = ink_env::test::recorded_events().collect::<Vec<_>>();
        //     assert_eq!(emitted_events.len(), 1);
        //     assert_contract_disabled_event(&emitted_events[0], caller, token_contract);
        // }

        // #[ink::test]
        // fn disable_token_contract_failed_if_the_caller_is_not_the_owner_of_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = charlie();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = django();
        //     art_factory.exists.insert(&token_contract, &true);
        //     assert_eq!(
        //         art_factory
        //             .disable_token_contract(token_contract,)
        //             .unwrap_err(),
        //         Error::OnlyOwner
        //     );
        // }

        // #[ink::test]
        // fn disable_token_contract_failed_if_the_token_contract_does_not_exist_in_the_contract() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = django();
        //     assert_eq!(
        //         art_factory
        //             .disable_token_contract(token_contract,)
        //             .unwrap_err(),
        //         Error::NFTContractIsNotRegistered
        //     );
        // }

        // #[ink::test]
        // fn exists_contract_works() {
        //     // Create a new contract instance.
        //     let mut art_factory = init_contract();
        //     let caller = alice();
        //     set_caller(caller);
        //     set_balance(caller, 10);
        //     set_balance(fee_recipient(), 0);
        //     let token_contract = charlie();
        //     art_factory.exists.insert(&token_contract, &true);

        //     // // Token 1 does not exists.
        //     assert!(art_factory.exists(token_contract));
        // }

        // fn assert_contract_created_event(
        //     event: &ink_env::test::EmittedEvent,
        //     expected_creator: AccountId,
        //     expected_nft_address: AccountId,
        // ) {
        //     let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
        //         .expect("encountered invalid contract event data buffer");
        //     if let Event::ContractCreated(ContractCreated {
        //         creator,
        //         nft_address,
        //     }) = decoded_event
        //     {
        //         assert_eq!(
        //             creator, expected_creator,
        //             "encountered invalid ContractCreated.creator"
        //         );
        //         assert_eq!(
        //             nft_address, expected_nft_address,
        //             "encountered invalid ContractCreated.nft_address"
        //         );
        //     } else {
        //         panic!("encountered unexpected event kind: expected a ContractCreated event")
        //     }
        // }

        // fn assert_contract_disabled_event(
        //     event: &ink_env::test::EmittedEvent,
        //     expected_caller: AccountId,
        //     expected_nft_address: AccountId,
        // ) {
        //     let decoded_event = <Event as scale::Decode>::decode(&mut &event.data[..])
        //         .expect("encountered invalid contract event data buffer");
        //     if let Event::ContractDisabled(ContractDisabled {
        //         caller,
        //         nft_address,
        //     }) = decoded_event
        //     {
        //         assert_eq!(
        //             caller, expected_caller,
        //             "encountered invalid ContractDisabled.caller"
        //         );
        //         assert_eq!(
        //             nft_address, expected_nft_address,
        //             "encountered invalid ContractDisabled.nft_address"
        //         );
        //     } else {
        //         panic!("encountered unexpected event kind: expected a ContractDisabled event")
        //     }
        // }

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
