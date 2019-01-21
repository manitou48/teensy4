// T4 TRNG from SDK
// reading last entropy word, intiates a new generation cycle

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define TRNG_MCTL_SAMP_MODE_MASK                 (0x3U)
#define TRNG_MCTL_OSC_DIV_MASK                   (0xCU)
#define TRNG_MCTL_UNUSED4_MASK                   (0x10U)
#define TRNG_MCTL_UNUSED5_MASK                   (0x20U)
#define TRNG_MCTL_RST_DEF_MASK                   (0x40U)
#define TRNG_MCTL_FOR_SCLK_MASK                  (0x80U)
#define TRNG_MCTL_FCT_FAIL_MASK                  (0x100U)
#define TRNG_MCTL_FCT_VAL_MASK                   (0x200U)
#define TRNG_MCTL_ENT_VAL_MASK                   (0x400U)
#define TRNG_MCTL_TST_OUT_MASK                   (0x800U)
#define TRNG_MCTL_ERR_MASK                       (0x1000U)
#define TRNG_MCTL_TSTOP_OK_MASK                  (0x2000U)
#define TRNG_MCTL_LRUN_CONT_MASK                 (0x4000U)
#define TRNG_MCTL_PRGM_MASK                      (0x10000U)

// PRGM 0 run mode   1 program mode
#define TRNG_MCTL_PRGM_SHIFT                     (16U)
#define MASKVAL(mask,val)  ((~(mask) & TRNG->MCTL) | val)
#define TRNG_MCTL_PRGM(x) (((uint32_t)(((uint32_t)(x)) << TRNG_MCTL_PRGM_SHIFT)) & TRNG_MCTL_PRGM_MASK)

#define __IO volatile
#define __I  volatile
#define TRNG_ENT_COUNT 16
#define TRNG_BASE                                (0x400CC000u)
#define TRNG                                     ((TRNG_Type *)TRNG_BASE)

/** TRNG - Register Layout Typedef */
typedef struct {
  __IO uint32_t MCTL;                              /**< Miscellaneous Control Register, offset: 0x0 */
  __IO uint32_t SCMISC;                            /**< Statistical Check Miscellaneous Register, offset: 0x4 */
  __IO uint32_t PKRRNG;                            /**< Poker Range Register, offset: 0x8 */
  union {                                          /* offset: 0xC */
    __IO uint32_t PKRMAX;                            /**< Poker Maximum Limit Register, offset: 0xC */
    __I  uint32_t PKRSQ;                             /**< Poker Square Calculation Result Register, offset: 0xC */
  };
  __IO uint32_t SDCTL;                             /**< Seed Control Register, offset: 0x10 */
  union {                                          /* offset: 0x14 */
    __IO uint32_t SBLIM;                             /**< Sparse Bit Limit Register, offset: 0x14 */
    __I  uint32_t TOTSAM;                            /**< Total Samples Register, offset: 0x14 */
  };
  __IO uint32_t FRQMIN;                            /**< Frequency Count Minimum Limit Register, offset: 0x18 */
  union {                                          /* offset: 0x1C */
    __I  uint32_t FRQCNT;                            /**< Frequency Count Register, offset: 0x1C */
    __IO uint32_t FRQMAX;                            /**< Frequency Count Maximum Limit Register, offset: 0x1C */
  };
  union {                                          /* offset: 0x20 */
    __I  uint32_t SCMC;                              /**< Statistical Check Monobit Count Register, offset: 0x20 */
    __IO uint32_t SCML;                              /**< Statistical Check Monobit Limit Register, offset: 0x20 */
  };
  union {                                          /* offset: 0x24 */
    __I  uint32_t SCR1C;                             /**< Statistical Check Run Length 1 Count Register, offset: 0x24 */
    __IO uint32_t SCR1L;                             /**< Statistical Check Run Length 1 Limit Register, offset: 0x24 */
  };
  union {                                          /* offset: 0x28 */
    __I  uint32_t SCR2C;                             /**< Statistical Check Run Length 2 Count Register, offset: 0x28 */
    __IO uint32_t SCR2L;                             /**< Statistical Check Run Length 2 Limit Register, offset: 0x28 */
  };
  union {                                          /* offset: 0x2C */
    __I  uint32_t SCR3C;                             /**< Statistical Check Run Length 3 Count Register, offset: 0x2C */
    __IO uint32_t SCR3L;                             /**< Statistical Check Run Length 3 Limit Register, offset: 0x2C */
  };
  union {                                          /* offset: 0x30 */
    __I  uint32_t SCR4C;                             /**< Statistical Check Run Length 4 Count Register, offset: 0x30 */
    __IO uint32_t SCR4L;                             /**< Statistical Check Run Length 4 Limit Register, offset: 0x30 */
  };
  union {                                          /* offset: 0x34 */
    __I  uint32_t SCR5C;                             /**< Statistical Check Run Length 5 Count Register, offset: 0x34 */
    __IO uint32_t SCR5L;                             /**< Statistical Check Run Length 5 Limit Register, offset: 0x34 */
  };
  union {                                          /* offset: 0x38 */
    __I  uint32_t SCR6PC;                            /**< Statistical Check Run Length 6+ Count Register, offset: 0x38 */
    __IO uint32_t SCR6PL;                            /**< Statistical Check Run Length 6+ Limit Register, offset: 0x38 */
  };
  __I  uint32_t STATUS;                            /**< Status Register, offset: 0x3C */
  __I  uint32_t ENT[TRNG_ENT_COUNT];                           /**< Entropy Read Register, array offset: 0x40, array step: 0x4 */
  __I  uint32_t PKRCNT10;                          /**< Statistical Check Poker Count 1 and 0 Register, offset: 0x80 */
  __I  uint32_t PKRCNT32;                          /**< Statistical Check Poker Count 3 and 2 Register, offset: 0x84 */
  __I  uint32_t PKRCNT54;                          /**< Statistical Check Poker Count 5 and 4 Register, offset: 0x88 */
  __I  uint32_t PKRCNT76;                          /**< Statistical Check Poker Count 7 and 6 Register, offset: 0x8C */
  __I  uint32_t PKRCNT98;                          /**< Statistical Check Poker Count 9 and 8 Register, offset: 0x90 */
  __I  uint32_t PKRCNTBA;                          /**< Statistical Check Poker Count B and A Register, offset: 0x94 */
  __I  uint32_t PKRCNTDC;                          /**< Statistical Check Poker Count D and C Register, offset: 0x98 */
  __I  uint32_t PKRCNTFE;                          /**< Statistical Check Poker Count F and E Register, offset: 0x9C */
  __IO uint32_t SEC_CFG;                           /**< Security Configuration Register, offset: 0xA0 */
  __IO uint32_t INT_CTRL;                          /**< Interrupt Control Register, offset: 0xA4 */
  __IO uint32_t INT_MASK;                          /**< Mask Register, offset: 0xA8 */
  __I  uint32_t INT_STATUS;                        /**< Interrupt Status Register, offset: 0xAC */
  uint8_t RESERVED_0[64];
  __I  uint32_t VID1;                              /**< Version ID Register (MS), offset: 0xF0 */
  __I  uint32_t VID2;                              /**< Version ID Register (LS), offset: 0xF4 */
} TRNG_Type;

