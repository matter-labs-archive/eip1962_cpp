#include "constants.h"
#include "gas_meter.h"
#include "deserialization.h"

#include <fstream>

#include "json/json.hh"

#include "g1_addition.h"
#include "g2_addition_ext2.h"
#include "g2_addition_ext3.h"
#include "g1_multiplication.h"
#include "g2_multiplication_ext2.h"
#include "g2_multiplication_ext3.h"
#include "multiexp_discounts.h"
#include "bls12_model.h"
#include "bn_model.h"
#include "mnt4_model.h"
#include "mnt6_model.h"

/*
Execution path goes run -> meter_limbed -> perform_metering -> {...}

Main way of transferring errors originating from input is through exceptions which are catched in run function.

There are no magic/hacks here, only templates.
*/

using json = nlohmann::json;

const std::string models_g1_addition_string(reinterpret_cast<const char*>(models_g1_addition_json), models_g1_addition_json_len);
const std::string models_g2_addition_ext2_string(reinterpret_cast<const char*>(models_g2_addition_ext2_json), models_g2_addition_ext2_json_len);
const std::string models_g2_addition_ext3_string(reinterpret_cast<const char*>(models_g2_addition_ext3_json), models_g2_addition_ext3_json_len);

const std::string models_g1_multiplication_string(reinterpret_cast<const char*>(models_g1_multiplication_json), models_g1_multiplication_json_len);
const std::string models_g2_multiplication_ext2_string(reinterpret_cast<const char*>(models_g2_multiplication_ext2_json), models_g2_multiplication_ext2_json_len);
const std::string models_g2_multiplication_ext3_string(reinterpret_cast<const char*>(models_g2_multiplication_ext3_json), models_g2_multiplication_ext3_json_len);

const std::string models_multiexp_discounts_string(reinterpret_cast<const char*>(models_multiexp_discounts_json), models_multiexp_discounts_json_len);

const std::string models_bn_model_json_string(reinterpret_cast<const char*>(models_bn_model_json), models_bn_model_json_len);
const std::string models_bls12_model_json_string(reinterpret_cast<const char*>(models_bls12_model_json), models_bls12_model_json_len);
const std::string models_mnt4_model_json_string(reinterpret_cast<const char*>(models_mnt4_model_json), models_mnt4_model_json_len);
const std::string models_mnt6_model_json_string(reinterpret_cast<const char*>(models_mnt6_model_json), models_mnt6_model_json_len);

template<typename MARKER>
class AdditionParametersModel {
public:
    static AdditionParametersModel& getInstance(std::string const &s) {
        static AdditionParametersModel instance(s);
        return instance;
    }
    std::unordered_map<u64, u64> prices;
private:
    AdditionParametersModel(std::string const &s) {
        std::cout << "Init addition with " << s << std::endl;
        auto prices_json = json::parse(s);
        std::vector<std::pair<u64, u64>> all_prices = prices_json["price"];
        for(auto const& pair: all_prices) {
            prices.emplace(std::get<0>(pair), std::get<1>(pair));
        }
    }
    ~AdditionParametersModel()= default;
    AdditionParametersModel(const AdditionParametersModel&)= delete;
    AdditionParametersModel& operator=(const AdditionParametersModel&)= delete;
};

template<typename MARKER>
class MultiplicationParametersModel {
public:
    static MultiplicationParametersModel& getInstance(std::string const &s) {
        static MultiplicationParametersModel instance(s);
        return instance;
    }
    std::unordered_map<u64, u64> base_prices;
    std::unordered_map<u64, u64> price_per_order_limb;
private:
    MultiplicationParametersModel(std::string const &s) {
        std::cout << "Init multiplication with " << s << std::endl;
        auto prices_json = json::parse(s);
        std::vector<std::pair<u64, u64>> base_prices_vec = prices_json["base"];
        std::vector<std::pair<u64, u64>> per_limb_prices_vec = prices_json["per_limb"];
        for(auto const& pair: base_prices_vec) {
            base_prices.emplace(std::get<0>(pair), std::get<1>(pair));
        }
        for(auto const& pair: per_limb_prices_vec) {
            price_per_order_limb.emplace(std::get<0>(pair), std::get<1>(pair));
        }
    }
    ~MultiplicationParametersModel()= default;
    MultiplicationParametersModel(const MultiplicationParametersModel&)= delete;
    MultiplicationParametersModel& operator=(const MultiplicationParametersModel&)= delete;
};

