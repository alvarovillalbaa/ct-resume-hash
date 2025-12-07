#include "ct_resume_hash.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 2) {
        return 0;
    }

    size_t mid = size / 2;
    const uint8_t *a = data;
    size_t a_len = mid;
    const uint8_t *b = data + mid;
    size_t b_len = size - mid;

    uint8_t norm_a[1024];
    uint8_t norm_b[1024];

    size_t norm_a_len = ct_normalize_ascii(a, a_len, norm_a, sizeof(norm_a));
    size_t norm_b_len = ct_normalize_ascii(b, b_len, norm_b, sizeof(norm_b));

    uint8_t hash_a[CT_RESUME_HASH_LEN];
    uint8_t hash_b[CT_RESUME_HASH_LEN];
    ct_resume_hash_once(a, a_len, hash_a);
    ct_resume_hash_once(b, b_len, hash_b);

    int same_norm = (norm_a_len == norm_b_len) &&
                    (memcmp(norm_a, norm_b, norm_a_len) == 0);
    if (same_norm && memcmp(hash_a, hash_b, CT_RESUME_HASH_LEN) != 0) {
        __builtin_trap();
    }
    return 0;
}

#if !defined(LIBFUZZER)
int main(void) { return LLVMFuzzerTestOneInput((const uint8_t *)"ab", 2); }
#endif

