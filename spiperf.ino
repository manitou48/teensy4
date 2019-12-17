// t4 spi
//  bus 528/7 mhz     max SPI 528/7/2 37.7 mhz
// 1.49-beta3  max SPI 720/3/2  120mhz
// jumper MISO to MOSI for err check

#include <SPI.h>
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define CS 10
#define SPICLOCK 4000000
#define SPI_BUFF_SIZE 1024
uint8_t rx_buffer[SPI_BUFF_SIZE];
uint8_t tx_buffer[SPI_BUFF_SIZE];

void setup() {
  Serial.begin(9600); while (!Serial);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  Serial.println(); Serial.print(F_CPU_ACTUAL); Serial.print(" ");
  Serial.print(__TIME__); Serial.print(" "); Serial.println(__DATE__);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));

  //LPSPI4_CCR = 1;   // DIV  + 2
  // LPSPI4_TCR = 31;   // frame
  PRREG(LPSPI4_CCR);
  PRREG(LPSPI4_TCR);
  PRREG(LPSPI4_FCR);
  Serial.printf("SPICLOCK %d MHz   CCR freq %.1f MHz\n", SPICLOCK / 1000000, 528. / 7 / ((0xff & LPSPI4_CCR) + 2));
}

void loop() {
  uint32_t t1;
  float mbs;

  for (int i = 0; i < SPI_BUFF_SIZE; i++) tx_buffer[i] = i;
  digitalWrite(CS, LOW);
  t1 = micros();
  SPI.transfer(tx_buffer, SPI_BUFF_SIZE);
  t1 = micros() - t1;
  digitalWrite(CS, HIGH);
  mbs = 8 * SPI_BUFF_SIZE / (float)t1;
  Serial.printf("Tx %d bytes in %d us  %.2f mbs \n", SPI_BUFF_SIZE, t1, mbs);

  // jumper MOSI to MISO
  for (int i = 0; i < SPI_BUFF_SIZE; i++) tx_buffer[i] = i;
  memset(rx_buffer, 0, SPI_BUFF_SIZE);
  digitalWrite(CS, LOW);
  SPI.transfer(tx_buffer, rx_buffer, SPI_BUFF_SIZE);
  digitalWrite(CS, HIGH);
  int errs = 0;
  for (int i = 0; i < SPI_BUFF_SIZE; i++) if (tx_buffer[i] != rx_buffer[i]) errs++;
  Serial.printf("errs %d  [3] %d %d \n", errs, tx_buffer[3], rx_buffer[3]);
  delay(3000);
}
