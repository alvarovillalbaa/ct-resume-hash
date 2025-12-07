Let’s treat `ct-resume-hash/` as if you’re going to ship it to a bank.

I’ll walk you end-to-end:

1. What this library *is* and *is not*.
2. Exact guarantees (constant-time, normalization, privacy).
3. API + directory layout.
4. Implementation plan in stages.
5. Constant-time engineering tricks (where people usually screw up).
6. Testing, fuzzing, and CT validation.
7. Python/Rust bindings and packaging.
8. Integration into your HR stack + versioning strategy.

---

## 1. What `ct-resume-hash` actually is

**Goal:**
A small, auditable C library (with optional asm) that computes **stable, constant-time fingerprints of resume text**.

* **Input:** Arbitrary resume/profile text (email, CV, LinkedIn scrape, etc.).
* **Output:** Fixed-size hash (e.g. 256 bits) suitable for:

  * deduplication,
  * PII-safe identifiers,
  * joining across systems.

**Constraints you care about:**

* **Deterministic** for given (normalized) input.
* **Content-independent timing** *for a fixed length* (no early exits, no branches on content).
* **Attack model:** You assume an attacker can:

  * measure timing,
  * control the resume contents,
  * but **not** control the runtime length or OS noise.

So the property is:

> For two inputs of the **same length**, timing distribution is indistinguishable as a function of contents (in practice, “no detectable difference in dudect-style tests”).

Length-dependence is fine: resume length is not treated as a secret.

---

## 2. Spec: normalization + hash + guarantees

You need a written spec or this will drift. Keep it tight.

### 2.1 Normalization spec

Define a **minimal, explicit pipeline**, e.g.:

1. **Encoding assumption**

   * v1: accept **UTF-8**, but only guarantee behavior for **ASCII subset** (0x20–0x7E).
   * Non-ASCII → treat as `?` or strip.

2. **Steps:**

   1. **Case folding:** `A–Z` → `a–z`.
   2. **Whitespace normalization:**

      * Any ASCII whitespace (`' '`, `\t`, `\n`, `\r`, `\f`) → space (`' '`).
      * Collapse consecutive spaces into a **single space**.
      * Strip leading/trailing spaces.
   3. **Control chars:** ASCII `< 0x20` except whitespace → removed.
   4. **Optional:** normalize certain punctuation (e.g. fancy quotes → `'`).

**Constant-time angle:**
All conditionals must be implementable via bit-twiddling + masks so the compiler doesn’t reintroduce branches on secret data.

If you want more complex Unicode normalization, do it in a later version. v1 = ASCII-safe.

### 2.2 Hash spec

You should *not* invent your own hash algorithm.

Pick one:

* **Cryptographic:** SHA-256, BLAKE2s, BLAKE3.
* **Keyed:** SipHash/BLAKE2s-keyed if you want to prevent rainbow tables on your PII.

For v1 I’d do:

* `ct_resume_hash` = `SHA-256(normalized_bytes)` or `BLAKE2s(normalized_bytes, key)`.

And treat the “constant-time” challenge as:

* making the **normalization** constant-time wrt contents,
* ensuring your **hash core** is from a reputable, CT implementation.

You can later add a custom assembly implementation for the chosen hash (still spec-compatible).

### 2.3 Guarantees to document

In README:

* For inputs of the same length:

  * the normalization + hash **runs in constant time up to statistical noise**.
* For different lengths:

  * runtime is linear in length; length is **not** considered secret.
* Implementation:

  * uses only constant-time operations wrt contents (no early-exit compares, etc.).
  * has a dudect-style test suite with no detected leaks under test conditions.

---

## 3. API design and directory layout

### 3.1 Repo structure

```text
ct-resume-hash/
  include/
    ct_resume_hash.h
  src/
    normalize.c
    normalize_ct.c          # low-level, CT primitives
    hash_core.c             # wraps chosen hash (e.g. BLAKE2 or SHA-256)
    hash_core_ref.c         # slow reference impl (optional)
    hash_core_x86_64.S      # optional asm fast path
    ct_resume_hash.c        # public API implementation
  bindings/
    python/
      pyproject.toml
      src/ct_resume_hash/__init__.py
      src/ct_resume_hash/_native.c
    rust/
      Cargo.toml
      src/lib.rs
  tests/
    unit/
      test_normalize.c
      test_hash.c
    fuzz/
      fuzz_normalize.c
      fuzz_roundtrip.c
    timing/
      dudect_runner.c
  benchmarks/
    bench_hash.c
    bench_normalize.c
  cmake/
    CMakeLists.txt
  .github/workflows/
    ci.yml
  README.md
  LICENSE
```

