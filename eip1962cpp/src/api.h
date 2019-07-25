#ifndef H_API
#define H_API

#include <variant>
#include <vector>
#include <string>

// Main API function for ABI.
bool run(std::vector<std::uint8_t> const &input, std::vector<std::uint8_t> &output, std::string &err);

#endif
