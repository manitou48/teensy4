// t4 eflexpwm timer   ppmout   TODO
// flexpwm 1 3 PWM1_A3    pin 8 B1_00 on 1062
//   clock at 150mhz/4
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define CLOCKS_PER_MICROSECOND (150./4)  // pcs 8+2
#define TX_PULSE_WIDTH      100.0
#define TX_DEFAULT_SIGNAL  1500.0
#define TX_MINIMUM_SPACE   5000.0
#define TX_MINIMUM_FRAME  20000.0
#define TX_MINIMUM_SIGNAL   300.0
#define TX_MAXIMUM_SIGNAL  2500.0
#define TX_MINIMUM_SPACE_CLOCKS   (uint32_t)(TX_MINIMUM_SPACE * CLOCKS_PER_MICROSECOND)
#define TX_MINIMUM_FRAME_CLOCKS   (uint32_t)(TX_MINIMUM_FRAME * CLOCKS_PER_MICROSECOND)
#define TX_PULSE_WIDTH_CLOCKS     (uint32_t)(TX_PULSE_WIDTH * CLOCKS_PER_MICROSECOND)

volatile uint32_t ticks;
void pwm1_3_isr() {
  static int count = 0;
  FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF;
  ticks++;
  if (ticks > 10000) {
    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(8);
    FLEXPWM1_SM3VAL1 = 4 * TX_PULSE_WIDTH_CLOCKS ;
   // if (count)FLEXPWM1_SM3VAL3 = 0;
   // else FLEXPWM1_SM3VAL3 = TX_PULSE_WIDTH_CLOCKS;
    count = 1-count;
    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(8);
  }
  asm volatile ("dsb");  // ? needed
}


void pwm_init() {
  uint32_t period = 3 * TX_PULSE_WIDTH_CLOCKS;
  uint32_t prescale = 2;   // 150/4

  FLEXPWM1_OUTEN |= FLEXPWM_OUTEN_PWMA_EN(8);  //A3
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 = 6 ;  // ALT6
  FLEXPWM1_FCTRL0 |= FLEXPWM_FCTRL0_FLVL(8);
  FLEXPWM1_FSTS0 = 0x0008;
  FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(8);
  FLEXPWM1_SM3CTRL2 = FLEXPWM_SMCTRL2_INDEP;
  FLEXPWM1_SM3CTRL = FLEXPWM_SMCTRL_HALF | FLEXPWM_SMCTRL_PRSC(prescale);
  FLEXPWM1_SM3INIT = 0;
  FLEXPWM1_SM3VAL0 = 0;
  FLEXPWM1_SM3VAL1 = period;
  FLEXPWM1_SM3VAL2 = 0;
  FLEXPWM1_SM3VAL3 = TX_PULSE_WIDTH_CLOCKS;
  FLEXPWM1_SM3VAL4 = 0;
  FLEXPWM1_SM3VAL5 = 0;
  attachInterruptVector(IRQ_FLEXPWM1_3, pwm1_3_isr);
  FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF;
  FLEXPWM1_SM3INTEN = FLEXPWM_SMINTEN_RIE;
  NVIC_ENABLE_IRQ(IRQ_FLEXPWM1_3);
  FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(8) | FLEXPWM_MCTRL_RUN(8);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);

  pwm_init();
}

void loop() {
  static int prev = 0;
  Serial.printf("%d ticks %d\n", ticks - prev, ticks);
  PRREG(FLEXPWM1_SM3VAL1);
  PRREG(FLEXPWM1_SM3VAL3);
  prev = ticks;
  delay(1000);
}
