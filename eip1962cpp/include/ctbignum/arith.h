#ifndef CT_ARITH
#define CT_ARITH

#include <ctbignum/type_traits.hpp>
#include <limits>

// Calculate a + b + carry, returning the sum and modifying the
// carry value.
template <typename T>
static inline constexpr T adc(T a, T b, T &carry)
{
  using TT = typename cbn::dbl_bitlen<T>::type;
  auto const tmp = static_cast<TT>(a) + static_cast<TT>(b) + static_cast<TT>(carry);

  carry = tmp >> std::numeric_limits<T>::digits;

  return tmp;
}

// Calculate a - b - borrow, returning the result and modifying
// the borrow value.
template <typename T>
static inline constexpr T sbb(T a, T b, T &borrow)
{
  using TT = typename cbn::dbl_bitlen<T>::type;
  constexpr TT base = TT(1) << std::numeric_limits<T>::digits;;
  auto const tmp = base + static_cast<u128>(a) - static_cast<u128>(b) - static_cast<u128>(borrow);
  if ((tmp >> std::numeric_limits<T>::digits) == 0)
  {
      borrow = 1;
  }
  else
  {
      borrow = 0;
  };

  return tmp;
}

/// A + B * C + carry
template <typename T>
static inline constexpr T mac_with_carry(T a, T b, T c, T &carry)
{
  using TT = typename cbn::dbl_bitlen<T>::type;
  auto const tmp = static_cast<TT>(a) + static_cast<TT>(b) * static_cast<TT>(c) + static_cast<TT>(carry);

  carry = tmp >> std::numeric_limits<T>::digits;

  return tmp;
}

#endif