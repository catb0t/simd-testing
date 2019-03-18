#include <cinttypes>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <immintrin.h>

#ifndef __AVX2__
#pragma message "no AVX2"
#define NO_AVX2
#else
#pragma message "have AVX2"
#endif

#ifndef STR_NUM_MAXLEN
#define STR_NUM_MAXLEN 8192
#endif

#define COUNT_U8S_IN_M256 32
#define COUNT_U64S_IN_M256 8
#define LL_MAX_BASE_10    20

alignas(64)
static const uint64_t tens_powers[1][COUNT_U8S_IN_M256] = {
  {
    (uint64_t) 10^31, (uint64_t) 10^30, (uint64_t) 10^29, (uint64_t) 10^28, (uint64_t) 10^27, (uint64_t) 10^26, (uint64_t) 10^25, (uint64_t) 10^24, // 2 m256is in this row
    (uint64_t) 10^23, (uint64_t) 10^22, (uint64_t) 10^21, (uint64_t) 10^20, (uint64_t) 10^19, (uint64_t) 10^18, (uint64_t) 10^17, (uint64_t) 10^16, // another 2
    (uint64_t) 10^15, (uint64_t) 10^14, (uint64_t) 10^13, (uint64_t) 10^12, (uint64_t) 10^11, (uint64_t) 10^10, (uint64_t) 10^9, (uint64_t) 10^8,   // etc
    (uint64_t) 10^7, (uint64_t) 10^6, (uint64_t) 10^5, (uint64_t) 10^4, (uint64_t) 10^3, (uint64_t) 10^2, (uint64_t) 10^1, (uint64_t) 10^0
  },
};

size_t strnlen_c (const uint8_t* const str, const size_t maxsize);
bool str_to_num (const uint8_t* const str, /* out */ int64_t* const i_res, /* out */long double* const d_res);

__m256i mm_subs_epu8 (const __m256i a, const __m256i b);
void say_m256i_u8 (const __m256i);

size_t strnlen_c (const uint8_t* const s, const size_t maxsize) {
  if ((NULL == s) || '\0' == s[0]) { return 0; }

  size_t i = 1;
  for (; i < maxsize; i++) {
    if ('\0' == s[i]) { return i; }
  }
  return i;
}

void say_m256i_u8 (const __m256i in) {
  alignas(32) uint8_t arr[32];
  _mm256_storeu_si256( (__m256i *) &arr, in);
  printf("32 of u8:\n\t%d %d %d %d | %d %d %d %d | %d %d %d %d | %d %d %d %d\n\t%d %d %d %d | %d %d %d %d | %d %d %d %d | %d %d %d %d\n",
    arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7], arr[8], arr[9], arr[10], arr[11], arr[12], arr[13], arr[14], arr[15], arr[16], arr[17], arr[18], arr[19], arr[20], arr[21], arr[22], arr[23], arr[24], arr[25], arr[26], arr[27], arr[28], arr[29], arr[30], arr[31]);
}

// SSE2: __m128i _mm_subs_epu8 (__m128i a, __m128i b)
// AVX2: __m256i _mm256_subs_epu8 (__m256i a, __m256i b)
__m256i mm_subs_epu8 (const __m256i a, const __m256i b) {
  #ifndef NO_AVX2
    return _mm256_subs_epu8(a, b);
  #else
    __m128i a_high, a_low;
    _mm256_storeu2_m128i(&a_high, &a_low, a);

    __m128i b_high, b_low;
    _mm256_storeu2_m128i(&b_high, &b_low, b);

    const __m128i
      // first, subtract a_high from b_high
      high_sub = _mm_subs_epi8(a_high, b_high),
      // then, subtract a_low from b_low
      low_sub = _mm_subs_epi8(a_low, b_low);

    // two 128-bit values [...] combine them into a 256-bit value
    return _mm256_loadu2_m128i(&high_sub, &low_sub);
  #endif
}


