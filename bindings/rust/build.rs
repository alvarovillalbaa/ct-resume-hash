use std::path::PathBuf;

fn main() {
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("bindings dir")
        .parent()
        .expect("repo root")
        .to_path_buf();

    let mut build = cc::Build::new();
    build
        .define("CT_RESUME_HASH_USE_CT", None)
        .include(root.join("include"))
        .file(root.join("src/ct_resume_hash.c"))
        .file(root.join("src/normalize_ref.c"))
        .file(root.join("src/normalize_ct.c"))
        .file(root.join("src/hash_core.c"))
        .file(root.join("src/sha256.c"));

    build.compile("ct_resume_hash");
}

