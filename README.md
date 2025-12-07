# ct-resume-hash

Constant-time resume fingerprinting in C with Python/Rust bindings.

## What it does
- Normalizes resume/profile text to a canonical ASCII form (case-folded, collapsed whitespace, control stripped, non-ASCII -> `?`).
- Hashes normalized bytes with SHA-256 using a bundled constant-time-friendly core.
- Targets content-independent timing for fixed-length inputs.

## Guarantees
- For equal-length inputs: normalization + hash aim for constant-time behavior (validated via timing harness).
- For differing lengths: time scales linearly with length; length is not treated as secret.
- Normalization avoids content-branching when built with `CT_RESUME_HASH_USE_CT=ON`.

## Build
```bash
cmake -S . -B build -DCT_RESUME_HASH_USE_CT=ON
cmake --build build
ctest --test-dir build
```

## C API
```c
int ct_resume_hash_once(const uint8_t *input, size_t input_len,
                        uint8_t out[CT_RESUME_HASH_LEN]);
ct_resume_hash_ctx *ct_resume_hash_new(void);
int ct_resume_hash_update(ct_resume_hash_ctx *ctx, const uint8_t *chunk, size_t chunk_len);
int ct_resume_hash_final(ct_resume_hash_ctx *ctx, uint8_t out[CT_RESUME_HASH_LEN]);
```

## Bindings
- Python: `pip install .` inside `bindings/python/`; use `ct_resume_hash.hash_once("text")`.
- Rust: `cargo test` inside `bindings/rust/`; call `ct_resume_hash::hash_once("text")`.

## Tests, fuzz, timing
- Unit: `ctest` (normalize + hash vectors).
- Fuzz: `fuzz_normalize`, `fuzz_roundtrip` harnesses (libFuzzer/AFL-ready).
- Timing: `dudect_runner` gives coarse ns timing sample; integrate with dudect for deeper stats.