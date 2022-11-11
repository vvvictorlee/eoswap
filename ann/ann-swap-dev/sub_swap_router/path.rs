use crate::bytes_lib::BytesLib;
use crate::primitives::U256;
use ink_env::AccountId;
 use ink_prelude::vec::Vec;
/// @title Functions for manipulating path data for multihop swaps
pub struct Path;
impl Path {
    /// @dev The length of the bytes encoded AccountId
    const ADDR_SIZE: u32 = 20;
    /// @dev The length of the bytes encoded fee
    const FEE_SIZE: u32 = 3;

    /// @dev The offset of a single token AccountId and pool fee
    const NEXT_OFFSET: u32 = Self::ADDR_SIZE +  Self::FEE_SIZE;
    /// @dev The offset of an encoded pool key
    const POP_OFFSET: u32 =  Self::NEXT_OFFSET +  Self::ADDR_SIZE;
    /// @dev The minimum length of an encoding that contains 2 or more pools
    const MULTIPLE_POOLS_MIN_LENGTH: u32 =  Self::POP_OFFSET +  Self::NEXT_OFFSET;

    /// @notice Returns true iff the path contains two or more pools
    /// @param path The encoded swap path
    /// @return True if path contains two or more pools, otherwise false
    pub fn has_multiple_pools(path: &Vec<u8>) -> bool {
        path.len() as u32 >= Self::MULTIPLE_POOLS_MIN_LENGTH
    }

    /// @notice Returns the number of pools in the path
    /// @param path The encoded swap path
    /// @return The number of pools in the path
    pub fn num_pools(path: &Vec<u8>) -> U256 {
        // Ignore the first token AccountId. From then on every fee and token offset indicates a pool.
        U256::from((path.len() as u32- Self::ADDR_SIZE) / Self::NEXT_OFFSET)
    }

    /// @notice Decodes the first pool in path
    /// @param path The bytes encoded swap path
    /// @return token_a The first token of the given pool
    /// @return token_b The second token of the given pool
    /// @return fee The fee level of the pool
    pub fn decode_first_pool(path: &Vec<u8>) -> (AccountId, AccountId, u32) {
        let token_a = BytesLib::to_address(path,0);
        let fee = BytesLib::to_uint24(path,Self::ADDR_SIZE);
        let token_b = BytesLib::to_address(path,Self::NEXT_OFFSET);
        (token_a, token_b, fee)
    }

    /// @notice Gets the segment corresponding to the first pool in the path
    /// @param path The bytes encoded swap path
    /// @return The segment containing all data necessary to target the first pool in the path
    pub fn get_first_pool(path: &Vec<u8>) -> Vec<u8> {
        BytesLib::slice(path,0, Self::POP_OFFSET)
    }

    /// @notice Skips a token + fee element from the buffer and returns the remainder
    /// @param path The swap path
    /// @return The remaining token + fee elements in the path
    pub fn skip_token(path: &Vec<u8>) -> Vec<u8> {
        BytesLib::slice(path,Self::NEXT_OFFSET, path.len() as u32- Self::NEXT_OFFSET)
    }
}
