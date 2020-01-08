#ifndef H_GAS_METER
#define H_GAS_METER

#include <variant>
#include <vector>
#include <string>

#include "operation.h"

// Main API function for ABI.
std::variant<u64, std::basic_string<char>> meter(std::vector<std::uint8_t> const &input);
std::variant<u64, std::basic_string<char>> meter_with_operation(operation_type operation, std::vector<std::uint8_t> const &input);

#endif
