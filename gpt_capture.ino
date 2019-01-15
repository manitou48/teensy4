// Paul's GPT pin capture of GPS PPT  altered by thd
//https://forum.pjrc.com/threads/54265-Teensy-4-testing-mbed-NXP-MXRT1050-EVKB-(600-Mhz-M7)?p=193217&viewfull=1#post193217
// CLKSRC 1 24mhz   4 32khz
//#include "debug/printf.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  Serial.println("ok");
  pinMode(13, OUTPUT);

  analogWriteFrequency(11, 100);  // test with PWM
  analogWrite(11, 128); // jumper 11 MOSI to pin 30  Serial 8 Rx breakout
  // Connect GPS 1PPS signal to pin 30 (EMC_24)
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = 4; // GPT1 Capture1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_24 = 0x13000; //Pulldown & Hyst
  CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) |
               CCM_CCGR1_GPT_SERIAL(CCM_CCGR_ON);
  GPT1_CR = 0;
  GPT1_PR = 0;
  GPT1_SR = 0x3F; // clear all prior status
  GPT1_IR = GPT_IR_IF1IE;
  GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) |
            GPT_CR_FRR | GPT_CR_IM1(1) ;
  attachInterruptVector(IRQ_GPT1, capture);
  NVIC_ENABLE_IRQ(IRQ_GPT1);
}

#define LEN  100  // was 124

volatile uint32_t ticks;
void capture() {
  static uint32_t prior = 0;
  static uint32_t list[LEN];
  static uint32_t count = 0;
  static int index = 0;
  uint32_t now = GPT1_ICR1;
  GPT1_SR = GPT_SR_IF1;
  uint32_t n = now - prior;
  prior = now;
  asm("dsb");
  //  Serial.println(index);
  if (index >= LEN) index = 0;
  list[index++] = n;
  count++;
  GPIO2_DR_TOGGLE = (1 << 3);
  if (index == LEN) {
    uint32_t sum = 0;
    for (int i = 0; i < LEN; i++) {
      sum = sum + list[i];
    }
    Serial.printf("capture=%u, sum=%u\n", n, sum);
    ticks++;

  }
}

void loop() {
  //Serial.println(ticks);
  //delay(1000);
}