### 3.2 C API (public header)

Keep it small and boring.

```c
// include/ct_resume_hash.h
#ifndef CT_RESUME_HASH_H
#define CT_RESUME_HASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CT_RESUME_HASH_LEN 32  // 256-bit hash

typedef struct ct_resume_hash_ctx ct_resume_hash_ctx;

// --- One-shot API ---

/**
 * Compute hash(resume_text) -> 32 bytes.
 *
 * - Input: UTF-8; only ASCII subset guaranteed stable in v1.
 * - Output: 32-byte hash.
 * - Returns 0 on success, non-zero on error.
 */
int ct_resume_hash_once(const uint8_t *input,
                        size_t input_len,
                        uint8_t out[CT_RESUME_HASH_LEN]);

// --- Streaming API (optional, v2) ---

ct_resume_hash_ctx *ct_resume_hash_new(void);
void ct_resume_hash_free(ct_resume_hash_ctx *ctx);

/**
 * Feed a segment of resume text into the hasher.
 * Can be called multiple times; normalization is done on the stream.
 */
int ct_resume_hash_update(ct_resume_hash_ctx *ctx,
                          const uint8_t *chunk,
                          size_t chunk_len);

/**
 * Finalize and write 32-byte hash.
 * After this, ctx must not be reused.
 */
int ct_resume_hash_final(ct_resume_hash_ctx *ctx,
                         uint8_t out[CT_RESUME_HASH_LEN]);

#ifdef __cplusplus
}
#endif

#endif // CT_RESUME_HASH_H
```

v1 can implement only `ct_resume_hash_once`; streaming can come later.

---

## 4. Implementation plan (staged)

### Stage 1 — Normalization in C (no CT yet)

* Implement `normalize.c` as a pure C function:

```c
size_t ct_normalize_ascii(const uint8_t *in, size_t in_len,
                          uint8_t *out, size_t out_cap);
```

Behavior:

* ASCII-only (drop non-ASCII or map to `?`).
* Case fold, whitespace normalization, collapse spaces, trim.

At this stage, write **unit tests** to fix the spec:

* Different whitespace sequences → canonical form.
* Punctuation handling.
* Unicode edge cases (explicitly documented as “best effort / not guaranteed”).

### Stage 2 — Hash core wrapper

Implement `hash_core.c`:

```c
int ct_hash_core_once(const uint8_t *data, size_t len,
                      uint8_t out[CT_RESUME_HASH_LEN]);
```

Inside, call:

* either a bundled C implementation of SHA-256/BLAKE2s, or
* a system library (OpenSSL/libsodium) via a clear interface.

For now, keep it **reference-grade C**; asm comes later.

Then implement `ct_resume_hash_once` in `ct_resume_hash.c`:

```c
int ct_resume_hash_once(const uint8_t *input, size_t input_len,
                        uint8_t out[CT_RESUME_HASH_LEN]) {
    if (!input || !out) return -1;

    // worst-case output length: input_len + 2 for trimming
    uint8_t *buf = malloc(input_len + 2);
    if (!buf) return -2;

    size_t norm_len = ct_normalize_ascii(input, input_len, buf, input_len + 2);
    int rc = ct_hash_core_once(buf, norm_len, out);

    // scrub buffer before free (optional)
    memset(buf, 0, norm_len);
    free(buf);

    return rc;
}
```

At this point, you have **functionally correct**, but not yet CT-disciplined, code.

### Stage 3 — Introduce constant-time primitives

Refactor normalization into:

* `normalize_ref.c` – easy to read, can be non-CT.
* `normalize_ct.c` – same semantics, but using CT patterns.

Patterns you’ll use:

* Replace `if` branches on character values with **mask-based selection**:

  ```c
  // Example: tolower for ASCII A-Z
  uint8_t is_upper = (ch >= 'A') & (ch <= 'Z'); // 0 or 1
  uint8_t mask = (uint8_t)-(int8_t)is_upper;    // 0x00 or 0xFF
  ch = ch ^ (mask & 0x20); // flip 0x20 bit only if uppercase
  ```

* For “is whitespace?”:

  ```c
  uint8_t is_space = (ch == ' ');
  uint8_t is_tab   = (ch == '\t');
  uint8_t is_nl    = (ch == '\n');
  uint8_t is_cr    = (ch == '\r');
  uint8_t is_ws = is_space | is_tab | is_nl | is_cr;
  ```

