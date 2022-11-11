/// @title Math library for liquidity
pub struct LiquidityMath;
impl LiquidityMath {
    /// @notice Add a signed liquidity delta to liquidity and revert if it overflows or underflows
    /// @param x The liquidity before change
    /// @param y The delta by which liquidity should be changed
    /// @return z The liquidity delta
    pub fn add_delta(x: u128, y: i128) -> u128 {
        let z;
        if y < 0 {
            z = x - (-y) as u128;
            assert!(z < x, "LS");
        } else {
            z = x + (y as u128);
            assert!(z >= x, "LA");
        }
        z
    }
}
