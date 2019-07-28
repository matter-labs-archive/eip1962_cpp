#ifndef __RELATIONAL_OPS_HPP
#define __RELATIONAL_OPS_HPP

#include <algorithm>
#include <cstddef>
#include <experimental/array>

template <size_t N, typename T>
[[gnu::always_inline]] bool equal(std::array<T, N> a, std::array<T, N> b) {
  size_t x = 0;
  for (auto i = 0; i < N; ++i)
    x += (a[i] != b[i]);
  return (x == 0);
}

template <size_t N, typename T>
constexpr bool less_than(std::array<T, N> a, std::array<T, N> b) {

  return subtract(a, b)[N];
}

template <typename T, size_t N1, size_t N2>
constexpr bool operator==(std::array<T, N1> a, std::array<T, N2> b) {
  constexpr auto L = std::max(N1, N2);
  return equal(pad<L - N1>(a), pad<L - N2>(b));
}

template <typename T, size_t N1, size_t N2>
constexpr bool operator!=(std::array<T, N1> a, std::array<T, N2> b) {
  return !(a==b);
}

template <typename T, size_t N1, size_t N2>
constexpr bool operator<(std::array<T, N1> a, std::array<T, N2> b) {
  constexpr auto L = std::max(N1, N2);
  return less_than(pad<L - N1>(a), pad<L - N2>(b));
}

template <typename T, size_t N1, size_t N2>
constexpr bool operator>(std::array<T, N1> a, std::array<T, N2> b) {
  return (b < a);
}

template <typename T, size_t N1, size_t N2>
constexpr bool operator<=(std::array<T, N1> a, std::array<T, N2> b) {
  return !(b < a);
}

template <typename T, size_t N1, size_t N2>
constexpr bool operator>=(std::array<T, N1> a, std::array<T, N2> b) {
  return  !(a < b);
}

#endif
