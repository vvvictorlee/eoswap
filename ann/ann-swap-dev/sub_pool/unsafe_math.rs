use crate::primitives::U256;
/// @title Math functions that do not check inputs or outputs
/// @notice Contains methods that perform common math functions but do not do any overflow or underflow checks
pub struct UnsafeMath;
impl UnsafeMath {
    /// @notice Returns ceil(x / y)
    /// @dev division by 0 has unspecified behavior, and must be checked externally
    /// @param x The dividend
    /// @param y The divisor
    /// @return z The quotient, ceil(x / y)
    pub fn div_rounding_up(x: U256, y: U256) -> U256 {
        x / y
            + if x % y > U256::zero() {
                U256::one()
            } else {
                U256::zero()
            }
    }
}
