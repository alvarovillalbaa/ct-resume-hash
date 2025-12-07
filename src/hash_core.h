#ifndef CT_RESUME_HASH_HASH_CORE_H
#define CT_RESUME_HASH_HASH_CORE_H

#include <stddef.h>
#include <stdint.h>

#include "ct_resume_hash.h"

int ct_hash_core_once(const uint8_t *data, size_t len,
                      uint8_t out[CT_RESUME_HASH_LEN]);

#endif // CT_RESUME_HASH_HASH_CORE_H