template<typename MARKER>
class MultiexpParametersModel {
public:
    static MultiexpParametersModel& getInstance(std::string const &s) {
        static MultiexpParametersModel instance(s);
        return instance;
    }
    u64 max_pairs;
    u64 max_discount;
    u64 multiplier;
    std::unordered_map<u64, u64> dicsounts;
private:
    MultiexpParametersModel(std::string const &s) {
        std::cout << "Init multiexp with " << s << std::endl;
        auto prices_json = json::parse(s);
        std::vector<std::pair<u64, u64>> discounts_vec = prices_json["discounts"];
        for(auto const& pair: discounts_vec) {
            dicsounts.emplace(std::get<0>(pair), std::get<1>(pair));
        }
        max_pairs = prices_json["max_pairs"];
        max_discount = prices_json["max_discount"];
        multiplier = prices_json["discount_multiplier"];
    }
    ~MultiexpParametersModel()= default;
    MultiexpParametersModel(const MultiexpParametersModel&)= delete;
    MultiexpParametersModel& operator=(const MultiexpParametersModel&)= delete;
};

template<typename MARKER, usize MAX>
class MntParametersModel {
public:
    static MntParametersModel& getInstance(std::string const &s) {
        static MntParametersModel instance(s);
        return instance;
    }
    std::unordered_map<u64, u64> one_off;
    u64 multiplier;
    std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> miller;
    std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> final_exp;
private:
    MntParametersModel(std::string const &s) {
        auto prices_json = json::parse(s);
        std::vector<std::pair<u64, u64>> one_off_prices_vec = prices_json["one_off"];
        std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> miller_prices_vec = prices_json["miller"];
        std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> final_exp_prices_vec = prices_json["final_exp"];
        for(auto const& pair: one_off_prices_vec) {
            one_off.emplace(std::get<0>(pair), std::get<1>(pair));
        }
        miller = miller_prices_vec;
        final_exp = final_exp_prices_vec;
        multiplier = prices_json["multiplier"];
    }
    ~MntParametersModel()= default;
    MntParametersModel(const MntParametersModel&)= delete;
    MntParametersModel& operator=(const MntParametersModel&)= delete;
};

template<typename MARKER>
class BlsBnParametersModel {
public:
    static BlsBnParametersModel& getInstance(std::string const &s) {
        static BlsBnParametersModel instance(s);
        return instance;
    }
    u64 multiplier;
    std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> miller;
    std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> final_exp;
private:
    BlsBnParametersModel(std::string const &s) {
        auto prices_json = json::parse(s);
        std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> miller_prices_vec = prices_json["miller"];
        std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> final_exp_prices_vec = prices_json["final_exp"];
        miller = miller_prices_vec;
        final_exp = final_exp_prices_vec;
        multiplier = prices_json["multiplier"];
    }
    ~BlsBnParametersModel()= default;
    BlsBnParametersModel(const BlsBnParametersModel&)= delete;
    BlsBnParametersModel& operator=(const BlsBnParametersModel&)= delete;
};

struct G1AdditionModelMarker{};
struct G2AdditionModelExt2Marker{};
struct G2AdditionModelExt3Marker{};

struct G1MultiplicationModelMarker{};
struct G2MultiplicationModelExt2Marker{};
struct G2MultiplicationModelExt3Marker{};

struct MultiexpModelMarker{};

struct Mnt4ModelMarker{};
struct Mnt6ModelMarker{};
struct Bls12ModelMarker{};
struct BnModelMarker{};

struct G1G2CurveData {
    u64 modulus_limbs;
    u64 group_order_limbs;
    bool in_extension;
    usize extension_degree;
};

template <usize EXT>
struct MntCurveData {
    u64 modulus_limbs;
    u64 group_order_limbs;
    u64 ate_loop_bits;
    u64 ate_loop_hamming;
    u64 w0_bits;
    u64 w0_hamming;
    u64 w1_bits;
    u64 w1_hamming;
    u64 num_pairs;
};

struct Bls12CurveData {
    u64 modulus_limbs;
    u64 group_order_limbs;
    u64 x_bits;
    u64 x_hamming;
    u64 num_pairs;
};

struct BnCurveData {
    u64 modulus_limbs;
    u64 group_order_limbs;
    u64 u_bits;
    u64 u_hamming;
    u64 six_u_plus_two_bits;
    u64 six_u_plus_two_hamming;
    u64 num_pairs;
};

