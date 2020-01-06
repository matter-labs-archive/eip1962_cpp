#include "api.h"
#include "wrapper.h"
#include "common.h"

int run(const char *i, uint32_t i_len, char *o, uint32_t *o_len, char *err, uint32_t *char_len) {
    std::vector<std::uint8_t> input;
    input.resize(i_len);
    std::copy(i, i + i_len, input.begin());
    auto result = run(input);
    if (auto answer = std::get_if<0>(&result))
    {
        std::copy(answer->begin(), answer->end(), o);
        *o_len = answer->size();
        return true;
    } else if (auto error_descr = std::get_if<1>(&result)) {
        auto str_len = error_descr->size();
        auto c_str = error_descr->c_str();
        std::copy(c_str, c_str + str_len + 1, err);
        *char_len = error_descr->size();
        return false;
    }

    return false;
}

int meter_gas(const char *i, uint32_t i_len, uint64_t *gas) {
    *gas = UINT64_MAX;
    return true;
}

std::optional<operation_type> parse_operation_type(char op) {
    u8 op_u8 = u8(op);

    std::optional<operation_type> operation;

    switch (op_u8) {
        case u8(operation_type::g1_add):
        {
            operation = operation_type::g1_add;
            break;
        }
        case u8(operation_type::g1_mul):
        {
            operation = operation_type::g1_mul;
            break;
        }
        case u8(operation_type::g1_multiexp):
        {
            operation = operation_type::g1_multiexp;
            break;
        }
        case u8(operation_type::g2_add):
        {
            operation = operation_type::g2_add;
            break;
        }
        case u8(operation_type::g2_mul):
        {
            operation = operation_type::g2_mul;
            break;
        }
        case u8(operation_type::g2_multiexp):
        {
            operation = operation_type::g2_multiexp;
            break;
        }
        case u8(operation_type::pair_bls12):
        {
            operation = operation_type::pair_bls12;
            break;
        }
        case u8(operation_type::pair_bn):
        {
            operation = operation_type::pair_bn;
            break;
        }
        case u8(operation_type::pair_mnt4):
        {
            operation = operation_type::pair_mnt4;
            break;
        }
        case u8(operation_type::pair_mnt6):
        {
            operation = operation_type::pair_mnt6;
            break;
        }
    } 

    return operation;
}

uint32_t c_perform_operation(char op,
                             const char *i,
                             uint32_t i_len,
                             char *o,
                             uint32_t *o_len,
                             char *err,
                             uint32_t *char_len) 
{
    std::vector<std::uint8_t> input;
    input.resize(i_len);
    std::copy(i, i + i_len, input.begin());
    if (auto operation = parse_operation_type(op))
    {
        auto result = run_with_operation(operation.value(), input);
        if (auto answer = std::get_if<0>(&result))
        {
            std::copy(answer->begin(), answer->end(), o);
            *o_len = answer->size();
            return 0;
        } else if (auto error_descr = std::get_if<1>(&result)) {
            auto str_len = error_descr->size();
            auto c_str = error_descr->c_str();
            std::copy(c_str, c_str + str_len + 1, err);
            *char_len = error_descr->size();
            return 1;
        }
    } else {
        const std::string error_descr("Unknown operation type");
        auto str_len = error_descr.size();
        auto c_str = error_descr.c_str();
        std::copy(c_str, c_str + str_len + 1, err);
        *char_len = error_descr.size();
        return 1;
    }

    return 1;
}