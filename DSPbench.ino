// DSP benchmark https://community.nxp.com/thread/327833
//#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"
#define printf Serial.printf
#define TIME_STRUCT uint32_t
#define _time_delay delay

#define REVERSEBITS 0

enum { TEST_INPUT_ARRAY_SIZE =  100 };
static float32_t DEBUG_preformances_test_input_f31[TEST_INPUT_ARRAY_SIZE];
static q31_t     DEBUG_preformances_test_input_q31[TEST_INPUT_ARRAY_SIZE];
static q15_t     DEBUG_preformances_test_input_q15[TEST_INPUT_ARRAY_SIZE];
static float32_t DEBUG_preformances_test_output_f31[TEST_INPUT_ARRAY_SIZE];
static q15_t     DEBUG_preformances_test_output_q15[TEST_INPUT_ARRAY_SIZE];
static q31_t     DEBUG_preformances_test_output_q31[TEST_INPUT_ARRAY_SIZE];

static char standard_format_vector_mul[] = "- %-20s - %6.3f us ; // %-16s %4d\n";
static char header_format_vector[]       = "  %-18s Time used/vector   %-28s %s\n";

static char standard_format[]            = "- %-20s - %6.3f us ; // %-28s\n";
static char header_format_std[]          = "  %-18s Time used/sample   %-28s\n";

static char standard_format_fir[]        = "- %-20s - %6.3f us ; //   %-18s   %4d %6d\n";
static char header_format_fir[]          = "  %-20s Time used/sample    %-20s %4s %6s\n";

static char standard_format_iir[]        = "- %-33s - %6.3f us ; //   %-18s   %4d %6d\n";
static char header_format_iir[]          = "  %-33s Time used/sample    %-20s %4s %6s\n";

static char standard_format_fft[]        = "- %-20s - %6.1f us ; // %-16s     %4d\n";
static char header_format_fft[]          = "  %-20s   Time used         %-28s %4s\n";

inline void _time_get(uint32_t *t) {
  *t = micros();
}

void _time_diff(uint32_t *t0, uint32_t *t1, uint32_t *t) {
  *t = *t1 - *t0;
}

static float test_arm_mult_f32(uint32_t NLOOPS, uint32_t length)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  float32_t * pSrcA = (float32_t *)malloc(sizeof(float32_t) * length);
  float32_t * pSrcB = (float32_t *)malloc(sizeof(float32_t) * length);
  float32_t * pDst  = (float32_t *)malloc(sizeof(float32_t) * length);

  for (int i = 0; i < length; i++)
  {
    pSrcA[i] = rand();
    pSrcB[i] = rand();
  }

  _time_get(&start_time);
  for (uint32_t n = 0; n < NLOOPS; n++)
  {
    for (int i = 0; i < TEST_INPUT_ARRAY_SIZE; i++)
    {
      arm_mult_f32(pSrcA, pSrcB, pDst, length);
    }
  }
  _time_get(&end_time);

  free(pSrcA);
  free(pSrcB);
  free(pDst);

  _time_diff(&start_time, &end_time,  &timediff_result);


  float time_used_us_std_f32 = timediff_result / (float)(NLOOPS * TEST_INPUT_ARRAY_SIZE);

  printf(standard_format_vector_mul, "arm_mult_f32", time_used_us_std_f32, "real float32", length);
  _time_delay (20);

  return time_used_us_std_f32;
}

static float test_arm_mult_q31(uint32_t NLOOPS, uint32_t length)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  q31_t * pSrcA = (q31_t *)malloc(sizeof(q31_t) * length);
  q31_t * pSrcB = (q31_t *)malloc(sizeof(q31_t) * length);
  q31_t * pDst  = (q31_t *)malloc(sizeof(q31_t) * length);

  for (int i = 0; i < length; i++)
  {
    pSrcA[i] = rand();
    pSrcB[i] = rand();
  }

  _time_get(&start_time);
  for (uint32_t n = 0; n < NLOOPS; n++)
  {
    for (int i = 0; i < TEST_INPUT_ARRAY_SIZE; i++)
    {
      arm_mult_q31(pSrcA, pSrcB, pDst, length);
    }
  }
  _time_get(&end_time);

  free(pSrcA);
  free(pSrcB);
  free(pDst);

  _time_diff(&start_time, &end_time,  &timediff_result);

  float time_used_us_std_f32 = timediff_result / (float)(NLOOPS * TEST_INPUT_ARRAY_SIZE);

  printf(standard_format_vector_mul, "arm_mult_q31", time_used_us_std_f32, "real q31", length);
  _time_delay (20);

  return time_used_us_std_f32;
}

