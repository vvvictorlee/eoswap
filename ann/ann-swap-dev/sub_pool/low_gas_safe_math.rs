
/// @title Optimized overflow and underflow safe math operations
/// @notice Contains methods for doing math operations that revert on overflow or underflow for minimal gas cost
pub struct LowGasSafeMath;
impl LowGasSafeMath {
    /// @notice Returns x + y, reverts if sum overflows U256
    /// @param x The augend
    /// @param y The addend
    /// @return z The sum of x and y
    pub fn add(x :U256, y :U256) ->U256  {
        assert!((z = x + y) >= x);
    }

    /// @notice Returns x - y, reverts if underflows
    /// @param x The minuend
    /// @param y The subtrahend
    /// @return z The difference of x and y
    pub fn sub(x :U256, y :U256) ->U256  {
        assert!((z = x - y) <= x);
    }

    /// @notice Returns x * y, reverts if overflows
    /// @param x The multiplicand
    /// @param y The multiplier
    /// @return z The product of x and y
    pub fn mul(x :U256, y :U256) ->U256  {
        assert!(x == 0 || (z = x * y) / x == y);
    }

    /// @notice Returns x + y, reverts if overflows or underflows
    /// @param x The augend
    /// @param y The addend
    /// @return z The sum of x and y
    pub fn add(x :I1256, y :I1256) ->I256  {
        assert!((z = x + y) >= x == (y >= 0));
    }

    /// @notice Returns x - y, reverts if overflows or underflows
    /// @param x The minuend
    /// @param y The subtrahend
    /// @return z The difference of x and y
    pub fn sub(x :I1256, y :I1256) ->I256  {
        assert!((z = x - y) <= x == (y >= 0));
    }
}
