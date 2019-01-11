//  T4 SPI DMA  transmit

#include <DMAChannel.h>
#include <SPI.h>
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define CS 10
#define SPICLOCK 4000000
#define SAMPLES 1024

/*DMAMEM*/ static uint8_t tx_buffer[SAMPLES];
DMAChannel dma(false);

volatile uint32_t ticks;
void isr(void)
{
  dma.clearInterrupt();
  ticks++;
}

void spidma()
{
  LPSPI4_CR &= ~LPSPI_CR_MEN;//disable LPSPI:
  LPSPI4_CFGR1 |= LPSPI_CFGR1_NOSTALL; //prevent stall from RX
  //LPSPI4_TCR = 15; // Framesize 16 Bits
  LPSPI4_FCR = 0; // Fifo Watermark
  LPSPI4_DER = LPSPI_DER_TDDE; //TX DMA Request Enable
  LPSPI4_CR |= LPSPI_CR_MEN; //enable LPSPI:

  // set up a DMA channel to send the SPI data
  dma.begin(true); // Allocate the DMA channel first
  dma.destination((uint8_t &)  LPSPI4_TDR);
  dma.sourceBuffer(tx_buffer, sizeof(tx_buffer));

  dma.TCD->CSR = DMA_TCD_CSR_INTMAJOR;
  dma.enable();
  dma.attachInterrupt(isr);
  digitalWrite(CS, LOW);
  uint32_t t = micros();
  dma.triggerAtHardwareEvent( DMAMUX_SOURCE_LPSPI4_TX );  // start
  while (ticks == 0) ;
  t = micros() - t;
  digitalWrite(CS, HIGH);
  Serial.printf("tx %d samples %d us  %.1f mbs\n", SAMPLES, t, 8.*SAMPLES / t);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  Serial.println("SPI DMA");
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));

  //LPSPI4_CCR = 1;   // DIV  + 2
  // LPSPI4_TCR = 31;   // frame
  PRREG(LPSPI4_CCR);
  PRREG(LPSPI4_TCR);
  PRREG(LPSPI4_FCR);
  Serial.printf("SPI CLOCK %d CCR freq %.1f MHz\n", SPICLOCK, 528. / 7 / ((0xff & LPSPI4_CCR) + 2));

  spidma();
}

void loop()
{

}
