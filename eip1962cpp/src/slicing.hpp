#ifndef __SLICING_HPP
#define __SLICING_HPP

#include <cstddef>

template <size_t Begin, size_t End, size_t Padding=0, 
          typename T, size_t N1>
	[[gnu::always_inline]]
T take(std::array<T, N1> t) {

  std::array<T, End - Begin + Padding> res{};
  for (auto i = Begin; i < End; ++i) {
    res[i-Begin] = t[i];
  }

  return res;
}

template <size_t ResultLength, typename T, size_t N1>
[[gnu::always_inline]]
T take(std::array <T, N1> t, const size_t Begin, const size_t End, const size_t Offset = 0) {

  std::array<T, ResultLength> res{};
  for (auto i = Begin; i < End; ++i) {
    res[i-Begin+Offset] = t[i];
  }

  return res;
}

template <size_t N, typename T,	size_t N1>
[[gnu::always_inline]]
	T first(std::array <T, N1> t) {
	return take<0, N>(t);
}

template <size_t N, typename T,	size_t N1>
[[gnu::always_inline]]
	T pad(std::array<T,N1> t) {
	// add N extra limbs (at msb side)
	return take<0, N1, N>(t);
}

template <size_t N, typename T, size_t N1>
[[gnu::always_inline]]
	T to_length(std::array<T, N1> t) {
	return (N1 < N) ? pad<N - N1>(t) : first<N>(t);
}

template <typename T,
	size_t N1, size_t N2>
	[[gnu::always_inline]]
	T join(std::array<T, N1> a, std::array<T, N2> b) {
	std::array<T, N1 + N2> result{};

	for (auto i = 0; i < N1; ++i)
		result[i] = a[i];

	for (auto i = 0; i < N2; ++i)
		result[N1 + i] = b[i];

	return result;
}

template <size_t N, size_t Padding = 0, typename T,
          size_t N1>
	[[gnu::always_inline]]
	T skip(std::array <T, N1> t) {
  
	return take<N, N1, Padding>(t);
}

template <typename T, size_t N1>
[[gnu::always_inline]]
	T skip(std::array <T, N1> t, size_t N) {

	return take<N1>(t, N, N1);
}


template <size_t ResultLength, typename T,
	size_t N1>
	[[gnu::always_inline]]
	T limbwise_shift_left(std::array<T, N1> t, const size_t k) {
	// shift left by k limbs (and produce output of limb-length ResultLength)
	return take<ResultLength>(t, 0, N1, k);
}

template <size_t K, size_t N, typename T = uint64_t>
[[gnu::always_inline]]
T unary_encoding() {
	// N limbs, Kth limb set to one
	std::array<T,N> res{};
	res[K] = 1;
	return res;
}

template <size_t N, typename T = uint64_t>
[[gnu::always_inline]]
T unary_encoding(size_t K) {
	std::array<T, N> res{};
	res[K] = 1;
	return res;
}

template <size_t N, typename T = uint64_t>
[[gnu::always_inline]]
T place_at(uint64_t value, size_t K) {
	// N limbs, Kth limb set to value
	std::array<T, N> res{};
	res[K] = value;
	return res;
}

#endif

