// flexio pwm  from SDK
// flexio2 clock 480 mhz.  pin 12 2:1   TIMER 0   no shifter

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define PWMHZ 50000

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  CCM_CCGR3 |= CCM_CCGR3_FLEXIO2(CCM_CCGR_ON);
  CCM_CS1CDR &= ~(CCM_CS1CDR_FLEXIO2_CLK_PODF(7) | CCM_CS1CDR_FLEXIO2_CLK_PRED(7)); // clear
  CCM_CS1CDR |= CCM_CS1CDR_FLEXIO2_CLK_PODF(7) | CCM_CS1CDR_FLEXIO2_CLK_PRED(5);
  int flexhz = 480000000 / 8 / 6;   // 480mhz
  int sum = (flexhz * 2 / PWMHZ + 1) / 2;
  int duty = 100 - 25;    // 25% high
  int locnt = (sum * duty / 50 + 1) / 2;
  int hicnt = sum - locnt;
  FLEXIO2_TIMCMP0 = ((locnt - 1) << 8 ) | (hicnt - 1);
  FLEXIO2_TIMCTL0 = FLEXIO_TIMCTL_PINSEL(1) | FLEXIO_TIMCTL_TRGPOL | FLEXIO_TIMCTL_TIMOD(2)
                    | FLEXIO_TIMCTL_PINCFG(3) | FLEXIO_TIMCTL_TRGSRC;
  FLEXIO2_TIMCFG0 = 0;
  Serial.printf("flexhz %d  pwmhz %d  lo %d hi %d\n", flexhz, PWMHZ, locnt, hicnt);
  PRREG(CCM_CSCMR2);
  PRREG(CCM_CS1CDR);
  PRREG(CCM_CCGR3);
  PRREG(FLEXIO2_PARAM);
  PRREG(FLEXIO2_TIMCTL0);
  PRREG(FLEXIO2_TIMCFG0);
  PRREG(FLEXIO2_TIMCMP0);
  *(portConfigRegister(12)) = 4; // alt
  FLEXIO2_CTRL |= FLEXIO_CTRL_FLEXEN;
}

void loop() {
}
