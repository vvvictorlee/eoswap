use crate::fixed_point96::FixedPoint96;
use crate::full_math::FullMath;
use crate::primitives::{I256, U160, U256};
use crate::safe_cast::SafeCast;
use crate::unsafe_math::UnsafeMath;

/// @title Functions based on Q64.96 sqrt price and liquidity
/// @notice Contains the math that uses square root of price as a Q64.96 and liquidity to compute deltas
pub struct SqrtPriceMath;
impl SqrtPriceMath {
    /// @notice Gets the next sqrt price given a delta of token0
    /// @dev Always rounds up, because in the exact output case (increasing price) we need to move the price at least
    /// far enough to get the desired output amount, and in the exact input case (decreasing price) we need to move the
    /// price less in order to not send too much output.
    /// The most precise formula for this is liquidity * sqrt_px96 / (liquidity +- amount * sqrt_px96),
    /// if this is impossible because of overflow, we calculate liquidity / (liquidity / sqrt_px96 +- amount).
    /// @param sqrt_px96 The starting price, i.e. before accounting for the token0 delta
    /// @param liquidity The amount of usable liquidity
    /// @param amount How much of token0 to add or remove from virtual reserves
    /// @param add Whether to add or remove the amount of token0
    /// @return The price after adding or removing amount, depending on add
    pub fn get_next_sqrt_price_from_amount0_rounding_up(
        sqrt_px96: U160,
        liquidity: u128,
        amount: U256,
        add: bool,
    ) -> U160 {
        // we short circuit amount == 0 because the result is otherwise not guaranteed to equal the input price
        if amount == U256::zero() {
            return sqrt_px96;
        }
        let numerator1 = U256::from(liquidity) << U256::from(FixedPoint96::RESOLUTION);

        if add {
            let product = amount * sqrt_px96;
            if product / amount == sqrt_px96 {
                let denominator = numerator1 + product;
                if denominator >= numerator1
                // always fits in 160 bits
                {
                    return FullMath::mul_div_rounding_up(numerator1, sqrt_px96, denominator);
                }
            }

            UnsafeMath::div_rounding_up(numerator1, (numerator1 / sqrt_px96).saturating_add(amount))
        } else {
            let product = amount * sqrt_px96;
            // if the product overflows, we know the denominator underflows
            // in addition, we must check that the denominator does not underflow
            assert!(product / amount == sqrt_px96 && numerator1 > product);
            let denominator = numerator1 - product;
            SafeCast::to_uint160(FullMath::mul_div_rounding_up(
                numerator1,
                sqrt_px96,
                denominator,
            ))
        }
    }

    /// @notice Gets the next sqrt price given a delta of token1
    /// @dev Always rounds down, because in the exact output case (decreasing price) we need to move the price at least
    /// far enough to get the desired output amount, and in the exact input case (increasing price) we need to move the
    /// price less in order to not send too much output.
    /// The formula we compute is within <1 wei of the lossless version: sqrt_px96 +- amount / liquidity
    /// @param sqrt_px96 The starting price, i.e., before accounting for the token1 delta
    /// @param liquidity The amount of usable liquidity
    /// @param amount How much of token1 to add, or remove, from virtual reserves
    /// @param add Whether to add, or remove, the amount of token1
    /// @return The price after adding or removing `amount`
    pub fn get_next_sqrt_price_from_amount1_rounding_down(
        sqrt_px96: U160,
        liquidity: u128,
        amount: U256,
        add: bool,
    ) -> U160 {
        // if we're adding (subtracting), rounding down requires rounding the quotient down (up)
        // in both cases, avoid a mul_div for most inputs
        if add {
            let quotient = if amount <= U160::MAX {
                (amount << U160::from(FixedPoint96::RESOLUTION)) / U160::from(liquidity)
            } else {
                FullMath::mul_div(amount, FixedPoint96::q96(), U160::from(liquidity))
            };

            SafeCast::to_uint160(sqrt_px96.saturating_add(quotient))
        } else {
            let quotient = if amount <= U160::MAX {
                UnsafeMath::div_rounding_up(
                    amount << U160::from(FixedPoint96::RESOLUTION),
                    U160::from(liquidity),
                )
            } else {
                FullMath::mul_div_rounding_up(
                    amount,
                    FixedPoint96::q96(),
                    U256::from(U160::from(liquidity)),
                )
            };

            assert!(sqrt_px96 > quotient);
            // always fits 160 bits
            sqrt_px96 - quotient
        }
    }

