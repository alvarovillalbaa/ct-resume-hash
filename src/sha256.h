#ifndef CT_RESUME_HASH_SHA256_H
#define CT_RESUME_HASH_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t buffer[64];
    size_t buffer_len;
} ct_sha256_ctx;

void ct_sha256_init(ct_sha256_ctx *ctx);
void ct_sha256_update(ct_sha256_ctx *ctx, const uint8_t *data, size_t len);
void ct_sha256_final(ct_sha256_ctx *ctx, uint8_t out[32]);

#endif // CT_RESUME_HASH_SHA256_H