template <usize N>
G1G2CurveData parse_curve_data(u8 mod_byte_len,  Deserializer &deserializer, bool in_extension) {
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);

    usize extension_degree = 1;

    if (in_extension) {
        auto const decoded_ext_degree = deserializer.byte("Input is not long enough to get extension degree");
        if (!(decoded_ext_degree == 2 || decoded_ext_degree == 3)) {
            input_err("Invalid extension degree");
        }
        extension_degree = usize(decoded_ext_degree);
        deserializer.advance(mod_byte_len, "Input is not long enough to read non-residue");
    }

    deserializer.advance(usize(mod_byte_len)*extension_degree, "Input is not long enough to read A parameter");
    deserializer.advance(usize(mod_byte_len)*extension_degree, "Input is not long enough to read B parameter");
    
    auto order_len = deserialize_group_order_length(deserializer);
    auto order = deserialize_group_order(order_len, deserializer);

    auto zero = is_zero(order);
    if (zero) {
        input_err("Group order is zero");
    }

    auto group_order_limbs = num_units_for_group_order(order);

    struct G1G2CurveData data = {u64(N), group_order_limbs, in_extension, extension_degree};

    return data;
}

template <usize N, usize EXT>
MntCurveData<EXT> parse_mnt_data(u8 mod_byte_len,  Deserializer &deserializer) {
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);

    deserializer.advance(mod_byte_len, "Input is not long enough to read A parameter");
    deserializer.advance(mod_byte_len, "Input is not long enough to read B parameter");
    
    auto order_len = deserialize_group_order_length(deserializer);
    auto order = deserialize_group_order(order_len, deserializer);

    auto zero = is_zero(order);
    if (zero) {
        input_err("Group order is zero");
    }

    auto group_order_limbs = num_units_for_group_order(order);

    deserializer.advance(mod_byte_len, "Input is not long enough to read non-residue");

    auto ate_loop = deserialize_scalar_with_bit_limit(usize(MAX_ATE_PAIRING_ATE_LOOP_COUNT_HAMMING), deserializer);
    auto ate_bits = num_bits(ate_loop);
    auto ate_hamming = calculate_hamming_weight(ate_loop);

    if (is_zero(ate_loop)) {
        input_err("Ate pairing loop is zero");
    }

    if (ate_hamming > MAX_ATE_PAIRING_ATE_LOOP_COUNT_HAMMING) {
        input_err("Ate pairing loop has too large hamming weight");
    }

    deserialize_sign(deserializer);

    auto w0 = deserialize_scalar_with_bit_limit(usize(MAX_ATE_PAIRING_FINAL_EXP_W0_BIT_LENGTH), deserializer);
    auto w0_bits = num_bits(w0);
    auto w0_hamming = calculate_hamming_weight(w0);

    if (is_zero(w0)) {
        input_err("W0 is zero");
    }

    auto w1 = deserialize_scalar_with_bit_limit(usize(MAX_ATE_PAIRING_FINAL_EXP_W1_BIT_LENGTH), deserializer);
    auto w1_bits = num_bits(w1);
    auto w1_hamming = calculate_hamming_weight(w1);

    if (is_zero(w1)) {
        input_err("W0 is zero");
    }

    deserialize_sign(deserializer);

    auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
    if (num_pairs == 0)
    {
        input_err("Zero pairs encoded");
    }

    if (deserializer.ended()) {
        input_err("Input is not long enough");
    }

    struct MntCurveData<EXT> data = {u64(N), group_order_limbs, ate_bits, ate_hamming, w0_bits, w0_hamming, w1_bits, w1_hamming, num_pairs};

    return data;
}

