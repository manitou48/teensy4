// GPT1 counter like FreqCount
// external pin is 25 GPIO_AD_B0_13 ALT1  (backside)
// test with  PWM pin 11 jumpered to 25

// FreqCount API
static inline void counter_init(void)
{
  CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) ;  // enable GPT1 module
  GPT1_CR = 0;
  GPT1_SR = 0x3F; // clear all prior status
  GPT1_CR =  GPT_CR_CLKSRC(3);// | GPT_CR_FRR ;// 3 external clock
  *(portConfigRegister(25)) = 1;  // ALT 1
}

static inline void counter_start(void)
{
  GPT1_CR |= GPT_CR_EN; // enable
}

static inline void counter_shutdown(void)
{
  GPT1_CR = 0;
}

static inline uint32_t counter_read(void)  // was uint16_t in FreqCount?
{
  return GPT1_CNT;
}

static inline uint8_t counter_overflow(void)
{
  return GPT1_SR & GPT_SR_ROV;
}

static inline void counter_overflow_reset(void)
{
  GPT1_SR |= GPT_SR_ROV;
}

volatile uint32_t count_ready, count_output, count_prev;
void tmr_callback() {
  uint32_t count = counter_read();

  //track rollover ?
  count_output = count - count_prev;
  count_prev = count;
  count_ready = 1;
}

IntervalTimer it1;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  analogWriteFrequency(11, 1234);  // test jumper 12 to 25
  analogWrite(11, 128);

  counter_init();
  it1.begin(tmr_callback, 1000000);  // us
  counter_start();

}

void loop() {
  if (count_ready) {
    Serial.println(count_output);
    count_ready = 0;
  }
}
