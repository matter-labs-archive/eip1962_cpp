#ifndef __ADDITION_HPP
#define __ADDITION_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <experimental/array>
#include "integer_sequence.hpp"

template <typename T, size_t M, size_t N>
[[gnu::always_inline]]
constexpr T add(std::array<T,M> a, std::array<T,N> b) {
  constexpr auto L = std::max(M, N);
  return add_same(pad<L - M>(a), detail::pad<L - N>(b));
}

template <typename T, size_t M, size_t N>
[[gnu::always_inline]]
constexpr T subtract(std::array<T, M> a, std::array<T, N> b) {
  constexpr auto L = std::max(M, N);
  return subtract_same(pad<L - M>(a), detail::pad<L - N>(b));
}


template <typename T, size_t N>
[[gnu::always_inline]]
constexpr T add_same(std::array<T, N> a, std::array<T, N> b) {
  T carry{};
  std::array<T,N + 1> r{};

  for (auto i = 0; i < N; ++i) {
    auto aa = a[i];
    auto sum = aa + b[i];
    auto res = sum + carry;
    carry = (sum < aa) | (res < sum);
    r[i] = res;
  }

  r[N] = carry;
  return r;
}

template <typename T, size_t N>
[[gnu::always_inline]]
constexpr T subtract_same(std::array<T, N> a, std::array<T, N> b) {
  T carry{};
  std::array<T,N + 1> r{};

  for (auto i = 0; i < N; ++i) {
    auto aa = a[i];
    auto diff = aa - b[i];
    auto res = diff - carry;
    carry = (diff > aa) | (res > diff);
    r[i] = res;
  }

  r[N] = carry * static_cast<T>(-1); // sign extension
  return r;
}

template <typename T, size_t N>
[[gnu::always_inline]]
constexpr T add_ignore_carry(std::array<T, N> a, std::array<T, N> b) {
  T carry{};
  std::array<T,N> r{};

  for (auto i = 0; i < N; ++i) {
    T aa = a[i];
    T sum = aa + b[i];
    T res = sum + carry;
    carry = (sum < aa) | (res < sum);
    r[i] = res;
  }

  return r;
}

template <typename T, size_t N>
constexpr T subtract_ignore_carry(std::array<T, N> a, std::array<T, N> b) {
  T carry{};
  std::array<T,N> r{};

  for (auto i = 0; i < N; ++i) {
    auto aa = a[i];
    auto diff = aa - b[i];
    auto res = diff - carry;
    carry = (diff > aa) | (res > diff);
    r[i] = res;
  }

  return r;
}

template <typename T, size_t N>
constexpr T mod_add(std::array<T,N> a, std::array<T,N> b,
                       std::array<T,N> modulus) {
  T carry{};
  big_int<N, T> r{};

  for (auto i = 0; i < N; ++i) {
    auto aa = a[i];
    auto sum = aa + b[i];
    auto res = sum + carry;
    carry = (sum < aa) | (res < sum);
    r[i] = res;
  }

  auto reduced = subtract_ignore_carry(r, modulus);
  big_int<N, T> res = (carry + (r >= modulus) != 0) ? reduced : r;
  return res;
}

template <typename T, size_t N>
constexpr T mod_sub(std::array<T,N> a, std::array<T,N> b,
                       std::array<T,N> modulus) {
  T carry{};
  std::array<T, N> r{};

  for (auto i = 0; i < N; ++i) {
    auto aa = a[i];
    auto diff = aa - b[i];
    auto res = diff - carry;
    carry = (diff > aa) | (res > diff);
    r[i] = res;
  }

  auto adjusted_r = add_ignore_carry(r, modulus);
  big_int<N, T> res = carry ? adjusted_r : r;
  return res;
}



template <typename T, size_t N, T... Modulus>
constexpr T mod_add(std::array<T, N> a, std::array<T, N> b, tl::integer_sequence<T, Modulus...>) {
  big_int<sizeof...(Modulus), T> modulus{{Modulus...}};
  return mod_add(a, b, modulus);
}

template <typename T, size_t N1, size_t N2>
constexpr T operator+(std::array<T,N1> a, std::array<T,N2> b) {
  return add(a, b);
}

template <typename T, size_t N1, size_t N2>
constexpr T operator-(std::array<T, N1> a, std::array<T, N2> b) {
  return subtract(a, b);
}

template <typename T, size_t N, T... Modulus>
constexpr T mod_sub(std::array<T, N> a, std::array<T, N> b, tl::integer_sequence<T, Modulus...>) {
  big_int<sizeof...(Modulus), T> modulus{{Modulus...}};
  return mod_sub(a, b, modulus);
}

#endif
