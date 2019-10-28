// PPM in, report pulse widths
//  test with PulsePosition output pin 9 to T4 pin 11
// QTIMER1   pin capture test  qtmr 1 ch 2  pin 11 B0_02,  ch 52
// free-running 16-bit timer
// QTIMER oflow interrupt no workee,  use 0xffff compare for 32-bit
// polarity TMR_SCTRL_IPS
#define PRREG(x) Serial.printf(#x" 0x%x\n",x);

#define PULSEPOSITION_MAXCHANNELS 16
uint32_t pulse_width[PULSEPOSITION_MAXCHANNELS + 1];
uint32_t pulse_buffer[PULSEPOSITION_MAXCHANNELS + 1];
uint32_t write_index, prev, total_channels;

#define CLOCKS_PER_MICROSECOND (150./4)  // pcs 8+2
#define RX_MINIMUM_SPACE   3500.0
#define RX_MINIMUM_SPACE_CLOCKS   (uint32_t)(RX_MINIMUM_SPACE * CLOCKS_PER_MICROSECOND)


volatile uint32_t ticks, overflow_count;
volatile bool overflow_inc, available_flag;

void my_isr() {  // capture and compare
  if (TMR1_CSCTRL2 & TMR_CSCTRL_TCF1) { // compare rollover
    TMR1_CSCTRL2 &= ~(TMR_CSCTRL_TCF1);  // clear
    overflow_count++;
    overflow_inc = true;
  }
  if (TMR1_SCTRL2 & TMR_SCTRL_IEF) { // capture
    uint32_t val, count;
    TMR1_SCTRL2 &= ~(TMR_SCTRL_IEF);  // clear
    val = TMR1_CAPT2;
    count = overflow_count;
    if (val > 0xE000 && overflow_inc) count--;
    val |= (count << 16);
    count = val - prev;
    prev = val;
    if (count >= RX_MINIMUM_SPACE_CLOCKS) {
      if (write_index < 255) {
        for (int i = 0; i < write_index; i++) {
          pulse_buffer[i] = pulse_width[i];
        }
        total_channels = write_index;
        available_flag = true;
      }
      write_index = 0;
    } else {
      if (write_index < PULSEPOSITION_MAXCHANNELS) {
        pulse_width[write_index++] = count;
      }
    }
  }
  ticks++;
  asm volatile ("dsb");  // wait for clear  memory barrier
  overflow_inc = false;
}

void capture_init() {
  CCM_CCGR6 |= CCM_CCGR6_QTIMER1(CCM_CCGR_ON);

  TMR1_CTRL2 = 0; // stop
  TMR1_LOAD2 = 0;
  TMR1_CSCTRL2 = 0;
  TMR1_LOAD2 = 0;  // start val after compare
  TMR1_COMP12 = 0xffff;  // count up to this val, interrupt,  and start again
  TMR1_CMPLD12 = 0xffff;

  TMR1_SCTRL2 = TMR_SCTRL_CAPTURE_MODE(1);  //rising
  attachInterruptVector(IRQ_QTIMER1, my_isr);
  TMR1_SCTRL2 |= TMR_SCTRL_IEFIE;  // enable compare interrupt
  TMR1_CSCTRL2 = TMR_CSCTRL_TCF1EN;  // enable capture interrupt
  NVIC_SET_PRIORITY(IRQ_QTIMER1, 32);
  NVIC_ENABLE_IRQ(IRQ_QTIMER1);
  TMR1_CTRL2 =  TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 2) | TMR_CTRL_SCS(2) | TMR_CTRL_LENGTH ; // prescale
  *(portConfigRegister(11)) = 1;  // ALT 1
}

int ppmIn_available() {
  uint32_t total;
  bool flag;

  __disable_irq();
  flag = available_flag;
  total = total_channels;
  __enable_irq();
  if (flag) return total;
  return -1;
}

float ppmIn_read(uint8_t channel) {
  uint32_t total, index, value = 0;

  if (channel == 0) return 0.0;
  index = channel - 1;
  __disable_irq();
  total = total_channels;
  if (index < total) value = pulse_buffer[index];
  if (channel >= total) available_flag = false;
  __enable_irq();
  return (float)value / (float)CLOCKS_PER_MICROSECOND;
}

void setup()   {
  Serial.begin(9600);
  while (!Serial);
  delay(1000);

  write_index = 255;
  available_flag = false;
  capture_init();

  PRREG(TMR1_SCTRL2);
  PRREG(TMR1_CSCTRL2);
  PRREG(TMR1_CTRL2);
  PRREG(TMR1_LOAD2);
  PRREG(TMR1_COMP12);
  PRREG(TMR1_CMPLD12);
  PRREG(TMR1_COMP22);
  PRREG(TMR1_CMPLD22);
}

void loop() {
  int i, num;
  static int count = 0;

  // Every time new data arrives, simply print it
  // to the Arduino Serial Monitor.
  num = ppmIn_available();
  if (num > 0) {
    count = count + 1;
    Serial.print(count);
    Serial.print(" :  ");
    for (i = 1; i <= num; i++) {
      float val = ppmIn_read(i);
      Serial.print(val);
      Serial.print("  ");
    }
    Serial.println();
  }
}
