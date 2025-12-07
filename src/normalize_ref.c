#include "ct_resume_hash.h"
#include "normalize.h"

#include <stddef.h>
#include <stdint.h>

static int is_space(uint8_t ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f';
}

size_t ct_normalize_ascii_ref(const uint8_t *in,
                              size_t in_len,
                              uint8_t *out,
                              size_t out_cap) {
    if (!in || !out || out_cap == 0) {
        return 0;
    }

    size_t out_idx = 0;
    int seen_non_ws = 0;
    int last_space = 0;

    for (size_t i = 0; i < in_len; i++) {
        uint8_t ch = in[i];

        if (ch < 0x20) {
            if (is_space(ch)) {
                ch = ' ';
            } else {
                continue;
            }
        } else if (ch > 0x7e) {
            ch = '?';
        }

        if (ch >= 'A' && ch <= 'Z') {
            ch = (uint8_t)(ch | 0x20);
        }

        int space = ch == ' ';
        if (space) {
            if (!seen_non_ws || last_space) {
                continue;
            }
        } else {
            seen_non_ws = 1;
        }

        if (out_idx + 1 >= out_cap) {
            break;
        }

        out[out_idx++] = space ? ' ' : ch;
        last_space = space;
    }

    if (out_idx > 0 && out[out_idx - 1] == ' ') {
        out_idx--;
    }

    if (out_idx < out_cap) {
        out[out_idx] = 0;
    }

    return out_idx;
}

