#include "common.h"
#include "deserialization.h"
#include "constants.h"

// True if minus
bool deserialize_sign(Deserializer &deserializer)
{
    auto sign = deserializer.byte("Input is not long enough to get sign encoding");
    switch (sign)
    {
    case SIGN_PLUS:
        return false;
    case SIGN_MINUS:
        return true;

    default:
        input_err("sign is not encoded properly");
    }
}

u64 num_units_for_group_order(const std::vector<u64> &order)
{
    auto bits = num_bits(order);
    auto limbs = (bits + 63) / 64;
    if (limbs < NUM_GROUP_LIMBS_MIN) {
        input_err("group order is zero");
    } else if (limbs > NUM_GROUP_LIMBS_MAX) {
        input_err("group order is too large");
    }

    return u64(limbs);
}

std::vector<u64> deserialize_scalar_with_bit_limit(usize bit_limit, Deserializer &deserializer)
{
    auto const length = deserializer.byte("Input is not long enough to get scalar length");
    auto const max_length_for_bits = (bit_limit + 7) / 8;
    if (length > max_length_for_bits)
    {
        input_err("Scalar is too larget for bit length");
    }
    auto const num = deserializer.dyn_number(length, "Input is not long enough to get scalar");
    if (num_bits(num) > bit_limit)
    {
        input_err("Number of bits for scalar is too large");
    }
    return num;
}