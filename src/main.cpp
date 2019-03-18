#include "str_to_num.cpp"

#define LEN 21
int main ( void ) {
/*  __m256i vec = _mm256_setr_epi8(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31);
  say_m256i_u8(vec);
*/
  /*uint8_t* str = static_cast<uint8_t*>( malloc(LEN) );
  for (uint8_t i = 0; i < LEN; i++) {
    str[i] = (i % 10) + 0x30;
  }
  str[LEN-1] = '\0';
  str_to_num(str, nullptr, nullptr);
  free(str);*/
  uint8_t* str = static_cast<uint8_t*>( malloc(LEN) );
  for (uint8_t i = 0; i < LEN; i++) {
    str[i] = (i % 10) + 0x30;
  }
  str[LEN-1] = '\0';
  printf("%s\n", str);
  int64_t val = 0;
  const bool res = str_to_int64(str, &val);
  if (res) {
    printf("Success: %" PRIi64 "\n", val );
  }
  return 0;
}
