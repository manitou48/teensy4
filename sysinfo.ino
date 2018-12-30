// sysinfo for teensy 4
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  Serial.println();
  Serial.print(F_CPU); Serial.print(" ");
//  Serial.print(F_BUS); Serial.print(" ");
  Serial.println(__TIME__ " " __DATE__);
  PRREG(CCM_ANALOG_PLL_ARM);
  PRREG(CCM_ANALOG_PLL_USB1);
  PRREG(CCM_ANALOG_PLL_USB2);
  PRREG(CCM_ANALOG_PLL_SYS);
  PRREG(CCM_ANALOG_PFD_480);
  PRREG(CCM_ANALOG_PFD_528);
  PRREG(CCM_CBCDR);
  PRREG(CCM_CBCMR);
  PRREG(CCM_CCGR1);
  PRREG(CCM_CCGR2);
  PRREG(CCM_CCGR3);
  PRREG(CCM_CCGR4);
  PRREG(CCM_CCGR5);
  PRREG(CCM_CCGR6);
  PRREG(CCM_CSCMR1);
  PRREG(CCM_CSCDR1);
  PRREG(SCB_CPACR);
  PRREG(SYST_CSR);
  PRREG(SYST_RVR);
}

void loop() {
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5; // pin 13
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
  GPIO2_GDIR |= (1 << 3);
  GPIO2_DR_SET = (1 << 3);
  while (1) {
    volatile uint32_t n;
    GPIO2_DR_TOGGLE = (1 << 3); //toggle vs SET CLEAR
    for (n = 0; n < 2000000; n++) ;
  }
}
