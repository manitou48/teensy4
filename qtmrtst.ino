// QTIMER1  test  qtmr 1 ch 0  pin 10 B0_00, F_BUS_ACTUAL 150 mhz, 50%duty  ch 52
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

volatile uint32_t ticks;
void ticker() {
  ticks++;
}

void my_isr() {
  TMR1_CSCTRL0 &= ~(TMR_CSCTRL_TCF1);  // clear
  ticks++;
  //  while (TMR1_CSCTRL0 & TMR_CSCTRL_TCF1); // wait for clear
  asm volatile ("dsb");  // wait for clear  memory barrier
}

void oflow_isr() {
  TMR1_SCTRL0 &= ~(TMR_SCTRL_TOF);  // clear
  ticks++;
  //  while (TMR1_CSCTRL0 & TMR_CSCTRL_TOF); // wait for clear
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
  TMR1_CTRL0 = 0; // stop
  TMR1_LOAD0 = 0;  // start val after compare
  TMR1_COMP10 = cnt - 1;  // count up to this val, interrupt,  and start again
  TMR1_CMPLD10 = cnt - 1;
  TMR1_CTRL0 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + pcs) | TMR_CTRL_LENGTH ;  // prescale
  attachInterruptVector(IRQ_QTIMER1, my_isr);
  TMR1_CSCTRL0 &= ~(TMR_CSCTRL_TCF1);  // clear
  TMR1_CSCTRL0 |= TMR_CSCTRL_TCF1EN;  // enable interrupt
  NVIC_ENABLE_IRQ(IRQ_QTIMER1);
}

void rollover() {
  TMR1_CTRL0 = 0; // stop
  TMR1_LOAD0 = 0;  // start val after compare
  TMR1_COMP10 = 0xffff;  // count up to this val, interrupt,  and start again
  TMR1_CMPLD10 = 0xffff;
  TMR1_CTRL0 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 2) | TMR_CTRL_LENGTH ;  // prescale
  attachInterruptVector(IRQ_QTIMER1, my_isr);
  TMR1_CSCTRL0 = TMR_CSCTRL_TCF1EN;  // enable interrupt
  NVIC_ENABLE_IRQ(IRQ_QTIMER1);
}


void oflows() {
  TMR1_CTRL0 = 0; // stop
  TMR1_LOAD0 = 0;  // start val after compare
  TMR1_CSCTRL0 = 0;
  // TMR1_COMP10 = 1000-1;  // count up to this val ... not used
  // TMR1_CMPLD10 = 1000-1;
  TMR1_CTRL0 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 2)  ;  // prescale
  attachInterruptVector(IRQ_QTIMER1, oflow_isr);
  TMR1_SCTRL0 = TMR_SCTRL_TOFIE ;  // enable oflow interrupt
  NVIC_ENABLE_IRQ(IRQ_QTIMER1);
}

void pwm4_init() {  // T4 default PWM freq
  uint32_t modulo, high, low;
  TMR1_CTRL0 = 0; // stop
  TMR1_CNTR0 = 0;
  TMR1_SCTRL0 = TMR_SCTRL_OEN | TMR_SCTRL_OPS | TMR_SCTRL_VAL | TMR_SCTRL_FORCE;
  TMR1_CSCTRL0 = TMR_CSCTRL_CL1(1) | TMR_CSCTRL_ALT_LOAD;
  TMR1_LOAD0 = 24000;
  TMR1_COMP10 = 0;
  TMR1_CMPLD10 = 0;
  TMR1_CTRL0 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8) |
               TMR_CTRL_LENGTH | TMR_CTRL_OUTMODE(6);  // prescale 1
  // duty cycle 50%  128
  modulo = 65536 - TMR1_LOAD0 + TMR1_CMPLD10;
  high = (128 * (modulo - 1)) >> 8;
  if (high >= modulo) high = modulo - 1;
  low = modulo - high;
  // counts from 0 to COMP1 then from LOAD0 to FFFF  outmode(6) clear and set
  TMR1_LOAD0 = 65536 - low;    // after comp, where to start counter
  TMR1_CMPLD10 = high;  // goes to COMP1 reg
  Serial.printf("mod %d low %d high %d\n", modulo, low, high);
  // config pin
  *(portConfigRegister(10)) = 1;  // ALT 1
}

void pwm_sdk(int duty, int hz) {  // from sdk fsl_qtmr.c
  uint32_t period, high, low, srcclk = 150000000, pcs = 0;
  period = srcclk / hz;
  while (period > 65536) {
    pcs++;
    srcclk /= 2;
    period = srcclk / hz;
  }
#if 0
  pcs = 3; // sdk at 50khz
  period = 150000000 / 8 / hz;
#endif
  high = (period * duty) / 100;
  low = period - high;
  Serial.printf("period %d low %d high %d pcs %d\n", period, low, high, pcs);
  TMR1_CTRL0 = 0; // stop
  TMR1_CNTR0 = 0;
  TMR1_LOAD0 = 0;

  TMR1_COMP10 = low;
  TMR1_CMPLD10 = low;
  TMR1_COMP20 = high;
  TMR1_CMPLD20 = high;
  TMR1_SCTRL0 =  TMR_SCTRL_OPS | TMR_SCTRL_OEN ;//| TMR_SCTRL_VAL | TMR_SCTRL_FORCE;
  TMR1_CSCTRL0 = TMR_CSCTRL_CL1(2) | TMR_CSCTRL_CL2(1);
  TMR1_CTRL0 =  TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + pcs) | TMR_CTRL_LENGTH |
                TMR_CTRL_OUTMODE(4);
  *(portConfigRegister(10)) = 1;  // ALT 1
}

void setup()   {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
#if 0
  // analogWriteFrequency(10, 10000);
  analogWrite(10, 128);
  PRREG(TMR1_SCTRL0);
  PRREG(TMR1_CSCTRL0);
  PRREG(TMR1_CTRL0);
  PRREG(TMR1_LOAD0);
  PRREG(TMR1_CMPLD10);
#endif
  attachInterrupt(12, ticker, RISING);  // jumper 10 to 12
  CCM_CCGR6 |= CCM_CCGR6_QTIMER1(CCM_CCGR_ON);

  // pick a test
  //  pwm4_init();  // T4 default pin 10 jumper 10 to 12
  //isr_init(20000);   // hz
  // pwm_sdk(50, 1000);  // duty, hz    pin 10
  // oflows();   // ? TOF not working ??
  rollover();

  PRREG(TMR1_SCTRL0);
  PRREG(TMR1_CSCTRL0);
  PRREG(TMR1_CTRL0);
  PRREG(TMR1_LOAD0);
  PRREG(TMR1_COMP10);
  PRREG(TMR1_CMPLD10);
  PRREG(TMR1_COMP20);
  PRREG(TMR1_CMPLD20);
}

void loop()
{
  Serial.println(ticks);
  PRREG(TMR1_CNTR0);
  delay(1000);
}
