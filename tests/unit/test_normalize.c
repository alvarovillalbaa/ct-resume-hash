#include "ct_resume_hash.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check_case(const char *input, const char *expected) {
    uint8_t out[256] = {0};
    size_t written = ct_normalize_ascii((const uint8_t *)input, strlen(input), out, sizeof(out));
    assert(written == strlen(expected));
    assert(memcmp(out, expected, written) == 0);
}

int main(void) {
    check_case("", "");
    check_case("   ", "");
    check_case("Hello World", "hello world");
    check_case("Hello   World", "hello world");
    check_case(" Hello\tWorld\n", "hello world");
    check_case("Mixed\tCASE\r\n", "mixed case");
    check_case("CTRL\x01\x02abc", "ctrlabc");
    check_case("UTF8 áéí", "utf8 ??");

    printf("test_normalize: ok\n");
    return 0;
}

