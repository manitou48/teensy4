// GPS PPS on pin 12  GPIO2 1  B0_01  time with GPT1
// clock(1) 24mhz   clock(4) 32khz   set  CLKSRC
// clock(5) doesn't tick
// 24mhz  PREDIV 0 TPS 24000000,  PREDIV 5  TPS 4000000
// @150 mhz PREDIV 0 TPS 150000000 to get 150 mhz

#define CLKSRC 1     // 1 or 4
#if CLKSRC == 1
#define TPS 150000000
#define PREDIV 0
#elif CLKSRC == 4
#define TPS 32768
#define PREDIV 0
#else
#error choose CLKSRC 1 or 4
#endif

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
  // uncomment following for 150mhz
  CCM_CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL; // turn off 24mhz mode
  CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) ;  // enable GPT1 module
  GPT1_CR = 0;
  GPT1_PR = PREDIV;   // prescale+1 /1 for 32k,  /24 for 24mhz   /24 clock 1
  GPT1_SR = 0x3F; // clear all prior status
  GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(CLKSRC) | GPT_CR_FRR ;// 1 ipg 24mhz  4 32khz
  attachInterrupt(12, pinisr, RISING);
}

void loop() {
  static uint32_t n = 1,  prev = 0;
  static float sum = 0;

  if (tick)  {
    if (prev != 0) {
      int d = ticks - prev;
      float ppm = 1000000. * (d - TPS) / TPS;
      sum += ppm;
      Serial.printf("%d secs %d ticks  %.3f  %.3f ppm  %.1f C\n", n, d, ppm, sum / n, tempmonGetTemp());
      n++;
    }
    tick = 0;
    prev = ticks;
  }
}
