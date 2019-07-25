#ifndef __TYPE_TRAITS_HPP
#define __TYPE_TRAITS_HPP

#include <cstdint>

template <typename T> struct dbl_bitlen { using type = void; };
template <> struct dbl_bitlen<uint8_t> { using type = uint16_t; };
template <> struct dbl_bitlen<uint16_t> { using type = uint32_t; };
template <> struct dbl_bitlen<uint32_t> { using type = uint64_t; };
template <> struct dbl_bitlen<uint64_t> { using type = uint64_t; };

#endif