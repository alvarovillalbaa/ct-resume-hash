#include "ct_resume_hash.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    uint8_t out[1024];
    size_t written = ct_normalize_ascii(data, size, out, sizeof(out));
    (void)written;
    return 0;
}

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if !defined(__AFL_LOOP) && !defined(LIBFUZZER)
int main(void) { return LLVMFuzzerTestOneInput((const uint8_t *)"", 0); }
#endif

