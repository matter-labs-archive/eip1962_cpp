#ifndef __DIVISION_HPP
#define __DIVISION_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <array>

template <typename Q, typename R> struct DivisionResult {
  Q quotient;
  R remainder;
};

template <size_t M, typename T> constexpr  
DivisionResult<std::array<T,M>,std::array<T,1>>
short_div(std::array<T,M> u, T v) {
  using TT = typename dbl_bitlen<T>::type;
  TT r{0};
  std::array<T,M> q{};
  for (int i = M - 1; i >= 0; --i) {
    TT w = (r << std::numeric_limits<T>::digits) + u[i];
    q[i] = w / v;
    r = w % v;
  }
  return {q, {static_cast<T>(r)}};
}

template <size_t M, size_t N, typename T>
constexpr DivisionResult<std::array<T,M>, std::array<T,N>> div(std::array<T,M> u,
	std::array<T,N> v) {

  using TT = typename dbl_bitlen<T>::type;
  size_t tight_N = N;
  while (tight_N > 0 && v[tight_N - 1] == 0)
    --tight_N;

  std::array<T,M> q{};

  if (tight_N == 1) { // short division
    TT r {};
    for (int i = M - 1; i >= 0; --i) {
      TT w = (r << std::numeric_limits<T>::digits) + u[i];
      q[i] = w / v[0];
      r = w % v[0];
    }
    return {q, {static_cast<T>(r)}};
  }

  uint8_t k = 0;
  while (v[tight_N - 1] <
         (static_cast<T>(1) << (std::numeric_limits<T>::digits - 1))) {
    ++k;
    v = first<N>(shift_left(v, 1));
  }
  auto us = shift_left(u, k);

  for (int j = M - tight_N; j >= 0; --j) {
    TT tmp = us[j + tight_N - 1];
    TT tmp2 = us[j + tight_N];
    tmp += (tmp2 << std::numeric_limits<T>::digits);
    TT qhat = tmp / v[tight_N - 1];
    TT rhat = tmp % v[tight_N - 1];

    auto b = static_cast<TT>(1) << std::numeric_limits<T>::digits;
    while (qhat == b ||
           (qhat * v[tight_N - 2] >
            (rhat << std::numeric_limits<T>::digits) + us[j + tight_N - 2])) {
      qhat -= 1;
      rhat += v[tight_N - 1];
      if (rhat >= b)
        break;
    }
    auto true_value = subtract(take<N + 1>(us, j, j + tight_N + 1),
                               mul(v, std::array<T,1>{{static_cast<T>(qhat)}}));
    if (true_value[tight_N]) {
      auto corrected =
          add_ignore_carry(true_value, unary_encoding<N + 2, T>(tight_N + 1));
      auto new_us_part = add_ignore_carry(corrected, pad<2>(v));
      for (auto i = 0; i <= tight_N; ++i)
        us[j + i] = new_us_part[i];
      --qhat;
    } else {
      for (auto i = 0; i <= tight_N; ++i)
        us[j + i] = true_value[i];
    }
    q[j] = qhat;
  }
  return {q, shift_right(first<N>(us), k) };
}

template <typename T, size_t N1, size_t N2>
constexpr T operator/(std::array<T,N1> a, std::array<T,N2> b) {
  return div(a, b).quotient;
}

template <typename T, size_t N1, size_t N2>
constexpr T operator%(std::array<T, N1> a, std::array<T, N2> b) {
  return div(a, b).remainder;
}

#endif
