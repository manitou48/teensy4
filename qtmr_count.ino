// count ticks from external pin with  QTIMER4 chnl 2  pin 9 GPIO_B0_11
// test with PWM on pin 8   jumper to 9

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;

uint32_t prev;

IntervalTimer it1;
volatile uint32_t ticks, dataReady;

void it1cb() {
  ticks = TMRx->CH[2].CNTR | TMRx->CH[3].HOLD << 16; // atomic
  dataReady = 1;
}

void setup() {
  int cnt;

  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  analogWriteFrequency(8, 50000000);  // test jumper 8 to 9, max 75mhz
  analogWrite(8, 128);

  CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON); //enable QTMR4

  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11 = 1;    // QT4 Timer2 on pin 9

  cnt = 65536 ; // full cycle
  TMRx->CH[2].CTRL = 0; // stop
  TMRx->CH[2].CNTR = 0;
  TMRx->CH[2].LOAD = 0;  // start val after compare
  TMRx->CH[2].COMP1 = cnt - 1;  // count up to this val and start again
  TMRx->CH[2].CMPLD1 = cnt - 1;
  TMRx->CH[2].SCTRL = 0;

  TMRx->CH[3].CTRL = 0; // stop
  TMRx->CH[3].CNTR = 0;
  TMRx->CH[3].LOAD = 0;  // start val after compare
  TMRx->CH[3].COMP1 = 0;
  TMRx->CH[3].CMPLD1 = 0;
  TMRx->CH[3].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(6);  //clock from clock 2

  TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH ;

  it1.begin(it1cb, 1000000);  // microseconds
}

void loop() {
  if (dataReady) {
    Serial.println(ticks - prev);
    prev = ticks;
    dataReady = 0;
  }
}
