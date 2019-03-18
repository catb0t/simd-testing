#include <stdio.h>
#include <immintrin.h>

int main(void) {
  const __m256d a = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);

  const __m256d c = _mm256_add_pd(a, a);

  __attribute__ ((aligned (32))) double* const output = malloc((sizeof (double)) * 4 );
  _mm256_store_pd(output, c);

  printf("%f %f %f %f\n",
         output[0], output[1], output[2], output[3]);

  free(output);
  return 0;
}
