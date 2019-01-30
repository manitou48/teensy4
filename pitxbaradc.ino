// from SDK PIT XBAR ADC_ETC ADC
// chain A0 A1   AD_B1_02  AD_B1_03   ADC1 IN7 8
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
volatile uint32_t val0, val1;

void adcetc0_isr() {
  ADC_ETC_DONE0_1_IRQ |= 1;   // clear
  val0 = ADC_ETC_TRIG0_RESULT_1_0 & 4095;
  asm("dsb");
}

void adcetc1_isr() {
  ADC_ETC_DONE0_1_IRQ |= 1 << 16;   // clear
  val1 = (ADC_ETC_TRIG0_RESULT_1_0 >> 16) & 4095;
  asm("dsb");
}

void adc_init() {
  // init and calibrate with help from core
  analogReadResolution(12);
  analogRead(0);
  analogRead(1);
  ADC1_CFG |= ADC_CFG_ADTRG;   // hardware trigger
  // ADC1_CFG = 0x200b;
  ADC1_HC0 = 16;   // ADC_ETC channel
  ADC1_HC1 = 16;
}

void adc_etc_init() {
  ADC_ETC_CTRL &= ~(1 << 31); // SOFTRST
  ADC_ETC_CTRL = 0x40000001;  // start with trigger 0
  ADC_ETC_TRIG0_CTRL = 0x100;   // chainlength -1
  ADC_ETC_TRIG0_CHAIN_1_0 = 0x50283017;   // ADC1 7 8, chain channel, HWTS, IE, B2B
  attachInterruptVector(IRQ_ADC_ETC0, adcetc0_isr);
  NVIC_ENABLE_IRQ(IRQ_ADC_ETC0);
  attachInterruptVector(IRQ_ADC_ETC1, adcetc1_isr);
  NVIC_ENABLE_IRQ(IRQ_ADC_ETC1);
}

void xbar_connect(unsigned int input, unsigned int output)
{
  if (input >= 88) return;
  if (output >= 132) return;
  volatile uint16_t *xbar = &XBARA1_SEL0 + (output / 2);
  uint16_t val = *xbar;
  if (!(output & 1)) {
    val = (val & 0xFF00) | input;
  } else {
    val = (val & 0x00FF) | (input << 8);
  }
  *xbar = val;
}

void xbar_init() {
  CCM_CCGR2 |= CCM_CCGR2_XBAR1(CCM_CCGR_ON);   //turn clock on for xbara1
  xbar_connect(56, 103);   // pit to adc_etc
}

void pit_init(uint32_t cycles)
{
  CCM_CCGR1 |= CCM_CCGR1_PIT(CCM_CCGR_ON);
  PIT_MCR = 0;

  IMXRT_PIT_CHANNELS[0].LDVAL = cycles;
  IMXRT_PIT_CHANNELS[0].TCTRL = PIT_TCTRL_TEN;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  xbar_init();
  adc_init();
  adc_etc_init();
  pit_init(24 * 1000000);

  PRREG(ADC1_CFG);
  PRREG(ADC1_HC0);
  PRREG(ADC1_HC1);
  PRREG(ADC_ETC_CTRL);
  PRREG(ADC_ETC_TRIG0_CTRL);
  PRREG(ADC_ETC_TRIG0_CHAIN_1_0);
}

void loop() {
  Serial.printf("%d  %d\n", val0, val1);
  delay(2000);
}
