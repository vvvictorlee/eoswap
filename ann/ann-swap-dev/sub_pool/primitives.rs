pub use crate::i256::I256 as OtherI256;
use core::ops::{
    Add, AddAssign, BitAnd, BitOr, BitOrAssign, BitXor, Div, Mul, MulAssign, Neg, Not, Rem, Shl,
    Shr, ShrAssign, Sub, SubAssign,
};
pub use sp_core::U256 as OtherU256;
// pub type U256=u128;
// pub type U160=u128;
pub type U256 = WrapperU256;
pub type U160 = WrapperU256;
pub type I256 = OtherI256;
// pub type U160=U256;
use ink_prelude::string::String;
use ink_prelude::vec::Vec;
use ink_primitives::Key;
use ink_storage::traits::{ExtKeyPtr, PackedAllocate, PackedLayout, SpreadAllocate, SpreadLayout};

// #[derive(Default,Debug,Clone,Copy, PartialEq, Eq,Encode, Decode)]
// #[cfg_attr(feature = "std", derive(TypeInfo))]            ink_storage::traits::StorageLayout

#[derive(Default, Debug, scale::Encode, scale::Decode, PartialEq, Eq, PartialOrd, Copy, Clone)]
#[cfg_attr(feature = "std", derive(scale_info::TypeInfo,))]
pub struct WrapperU256 {
    pub value: OtherU256,
}
impl WrapperU256 {
    pub const MAX: Self = Self {
        value: OtherU256::MAX,
    };
    pub fn new() -> Self {
        Self::zero()
    }
    pub fn zero() -> Self {
        Self {
            value: OtherU256::zero(),
        }
    }

    pub fn one() -> Self {
        Self {
            value: OtherU256::one(),
        }
    }
    pub fn is_zero(&self) -> bool {
        self.value.is_zero()
    }
    pub fn from_hex_str(value: &str) -> Self {
        let v: Vec<u8> = (if &value[..2] == "0x" {
            &value[2..]
        } else {
            value
        })
        .bytes()
        .collect();
        let vv: Vec<u8> = v
            .chunks(2)
            .map(|x| {
                u8::from_str_radix(String::from_utf8(x.to_vec()).unwrap().as_str(), 16).unwrap()
            })
            .collect();
        Self {
            value: OtherU256::from_big_endian(&vv[..]),
        }
    }
    pub fn from_dec_str(value: &str) -> Self {
        Self {
            value: OtherU256::from_dec_str(value).unwrap(),
        }
    }
    pub fn as_u128(&self) -> u128 {
        self.value.as_u128()
    }
    pub fn saturating_add(self, other: Self) -> Self {
        Self {
            value: self.value.saturating_add(other.value),
        }
    }
    pub fn saturating_sub(self, other: Self) -> Self {
        Self {
            value: self.value.saturating_sub(other.value),
        }
    }
}

impl From<i32> for WrapperU256 {
    fn from(n: i32) -> Self {
        Self {
            value: OtherU256::from(n),
        }
    }
}

impl From<u8> for WrapperU256 {
    fn from(n: u8) -> Self {
        Self {
            value: OtherU256::from(n),
        }
    }
}
impl From<u16> for WrapperU256 {
    fn from(n: u16) -> Self {
        Self {
            value: OtherU256::from(n),
        }
    }
}

impl From<u32> for WrapperU256 {
    fn from(n: u32) -> Self {
        Self {
            value: OtherU256::from(n),
        }
    }
}

impl From<u64> for WrapperU256 {
    fn from(n: u64) -> Self {
        Self {
            value: OtherU256::from(n),
        }
    }
}

impl From<u128> for WrapperU256 {
    fn from(n: u128) -> Self {
        Self {
            value: OtherU256::from(n),
        }
    }
}

impl From<OtherI256> for WrapperU256 {
    fn from(n: OtherI256) -> Self {
        Self {
            value: n.into_raw(),
        }
    }
}

impl Add for WrapperU256 {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value.add(rhs.value),
        }
    }
}
impl Sub for WrapperU256 {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value.sub(rhs.value),
        }
    }
}
impl Mul for WrapperU256 {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value.mul(rhs.value),
        }
    }
}
impl Div for WrapperU256 {
    type Output = Self;

    fn div(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value.div(rhs.value),
        }
    }
}
impl Neg for WrapperU256 {
    type Output = Self;

    fn neg(self) -> Self::Output {
        Self {
            value: OtherU256::from_big_endian(&[0xff_u8; 32])
                .saturating_sub(self.value)
                .saturating_add(OtherU256::from(1)),
        }
    }
}

impl Not for WrapperU256 {
    type Output = Self;

