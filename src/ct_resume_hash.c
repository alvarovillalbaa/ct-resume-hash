#include "ct_resume_hash.h"
#include "hash_core.h"
#include "normalize.h"

#include <stdlib.h>
#include <string.h>

// Select normalization implementation at build time.
size_t ct_normalize_ascii(const uint8_t *in,
                          size_t in_len,
                          uint8_t *out,
                          size_t out_cap) {
#ifdef CT_RESUME_HASH_USE_CT
    return ct_normalize_ascii_ct(in, in_len, out, out_cap);
#else
    return ct_normalize_ascii_ref(in, in_len, out, out_cap);
#endif
}

int ct_resume_hash_once(const uint8_t *input,
                        size_t input_len,
                        uint8_t out[CT_RESUME_HASH_LEN]) {
    if (!input || !out) {
        return -1;
    }

    // worst-case output length: input_len + 2 for trimming
    uint8_t *buf = (uint8_t *)malloc(input_len + 2);
    if (!buf) {
        return -2;
    }

    size_t norm_len = ct_normalize_ascii(input, input_len, buf, input_len + 2);
    int rc = ct_hash_core_once(buf, norm_len, out);

    // scrub buffer before free (best-effort)
    if (norm_len > 0) {
        memset(buf, 0, norm_len);
    }
    free(buf);

    return rc;
}

struct ct_resume_hash_ctx {
    uint8_t *buffer;
    size_t len;
    size_t cap;
};

ct_resume_hash_ctx *ct_resume_hash_new(void) {
    ct_resume_hash_ctx *ctx = (ct_resume_hash_ctx *)calloc(1, sizeof(ct_resume_hash_ctx));
    return ctx;
}

void ct_resume_hash_free(ct_resume_hash_ctx *ctx) {
    if (!ctx) {
        return;
    }
    if (ctx->buffer) {
        memset(ctx->buffer, 0, ctx->len);
        free(ctx->buffer);
    }
    free(ctx);
}

static int ensure_capacity(ct_resume_hash_ctx *ctx, size_t extra) {
    if (ctx->len + extra <= ctx->cap) {
        return 0;
    }
    size_t new_cap = ctx->cap ? ctx->cap * 2 : 256;
    while (new_cap < ctx->len + extra) {
        new_cap *= 2;
    }
    uint8_t *new_buf = (uint8_t *)realloc(ctx->buffer, new_cap);
    if (!new_buf) {
        return -1;
    }
    ctx->buffer = new_buf;
    ctx->cap = new_cap;
    return 0;
}

int ct_resume_hash_update(ct_resume_hash_ctx *ctx,
                          const uint8_t *chunk,
                          size_t chunk_len) {
    if (!ctx || !chunk) {
        return -1;
    }
    if (ensure_capacity(ctx, chunk_len) != 0) {
        return -2;
    }
    memcpy(ctx->buffer + ctx->len, chunk, chunk_len);
    ctx->len += chunk_len;
    return 0;
}

int ct_resume_hash_final(ct_resume_hash_ctx *ctx,
                         uint8_t out[CT_RESUME_HASH_LEN]) {
    if (!ctx || !out) {
        return -1;
    }
    int rc = ct_resume_hash_once(ctx->buffer, ctx->len, out);
    memset(ctx->buffer, 0, ctx->len);
    free(ctx->buffer);
    ctx->buffer = NULL;
    ctx->len = 0;
    ctx->cap = 0;
    return rc;
}

