// variable PWM  qtmr 1 ch 0  pin 10 B0_00
// ref 54.4.5.14 Variable-Frequency PWM Mode

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

static volatile uint32_t ticks, update, pd_index, high, low;
uint16_t pd[] = {   // period ticks and duty
  50000, 80,
  30000, 75,
  15000, 33
};


void my_isr() {
  TMR1_CSCTRL0 &= ~(TMR_CSCTRL_TCF1 | TMR_CSCTRL_TCF2);  // clear
  ticks++;
  if (update) {   // new period duty
    update = 0;
    TMR1_CMPLD10 = low - 1;
    TMR1_CMPLD20 = high - 1 ;
  }

  asm volatile ("dsb");  // wait for clear  memory barrier
}


void pwm_init() {  // used fixed freq 150mhz, should calculate prescale
  uint32_t period, duty,  srcclk = 150000000, pcs = 0;
  period = pd[0];
  duty = pd[1];

  high = (period * duty) / 100;
  low = period - high;
  Serial.printf("period %d low %d high %d pcs %d\n", period, low, high, pcs);
  TMR1_CTRL0 = 0; // stop
  TMR1_CNTR0 = 0;
  TMR1_LOAD0 = 0;

  TMR1_COMP10 = low - 1;
  TMR1_CMPLD10 = low - 1;
  TMR1_COMP20 = high - 1;
  TMR1_CMPLD20 = high - 1;
  TMR1_SCTRL0 =  TMR_SCTRL_OEN ;
  TMR1_CSCTRL0 = TMR_CSCTRL_CL1(2) | TMR_CSCTRL_CL2(1);
  attachInterruptVector(IRQ_QTIMER1, my_isr);
  TMR1_CSCTRL0 |= TMR_CSCTRL_TCF2EN;  // enable interrupt
  NVIC_ENABLE_IRQ(IRQ_QTIMER1);
  TMR1_CTRL0 =  TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + pcs) | TMR_CTRL_LENGTH |
                TMR_CTRL_OUTMODE(4);
  *(portConfigRegister(10)) = 1;  // ALT 1
}

void setup()   {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);
  pwm_init();

  PRREG(TMR1_SCTRL0);
  PRREG(TMR1_CSCTRL0);
  PRREG(TMR1_CTRL0);
  PRREG(TMR1_LOAD0);
  PRREG(TMR1_COMP10);
  PRREG(TMR1_CMPLD10);
  PRREG(TMR1_COMP20);
  PRREG(TMR1_CMPLD20);
}

void loop()
{
  Serial.println(ticks);
  delay(1000);
  if (Serial.available()) {
    uint32_t period, duty;
    while (Serial.available()) Serial.read();
    pd_index += 2;
    if (pd_index >= (sizeof(pd) / sizeof(pd[0]))) pd_index = 0;
    period = pd[pd_index];
    duty = pd[pd_index + 1];
    high = (period * duty) / 100;
    low = period - high;
    update = 1;  // notify ISR
    Serial.printf("period %d low %d high %d \n", period, low, high);
  }
}
