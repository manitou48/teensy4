// T4 ACMP and DAC  with XBAR
// T4 CMP out pins 26 (ACMP3)  or 27 (ACMP4)  or XBAR reroute to pin 2
// ACMPn IN0 input is T4 A4(18)   jumper GND or 3v3 to A4/18
// IN7  DAC 6-bit

#define CMP_MUXCR_PSEL(n)       (uint8_t)(((n) & 0x07) << 3) // Plus Input Mux Control
#define CMP_MUXCR_MSEL(n)       (uint8_t)(((n) & 0x07) << 0) // Minus Input Mux Control
#define DACCR_ENABLE (1<<7)
#define DACCR_VIN2 (1<<6)        //VIN1 works too
#define DACCR_VOSEL(n) (uint8_t)((n) & 0x3f)   // 6-bit
#define CMP_CR1_ENABLE (1<<0)
#define CMP_CR1_OPE (1<<1)
#define CMP_SCR_COUT (1<<0)
#define CMP_CR0_FILTER_CNT(n)   (uint8_t)(((n) & 0x07) << 4)
#define CMP_CR0_HYSTCTR(n)      (uint8_t)(((n) & 0x03) << 0)

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
  xbar_connect(XBARA1_IN_ACMP3_OUT, XBARA1_OUT_IOMUX_XBAR_INOUT06); // ACMP3 out to pin 2
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  xbar_init();
  IOMUXC_GPR_GPR6 |= IOMUXC_GPR_GPR6_IOMUXC_XBAR_DIR_SEL_6; // direction OUT
  //IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 = 3;      //ALT3 connects T4 Pin 2 to XBAR1_INOUT6
  CORE_PIN2_CONFIG = 3;      //ALT3 connects T4 Pin 2 to XBAR1_INOUT6
  // pinMode(2, OUTPUT);    // not needed

  CCM_CCGR3 |= CCM_CCGR3_ACMP3(CCM_CCGR_ON);  // ACMP on
  CMP3_MUXCR = CMP_MUXCR_PSEL(0) | CMP_MUXCR_MSEL(7);  //INO and DAC
  CMP3_CR0 = CMP_CR0_FILTER_CNT(7) | CMP_CR0_HYSTCTR(3);
  CMP3_CR1 = CMP_CR1_ENABLE | CMP_CR1_OPE ;   // enable
  CMP3_DACCR = DACCR_ENABLE | DACCR_VIN2 | DACCR_VOSEL(32);  //3v3/2  VIN1 ok too
  //   output ACMP result HIGH or LOW
}

void loop() {
  Serial.println(CMP3_SCR & CMP_SCR_COUT);   // COUT low bit
  delay(2000);
}
