# Constant-time posture, risks, and next steps

What is constant-time here
- Normalization: mask-based CT path (`CT_RESUME_HASH_USE_CT`) removes data-dependent branches; still iterates over declared length (length not secret).
- Hash: bundled SHA-256 is conventional portable C; assumed CT for this threat model, but not formally constant-time on all CPUs.
- Memory hygiene: temporary normalization buffer is zeroed before free; streaming buffer is cleared at `final`.

Residual risks / gaps
- SHA-256 implementation is not proven CT under cache effects; if attacker can observe micro-architectural leakage, consider a vetted CT SHA-256 or keyed hash (BLAKE2s keyed) to resist rainbow tables.
- Streaming API buffers all input; normalization is not incremental. Very large inputs could increase timing variance and memory footprint.
- Non-ASCII mapping to `?` may reduce dedup quality for international resumes; rules are ASCII-first.
- Dudect harness provided is a sampler only; no automated pass/fail gate in CI.
- No key management or salt support; hashes are deterministic and reversible via dictionary attack if input space is small.

Quick improvements (order of impact)
- Swap SHA-256 for BLAKE2s keyed mode with per-tenant keys; add key ID to outputs (DB schema note in `docs/init.md`).
- Add dudect (or ctgrind) to CI with fixed-length test vectors for regression catching.
- Implement streaming normalization to avoid buffering; process in fixed-size blocks while preserving CT properties.
- Expand Unicode handling with an explicit spec + versioned hash format; store `(algo, version, salt_id)` alongside hashes.
- Provide minimal HTTP/gRPC sidecar for language-agnostic deployments if needed.
