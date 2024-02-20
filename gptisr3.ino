// GPT1 ISR test, 3 compare registers, OCR1 needs to be longest period, resets count


volatile uint32_t of1, of2, of3, us1, us2, us3;

void gpt1_isr() {
  uint32_t srtmp = GPT1_SR, ustmp = GPT1_CNT;
  GPT1_SR |= GPT_SR_OF1;  // clear all set bits
  if (srtmp & GPT_SR_OF1) {
    of1++;
    us1 = ustmp;
    digitalWriteFast(13, 1 - digitalReadFast(13));
  }
  if (srtmp & GPT_SR_OF2) {
    of2++;
    us2 = ustmp;
    //digitalWriteFast(12, 1 - digitalReadFast(12));
  }
  if (srtmp & GPT_SR_OF3) {
    of3++;
    us3 = ustmp;
    digitalWriteFast(12, 1 - digitalReadFast(12));
  }
  asm volatile ("dsb");
}

void gpt1_init() {
  CCM_CCGR1 |= CCM_CCGR1_GPT(CCM_CCGR_ON) ;  // enable GPT1 module
  GPT1_CR = 0;     // FRR 0 reset counter on OF1
  GPT1_PR = 23;   // prescale+1   1 mhz
  GPT1_OCR1 = 5000 - 1;  // compare  longest period 5 ms, will reset counter
  GPT1_OCR2 = 3000 - 1;  // compare
  GPT1_OCR3 = 1000 - 1;  // compare
  GPT1_SR = 0x3F; // clear all prior status
  GPT1_IR = GPT_IR_OF1IE | GPT_IR_OF2IE | GPT_IR_OF3IE;
  GPT1_CR = GPT_CR_EN | GPT_CR_CLKSRC(1) ;// 1 ipg 24mhz  4 32khz
  attachInterruptVector(IRQ_GPT1, gpt1_isr);
  NVIC_ENABLE_IRQ(IRQ_GPT1);
}


void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  pinMode(13, OUTPUT);    //scope check stagger
  pinMode(12, OUTPUT);
  gpt1_init();
}

void loop() {
  delay(1000);
  Serial.printf("of3 %u  %u us  of2 %u %u us of1 %u %u us\n",
                of3, us3, of2, us2, of1, us1);

}