void trng_init() {
  CCM_CCGR6 |= CCM_CCGR6_TRNG(CCM_CCGR_ON);
  // program mode
  TRNG->MCTL = MASKVAL((TRNG_MCTL_PRGM_MASK | TRNG_MCTL_ERR_MASK), TRNG_MCTL_PRGM(1));
  TRNG->MCTL |= TRNG_MCTL_RST_DEF_MASK;  // reset  ? clear ERR_MASK

  // config regs ...
  TRNG->MCTL &= ~TRNG_MCTL_SAMP_MODE_MASK; // vonneuman  SDK
  uint32_t * p = (uint32_t *) TRNG;
  for (int i = 0; i < 16; i++) Serial.printf("%d %08X\n", i, p[i]);
  // run mode
  TRNG->MCTL = MASKVAL((TRNG_MCTL_PRGM_MASK | TRNG_MCTL_ERR_MASK), TRNG_MCTL_PRGM(0));
  uint32_t tmp = TRNG->ENT[TRNG_ENT_COUNT - 1]; // start gen cycle
}

void trng_deinit() {
  TRNG->MCTL = MASKVAL((TRNG_MCTL_PRGM_MASK | TRNG_MCTL_ERR_MASK), TRNG_MCTL_PRGM(1));
}

void trng512(uint32_t *udata) {

  while ((TRNG->MCTL & TRNG_MCTL_ENT_VAL_MASK) == 0 &
         (TRNG->MCTL & TRNG_MCTL_ERR_MASK) == 0) ; // wait for entropy ready
  for (int i = 0; i < TRNG_ENT_COUNT; i++) udata[i] = TRNG->ENT[i];
  // reading last ENT word will start a new entropy cycle
  uint32_t tmp = TRNG->ENT[0];  // dummy read work-around from SDK
}

void logger() {
  // pin 12 to gnd to start
  uint32_t v[16];
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


void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  trng_init();
  PRREG(TRNG->STATUS);
  PRREG(TRNG->MCTL);
  uint32_t * p = (uint32_t *) TRNG;
  for (int i = 0; i < 16; i++) Serial.printf("%d %08X\n", i, p[i]);
  //logger();  // log to serial
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
  for (int i = 0; i < sizeof(edata) / 4; i += 16) trng512(edata + i);
  //  for (int i = 0; i < sizeof(edata) / 4; i++)edata[i] = random();
  entropy(edata, sizeof(edata));

  delay(2000);
}
