// GPT micros  GPT1 1mhz  T4 core micros only 10 us resolution

uint32_t gpt_micros() {
  return GPT1_CNT;
}

void gpt_init() {
  CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) |
               CCM_CCGR1_GPT_SERIAL(CCM_CCGR_ON);  // enable GPT1 module
  GPT1_CR = 0;
  GPT1_PR = 23;   // prescale+1
  GPT1_SR = 0x3F; // clear all prior status
  GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) | GPT_CR_FRR ;// 1 ipg 24mhz  4 32khz
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  gpt_init();
  delay(2000);
  uint32_t t = gpt_micros();
  for (int i = 0; i < 10000; i++) sqrt(i); ;
  t = gpt_micros() - t;
  Serial.println(t);
  t = micros();
  for (int i = 0; i < 10000; i++) sqrt(i);;
  t = micros() - t;
  Serial.println(t);
}

void loop() {
  Serial.println(gpt_micros());
  delay(1234);
}
