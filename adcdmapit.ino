// from SDK PIT XBAR ADC_ETC ADC   DMA
// chain A0  AD_B1_02     ADC1 IN 7
// TODO not working, DMA trigger ADC no interrupts?
//   trigger ADC_ETC too many interrupts ??
#include <DMAChannel.h>

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define SAMPLES 256

/*DMAMEM*/ static uint16_t rx_buffer[SAMPLES];
DMAChannel dma(false);

volatile uint32_t ticks;
void isr(void)
{
  dma.clearInterrupt();
  ticks++;
  asm volatile ("dsb");
}

void dma_init() {
  // set up a DMA channel to store the ADC data
  dma.begin(true); // Allocate the DMA channel first
  dma.source((uint16_t &) ADC1_R0);
  //  dma.source((uint16_t &) ADC_ETC_TRIG0_RESULT_1_0);  // either works
  dma.destinationBuffer(rx_buffer, sizeof(rx_buffer));

  dma.TCD->CSR = DMA_TCD_CSR_INTHALF | DMA_TCD_CSR_INTMAJOR;  // double buffer
  dma.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC1);
  // dma.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC_ETC);

  dma.attachInterrupt(isr);
  dma.enable();
}

void adc_init() {
  // init and calibrate with help from core
  analogReadResolution(12);
  analogRead(0);
  analogRead(1);
  ADC1_CFG |= ADC_CFG_ADTRG | ADC_GC_DMAEN;   // hardware trigger, DMA?
  // ADC1_CFG = 0x200b;
  ADC1_HC0 = 16;   // ADC_ETC channel
}

void adc_etc_init() {
  ADC_ETC_CTRL &= ~(1 << 31); // SOFTRST
  ADC_ETC_CTRL = 0x40000001;  // start with trigger 0
  ADC_ETC_TRIG0_CTRL = 0;   // chainlength -1
  ADC_ETC_TRIG0_CHAIN_1_0 = 0x1017;   // ADC1 7  chain channel, HWTS,  BB? TODO
  ADC_ETC_DMA_CTRL = 1; // ch 0 enable dma
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
  dma_init();
  adc_etc_init();
  pit_init(24 * 1000);   // 1 khz  1000 ADCs/sec

  PRREG(ADC1_CFG);
  PRREG(ADC1_HC0);
  PRREG(ADC_ETC_CTRL);
  PRREG(ADC_ETC_TRIG0_CTRL);
  PRREG(ADC_ETC_TRIG0_CHAIN_1_0);
}

void loop() {
  static uint32_t prev = 0;
  //arm_dcache_delete(rx_buffer, sizeof(rx_buffer));  // needed for DMAMEM ?
  Serial.printf("%d ticks %d   %d\n", ticks, ticks - prev, rx_buffer[3]);
  prev = ticks;
  delay(2000);
}
