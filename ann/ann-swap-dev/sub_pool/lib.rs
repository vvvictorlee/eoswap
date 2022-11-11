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
//! Any function that modifies the state returns a `Result` type and does not changes the state
//! if the `Error` occurs.
//! The errors are defined as an `enum` type. Any other error or invariant violation
//! triggers a panic and therefore rolls back the transaction.
//!
//! ## NFT Tradable Private Token Factory
//!
//! After creating a new factory contract instance , the function caller becomes the owner.
//!
//! Token owners can assign other accounts for transferring specific tokens on their behalf.
//! It is also possible to authorize an operator (higher rights) for another account to handle tokens.
//!
//! ### NFT Token Creation
//!
//! Token creation start by calling the `mint(&mut self, id: u32)` function.
//! The token owner becomes the function caller. The Token ID needs to be specified
//! as the argument on this function call.
//!
//! ### Token Registry
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
//! ### Token Disabled
//!
//! Tokens can be destroyed by burning them. Only the token owner is allowed to burn a token.

#![cfg_attr(not(feature = "std"), no_std)]
pub use self::sub_pool::{SubPool, SubPoolRef};
mod bit_math;
mod fixed_point128;
mod fixed_point96;
mod full_math;
mod i256;
mod liquidity_math;
mod primitives;
mod safe_cast;
mod sqrt_price_math;
mod swap_math;
mod tick_math;
mod unsafe_math;

use ink_lang as ink;
macro_rules! ensure {
    ( $condition:expr, $error:expr $(,)? ) => {{
        if !$condition {
            return ::core::result::Result::Err(::core::convert::Into::into($error));
        }
    }};
}
#[ink::contract]
pub mod sub_pool {
    use crate::bit_math::BitMath;
    use crate::fixed_point128::FixedPoint128;
    use crate::full_math::FullMath;
    use crate::liquidity_math::LiquidityMath;
    use crate::primitives::{I256, U160, U256};
    use crate::safe_cast::SafeCast;
    use crate::sqrt_price_math::SqrtPriceMath;
    use crate::swap_math::SwapMath;
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
    pub struct Slot0 {
        // the current price
        sqrt_price_x96: U160,
        // the current tick
        tick: i32,
        // the most-recently updated index of the observations array
        observation_index: u16,
        // the current maximum number of observations that are being stored
        observation_cardinality: u16,
        // the next maximum number of observations to store, triggered in observations.write
        observation_cardinality_next: u16,
        // the current protocol fee as a percentage of the swap fee taken on withdrawal
        // represented as an integer denominator (1/x)%
        fee_protocol: u8,
        // whether the pool is locked
        unlocked: bool,
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
    pub struct ProtocolFees {
        // the current tick
        token0: u128,
        // the most-recently updated index of the observations array
        token1: u128,
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
    // info stored for each initialized individual tick
    pub struct TickInfo {
        // the total position liquidity that references this tick
        liquidity_gross: u128,
        // amount of net liquidity added (subtracted) when tick is crossed from left to right (right to left),
        liquidity_net: i128,
        // fee growth per unit of liquidity on the _other_ side of this tick (relative to the current tick)
        // only has relative meaning, not absolute — the value depends on when the tick is initialized
        fee_growth_outside0_x128: U256,
        fee_growth_outside1_x128: U256,
        // the cumulative tick value on the other side of the tick
        tick_cumulative_outside: i64,
        // the seconds per unit of liquidity on the _other_ side of this tick (relative to the current tick)
        // only has relative meaning, not absolute — the value depends on when the tick is initialized
        seconds_per_liquidity_outside_x128: U160,
        // the seconds spent on the other side of the tick (relative to the current tick)
        // only has relative meaning, not absolute — the value depends on when the tick is initialized
        seconds_outside: u32,
        // true iff the tick is initialized, i.e. the value is exactly equivalent to the expression liquidity_gross != 0
        // these 8 bits are set to prevent fresh sstores when crossing newly initialized ticks
        initialized: bool,
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
    // info stored for each user's position
    pub struct PositionInfo {
        // the amount of liquidity owned by this position
        liquidity: u128,
        // fee growth per unit of liquidity as of the last update to liquidity or fees owed
        fee_growth_inside0_last_x128: U256,
        fee_growth_inside1_last_x128: U256,
        // the fees owed to the position owner in token0/token1
        tokens_owed0: u128,
        tokens_owed1: u128,
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
    pub struct Observation {
        // the block timestamp of the observation
        block_timestamp: u32,
        // the tick accumulator, i.e. tick * time elapsed since the pool was first initialized
        tick_cumulative: i64,
        // the seconds per liquidity, i.e. seconds elapsed / max(1, liquidity) since the pool was first initialized
        seconds_per_liquidity_cumulative_x128: U160,
        // whether or not the observation is initialized
        initialized: bool,
    }

    impl PackedAllocate for Observation {
        fn allocate_packed(&mut self, at: &Key) {
            PackedAllocate::allocate_packed(&mut self.block_timestamp, at);
            PackedAllocate::allocate_packed(&mut self.tick_cumulative, at);
            PackedAllocate::allocate_packed(&mut self.seconds_per_liquidity_cumulative_x128, at);
            PackedAllocate::allocate_packed(&mut self.initialized, at);
        }
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
    pub struct ModifyPositionParams {
        // the address that owns the position
        owner: AccountId,
        // the lower and upper tick of the position
        tick_lower: i32,
        tick_upper: i32,
        // any change in liquidity
        liquidity_delta: i128,
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
    pub struct SwapCache {
        // the protocol fee for the input token
        fee_protocol: u8,
        // liquidity at the beginning of the swap
        liquidity_start: u128,
        // the timestamp of the current block
        block_timestamp: u32,
        // the current value of the tick accumulator, computed only if we cross an initialized tick
        tick_cumulative: i64,
        // the current value of seconds per liquidity accumulator, computed only if we cross an initialized tick
        seconds_per_liquidity_cumulative_x128: U160,
        // whether we've computed and cached the above two accumulators
        computed_latest_observation: bool,
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
    // the top level state of the swap, the results of which are recorded in storage at the end
    pub struct SwapState {
        // the amount remaining to be swapped in/out of the input/output asset
        amount_specified_remaining: I256,
        // the amount already swapped out/in of the output/input asset
        amount_calculated: I256,
        // current sqrt(price)
        sqrt_price_x96: U160,
        // the tick associated with the current price
        tick: i32,
        // the global fee growth of the input token
        fee_growth_global_x128: U256,
        // amount of input token paid as protocol fee
        protocol_fee: u128,
        // the current liquidity in range
        liquidity: u128,
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
    pub struct StepComputations {
        // the price at the beginning of the step
        sqrt_price_start_x96: U160,
        // the next tick to swap to from the current tick in the swap direction
        tick_next: i32,
        // whether tick_next is initialized or not
        initialized: bool,
        // sqrt(price) for the next tick (1/0)
        sqrt_price_next_x96: U160,
        // how much is being swapped in in this step
        amount_in: U256,
        // how much is being swapped out
        amount_out: U256,
        // how much fee is being paid in
        fee_amount: U256,
    }

    #[ink(storage)]
    #[derive(Default, SpreadAllocate)]
    pub struct SubPool {
        //// Auction contract
        factory: AccountId,
        //// Marketplace contract
        token0: AccountId,
        //// BundleMarketplace contract
        token1: AccountId,
        /// NFT mint fee
        fee: u32,
        /// Platform fee
        tick_spacing: i32,
        /// Platform fee receipient
        max_liquidity_per_tick: u128,
        slot0: Slot0,
        fee_growth_global0_x128: U256,
        fee_growth_global1_x128: U256,
        protocol_fees: ProtocolFees,
        liquidity: u128,
        ticks: Mapping<i32, TickInfo>,
        tick_bitmap: Mapping<i16, U256>,
        positions: Mapping<(AccountId, i32, i32), PositionInfo>,
        observations: Vec<Observation>,
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
        AmountIsZero,
        FeeProocolWrong,
        TickIsNotSpaced,
        UninitializedLower,
        UninitializedUpper,
        TransactionFailed,
        /// Returned if new owner  is the zero address.
        NewOwnerIsTheZeroAddress,
    }

    // The SubPool result types.
    pub type Result<T> = core::result::Result<T, Error>;

    /// @notice Emitted exactly once by a pool when #initialize is first called on the pool
    /// @dev Mint/Burn/Swap cannot be emitted by the pool before Initialize
    /// @param sqrt_price_x96 The initial sqrt price of the pool, as a Q64.96
    /// @param tick The initial tick of the pool, i.e. log base 1.0001 of the starting price of the pool
    #[ink(event)]
    pub struct Initialize {
        sqrt_price_x96: U160,
        tick: i32,
    }

    /// @notice Emitted when liquidity is minted for a given position
    /// @param sender The address that minted the liquidity
    /// @param owner The owner of the position and recipient of any minted liquidity
    /// @param tickLower The lower tick of the position
    /// @param tickUpper The upper tick of the position
    /// @param amount The amount of liquidity minted to the position range
    /// @param amount0 How much token0 was required for the minted liquidity
    /// @param amount1 How much token1 was required for the minted liquidity
    #[ink(event)]
    pub struct Mint {
        sender: AccountId,
        #[ink(topic)]
        owner: AccountId,
        #[ink(topic)]
        tick_lower: i32,
        #[ink(topic)]
        tick_upper: i32,
        amount: u128,
        amount0: U256,
        amount1: U256,
    }
    /// @notice Emitted when fees are collected by the owner of a position
    /// @dev Collect events may be emitted with zero amount0 and amount1 when the caller chooses not to collect fees
    /// @param owner The owner of the position for which fees are collected
    /// @param tickLower The lower tick of the position
    /// @param tickUpper The upper tick of the position
    /// @param amount0 The amount of token0 fees collected
    /// @param amount1 The amount of token1 fees collected
    #[ink(event)]
    pub struct Collect {
        #[ink(topic)]
        owner: AccountId,
        recipient: AccountId,
        #[ink(topic)]
        tick_lower: i32,
        #[ink(topic)]
        tick_upper: i32,
        amount0: u128,
        amount1: u128,
    }
    /// @notice Emitted when a position's liquidity is removed
    /// @dev Does not withdraw any fees earned by the liquidity position, which must be withdrawn via #collect
    /// @param owner The owner of the position for which liquidity is removed
    /// @param tickLower The lower tick of the position
    /// @param tickUpper The upper tick of the position
    /// @param amount The amount of liquidity to remove
    /// @param amount0 The amount of token0 withdrawn
    /// @param amount1 The amount of token1 withdrawn

