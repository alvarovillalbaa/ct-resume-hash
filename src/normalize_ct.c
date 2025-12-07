#include "ct_resume_hash.h"
#include "normalize.h"

#include <stddef.h>
#include <stdint.h>

// Bit-mask helpers to avoid branches on character contents.
static inline uint8_t mask_if(int condition) {
    return (uint8_t)-(int8_t)(condition != 0);
}

static inline uint8_t ct_to_lower(uint8_t ch) {
    uint8_t is_upper = (uint8_t)((ch >= 'A') & (ch <= 'Z'));
    uint8_t mask = mask_if(is_upper);
    return (uint8_t)(ch ^ (mask & 0x20));
}

size_t ct_normalize_ascii_ct(const uint8_t *in,
                             size_t in_len,
                             uint8_t *out,
                             size_t out_cap) {
    if (!in || !out || out_cap == 0) {
        return 0;
    }

    size_t out_idx = 0;
    uint8_t seen_non_ws = 0;
    uint8_t last_space = 0;

    for (size_t i = 0; i < in_len; i++) {
        uint8_t ch = in[i];

        uint8_t is_space = (uint8_t)((ch == ' ') | (ch == '\t') | (ch == '\n') | (ch == '\r') | (ch == '\f'));
        uint8_t is_ctrl = (uint8_t)(ch < 0x20);
        uint8_t keep_ctrl = (uint8_t)(is_space | (uint8_t)(1u - is_ctrl));
        ch &= mask_if(keep_ctrl);

        uint8_t is_non_ascii = (uint8_t)(ch > 0x7e);
        ch = (uint8_t)((ch & ~mask_if(is_non_ascii)) | ('?' & mask_if(is_non_ascii)));

        ch = ct_to_lower(ch);
        ch = (uint8_t)((ch & ~mask_if(is_space)) | (' ' & mask_if(is_space)));

        uint8_t should_emit_space = (uint8_t)(is_space & (uint8_t)~last_space & seen_non_ws);
        uint8_t should_emit_char = (uint8_t)((uint8_t)~is_space & keep_ctrl);
        uint8_t should_emit = (uint8_t)(should_emit_space | should_emit_char);

        uint8_t has_room = (uint8_t)((out_idx + 1) < out_cap);
        should_emit &= has_room;

        uint8_t emit_char = is_space ? ' ' : ch;
        size_t write_pos = out_idx;
        out[write_pos] = (uint8_t)((out[write_pos] & ~mask_if(should_emit)) | (emit_char & mask_if(should_emit)));
        out_idx += should_emit;

        last_space = (uint8_t)((last_space & ~mask_if(should_emit_char)) | (is_space & mask_if(should_emit_char | should_emit_space)));
        seen_non_ws |= should_emit_char;
    }

    if (out_idx > 0 && out[out_idx - 1] == ' ') {
        out_idx--;
    }

    if (out_idx < out_cap) {
        out[out_idx] = 0;
    }

    return out_idx;
}

