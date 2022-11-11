 use ink_env::AccountId;
 use ink_prelude::vec::Vec;
/*
 * @title Solidity Bytes Arrays Utils
 * @author Gonçalo Sá <goncalo.sa@consensys.net>
 *
 * @dev Bytes tightly packed arrays utility library for ethereum contracts written in Solidity.
 *      The library lets you concatenate, slice and type cast bytes arrays both in memory and storage.
 */
pub struct BytesLib;

impl BytesLib {
    pub fn slice(bytes: &Vec<u8>, start: u32, _length: u32) -> Vec<u8> {
        assert!(_length + 31 >= _length, "slice_overflow");
        assert!(start + _length >= start, "slice_overflow");
        assert!(bytes.len() as u32 >= start + _length, "slice_outOfBounds");
        let i=start as usize;
        bytes[i..i+_length as usize].to_vec()
        // bytes tempBytes;

        // assembly {
        //     switch iszero(_length)
        //         case 0 {
        //             // Get a location of some free memory and store it in tempBytes as
        //             // Solidity does for memory variables.
        //             tempBytes := mload(0x40)

        //             // The first word of the slice result is potentially a partial
        //             // word read from the original array. To read it, we calculate
        //             // the length of that partial word and start copying that many
        //             // bytes into the array. The first word we copy will start with
        //             // data we don't care about, but the last `lengthmod` bytes will
        //             // land at the beginning of the contents of the new array. When
        //             // we're done copying, we overwrite the full first word with
        //             // the actual length of the slice.
        //             let lengthmod := and(_length, 31)

        //             // The multiplication in the next line is necessary
        //             // because when slicing multiples of 32 bytes (lengthmod == 0)
        //             // the following copy loop was copying the origin's length
        //             // and then ending prematurely not copying everything it should.
        //             let mc := add(add(tempBytes, lengthmod), mul(0x20, iszero(lengthmod)))
        //             let end := add(mc, _length)

        //             for {
        //                 // The multiplication in the next line has the same exact purpose
        //                 // as the one above.
        //                 let cc := add(add(add(bytes, lengthmod), mul(0x20, iszero(lengthmod))), start)
        //             } lt(mc, end) {
        //                 mc := add(mc, 0x20)
        //                 cc := add(cc, 0x20)
        //             } {
        //                 mstore(mc, mload(cc))
        //             }

        //             mstore(tempBytes, _length)

        //             //update free-memory pointer
        //             //allocating the array padded to 32 bytes like the compiler does now
        //             mstore(0x40, and(add(mc, 31), not(31)))
        //         }
        //         //if we want a zero-length slice let's just return a zero-length array
        //         default {
        //             tempBytes := mload(0x40)
        //             //zero out the 32 bytes slice we are about to return
        //             //we need to do it because Solidity does not garbage collect
        //             mstore(tempBytes, 0)

        //             mstore(0x40, add(tempBytes, 0x20))
        //         }
        // }

        // return tempBytes;
    }

    pub fn to_address(bytes: &Vec<u8>, start: u32) -> AccountId {
        assert!(start + 20 >= start, "toAddress_overflow");
        assert!(bytes.len() as u32 >= start + 20, "toAddress_outOfBounds");
        // address tempAddress;

        // assembly {
        //     tempAddress := div(mload(add(add(bytes, 0x20), start)), 0x1000000000000000000000000)
        // }

        // return tempAddress;
        let mut b=[0x0;32];
        for i in 0..32{
            b[i]=bytes[i+start as usize];
        }

        AccountId::from(b)
    }

    pub fn to_uint24(bytes: &Vec<u8>, start: u32) -> u32 {
        assert!(start + 3 >= start, "toUint24_overflow");
        assert!(bytes.len() as u32>= start + 3, "toUint24_outOfBounds");
        // uint24 tempUint;

        // assembly {
        //     tempUint := mload(add(add(bytes, 0x3), start))
        // }

        // return tempUint;
        let i = start as usize + 3;
        let temp_uint:[u8;4]=[0,bytes[i],bytes[i+1],bytes[i+2]];
        u32::from_be_bytes(temp_uint)
    }
}
