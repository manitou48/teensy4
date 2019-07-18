// QTIMER4 cascade  test   F_BUS_ACTUAL 150 mhz, 50% duty  ch 53
// 16-bit counter, chain with 3 channels, count us

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define IRQ_QTIMERx IRQ_QTIMER4

IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;


volatile uint32_t ticks, isrms;
void my_isr() {
  TMRx->CH[1].CSCTRL &= ~(TMR_CSCTRL_TCF1);  // clear
  ticks++;
  isrms = millis();
  asm volatile ("dsb");  // wait for clear  memory barrier
}


void isr_init(int hz) {
  int cnt, pcs = 0;
  cnt = 150000000 / hz;
  while (cnt > 65536) {
    pcs++;
    hz *= 2;
    cnt = 150000000 / hz;
  }
  TMRx->CH[1].CTRL = 0; // stop
  TMRx->CH[1].LOAD = 0;  // start val after compare
  TMRx->CH[1].COMP1 = cnt - 1;  // count up to this val, interrupt,  and start again
  TMRx->CH[1].CMPLD1 = cnt - 1;
  TMRx->CH[1].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + pcs) | TMR_CTRL_LENGTH ;  // prescale
  attachInterruptVector(IRQ_QTIMERx, my_isr);
  TMRx->CH[1].CSCTRL &= ~(TMR_CSCTRL_TCF1);  // clear
  TMRx->CH[1].CSCTRL |= TMR_CSCTRL_TCF1EN;  // enable interrupt
  NVIC_ENABLE_IRQ(IRQ_QTIMERx);
}

void cascade() {
  // CH0 tick every 1 ms, clock 150mhz/128  1171875 hz, comp 1172 ish
  // CH1 clocked from CH0 ticks, interrupt every 5000 ms
  int cnt;

  cnt = 1172; // 1 ms
  TMRx->CH[0].CTRL = 0; // stop
  TMRx->CH[0].LOAD = 0;  // start val after compare
  TMRx->CH[0].COMP1 = cnt - 1;  // count up to this val and start again
  TMRx->CH[0].CMPLD1 = cnt - 1;
  TMRx->CH[0].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 7) | TMR_CTRL_LENGTH ;  // /128
  cnt = 5000 ; //  5000 ms
  TMRx->CH[1].CTRL = 0; // stop
  TMRx->CH[1].LOAD = 0;  // start val after compare
  TMRx->CH[1].COMP1 = cnt - 1;  // count up to this val, interrupt,  and start again
  TMRx->CH[1].CMPLD1 = cnt - 1;
  TMRx->CH[1].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(4) | TMR_CTRL_LENGTH ;  //clock from clock 0
  attachInterruptVector(IRQ_QTIMERx, my_isr);
  TMRx->CH[1].CSCTRL &= ~(TMR_CSCTRL_TCF1);  // clear
  TMRx->CH[1].CSCTRL |= TMR_CSCTRL_TCF1EN;  // enable interrupt
  NVIC_ENABLE_IRQ(IRQ_QTIMERx);
}

void chain() {
  // CH0 tick every 1 us, clock 150mhz/150
  // CH1 clocked from CH0 ticks, count 1000 micros
  // CH2 clocked from CH1 ticks, count ms  rollover 65000

  int cnt;

  cnt = 150 ; // 1 us
  TMRx->CH[0].CTRL = 0; // stop
  TMRx->CH[0].CNTR = 0;
  TMRx->CH[0].LOAD = 0;  // start val after compare
  TMRx->CH[0].COMP1 = cnt - 1;  // count up to this val and start again
  TMRx->CH[0].CMPLD1 = cnt - 1  ;
  TMRx->CH[0].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 ) | TMR_CTRL_LENGTH ;  // no prescale
  cnt = 65536;
  TMRx->CH[1].CTRL = 0; // stop
  TMRx->CH[1].LOAD = 0;  // start val after compare
  TMRx->CH[1].COMP1 = cnt - 1;
  TMRx->CH[1].CMPLD1 = cnt - 1;
  TMRx->CH[1].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(4) | TMR_CTRL_LENGTH ;  //clock from clock 0
  cnt = 0; //  count ms
  TMRx->CH[2].CTRL = 0; // stop
  TMRx->CH[2].LOAD = 0;  // start val after compare
  TMRx->CH[2].COMP1 = cnt  ;
  TMRx->CH[2].CMPLD1 = cnt ;
  TMRx->CH[2].CTRL = TMR_CTRL_CM(7) | TMR_CTRL_PCS(5)  ;  //clock from clock 1
}

void setup()   {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  CCM_CCGR6 |= CCM_CCGR6_QTIMER1(CCM_CCGR_ON);

  // pick a test
  //  cascade();
  chain();
  // isr_init(20000);   // hz

  PRREG(TMRx->CH[0].CTRL);
  PRREG(TMRx->CH[0].COMP1);

  PRREG(TMRx->CH[1].CTRL);
  PRREG(TMRx->CH[1].SCTRL);
  PRREG(TMRx->CH[1].CSCTRL);
  PRREG(TMRx->CH[1].LOAD);
  PRREG(TMRx->CH[1].COMP1);
  PRREG(TMRx->CH[1].CMPLD1);
  PRREG(TMRx->CH[1].COMP2);
  PRREG(TMRx->CH[1].CMPLD2);
  // chain test
  uint32_t us = TMRx->CH[1].CNTR + 65536 * TMRx->CH[2].HOLD;
  delay(10);
  uint32_t us1 = TMRx->CH[1].CNTR + 65536 * TMRx->CH[2].HOLD;
  Serial.println(us1 - us);
  us = micros();
  delay(10);
  us1 = micros();
  Serial.println(us1 - us);
}

uint32_t prev = micros();
void loop()
{
  if (micros() - prev > 1000000) {
    uint32_t us = TMRx->CH[1].CNTR + 65536 * TMRx->CH[2].HOLD; // could count second chnl 3
    Serial.printf("%d ticks %d ms   %d us %d\n", ticks, isrms, us, micros());
    prev = micros();
  }
}