static float test_arm_mult_q15(uint32_t NLOOPS, uint32_t length)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  q15_t * pSrcA = (q15_t *)malloc(sizeof(q15_t) * length);
  q15_t * pSrcB = (q15_t *)malloc(sizeof(q15_t) * length);
  q15_t * pDst  = (q15_t *)malloc(sizeof(q15_t) * length);

  for (int i = 0; i < length; i++)
  {
    pSrcA[i] = rand();
    pSrcB[i] = rand();
  }

  _time_get(&start_time);
  for (uint32_t n = 0; n < NLOOPS; n++)
  {
    for (int i = 0; i < TEST_INPUT_ARRAY_SIZE; i++)
    {
      arm_mult_q15(pSrcA, pSrcB, pDst, length);
    }
  }
  _time_get(&end_time);

  free(pSrcA);
  free(pSrcB);
  free(pDst);

  _time_diff(&start_time, &end_time,  &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS * TEST_INPUT_ARRAY_SIZE);

  printf(standard_format_vector_mul, "arm_mult_q15", time_used_us, "real q15", length);
  _time_delay (20);

  return time_used_us;
}

static float test_single_q31_sin_cos(uint32_t NLOOPS, const char *func_name)
{
  TIME_STRUCT q31_sincos_start_time, q31_sincos_end_time, timediff_result;

  _time_get(&q31_sincos_start_time);
  for (int n = 0; n < NLOOPS; n++)
  {
    q31_t sinVal, cosVal;
    q31_t  *test_in = DEBUG_preformances_test_input_q31;
    q31_t  *test_out = DEBUG_preformances_test_output_q31;
    for (int i = 0; i < TEST_INPUT_ARRAY_SIZE; i++)
    {

      arm_sin_cos_q31(*test_in++, &sinVal, &cosVal);
      *test_out++ = sinVal;
    }
  }
  _time_get(&q31_sincos_end_time);

  _time_diff(&q31_sincos_start_time, &q31_sincos_end_time,      &timediff_result);

  float time_used_us_sincos_q31 = timediff_result / (float)(NLOOPS * TEST_INPUT_ARRAY_SIZE);

  printf(standard_format, "arm_sin_cos_q31", time_used_us_sincos_q31, "real q31_t");
  _time_delay (20);

  return time_used_us_sincos_q31;
}

static float test_single_f32_sin_cos(uint32_t NLOOPS, const char *func_name)
{
  TIME_STRUCT sincos_start_time, sincos_end_time, timediff_result;

  _time_get(&sincos_start_time);
  for (int n = 0; n < NLOOPS; n++)
  {
    float32_t sinVal, cosVal;
    float32_t  *test_in = DEBUG_preformances_test_input_f31;
    float32_t  *test_out = DEBUG_preformances_test_output_f31;
    for (int i = 0; i < TEST_INPUT_ARRAY_SIZE; i++)
    {

      arm_sin_cos_f32(*test_in++, &sinVal, &cosVal);
      *test_out++ = sinVal;
    }
  }
  _time_get(&sincos_end_time);

  _time_diff(&sincos_start_time, &sincos_end_time,      &timediff_result);


  float time_used_us_sincos_q31 = timediff_result / (float)(NLOOPS * TEST_INPUT_ARRAY_SIZE);

  printf(standard_format, "arm_sin_cos_f32", time_used_us_sincos_q31, "real float32");
  _time_delay (20);

  return time_used_us_sincos_q31;
}

