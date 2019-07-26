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

template <size_t N, typename T,	size_t N1>
	constexpr T first(std::array <T, N1> t) {
	// take first N limbs
	// first<N>(x) corresponds with x modulo (2^64)^N
	return take<0, N>(t);
}

template <size_t N, typename T,	size_t N1>
	constexpr T pad(std::array<T,N1> t) {
	// add N extra limbs (at msb side)
	return take<0, N1, N>(t);
}

template <size_t N, typename T, size_t N1>
constexpr T to_length(std::array<T, N1> t) {
	return (N1 < N) ? pad<N - N1>(t) : first<N>(t);
}

template <typename T,
	size_t N1, size_t N2>
	constexpr T join(std::array<T, N1> a, std::array<T, N2> b) {
	std::array<T, N1 + N2> result{};

	for (auto i = 0; i < N1; ++i)
		result[i] = a[i];

	for (auto i = 0; i < N2; ++i)
		result[N1 + i] = b[i];

	return result;
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

#endif

