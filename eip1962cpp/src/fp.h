#ifndef H_FP
#define H_FP

#include "common.h"
// #include "element.h"
#include "repr.h"
#include "field.h"

using namespace cbn::literals;

template <usize N>
class Fp // : public Element<Fp<N>>
{
    PrimeField<N> const &field;
    Repr<N> repr;

public:
    Fp(Repr<N> repr, PrimeField<N> const &field) : field(field), repr(repr) {}

    static Fp<N> from_repr(Repr<N> repr, PrimeField<N> const &field)
    {
        auto fpo = Fp::from_repr_try(repr, field);
        if (fpo)
        {
            return fpo.value();
        }
        else
        {
            api_err("not an element of the field");
        }
    }

    Fp(Fp<N> const &other) : Fp(other.repr, other.field) {}

    auto inline operator=(Fp<N> const &other)
    {
        repr = other.repr;
    }

    Repr<N> const representation() const
    {
        return repr;
    }

    ~Fp() {}

    // ************************* ELEMENT impl ********************************* //
    template <class C>
    static Fp<N> one(C const &context)
    {
        PrimeField<N> const &field = context;
        return Fp(field.mont_r(), field);
    }

    template <class C>
    static Fp<N> zero(C const &context)
    {
        constexpr Repr<N> zero = {0};
        PrimeField<N> const &field = context;
        return Fp(zero, field);
    }

    Fp<N> one() const
    {
        return Fp::one(field);
    }

    Fp<N> zero() const
    {
        return Fp::zero(field);
    }

    Fp<N> inline self()
    {
        Fp<N> const s = *this;
        return s;
        // return this;
    }

    Fp<N> inline const self() const
    {
        Fp<N> const s = *this;
        return s;
        // return this;
    }

    // Serializes bytes from number to BigEndian u8 format.
    void serialize(u8 mod_byte_len, std::vector<u8> &data) const
    {
        auto const normal_repr = into_repr();
        for (i32 i = i32(mod_byte_len) - 1; i >= 0; i--)
        {
            auto const j = i / sizeof(u64);
            if (j < N)
            {

                auto const off = (i - j * sizeof(u64)) * 8;
                data.push_back(normal_repr[j] >> off);
            }
            else
            {
                data.push_back(0);
            }
        }
    }

    Option<Fp<N>> inverse() const
    {
        return new_mont_inverse();
        // return mont_inverse();
    }

    void inline square()
    {
        // repr = cbn::montgomery_square_alt(repr, field.mod(), field.mont_inv());
        // repr = cbn::montgomery_square(repr, field.mod(), field.mont_inv());
        repr = cbn::montgomery_mul(repr, repr, field.mod(), field.mont_inv());
    }

    void inline mul2()
    {
        // repr = cbn::alt_mod_add(repr, repr, field.mod());
        // repr = cbn::mod_add(repr, repr, field.mod());
        repr = cbn::overflowing_shift_left(repr, 1);
        if (repr >= field.mod()) {
            repr = cbn::alt_subtract_ignore_carry(repr, field.mod());
        }
    }

    void inline mul(Fp<N> const e)
    {
        // repr = cbn::montgomery_mul_alt(repr, e.repr, field.mod(), field.mont_inv());
        // cbn::inplace_montgomery_mul(repr, e.repr, field.mod(), field.mont_inv());
        repr = cbn::montgomery_mul(repr, e.repr, field.mod(), field.mont_inv());
    }

    void inline sub(Fp<N> const e)
    {
        repr = cbn::alt_mod_sub(repr, e.repr, field.mod());
        // repr = cbn::mod_sub(repr, e.repr, field.mod());
    }

    void inline add(Fp<N> const e)
    {
        repr = cbn::alt_mod_add(repr, e.repr, field.mod());
        // repr = cbn::mod_add(repr, e.repr, field.mod());
    }

    void inline negate()
    {
        if (!is_zero())
        {
            repr = cbn::alt_subtract_ignore_carry(field.mod(), repr);
            // repr = cbn::subtract_ignore_carry(field.mod(), repr);
        }
    }

    bool inline is_zero() const
    {
        return cbn::is_zero(repr);
    }

    bool inline operator==(Fp<N> const other) const
    {
        return repr == other.repr;
    }

    bool inline operator!=(Fp<N> const other) const
    {
        return repr != other.repr;
    }

    // *************** impl ************ //
    template <usize M>
    auto pow(Repr<M> const e) const
    {
        auto res = one();
        auto const base = self();
        auto found_one = false;

        for (auto it = RevBitIterator(e); it.before();)
        {
            auto i = *it;
            if (found_one)
            {
                res.square();
            }
            else
            {
                found_one = i;
            }

            if (i)
            {
                res.mul(base);
            }
        }

        return res;
    }

    bool is_non_nth_root(u64 n) const
    {
        if (is_zero())
        {
            return false;
        }

        constexpr Repr<N> one = {1};

        auto power = field.mod();
        power = cbn::subtract_ignore_carry(power, one);
        Repr<N> rdiv = {n};
        auto const div_res = cbn::div(power, rdiv);
        auto const rem = div_res.remainder;
        if (!cbn::is_zero(rem))
        {
            return false;
        }
        power = div_res.quotient;

        auto l = this->pow(power);
        auto e_one = this->one();

        return l != e_one;
    }