template <usize N>
Bls12CurveData parse_bls12_data(u8 mod_byte_len,  Deserializer &deserializer) {
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);

    deserializer.advance(mod_byte_len, "Input is not long enough to read A parameter");
    deserializer.advance(mod_byte_len, "Input is not long enough to read B parameter");
    
    auto order_len = deserialize_group_order_length(deserializer);
    auto order = deserialize_group_order(order_len, deserializer);

    auto zero = is_zero(order);
    if (zero) {
        input_err("Group order is zero");
    }

    auto group_order_limbs = num_units_for_group_order(order);

    deserializer.advance(mod_byte_len, "Input is not long enough to read Fp2 non-residue");
    deserializer.advance(mod_byte_len*2, "Input is not long enough to read Fp6/Fp12 non-residue");

    deserialize_pairing_twist_type(deserializer);

    auto x = deserialize_scalar_with_bit_limit(usize(MAX_BLS12_X_BIT_LENGTH), deserializer);
    auto x_bits = num_bits(x);
    auto x_hamming = calculate_hamming_weight(x);

    if (is_zero(x)) {
        input_err("X loop is zero");
    }

    if (x_hamming > MAX_BLS12_X_HAMMING) {
        input_err("X has too large hamming weight");
    }

    deserialize_sign(deserializer);

    auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
    if (num_pairs == 0)
    {
        input_err("Zero pairs encoded");
    }

    if (deserializer.ended()) {
        input_err("Input is not long enough");
    }


    struct Bls12CurveData data = {u64(N), group_order_limbs, x_bits, x_hamming, num_pairs};

    return data;
}

template <usize N>
BnCurveData parse_bn_data(u8 mod_byte_len,  Deserializer &deserializer) {
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);

    deserializer.advance(mod_byte_len, "Input is not long enough to read A parameter");
    deserializer.advance(mod_byte_len, "Input is not long enough to read B parameter");
    
    auto order_len = deserialize_group_order_length(deserializer);
    auto order = deserialize_group_order(order_len, deserializer);

    auto zero = is_zero(order);
    if (zero) {
        input_err("Group order is zero");
    }

    auto group_order_limbs = num_units_for_group_order(order);

    deserializer.advance(mod_byte_len, "Input is not long enough to read Fp2 non-residue");
    deserializer.advance(mod_byte_len*2, "Input is not long enough to read Fp6/Fp12 non-residue");

    deserialize_pairing_twist_type(deserializer);

    auto u = deserialize_scalar_with_bit_limit(usize(MAX_BN_U_BIT_LENGTH), deserializer);
    auto u_bits = num_bits(u);
    auto u_hamming = calculate_hamming_weight(u);

    if (is_zero(u)) {
        input_err("U loop is zero");
    }

    auto u_is_negative = deserialize_sign(deserializer);

    std::vector<u64> six_u_plus_2(u);
    mul_scalar(six_u_plus_2, 6);
    if (u_is_negative) {
        sub_scalar(six_u_plus_2, 2);
    } else {
        add_scalar(six_u_plus_2, 2);
    }

    auto six_u_plus_two_bits = num_bits(six_u_plus_2);
    auto six_u_plus_two_hamming = calculate_hamming_weight(six_u_plus_2);

    if (six_u_plus_two_hamming > MAX_BN_SIX_U_PLUS_TWO_HAMMING) {
        input_err("X has too large hamming weight");
    }

    auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
    if (num_pairs == 0)
    {
        input_err("Zero pairs encoded");
    }

    if (deserializer.ended()) {
        input_err("Input is not long enough");
    }

    struct BnCurveData data = {u64(N), group_order_limbs, u_bits, u_hamming, six_u_plus_two_bits, six_u_plus_two_hamming, num_pairs};

    return data;
}



template<typename MARKER>
u64 calculate_addition_metering(u64 modulus_limbs, const std::string &model) {
    // std::unordered_map<u64,u64>::const_iterator
    AdditionParametersModel<MARKER> &instance = AdditionParametersModel<MARKER>::getInstance(model);
    auto price = instance.prices.find(modulus_limbs);
    if (price == instance.prices.end() ){
        input_err("invalid number of limbs");
    }    

    return price->second;
}

u64 checked_mul(u64 a, u64 b) {
    auto bits_a = 64 - leading_zero(a);
    auto bits_b = 64 - leading_zero(b);
    if (bits_a + bits_b > 64) {
        input_err("overflow");
    }

    return a * b;
}

u64 checked_add(u64 a, u64 b) {
    auto bits_a = 64 - leading_zero(a);
    auto bits_b = 64 - leading_zero(b);
    if (bits_a > 63 || bits_b > 63) {
        input_err("overflow");
    }

    return a + b;
}

std::vector<u64> make_powers(u64 value, u64 max_power) {
    std::vector<u64> powers;
    powers.reserve(max_power);
    u64 p = 1;
    for (auto i = 0; i < max_power; i++) {
        p = checked_mul(p, value);
        powers.emplace_back(p);
    }

    return powers;
}

