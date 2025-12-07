use std::os::raw::{c_int, c_uchar};

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

    let rc = unsafe { ct_resume_hash_once(bytes.as_ptr(), bytes.len(), out.as_mut_ptr()) };
    if rc == 0 {
        Ok(out)
    } else {
        Err("ct_resume_hash_once failed")
    }
}