    /// @notice Gets the next sqrt price given an input amount of token0 or token1
    /// @dev Throws if price or liquidity are 0, or if the next price is out of bounds
    /// @param sqrt_px96 The starting price, i.e., before accounting for the input amount
    /// @param liquidity The amount of usable liquidity
    /// @param amount_in How much of token0, or token1, is being swapped in
    /// @param zero_for_one Whether the amount in is token0 or token1
    /// @return sqrtQX96 The price after adding the input amount to token0 or token1
    pub fn get_next_sqrt_price_from_input(
        sqrt_px96: U160,
        liquidity: u128,
        amount_in: U256,
        zero_for_one: bool,
    ) -> U160 {
        assert!(sqrt_px96 > U160::zero());
        assert!(liquidity > 0);

        // round to make sure that we don't pass the target price
        if zero_for_one {
            Self::get_next_sqrt_price_from_amount0_rounding_up(
                sqrt_px96, liquidity, amount_in, true,
            )
        } else {
            Self::get_next_sqrt_price_from_amount1_rounding_down(
                sqrt_px96, liquidity, amount_in, true,
            )
        }
    }

    /// @notice Gets the next sqrt price given an output amount of token0 or token1
    /// @dev Throws if price or liquidity are 0 or the next price is out of bounds
    /// @param sqrt_px96 The starting price before accounting for the output amount
    /// @param liquidity The amount of usable liquidity
    /// @param amount_out How much of token0, or token1, is being swapped out
    /// @param zero_for_one Whether the amount out is token0 or token1
    /// @return sqrtQX96 The price after removing the output amount of token0 or token1
    pub fn get_next_sqrt_price_from_output(
        sqrt_px96: U160,
        liquidity: u128,
        amount_out: U256,
        zero_for_one: bool,
    ) -> U160 {
        assert!(sqrt_px96 > U160::zero());
        assert!(liquidity > 0);

        // round to make sure that we pass the target price
        if zero_for_one {
            Self::get_next_sqrt_price_from_amount1_rounding_down(
                sqrt_px96, liquidity, amount_out, false,
            )
        } else {
            Self::get_next_sqrt_price_from_amount0_rounding_up(
                sqrt_px96, liquidity, amount_out, false,
            )
        }
    }

    /// @notice Gets the amount0 delta between two prices
    /// @dev Calculates liquidity / sqrt(lower) - liquidity / sqrt(upper),
    /// i.e. liquidity * (sqrt(upper) - sqrt(lower)) / (sqrt(upper) * sqrt(lower))
    /// @param sqrt_ratio_ax96  A sqrt price
    /// @param sqrt_ratio_bx96  Another sqrt price
    /// @param liquidity The amount of usable liquidity
    /// @param round_up Whether to round the amount up or down
    /// @return amount0 Amount of token0 required to cover a position of size liquidity between the two passed prices
    pub fn get_amount0_delta_between_two_prices(
        sqrt_ratio_ax96: U160,
        sqrt_ratio_bx96: U160,
        liquidity: u128,
        round_up: bool,
    ) -> U256 {
        let (sqrt_ratio_ax96, sqrt_ratio_bx96) = if sqrt_ratio_ax96 > sqrt_ratio_bx96 {
            (sqrt_ratio_bx96, sqrt_ratio_ax96)
        } else {
            (sqrt_ratio_ax96, sqrt_ratio_bx96)
        };

        let numerator1 = U256::from(liquidity) << U256::from(FixedPoint96::RESOLUTION);
        let numerator2 = sqrt_ratio_bx96 - sqrt_ratio_ax96;

        assert!(sqrt_ratio_ax96 > U160::zero());

        if round_up {
            UnsafeMath::div_rounding_up(
                FullMath::mul_div_rounding_up(numerator1, numerator2, sqrt_ratio_bx96),
                sqrt_ratio_ax96,
            )
        } else {
            FullMath::mul_div(numerator1, numerator2, sqrt_ratio_bx96) / sqrt_ratio_ax96
        }
    }

