# simd-testing

---

testing out string-to-number conversion with vectorized intrinsics, which should
be easy but isn't because Intel forgot some instructions in AVX2.

even if you had AVX512 (which i don't), there's still no instruction to multiply
64-bit numbers and get 64-bit numbers... or to horizontally add u64 or u8...

where is `_mm256_mul_epu64`???