    #[ink(event)]
    pub struct Burn {
        #[ink(topic)]
        owner: AccountId,
        #[ink(topic)]
        tick_lower: i32,
        #[ink(topic)]
        tick_upper: i32,
        amount: u128,
        amount0: U256,
        amount1: U256,
    }
    /// @notice Emitted by the pool for any swaps between token0 and token1
    /// @param sender The address that initiated the swap call, and that received the callback
    /// @param recipient The address that received the output of the swap
    /// @param amount0 The delta of the token0 balance of the pool
    /// @param amount1 The delta of the token1 balance of the pool
    /// @param sqrt_price_x96 The sqrt(price) of the pool after the swap, as a Q64.96
    /// @param liquidity The liquidity of the pool after the swap
    /// @param tick The log base 1.0001 of price of the pool after the swap
    #[ink(event)]
    pub struct Swap {
        #[ink(topic)]
        sender: AccountId,
        #[ink(topic)]
        recipient: AccountId,
        amount0: I256,
        amount1: I256,
        sqrt_price_x96: U160,
        liquidity: u128,
        tick: i32,
    }
    /// @notice Emitted by the pool for any flashes of token0/token1
    /// @param sender The address that initiated the swap call, and that received the callback
    /// @param recipient The address that received the tokens from flash
    /// @param amount0 The amount of token0 that was flashed
    /// @param amount1 The amount of token1 that was flashed
    /// @param paid0 The amount of token0 paid for the flash, which can exceed the amount0 plus the fee
    /// @param paid1 The amount of token1 paid for the flash, which can exceed the amount1 plus the fee
    #[ink(event)]
    pub struct Flash {
        #[ink(topic)]
        sender: AccountId,
        #[ink(topic)]
        recipient: AccountId,
        amount0: U256,
        amount1: U256,
        paid0: U256,
        paid1: U256,
    }
    /// @notice Emitted by the pool for increases to the number of observations that can be stored
    /// @dev observation_cardinality_next is not the observation cardinality until an observation is written at the index
    /// just before a mint/swap/burn.
    /// @param observation_cardinality_next_old The previous value of the next observation cardinality
    /// @param observation_cardinality_next_new The updated value of the next observation cardinality
    #[ink(event)]
    pub struct IncreaseObservationCardinalityNext {
        observation_cardinality_next_old: u16,
        observation_cardinality_next_new: u16,
    }
    /// @notice Emitted when the protocol fee is changed by the pool
    /// @param feeProtocol0Old The previous value of the token0 protocol fee
    /// @param feeProtocol1Old The previous value of the token1 protocol fee
    /// @param feeProtocol0New The updated value of the token0 protocol fee
    /// @param feeProtocol1New The updated value of the token1 protocol fee
    #[ink(event)]
    pub struct SetFeeProtocol {
        fee_protocol0_old: u8,
        fee_protocol1_old: u8,
        fee_protocol0_new: u8,
        fee_protocol1_new: u8,
    }
    /// @notice Emitted when the collected protocol fees are withdrawn by the factory owner
    /// @param sender The address that collects the protocol fees
    /// @param recipient The address that receives the collected protocol fees
    /// @param amount0 The amount of token0 protocol fees that is withdrawn
    /// @param amount0 The amount of token1 protocol fees that is withdrawn
    #[ink(event)]
    pub struct CollectProtocol {
        #[ink(topic)]
        sender: AccountId,
        #[ink(topic)]
        recipient: AccountId,
        amount0: u128,
        amount1: u128,
    }

    /// @notice Emitted when the owner of the factory is changed
    /// @param oldOwner The owner before the owner was changed
    /// @param newOwner The owner after the owner was changed
    #[ink(event)]
    pub struct OwnerChanged {
        #[ink(topic)]
        pub old_owner: AccountId,
        #[ink(topic)]
        pub new_owner: AccountId,
    }
    /// @notice Emitted when a pool is created
    /// @param token0 The first token of the pool by address sort order
    /// @param token1 The second token of the pool by address sort order
    /// @param fee The fee collected upon every swap in the pool, denominated in hundredths of a bip
    /// @param tick_spacing The minimum number of ticks between initialized ticks
    /// @param pool The address of the created pool
    #[ink(event)]
    pub struct PoolCreated {
        #[ink(topic)]
        pub token0: AccountId,
        #[ink(topic)]
        pub token1: AccountId,
        #[ink(topic)]
        pub fee: u32,
        pub tick_spacing: i32,
        pub pool: AccountId,
    }
    /// @notice Emitted when a new fee amount is enabled for pool creation via the factory
    /// @param fee The enabled fee, denominated in hundredths of a bip
    /// @param tick_spacing The minimum number of ticks between initialized ticks for pools created with the given fee
    #[ink(event)]
    pub struct FeeAmountEnabled {
        #[ink(topic)]
        pub fee: u32,
        #[ink(topic)]
        pub tick_spacing: i32,
    }

