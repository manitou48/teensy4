// QTIMER4 timer 0  xbar 36 to EXT_CLK flexpwm via XBAR  34
// pin 8 FLEXPWM1_A3   XBAR1_OUT34

#define MMASK (1<<3)   // module mask 3 for PWM1_A3

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

void flexpwm_init(int scale) {
  // flexpwm pin 8  external clock

  FLEXPWM1_OUTEN |= FLEXPWM_OUTEN_PWMA_EN(MMASK);  //A3
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 = 6 ;  // ALT6
  FLEXPWM1_FCTRL0 |= FLEXPWM_FCTRL0_FLVL(MMASK);   // clear
  FLEXPWM1_FSTS0 = FLEXPWM_FSTS0_FFLAG(MMASK);    // clear
  FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(MMASK);
  FLEXPWM1_SM3CTRL2 = FLEXPWM_SMCTRL2_INDEP | FLEXPWM_SMCTRL2_CLK_SEL(1); // ext clk
  FLEXPWM1_SM3CTRL = FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_PRSC(0);
  FLEXPWM1_SM3INIT = 0;
  FLEXPWM1_SM3VAL0 = 0;
  FLEXPWM1_SM3VAL1 = scale - 1;  // period ticks
  FLEXPWM1_SM3VAL2 = 0;
  FLEXPWM1_SM3VAL3 = scale / 2;
  FLEXPWM1_SM3VAL4 = 0;
  FLEXPWM1_SM3VAL5 = 0;
  FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(MMASK) | FLEXPWM_MCTRL_RUN(MMASK);
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
  xbar_connect(XBARA1_IN_QTIMER4_TIMER0, XBARA1_OUT_FLEXPWM1_EXT_CLK);   // 36,34
}

void qtmr40_init(int hz) {
  int cnt, pcs = 0;
  cnt = 150000000 / hz;
  while (cnt > 65536) {
    pcs++;
    hz *= 2;
    cnt = 150000000 / hz;
  }
  CCM_CCGR6 |= CCM_CCGR6_QTIMER4(CCM_CCGR_ON);  // enable qtmr4
  TMR4_CTRL0 = 0; // stop
  TMR4_SCTRL0 =  TMR_SCTRL_OPS | TMR_SCTRL_OEN ;
  TMR4_LOAD0 = 0;  // start val after compare
  TMR4_COMP10 = cnt - 1;  // count up to this val, interrupt,  and start again
  TMR4_CMPLD10 = cnt - 1;
  TMR4_CTRL0 = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + pcs) | TMR_CTRL_LENGTH | TMR_CTRL_OUTMODE(4) ; // prescale
}


void setup() {
  Serial.begin(9600);

  delay(1000);
  xbar_init();
  flexpwm_init(10);   //divisor
  qtmr40_init(10000);  // 10 khz
}

void loop() {

}
