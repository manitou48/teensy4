// count ticks from external pin with  QTIMER3 chnl 0   T4 pin 19 GPIO_AD_B1_00
// cascade with chnl 1 for 32-bit
// test with PWM on pin 11   jumper to 19 SCL

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR3;

uint32_t us, prev;


void setup() {
  int cnt;

  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  analogWriteFrequency(11, 60000000);  // test jumper 11 to 19
  analogWrite(11, 128);
  // pin 10 PWM
  TMR1_CTRL0 = 0;  // stop  timer
  TMR1_SCTRL0 = TMR_SCTRL_OEN; // output enable
  TMR1_CNTR0 = 0;
  TMR1_LOAD0 = 0;
  TMR1_CMPLD10 = 1 - 1;
  TMR1_CSCTRL0 = TMR_CSCTRL_CL1(1);
  TMR1_CTRL0 =  TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 ) | TMR_CTRL_LENGTH | TMR_CTRL_OUTMODE(3);

  //configure Teensy pin Compare output
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_00 = 1;

  CCM_CCGR6 |= CCM_CCGR6_QTIMER3(CCM_CCGR_ON); //enable QTMR3

  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_00 = 1;    // QT3 Timer0 on pin 19
  IOMUXC_QTIMER3_TIMER0_SELECT_INPUT = 1;  // select which input pin map

  cnt = 65536 ; // full cycle
  TMRx->CH[0].CTRL = 0; // stop
  TMRx->CH[0].CNTR = 0;
  TMRx->CH[0].LOAD = 0;  // start val after compare
  TMRx->CH[0].COMP1 = cnt - 1;  // count up to this val and start again
  TMRx->CH[0].CMPLD1 = cnt - 1;
  TMRx->CH[0].SCTRL = 0;

  TMRx->CH[1].CTRL = 0; // stop
  TMRx->CH[1].CNTR = 0;
  TMRx->CH[1].LOAD = 0;  // start val after compare
  TMRx->CH[1].COMP1 = 0;
  TMRx->CH[1].CMPLD1 = 0;
  TMRx->CH[1].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(4);  //clock from clock 0

  TMRx->CH[0].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(0) | TMR_CTRL_LENGTH ;

  us = micros();
}

void loop() {
  if (micros() - us > 1000000) {
    uint32_t ticks = TMRx->CH[0].CNTR + 65536 * TMRx->CH[1].HOLD; // atomic
    us = micros();
    Serial.println(ticks - prev);
    prev = ticks;
  }

}
