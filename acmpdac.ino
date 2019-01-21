// T4 ACMP and DAC  polling  (could do interrupt)
//T4 DAC out pins 26 (ACMP3)  or 27 (ACMP4)
// ACMPn 0 is T4 A4   jumper GND or 3v3 to A4
// DAC 6-bit

#define CMP_MUXCR_PSEL(n)       (uint8_t)(((n) & 0x07) << 3) // Plus Input Mux Control
#define CMP_MUXCR_MSEL(n)       (uint8_t)(((n) & 0x07) << 0) // Minus Input Mux Control
#define DACCR_ENABLE (1<<7)
#define CMP_CR1_ENABLE (1<<0)
#define CMP_SCR_COUT (1<<0)

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  CCM_CCGR3 |= CCM_CCGR3_ACMP3(CCM_CCGR_ON);  // ACMP on
  CMP3_MUXCR = CMP_MUXCR_PSEL(0) | CMP_MUXCR_MSEL(7);
  CMP3_CR1 = CMP_CR1_ENABLE ;   // enable
  CMP3_DACCR = DACCR_ENABLE  | 32 | 0x40;  // 3v3/2 and enable
  // ? do i need to configure DAC pin to see output?  output ACMP result HIGH or LOW
  pinMode(26, OUTPUT);
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_14 = 1;  // ALT 1 ACMP3_OUT

}

void loop() {
  Serial.println(CMP3_SCR & CMP_SCR_COUT);   // COUT low bit
  delay(2000);
}
