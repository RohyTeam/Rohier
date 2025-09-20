#include "safe_utils.h"

uint32_t safe_convert_int32_to_uint32(int32_t value) {
    return value < 0 ? 0 : static_cast<uint32_t>(value);
}