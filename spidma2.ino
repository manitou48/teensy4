//  T4 SPI DMA  transmit   v2 single shot tx rx

#include <DMAChannel.h>
#include <SPI.h>
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define CS 10
#define SPICLOCK 4000000
#define SAMPLES 1024

/*DMAMEM*/ static uint8_t tx_buffer[SAMPLES];
/*DMAMEM*/ static uint8_t rx_buffer[SAMPLES];
DMAChannel tx(false), rx(false);


void spidma()
{
  LPSPI4_CR &= ~LPSPI_CR_MEN;//disable LPSPI:
  LPSPI4_CFGR1 |= LPSPI_CFGR1_NOSTALL; //prevent stall from RX
  //LPSPI4_TCR = 15; // Framesize 16 Bits
  LPSPI4_FCR = 0; // Fifo Watermark
  LPSPI4_DER = LPSPI_DER_TDDE | LPSPI_DER_RDDE; //TX RX DMA Request Enable
  LPSPI4_CR |= LPSPI_CR_MEN; //enable LPSPI:

  // set up a DMA channel to send the SPI data
  tx.begin(true); // Allocate the DMA channel first
  tx.destination((uint8_t &)  LPSPI4_TDR);
  tx.sourceBuffer(tx_buffer, sizeof(tx_buffer));
  tx.disableOnCompletion();
  tx.triggerAtHardwareEvent( DMAMUX_SOURCE_LPSPI4_TX );

  rx.begin(true); // Allocate the DMA channel first
  rx.source((uint8_t &)  LPSPI4_RDR);
  rx.destinationBuffer(rx_buffer, sizeof(rx_buffer));
  rx.disableOnCompletion();
  rx.triggerAtHardwareEvent( DMAMUX_SOURCE_LPSPI4_RX );  // hangs


  digitalWrite(CS, LOW);
  uint32_t t = micros();
  rx.enable();
  tx.enable();
  while (!rx.complete()) ;
  t = micros() - t;
  digitalWrite(CS, HIGH);
  tx.clearComplete();
  rx.clearComplete();
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

  for (int i = 0; i < SAMPLES; i++) {
    tx_buffer[i] = i;
    rx_buffer[i] = 0;
  }
  spidma();
  int errs = 0;
  for (int i = 0; i < SAMPLES; i++) if (tx_buffer[i] != rx_buffer[i]) errs++;
  Serial.printf("errs %d  [3] %d\n", errs, rx_buffer[3]);

}

void loop()
{

}
