#include "ct_resume_hash.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check_once(void) {
    const char *input = "Hello\nWorld";
    const uint8_t expected[CT_RESUME_HASH_LEN] = {
        0xb9, 0x4d, 0x27, 0xb9, 0x93, 0x4d, 0x3e, 0x08,
        0xa5, 0x2e, 0x52, 0xd7, 0xda, 0x7d, 0xab, 0xfa,
        0xc4, 0x84, 0xef, 0xe3, 0x7a, 0x53, 0x80, 0xee,
        0x90, 0x88, 0xf7, 0xac, 0xe2, 0xef, 0xcd, 0xe9};

    uint8_t out[CT_RESUME_HASH_LEN];
    int rc = ct_resume_hash_once((const uint8_t *)input, strlen(input), out);
    assert(rc == 0);
    assert(memcmp(out, expected, CT_RESUME_HASH_LEN) == 0);
}

static void check_streaming(void) {
    const char *chunk1 = "Hello";
    const char *chunk2 = "\nWorld";

    const uint8_t expected[CT_RESUME_HASH_LEN] = {
        0xb9, 0x4d, 0x27, 0xb9, 0x93, 0x4d, 0x3e, 0x08,
        0xa5, 0x2e, 0x52, 0xd7, 0xda, 0x7d, 0xab, 0xfa,
        0xc4, 0x84, 0xef, 0xe3, 0x7a, 0x53, 0x80, 0xee,
        0x90, 0x88, 0xf7, 0xac, 0xe2, 0xef, 0xcd, 0xe9};

    ct_resume_hash_ctx *ctx = ct_resume_hash_new();
    assert(ctx);
    assert(ct_resume_hash_update(ctx, (const uint8_t *)chunk1, strlen(chunk1)) == 0);
    assert(ct_resume_hash_update(ctx, (const uint8_t *)chunk2, strlen(chunk2)) == 0);

    uint8_t out[CT_RESUME_HASH_LEN];
    assert(ct_resume_hash_final(ctx, out) == 0);
    ct_resume_hash_free(ctx);
    assert(memcmp(out, expected, CT_RESUME_HASH_LEN) == 0);
}

int main(void) {
    check_once();
    check_streaming();
    printf("test_hash: ok\n");
    return 0;
}

