# Build, run, and validate

Core C build
- Configure + build:
  - `cmake -S . -B build -DCT_RESUME_HASH_USE_CT=ON`
  - `cmake --build build`
- Options: flip `CT_RESUME_HASH_BUILD_TESTS`, `CT_RESUME_HASH_ENABLE_FUZZ`, `CT_RESUME_HASH_ENABLE_BENCH` as needed (all ON by default in CMake).

API quickstart (C)
- One-shot:
  - `ct_resume_hash_once((const uint8_t *)input, input_len, out32);`
- Streaming:
  - `ctx = ct_resume_hash_new();`
  - `ct_resume_hash_update(ctx, chunk, len);` (can repeat; buffers internally)
  - `ct_resume_hash_final(ctx, out32);`
  - `ct_resume_hash_free(ctx);`
- Normalization helper for tests/bindings: `ct_normalize_ascii` returns bytes written.

Tests
- Unit: `ctest --test-dir build` (runs `test_normalize`, `test_hash`).
- Fuzz harnesses (libFuzzer/AFL-friendly): `fuzz_normalize`, `fuzz_roundtrip` built when `CT_RESUME_HASH_ENABLE_FUZZ=ON`.
- Timing sampler: `dudect_runner` produces average ns timing over randomized inputs; integrate with full dudect for leakage stats.
- Benchmarks: `bench_hash`, `bench_normalize` print per-call latency (ns/us) for representative inputs.

Python binding
- From `bindings/python/`: `pip install .`
- Usage:
  - `import ct_resume_hash`
  - `digest = ct_resume_hash.hash_once("some resume text")  # bytes length 32`
- Build flags: defines `CT_RESUME_HASH_USE_CT`, includes shared C sources, compiles with `-O2 -fwrapv -fno-builtin-memcmp`.

Rust binding
- From `bindings/rust/`: `cargo test` (compiles C sources via `build.rs` with CT flag).
- Usage:
  - `let digest = ct_resume_hash::hash_once("some resume text")?;` (returns `[u8; 32]`).