    fn not(self) -> Self::Output {
        Self { value: !self.value }
    }
}
impl Rem for WrapperU256 {
    type Output = Self;

    fn rem(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value % rhs.value,
        }
    }
}

impl BitAnd for WrapperU256 {
    type Output = Self;

    // rhs is the "right-hand side" of the expression `a & b`
    fn bitand(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value & rhs.value,
        }
    }
}
impl BitOr for WrapperU256 {
    type Output = Self;

    // rhs is the "right-hand side" of the expression `a | b`
    fn bitor(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value | rhs.value,
        }
    }
}
impl BitXor for WrapperU256 {
    type Output = Self;

    // rhs is the "right-hand side" of the expression `a^b`
    fn bitxor(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value ^ rhs.value,
        }
    }
}

impl Shl for WrapperU256 {
    type Output = Self;

    fn shl(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value << rhs.value,
        }
    }
}

impl Shr for WrapperU256 {
    type Output = Self;

    fn shr(self, rhs: Self) -> Self::Output {
        Self {
            value: self.value >> rhs.value,
        }
    }
}

impl BitOrAssign for WrapperU256 {
    fn bitor_assign(&mut self, rhs: Self) {
        *self = Self {
            value: self.value | rhs.value,
        };
    }
}

impl AddAssign for WrapperU256 {
    fn add_assign(&mut self, rhs: Self) {
        *self = Self {
            value: self.value + rhs.value,
        };
    }
}

impl SubAssign for WrapperU256 {
    // rhs is the "right-hand side" of the expression `a -= b`
    fn sub_assign(&mut self, rhs: Self) {
        *self = Self {
            value: self.value - rhs.value,
        };
    }
}
impl MulAssign for WrapperU256 {
    // rhs is the "right-hand side" of the expression `a *= b`
    fn mul_assign(&mut self, rhs: Self) {
        *self = Self {
            value: self.value * rhs.value,
        };
    }
}
impl ShrAssign for WrapperU256 {
    // rhs is the "right-hand side" of the expression `a >>= b`
    fn shr_assign(&mut self, rhs: Self) {
        *self = Self {
            value: self.value >> rhs.value,
        };
    }
}

// impl PartialOrd for WrapperU256 {
//     fn partial_cmp(&self, other:&Self) -> Option<std::cmp::Ordering> {
//         Some(self.cmp(other))
//     }
// }

// impl Eq for WrapperU256 {
//     fn eq(&self, other:&Self) -> Option<std::cmp::Ordering> {
//         Some(self.value.eq(other.value))
//     }
// }

// impl Ord for WrapperU256 {
//     fn cmp(&self, other: &Self) -> std::cmp::Ordering {
//         // use cmp::Ordering::*;
//         // use Sign::*;
//         self.value.cmp(other.value)
//     }
// }

impl SpreadLayout for WrapperU256 {
    const FOOTPRINT: u64 = 4;
    const REQUIRES_DEEP_CLEAN_UP: bool = true;
    fn pull_spread(ptr: &mut ink_primitives::KeyPtr) -> Self {
        let slice: [u64; 4] = SpreadLayout::pull_spread(ptr);
        Self {
            value: OtherU256(slice),
        }
    }

    fn push_spread(&self, ptr: &mut ink_primitives::KeyPtr) {
        SpreadLayout::push_spread(&self.value.0, ptr);
    }

    fn clear_spread(&self, ptr: &mut ink_primitives::KeyPtr) {
        SpreadLayout::clear_spread(&self.value.0, ptr);
    }
}

impl PackedAllocate for WrapperU256 {
    fn allocate_packed(&mut self, at: &Key) {
        PackedAllocate::allocate_packed(&mut self.value.0, at);
    }
}
impl PackedLayout for WrapperU256 {
    fn pull_packed(&mut self, at: &ink_primitives::Key) {
        self.value.0.pull_packed(at);
    }

    fn push_packed(&self, at: &ink_primitives::Key) {
        self.value.0.push_packed(at);
    }

    fn clear_packed(&self, at: &ink_primitives::Key) {
        self.value.0.clear_packed(at);
    }
}

impl SpreadAllocate for WrapperU256 {
    fn allocate_spread(ptr: &mut ink_primitives::KeyPtr) -> Self {
        ptr.next_for::<WrapperU256>();
        WrapperU256::new()
    }
}

#[cfg(feature = "std")]
const _: () = {
    use ink_metadata::layout::Layout;
    use ink_storage::traits::StorageLayout;

    impl StorageLayout for WrapperU256 {
        fn layout(key_ptr: &mut ink_primitives::KeyPtr) -> Layout {
            <[u64; 4] as StorageLayout>::layout(key_ptr)
        }
    }
};