* For **collapse sequences** and **trim**, you’ll keep **state** like `seen_non_ws` and `last_was_space`, but updates must be mask-based, not branching per character.

Example technique for “only emit one space for each run”:

* Keep `uint8_t last_was_space`.
* For each char:

  * compute `is_ws`.
  * compute `should_emit_space = is_ws & !last_was_space & seen_non_ws`.
  * compute `should_emit_char = !is_ws`.
  * update output index via: `out_idx += should_emit_space | should_emit_char;`
  * write `out[out_idx]` through masked assignments.

It’s a bit ugly but stays branch-free w.r.t. contents (only loops over length).

### Stage 4 — Lock down constant-time at the library boundary

* Replace any `memcmp`, `strcmp`, etc., with CT equivalents if used.
* Add compiler flags to reduce surprises:

  * `-O2 -fno-builtin-memcmp -fwrapv` (depending on patterns used).
* Consider using `volatile` or inline asm barriers **sparingly** where you absolutely must prevent compiler reordering that breaks CT patterns.

At this point you have:

* C implementation with CT normalization primitives,
* a hash core your trust to be (practically) constant-time for your threat model.

### Stage 5 — Optional assembly

Only once the C version is correct and passes CT tests:

* Implement performance-critical pieces in assembly:

  * Tight inner loop of normalization (e.g., using SIMD to process 16/32 bytes at a time).
  * Hash compression function if you control that code.

* Gate via `#ifdef CT_RESUME_HASH_USE_ASM` so you can flip it off if needed.

---

## 5. Constant-time engineering: common failure modes

Watch out for:

1. **Early exit on special chars**

   * e.g., stopping at `'\0'` even if `input_len` is larger.
   * Always iterate fixed `input_len` chars.

2. **Branching on text features**

   * “If this is a space, skip…”: must be implemented mask-wise, not branch-wise.

3. **Compiler “help”**

   * CT tricks in C are not guaranteed if the compiler “optimizes” them.
   * You need tests (dudect) to detect regressions when upgrading compilers.

4. **Data-dependent memory access**

   * Avoid table lookups keyed by secret characters unless the table is small and you know the CPU cache behavior you’re willing to accept.

5. **Mixed CT + non-CT path**

   * Don’t accidentally branch into a simpler, non-CT normalization for “short” inputs.

---

## 6. Testing, fuzzing, and CT validation

### 6.1 Unit tests

In `tests/unit/`:

* **Normalization tests**:

  * Input → expected normalized string.
  * Cover:

    * multiple whitespace patterns,
    * edge cases (`""`, `"   "`, only punctuation),
    * some UTF-8 junk.

* **Hash tests**:

  * Known input / known output vectors.
  * Regression tests (if you change normalize rules, bump a format version).

### 6.2 Fuzz tests

In `tests/fuzz/` (with libFuzzer / AFL++ / honggfuzz):

* **Fuzz normalize:**

  * Generate random bytes, check:

    * output is valid ASCII (by spec),
    * function doesn’t crash,
    * length ≤ `input_len + 2`.

* **Fuzz roundtrip:**

  * Fuzz two inputs; if `normalize(a) == normalize(b)` then assert `hash(a) == hash(b)`; if different, hash may differ.

### 6.3 Constant-time tests

In `tests/timing/`:

* Integrate a simple `dudect`-style harness:

  * Fix a length `N`.
  * Generate many random inputs of length `N`.
  * Label them into two classes (e.g., highest bit of first byte 0 vs 1).
  * Measure runtime over many runs; compute t-test / Welch’s test statistic.

You want “no significant difference” signal. If you see big differences, dig into disassembly and rework.

You can gradually:

* Start with normalization only,
* then add overall `ct_resume_hash_once`,
* test with and without ASM.

---

## 7. Python & Rust bindings

### 7.1 Python binding (C extension or CFFI)

Minimal target: importable module with:

```python
import ct_resume_hash

digest = ct_resume_hash.hash_once("some resume text")
# returns bytes of length 32
```

Implementation sketch (C extension):

