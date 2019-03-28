//  https://forum.pjrc.com/threads/24963-Teensy-3-1-ADC-with-DMA
// T4 A0 ADC1 7   A1 ch 8
#include <DMAChannel.h>
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define SAMPLES 1024

DMAMEM static uint16_t rx_buffer[SAMPLES];
DMAChannel dma(false);
uint16_t dc_average;

void setupADC(int pin)
{
  uint32_t i, sum = 0;

  // pin must be 0 to 13 (for A0 to A13)
  // or 14 to 23 for digital pin numbers A0-A9
  // or 34 to 37 corresponding to A10-A13
  if (pin > 23 && !(pin >= 34 && pin <= 37)) return;

  // Configure the ADC and run at least one software-triggered
  // conversion.  This completes the self calibration stuff and
  // leaves the ADC in a state that's mostly ready to use
  analogReadRes(10);
  analogReadAveraging(1);
  // Actually, do many normal reads, to start with a nice DC level
  uint32_t us = micros();
  for (i = 0; i < 1024; i++) {
    sum += analogRead(pin);
  }
  us = micros() - us;
  float t = us / 1024.;
  Serial.print(t); Serial.println(" us");
  dc_average = sum >> 10;
  Serial.println(dc_average);

  // enable the ADC for DMA
  ADC1_GC |= ADC_GC_DMAEN | ADC_GC_ADCO;
#if 0
  ADC1_CFG = ADC_CFG_AVGS(0) | ADC_CFG_REFSEL(0) | ADC_CFG_ADHSC |
             ADC_CFG_ADSTS(0) |  ADC_CFG_ADIV(0) | //ADC_CFG_ADLPC | ADC_CFG_ADLSMP |
             ADC_CFG_MODE(1) | ADC_CFG_ADICLK(3);  // hi speed 10-bit
#endif

  PRREG(ADC1_GC);
  PRREG(ADC1_HC0);
  PRREG(ADC1_CFG);
  Serial.printf("buff addr %x\n", (uint32_t)rx_buffer);


  // set up a DMA channel to store the ADC data
  dma.begin(true); // Allocate the DMA channel first
  dma.source((uint16_t &) ADC1_R0);
  dma.destinationBuffer(rx_buffer, sizeof(rx_buffer));

  dma.TCD->CSR = DMA_TCD_CSR_INTHALF | DMA_TCD_CSR_INTMAJOR;
  dma.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC1);

  dma.attachInterrupt(isr);
  dma.enable();

  ADC1_GC |= ADC_GC_ADACKEN;
  ADC1_HC0 = 7;   // need pin_to_channel table analog.c
}

volatile uint32_t ticks;
void isr(void)
{
  dma.clearInterrupt();
  ticks++;
  asm volatile ("dsb");
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  setupADC(14);  //A0
}

void loop()
{
  static int prev;
  arm_dcache_delete(rx_buffer, sizeof(rx_buffer));  // needed for DMAMEM ?
  Serial.printf("%d ticks A0 = %d \n", ticks - prev, rx_buffer[13]);
  prev = ticks;
  delay(2000);
}
