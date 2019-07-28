#ifndef H_FIELD
#define H_FIELD

#include "common.h"
#include "repr.h"
//#include "ctbignum/slicing.hpp"

template <usize N>
class PrimeField
{
    std::array<uint64_t, N> modulus;
    u64 mont_power_;
	std::array<uint64_t, N+1> mont_r_;
	std::array<uint64_t, 2*(N+1)> mont_r2_;
    u64 mont_inv_;

public:
    PrimeField(std::array<uint64_t, N> modulus) : modulus(modulus), mont_power_(N * LIMB_BITS)
    {
        // Compute -m^-1 mod 2**64 by exponentiating by totient(2**64) - 1
        u64 inv = 1;
        for (auto i = 0; i < 63; i++)
        {
            inv = inv * inv;
            inv = inv * modulus[0];
        }
        inv = (std::numeric_limits<u64>::max() - inv) + 2 + std::numeric_limits<u64>::max();
        mont_inv_ = inv;

		std::array<uint64_t, N+1> pow_N_LIMB_BITS = {0};
        pow_N_LIMB_BITS[N] = 1;
        mont_r_ = pow_N_LIMB_BITS % modulus;

        mont_r2_ = (mont_r_ * mont_r_) % modulus;
    }

	std::array<uint64_t, N> mod()
    {
        return modulus;
    }

	std::array<uint64_t, N> mont_r()
    {
        return mont_r_;
    }

	std::array<uint64_t, N> mont_r2()
    {
        return mont_r2_;
    }

    u64 mont_power() const
    {
        return mont_power_;
    }

    // Montgomery parametare for multiplication
    u64 mont_inv()
    {
        return mont_inv_;
    }

    bool is_valid(std::array<uint64_t, N> const &repr)
    {
        return repr < modulus;
    }
};

#endif