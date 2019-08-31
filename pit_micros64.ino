// PIT lifetime timer free running  24mhz clock  64-bit cycle count
// chain chn 1 to chn 0
//  teensy4 uses PIT for IntervalTimer

#define PRREG(x) Serial.printf(#x" 0x%x\n",x);

void pit_init()
{
  CCM_CCGR1 |= CCM_CCGR1_PIT(CCM_CCGR_ON);

  PIT_MCR = 0;

  IMXRT_PIT_CHANNELS[1].LDVAL = 0xffffffff;   // max
  IMXRT_PIT_CHANNELS[1].TFLG = 1;        // clear
  IMXRT_PIT_CHANNELS[1].TCTRL = PIT_TCTRL_TEN | PIT_TCTRL_CHN; // chain to 0

  IMXRT_PIT_CHANNELS[0].LDVAL = 0xffffffff;
  IMXRT_PIT_CHANNELS[0].TFLG = 1;
  IMXRT_PIT_CHANNELS[0].TCTRL = PIT_TCTRL_TEN;
}

uint64_t pit_cycles() {
  uint64_t valueH;
  volatile uint32_t valueL;
  valueH = PIT_LTMR64H;   //  no rollover
  valueL = PIT_LTMR64L;
  // NXP errata workaround
  if (valueL == IMXRT_PIT_CHANNELS[0].LDVAL) {
    valueH = PIT_LTMR64H;
    valueL = PIT_LTMR64L;
  }
  return ~((valueH << 32) | valueL);   // invert for up counting
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  pit_init();
}

void loop()
{
  Serial.printf("%lld us\n", pit_cycles() / 24);
  delay(1000);
}
