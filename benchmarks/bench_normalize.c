#include "ct_resume_hash.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void) {
    const char *input = "   Senior ENGINEER\twith\n   spacing   and CAPS   ";
    uint8_t out[256];

    const size_t iters = 50000;
    uint64_t start = now_ns();
    for (size_t i = 0; i < iters; i++) {
        ct_normalize_ascii((const uint8_t *)input, strlen(input), out, sizeof(out));
    }
    uint64_t end = now_ns();

    printf("bench_normalize: %.2f ns per call\n",
           (double)(end - start) / (double)iters);
    return 0;
}