    impl SubPool {
        /// Creates a new SubPool contract.
        #[ink(constructor)]
        pub fn new() -> Self {
            let (factory, token0, token1, fee, tick_spacing) =
                Self::pool_deployer_parameters(Self::env().caller()).unwrap();
            let max_liquidity_per_tick = Self::tick_spacing_to_max_liquidity_per_tick(tick_spacing);

            // This call is required in order to correctly initialize the
            // `Mapping`s of our contract.
            ink_lang::utils::initialize_contract(|contract: &mut Self| {
                contract.owner = Self::env().caller();
                contract.factory = factory;
                contract.token0 = token0;
                contract.token1 = token1;
                contract.fee = fee;
                contract.tick_spacing = tick_spacing;
                contract.max_liquidity_per_tick = max_liquidity_per_tick;
            })
        }
        fn only_factory_owner(&self) -> Result<()> {
            ensure!(
                self.env().caller() == self.factory_owner(self.factory)?,
                Error::OnlyOwner
            );
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
        #[cfg_attr(test, allow(unused_variables))]
        fn factory_owner(&self, factory: AccountId) -> Result<AccountId> {
            #[cfg(test)]
            {
                Ok(AccountId::default())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] = ink_lang::selector_bytes!("owner");
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(factory)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()))
                    .returns::<AccountId>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("factory_owner= {:?}", e);
                        Error::TransactionFailed
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
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(token)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(ExecutionInput::new(selector.into()).push_arg(owner))
                    .returns::<U256>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("erc20_balance_of= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn sub_swap_callback(
            &mut self,
            caller: AccountId,
            amount0_delta: I256,
            amount1_delta: I256,
            data: Vec<u8>,
        ) -> Result<()> {
            #[cfg(test)]
            {
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] =
                    ink_lang::selector_bytes!("isub_swap_callback::sub_swap_callback");
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(caller)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(amount0_delta)
                            .push_arg(amount1_delta)
                            .push_arg(data),
                    )
                    .returns::<()>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("sub_swap_callback= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn sub_mint_callback(
            &mut self,
            caller: AccountId,
            amount0_owed: U256,
            amount1_owed: U256,
            data: Vec<u8>,
        ) -> Result<()> {
            #[cfg(test)]
            {
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] =
                    ink_lang::selector_bytes!("isub_mint_callback::sub_mint_callback");
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(caller)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(amount0_owed)
                            .push_arg(amount1_owed)
                            .push_arg(data),
                    )
                    .returns::<()>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("sub_mint_callback= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        #[cfg_attr(test, allow(unused_variables))]
        fn sub_flash_callback(
            &mut self,
            caller: AccountId,
            fee0: U256,
            fee1: U256,
            data: Vec<u8>,
        ) -> Result<()> {
            #[cfg(test)]
            {
                Ok(())
            }
            #[cfg(not(test))]
            {
                use ink_env::call::{build_call, Call, ExecutionInput};
                let selector: [u8; 4] =
                    ink_lang::selector_bytes!("isub_flash_callback::sub_flash_callback");
                let (gas_limit, transferred_value) = (0, 0);
                build_call::<<Self as ::ink_lang::reflect::ContractEnv>::Env>()
                    .call_type(
                        Call::new()
                            .callee(caller)
                            .gas_limit(gas_limit)
                            .transferred_value(transferred_value),
                    )
                    .exec_input(
                        ExecutionInput::new(selector.into())
                            .push_arg(fee0)
                            .push_arg(fee1)
                            .push_arg(data),
                    )
                    .returns::<()>()
                    .fire()
                    .map_err(|e| {
                        ink_env::debug_println!("sub_flash_callback= {:?}", e);
                        Error::TransactionFailed
                    })
            }
        }
        /// @dev Get the pool's balance of token0
        /// @dev This function is gas optimized to avoid a redundant extcodesize check in addition to the returndatasize
        /// check
        fn balance0(&self) -> Result<U256> {
            self.erc20_balance_of(self.token0, self.env().account_id())
        }

        /// @dev Get the pool's balance of token1
        /// @dev This function is gas optimized to avoid a redundant extcodesize check in addition to the returndatasize
        /// check
        fn balance1(&self) -> Result<U256> {
            self.erc20_balance_of(self.token1, self.env().account_id())
        }

        /// @inheritdoc IUniswapV3PoolDerivedState
        #[ink(message)]
        pub fn snapshot_cumulatives_inside(
            &self,
            tick_lower: i32,
            tick_upper: i32,
        ) -> Result<(i64, U160, u32)> {
            self.check_ticks(tick_lower, tick_upper)?;

            let lower = self.ticks.get(tick_lower).unwrap_or_default();
            let upper = self.ticks.get(tick_upper).unwrap_or_default();
            let (
                tick_cumulative_lower,
                seconds_per_liquidity_outside_lower_x128,
                seconds_outside_lower,
                initialized_lower,
            ) = (
                lower.tick_cumulative_outside,
                lower.seconds_per_liquidity_outside_x128,
                lower.seconds_outside,
                lower.initialized,
            );
            ensure!(initialized_lower, Error::UninitializedLower);

            let (
                tick_cumulative_upper,
                seconds_per_liquidity_outside_upper_x128,
                seconds_outside_upper,
                initialized_upper,
            ) = (
                upper.tick_cumulative_outside,
                upper.seconds_per_liquidity_outside_x128,
                upper.seconds_outside,
                upper.initialized,
            );
            ensure!(initialized_upper, Error::UninitializedUpper);

            Ok(if self.slot0.tick < tick_lower {
                (
                    tick_cumulative_lower - tick_cumulative_upper,
                    seconds_per_liquidity_outside_lower_x128
                        - seconds_per_liquidity_outside_upper_x128,
                    seconds_outside_lower - seconds_outside_upper,
                )
            } else if self.slot0.tick < tick_upper {
                let time = self.block_timestamp();
                let (tick_cumulative, seconds_per_liquidity_cumulative_x128) = self
                    .observe_single(
                        time,
                        0,
                        self.slot0.tick,
                        self.slot0.observation_index,
                        self.liquidity,
                        self.slot0.observation_cardinality,
                    )?;
                (
                    tick_cumulative - tick_cumulative_lower - tick_cumulative_upper,
                    seconds_per_liquidity_cumulative_x128
                        - seconds_per_liquidity_outside_lower_x128
                        - seconds_per_liquidity_outside_upper_x128,
                    time - seconds_outside_lower - seconds_outside_upper,
                )
            } else {
                (
                    tick_cumulative_upper - tick_cumulative_lower,
                    seconds_per_liquidity_outside_upper_x128
                        - seconds_per_liquidity_outside_lower_x128,
                    seconds_outside_upper - seconds_outside_lower,
                )
            })
        }
        fn no_delegate_call(&self) -> Result<()> {
            ensure!(
                self.env().account_id() == self.original,
                Error::OnlyOriginal
            );
            Ok(())
        }
        /// @inheritdoc IUniswapV3PoolDerivedState
        #[ink(message)]
        pub fn observe(&self, seconds_agos: Vec<u32>) -> Result<(Vec<i64>, Vec<U160>)> {
            ensure!(
                self.env().account_id() == self.original,
                Error::OnlyOriginal
            );

            self.oracle_observe(
                self.block_timestamp(),
                seconds_agos,
                self.slot0.tick,
                self.slot0.observation_index,
                self.liquidity,
                self.slot0.observation_cardinality,
            )
        }

        /// @inheritdoc IUniswapV3PoolActions
        #[ink(message)]
        pub fn increase_observation_cardinality_next(
            &mut self,
            observation_cardinality_next: u16,
        ) -> Result<()> {
            ensure!(
                self.env().account_id() == self.original,
                Error::OnlyOriginal
            );

            let observation_cardinality_next_old = self.slot0.observation_cardinality_next; // for the event
            let observation_cardinality_next_new = self.grow(
                observation_cardinality_next_old,
                observation_cardinality_next,
            )?;
            self.slot0.observation_cardinality_next = observation_cardinality_next_new;
            if observation_cardinality_next_old != observation_cardinality_next_new {
                self.env().emit_event(IncreaseObservationCardinalityNext {
                    observation_cardinality_next_old,
                    observation_cardinality_next_new,
                });
            }
            Ok(())
        }

        /// @inheritdoc IUniswapV3PoolActions
        /// @dev not locked because it initializes unlocked
        #[ink(message)]
        pub fn initialize(&mut self, sqrt_price_x96: U160) -> Result<()> {
            ensure!(self.slot0.sqrt_price_x96 == U160::zero(), Error::AI);

            let tick = TickMath::get_tick_at_sqrt_ratio(sqrt_price_x96);

            let (cardinality, cardinality_next) = self.oracle_initialize(self.block_timestamp());

            self.slot0 = Slot0 {
                sqrt_price_x96,
                tick,
                observation_index: 0,
                observation_cardinality: cardinality,
                observation_cardinality_next: cardinality_next,
                fee_protocol: 0,
                unlocked: true,
            };

            self.env().emit_event(Initialize {
                sqrt_price_x96,
                tick,
            });
            Ok(())
        }

        /// @dev Effect some changes to a position
        /// @param params the position details and the change to the position's liquidity to effect
        /// @return position a storage pointer referencing the position with the given owner and tick range
        /// @return amount0 the amount of token0 owed to the pool, negative if the pool should pay the recipient
        /// @return amount1 the amount of token1 owed to the pool, negative if the pool should pay the recipient
        fn _modify_position(
            &mut self,
            params: ModifyPositionParams,
        ) -> Result<(PositionInfo, I256, I256)> {
            self.no_delegate_call()?;
            self.check_ticks(params.tick_lower, params.tick_upper)?;

            let position = self._update_position(
                params.owner,
                params.tick_lower,
                params.tick_upper,
                params.liquidity_delta,
                self.slot0.tick,
            )?;
            let (mut amount0, mut amount1) = (I256::zero(), I256::zero());
            if params.liquidity_delta != 0 {
                if self.slot0.tick < params.tick_lower {
                    // current tick is below the passed range; liquidity can only become in range by crossing from left to
                    // right, when we'll need _more_ token0 (it's becoming more valuable) so user must provide it
                    amount0 = SqrtPriceMath::get_amount0_delta(
                        TickMath::get_sqrt_ratio_at_tick(params.tick_lower),
                        TickMath::get_sqrt_ratio_at_tick(params.tick_upper),
                        params.liquidity_delta,
                    );
                } else if self.slot0.tick < params.tick_upper {
                    // current tick is inside the passed range
                    let liquidity_before = self.liquidity; // SLOAD for gas optimization

                    // write an oracle entry
                    let (observation_index, observation_cardinality) = self.write(
                        self.slot0.observation_index,
                        self.block_timestamp(),
                        self.slot0.tick,
                        liquidity_before,
                        self.slot0.observation_cardinality,
                        self.slot0.observation_cardinality_next,
                    );
                    self.slot0.observation_index = observation_index;
                    self.slot0.observation_cardinality = observation_cardinality;
                    amount0 = SqrtPriceMath::get_amount0_delta(
                        self.slot0.sqrt_price_x96,
                        TickMath::get_sqrt_ratio_at_tick(params.tick_upper),
                        params.liquidity_delta,
                    );
                    amount1 = SqrtPriceMath::get_amount1_delta(
                        TickMath::get_sqrt_ratio_at_tick(params.tick_lower),
                        self.slot0.sqrt_price_x96,
                        params.liquidity_delta,
                    );

                    self.liquidity =
                        LiquidityMath::add_delta(liquidity_before, params.liquidity_delta);
                } else {
                    // current tick is above the passed range; liquidity can only become in range by crossing from right to
                    // left, when we'll need _more_ token1 (it's becoming more valuable) so user must provide it
                    amount1 = SqrtPriceMath::get_amount1_delta(
                        TickMath::get_sqrt_ratio_at_tick(params.tick_lower),
                        TickMath::get_sqrt_ratio_at_tick(params.tick_upper),
                        params.liquidity_delta,
                    );
                }
            }
            Ok((position, amount0, amount1))
        }

        /// @dev Gets and updates a position with the given liquidity delta
        /// @param owner the owner of the position
        /// @param tick_lower the lower tick of the position's tick range
        /// @param tick_upper the upper tick of the position's tick range
        /// @param tick the current tick, passed to avoid sloads
        fn _update_position(
            &mut self,
            owner: AccountId,
            tick_lower: i32,
            tick_upper: i32,
            liquidity_delta: i128,
            tick: i32,
        ) -> Result<PositionInfo> {
            let position = self
                .positions
                .get(&(owner, tick_lower, tick_upper))
                .unwrap_or_default();

            let _fee_growth_global0_x128 = self.fee_growth_global0_x128; // SLOAD for gas optimization
            let _fee_growth_global1_x128 = self.fee_growth_global1_x128; // SLOAD for gas optimization

            // if we need to update the ticks, do it
            let mut flipped_lower = false;
            let mut flipped_upper = false;
            if liquidity_delta != 0 {
                let time = self.block_timestamp();
                let (tick_cumulative, seconds_per_liquidity_cumulative_x128) = self
                    .observe_single(
                        time,
                        0,
                        self.slot0.tick,
                        self.slot0.observation_index,
                        self.liquidity,
                        self.slot0.observation_cardinality,
                    )?;

                flipped_lower = self.tick_update(
                    tick_lower,
                    tick,
                    liquidity_delta,
                    _fee_growth_global0_x128,
                    _fee_growth_global1_x128,
                    seconds_per_liquidity_cumulative_x128,
                    tick_cumulative,
                    time,
                    false,
                    self.max_liquidity_per_tick,
                )?;
                flipped_upper = self.tick_update(
                    tick_upper,
                    tick,
                    liquidity_delta,
                    _fee_growth_global0_x128,
                    _fee_growth_global1_x128,
                    seconds_per_liquidity_cumulative_x128,
                    tick_cumulative,
                    time,
                    true,
                    self.max_liquidity_per_tick,
                )?;

                if flipped_lower {
                    self.flip_tick(tick_lower, self.tick_spacing);
                }
                if flipped_upper {
                    self.flip_tick(tick_upper, self.tick_spacing);
                }
            }

            let (fee_growth_inside0_x128, fee_growth_inside1_x128) = self.get_fee_growth_inside(
                tick_lower,
                tick_upper,
                tick,
                _fee_growth_global0_x128,
                _fee_growth_global1_x128,
            );

            self.position_update(
                owner,
                tick_lower,
                tick_upper,
                liquidity_delta,
                fee_growth_inside0_x128,
                fee_growth_inside1_x128,
            )?;

            // clear any tick data that is no longer needed
            if liquidity_delta < 0 {
                if flipped_lower {
                    self.clear(tick_lower);
                }
                if flipped_upper {
                    self.clear(tick_upper);
                }
            }
            Ok(position)
        }

        /// @inheritdoc IUniswapV3PoolActions
        /// @dev noDelegateCall is applied indirectly via _modifyPosition
        #[ink(message)]
        pub fn mint(
            &mut self,
            recipient: AccountId,
            tick_lower: i32,
            tick_upper: i32,
            amount: u128,
            data: Vec<u8>,
        ) -> Result<(U256, U256)> {
            ensure!(amount > 0, Error::AmountIsZero);
            let (_, amount0_int, amount1_int) = self._modify_position(ModifyPositionParams {
                owner: recipient,
                tick_lower,
                tick_upper,
                liquidity_delta: SafeCast::to_int128(I256::from(amount)),
            })?;

            let amount0 = U256::from(amount0_int);
            let amount1 = U256::from(amount1_int);

            let mut balance0_before = U256::zero();
            let mut balance1_before = U256::zero();
            if amount0 > U256::zero() {
                balance0_before = self.balance0()?;
            }
            if amount1 > U256::zero() {
                balance1_before = self.balance1()?;
            }
            self.sub_mint_callback(self.env().caller(), amount0, amount1, data)?;
            if amount0 > U256::zero() {
                ensure!(
                    balance0_before.saturating_add(amount0) <= self.balance0()?,
                    Error::M0
                );
            }
            if amount1 > U256::zero() {
                ensure!(
                    balance1_before.saturating_add(amount1) <= self.balance1()?,
                    Error::M1
                );
            }

            self.env().emit_event(Mint {
                sender: self.env().caller(),
                owner: recipient,
                tick_lower,
                tick_upper,
                amount,
                amount0,
                amount1,
            });
            Ok((amount0, amount1))
        }

        /// @inheritdoc IUniswapV3PoolActions
        #[ink(message)]
        pub fn collect(
            &mut self,
            recipient: AccountId,
            tick_lower: i32,
            tick_upper: i32,
            amount0_requested: u128,
            amount1_requested: u128,
        ) -> Result<(u128, u128)> {
            // we don't need to check_ticks here, because invalid positions will never have non-zero tokensOwed{0,1}
            let mut position = self
                .positions
                .get((self.env().caller(), tick_lower, tick_upper))
                .unwrap_or_default();

            let amount0 = if amount0_requested > position.tokens_owed0 {
                position.tokens_owed0
            } else {
                amount0_requested
            };
            let amount1 = if amount1_requested > position.tokens_owed1 {
                position.tokens_owed1
            } else {
                amount1_requested
            };

            if amount0 > 0 {
                position.tokens_owed0 -= amount0;
                self.safe_transfer(self.token0, recipient, U256::from(amount0))?;
            }
            if amount1 > 0 {
                position.tokens_owed1 -= amount1;
                self.safe_transfer(self.token1, recipient, U256::from(amount1))?;
            }

            self.env().emit_event(Collect {
                owner: self.env().caller(),
                recipient,
                tick_lower,
                tick_upper,
                amount0,
                amount1,
            });

            Ok((amount0, amount1))
        }

        /// @inheritdoc IUniswapV3PoolActions
        /// @dev noDelegateCall is applied indirectly via _modifyPosition
        #[ink(message)]
        pub fn burn(
            &mut self,
            tick_lower: i32,
            tick_upper: i32,
            amount: u128,
        ) -> Result<(U256, U256)> {
            let (mut position, amount0_int, amount1_int) =
                self._modify_position(ModifyPositionParams {
                    owner: self.env().caller(),
                    tick_lower: tick_lower,
                    tick_upper: tick_upper,
                    liquidity_delta: -SafeCast::to_int128(I256::from(amount)),
                })?;

            let amount0 = U256::from(-amount0_int);
            let amount1 = U256::from(-amount1_int);

            if amount0 > U256::zero() || amount1 > U256::zero() {
                position.tokens_owed0 += amount0.as_u128();
                position.tokens_owed1 += amount1.as_u128();
                self.positions
                    .insert(&(self.env().caller(), tick_lower, tick_upper), &position);
            }

            self.env().emit_event(Burn {
                owner: self.env().caller(),
                tick_lower,
                tick_upper,
                amount,
                amount0,
                amount1,
            });
            Ok((amount0, amount1))
        }

        /// @inheritdoc IUniswapV3PoolActions
        #[ink(message)]
        pub fn swap(
            &mut self,
            recipient: AccountId,
            zero_for_one: bool,
            amount_specified: I256,
            sqrt_price_limit_x96: U160,
            data: Vec<u8>,
        ) -> Result<(I256, I256)> {
            self.no_delegate_call()?;
            ensure!(amount_specified != I256::zero(), Error::AS);

            ensure!(self.slot0.unlocked, Error::LOK);
            ensure!(
                if zero_for_one {
                    sqrt_price_limit_x96 < self.slot0.sqrt_price_x96
                        && sqrt_price_limit_x96 > TickMath::min_sqrt_ratio()
                } else {
                    sqrt_price_limit_x96 > self.slot0.sqrt_price_x96
                        && sqrt_price_limit_x96 < TickMath::max_sqrt_ratio()
                },
                Error::SPL
            );

            self.slot0.unlocked = false;

            let mut cache = SwapCache {
                liquidity_start: self.liquidity,
                block_timestamp: self.block_timestamp(),
                fee_protocol: if zero_for_one {
                    self.slot0.fee_protocol % 16
                } else {
                    self.slot0.fee_protocol >> 4
                },
                seconds_per_liquidity_cumulative_x128: U160::zero(),
                tick_cumulative: 0,
                computed_latest_observation: false,
            };

            let exact_input = amount_specified > I256::zero();

            let mut state = SwapState {
                amount_specified_remaining: amount_specified,
                amount_calculated: I256::zero(),
                sqrt_price_x96: self.slot0.sqrt_price_x96,
                tick: self.slot0.tick,
                fee_growth_global_x128: if zero_for_one {
                    self.fee_growth_global0_x128
                } else {
                    self.fee_growth_global1_x128
                },
                protocol_fee: 0,
                liquidity: cache.liquidity_start,
            };

            // continue swapping as long as we haven't used the entire input/output and haven't reached the price limit
            while state.amount_specified_remaining != I256::zero()
                && state.sqrt_price_x96 != sqrt_price_limit_x96
            {
                let mut step = StepComputations::default();

                step.sqrt_price_start_x96 = state.sqrt_price_x96;

                (step.tick_next, step.initialized) = self.next_initialized_tick_within_one_word(
                    state.tick,
                    self.tick_spacing,
                    zero_for_one,
                );

                // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
                if step.tick_next < TickMath::MIN_TICK {
                    step.tick_next = TickMath::MIN_TICK;
                } else if step.tick_next > TickMath::MAX_TICK {
                    step.tick_next = TickMath::MAX_TICK;
                }

                // get the price for the next tick
                step.sqrt_price_next_x96 = TickMath::get_sqrt_ratio_at_tick(step.tick_next);

                let flag = if zero_for_one {
                    step.sqrt_price_next_x96 < sqrt_price_limit_x96
                } else {
                    step.sqrt_price_next_x96 > sqrt_price_limit_x96
                };
                let next_price = if flag {
                    sqrt_price_limit_x96
                } else {
                    step.sqrt_price_next_x96
                };
                // compute values to swap to the target tick, price limit, or point where input/output amount is exhausted
                let (sqrt_price_x96, amount_in, amount_out, fee_amount) =
                    SwapMath::compute_swap_step(
                        state.sqrt_price_x96,
                        next_price,
                        state.liquidity,
                        state.amount_specified_remaining,
                        self.fee,
                    );

                state.sqrt_price_x96 = sqrt_price_x96;
                step.amount_in = amount_in;
                step.amount_out = amount_out;
                step.fee_amount = fee_amount;
                if exact_input {
                    state.amount_specified_remaining -=
                        SafeCast::to_int256(step.amount_in + step.fee_amount);
                    state.amount_calculated = state
                        .amount_calculated
                        .saturating_sub(SafeCast::to_int256(step.amount_out));
                } else {
                    state.amount_specified_remaining += SafeCast::to_int256(step.amount_out);
                    state.amount_calculated = state
                        .amount_calculated
                        .saturating_add(SafeCast::to_int256(step.amount_in + step.fee_amount));
                }

                // if the protocol fee is on, calculate how much is owed, decrement fee_amount, and increment protocol_fee
                if cache.fee_protocol > 0 {
                    let delta = step.fee_amount / U256::from(cache.fee_protocol);
                    step.fee_amount -= delta;
                    state.protocol_fee += delta.as_u128();
                }

                // update global fee tracker
                if state.liquidity > 0 {
                    state.fee_growth_global_x128 += FullMath::mul_div(
                        step.fee_amount,
                        FixedPoint128::q128(),
                        U256::from(state.liquidity),
                    );
                }
                // shift tick if we reached the next price
                if state.sqrt_price_x96 == step.sqrt_price_next_x96 {
                    // if the tick is initialized, run the tick transition
                    if step.initialized {
                        // check for the placeholder value, which we replace with the actual value the first time the swap
                        // crosses an initialized tick
                        if !cache.computed_latest_observation {
                            let (tick_cumulative, seconds_per_liquidity_cumulative_x128) = self
                                .observe_single(
                                    cache.block_timestamp,
                                    0,
                                    self.slot0.tick,
                                    self.slot0.observation_index,
                                    cache.liquidity_start,
                                    self.slot0.observation_cardinality,
                                )?;
                            cache.tick_cumulative = tick_cumulative;
                            cache.seconds_per_liquidity_cumulative_x128 =
                                seconds_per_liquidity_cumulative_x128;
                            cache.computed_latest_observation = true;
                        }
                        let mut liquidity_net = self.cross(
                            step.tick_next,
                            if zero_for_one {
                                state.fee_growth_global_x128
                            } else {
                                self.fee_growth_global0_x128
                            },
                            if zero_for_one {
                                self.fee_growth_global1_x128
                            } else {
                                state.fee_growth_global_x128
                            },
                            cache.seconds_per_liquidity_cumulative_x128,
                            cache.tick_cumulative,
                            cache.block_timestamp,
                        );
                        // if we're moving leftward, we interpret liquidity_net as the opposite sign
                        // safe because liquidity_net cannot be type(int128).min
                        if zero_for_one {
                            liquidity_net = -liquidity_net;
                        }

                        state.liquidity = LiquidityMath::add_delta(state.liquidity, liquidity_net);
                    }

                    state.tick = if zero_for_one {
                        step.tick_next - 1
                    } else {
                        step.tick_next
                    };
                } else if state.sqrt_price_x96 != step.sqrt_price_start_x96 {
                    // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved
                    state.tick = TickMath::get_tick_at_sqrt_ratio(state.sqrt_price_x96);
                }
            }

            // update tick and write an oracle entry if the tick change
            if state.tick != self.slot0.tick {
                let (observation_index, observation_cardinality) = self.write(
                    self.slot0.observation_index,
                    cache.block_timestamp,
                    self.slot0.tick,
                    cache.liquidity_start,
                    self.slot0.observation_cardinality,
                    self.slot0.observation_cardinality_next,
                );
                let (sqrt_price_x96, tick, observation_index, observation_cardinality) = (
                    state.sqrt_price_x96,
                    state.tick,
                    observation_index,
                    observation_cardinality,
                );
                self.slot0.sqrt_price_x96 = sqrt_price_x96;
                self.slot0.tick = tick;
                self.slot0.observation_index = observation_index;
                self.slot0.observation_cardinality = observation_cardinality;
            } else {
                // otherwise just update the price
                self.slot0.sqrt_price_x96 = state.sqrt_price_x96;
            }

            // update liquidity if it changed
            if cache.liquidity_start != state.liquidity {
                self.liquidity = state.liquidity;
            }

            // update fee growth global and, if necessary, protocol fees
            // overflow is acceptable, protocol has to withdraw before it hits type(uint128).max fees
            if zero_for_one {
                self.fee_growth_global0_x128 = state.fee_growth_global_x128;
                if state.protocol_fee > 0 {
                    self.protocol_fees.token0 += state.protocol_fee;
                }
            } else {
                self.fee_growth_global1_x128 = state.fee_growth_global_x128;
                if state.protocol_fee > 0 {
                    self.protocol_fees.token1 += state.protocol_fee;
                }
            }

            let (amount0, amount1) = if zero_for_one == exact_input {
                (
                    amount_specified - state.amount_specified_remaining,
                    state.amount_calculated,
                )
            } else {
                (
                    state.amount_calculated,
                    amount_specified - state.amount_specified_remaining,
                )
            };

            // do the transfers and collect payment
            if zero_for_one {
                if amount1 < I256::zero() {
                    self.safe_transfer(self.token1, recipient, U256::from(-amount1))?;
                }

                let balance0_before = self.balance0()?;

                self.sub_swap_callback(self.env().caller(), amount0, amount1, data)?;
                ensure!(
                    balance0_before.saturating_add(U256::from(amount0)) <= self.balance0()?,
                    Error::IIA
                );
            } else {
                if amount0 < I256::zero() {
                    self.safe_transfer(self.token0, recipient, U256::from(-amount0))?;
                }

                let balance1_before = self.balance1()?;
                self.sub_swap_callback(self.env().caller(), amount0, amount1, data)?;
                ensure!(
                    balance1_before.saturating_add(U256::from(amount1)) <= self.balance1()?,
                    Error::IIA
                );
            }

            self.env().emit_event(Swap {
                sender: self.env().caller(),
                recipient,
                amount0,
                amount1,
                sqrt_price_x96: state.sqrt_price_x96,
                liquidity: state.liquidity,
                tick: state.tick,
            });
            self.slot0.unlocked = true;
            Ok((amount0, amount1))
        }

        /// @inheritdoc IUniswapV3PoolActions
        #[ink(message)]
        pub fn flash(
            &mut self,
            recipient: AccountId,
            amount0: U256,
            amount1: U256,
            data: Vec<u8>,
        ) -> Result<()> {
            self.no_delegate_call()?;
            let _liquidity = self.liquidity;
            ensure!(_liquidity > 0, Error::L);

            let fee0 =
                FullMath::mul_div_rounding_up(amount0, U256::from(self.fee), U256::from(1_000_000));
            let fee1 =
                FullMath::mul_div_rounding_up(amount1, U256::from(self.fee), U256::from(1_000_000));
            let balance0_before = self.balance0()?;
            let balance1_before = self.balance1()?;

            if amount0 > U256::zero() {
                self.safe_transfer(self.token0, recipient, amount0)?;
            }
            if amount1 > U256::zero() {
                self.safe_transfer(self.token1, recipient, amount1)?;
            }

            self.sub_flash_callback(self.env().caller(), fee0, fee1, data)?;
            let balance0_after = self.balance0()?;
            let balance1_after = self.balance1()?;

            ensure!(
                balance0_before.saturating_add(fee0) <= balance0_after,
                Error::F0
            );
            ensure!(
                balance1_before.saturating_add(fee1) <= balance1_after,
                Error::F1
            );

            // sub is safe because we know balance_after is gt balance_before by at least fee
            let paid0 = balance0_after - balance0_before;
            let paid1 = balance1_after - balance1_before;

            if paid0 > U256::zero() {
                let fee_protocol0 = self.slot0.fee_protocol % 16;
                let fees0 = if fee_protocol0 == 0 {
                    U256::zero()
                } else {
                    paid0 / U256::from(fee_protocol0)
                };
                if fees0.as_u128() > 0 {
                    self.protocol_fees.token0 += fees0.as_u128();
                }
                self.fee_growth_global0_x128 +=
                    FullMath::mul_div(paid0 - fees0, FixedPoint128::q128(), U256::from(_liquidity));
            }
            if paid1 > U256::zero() {
                let fee_protocol1 = self.slot0.fee_protocol >> 4;
                let fees1 = if fee_protocol1 == 0 {
                    U256::zero()
                } else {
                    paid1 / U256::from(fee_protocol1)
                };
                if fees1.as_u128() > 0 {
                    self.protocol_fees.token1 += fees1.as_u128();
                }
                self.fee_growth_global1_x128 +=
                    FullMath::mul_div(paid1 - fees1, FixedPoint128::q128(), U256::from(_liquidity));
            }

            self.env().emit_event(Flash {
                sender: self.env().caller(),
                recipient,
                amount0,
                amount1,
                paid0,
                paid1,
            });
            Ok(())
        }

        /// @inheritdoc IUniswapV3PoolOwnerActions
        #[ink(message)]
        pub fn set_fee_protocol(&mut self, fee_protocol0: u8, fee_protocol1: u8) -> Result<()> {
            self.only_factory_owner()?;
            ensure!(
                (fee_protocol0 == 0 || (fee_protocol0 >= 4 && fee_protocol0 <= 10))
                    && (fee_protocol1 == 0 || (fee_protocol1 >= 4 && fee_protocol1 <= 10)),
                Error::FeeProocolWrong
            );
            let fee_protocol_old = self.slot0.fee_protocol;
            self.slot0.fee_protocol = fee_protocol0 + (fee_protocol1 << 4);
            self.env().emit_event(SetFeeProtocol {
                fee_protocol0_old: fee_protocol_old % 16,
                fee_protocol1_old: fee_protocol_old >> 4,
                fee_protocol0_new: fee_protocol0,
                fee_protocol1_new: fee_protocol1,
            });
            Ok(())
        }

        /// @inheritdoc IUniswapV3PoolOwnerActions
        #[ink(message)]
        pub fn collect_protocol(
            &mut self,
            recipient: AccountId,
            amount0_requested: u128,
            amount1_requested: u128,
        ) -> Result<(u128, u128)> {
            self.only_factory_owner()?;
            let mut amount0 = if amount0_requested > self.protocol_fees.token0 {
                self.protocol_fees.token0
            } else {
                amount0_requested
            };
            let mut amount1 = if amount1_requested > self.protocol_fees.token1 {
                self.protocol_fees.token1
            } else {
                amount1_requested
            };

            if amount0 > 0 {
                if amount0 == self.protocol_fees.token0 {
                    amount0 -= 1;
                } // ensure that the slot is not cleared, for gas savings
                self.protocol_fees.token0 -= amount0;
                self.safe_transfer(self.token0, recipient, U256::from(amount0))?;
            }
            if amount1 > 0 {
                if amount1 == self.protocol_fees.token1 {
                    amount1 -= 1;
                } // ensure that the slot is not cleared, for gas savings
                self.protocol_fees.token1 -= amount1;
                self.safe_transfer(self.token1, recipient, U256::from(amount1))?;
            }

            self.env().emit_event(CollectProtocol {
                sender: self.env().caller(),
                recipient,
                amount0,
                amount1,
            });
            Ok((amount0, amount1))
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

        /// Querying factory contract address
        /// # return factory contract address
        #[ink(message)]
        pub fn factory(&self) -> AccountId {
            self.factory
        }
        /// Querying token0 contract address
        /// # return token0 contract address
        #[ink(message)]
        pub fn token0(&self) -> AccountId {
            self.token0
        }

        /// Get fee
        /// # Return
        ///  fee  fee
        #[ink(message)]
        pub fn fee(&self) -> u32 {
            self.fee
        }
        /// Querying tick_spacingcontract address
        /// # return tick_spacing contract address
        #[ink(message)]
        pub fn tick_spacing(&self) -> i32 {
            self.tick_spacing
        }
        /// Get max_liquidity_per_tick
        /// # Return
        ///  max_liquidity_per_tick  max_liquidity_per_tick
        #[ink(message)]
        pub fn max_liquidity_per_tick(&self) -> u128 {
            self.max_liquidity_per_tick
        }
        /// Get slot0
        /// # Return
        ///  slot0  slot0
        #[ink(message)]
        pub fn slot0(&self) -> Slot0 {
            self.slot0.clone()
        }
        /// Get fee_growth_global0_x128
        /// # Return
        ///  fee_growth_global0_x128  fee_growth_global0_x128
        #[ink(message)]
        pub fn fee_growth_global0_x128(&self) -> U256 {
            self.fee_growth_global0_x128
        }
        /// Get fee_growth_global1_x128
        /// # Return
        ///  fee_growth_global1_x128  fee_growth_global1_x128
        #[ink(message)]
        pub fn fee_growth_global1_x128(&self) -> U256 {
            self.fee_growth_global1_x128
        }
        /// Get protocol_fees
        /// # Return
        ///  protocol_fees  protocol_fees
        #[ink(message)]
        pub fn protocol_fees(&self) -> ProtocolFees {
            self.protocol_fees.clone()
        }
        /// Get liquidity
        /// # Return
        ///  liquidity  liquidity
        #[ink(message)]
        pub fn liquidity(&self) -> u128 {
            self.liquidity
        }
        /// Get owner
        /// # Return
        ///  owner  owner
        #[ink(message)]
        pub fn owner(&self) -> AccountId {
            self.owner
        }
        #[ink(message)]
        pub fn tick_of(&self, tick: i32) -> TickInfo {
            self.ticks.get(&tick).unwrap_or_default()
        }
        #[ink(message)]
        pub fn tick_bitmap_of(&self, tick: i16) -> U256 {
            self.tick_bitmap.get(tick).unwrap_or_default()
        }
        #[ink(message)]
        pub fn position_of(&self, position: (AccountId, i32, i32)) -> PositionInfo {
            self.positions.get(&position).unwrap_or_default()
        }
        #[ink(message)]
        pub fn observation_of(&self, index: u32) -> Observation {
            self.observations[index as usize].clone()
        }
    }

    /// @title Oracle
    /// @notice Provides price and liquidity data useful for a wide variety of system designs
    /// @dev Instances of stored oracle data, "observations", are collected in the oracle array
    /// Every pool is initialized with an oracle array length of 1. Anyone can pay the SSTOREs to increase the
    /// maximum length of the oracle array. New slots will be added when the array is fully populated.
    /// Observations are overwritten when the full length of the oracle array is populated.
    /// The most recent observation is available, independent of the length of the oracle array, by passing 0 to observe()

    #[ink(impl)]
    impl SubPool {
        /// @notice Transforms a previous observation into a new observation, given the passage of time and the current tick and liquidity values
        /// @dev block_timestamp _must_ be chronologically equal to or greater than last.block_timestamp, safe for 0 or 1 overflows
        /// @param last The specified observation to be transformed
        /// @param block_timestamp The timestamp of the new observation
        /// @param tick The active tick at the time of the new observation
        /// @param liquidity The total in-range liquidity at the time of the new observation
        /// @return Observation The newly populated observation
        fn transform(
            last: &Observation,
            block_timestamp: u32,
            tick: i32,
            liquidity: u128,
        ) -> Observation {
            let delta = block_timestamp - last.block_timestamp;

            Observation {
                block_timestamp: block_timestamp,
                tick_cumulative: last.tick_cumulative + (tick as i64) * delta as i64,
                seconds_per_liquidity_cumulative_x128: last.seconds_per_liquidity_cumulative_x128
                    + ((U160::from(delta) << U160::from(64))
                        / (if liquidity > 0 {
                            U160::from(liquidity)
                        } else {
                            U160::one()
                        })),
                initialized: true,
            }
        }

        /// @notice Initialize the oracle array by writing the first slot. Called once for the lifecycle of the observations array
        /// @param self The stored oracle array
        /// @param time The time of the oracle initialization, via block.timestamp truncated to uint32
        /// @return cardinality The number of populated elements in the oracle array
        /// @return cardinalityNext The new length of the oracle array, independent of population
        fn oracle_initialize(&mut self, time: u32) -> (u16, u16) {
            self.observations[0] = Observation {
                block_timestamp: time,
                tick_cumulative: 0,
                seconds_per_liquidity_cumulative_x128: U160::zero(),
                initialized: true,
            };
            (1, 1)
        }

        /// @notice Writes an oracle observation to the array
        /// @dev Writable at most once per block. Index represents the most recently written element. cardinality and index must be tracked externally.
        /// If the index is at the end of the allowable array length (according to cardinality), and the next cardinality
        /// is greater than the current one, cardinality may be increased. This restriction is created to preserve ordering.
        /// @param self The stored oracle array
        /// @param index The index of the observation that was most recently written to the observations array
        /// @param block_timestamp The timestamp of the new observation
        /// @param tick The active tick at the time of the new observation
        /// @param liquidity The total in-range liquidity at the time of the new observation
        /// @param cardinality The number of populated elements in the oracle array
        /// @param cardinalityNext The new length of the oracle array, independent of population
        /// @return indexUpdated The new index of the most recently written element in the oracle array
        /// @return cardinalityUpdated The new cardinality of the oracle array
        fn write(
            &mut self,
            index: u16,
            block_timestamp: u32,
            tick: i32,
            liquidity: u128,
            cardinality: u16,
            cardinality_next: u16,
        ) -> (u16, u16) {
            let last = &self.observations[index as usize];

            // early return if we've already written an observation this block
            if last.block_timestamp == block_timestamp {
                return (index, cardinality);
            }

            // if the conditions are right, we can bump the cardinality
            let cardinality_updated =
                if cardinality_next > cardinality && index == (cardinality - 1) {
                    cardinality_next
                } else {
                    cardinality
                };

            let index_updated = (index + 1) % cardinality_updated;
            self.observations[index_updated as usize] =
                Self::transform(last, block_timestamp, tick, liquidity);
            (index_updated, cardinality_updated)
        }

        /// @notice Prepares the oracle array to store up to `next` observations
        /// @param self The stored oracle array
        /// @param current The current next cardinality of the oracle array
        /// @param next The proposed next cardinality which will be populated in the oracle array
        /// @return next The next cardinality which will be populated in the oracle array
        fn grow(&mut self, current: u16, next: u16) -> Result<u16> {
            ensure!(current > 0, Error::I);
            // no-op if the passed next value isn't greater than the current next value
            if next <= current {
                return Ok(current);
            }
            // store in each slot to prevent fresh SSTOREs in swaps
            // this data will not be used because the initialized boolean is still false
            for i in current..next {
                self.observations[i as usize].block_timestamp = 1;
            }
            Ok(next)
        }

        /// @notice comparator for 32-bit timestamps
        /// @dev safe for 0 or 1 overflows, a and b _must_ be chronologically before or equal to time
        /// @param time A timestamp truncated to 32 bits
        /// @param a A comparison timestamp from which to determine the relative position of `time`
        /// @param b From which to determine the relative position of `time`
        /// @return bool Whether `a` is chronologically <= `b`
        fn lte(time: u32, a: u32, b: u32) -> bool {
            // if there hasn't been overflow, no need to adjust
            if a <= time && b <= time {
                return a <= b;
            }
            let (time, a, b) = (time as u64, a as u64, b as u64);
            let a_adjusted = if a > time { a } else { a + (1 << 32) };
            let b_adjusted = if b > time { b } else { b + (1 << 32) };

            a_adjusted <= b_adjusted
        }

        /// @notice Fetches the observations beforeOrAt and atOrAfter a target, i.e. where [beforeOrAt, atOrAfter] is satisfied.
        /// The result may be the same observation, or adjacent observations.
        /// @dev The answer must be contained in the array, used when the target is located within the stored observation
        /// boundaries: older than the most recent observation and younger, or the same age as, the oldest observation
        /// @param self The stored oracle array
        /// @param time The current block.timestamp
        /// @param target The timestamp at which the reserved observation should be for
        /// @param index The index of the observation that was most recently written to the observations array
        /// @param cardinality The number of populated elements in the oracle array
        /// @return beforeOrAt The observation recorded before, or at, the target
        /// @return atOrAfter The observation recorded at, or after, the target
        fn binary_search(
            &self,
            time: u32,
            target: u32,
            index: u16,
            cardinality: u16,
        ) -> (Observation, Observation) {
            let mut l = (index + 1) % cardinality; // oldest observation
            let mut r = l + cardinality - 1; // newest observation
            let mut i;
            let (mut before_or_at, mut at_or_after); // = (Observation::default(), Observation::default());
            loop {
                i = (l + r) / 2;

                before_or_at = self.observations[(i % cardinality) as usize].clone();

                // we've landed on an uninitialized tick, keep searching higher (more recently)
                if !before_or_at.initialized {
                    l = i + 1;
                    continue;
                }

                at_or_after = self.observations[((i + 1) % cardinality) as usize].clone();

                let target_at_or_after = Self::lte(time, before_or_at.block_timestamp, target);

                // check if we've found the answer!
                if target_at_or_after && Self::lte(time, target, at_or_after.block_timestamp) {
                    break;
                }

                if !target_at_or_after {
                    r = i - 1;
                } else {
                    l = i + 1;
                }
            }
            (before_or_at, at_or_after)
        }

        /// @notice Fetches the observations beforeOrAt and atOrAfter a given target, i.e. where [beforeOrAt, atOrAfter] is satisfied
        /// @dev Assumes there is at least 1 initialized observation.
        /// Used by observe_single() to compute the counterfactual accumulator values as of a given block timestamp.
        /// @param self The stored oracle array
        /// @param time The current block.timestamp
        /// @param target The timestamp at which the reserved observation should be for
        /// @param tick The active tick at the time of the returned or simulated observation
        /// @param index The index of the observation that was most recently written to the observations array
        /// @param liquidity The total pool liquidity at the time of the call
        /// @param cardinality The number of populated elements in the oracle array
        /// @return beforeOrAt The observation which occurred at, or before, the given timestamp
        /// @return atOrAfter The observation which occurred at, or after, the given timestamp
        fn get_surrounding_observations(
            &self,
            time: u32,
            target: u32,
            tick: i32,
            index: u16,
            liquidity: u128,
            cardinality: u16,
        ) -> Result<(Observation, Observation)> {
            // optimistically set before to the newest observation
            let mut before_or_at = self.observations[index as usize].clone();

            // if the target is chronologically at or after the newest observation, we can early return
            if Self::lte(time, before_or_at.block_timestamp, target) {
                return Ok(if before_or_at.block_timestamp == target {
                    // if newest observation equals target, we're in the same block, so we can ignore at_or_after
                    (before_or_at, Observation::default())
                } else {
                    // otherwise, we need to transform
                    let at_or_after = Self::transform(&before_or_at, target, tick, liquidity);
                    (before_or_at, at_or_after)
                });
            }

            // now, set before to the oldest observation
            before_or_at = self.observations[((index + 1) % cardinality) as usize].clone();
            if !before_or_at.initialized {
                before_or_at = self.observations[0].clone();
            }

            // ensure that the target is chronologically at or after the oldest observation
            ensure!(
                Self::lte(time, before_or_at.block_timestamp, target),
                Error::OLD
            );

            // if we've reached this point, we have to binary search
            Ok(self.binary_search(time, target, index, cardinality))
        }

        /// @dev Reverts if an observation at or before the desired observation timestamp does not exist.
        /// 0 may be passed as `secondsAgo' to return the current cumulative values.
        /// If called with a timestamp falling between two observations, returns the counterfactual accumulator values
        /// at exactly the timestamp between the two observations.
        /// @param self The stored oracle array
        /// @param time The current block timestamp
        /// @param secondsAgo The amount of time to look back, in seconds, at which point to return an observation
        /// @param tick The current tick
        /// @param index The index of the observation that was most recently written to the observations array
        /// @param liquidity The current in-range pool liquidity
        /// @param cardinality The number of populated elements in the oracle array
        /// @return tick_cumulative The tick * time elapsed since the pool was first initialized, as of `secondsAgo`
        /// @return seconds_per_liquidity_cumulative_x128 The time elapsed / max(1, liquidity) since the pool was first initialized, as of `secondsAgo`
        fn observe_single(
            &self,
            time: u32,
            seconds_ago: u32,
            tick: i32,
            index: u16,
            liquidity: u128,
            cardinality: u16,
        ) -> Result<(i64, U160)> {
            if seconds_ago == 0 {
                let mut last = self.observations[index as usize].clone();
                if last.block_timestamp != time {
                    last = Self::transform(&last, time, tick, liquidity);
                }
                return Ok((
                    last.tick_cumulative,
                    last.seconds_per_liquidity_cumulative_x128,
                ));
            }

            let target = time - seconds_ago;

            let (before_or_at, at_or_after) = self.get_surrounding_observations(
                time,
                target,
                tick,
                index,
                liquidity,
                cardinality,
            )?;

            Ok(if target == before_or_at.block_timestamp {
                // we're at the left boundary
                (
                    before_or_at.tick_cumulative,
                    before_or_at.seconds_per_liquidity_cumulative_x128,
                )
            } else if target == at_or_after.block_timestamp {
                // we're at the right boundary
                (
                    at_or_after.tick_cumulative,
                    at_or_after.seconds_per_liquidity_cumulative_x128,
                )
            } else {
                // we're in the middle
                let observation_time_delta =
                    at_or_after.block_timestamp - before_or_at.block_timestamp;
                let target_delta = target - before_or_at.block_timestamp;
                (
                    before_or_at.tick_cumulative
                        + ((at_or_after.tick_cumulative - before_or_at.tick_cumulative)
                            / observation_time_delta as i64)
                            * target_delta as i64,
                    before_or_at.seconds_per_liquidity_cumulative_x128
                        + U160::from(
                            (U256::from(
                                at_or_after.seconds_per_liquidity_cumulative_x128
                                    - before_or_at.seconds_per_liquidity_cumulative_x128,
                            ) * U256::from(target_delta))
                                / U256::from(observation_time_delta),
                        ),
                )
            })
        }

        /// @notice Returns the accumulator values as of each time seconds ago from the given time in the array of `seconds_agos`
        /// @dev Reverts if `seconds_agos` > oldest observation
        /// @param self The stored oracle array
        /// @param time The current block.timestamp
        /// @param seconds_agos Each amount of time to look back, in seconds, at which point to return an observation
        /// @param tick The current tick
        /// @param index The index of the observation that was most recently written to the observations array
        /// @param liquidity The current in-range pool liquidity
        /// @param cardinality The number of populated elements in the oracle array
        /// @return tickCumulatives The tick * time elapsed since the pool was first initialized, as of each `secondsAgo`
        /// @return secondsPerLiquidityCumulative_x128s The cumulative seconds / max(1, liquidity) since the pool was first initialized, as of each `secondsAgo`
        fn oracle_observe(
            &self,
            time: u32,
            seconds_agos: Vec<u32>,
            tick: i32,
            index: u16,
            liquidity: u128,
            cardinality: u16,
        ) -> Result<(Vec<i64>, Vec<U160>)> {
            ensure!(cardinality > 0, Error::I);
            let n = seconds_agos.len();
            let mut tick_cumulatives = vec![0; n];
            let mut seconds_per_liquidity_cumulative_x128s = vec![U160::zero(); n];
            for i in 0..seconds_agos.len() {
                let (tick_cumulative, seconds_per_liquidity_cumulative_x128) = self
                    .observe_single(time, seconds_agos[i], tick, index, liquidity, cardinality)?;
                tick_cumulatives[i] = tick_cumulative;
                seconds_per_liquidity_cumulative_x128s[i] = seconds_per_liquidity_cumulative_x128;
            }
            Ok((tick_cumulatives, seconds_per_liquidity_cumulative_x128s))
        }
    }

    /// @title Position
    /// @notice Positions represent an owner address' liquidity between a lower and upper tick boundary
    /// @dev Positions store additional state for tracking fees owed to the position
    #[ink(impl)]
    impl SubPool {
        /// @notice Returns the Info struct of a position, given an owner and position boundaries
        /// @param self The mapping containing all user positions
        /// @param owner The address of the position owner
        /// @param tickLower The lower tick boundary of the position
        /// @param tickUpper The upper tick boundary of the position
        /// @return position The position info struct of the given owners' position
        #[cfg_attr(test, allow(unused_variables))]
        fn get(&self, owner: AccountId, tick_lower: i32, tick_upper: i32) -> PositionInfo {
            self.positions
                .get(&(owner, tick_lower, tick_upper))
                .unwrap_or_default()
        }

        /// @notice Credits accumulated fees to a user's position
        /// @param self The individual position to update
        /// @param liquidity_delta The change in pool liquidity as a result of the position update
        /// @param feeGrowthInside0_x128 The all-time fee growth in token0, per unit of liquidity, inside the position's tick boundaries
        /// @param feeGrowthInside1_x128 The all-time fee growth in token1, per unit of liquidity, inside the position's tick boundaries
        fn position_update(
            &mut self,
            owner: AccountId,
            tick_lower: i32,
            tick_upper: i32,
            liquidity_delta: i128,
            fee_growth_inside0_x128: U256,
            fee_growth_inside1_x128: U256,
        ) -> Result<()> {
            let mut position = self
                .positions
                .get(&(owner, tick_lower, tick_upper))
                .unwrap_or_default();

            let liquidity_next;
            if liquidity_delta == 0 {
                ensure!(position.liquidity > 0, Error::NP); // disallow pokes for 0 liquidity positions
                liquidity_next = position.liquidity;
            } else {
                liquidity_next = LiquidityMath::add_delta(position.liquidity, liquidity_delta);
            }

            // calculate accumulated fees
            let tokens_owed0 = FullMath::mul_div(
                fee_growth_inside0_x128 - position.fee_growth_inside0_last_x128,
                U256::from(position.liquidity),
                FixedPoint128::q128(),
            )
            .as_u128();
            let tokens_owed1 = FullMath::mul_div(
                fee_growth_inside1_x128 - position.fee_growth_inside1_last_x128,
                U256::from(position.liquidity),
                FixedPoint128::q128(),
            )
            .as_u128();

            // update the position
            if liquidity_delta != 0 {
                position.liquidity = liquidity_next;
            }
            position.fee_growth_inside0_last_x128 = fee_growth_inside0_x128;
            position.fee_growth_inside1_last_x128 = fee_growth_inside1_x128;
            if tokens_owed0 > 0 || tokens_owed1 > 0 {
                // overflow is acceptable, have to withdraw before you hit type(uint128).max fees
                position.tokens_owed0 += tokens_owed0;
                position.tokens_owed1 += tokens_owed1;
            }
            self.positions
                .insert(&(owner, tick_lower, tick_upper), &position);
            Ok(())
        }
    }

    /// @title Tick
    /// @notice Contains functions for managing tick processes and relevant calculations
    #[ink(impl)]
    impl SubPool {
        /// @notice Derives max liquidity per tick from given tick spacing
        /// @dev Executed within the pool constructor
        /// @param tick_spacing The amount of required tick separation, realized in multiples of `tick_spacing`
        ///     e.g., a tick_spacing of 3 requires ticks to be initialized every 3rd tick i.e., ..., -6, -3, 0, 3, 6, ...
        /// @return The max liquidity per tick
        fn tick_spacing_to_max_liquidity_per_tick(tick_spacing: i32) -> u128 {
            let min_tick = (TickMath::MIN_TICK / tick_spacing) * tick_spacing;
            let max_tick = (TickMath::MAX_TICK / tick_spacing) * tick_spacing;
            let num_ticks = ((max_tick - min_tick) / tick_spacing) as u32 + 1;
            u128::MAX / num_ticks as u128
        }

        /// @notice Retrieves fee growth data
        /// @param self The mapping containing all tick information for initialized ticks
        /// @param tickLower The lower tick boundary of the position
        /// @param tickUpper The upper tick boundary of the position
        /// @param tickCurrent The current tick
        /// @param feeGrowthGlobal0_x128 The all-time global fee growth, per unit of liquidity, in token0
        /// @param feeGrowthGlobal1_x128 The all-time global fee growth, per unit of liquidity, in token1
        /// @return feeGrowthInside0_x128 The all-time fee growth in token0, per unit of liquidity, inside the position's tick boundaries
        /// @return feeGrowthInside1_x128 The all-time fee growth in token1, per unit of liquidity, inside the position's tick boundaries
        fn get_fee_growth_inside(
            &self,
            tick_lower: i32,
            tick_upper: i32,
            tick_current: i32,
            fee_growth_global0_x128: U256,
            fee_growth_global1_x128: U256,
        ) -> (U256, U256) {
            let lower = self.ticks.get(&tick_lower).unwrap_or_default();
            let upper = self.ticks.get(&tick_upper).unwrap_or_default();

            // calculate fee growth below
            let (fee_growth_below0_x128, fee_growth_below1_x128) = if tick_current >= tick_lower {
                (
                    lower.fee_growth_outside0_x128,
                    lower.fee_growth_outside1_x128,
                )
            } else {
                (
                    fee_growth_global0_x128 - lower.fee_growth_outside0_x128,
                    fee_growth_global1_x128 - lower.fee_growth_outside1_x128,
                )
            };

            // calculate fee growth above
            let (fee_growth_above0_x128, fee_growth_above1_x128) = if tick_current < tick_upper {
                (
                    upper.fee_growth_outside0_x128,
                    upper.fee_growth_outside1_x128,
                )
            } else {
                (
                    fee_growth_global0_x128 - upper.fee_growth_outside0_x128,
                    fee_growth_global1_x128 - upper.fee_growth_outside1_x128,
                )
            };

            (
                fee_growth_global0_x128 - fee_growth_below0_x128 - fee_growth_above0_x128,
                fee_growth_global1_x128 - fee_growth_below1_x128 - fee_growth_above1_x128,
            )
        }

        /// @notice Updates a tick and returns true if the tick was flipped from initialized to uninitialized, or vice versa
        /// @param self The mapping containing all tick information for initialized ticks
        /// @param tick The tick that will be updated
        /// @param tickCurrent The current tick
        /// @param liquidity_delta A new amount of liquidity to be added (subtracted) when tick is crossed from left to right (right to left)
        /// @param feeGrowthGlobal0_x128 The all-time global fee growth, per unit of liquidity, in token0
        /// @param feeGrowthGlobal1_x128 The all-time global fee growth, per unit of liquidity, in token1
        /// @param seconds_per_liquidity_cumulative_x128 The all-time seconds per max(1, liquidity) of the pool
        /// @param tick_cumulative The tick * time elapsed since the pool was first initialized
        /// @param time The current block timestamp cast to a uint32
        /// @param upper true for updating a position's upper tick, or false for updating a position's lower tick
        /// @param maxLiquidity The maximum liquidity allocation for a single tick
        /// @return flipped Whether the tick was flipped from initialized to uninitialized, or vice versa
        fn tick_update(
            &mut self,
            tick: i32,
            tick_current: i32,
            liquidity_delta: i128,
            fee_growth_global0_x128: U256,
            fee_growth_global1_x128: U256,
            seconds_per_liquidity_cumulative_x128: U160,
            tick_cumulative: i64,
            time: u32,
            upper: bool,
            max_liquidity: u128,
        ) -> Result<bool> {
            let mut info = self.ticks.get(&tick).unwrap_or_default();

            let liquidity_gross_before = info.liquidity_gross;
            let liquidity_gross_after =
                LiquidityMath::add_delta(liquidity_gross_before, liquidity_delta);

            ensure!(liquidity_gross_after <= max_liquidity, Error::LO);

            if liquidity_gross_before == 0 {
                // by convention, we assume that all growth before a tick was initialized happened _below_ the tick
                if tick <= tick_current {
                    info.fee_growth_outside0_x128 = fee_growth_global0_x128;
                    info.fee_growth_outside1_x128 = fee_growth_global1_x128;
                    info.seconds_per_liquidity_outside_x128 = seconds_per_liquidity_cumulative_x128;
                    info.tick_cumulative_outside = tick_cumulative;
                    info.seconds_outside = time;
                }
                info.initialized = true;
            }

            info.liquidity_gross = liquidity_gross_after;

            // when the lower (upper) tick is crossed left to right (right to left), liquidity must be added (removed)
            info.liquidity_net = if upper {
                SafeCast::to_int128(
                    I256::from(info.liquidity_net).saturating_sub(I256::from(liquidity_delta)),
                )
            } else {
                SafeCast::to_int128(
                    I256::from(info.liquidity_net).saturating_add(I256::from(liquidity_delta)),
                )
            };
            self.ticks.insert(tick, &info);
            Ok((liquidity_gross_after == 0) != (liquidity_gross_before == 0))
        }

        /// @notice Clears tick data
        /// @param self The mapping containing all initialized tick information for initialized ticks
        /// @param tick The tick that will be cleared
        fn clear(&mut self, tick: i32) {
            self.ticks.remove(&tick);
        }

        /// @notice Transitions to next tick as needed by price movement
        /// @param self The mapping containing all tick information for initialized ticks
        /// @param tick The destination tick of the transition
        /// @param feeGrowthGlobal0_x128 The all-time global fee growth, per unit of liquidity, in token0
        /// @param feeGrowthGlobal1_x128 The all-time global fee growth, per unit of liquidity, in token1
        /// @param seconds_per_liquidity_cumulative_x128 The current seconds per liquidity
        /// @param tick_cumulative The tick * time elapsed since the pool was first initialized
        /// @param time The current block.timestamp
        /// @return liquidity_net The amount of liquidity added (subtracted) when tick is crossed from left to right (right to left)
        fn cross(
            &mut self,
            tick: i32,
            fee_growth_global0_x128: U256,
            fee_growth_global1_x128: U256,
            seconds_per_liquidity_cumulative_x128: U160,
            tick_cumulative: i64,
            time: u32,
        ) -> i128 {
            let mut info = self.ticks.get(&tick).unwrap_or_default();
            info.fee_growth_outside0_x128 = fee_growth_global0_x128 - info.fee_growth_outside0_x128;
            info.fee_growth_outside1_x128 = fee_growth_global1_x128 - info.fee_growth_outside1_x128;
            info.seconds_per_liquidity_outside_x128 =
                seconds_per_liquidity_cumulative_x128 - info.seconds_per_liquidity_outside_x128;
            info.tick_cumulative_outside = tick_cumulative - info.tick_cumulative_outside;
            info.seconds_outside = time - info.seconds_outside;
            self.ticks.insert(&tick, &info);
            info.liquidity_net
        }
    }

    /// @title Packed tick initialized state library
    /// @notice Stores a packed mapping of tick index to its initialized state
    /// @dev The mapping uses int16 for keys since ticks are represented as and:i32 there are 256 (2^8) values per word.
    #[ink(impl)]
    impl SubPool {
        /// @notice Computes the position in the mapping where the initialized bit for a tick lives
        /// @param tick The tick for which to compute the position
        /// @return wordPos The key in the mapping containing the word in which the bit is stored
        /// @return bitPos The bit position in the word where the flag is stored
        fn position(tick: i32) -> (i16, u8) {
            ((tick >> 8) as i16, (tick % 256) as u8)
        }

        /// @notice Flips the initialized state for a given tick from false to true, or vice versa
        /// @param self The mapping in which to flip the tick
        /// @param tick The tick to flip
        /// @param tick_spacing The spacing between usable ticks
        fn flip_tick(&mut self, tick: i32, tick_spacing: i32) {
            assert!(tick % tick_spacing == 0); // ensure that the tick is spaced
            let (word_pos, bit_pos) = Self::position(tick / tick_spacing);
            let mask = 1 << bit_pos;
            let b = self.tick_bitmap.get(&word_pos).unwrap_or_default();
            self.tick_bitmap.insert(word_pos, &(b ^ U256::from(mask)));
        }

        /// @notice Returns the next initialized tick contained in the same word (or adjacent word) as the tick that is either
        /// to the left (less than or equal to) or right (greater than) of the given tick
        /// @param self The mapping in which to compute the next initialized tick
        /// @param tick The starting tick
        /// @param tick_spacing The spacing between usable ticks
        /// @param lte Whether to search for the next initialized tick to the left (less than or equal to the starting tick)
        /// @return next The next initialized or uninitialized tick up to 256 ticks away from the current tick
        /// @return initialized Whether the next tick is initialized, as the fn only searches within up to 256 ticks
        fn next_initialized_tick_within_one_word(
            &self,
            tick: i32,
            tick_spacing: i32,
            lte: bool,
        ) -> (i32, bool) {
            let mut compressed = tick / tick_spacing;
            if tick < 0 && tick % tick_spacing != 0 {
                compressed -= 1;
            } // round towards negative infinity

            if lte {
                let (word_pos, bit_pos) = Self::position(compressed);
                // all the 1s at or to the right of the current bit_pos
                let mask = (1 << bit_pos) - 1 + (1 << bit_pos);
                let masked = self.tick_bitmap.get(&word_pos).unwrap_or_default() & U256::from(mask);

                // if there are no initialized ticks to the right of or at the current tick, return rightmost in the word
                let initialized = masked != U256::zero();
                // overflow/underflow is possible, but prevented externally by limiting both tick_spacing and tick
                (
                    if initialized {
                        (compressed - (bit_pos - BitMath::most_significant_bit(masked)) as i32)
                            * tick_spacing
                    } else {
                        (compressed - bit_pos as i32) * tick_spacing
                    },
                    initialized,
                )
            } else {
                // start from the word of the next tick, since the current tick state doesn't matter
                let (word_pos, bit_pos) = Self::position(compressed + 1);
                // all the 1s at or to the left of the bit_pos
                let mask = !((1 << bit_pos) - 1);
                let masked = self.tick_bitmap.get(&word_pos).unwrap_or_default() & U256::from(mask);

                // if there are no initialized ticks to the left of the current tick, return leftmost in the word
                let initialized = masked != U256::zero();
                // overflow/underflow is possible, but prevented externally by limiting both tick_spacing and tick
                (
                    if initialized {
                        (compressed + 1 + (BitMath::least_significant_bit(masked) - bit_pos) as i32)
                            * tick_spacing
                    } else {
                        (compressed + 1 + (u8::MAX - bit_pos) as i32) * tick_spacing
                    },
                    initialized,
                )
            }
        }
    }

    /// @title TransferHelper
    /// @notice Contains helper methods for interacting with ERC20 tokens that do not consistently return true/false
    #[ink(impl)]
    impl SubPool {
        /// @notice Transfers tokens from msg.sender to a recipient
        /// @dev Calls transfer on token contract, errors with TF if transfer fails
        /// @param token The contract address of the token which will be transferred
        /// @param to The recipient of the transfer
        /// @param value The value of the transfer
        #[cfg_attr(test, allow(unused_variables))]
        fn safe_transfer(&mut self, token: AccountId, to: AccountId, value: U256) -> Result<()> {
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
    }

    /// Unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;
        use ink_lang as ink;
        type Event = <SubPool as ::ink_lang::reflect::ContractEventBase>::Type;
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
        // fn init_contract() -> SubPool {
        //     let erc = SubPool::new(
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
