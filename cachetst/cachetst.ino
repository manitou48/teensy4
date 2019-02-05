// disableDCache() based on SDK SCB_DisableDCache()
// 32KB D and I cache
// no cache data in DTCM,  OCRAM (malloc()) and FLASH (PROGRMEM RO?) cache data
#include "pg.h"
#define REPS 1000
#define XN 1000

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define SCB_CSSELR       (*(volatile    uint32_t *)0xE000ED84) // Cache Size Selection

void disableDCache() {
  uint32_t ccsidr, sets, ways, lines, linesz;
  SCB_CSSELR = 0;     // L1 D cache   ?? compile error read only for _ID_ version
  asm("dsb");
  SCB_CCR &= ~SCB_CCR_DC;   // disable D cache
  asm("dsb");
  // clean and invalidate D cache
  ccsidr = SCB_ID_CCSIDR;
  linesz = ccsidr & 7;  // 1 is 32 bytes
  lines = 4;
  sets = (ccsidr >> 13) & 0x7fff;
  do {
    ways = (ccsidr >> 3) * 0x3ff;
    do {
      // SCB_CACHE_DCISW = (ways & 3) << 30 | (sets & 0x1ff) << 5 ;  // hangs
    } while (ways-- != 0);
  } while (sets-- != 0);
  asm("dsb");
  asm("isb");

}

float  sdot(float *a, float *b, int n) {
  float sum = 0;

  uint32_t t = micros();
  for (int k = 0; k < REPS; k++) {
    for (int i = 0; i < n; i++) sum += a[i] * b[i];
  }
  t = micros() - t;
  Serial.printf("%d us\n", t);
  return sum;
}


void setup() {
  float a[XN], b[XN], r, *am, *bm; // more than 32KB

  disableDCache();  // ? halts
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  am = (float *) malloc(XN * 4);
  bm = (float *) malloc(XN * 4);
  Serial.printf("stack 0x%x  malloc 0x%x PROGMEM 0x%x\n",
                (uint32_t)a, (uint32_t) am, (uint32_t) ap);
  for (int i = 0; i < XN; i++) a[i] = b[i] = am[i] = bm[i] = ap[i];
  PRREG(SCB_CCR);
  PRREG(SCB_ID_CCSIDR);
  PRREG(SCB_ID_CSSELR);
  PRREG(SCB_ID_CLIDR);
  r = sdot(a, b, XN);
  Serial.println(r);
  r = sdot(ap, bp, XN);
  Serial.println(r);
  r = sdot(am, bm, XN);
  Serial.println(r);
}

void loop() {

}
