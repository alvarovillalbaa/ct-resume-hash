#include "hash_core.h"
#include "sha256.h"

int ct_hash_core_once(const uint8_t *data, size_t len,
                      uint8_t out[CT_RESUME_HASH_LEN]) {
    if (!data || !out) {
        return -1;
    }

    ct_sha256_ctx ctx;
    ct_sha256_init(&ctx);
    ct_sha256_update(&ctx, data, len);
    ct_sha256_final(&ctx, out);
    return 0;
}

