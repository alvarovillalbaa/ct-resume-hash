# ct-resume-hash — what it is and why it exists

- Purpose: constant-time resume fingerprinting in C with small, auditable code and Python/Rust bindings. Intended for dedup, PII-safe joins, and cross-system linkage where resume length is not secret.
- Threat model: attacker can control resume contents and measure timing; length leaks are acceptable. For equal-length inputs, normalization + SHA-256 aim for indistinguishable timing (validated with dudect-style harness).
- Guarantees:
  - Normalizes text to canonical ASCII: case-fold, collapse whitespace, drop controls, map non-ASCII to `?`.
  - Hashes normalized bytes with bundled SHA-256; output is 32 bytes.
  - Constant-time normalization path gated by `CT_RESUME_HASH_USE_CT`; reference path stays for readability/tests.
- Interfaces:
  - C API: `ct_resume_hash_once` for one-shot, streaming via `ct_resume_hash_*` ctx helpers.
  - Python binding: `ct_resume_hash.hash_once("text")`.
  - Rust binding: `ct_resume_hash::hash_once("text") -> [u8; 32]`.
- Deliverables in this repo: library sources (`src/`, `include/`), build files (CMake + per-binding builds), tests (unit/fuzz/timing), and docs.

Fit for HR stack:
- Embed in Django workers via Python extension for low-latency dedup.
- Sidecar is possible but not provided; CI-ready CMake build, fuzz hooks, and dudect-style timing sampler included.
