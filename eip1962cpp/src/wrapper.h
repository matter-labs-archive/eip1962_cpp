#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int run(const char *i, uint32_t i_len, char *o, uint32_t *o_len, char *err, uint32_t *char_len);

uint32_t c_perform_operation(char op,
                             const char *i,
                             uint32_t i_len,
                             char *o,
                             uint32_t *o_len,
                             char *err,
                             uint32_t *char_len);

int meter_gas(const char *i, uint32_t i_len, uint64_t *gas);

uint32_t c_meter_operation(char op,
                             const char *i,
                             uint32_t i_len,
                             uint64_t *gas,
                             char *err,
                             uint32_t *char_len);

#ifdef __cplusplus
}
#endif

#endif /* __WRAPPER_H__ */