u64 eval_model(
    const std::vector<std::pair<u64, std::vector<std::pair<u64, u64>>>> &coeffs_variables_and_powers, 
    const std::vector<std::vector<u64>> &variables)
{
    u64 final_result = 0;
    if (coeffs_variables_and_powers.empty()) {
        input_err("missing model values");
    }

    usize max_var_id = 0;

    for(auto const& tup: coeffs_variables_and_powers) {
        for (auto const& var_and_power: std::get<1>(tup)) {
            if (max_var_id < std::get<0>(var_and_power)) {
                max_var_id = std::get<0>(var_and_power);
            }
        }
    }

    if (max_var_id + 1 != variables.size()) {
        input_err("discrepancy in model and number of variables");
    }

    for(auto const& tup: coeffs_variables_and_powers) {
        u64 subpart = std::get<0>(tup);
        for (auto const& var_and_power: std::get<1>(tup)) {
            auto variable = std::get<0>(var_and_power);
            auto power = std::get<1>(var_and_power);
            auto variable_powers = variables.at(variable);
            auto variable_powers_value = variable_powers.at(power - 1);
            subpart = checked_mul(subpart, variable_powers_value);
        }
        final_result = checked_add(final_result, subpart);
    }

    return final_result;
}

template<typename MARKER>
u64 calculate_multiplication_metering(u64 modulus_limbs, u64 group_order_limbs, const std::string &model) {
    // std::unordered_map<u64,u64>::const_iterator
    MultiplicationParametersModel<MARKER> &instance = MultiplicationParametersModel<MARKER>::getInstance(model);
    auto base_price = instance.base_prices.find(modulus_limbs);
    if (base_price == instance.base_prices.end() ){
        input_err("invalid number of limbs");
    }    
    auto per_limb_price = instance.price_per_order_limb.find(modulus_limbs);
    if (per_limb_price == instance.price_per_order_limb.end() ){
        input_err("invalid number of limbs");
    }    

    u64 result = checked_mul(group_order_limbs, per_limb_price->second);
    result = checked_add(result, base_price->second);
    return result;
}

template<typename MARKER>
u64 calculate_multiexp_metering(u64 modulus_limbs, u64 group_order_limbs, u64 num_pairs, const std::string &mul_model) {
    u64 result = calculate_multiplication_metering<MARKER>(modulus_limbs, group_order_limbs, mul_model);
    result = checked_mul(result, num_pairs);

    MultiexpParametersModel<MultiexpModelMarker> &instance = MultiexpParametersModel<MultiexpModelMarker>::getInstance(models_multiexp_discounts_string);

    u64 discount = 0;
    if (num_pairs > instance.max_pairs) {
        discount = instance.max_discount;
    } else {
        auto discount_per_num_pairs = instance.dicsounts.find(num_pairs);
        if (discount_per_num_pairs == instance.dicsounts.end() ){
            input_err("invalid number of pairs");
        }    
        discount = discount_per_num_pairs->second;
    }

    if (discount == 0) {
        input_err("multiexp discount is zero");
    }

    result = checked_mul(result, discount);
    auto multiplier = instance.multiplier;
    result = result / multiplier;
    
    return result;
}

