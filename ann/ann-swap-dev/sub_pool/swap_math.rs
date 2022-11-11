use crate::full_math::FullMath;
use crate::primitives::{I256, U160, U256};
use crate::sqrt_price_math::SqrtPriceMath;

/// @title Computes the result of a swap within ticks
/// @notice Contains methods for computing the result of a swap within a single tick price range, i.e., a single tick.
pub struct SwapMath;
impl SwapMath {
    /// @notice Computes the result of swapping some amount in, or amount out, given the parameters of the swap
    /// @dev The fee, plus the amount in, will never exceed the amount remaining if the swap's `amountSpecified` is positive
    /// @param sqrtRatioCurrentX96 The current sqrt price of the pool
    /// @param sqrtRatioTargetX96 The price that cannot be exceeded, from which the direction of the swap is inferred
    /// @param liquidity The usable liquidity
    /// @param amountRemaining How much input or output amount is remaining to be swapped in/out
    /// @param feePips The fee taken from the input amount, expressed in hundredths of a bip
    /// @return sqrtRatioNextX96 The price after swapping the amount in/out, not to exceed the price target
    /// @return amountIn The amount to be swapped in, of either token0 or token1, based on the direction of the swap
    /// @return amountOut The amount to be received, of either token0 or token1, based on the direction of the swap
    /// @return feeAmount The amount of input that will be taken as a fee
    pub fn compute_swap_step(
        sqrt_ratio_current_x96: U160,
        sqrt_ratio_target_x96: U160,
        liquidity: u128,
        amount_remaining: I256,
        fee_pips: u32,
    ) -> (U160, U256, U256, U256) {
        let sqrt_ratio_next_x96;
        let (mut amount_in, mut amount_out) = (U256::zero(), U256::zero());
        let zero_for_one = sqrt_ratio_current_x96 >= sqrt_ratio_target_x96;
        let exact_in = amount_remaining >= I256::zero();
        let fee_pips = U256::from(fee_pips);
        if exact_in {
            let amount_remaining_less_fee = FullMath::mul_div(
                U256::from(amount_remaining),
                U256::from(1_000_000) - fee_pips,
                U256::from(1_000_000),
            );
            amount_in = if zero_for_one {
                SqrtPriceMath::get_amount0_delta_between_two_prices(
                    sqrt_ratio_target_x96,
                    sqrt_ratio_current_x96,
                    liquidity,
                    true,
                )
            } else {
                SqrtPriceMath::get_amount1_delta_between_two_prices(
                    sqrt_ratio_current_x96,
                    sqrt_ratio_target_x96,
                    liquidity,
                    true,
                )
            };
            sqrt_ratio_next_x96 = if amount_remaining_less_fee >= amount_in {
                sqrt_ratio_target_x96
            } else {
                SqrtPriceMath::get_next_sqrt_price_from_input(
                    sqrt_ratio_current_x96,
                    liquidity,
                    amount_remaining_less_fee,
                    zero_for_one,
                )
            };
        } else {
            amount_out = if zero_for_one {
                SqrtPriceMath::get_amount1_delta_between_two_prices(
                    sqrt_ratio_target_x96,
                    sqrt_ratio_current_x96,
                    liquidity,
                    false,
                )
            } else {
                SqrtPriceMath::get_amount0_delta_between_two_prices(
                    sqrt_ratio_current_x96,
                    sqrt_ratio_target_x96,
                    liquidity,
                    false,
                )
            };
            sqrt_ratio_next_x96 = if U256::from(-amount_remaining) >= amount_out {
                sqrt_ratio_target_x96
            } else {
                SqrtPriceMath::get_next_sqrt_price_from_output(
                    sqrt_ratio_current_x96,
                    liquidity,
                    U256::from(-amount_remaining),
                    zero_for_one,
                )
            };
        }

        let max = sqrt_ratio_target_x96 == sqrt_ratio_next_x96;

        // get the input/output amounts
        if zero_for_one {
            amount_in = if max && exact_in {
                amount_in
            } else {
                SqrtPriceMath::get_amount0_delta_between_two_prices(
                    sqrt_ratio_next_x96,
                    sqrt_ratio_current_x96,
                    liquidity,
                    true,
                )
            };
            amount_out = if max && !exact_in {
                amount_out
            } else {
                SqrtPriceMath::get_amount1_delta_between_two_prices(
                    sqrt_ratio_next_x96,
                    sqrt_ratio_current_x96,
                    liquidity,
                    false,
                )
            };
        } else {
            amount_in = if max && exact_in {
                amount_in
            } else {
                SqrtPriceMath::get_amount1_delta_between_two_prices(
                    sqrt_ratio_current_x96,
                    sqrt_ratio_next_x96,
                    liquidity,
                    true,
                )
            };
            amount_out = if max && !exact_in {
                amount_out
            } else {
                SqrtPriceMath::get_amount0_delta_between_two_prices(
                    sqrt_ratio_current_x96,
                    sqrt_ratio_next_x96,
                    liquidity,
                    false,
                )
            };
        }

        // cap the output amount to not exceed the remaining output amount
        if !exact_in && amount_out > U256::from(-amount_remaining) {
            amount_out = U256::from(-amount_remaining);
        }

        let fee_amount = if exact_in && sqrt_ratio_next_x96 != sqrt_ratio_target_x96 {
            // we didn't reach the target, so take the remainder of the maximum input as fee
            U256::from(amount_remaining) - amount_in
        } else {
            FullMath::mul_div_rounding_up(amount_in, fee_pips, U256::from(1_000_000) - fee_pips)
        };
        (sqrt_ratio_next_x96, amount_in, amount_out, fee_amount)
    }
}
