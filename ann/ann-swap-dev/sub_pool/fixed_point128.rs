use crate::primitives::U256;

/// @title FixedPoint128
/// @notice A library for handling binary fixed point numbers, see https://en.wikipedia.org/wiki/Q_(number_format)
pub struct FixedPoint128;
impl FixedPoint128 {
    // pub  const Q128:U256 =U256::from_hex_str( "0x100000000000000000000000000000000");
    pub fn q128() -> U256 {
        U256::from_hex_str("0x100000000000000000000000000000000")
    }
}
