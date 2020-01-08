#ifndef H_DESERIALIZATION
#define H_DESERIALIZATION

#include "features.h"
#include "common.h"
#include "repr.h"
#include "field.h"
#include "curve.h"
#include "extension_towers/fp2.h"
#include "extension_towers/fp3.h"

// *************************** PRIMITIVE deserialization *********************** //

class Deserializer
{
    std::vector<uint8_t>::const_iterator begin;
    std::vector<uint8_t>::const_iterator const end;

public:
    Deserializer(std::vector<std::uint8_t> const &input) : begin(input.cbegin()), end(input.cend()) {}

    // Consumes a byte, throws error otherwise
    u8 byte(str &err)
    {
        if (!ended())
        {
            auto ret = *begin;
            begin++;
            return ret;
        }
        else
        {
            input_err(err);
        }
    }

    // Looks at a byte, throws error otherwise
    u8 peek_byte(str &err) const
    {
        if (!ended())
        {
            return *begin;
        }
        else
        {
            input_err(err);
        }
    }

    // Deserializes number in Big endian format with bytes.
    template <usize N>
    Repr<N> number(u8 bytes, str &err)
    {
        Repr<N> num = {0};
        read(bytes, num, err);
        return num;
    }

    // Deserializes number in Big endian format with bytes.
    std::vector<u64> dyn_number(u8 bytes, str &err)
    {
        std::vector<u64> num;
        num.resize((bytes + sizeof(u64) - 1) / sizeof(u64), 0);
        read(bytes, num, err);
        return num;
    }

    void advance(u8 bytes, str &err)
    {
        for (auto i = 0; i < bytes; i++)
        {
            byte(err);
        }
    }

    bool ended() const
    {
        return begin == end;
    }

    u32 remaining() const
    {
        return end - begin;
    }

private:
    // Deserializes number in Big endian format with bytes.
    template <class T>
    void read(u8 bytes, T &num, str &err)
    {
        for (auto i = 0; i < bytes; i++)
        {
            auto b = byte(err);
            auto j = bytes - 1 - i;
            auto at = j / sizeof(u64);
            auto off = (j - at * sizeof(u64)) * 8;
            num[at] |= ((u64)b) << off;
        }
    }
};

bool deserialize_sign(Deserializer &deserializer);

template <class E>
std::vector<u64> deserialize_scalar(WeierstrassCurve<E> const &wc, Deserializer &deserializer);

std::vector<u64> deserialize_scalar_with_bit_limit(usize bit_limit, Deserializer &deserializer);

u8 deserialize_pairing_curve_type(Deserializer &deserializer);

TwistType deserialize_pairing_twist_type(Deserializer &deserializer);

template <usize N>
Repr<N> deserialize_modulus(u8 mod_byte_len, Deserializer &deserializer);

template <class F, class C>
F deserialize_non_residue(u8 mod_byte_len, C const &field, u8 extension_degree, Deserializer &deserializer);

u8 deserialize_group_order_length(Deserializer &deserializer);

std::vector<u64> deserialize_group_order(u8 order_len, Deserializer &deserializer);

template <class F, class C>
WeierstrassCurve<F> deserialize_weierstrass_curve(u8 mod_byte_len, C const &field, Deserializer &deserializer, bool a_must_be_zero);

u64 num_units_for_group_order(const std::vector<u64> &order);

template <class F, class C>
CurvePoint<F> deserialize_curve_point(u8 mod_byte_len, C const &field, WeierstrassCurve<F> const &wc, Deserializer &deserializer);

template <usize N, class F, class C>
std::vector<std::tuple<CurvePoint<Fp<N>>, CurvePoint<F>>> deserialize_points(u8 mod_byte_len, C const &field, WeierstrassCurve<Fp<N>> const &g1_curve, WeierstrassCurve<F> const &g2_curve, Deserializer &deserializer);

// ********************* OVERLOADED deserializers of Fp and Fp2 and Fp3 *********************** //

template <usize N>
Fp<N> deserialize_fpM(u8 mod_byte_len, PrimeField<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp_c element"), field);
    return c0;
}

template <usize N>
Fp2<N> deserialize_fpM(u8 mod_byte_len, FieldExtension2<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp2_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp2_c1 element"), field);
    return Fp2<N>(c0, c1, field);
}

template <usize N>
Fp3<N> deserialize_fpM(u8 mod_byte_len, FieldExtension3<N> const &field, Deserializer &deserializer)
{
    auto const c0 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c0 element"), field);
    auto const c1 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c1 element"), field);
    auto const c2 = Fp<N>::from_repr(deserializer.number<N>(mod_byte_len, "Input is not long enough to get Fp3_c2 element"), field);
    return Fp3<N>(c0, c1, c2, field);
}

#endif