#include "ct_resume_hash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void) {
    const size_t len = 128;
    uint8_t input[len];
    uint8_t out[CT_RESUME_HASH_LEN];

    uint64_t samples = 1000;
    uint64_t total = 0;

    for (uint64_t i = 0; i < samples; i++) {
        for (size_t j = 0; j < len; j++) {
            input[j] = (uint8_t)(rand() & 0xff);
        }
        uint64_t start = now_ns();
        ct_resume_hash_once(input, len, out);
        uint64_t end = now_ns();
        total += (end - start);
    }

    printf("dudect_runner: avg = %.2f ns over %llu samples\n",
           (double)total / (double)samples, (unsigned long long)samples);
    return 0;
}

