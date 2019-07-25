#include "api.h"
#include "wrapper.h"

int run(const char *i, uint32_t i_len, char *o, uint32_t *o_len, char *err, uint32_t *char_len) {
    std::vector<std::uint8_t> input;
	std::vector<std::uint8_t> output;
	std::string err_desc;
    input.resize(i_len);
    std::copy(i, i + i_len, input.begin());
    bool result = run(input, output, err_desc);
    if (result)
    {
        std::copy(output.begin(), output.end(), o);
        *o_len = static_cast<uint32_t>(output.size());
        return true;
    } else if (err_desc.length()) {
        auto str_len = err_desc.size();
        auto c_str = err_desc.c_str();
        std::copy(c_str, c_str + str_len + 1, err);
        *char_len = static_cast<uint32_t>(err_desc.size());
        return false;
    }

    return false;
}

int meter_gas(const char *i, uint32_t i_len, uint64_t *gas) {
    *gas = UINT64_MAX;
    return true;
}