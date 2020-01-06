#ifndef H_API
#define H_API

#include <variant>
#include <vector>
#include <string>

enum operation_type 
{   
    g1_add = 1, 
    g1_mul = 2,
    g1_multiexp = 3,
    g2_add = 4,
    g2_mul = 5,
    g2_multiexp = 6,
    pair_bls12 = 7,
    pair_bn = 8,
    pair_mnt4 = 9,
    pair_mnt6 = 10 
};

// Main API function for ABI.
std::variant<std::vector<std::uint8_t>, std::basic_string<char>> run(std::vector<std::uint8_t> const &input);
std::variant<std::vector<std::uint8_t>, std::basic_string<char>> run_with_operation(operation_type operation, std::vector<std::uint8_t> const &input);

#endif
