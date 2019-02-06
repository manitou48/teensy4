// disableDCache() based on SDK SCB_DisableDCache()
// 32KB D and I cache
// no cache data in DTCM,  OCRAM (malloc()) and FLASH (PROGMEM RO) cache data
#include "pg.h"

#define REPS 1000
#define XN 1000
//PROGMEM float ap[XN], bp[XN];

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define SCB_CSSELR       (*(volatile    uint32_t *)0xE000ED84) // Cache Size Selection

void disableDCache() {
  uint32_t ccsidr, sets, ways;
  SCB_CSSELR = 0;     // L1 D cache   ?? compile error read only for _ID_ version
  asm("dsb");
  SCB_CCR &= ~SCB_CCR_DC;   // disable D cache
  asm("dsb");
  // clean and invalidate D cache
  ccsidr = SCB_ID_CCSIDR;
  sets = (ccsidr >> 13) & 0x7fff;
  do {
    ways = (ccsidr >> 3) & 0x3ff;
    do {
      SCB_CACHE_DCCISW = ((ways & 3) << 30) | ((sets & 0x1ff) << 5);
    } while (ways-- != 0);
  } while (sets-- != 0);
  asm("dsb");
  asm("isb");
}

float  sdot(float *a, float *b, int n, char *memlbl) {
  float sum = 0;

  uint32_t t = micros();
  for (int k = 0; k < REPS; k++) {
    for (int i = 0; i < n; i++) sum += a[i] * b[i];
  }
  t = micros() - t;
  Serial.printf("%-15s 0x%0X  %8d  %6.1f %.0f\n", memlbl, (uint32_t)a, t, 2.*n * REPS / t, sum);
  return sum;
}

void setup() {
  float a[XN], b[XN], r, *am, *bm; // more than 32KB

  Serial.begin(9600);
  while (!Serial);
  delay(1000);
#if 1
  am = (float *) malloc(XN * 4);
  bm = (float *) malloc(XN * 4);
#else
  const int bufalign = 32; //alignment of buffer cache line 32
  am = (float * )malloc(XN * 4 + bufalign);
  am = (float *) (((uintptr_t)am + bufalign) & ~ ((uintptr_t) (bufalign - 1 )));
  bm = (float * )malloc(XN * 4 + bufalign);
  bm = (float *) (((uintptr_t)bm + bufalign) & ~ ((uintptr_t) (bufalign - 1 )));
#endif
  for (int i = 0; i < XN; i++) a[i] = b[i] = am[i] = bm[i] = ap[i];
  PRREG(SCB_CCR);
  PRREG(SCB_ID_CCSIDR);
  PRREG(SCB_ID_CSSELR);
  PRREG(SCB_ID_CLIDR);
  Serial.printf("N %d  REPS %d\n", XN, REPS);
  Serial.printf("memory              addr        us    mflops  sum\n");
  sdot(a, b, XN, "stack/DTCM");
  sdot(ap, bp, XN, "malloc/OCRAM");
  sdot(am, bm, XN, "PROGMEM/flash");
  disableDCache();  // or not
  Serial.printf("D cache off\n");
  sdot(a, b, XN, "stack/DTCM");
  sdot(ap, bp, XN, "malloc/OCRAM");
  sdot(am, bm, XN, "PROGMEM/flash");
  PRREG(SCB_CCR);
}

void loop() {

}
