// GPS PPS on pin 12  GPIO2 1  B0_01  time with GPT1
// clock(5) doesn't tick

#include "Arduino.h"

#define TPS 1000000
//#define TPS 32768

uint32_t gpt_ticks() {
  return GPT1_CNT;
}

volatile uint32_t tick, ticks;
void pinisr() {
  tick = 1;
  ticks = gpt_ticks();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(3000);
  Serial.println("starting");
  CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) |
               CCM_CCGR1_GPT_SERIAL(CCM_CCGR_ON);  // enable GPT1 module
  GPT1_CR = 0;
  GPT1_PR = 23;   // prescale+1 /1 for 32k,  /24 for 24mhz   /24 clock 1
  GPT1_SR = 0x3F; // clear all prior status
  GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) | GPT_CR_FRR ;// 1 ipg 24mhz  4 32khz 
  attachInterrupt(12, pinisr, RISING);
}

void loop() {
  static uint32_t n = 1,  prev = 0;
  static float sum = 0;

  if (tick)  {
    if (prev != 0) {
      int d = ticks - prev;
      int ppm = 1000000 * (d - TPS) / TPS;
      sum += ppm;
      Serial.printf("%d secs %d ticks  %d  %.1f ppm\n", n, d, ppm, sum / n);
      n++;
    }
    tick = 0;
    prev = ticks;
  }
}