    /// @notice Gets the amount1 delta between two prices
    /// @dev Calculates liquidity * (sqrt(upper) - sqrt(lower))
    /// @param sqrt_ratio_ax96  A sqrt price
    /// @param sqrt_ratio_bx96  Another sqrt price
    /// @param liquidity The amount of usable liquidity
    /// @param round_up Whether to round the amount up, or down
    /// @return amount1 Amount of token1 required to cover a position of size liquidity between the two passed prices
    pub fn get_amount1_delta_between_two_prices(
        sqrt_ratio_ax96: U160,
        sqrt_ratio_bx96: U160,
        liquidity: u128,
        round_up: bool,
    ) -> U256 {
        let (sqrt_ratio_ax96, sqrt_ratio_bx96) = if sqrt_ratio_ax96 > sqrt_ratio_bx96 {
            (sqrt_ratio_bx96, sqrt_ratio_ax96)
        } else {
            (sqrt_ratio_ax96, sqrt_ratio_bx96)
        };

        if round_up {
            FullMath::mul_div_rounding_up(
                U256::from(liquidity),
                sqrt_ratio_bx96 - sqrt_ratio_ax96,
                FixedPoint96::q96(),
            )
        } else {
            FullMath::mul_div(
                U256::from(liquidity),
                sqrt_ratio_bx96 - sqrt_ratio_ax96,
                FixedPoint96::q96(),
            )
        }
    }

    /// @notice Helper that gets signed token0 delta
    /// @param sqrt_ratio_ax96  A sqrt price
    /// @param sqrt_ratio_bx96  Another sqrt price
    /// @param liquidity The change in liquidity for which to compute the amount0 delta
    /// @return amount0 Amount of token0 corresponding to the passed liquidity_delta between the two prices
    pub fn get_amount0_delta(
        sqrt_ratio_ax96: U160,
        sqrt_ratio_bx96: U160,
        liquidity: i128,
    ) -> I256 {
        if liquidity < 0 {
            -SafeCast::to_int256(Self::get_amount0_delta_between_two_prices(
                sqrt_ratio_ax96,
                sqrt_ratio_bx96,
                (-liquidity) as u128,
                false,
            ))
        } else {
            SafeCast::to_int256(Self::get_amount0_delta_between_two_prices(
                sqrt_ratio_ax96,
                sqrt_ratio_bx96,
                liquidity as u128,
                true,
            ))
        }
    }

    /// @notice Helper that gets signed token1 delta
    /// @param sqrt_ratio_ax96  A sqrt price
    /// @param sqrt_ratio_bx96  Another sqrt price
    /// @param liquidity The change in liquidity for which to compute the amount1 delta
    /// @return amount1 Amount of token1 corresponding to the passed liquidity_delta between the two prices
    pub fn get_amount1_delta(
        sqrt_ratio_ax96: U160,
        sqrt_ratio_bx96: U160,
        liquidity: i128,
    ) -> I256 {
        if liquidity < 0 {
            -SafeCast::to_int256(Self::get_amount1_delta_between_two_prices(
                sqrt_ratio_ax96,
                sqrt_ratio_bx96,
                (-liquidity) as u128,
                false,
            ))
        } else {
            SafeCast::to_int256(Self::get_amount1_delta_between_two_prices(
                sqrt_ratio_ax96,
                sqrt_ratio_bx96,
                liquidity as u128,
                true,
            ))
        }
    }
}
