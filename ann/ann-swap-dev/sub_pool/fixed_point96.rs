/// @title FixedPoint96
/// @notice A library for handling binary fixed point numbers, see https://en.wikipedia.org/wiki/Q_(number_format)
/// @dev Used in SqrtPriceMath.sol
use crate::primitives::U256;
pub struct FixedPoint96;
impl FixedPoint96 {
    pub const RESOLUTION: u8 = 96;
    //  pub const Q96:U256 =U256::from (0x1000000000000000000000000);
    pub fn q96() -> U256 {
        U256::from(0x1000000000000000000000000u128)
    }
}
