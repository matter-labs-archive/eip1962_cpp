pub const G1ADD_OPERATION_RAW_VALUE: u8 = OperationType::G1ADD as u8;
pub const G1MUL_OPERATION_RAW_VALUE: u8 = OperationType::G1MUL as u8;
pub const G1MULTIEXP_OPERATION_RAW_VALUE: u8 = OperationType::G1MULTIEXP as u8;

pub const G2ADD_OPERATION_RAW_VALUE: u8 = OperationType::G2ADD as u8;
pub const G2MUL_OPERATION_RAW_VALUE: u8 = OperationType::G2MUL as u8;
pub const G2MULTIEXP_OPERATION_RAW_VALUE: u8 = OperationType::G2MULTIEXP as u8;

pub const BLS12PAIR_OPERATION_RAW_VALUE: u8 = OperationType::BLS12PAIR as u8;
pub const BNPAIR_OPERATION_RAW_VALUE: u8 = OperationType::BNPAIR as u8;
pub const MNT4PAIR_OPERATION_RAW_VALUE: u8 = OperationType::MNT4PAIR as u8;
pub const MNT6PAIR_OPERATION_RAW_VALUE: u8 = OperationType::MNT6PAIR as u8;


#[repr(u8)]
#[derive(Copy, Clone, Debug)]
pub enum OperationType {
    G1ADD = 1,
    G1MUL = 2,
    G1MULTIEXP = 3,
    G2ADD = 4,
    G2MUL = 5,
    G2MULTIEXP = 6,
    BLS12PAIR = 7,
    BNPAIR = 8,
    MNT4PAIR = 9,
    MNT6PAIR = 10,
}

impl OperationType {
    pub fn from_u8(value: u8) -> Option<Self> {
        match value {
            G1ADD_OPERATION_RAW_VALUE => {
                Some(OperationType::G1ADD)
            },
            G1MUL_OPERATION_RAW_VALUE => {
                Some(OperationType::G1MUL)
            },
            G1MULTIEXP_OPERATION_RAW_VALUE => {
                Some(OperationType::G1MULTIEXP)
            },
            G2ADD_OPERATION_RAW_VALUE => {
                Some(OperationType::G2ADD)
            },
            G2MUL_OPERATION_RAW_VALUE => {
                Some(OperationType::G2MUL)
            },
            G2MULTIEXP_OPERATION_RAW_VALUE => {
                Some(OperationType::G2MULTIEXP)
            },
            BLS12PAIR_OPERATION_RAW_VALUE => {
                Some(OperationType::BLS12PAIR)
            },
            BNPAIR_OPERATION_RAW_VALUE => {
                Some(OperationType::BNPAIR)
            },
            MNT4PAIR_OPERATION_RAW_VALUE => {
                Some(OperationType::MNT4PAIR)
            },
            MNT6PAIR_OPERATION_RAW_VALUE => {
                Some(OperationType::MNT6PAIR)
            },
            _ => {
                None
            }
        }
    }

    pub fn as_u8(&self) -> u8 {
        *self as u8
    }
}