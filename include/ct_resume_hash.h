#ifndef CT_RESUME_HASH_H
#define CT_RESUME_HASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CT_RESUME_HASH_LEN 32u

typedef struct ct_resume_hash_ctx ct_resume_hash_ctx;

/**
 * Compute hash(resume_text) -> 32 bytes.
 *
 * - Input: UTF-8; only ASCII subset guaranteed stable in v1.
 * - Output: 32-byte hash.
 * - Returns 0 on success, non-zero on error.
 */
int ct_resume_hash_once(const uint8_t *input,
                        size_t input_len,
                        uint8_t out[CT_RESUME_HASH_LEN]);

/**
 * Normalization helper exposed for testing and bindings.
 * Returns number of bytes written to `out`.
 */
size_t ct_normalize_ascii(const uint8_t *in,
                          size_t in_len,
                          uint8_t *out,
                          size_t out_cap);

ct_resume_hash_ctx *ct_resume_hash_new(void);
void ct_resume_hash_free(ct_resume_hash_ctx *ctx);
int ct_resume_hash_update(ct_resume_hash_ctx *ctx,
                          const uint8_t *chunk,
                          size_t chunk_len);
int ct_resume_hash_final(ct_resume_hash_ctx *ctx,
                         uint8_t out[CT_RESUME_HASH_LEN]);

#ifdef __cplusplus
}
#endif

#endif // CT_RESUME_HASH_H