template<typename MARKER, usize EXT, usize MAX>
u64 calculate_mnt_metering(MntCurveData<EXT> curve_data, const std::string &model) {
    u64 final_result = 0;

    MntParametersModel<MARKER, MAX> &instance = MntParametersModel<MARKER, MAX>::getInstance(model);

    // std::unordered_map<u64,u64>::const_iterator
    auto one_off_price = instance.one_off.find(curve_data.modulus_limbs);
    if (one_off_price == instance.one_off.end()){
        input_err("invalid number of limbs");
    }    

    u64 multiplier = instance.multiplier;

    auto miller_price_model = instance.miller;
 
    auto final_ext_price_model = instance.final_exp; 

    final_result = checked_add(final_result, one_off_price->second); 

    auto modulus_limbs_powers = make_powers(curve_data.modulus_limbs, u64(MAX));

    std::vector<std::vector<u64>> miller_params;
    miller_params.reserve(4);

    std::vector<u64> group_order_limbs;
    group_order_limbs.emplace_back(curve_data.group_order_limbs);

    std::vector<u64> ate_loop_bits;
    ate_loop_bits.emplace_back(curve_data.ate_loop_bits);

    std::vector<u64> ate_loop_hamming;
    ate_loop_hamming.emplace_back(curve_data.ate_loop_hamming);

    miller_params.emplace_back(group_order_limbs);
    miller_params.emplace_back(ate_loop_bits);
    miller_params.emplace_back(ate_loop_hamming);
    miller_params.emplace_back(modulus_limbs_powers);

    u64 miller_cost = eval_model(miller_price_model, miller_params);
    miller_cost = checked_mul(miller_cost, curve_data.num_pairs);

    final_result = checked_add(final_result, miller_cost);

    std::vector<std::vector<u64>> final_exp_params;
    final_exp_params.reserve(5);

    std::vector<u64> w0_bits;
    w0_bits.emplace_back(curve_data.w0_bits);

    std::vector<u64> w0_hamming;
    w0_hamming.emplace_back(curve_data.w0_hamming);

    std::vector<u64> w1_bits;
    w1_bits.emplace_back(curve_data.w1_bits);

    std::vector<u64> w1_hamming;
    w1_hamming.emplace_back(curve_data.w1_hamming);

    final_exp_params.emplace_back(w0_bits);
    final_exp_params.emplace_back(w0_hamming);
    final_exp_params.emplace_back(w1_bits);
    final_exp_params.emplace_back(w1_hamming);
    final_exp_params.emplace_back(modulus_limbs_powers);

    u64 final_exp_cost = eval_model(final_ext_price_model, final_exp_params);

    final_result = checked_add(final_result, final_exp_cost);

    final_result = final_result / multiplier;

    return final_result;
}

template <usize N>
u64 perform_addition_metering(u8 mod_byte_len, Deserializer deserializer, bool in_extension) {
    auto const data = parse_curve_data<N>(mod_byte_len, deserializer, in_extension);
    if (deserializer.ended()) {
        input_err("input is not long enough");
    }
    
    switch (data.extension_degree) {
        case 1:
            return calculate_addition_metering<G1AdditionModelMarker>(data.modulus_limbs, models_g1_addition_string);
        case 2:
            return calculate_addition_metering<G2AdditionModelExt2Marker>(data.modulus_limbs, models_g2_addition_ext2_string);
        case 3:
            return calculate_addition_metering<G2AdditionModelExt3Marker>(data.modulus_limbs, models_g2_addition_ext3_string);
    }
    input_err("unknown extension degree");
}

template <usize N>
u64 perform_multiplication_metering(u8 mod_byte_len, Deserializer deserializer, bool in_extension) {
    auto const data = parse_curve_data<N>(mod_byte_len, deserializer, in_extension);
    if (deserializer.ended()) {
        input_err("input is not long enough");
    }

    switch (data.extension_degree) {
        case 1:
            return calculate_multiplication_metering<G1MultiplicationModelMarker>(data.modulus_limbs, data.group_order_limbs, models_g1_multiplication_string);
        case 2:
            return calculate_multiplication_metering<G2MultiplicationModelExt2Marker>(data.modulus_limbs, data.group_order_limbs, models_g2_multiplication_ext2_string);
        case 3:
            return calculate_multiplication_metering<G2MultiplicationModelExt3Marker>(data.modulus_limbs, data.group_order_limbs, models_g2_multiplication_ext3_string);
    }
    input_err("unknown extension degree");
}

template <usize N>
u64 perform_multiexp_metering(u8 mod_byte_len, Deserializer deserializer, bool in_extension) {
    auto const data = parse_curve_data<N>(mod_byte_len, deserializer, in_extension);
    auto const num_pairs = deserializer.byte("Input is not long enough to get number of pairs");
    if (num_pairs == 0)
    {
        input_err("Invalid number of pairs");
    }

    if (deserializer.ended()) {
        input_err("input is not long enough");
    }

    switch (data.extension_degree) {
        case 1:
            return calculate_multiexp_metering<G1MultiplicationModelMarker>(data.modulus_limbs, data.group_order_limbs, u64(num_pairs), models_g1_multiplication_string);
        case 2:
            return calculate_multiexp_metering<G2MultiplicationModelExt2Marker>(data.modulus_limbs, data.group_order_limbs, u64(num_pairs), models_g2_multiplication_ext2_string);
        case 3:
            return calculate_multiexp_metering<G2MultiplicationModelExt3Marker>(data.modulus_limbs, data.group_order_limbs, u64(num_pairs), models_g2_multiplication_ext3_string);
    }
    input_err("unknown extension degree");
}

