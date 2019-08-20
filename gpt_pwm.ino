//TESTT4004 - GPT TEST PROGRAM for T4
//===================================
//Author: TelephoneBill refactored by thd
//Date: 19 AUG 2019
//Version: 001
// https://forum.pjrc.com/threads/57276-T4-jitter-free-low-frequency-output-pulse?p=213016&viewfull=1#post213016

//NOTES: Using GPT2 as the timer. GPT2 Output Compare3 (1 KHz) is on pin 16. CompareValue = 0x000124F7 = (75,000 - 1)
//Compare1 is the counter reset mechanism. Compare2 is not used here. Compare3 is the toggle output on pin 16.

#define TICKS  75000

volatile uint32_t ISRTicks = 0;

void setup() {
  //initialise general hardware
  Serial.begin(115200);             //setup serial port
  pinMode(13, OUTPUT);              //pin 13 as digital output
  FlashLED(4);                      //confidence boost

  CCM_CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL; // turn off 24mhz mode
  CCM_CCGR0 |= CCM_CCGR0_GPT2_BUS(CCM_CCGR_ON) ;  // enable GPT2 module

  //configure GPT2 for test
  GPT2_CR = 0;                                //clear the control register, FRR = 0 means restart after Compare
  GPT2_SR = 0x3F;                             //clear all prior status
  GPT2_PR = 0;                                //prescale register set divide by 1
  GPT2_CR |= GPT_CR_CLKSRC(1);                //clock selection #1 (peripheral clock = 150 MHz)
 // GPT2_CR =  GPT_CR_EN_24M | GPT_CR_CLKSRC(5);  // 24mhz clock
  GPT2_CR |= GPT_CR_ENMOD;                    //reset count to zero before enabling
  GPT2_CR |= GPT_CR_OM3(1);           // toggle mode 3
  GPT2_OCR1 = TICKS - 1;                   //Compare1 value
  //GPT2_OCR2 = TICKS - 1;                   //Compare2 value
  GPT2_OCR3 = TICKS - 1;                   //Compare3 value
  GPT2_IR = GPT_IR_OF3IE;                  //enable interrupt for Compare flag
  GPT2_CR |= GPT_CR_EN;                   //enable GPT1 counting at 150 MHz

  //configure Teensy pin 16 as Compare3 output
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_07 = 8;    // GPT2 Compare3 is now output on pin 16

  attachInterruptVector(IRQ_GPT2, GPT2_isr);  //declare which routine performs the ISR function
  NVIC_ENABLE_IRQ(IRQ_GPT2);
}

void GPT2_isr(void) {
  GPT2_SR = GPT_IR_OF3IE;      //reset the interrupt flags
  ISRTicks++;
  if (ISRTicks >= 2000) {
    ISRTicks = 0;
    digitalWriteFast(13, 1);
  }
  if (ISRTicks == 100) {
    digitalWriteFast(13, 0);
  }
  asm volatile("dsb");
}

void loop() {

}

void FlashLED(int m) {
  for (int n = 0; n < m; n++) {
    digitalWriteFast(13, 1);          //set pin 13 high
    delay(100);
    digitalWriteFast(13, 0);          //set pin 13 low
    delay(100);
  }
}
