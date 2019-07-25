#ifndef __SLICING_HPP
#define __SLICING_HPP

#include <cstddef>

template <size_t Begin, size_t End, size_t Padding=0, 
          typename T, size_t N1>
constexpr T take(std::array<T, N1> t) {
  //static_assert(End >= Begin, "invalid range");
  //static_assert(End - Begin <= N1, "invalid range");

  std::array<T, End - Begin + Padding> res{};
  for (auto i = Begin; i < End; ++i) {
    res[i-Begin] = t[i];
  }

  return res;
}

template <size_t ResultLength, 
          typename T, size_t N1>
constexpr T take(std::array <T, N1> t, const size_t Begin, const size_t End, const size_t Offset = 0) {

  std::array<T, ResultLength> res{};
  for (auto i = Begin; i < End; ++i) {
    res[i-Begin+Offset] = t[i];
  }

  return res;
}

template <size_t N, size_t Padding = 0, typename T,
          size_t N1>
constexpr T skip(std::array <T, N1> t) {
  // skip first N limbs
  // skip<N>(x) corresponds with right-shifting x by N limbs
  return take<N, N1, Padding>(t);
}

template <typename T, size_t N1>
constexpr T skip(std::array <T, N1> t, size_t N) {
  // skip first N limbs, runtime version
  // skip<N>(x) corresponds with right-shifting x by N limbs
  return take<N1>(t, N, N1);
}

template <size_t N, typename T,
          size_t N1>
constexpr T first(std::array <T, N1> t) {
  // take first N limbs
  // first<N>(x) corresponds with x modulo (2^64)^N
  return take<0, N>(t);
}

#endif

