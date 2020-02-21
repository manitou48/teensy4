// sysinfo for teensy 4
#include <time.h>
#define PRREG(x) Serial.printf(#x" 0x%x\n",x);

// needs to be updated for variable size ITCM
void flexRamInfo(void)
{
#if defined(__IMXRT1052__) || defined(__IMXRT1062__)
  int itcm = 0;
  int dtcm = 0;
  int ocram = 0;
  Serial.print("FlexRAM-Banks: ");
  for (int i = 15; i >= 0; i--) {
    switch ((IOMUXC_GPR_GPR17 >> (i * 2)) & 0b11) {
      case 0b00: Serial.print("."); break;
      case 0b01: Serial.print("O"); ocram++; break;
      case 0b10: Serial.print("D"); dtcm++; break;
      case 0b11: Serial.print("I"); itcm++; break;
    }
  }
  Serial.print(": ITCM: ");
  Serial.print(itcm * 32);
  Serial.print(" KB, DTCM: ");
  Serial.print(dtcm * 32);
  Serial.print("KB, OCRAM: ");
  Serial.print(ocram * 32);
#if defined(__IMXRT1062__)
  Serial.print("(+512)");
#endif
  Serial.println(" KB");
  extern unsigned long _stext;
  extern unsigned long _etext;
  extern unsigned long _sdata;
  extern unsigned long _ebss;
  extern unsigned long _flashimagelen;
  extern unsigned long _heap_start;

  Serial.print("MEM (static usage): ITCM:");
  Serial.print((unsigned)&_etext - (unsigned)&_stext);
  Serial.print(", DTCM:");
  Serial.print((unsigned)&_ebss - (unsigned)&_sdata);
  Serial.print(", OCRAM:");
  Serial.print((unsigned)&_heap_start - 0x20200000);
  Serial.print(", Flash:");
  Serial.println((unsigned)&_flashimagelen);
  Serial.println();
#endif
}
void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  Serial.println();
  Serial.print(F_CPU); Serial.print(" ");
  Serial.print(F_CPU_ACTUAL); Serial.print(" ");
  Serial.print(F_BUS_ACTUAL); Serial.print(" ");
  Serial.println(__TIME__ " " __DATE__);
#if defined(__IMXRT1062__)
  time_t tt = rtc_get();
  Serial.printf("RTC: %s", ctime(&tt));
  Serial.print("ARDUINO "); Serial.print( ARDUINO);
  Serial.print("   TEENSYDUINO "); Serial.println(TEENSYDUINO);
  Serial.println("GCC " __VERSION__);
  float tmpc = tempmonGetTemp();
  Serial.printf("MAC %04X%08X  %.1f C\n", HW_OCOTP_MAC1, HW_OCOTP_MAC0, tmpc);
#endif
  PRREG(CCM_ANALOG_PLL_ARM);
  PRREG(CCM_ANALOG_PLL_USB1);
  PRREG(CCM_ANALOG_PLL_USB2);
  PRREG(CCM_ANALOG_PLL_SYS);
  PRREG(CCM_ANALOG_PFD_480);
  PRREG(CCM_ANALOG_PFD_528);
  PRREG(CCM_ANALOG_MISC0);
  PRREG(CCM_CBCDR);
  PRREG(CCM_CBCMR);
  PRREG(CCM_CDCDR);
  PRREG(CCM_CCSR);
  PRREG(CCM_CACRR);
  PRREG(CCM_CCGR1);
  PRREG(CCM_CCGR2);
  PRREG(CCM_CCGR3);
  PRREG(CCM_CCGR4);
  PRREG(CCM_CCGR5);
  PRREG(CCM_CCGR6);
  PRREG(CCM_CSCMR1);
  PRREG(CCM_CSCMR2);
  PRREG(CCM_CSCDR1);
  PRREG(SCB_CPACR);
  PRREG(SCB_CCR);
  PRREG(SCB_ID_CCSIDR);
  PRREG(SCB_ID_CSSELR);
  PRREG(SCB_CPUID);
  PRREG(SYST_CSR);
  PRREG(SYST_RVR);
  PRREG(IOMUXC_GPR_GPR14);
  PRREG(IOMUXC_GPR_GPR16);
  PRREG(IOMUXC_GPR_GPR17);
  PRREG(IOMUXC_GPR_GPR26);
  flexRamInfo();
}

void loop() {
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 = 5; // pin 13  alt
  IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 = IOMUXC_PAD_DSE(7);
  if (IOMUXC_GPR_GPR27 & (1 << 3)) { // fast GPIO on
    GPIO7_GDIR |= (1 << 3);  //output
    GPIO7_DR_SET = (1 << 3);  // high
    while (1) {
      volatile uint32_t n;
      GPIO7_DR_TOGGLE = (1 << 3); //toggle vs SET CLEAR
      for (n = 0; n < 2000000; n++) ;
    }
  } else {
    GPIO2_GDIR |= (1 << 3);  //output
    GPIO2_DR_SET = (1 << 3);  // high
    while (1) {
      volatile uint32_t n;
      GPIO2_DR_TOGGLE = (1 << 3); //toggle vs SET CLEAR
      for (n = 0; n < 2000000; n++) ;
    }
  }
}
