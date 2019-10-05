// T4 WDOG1 based on SDK
#define PRREG(x) Serial.printf(#x" 0x%x\n",x);

#define WDOG1_WICR_WTIS ((uint16_t)(1<<14))
#define WDOG1_WICR_WIE ((uint16_t)(1<<15))

void wdog1_isr() {
  WDOG1_WICR |= WDOG1_WICR_WTIS;
  wdog1_feed();
  Serial.printf("fed the dog\n");
}

void wdog1_init() {
  WDOG1_WMCR = 0;   // disable power down PDE
  uint8_t wt = 20;  //  10.5 secs reset timeout
  WDOG1_WCR |=  (wt << 8) | WDOG_WCR_WDE | WDOG_WCR_WDT | WDOG_WCR_SRE;
  _VectorsRam[16 + IRQ_WDOG1] = wdog1_isr;
  NVIC_ENABLE_IRQ(IRQ_WDOG1);
}

void wdog1_reset() {
  WDOG1_WCR &= ~WDOG_WCR_SRS;
  // SCB_AIRCR = 0x05FA0004;  // does reset too, reported as TOUT
}

void wdog1_feed() {
  // feed the dog
  WDOG1_WSR = 0x5555;
  WDOG1_WSR = 0xAAAA;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  PRREG(WDOG1_WRSR);
  PRREG(WDOG1_WCR);
  PRREG(WDOG1_WICR);
  PRREG(WDOG1_WMCR);

  if (WDOG1_WRSR & WDOG_WRSR_POR) {
    Serial.printf("POR reset ... init for 10s wdog timeout\n");
    wdog1_init();   // 10s wdog timeout
  }
  if (WDOG1_WRSR & WDOG_WRSR_TOUT) {
    Serial.printf("WDOG TOUT reset ... software reset in 5 seconds\n");
    wdog1_init();   // 10s wdog timeout
    delay(5000);
    wdog1_reset();
  }
  if (WDOG1_WRSR & WDOG_WRSR_SFTW) {
    Serial.printf("SFTW reset  ... feed the dog every 5s\n");
    WDOG1_WICR = WDOG1_WICR_WIE | WDOG1_WICR_WTIS | 10; //interrupt 5s
    wdog1_init();
  }
  PRREG(WDOG1_WRSR);
  PRREG(WDOG1_WCR);
  PRREG(WDOG1_WICR);
  PRREG(WDOG1_WMCR);
}

void loop() {
  Serial.printf("%d ms\n", millis());
  delay(1000);
}
