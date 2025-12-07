from pathlib import Path
from setuptools import Extension, setup

ROOT = Path(__file__).resolve().parents[1]

sources = [
    "src/ct_resume_hash/_native.c",
    str(ROOT / "src" / "ct_resume_hash.c"),
    str(ROOT / "src" / "normalize_ref.c"),
    str(ROOT / "src" / "normalize_ct.c"),
    str(ROOT / "src" / "hash_core.c"),
    str(ROOT / "src" / "sha256.c"),
]

ext_modules = [
    Extension(
        "ct_resume_hash._native",
        sources=sources,
        include_dirs=[str(ROOT / "include"), str(ROOT / "src")],
        define_macros=[("CT_RESUME_HASH_USE_CT", "1")],
        extra_compile_args=["-O2", "-fwrapv", "-fno-builtin-memcmp"],
    )
]

setup(ext_modules=ext_modules)