```c
// bindings/python/src/ct_resume_hash/_native.c
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "ct_resume_hash.h"

static PyObject *py_ct_resume_hash_once(PyObject *self, PyObject *args) {
    const char *input;
    Py_ssize_t input_len;

    if (!PyArg_ParseTuple(args, "s#", &input, &input_len)) {
        return NULL;
    }

    uint8_t out[CT_RESUME_HASH_LEN];
    if (ct_resume_hash_once((const uint8_t *)input,
                            (size_t)input_len,
                            out) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "ct_resume_hash_once failed");
        return NULL;
    }

    return PyBytes_FromStringAndSize((const char *)out, CT_RESUME_HASH_LEN);
}

static PyMethodDef Methods[] = {
    {"hash_once", py_ct_resume_hash_once, METH_VARARGS, "Hash resume text"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "ct_resume_hash._native",
    NULL,
    -1,
    Methods,
};

PyMODINIT_FUNC PyInit__native(void) {
    return PyModule_Create(&moduledef);
}
```

Then a light Python wrapper:

```python
# bindings/python/src/ct_resume_hash/__init__.py
from ._native import hash_once
```

Expose wheels via `pyproject.toml` + `setuptools` or `maturin` (if you lean Rust-ward).

### 7.2 Rust binding (safe wrapper)

Create `bindings/rust/`:

* FFI:

```rust
// bindings/rust/src/lib.rs
use std::os::raw::{c_int, c_uchar, c_void};
use std::slice;

pub const CT_RESUME_HASH_LEN: usize = 32;

extern "C" {
    fn ct_resume_hash_once(
        input: *const c_uchar,
        input_len: usize,
        out: *mut c_uchar,
    ) -> c_int;
}

pub fn hash_once(input: &str) -> Result<[u8; CT_RESUME_HASH_LEN], &'static str> {
    let bytes = input.as_bytes();
    let mut out = [0u8; CT_RESUME_HASH_LEN];

    let rc = unsafe {
        ct_resume_hash_once(
            bytes.as_ptr(),
            bytes.len(),
            out.as_mut_ptr(),
        )
    };

    if rc == 0 {
        Ok(out)
    } else {
        Err("ct_resume_hash_once failed")
    }
}
```

* Build with `build.rs` that links against `libctresume`.

---

## 8. Integration into your HR pipeline

### 8.1 Deployment model

You have two typical options:

1. **In-process library**

   * Link `libctresume` into your Django worker via a Python extension module.
   * Low latency, fewer moving parts.
   * Good for high-volume dedup checks.

2. **Sidecar service**

   * Build a small gRPC/HTTP service around `ct-resume-hash`.
   * Language-agnostic usage, but more infra.

Given your stack, I’d start with **Python in-process binding** integrated into:

* The pipeline that:

  * ingests resumes,
  * normalizes / parses them,
  * stores hashed fingerprints for dedup or PII-safe joins.

### 8.2 Versioning & migrations

Hash schemes *will* evolve. Plan for it:

* Store alongside each hash:

  * `algorithm`: `"ct-resume-hash"`
  * `version`: e.g. `1` (ASCII-only, SHA-256), `2` (better Unicode, BLAKE2s-keyed)
  * `salt_id` or `key_id` if using keyed hashes.

Design DB schema:

```text
resume_fingerprints(
  id,
  resume_id,
  hash BINARY(32),
  algo VARCHAR,
  version INT,
  salt_id INT NULL
)
```

When you change normalization or hash algorithm:

* bump version,
* optionally run a backfill task for new version hashes,
* support lookups by `(algo, version)`.

---

## 9. Concrete 10-day execution plan

To make this real:

**Days 1–2: Spec + skeleton**

* Write `README.md` with:

  * normalization rules,
  * hash algorithm choice,
  * constant-time guarantees.
* Create repo skeleton + CMakeLists + header.

**Days 3–4: Functional C version**

* Implement `normalize_ref.c` + unit tests.
* Implement `hash_core.c` via a known hash.
* Implement `ct_resume_hash_once`.
* Get tests passing.

**Days 5–6: CT refactor**

* Implement `normalize_ct.c` using mask tricks.
* Swap `normalize_ref` → `normalize_ct` behind a flag.
* Add basic timing test (even without full dudect).

**Days 7–8: Bindings**

* Add Python extension; minimal `hash_once()` function.
* Wire test in Python: compare C vs Python calls.

**Days 9–10: CI + integration**

* GitHub Actions:

  * build + run tests on Linux x86-64.
* Add simple demo script in your backend that:

  * pulls a few real resumes,
  * logs hashes,
  * checks dedup behavior.

From there, you can iterate: CT harness, Rust binding, ASM path, Unicode v2, etc.