    Repr<N> into_repr() const
    {
        return cbn::montgomery_reduction(cbn::detail::pad<N>(repr), field.mod(), field.mont_inv());
    }

private:
    static Option<Fp<N>> from_repr_try(Repr<N> repr, PrimeField<N> const &field)
    {
        if (field.is_valid(repr))
        {
            Fp<N> r1 = Fp(repr, field);
            Fp<N> r2 = Fp(field.mont_r2(), field);

            r1.mul(r2);

            return r1;
        }
        else
        {
            return {};
        }
    }

    Option<Fp<N>> mont_inverse() const
    {
        if (is_zero())
        {
            return {};
        }

        // The Montgomery Modular Inverse - Revisited

        // Phase 1
        auto const modulus = field.mod();
        auto u = modulus;
        auto v = repr;
        Repr<N> r = {0};
        Repr<N> s = {1};
        u64 k = 0;

        auto found = false;
        for (usize i = 0; i < N * 128; i++)
        {
            if (cbn::is_zero(v))
            {
                found = true;
                break;
            }
            if (cbn::is_even(u))
            {
                u = cbn::div2(u);
                s = cbn::mul2(s);
            }
            else if (cbn::is_even(v))
            {
                v = cbn::div2(v);
                r = cbn::mul2(r);
            }
            else if (u > v)
            {
                u = cbn::subtract_ignore_carry(u, v);
                u = cbn::div2(u);
                r = cbn::add_ignore_carry(r, s);
                s = cbn::mul2(s);
            }
            else if (v >= u)
            {
                v = cbn::subtract_ignore_carry(v, u);
                v = cbn::div2(v);
                s = cbn::add_ignore_carry(s, r);
                r = cbn::mul2(r);
            }

            k += 1;
        }

        if (!found)
        {
            return {};
        }

        if (r >= modulus)
        {
            r = cbn::subtract_ignore_carry(r, modulus);
        }

        r = cbn::subtract_ignore_carry(modulus, r);

        // phase 2

        auto const mont_power_param = field.mont_power();
        if (k < mont_power_param)
        {
            return {};
        }

        for (usize i = 0; i < (k - mont_power_param); i++)
        {
            if (cbn::is_even(r))
            {
                r = cbn::div2(r);
            }
            else
            {
                r = cbn::add_ignore_carry(r, modulus);
                r = cbn::div2(r);
            }
        }

        auto const el = Fp::from_repr_try(r, field);
        if (el)
        {
            return el.value();
        }
        else
        {
            return {};
        }
    }

    Option<Fp<N>> new_mont_inverse() const
    {
        if (is_zero())
        {
            return {};
        }

        // The Montgomery Modular Inverse - Revisited

        // Phase 1
        auto const modulus = field.mod();
        auto u = modulus;
        auto v = repr;
        Repr<N> r = {0};
        Repr<N> s = {1};
        u64 k = 0;

        auto found = false;
        for (usize i = 0; i < N * 128; i++)
        {
            if (cbn::is_zero(v))
            {
                found = true;
                break;
            }
            if (cbn::is_even(u))
            {
                u = cbn::div2(u);
                s = cbn::mul2(s);
            }
            else if (cbn::is_even(v))
            {
                v = cbn::div2(v);
                r = cbn::mul2(r);
            }
            else if (u > v)
            {
                u = cbn::subtract_ignore_carry(u, v);
                u = cbn::div2(u);
                r = cbn::add_ignore_carry(r, s);
                s = cbn::mul2(s);
            }
            else if (v >= u)
            {
                v = cbn::subtract_ignore_carry(v, u);
                v = cbn::div2(v);
                s = cbn::add_ignore_carry(s, r);
                r = cbn::mul2(r);
            }

            k += 1;
        }

        if (!found)
        {
            return {};
        }

        if (r >= modulus)
        {
            r = cbn::subtract_ignore_carry(r, modulus);
        }

        r = cbn::subtract_ignore_carry(modulus, r);

        // phase 2

        auto const mont_power = field.mont_power();
        auto const modulus_bits_ceil = field.modulus_bits();
        auto const k_in_range = (modulus_bits_ceil <= k) && (k <= mont_power + modulus_bits_ceil);
        if (!k_in_range)
        {
            return {};
        }

        if ( (modulus_bits_ceil <= k) && (k <= mont_power) ) {
            Fp<N> r1 = Fp(r, field);
            Fp<N> r2 = Fp(field.mont_r2(), field);

            r1.mul(r2);
            r = r1.repr;
            k += mont_power;
        }

        if (k > 2*mont_power) {
            return {};
        }

        if (2*mont_power - k > mont_power) {
            // we are not in range
            return {};
        }

        Repr<N> two_in_two_m_minus_k_repr = {1};
        auto shift_amount = 2*mont_power - k;
        // while (shift_amount >= 64) {
        //     two_in_two_m_minus_k_repr = cbn::overflowing_shift_left(two_in_two_m_minus_k_repr, 64);
        //     shift_amount -= 64;
        // }

        while (shift_amount >= 64) {
            two_in_two_m_minus_k_repr = cbn::overflowing_shift_left(two_in_two_m_minus_k_repr, 32);
            shift_amount -= 32;
        }

        two_in_two_m_minus_k_repr = cbn::overflowing_shift_left(two_in_two_m_minus_k_repr, shift_amount);

        Fp<N> r1 = Fp(r, field);
        Fp<N> r2 = Fp(two_in_two_m_minus_k_repr, field);

        r1.mul(r2);
        r = r1.repr;

        auto const el = Fp::from_repr_try(r, field);
        if (el)
        {
            return el.value();
        }
        else
        {
            return {};
        }
    }
};

template <usize N>
std::ostream &operator<<(std::ostream &strm, Fp<N> num) {
    strm << "Fp(" << num.into_repr() << ")";
    return strm;
}

#endif