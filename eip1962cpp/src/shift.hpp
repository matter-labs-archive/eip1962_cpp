#ifndef __SHIFT_HPP
#define __SHIFT_HPP

template <size_t N, typename T>
constexpr T shift_right(std::array<T,N> a, size_t k) {
  // shift-right the big integer a by k bits
	std::array<T, N> res{};

  if (k == 0) return a;
  
  for (auto i = 0; i < N - 1; ++i) {
    res[i] = (a[i] >> k) | (a[i + 1] << (std::numeric_limits<T>::digits - k));
  }
  res[N - 1] = (a[N - 1] >> k);
  return res;
}

template <size_t N, typename T>
constexpr T shift_left(std::array<T,N> a, size_t k) {
  // shift-left the big integer a by k bits
  // answer has 1 limb more
  //
  
  if (k == 0) return pad<1>(a);

  std::array<T,N + 1> res{};

  res[0] = (a[0] << k);

  for (auto i = 1; i < N; ++i) {
    res[i] = (a[i] << k) | (a[i - 1] >> (std::numeric_limits<T>::digits - k));
  }

  res[N] = a[N - 1] >> (std::numeric_limits<T>::digits - k);
  return res;
}

#endif
