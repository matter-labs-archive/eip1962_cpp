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

template <class E>
std::vector<u64> deserialize_scalar(WeierstrassCurve<E> const &wc, Deserializer &deserializer)
{
    auto scalar = deserializer.dyn_number(wc.order_len(), "Input is not long enough to get scalar");
    if (greater_or_equal(scalar, wc.subgroup_order()))
    {
        input_err("Group order is less or equal scalar");
    }
    return scalar;
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

u8 deserialize_pairing_curve_type(Deserializer &deserializer)
{
    auto const curve_byte = deserializer.byte("Input should be longer than curve type encoding");
    switch (curve_byte)
    {
    case BLS12:
    case BN:
    case MNT4:
    case MNT6:
        break;
    default:
        input_err("Unknown curve type");
    }
    return curve_byte;
}

TwistType deserialize_pairing_twist_type(Deserializer &deserializer)
{
    auto const twist_byte = deserializer.byte("Input is not long enough to get twist type");
    switch (twist_byte)
    {
    case TWIST_TYPE_D:
        return D;
    case TWIST_TYPE_M:
        return M;
    default:
        unknown_parameter_err("Unknown twist type supplied");
    }
}

// *************************** SPECIAL PRIMITIVE deserialization *********************** //

template <usize N>
Repr<N> deserialize_modulus(u8 mod_byte_len, Deserializer &deserializer)
{
    if (deserializer.peek_byte("Input is not long enough to get modulus") == 0)
    {
        input_err("In modulus encoding highest byte is zero");
    }
    auto modulus = deserializer.number<N>(mod_byte_len, "Input is not long enough to get modulus");
    constexpr Repr<N> zero = {0};
    if (modulus == zero)
    {
        unexpected_zero_err("Modulus can not be zero");
    }
    if (is_even(modulus))
    {
        input_err("Modulus is even");
    }
    constexpr Repr<N> three = {3};
    if (modulus < three)
    {
        input_err("Modulus is less than 3");
    }
    return modulus;
}

template <class F, class C>
F deserialize_non_residue(u8 mod_byte_len, C const &field, u8 extension_degree, Deserializer &deserializer)
{
    F const non_residue = deserialize_fpM(mod_byte_len, field, deserializer);
    // auto x = deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp element");
    // auto const non_residue = Fp<N>::from_repr(x, field);
    if (non_residue.is_zero())
    {
        unexpected_zero_err("Fp* non-residue can not be zero");
    }

    if (!non_residue.is_non_nth_root(extension_degree))
    {
        input_err("Non-residue for Fp* is actually a residue");
    }

    return non_residue;
}

// ************************* CURVE deserializers ***************************** //

u8 deserialize_group_order_length(Deserializer &deserializer) 
{
    auto order_len = deserializer.byte("Input is not long enough to get group size length");
    if (order_len > MAX_GROUP_BYTE_LEN) {
        input_err("Group order length is too large");
    }

    return order_len;
}

std::vector<u64> deserialize_group_order(u8 order_len, Deserializer &deserializer) 
{
    auto order = deserializer.dyn_number(order_len, "Input is not long enough to get main group order size");

    return order;
}

template <class F, class C>
WeierstrassCurve<F> deserialize_weierstrass_curve(u8 mod_byte_len, C const &field, Deserializer &deserializer, bool a_must_be_zero)
{
    F a = deserialize_fpM(mod_byte_len, field, deserializer);
    F b = deserialize_fpM(mod_byte_len, field, deserializer);

    if (a_must_be_zero && !a.is_zero())
    {
        unknown_parameter_err("A parameter must be zero");
    }

    if (b.is_zero()) {
        input_err("curve shape is not supported");
    }

    auto order_len = deserialize_group_order_length(deserializer);
    auto order = deserialize_group_order(order_len, deserializer);

    auto zero = is_zero(order);
    if (zero)
    {
        input_err("Group order is zero");
    }

    return WeierstrassCurve(a, b, order, order_len);
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

template <class F, class C>
CurvePoint<F> deserialize_curve_point(u8 mod_byte_len, C const &field, WeierstrassCurve<F> const &wc, Deserializer &deserializer)
{
    F x = deserialize_fpM(mod_byte_len, field, deserializer);
    F y = deserialize_fpM(mod_byte_len, field, deserializer);
    auto const cp = CurvePoint(x, y);

    if (!cp.check_on_curve(wc))
    {
        if (!in_fuzzing()) {
            input_err("Point is not on curve");
        }
    }

    return cp;
}

// ********************** POINTS deserialization ******************************* //
template <usize N, class F, class C>
std::vector<std::tuple<CurvePoint<Fp<N>>, CurvePoint<F>>> deserialize_points(u8 mod_byte_len, C const &field, WeierstrassCurve<Fp<N>> const &g1_curve, WeierstrassCurve<F> const &g2_curve, Deserializer &deserializer)
{
    // deser (CurvePoint<Fp<N>>,CurvePoint<F>) pairs
    auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
    if (num_pairs == 0)
    {
        input_err("Zero pairs encoded");
    }

    std::vector<std::tuple<CurvePoint<Fp<N>>, CurvePoint<F>>> points;
    for (auto i = 0; i < num_pairs; i++)
    {
        auto const g1 = deserialize_curve_point<Fp<N>, PrimeField<N>>(mod_byte_len, field, g1_curve, deserializer);
        auto const g2 = deserialize_curve_point<F>(mod_byte_len, field, g2_curve, deserializer);

        if (!g1.check_correct_subgroup(g1_curve, field) || !g2.check_correct_subgroup(g2_curve, field))
        {
            if (!in_fuzzing()) {
                input_err("G1 or G2 point is not in the expected subgroup");
            }
        }

        if (!g1.is_zero() && !g2.is_zero()) {
            points.push_back(std::tuple(g1, g2));
        }
    }

    return points;
}