static float test_cfft_q15(bool radex4, uint32_t NLOOPS,  uint16_t fftLen)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  q15_t * pSrc = (q15_t *)malloc(sizeof(q15_t) * 4 * fftLen);
  for (int i = 0; i < 4 * fftLen; i++)
    pSrc [i] = 0xFFFF & rand();

  if (! radex4)
  {
    // arm_cfft_radix2_q15    arm_cfft_radix2_init_q15
    arm_cfft_radix2_instance_q15  S;

    arm_cfft_radix2_init_q15(&S, fftLen, 0, REVERSEBITS);


    _time_get(&start_time);
    for (int n = 0; n < NLOOPS; n++)
    {
      arm_cfft_radix2_q15(&S, pSrc);
    }
    _time_get(&end_time);
  }
  else
  {
    // arm_cfft_radix4_q15    arm_cfft_radix4_init_q15
    arm_cfft_radix4_instance_q15  S;

    arm_cfft_radix4_init_q15(&S, fftLen, 0, REVERSEBITS);

    _time_get(&start_time);
    for (int n = 0; n < NLOOPS; n++)
    {
      arm_cfft_radix4_q15(&S, pSrc);
    }
    _time_get(&end_time);

  }
  free(pSrc);

  _time_diff(&start_time, &end_time,      &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS);
  printf(standard_format_fft, (! radex4) ? "arm_cfft_radix2_q15" : "arm_cfft_radix4_q15", time_used_us, "real q15_t", (uint32_t)fftLen);
  _time_delay (20);

  return time_used_us;
}

static float test_cfft_q31(bool radex4, uint32_t NLOOPS,  uint16_t fftLen)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  q31_t * pSrc = (q31_t *)malloc(sizeof(q31_t) * 4 * fftLen);
  for (int i = 0; i < 4 * fftLen; i++)
    pSrc [i] = rand();

  if (! radex4)
  {
    // arm_cfft_radix2_q31    arm_cfft_radix2_init_q31
    arm_cfft_radix2_instance_q31  S;

    arm_cfft_radix2_init_q31(&S, fftLen, 0, REVERSEBITS);


    _time_get(&start_time);
    for (int n = 0; n < NLOOPS; n++)
    {
      arm_cfft_radix2_q31(&S, pSrc);
    }
    _time_get(&end_time);
  }
  else
  {
    // arm_cfft_radix4_q31    arm_cfft_radix4_init_q31
    arm_cfft_radix4_instance_q31  S;

    arm_cfft_radix4_init_q31(&S, fftLen, 0, REVERSEBITS);

    _time_get(&start_time);
    for (int n = 0; n < NLOOPS; n++)
    {
      arm_cfft_radix4_q31(&S, pSrc);
    }
    _time_get(&end_time);

  }
  free(pSrc);

  _time_diff(&start_time, &end_time,      &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS);
  printf(standard_format_fft, (! radex4) ? "arm_cfft_radix2_q31" : "arm_cfft_radix4_q31", time_used_us, "real q31_t", (uint32_t)fftLen);
  _time_delay (20);

  return time_used_us;
}

static float test_cfft_f32(bool radex4, uint32_t NLOOPS,  uint16_t fftLen)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  float32_t * pSrc = (float32_t *)malloc(sizeof(float32_t) * 4 * fftLen);
  for (int i = 0; i < 4 * fftLen; i++)
    pSrc [i] = rand();

  if (! radex4)
  {
    // arm_cfft_radix2_f32    arm_cfft_radix2_init_f32
    arm_cfft_radix2_instance_f32  S;

    arm_cfft_radix2_init_f32(&S, fftLen, 0, REVERSEBITS);


    _time_get(&start_time);
    for (int n = 0; n < NLOOPS; n++)
    {
      arm_cfft_radix2_f32(&S, pSrc);
    }
    _time_get(&end_time);
  }
  else
  {
    // arm_cfft_radix4_f32    arm_cfft_radix4_init_f32
    arm_cfft_radix4_instance_f32  S;

    arm_cfft_radix4_init_f32(&S, fftLen, 0, REVERSEBITS);

    _time_get(&start_time);
    for (int n = 0; n < NLOOPS; n++)
    {
      arm_cfft_radix4_f32(&S, pSrc);
    }
    _time_get(&end_time);

  }
  free(pSrc);

  _time_diff(&start_time, &end_time,      &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS);
  printf(standard_format_fft, (! radex4) ? "arm_cfft_radix2_f32" : "arm_cfft_radix4_f32", time_used_us, "real float32_t", (uint32_t)fftLen);
  _time_delay (20);

  return time_used_us;
}

