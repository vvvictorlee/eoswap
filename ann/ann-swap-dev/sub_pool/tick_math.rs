use crate::primitives::{I256, U160, U256};
use ink_prelude::vec::Vec;
/// @title Math library for computing sqrt prices from ticks and vice versa
/// @notice Computes sqrt price for ticks of size 1.0001, i.e. sqrt(1.0001^tick) as fixed point Q64.96 numbers. Supports
/// prices between 2**-128 and 2**128
pub struct TickMath;
impl TickMath {
    /// @dev The minimum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**-128
    pub const MIN_TICK: i32 = -887272;
    /// @dev The maximum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**128
    pub const MAX_TICK: i32 = 887272;

    /// @dev The minimum value that can be returned from #getSqrtRatioAtTick. Equivalent to getSqrtRatioAtTick(MIN_TICK)
    //    pub  const MIN_SQRT_RATIO: U160 = U160::from(4295128739);
    pub fn min_sqrt_ratio() -> U160 {
        U160::from(4295128739u128)
    }
    /// @dev The maximum value that can be returned from #getSqrtRatioAtTick. Equivalent to getSqrtRatioAtTick(MAX_TICK)
    //    pub  const MAX_SQRT_RATIO: U160 = U160::from_dec_str("1461446703485210103287273052203988822378723970342");
    pub fn max_sqrt_ratio() -> U160 {
        U160::from_dec_str("1461446703485210103287273052203988822378723970342")
    }
    /// @notice Calculates sqrt(1.0001^tick) * 2^96
    /// @dev Throws if |tick| > max tick
    /// @param tick The input tick for the above formula
    /// @return sqrtPriceX96 A Fixed point Q64.96 number representing the sqrt of the ratio of the two assets (token1/token0)
    /// at the given tick
    pub fn get_sqrt_ratio_at_tick(tick: i32) -> U160 {
        let abs_tick = if tick < 0 {
            U256::from(-I256::from(tick))
        } else {
            U256::from(I256::from(tick))
        };
        assert!(abs_tick <= U256::from(Self::MAX_TICK), "T");

        let mut ratio = U256::from_hex_str("0x100000000000000000000000000000000");
        let bignums = [
            "0xfffcb933bd6fad37aa2d162d1a594001",
            "0xfff97272373d413259a46990580e213a",
            "0xfff2e50f5f656932ef12357cf3c7fdcc",
            "0xffe5caca7e10e4e61c3624eaa0941cd0",
            "0xffcb9843d60f6159c9db58835c926644",
            "0xff973b41fa98c081472e6896dfb254c0",
            "0xff2ea16466c96a3843ec78b326b52861",
            "0xfe5dee046a99a2a811c461f1969c3053",
            "0xfcbe86c7900a88aedcffc83b479aa3a4",
            "0xf987a7253ac413176f2b074cf7815e54",
            "0xf3392b0822b70005940c7a398e4b70f3",
            "0xe7159475a2c29b7443b29c7fa6e889d9",
            "0xd097f3bdfd2022b8845ad8f792aa5825",
            "0xa9f746462d870fdf8a65dc1f90e061e5",
            "0x70d869a156d2a1b890bb3df62baf32f7",
            "0x31be135f97d08fd981231505542fcfa6",
            "0x9aa508b5b7a84e1c677de54f3e99bc9",
            "0x5d6af8dedb81196699c329225ee604",
            "0x2216e584f5fa1ea926041bedfe98",
            "0x48a170391f7dc42444e8fa2",
        ];
        let mut indices = Vec::new();
        for i in 0..5 {
            for j in 0..4 {
                indices.push(1 << i * 4 + j);
            }
        }

        for (i, v) in indices.into_iter().zip(bignums) {
            if abs_tick & U256::from(i) != U256::zero() {
                ratio = if i == 1 {
                    U256::from_hex_str(v)
                } else {
                    (ratio * U256::from_hex_str(v)) >> U256::from(128)
                };
            }
        }

        if tick > 0 {
            ratio = U256::MAX / ratio;
        }
        let uu32 = U256::from(32);
        // this divides by 1<<32 rounding up to go from a Q128.128 to a Q128.96.
        // we then downcast because we know the result always fits within 160 bits due to our tick input constraint
        // we round up in the division so get_tick_atSqrt_ratio of the output price is always consistent
        U160::from(
            (ratio >> uu32)
                + (if ratio % (U256::one() << uu32) == U256::zero() {
                    U256::zero()
                } else {
                    U256::one()
                }),
        )
    }

    /// @notice Calculates the greatest tick value such that getRatioAtTick(tick) <= ratio
    /// @dev Throws in case sqrt_price_x96 < MIN_SQRT_RATIO, as MIN_SQRT_RATIO is the lowest value getRatioAtTick may
    /// ever return.
    /// @param sqrt_price_x96 The sqrt ratio for which to compute the tick as a Q64.96
    /// @return tick The greatest tick for which the ratio is less than or equal to the input ratio
    pub fn get_tick_at_sqrt_ratio(sqrt_price_x96: U160) -> i32 {
        // second inequality must be < because the price can never reach the price at the max tick

        assert!(
            sqrt_price_x96 >= Self::min_sqrt_ratio() && sqrt_price_x96 < Self::max_sqrt_ratio(),
            "R"
        );
        let ratio = U256::from(sqrt_price_x96) << U256::from(32);

        let mut r = ratio;
        let mut msb = U256::zero();
        let gt = |r: U256, b: u128| {
            if r > U256::from(b) {
                1
            } else {
                0
            }
        };
        let shl = |b: u8, r: u8| r << b;
        let shr = |b: u8, r: U256| r >> U256::from(b);
        let or = |r: U256, f: u8| r | U256::from(f);
        let mut b = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;
        let mut j = 128;
        for i in (1..8).rev() {
            let f = shl(i, gt(r, b));
            msb = or(msb, f);
            r = shr(f, r);
            j >>= 1;
            b >>= j;
        }
        let f = gt(r, 0x1);
        msb = or(msb, f);
        if msb >= U256::from(128) {
            r = ratio >> (msb - U256::from(127));
        } else {
            r = ratio << (U256::from(127) - msb);
        }

        let mut log_2 = (I256::from(msb) - I256::from(128)) << I256::from(64);
        let mul = |r: U256, rr: U256| r * rr;

        let shl256 = |b: u8, r: U256| r << U256::from(b);
        let shr256 = |b: U256, r: U256| r >> b;
        let or256 = |log_2: I256, f: U256| log_2 | I256::from(f);

        for i in (51..=63).rev() {
            r = shr(127, mul(r, r));
            let f = shr(128, r);
            log_2 = or256(log_2, shl256(i, f));
            r = shr256(f, r);
        }
        r = shr(127, mul(r, r));
        let f = shr(128, r);
        log_2 = or256(log_2, shl256(50, f));

        let log_sqrt10001 = log_2 * I256::from(255738958999603826347141u128); // 128.128 number

        let tick_low = ((log_sqrt10001
            - I256::from_dec_str("3402992956809132418596140100660247210").unwrap())
            >> I256::from(128))
        .as_i32();
        let tick_hi = ((log_sqrt10001
            + I256::from_dec_str("291339464771989622907027621153398088495").unwrap())
            >> I256::from(128))
        .as_i32();

        if tick_low == tick_hi {
            tick_low
        } else {
            if Self::get_sqrt_ratio_at_tick(tick_hi) <= sqrt_price_x96 {
                tick_hi
            } else {
                tick_low
            }
        }
    }
}
