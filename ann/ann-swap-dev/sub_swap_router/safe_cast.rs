use crate::primitives::{I256, U160, U256};

/// @title Safe casting methods
/// @notice Contains methods for safely casting between types
pub struct SafeCast;
impl SafeCast {
    /// @notice Cast a U256 to a U160, revert on overflow
    /// @param y The U256 to be downcasted
    /// @return z The downcasted integer, now type U160
    pub fn to_uint160(y: U256) -> U160 {
        let z = U160::from(y);
        assert!(U256::from(z) == y);
        z
    }

    /// @notice Cast a I256 to a i128, revert on overflow or underflow
    /// @param y The I256 to be downcasted
    /// @return z The downcasted integer, now type i128
    pub fn to_int128(y: I256) -> i128 {
        let z = y.as_i128();
        assert!(I256::from(z) == y);
        z
    }

    /// @notice Cast a U256 to a I256, revert on overflow
    /// @param y The U256 to be casted
    /// @return z The casted integer, now type I256
    pub fn to_int256(y: U256) -> I256 {
        assert!(y < (U256::one() << U256::from(255)));
        I256::from(y)
    }
}