static float new_cfft_q31( uint32_t NLOOPS,  uint16_t fftLen)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  q31_t * pSrc = (q31_t *)malloc(sizeof(q31_t) * 4 * fftLen);
  for (int i = 0; i < 4 * fftLen; i++)
    pSrc [i] = rand();

  _time_get(&start_time);
  for (int n = 0; n < NLOOPS; n++)
  {
    arm_cfft_q31(&arm_cfft_sR_q31_len1024, pSrc, 0, REVERSEBITS);
  }
  _time_get(&end_time);

  free(pSrc);

  _time_diff(&start_time, &end_time,      &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS);
  printf(standard_format_fft,  "arm_cfft_q31", time_used_us, "real q31_t", (uint32_t)fftLen);
  _time_delay (20);

  return time_used_us;
}

static float new_cfft_q15( uint32_t NLOOPS,  uint16_t fftLen)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  q15_t * pSrc = (q15_t *)malloc(sizeof(q15_t) * 4 * fftLen);
  for (int i = 0; i < 4 * fftLen; i++)
    pSrc [i] = rand();

  _time_get(&start_time);
  for (int n = 0; n < NLOOPS; n++)
  {
    arm_cfft_q15(&arm_cfft_sR_q15_len1024, pSrc, 0, REVERSEBITS);
  }
  _time_get(&end_time);

  free(pSrc);

  _time_diff(&start_time, &end_time,      &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS);
  printf(standard_format_fft,  "arm_cfft_q15", time_used_us, "real q15_t", (uint32_t)fftLen);
  _time_delay (20);

  return time_used_us;
}

static float new_cfft_f32( uint32_t NLOOPS,  uint16_t fftLen)
{
  TIME_STRUCT start_time, end_time, timediff_result;
  float32_t * pSrc = (float32_t *)malloc(sizeof(float32_t) * 4 * fftLen);
  for (int i = 0; i < 4 * fftLen; i++)
    pSrc [i] = rand();

  _time_get(&start_time);
  for (int n = 0; n < NLOOPS; n++)
  {
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, pSrc, 0, REVERSEBITS);
  }
  _time_get(&end_time);

  free(pSrc);

  _time_diff(&start_time, &end_time,      &timediff_result);


  float time_used_us = timediff_result / (float)(NLOOPS);
  printf(standard_format_fft,  "arm_cfft_f32", time_used_us, "real float32_t", (uint32_t)fftLen);
  _time_delay (20);

  return time_used_us;
}


void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  test_arm_mult_f32(2000,    8);
  test_arm_mult_f32( 500,   64);
  test_arm_mult_f32( 150,  256);
  test_arm_mult_f32(  50, 1024);
  test_arm_mult_q31(2000,    8);
  test_arm_mult_q31( 500,   64);
  test_arm_mult_q31( 150,  256);
  test_arm_mult_q31(  50, 1024);
  test_arm_mult_q15(2000,    8);
  test_arm_mult_q15( 500,   64);
  test_arm_mult_q15( 150,  256);
  test_arm_mult_q15(  50, 1024);

  test_single_f32_sin_cos(1000, "arm_sin_cos_f32");
  test_single_q31_sin_cos(1000, "arm_sin_cos_q31");

  test_cfft_q15(false, 4000,   64);
  test_cfft_q15(false, 1000,  256);
  test_cfft_q15(false,  200, 1024);
  test_cfft_q15(true,  8000,   64);
  test_cfft_q15(true,  2000,  256);
  test_cfft_q15(true,   200, 1024);

  test_cfft_q31(false, 2400,   64);
  test_cfft_q31(false,  600,  256);
  test_cfft_q31(false,  100, 1024);
  test_cfft_q31(true,  2400,   64);
  test_cfft_q31(true,   600,  256);
  test_cfft_q31(true,   100, 1024);

  test_cfft_f32(false, 4000,   64);
  test_cfft_f32(false, 1000,  256);
  test_cfft_f32(false,  200, 1024);
  test_cfft_f32(true,  4000,   64);
  test_cfft_f32(true,  1000,  256);
  test_cfft_f32(true,   100, 1024);
  new_cfft_q15(100, 1024);
  new_cfft_q31(100, 1024);
  new_cfft_f32(100, 1024);

}

void loop() {

}
