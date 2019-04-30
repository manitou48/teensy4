// Paul's GPT pin capture of GPS PPT  altered by thd
//https://forum.pjrc.com/threads/54265-Teensy-4-testing-mbed-NXP-MXRT1050-EVKB-(600-Mhz-M7)?p=193217&viewfull=1#post193217
// CLKSRC 1 24mhz   4 32khz  (GPS PPS width too small for 32khz?)
// 1062 capture pin 15  Serial3 Rx    test with GPS pps or pwm on pin 14
// GPT2 capture 1 GPIO_AD_B1_03   Alt 8
//  for 32khz clock, GPS pps pulse width > 30 us
//#include "debug/printf.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  Serial.println("ok");
  pinMode(13, OUTPUT);

  analogWriteFrequency(14, 100);  // test with PWM
  analogWrite(14, 128); // jumper pwm 14  to pin 15  Serial3 on T4B2 breakout
  // Connect GPS 1PPS signal to pin 15 (GPIO_AD_B1_03)
  IOMUXC_GPT2_IPP_IND_CAPIN1_SELECT_INPUT = 1;  // remap GPT2 capture 1
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03 = 8; // GPT2 Capture1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_03 = 0x13000; //Pulldown & Hyst
  CCM_CCGR0 |= CCM_CCGR0_GPT2_BUS(CCM_CCGR_ON) |
               CCM_CCGR0_GPT2_SERIAL(CCM_CCGR_ON);  // enable clock
  GPT2_CR = 0;
  GPT2_PR = 0;
  GPT2_SR = 0x3F; // clear all prior status
  GPT2_IR = GPT_IR_IF1IE;
  GPT2_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) |
            GPT_CR_FRR | GPT_CR_IM1(1);
  attachInterruptVector(IRQ_GPT2, capture);
  NVIC_ENABLE_IRQ(IRQ_GPT2);
}

#define LEN  10  // was 124

volatile uint32_t ticks;

void capture() {
  static uint32_t prior = 0;
  static uint32_t list[LEN];
  static uint32_t count = 0;
  static int index = 0;
  uint32_t now = GPT2_ICR1;
  GPT2_SR = GPT_SR_IF1;
  uint32_t n = now - prior;
  prior = now;
  asm("dsb");
  ticks++;
  //  Serial.println(index);
  if (index >= LEN) index = 0;
  list[index++] = n;
  count++;
  // GPIO2_DR_TOGGLE = (1 << 3);  // 1050
  if (index == LEN) {
    uint32_t sum = 0;
    for (int i = 0; i < LEN; i++) {
      sum = sum + list[i];
    }
    Serial.printf("capture=%u, sum=%u\n", n, sum);
  }
}

void loop() {
#if 0
  Serial.println(ticks);
  delay(1000);
#endif
}
