#ifndef CT_RESUME_HASH_NORMALIZE_H
#define CT_RESUME_HASH_NORMALIZE_H

#include <stddef.h>
#include <stdint.h>

size_t ct_normalize_ascii_ref(const uint8_t *in,
                              size_t in_len,
                              uint8_t *out,
                              size_t out_cap);

size_t ct_normalize_ascii_ct(const uint8_t *in,
                             size_t in_len,
                             uint8_t *out,
                             size_t out_cap);

#endif // CT_RESUME_HASH_NORMALIZE_H

