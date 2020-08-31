// T4 TRNG from SDK, updated with imxrt.h symbols
// reading last entropy word, intiates a new generation cycle

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define TRNG_ENT_COUNT 16

static uint32_t rng_index;

void trng_init() {
  CCM_CCGR6 |= CCM_CCGR6_TRNG(CCM_CCGR_ON);
  TRNG_MCTL = TRNG_MCTL_RST_DEF | TRNG_MCTL_PRGM; // reset to program mode
  TRNG_MCTL = TRNG_MCTL_SAMP_MODE(2); // start run mode, vonneumann
  TRNG_ENT15; // discard any stale data, start gen cycle
}


void trng512(uint32_t *udata) {

  while ((TRNG_MCTL & TRNG_MCTL_ENT_VAL) == 0 &
         (TRNG_MCTL & TRNG_MCTL_ERR) == 0) ; // wait for entropy ready
  for (int i = 0; i < TRNG_ENT_COUNT; i++) udata[i] = *(&TRNG_ENT0 + i);
  // reading last ENT word will start a new entropy cycle
  uint32_t tmp = TRNG_ENT0;  // dummy read work-around from SDK
}

uint32_t trng_word() {
  uint32_t r;
  while ((TRNG_MCTL & TRNG_MCTL_ENT_VAL) == 0 &
         (TRNG_MCTL & TRNG_MCTL_ERR) == 0) ; // wait for entropy ready
  r = *(&TRNG_ENT0 + rng_index++);
  if (rng_index >= TRNG_ENT_COUNT) rng_index = 0;
  return r;
}

void logger() {
  // pin 12 to gnd to start
  uint32_t v[TRNG_ENT_COUNT];
  pinMode(12, INPUT_PULLUP);
  delay(2);
  while (digitalRead(12)) ; // spin
  while (1) {
    trng512(v);
    Serial.write((uint8_t *)v, sizeof(v));
  }
}

void entropy(uint32_t *w, int bytes) {
  uint8_t *byte = (uint8_t *) w;  // 64 bytes
  int i, j, v, cnts[256], bits[8];
  int vmax = 0, vmin = 999999;
  double p, e = 0, avrg = 0;
  unsigned long t;


  for (j = 0; j < 8; j++) bits[j] = 0;
  for (j = 0; j < 256; j++) cnts[j] = 0;

  for (i = 0; i < bytes; i++) {
    v = byte[i];
    if (v < vmin) vmin = v;
    if (v > vmax) vmax = v;
    cnts[v & 0xff]++;
    for (j = 0; j < 8; j++) if ( v &  1 << j) bits[j]++;
  }

  for (i = 0; i < 256; i++) {
    avrg += 1.0 * i * cnts[i];
    if (cnts[i]) {
      // Serial.print(i); Serial.print(" "); Serial.println(cnts[i]);
      p = 1.0 * cnts[i] / bytes;
      e -= p * log(p) / log(2.0);
    }
  }
  Serial.print(bytes);
  Serial.print(" bytes  avrg "); Serial.print(avrg / bytes, 3);
  Serial.print(" min max "); Serial.print(vmin); Serial.print(" "); Serial.print(vmax);
  Serial.print("  entropy "); Serial.println(e, 3);
  for (j = 0; j < 8; j++) {
    Serial.print(bits[j]);
    Serial.print(" ");
  }
  Serial.println();
}

void words() {
  uint32_t x, us, i = 0;
  while (1) {
    us = micros();
    x = trng_word();
    us = micros() - us;
    Serial.printf("%d %08x %d us\n", i++, x, us);
  }
}


void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  trng_init();
  PRREG(TRNG_STATUS);
  PRREG(TRNG_MCTL);
  uint32_t * p = (uint32_t *) &TRNG_MCTL;
  for (int i = 0; i < 16; i++) Serial.printf("%d %08X\n", i, p[i]);

  //logger();  // log to serial
  //words();  // timing test
}

void loop() {
  uint32_t t1, t2, t3, edata[4096];
  uint32_t data[16];   // 512 random bits

  t1 = micros();
  trng512(data);
  t1 = micros() - t1;
  t2 = micros();
  trng512(data);
  t2 = micros() - t2;
  t3 = micros();
  trng512(data);
  t3 = micros() - t3;
  Serial.printf("%d us  %d us  %d us   %0x\n", t1, t2, t3, data[3]);
  // collect lots of data for entropy calculation
  for (int i = 0; i < sizeof(edata) / 4; i += TRNG_ENT_COUNT) trng512(edata + i);
  //  for (int i = 0; i < sizeof(edata) / 4; i++)edata[i] = random();
  entropy(edata, sizeof(edata));

  delay(2000);
}