template <usize N, usize EXT>
u64 perform_mnt_metering(u8 mod_byte_len, Deserializer deserializer) {
    auto const data = parse_mnt_data<N, EXT>(mod_byte_len, deserializer);
    if (deserializer.ended()) {
        input_err("input is not long enough");
    }

    switch (EXT) {
        case 4:
            return calculate_mnt_metering<Mnt4ModelMarker, EXT, 4>(data, models_mnt4_model_json_string);
        case 6:
            return calculate_mnt_metering<Mnt6ModelMarker, EXT, 6>(data, models_mnt6_model_json_string);
        default:
            input_err("unknown MNT curve type");
    }
}

template <usize N>
u64 perform_bls12_metering(u8 mod_byte_len, Deserializer deserializer) {
    auto const data = parse_bls12_data<N>(mod_byte_len, deserializer);
    if (deserializer.ended()) {
        input_err("input is not long enough");
    }

    BlsBnParametersModel<Bls12ModelMarker> &instance = BlsBnParametersModel<Bls12ModelMarker>::getInstance(models_bls12_model_json_string);

    return 1;
}

template <usize N>
u64 perform_bn_metering(u8 mod_byte_len, Deserializer deserializer) {
    auto const data = parse_bn_data<N>(mod_byte_len, deserializer);
    if (deserializer.ended()) {
        input_err("input is not long enough");
    }

    BlsBnParametersModel<BnModelMarker> &instance = BlsBnParametersModel<BnModelMarker>::getInstance(models_bn_model_json_string);
    return 2;
}

template <usize N>
u64 perform_metering(u8 operation, std::optional<u8> curve_type, u8 mod_byte_len, Deserializer deserializer)
{
    if (curve_type)
    {
        // Pairing operation
        assert(operation == OPERATION_PAIRING);
        assert(curve_type);
        auto const curve_type_value = curve_type.value();
        switch (curve_type_value)
        {
        case MNT4:
            return perform_mnt_metering<N, 4>(mod_byte_len, deserializer);
        case MNT6:
            return perform_mnt_metering<N, 6>(mod_byte_len, deserializer);
        case BLS12:
            return perform_bls12_metering<N>(mod_byte_len, deserializer);
        case BN:
            return perform_bn_metering<N>(mod_byte_len, deserializer);
        default:
            input_err(stringf("invalid curve type %u", curve_type_value));
        }
    }
    else
    {
        // Non pairing operations

        switch (operation)
        {
            case OPERATION_G1_ADD:
                {
                    return perform_addition_metering<N>(mod_byte_len, deserializer, false);
                }
            case OPERATION_G2_ADD:
                {
                    return perform_addition_metering<N>(mod_byte_len, deserializer, true);
                }
            case OPERATION_G1_MUL:
                {
                    return perform_multiplication_metering<N>(mod_byte_len, deserializer, false);
                }
            case OPERATION_G2_MUL:
                {
                    return perform_multiplication_metering<N>(mod_byte_len, deserializer, true);
                }
            case OPERATION_G1_MULTIEXP:
                {
                    return perform_multiexp_metering<N>(mod_byte_len, deserializer, false);
                }
            case OPERATION_G2_MULTIEXP:
                {
                    return perform_multiexp_metering<N>(mod_byte_len, deserializer, true);
                }
            default:
                input_err("Unknown operation");
        }
    }
}

