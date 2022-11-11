use crate::primitives::U256;

/// @title BitMath
/// @dev This library provides functionality for computing bit properties of an unsigned integer
pub struct BitMath;

impl BitMath {
    /// @notice Returns the index of the most significant bit of the number,
    ///     where the least significant bit is at index 0 and the most significant bit is at index 255
    /// @dev The function satisfies the property:
    ///     x >= 2**mostSignificantBit(x) and x < 2**(mostSignificantBit(x)+1)
    /// @param x the value for which to compute the most significant bit, must be greater than 0
    /// @return r the index of the most significant bit
    pub fn most_significant_bit(mut x: U256) -> u8 {
        assert!(x > U256::zero());
        let mut r = 0;
        if x >= U256::from_hex_str("0x1000000000000000000000000000000") {
            x >>= U256::from(128);
            r += 128;
        }
        if x >= U256::from(0x10000000000000000u128) {
            x >>= U256::from(64);
            r += 64;
        }
        if x >= U256::from(0x100000000u128) {
            x >>= U256::from(32);
            r += 32;
        }
        if x >= U256::from(0x10000) {
            x >>= U256::from(16);
            r += 16;
        }
        if x >= U256::from(0x100) {
            x >>= U256::from(8);
            r += 8;
        }
        if x >= U256::from(0x10) {
            x >>= U256::from(4);
            r += 4;
        }
        if x >= U256::from(0x4) {
            x >>= U256::from(2);
            r += 2;
        }
        if x >= U256::from(0x2) {
            r += 1;
        }
        r
    }

    /// @notice Returns the index of the least significant bit of the number,
    ///     where the least significant bit is at index 0 and the most significant bit is at index 255
    /// @dev The function satisfies the property:
    ///     (x & 2**leastSignificantBit(x)) != 0 and (x & (2**(leastSignificantBit(x)) - 1)) == 0)
    /// @param x the value for which to compute the least significant bit, must be greater than 0
    /// @return r the index of the least significant bit
    pub fn least_significant_bit(mut x: U256) -> u8 {
        assert!(x > U256::zero());

        let mut r = 255;
        if x & U256::from(u128::MAX) > U256::zero() {
            r -= 128;
        } else {
            x >>= U256::from(128)
        }
        if x & U256::from(u64::MAX) > U256::zero() {
            r -= 64;
        } else {
            x >>= U256::from(64);
        }
        if x & U256::from(u32::MAX) > U256::zero() {
            r -= 32;
        } else {
            x >>= U256::from(32);
        }
        if x & U256::from(u16::MAX) > U256::zero() {
            r -= 16;
        } else {
            x >>= U256::from(16);
        }
        if x & U256::from(u8::MAX) > U256::zero() {
            r -= 8;
        } else {
            x >>= U256::from(8);
        }
        if x & U256::from(0xf) > U256::zero() {
            r -= 4;
        } else {
            x >>= U256::from(4);
        }
        if x & U256::from(0x3) > U256::zero() {
            r -= 2;
        } else {
            x >>= U256::from(2);
        }
        if x & U256::one() > U256::zero() {
            r -= 1;
        }
        r
    }
}
