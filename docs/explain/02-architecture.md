# Architecture and code map

Top-level layout
- `include/ct_resume_hash.h`: public API, length constant, normalize helper for tests/bindings.
- `src/`: normalization (ref + CT), hash core wrapper, bundled SHA-256, API plumbing, stream buffer.
- `bindings/`: Python C-extension and Rust FFI wrapper.
- `tests/`: unit, fuzz, timing; `benchmarks/`: microbench; `cmake/`: build graph.

Data flow (one-shot path)
1) `ct_resume_hash_once` (`src/ct_resume_hash.c`):
   - Allocates scratch buffer `input_len + 2` (trim/space collapse margin).
   - Calls `ct_normalize_ascii`.
   - Calls `ct_hash_core_once` on normalized bytes.
   - Scrubs and frees buffer.
2) `ct_hash_core_once` (`src/hash_core.c`):
   - Thin wrapper around bundled SHA-256 (`src/sha256.c`).
3) Output: 32-byte digest returned to caller.

Normalization implementations (`src/normalize_ref.c`, `src/normalize_ct.c`)
- Behavior: ASCII-only guarantee; controls dropped except whitespace → space; non-ASCII → `?`; uppercase → lowercase; collapse and trim spaces.
- Reference version: branchy, readability-first, shared with tests.
- CT version: mask-based operations to avoid branching on data; updates `seen_non_ws` / `last_space` via bitwise masks; selectable with `CT_RESUME_HASH_USE_CT`.

Hash core (`src/sha256.c`)
- Internal SHA-256 implementation (portable C11), no external deps.
- State struct: 8-word state, bit length counter, 64-byte buffer.
- Functions: `ct_sha256_init`, `ct_sha256_update`, `ct_sha256_final`; used only through `ct_hash_core_once`.

Streaming API (`ct_resume_hash_ctx`)
- Minimal buffer-then-finalize approach: `ct_resume_hash_update` appends chunks to a growable buffer; `ct_resume_hash_final` normalizes+hashes accumulated bytes and clears memory.
- Does not stream-normalize incrementally; normalization still happens once at `final`.

Build-time controls (CMake options in `cmake/CMakeLists.txt`)
- `CT_RESUME_HASH_USE_CT` (default ON): select CT normalization.
- `CT_RESUME_HASH_BUILD_TESTS`, `CT_RESUME_HASH_ENABLE_FUZZ`, `CT_RESUME_HASH_ENABLE_BENCH`: toggle unit/fuzz/bench targets.
- Compiler flags: `-O2 -Wall -Wextra -Werror -pedantic -fwrapv -fno-builtin-memcmp` to reduce CT surprises and tighten warnings.

Bindings
- Python (`bindings/python`): extension module `_native` built from shared C sources with CT flag; exposes `hash_once`.
- Rust (`bindings/rust`): FFI call to `ct_resume_hash_once`; `build.rs` compiles C sources with CT flag.