u64 meter_limbed(u8 operation, std::optional<u8> curve_type, Deserializer deserializer)
{
    // Deserialize modulus length
    auto mod_byte_len = deserializer.byte("Input is not long enough to get modulus length");
    auto mod_top_byte = deserializer.peek_byte("Input is not long enough to get modulus");
    if (mod_top_byte == 0) {
        input_err("Invalid modulus encoding");
    }
    auto modulus_bits = u32(mod_byte_len - 1) * 8;
    if (mod_top_byte >> 7) {
        modulus_bits += 8;
    } // alternative case does not matter

    auto limb_count = (modulus_bits / 64) + 1; // e.g. 256 bits will result in 5 limbs

    // Call run_operation with adequate number of limbs
    switch (limb_count)
    {
    case 0:
        input_err("Modulus length is zero");
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        return perform_metering<4>(operation, curve_type, mod_byte_len, deserializer);
    case 5:
        return perform_metering<5>(operation, curve_type, mod_byte_len, deserializer);
    case 6:
        return perform_metering<6>(operation, curve_type, mod_byte_len, deserializer);
    case 7:
        return perform_metering<7>(operation, curve_type, mod_byte_len, deserializer);
    case 8:
        return perform_metering<8>(operation, curve_type, mod_byte_len, deserializer);
    case 9:
        return perform_metering<9>(operation, curve_type, mod_byte_len, deserializer);
    case 10:
        return perform_metering<10>(operation, curve_type, mod_byte_len, deserializer);
    case 11:
        return perform_metering<11>(operation, curve_type, mod_byte_len, deserializer);
    case 12:
        return perform_metering<12>(operation, curve_type, mod_byte_len, deserializer);
    case 13:
        return perform_metering<13>(operation, curve_type, mod_byte_len, deserializer);
    case 14:
        return perform_metering<14>(operation, curve_type, mod_byte_len, deserializer);
    case 15:
        return perform_metering<15>(operation, curve_type, mod_byte_len, deserializer);
    case 16:
        return perform_metering<16>(operation, curve_type, mod_byte_len, deserializer);

    default:
        unimplemented(stringf("operations are not supported for %u modulus limbs", limb_count));
    }
}

// Main API function which receives ABI input and returns the result of operations, or description of occured error.
std::variant<u64, std::basic_string<char>> meter(std::vector<std::uint8_t> const &input)
{
    try
    {
        // Deserialize operation
        auto deserializer = Deserializer(input);
        auto operation = deserializer.byte("Input should be longer than operation type encoding");

        std::optional<u8> curve_type;
        switch (operation)
        {
        case OPERATION_PAIRING:
            {
                auto const decoded_curve_type = deserialize_pairing_curve_type(deserializer);
                curve_type = decoded_curve_type;
            }
            // Intentional fall through
        case OPERATION_G1_ADD:
        case OPERATION_G1_MUL:
        case OPERATION_G1_MULTIEXP:
        case OPERATION_G2_ADD:
        case OPERATION_G2_MUL:
        case OPERATION_G2_MULTIEXP:
            return meter_limbed(operation, curve_type, deserializer);

        default:
            input_err("Unknown operation type");
        }
    }
    catch (std::domain_error const &e)
    {
        return e.what();
    }
    catch (std::runtime_error const &e)
    {
        return e.what();
    }
    catch (std::bad_optional_access const &e) // TODO: Remove when rework the arithmetics
    {
        return e.what();
    }
}

// Main API function which receives ABI input and returns the result of operations, or description of occured error.
std::variant<u64, std::basic_string<char>> meter_with_operation(operation_type operation, std::vector<std::uint8_t> const &input)
{
    try
    {
        // Deserialize operation
        auto deserializer = Deserializer(input);
        u8 raw_operation;
        std::optional<u8> curve_type;
        
        switch (operation)
        {
        case pair_bls12:
        {
            curve_type = BLS12;
            raw_operation = OPERATION_PAIRING;
            break;
        }
        case pair_bn:
        {
            curve_type = BN;
            raw_operation = OPERATION_PAIRING;
            break;
        }
        case pair_mnt4:
        {
            curve_type = MNT4;
            raw_operation = OPERATION_PAIRING;
            break;
        }
        case pair_mnt6:
        {
            curve_type = MNT6;
            raw_operation = OPERATION_PAIRING;
            break;
        }
        case g1_add:
        {
            raw_operation = OPERATION_G1_ADD;
            break;
        }
        case g1_mul:
        {
            raw_operation = OPERATION_G1_MUL;
            break;
        }
        case g1_multiexp:
        {
            raw_operation = OPERATION_G1_MULTIEXP;
            break;
        }
        case g2_add:
        {
            raw_operation = OPERATION_G2_ADD;
            break;
        }
        case g2_mul:
        {
            raw_operation = OPERATION_G2_MUL;
            break;
        }
        case g2_multiexp:
        {
            raw_operation = OPERATION_G2_MULTIEXP;
            break;
        }
        default:
            input_err("Unknown operation type");
        }

        return meter_limbed(operation, curve_type, deserializer);
    }
    catch (std::domain_error const &e)
    {
        return e.what();
    }
    catch (std::runtime_error const &e)
    {
        return e.what();
    }
    catch (std::bad_optional_access const &e) // TODO: Remove when rework the arithmetics
    {
        return e.what();
    }
}