bool str_to_int64 (const uint8_t* const buf, /* out */ int64_t* const val);
bool str_to_int64 (const uint8_t* const buf, /* out */ int64_t* const val) {
  const size_t slen = strnlen_c(buf, STR_NUM_MAXLEN);
  if ( 0 == slen || slen > 32 ) { return false; }

  const bool
    is_neg   = '-' == buf[0],
    has_sign = is_neg || ('+' == buf[0]);

  const uint8_t digits_len = ((uint8_t) slen) - has_sign;

  // temporary
  assert(digits_len <= LL_MAX_BASE_10);

  const uint8_t padding_zeroes = COUNT_U8S_IN_M256 - digits_len;

  alignas(8) uint8_t* const padded_zeroes_buf = static_cast<uint8_t* const>( memcpy(
    static_cast<uint8_t* const>( calloc(COUNT_U8S_IN_M256, (sizeof (uint8_t)) )) + padding_zeroes,
    buf,
    digits_len
  ) ) - padding_zeroes;

  static const __m256i
    digit_char_base = _mm256_set1_epi8(0x30); // 32 u8s of 0x30
  const __m256i
    chars = _mm256_load_si256( reinterpret_cast<const __m256i* const>( padded_zeroes_buf )),
    digits = mm_subs_epu8(chars, digit_char_base);;
  free(padded_zeroes_buf);

  say_m256i_u8(digits);

  //__m256i *const results = reinterpret_cast<__m256i* const>( malloc((sizeof (__m256i)) * COUNT_U64S_IN_M256 ) );
  //  *const mul_tens_places = reinterpret_cast<__m256i* const>( malloc((sizeof (__m256i)) * COUNT_U64S_IN_M256 ) ),
  //  *const digits_u64 = reinterpret_cast<__m256i* const>( malloc((sizeof (__m256i)) * COUNT_U64S_IN_M256 ) );

  alignas(COUNT_U8S_IN_M256) uint8_t digits_tmp[COUNT_U8S_IN_M256];
  _mm256_store_si256( (__m256i *) &digits_tmp, digits);

  for (int i = 0; i < COUNT_U8S_IN_M256; i++) {
    printf("%d ", digits_tmp[i]);
  }

  uint64_t result = 0;

  for (int i = 0; i < COUNT_U64S_IN_M256; i++) {
    const uint64_t* const tens_powers_octet_base = tens_powers[0] + i;
    //mul_tens_places[i]
    const __m256i
      tens_pows = _mm256_set_epi64x( (int64_t) tens_powers_octet_base[0], (int64_t) tens_powers_octet_base[1], (int64_t) tens_powers_octet_base[2], (int64_t) tens_powers_octet_base[3]),
      digits_u64 = _mm256_set_epi64x(digits_tmp[0], digits_tmp[1], digits_tmp[2], digits_tmp[3]),
    // NOTE: this only works up to 10^9 because __m256i _mm256_mul_epu32 (__m256i a, __m256i b) only multiplies the lower 32 bits
    // where is _mm256_mul_epu64 ??
    // i would even accept _mm256_mullo_epi32 except there's no _mm256_mulhi_epi32
      this_res = _mm256_mul_epu32(tens_pows, digits_u64);
    result += static_cast<uint64_t>( this_res[0] + this_res[1] + this_res[2] + this_res[3]);
  }

  //free(mul_tens_places);
  //free(digits_u64);
  *val = (is_neg ? -1 : 1) * ((int64_t) result);
  return true;
}

bool str_to_num (const uint8_t* const buf, /* out */ int64_t* const i_res, /* out */long double* const d_res) {
  const size_t slen = strnlen_c(buf, STR_NUM_MAXLEN);
  if ( 0 == slen ) { return false; }
  assert(64 == slen);

  const bool
    is_neg   = '-' == buf[0],
    has_sign = is_neg || ('+' == buf[0]);

  // not loop-ified or padded yet; only doing a string of single-chunk size
  (void) i_res;
  (void) d_res;
  (void) is_neg;
  (void) has_sign;

  static const __m256i
    upper_tps = _mm256_load_si256( reinterpret_cast<const __m256i *>( tens_powers[0] )),
    lower_tps = _mm256_load_si256( reinterpret_cast<const __m256i *>( tens_powers[1] ));

  // 30h is the number to subtract from ASCII numerals to get their base 10 values
  const __m256i
    digit_char_base = _mm256_set1_epi8(0x30),
    buf_low  = _mm256_load_si256( reinterpret_cast<const __m256i *>(buf + 0) ),
    buf_high = _mm256_load_si256( reinterpret_cast<const __m256i *>(buf + 32) ),
    digits_low  = mm_subs_epu8(buf_low, digit_char_base),
    digits_high = mm_subs_epu8(buf_high, digit_char_base);

  say_m256i_u8(digits_low);
  say_m256i_u8(digits_high);

  (void) upper_tps;
  (void) lower_tps;

  return false;
}
