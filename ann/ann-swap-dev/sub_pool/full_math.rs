use crate::primitives::{I256, U256};

/// @title Contains 512-bit math functions
/// @notice Facilitates multiplication and division that can have overflow of an intermediate value without any loss of precision
/// @dev Handles "phantom overflow" i.e., allows multiplication and division where an intermediate value overflows 256 bits
pub struct FullMath;
impl FullMath {
    /// @notice Calculates floor(a×b÷denominator) with full precision. Throws if result overflows a U256 or denominator == 0
    /// @param a The multiplicand
    /// @param b The multiplier
    /// @param denominator The divisor
    /// @return result The 256-bit result
    /// @dev Credit to Remco Bloemen under MIT license https://xn--2-umb.com/21/muldiv
    pub fn mul_div(a: U256, b: U256, mut denominator: U256) -> U256 {
        // 512-bit multiply [prod1 prod0] = a * b
        // Compute the product mod 2**256 and mod 2**256 - 1
        // then use the Chinese Remainder Theorem to reconstruct
        // the 512 bit result. The result is stored in two 256
        // variables such that product = prod1 * 2**256 + prod0
        // let mut prod0: U256 = U256::zero(); // Least significant 256 bits of the product
        // let mut prod1: U256 = U256::zero(); // Most significant 256 bits of the product
        let mulmod = |a: U256, b: U256, c: U256| a * b % c;
        let add = |a: U256, b: U256| a + b;
        let sub = |a: U256, b: U256| a - b;
        let mul = |a: U256, b: U256| a * b;
        let div = |a: U256, b: U256| a / b;

        let lt = |a: U256, b: U256| {
            if a < b {
                U256::one()
            } else {
                U256::zero()
            }
        };
        let gt = |a: U256, b: U256| {
            if a > b {
                U256::one()
            } else {
                U256::zero()
            }
        };
        let not = |a: U256| !a;
        let mm = mulmod(a, b, not(U256::zero()));
        let mut prod0 = mul(a, b);
        let mut prod1 = sub(sub(mm, prod0), lt(mm, prod0));
        // Handle non-overflow cases, 256 by 256 division
        if prod1 == U256::zero() {
            assert!(denominator > U256::zero());
            return div(prod0, denominator);
        }

        // Make sure the result is less than 2**256.
        // Also prevents denominator == 0
        assert!(denominator > prod1);

        ///////////////////////////////////////////////
        // 512 by 256 division.
        ///////////////////////////////////////////////

        // Make division exact by subtracting the remainder from [prod1 prod0]
        // Compute remainder using mulmod
        let remainder = mulmod(a, b, denominator);

        // Subtract 256 bit number from 512 bit number
        prod1 = sub(prod1, gt(remainder, prod0));
        prod0 = sub(prod0, remainder);

        // Factor powers of two out of denominator
        // Compute largest power of two divisor of denominator.
        // Always >= 1.
        let mut twos = U256::from(-I256::from(denominator) & I256::from(denominator));
        // Divide denominator by power of two
        denominator = div(denominator, twos);

        // // Divide [prod1 prod0] by the factors of two
        prod0 = div(prod0, twos);
        // // Shift in bits from prod1 into prod0. For this we need
        // // to flip `twos` such that it is 2**256 / twos.
        // // If twos is zero, then it becomes one
        twos = add(div(sub(U256::zero(), twos), twos), U256::one());
        prod0 |= prod1 * twos;

        // Invert denominator mod 2**256
        // Now that denominator is an odd number, it has an inverse
        // modulo 2**256 such that denominator * inv = 1 mod 2**256.
        // Compute the inverse by starting with a seed that is correct
        // correct for four bits. That is, denominator * inv = 1 mod 2**4
        let u2 = U256::from(2);
        let mut inv = (U256::from(3) * denominator) ^ u2;
        // Now use Newton-Raphson iteration to improve the precision.
        // Thanks to Hensel's lifting lemma, this also works in modular
        // arithmetic, doubling the correct bits in each step.
        inv *= u2 - denominator * inv; // inverse mod 2**8
        inv *= u2 - denominator * inv; // inverse mod 2**16
        inv *= u2 - denominator * inv; // inverse mod 2**32
        inv *= u2 - denominator * inv; // inverse mod 2**64
        inv *= u2 - denominator * inv; // inverse mod 2**128
        inv *= u2 - denominator * inv; // inverse mod 2**256

        // Because the division is now exact we can divide by multiplying
        // with the modular inverse of denominator. This will give us the
        // correct result modulo 2**256. Since the precoditions guarantee
        // that the outcome is less than 2**256, this is the final result.
        // We don't need to compute the high bits of the result and prod1
        // is no longer required.
        prod0 * inv
    }

    /// @notice Calculates ceil(a×b÷denominator) with full precision. Throws if result overflows a U256 or denominator == 0
    /// @param a The multiplicand
    /// @param b The multiplier
    /// @param denominator The divisor
    /// @return result The 256-bit result
    pub fn mul_div_rounding_up(a: U256, b: U256, denominator: U256) -> U256 {
        let mut result = Self::mul_div(a, b, denominator);
        if (a * b) % denominator > U256::zero() {
            assert!(result < U256::MAX);
            result += U256::one();
        }
        result
    }
}
