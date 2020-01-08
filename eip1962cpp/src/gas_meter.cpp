#include "constants.h"
#include "deserialization.h"
#include "repr.h"
#include "gas_meter.h"

#include <fstream>

#include "json/json.hh"

/*
Execution path goes run -> meter_limbed -> perform_metering -> {...}

Main way of transferring errors originating from input is through exceptions which are catched in run function.

There are no magic/hacks here, only templates.
*/

using json = nlohmann::json;

// class ThreadSafeSingleton{
// private:
//   ThreadSafeSingleton()= default;
//   ~ThreadSafeSingleton()= default;
//   ThreadSafeSingleton(const ThreadSafeSingleton&)= delete;
//   ThreadSafeSingleton& operator=(const ThreadSafeSingleton&)= delete;
// };

// class AdditionParameters: ThreadSafeSingleton {

template<const char *filename>
class AdditionParameters {
public:
  static AdditionParameters& getInstance() {
    static AdditionParameters instance;
    return instance;
  }

    std::unordered_map<u64, u64> prices;
private:
    AdditionParameters() {
        // std::fstream fs;
        // fs.open (filename, std::fstream::in);

        std::ifstream json_data_stream(filename);
        json prices_json;
        json_data_stream >> prices_json;
        std::vector<std::vector<u64>> all_prices = prices_json["price"];
        for(auto const& pair: all_prices) {
            prices.emplace(pair.at(0), pair.at(1));
        }
    }
    ~AdditionParameters()= default;
    AdditionParameters(const AdditionParameters&)= delete;
    AdditionParameters& operator=(const AdditionParameters&)= delete;
};

static const char g1_addition_params_filename[] = "g1_addition.json";
static const char g2_ext2_addition_params_filename[] = "g2_addition_ext2.json";
static const char g2_ext3_addition_params_filename[] = "g2_addition_ext3.json";

bool is_zero_dyn_number(const std::vector<u64> &num) {
    auto zero = true;
    for (auto it = num.cbegin(); it != num.cend(); it++)
    {
        zero &= *it == 0;
    }

    return zero;
}

template <usize N>
struct G1G2CurveData {
    Repr<N> modulus;
    std::vector<u64> order;
    bool in_extension;
    usize extension_degree;
};

template <usize N>
G1G2CurveData<N> parse_curve_data(u8 mod_byte_len,  Deserializer &deserializer, bool in_extension) {
    auto const modulus = deserialize_modulus<N>(mod_byte_len, deserializer);

    usize extension_degree = 1;

    if (in_extension) {
        auto const decoded_ext_degree = deserializer.byte("Input is not long enough to get extension degree");
        if (!(decoded_ext_degree == 2 || decoded_ext_degree == 3)) {
            input_err("Invalid extension degree");
        }
        extension_degree = usize(decoded_ext_degree);
        deserializer.dyn_number(mod_byte_len, "Input is not long enough to read non-residue");
    }

    deserializer.dyn_number(mod_byte_len, "Input is not long enough to read A parameter");
    deserializer.dyn_number(mod_byte_len, "Input is not long enough to read B parameter");
    
    auto order_len = deserializer.byte("Input is not long enough to get group size length");
    if (order_len > MAX_GROUP_BYTE_LEN) {
        input_err("Group order length is too large");
    }
    auto const order = deserializer.dyn_number(order_len, "Input is not long enough to get main group order size");

    auto order_is_zero = is_zero_dyn_number(order);
    if (order_is_zero) {
        input_err("Group order is zero");
    }

    struct G1G2CurveData<N> data = {modulus, order, in_extension, extension_degree};

    return data;
}

template<const char *filename>
u64 calculate_addition_metering(u64 modulus_limbs) {
    // std::unordered_map<u64,u64>::const_iterator
    auto price = AdditionParameters<filename>::getInstance().prices.find(modulus_limbs);
    if (price == AdditionParameters<filename>::getInstance().prices.end() ){
        input_err("invalid number of limbs");
    }    
    else {
        return price->second;
    }
}

template <usize N>
u64 perform_addition_metering(u8 mod_byte_len, Deserializer deserializer, bool in_extension) {
    auto data = parse_curve_data<N>(mod_byte_len, deserializer, in_extension);
    if (deserializer.ended()) {
        input_err("input is not long enough");
    }
    switch (data.extension_degree) {
        case 1:
            return calculate_addition_metering<g1_addition_params_filename>(u64(N));
        case 2:
            return calculate_addition_metering<g2_ext2_addition_params_filename>(u64(N));
        case 3:
            return calculate_addition_metering<g2_ext3_addition_params_filename>(u64(N));
    }
    input_err("unknown extension degree");
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
        // case MNT4:
        //     return meter_pairing_mnt4<N>(mod_byte_len, field, 2, deserializer);
        // case MNT6:
        //     return run_pairing_mnt<Fp3<N>, Fp6_2<N>, FieldExtension2over3<N>, FieldExtension3<N>, MNT6engine<N>>(mod_byte_len, field, 3, deserializer);
        // case BLS12:
        //     return run_pairing_b<BLS12engine<N>>(mod_byte_len, field, MAX_BLS12_X_BIT_LENGTH, deserializer);
        // case BN:
        //     return run_pairing_b<BNengine<N>>(mod_byte_len, field, MAX_BN_U_BIT_LENGTH, deserializer);
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
            case OPERATION_G2_MUL:
                {
                    return 10000;
                }
                break;
            case OPERATION_G1_MULTIEXP:
            case OPERATION_G2_MULTIEXP:
                {
                    return 1;
                }
                break;
            default:
                break;
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

