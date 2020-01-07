#ifndef H_API
#define H_API

#include <variant>
#include <vector>
#include <string>

#include "operation.h"

// Main API function for ABI.
std::variant<std::vector<std::uint8_t>, std::basic_string<char>> run(std::vector<std::uint8_t> const &input);
std::variant<std::vector<std::uint8_t>, std::basic_string<char>> run_with_operation(operation_type operation, std::vector<std::uint8_t> const &input);

#endif
