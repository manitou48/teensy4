// GPT2 counter like FreqCount
// external pin is 14 GPIO_AD_B1_02 ALT8  (Frontside - A0) Serial3 Tx
// test with  PWM pin 11 jumpered to 14 

// FreqCount API
static inline void counter_init(void)
{
  CCM_CCGR0 |= CCM_CCGR0_GPT2_BUS(CCM_CCGR_ON) ;  // enable GPT1 module
  //CCM_CCGR0 |= CCM_CCGR0_GPT2_SERIAL(CCM_CCGR_ON) ;
  GPT2_CR = 0;
  GPT2_SR = 0x3F; // clear all prior status
  GPT2_CR =  GPT_CR_CLKSRC(3);// | GPT_CR_FRR ;// 3 external clock
  //*(portConfigRegister(14)) = 8;  // ALT 1
  //CORE_PIN14_CONFIG = 8;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02 = 8;
  IOMUXC_GPT2_IPP_IND_CLKIN_SELECT_INPUT = 1;
}

static inline void counter_start(void)
{
  GPT2_CR |= GPT_CR_EN; // enable
}

static inline void counter_shutdown(void)
{
  GPT2_CR = 0;
}

static inline uint32_t counter_read(void)  // was uint16_t in FreqCount?
{
  return GPT2_CNT;
}

static inline uint8_t counter_overflow(void)
{
  return GPT2_SR & GPT_SR_ROV;
}

static inline void counter_overflow_reset(void)
{
  GPT2_SR |= GPT_SR_ROV;
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
  analogWriteFrequency(11, 1234);  // test jumper 11 to 14
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
