// PPM in, report pulse widths
//  test with PulsePosition output pin 9 to T4 pin 8
// flexpwm 1 3 PWM1_A3   pin 8 B1_00 on 1062 ALT 6 daisy
// free-running 16-bit timer,  150mhz/4

#define PRREG(x) Serial.printf(#x" 0x%x\n",x);
#define MMASK (1<<3)   // module mask 3 for PWM1_A3

#define PULSEPOSITION_MAXCHANNELS 16
uint32_t pulse_width[PULSEPOSITION_MAXCHANNELS + 1];
uint32_t pulse_buffer[PULSEPOSITION_MAXCHANNELS + 1];
uint32_t write_index, prev, total_channels;

#define CLOCKS_PER_MICROSECOND (150./4)  // pcs 8+2
#define RX_MINIMUM_SPACE   3500.0
#define RX_MINIMUM_SPACE_CLOCKS   (uint32_t)(RX_MINIMUM_SPACE * CLOCKS_PER_MICROSECOND)


volatile uint32_t ticks, overflow_count;
volatile bool overflow_inc, available_flag;

void pwm1_3_isr() {
  if (FLEXPWM1_SM3STS &  FLEXPWM_SMSTS_RF) {
    FLEXPWM1_SM3STS = FLEXPWM_SMSTS_RF;   // rollover clear
    overflow_count++;
    overflow_inc = true;
  }
  if (FLEXPWM1_SM3STS &  FLEXPWM_SMSTS_CFA0) {
    uint32_t val, count;
    FLEXPWM1_SM3STS = FLEXPWM_SMSTS_CFA0;   // capture clear

    val = FLEXPWM1_SM3CVAL2;
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
  FLEXPWM1_FCTRL0 |= FLEXPWM_FCTRL0_FLVL(MMASK);  // clear
  FLEXPWM1_FSTS0 = FLEXPWM_FSTS0_FFLAG(MMASK);    //clear
  FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(MMASK);
  FLEXPWM1_SM3CTRL2 = FLEXPWM_SMCTRL2_INDEP;
  FLEXPWM1_SM3CTRL = FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_PRSC(2);
  FLEXPWM1_SM3INIT = 0;
  FLEXPWM1_SM3VAL0 = 0;
  FLEXPWM1_SM3VAL1 = 0xffff;
  FLEXPWM1_SM3VAL2 = 0;
  FLEXPWM1_SM3VAL3 = 0;
  FLEXPWM1_SM3VAL4 = 0;
  FLEXPWM1_SM3VAL5 = 0;
  FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(MMASK) | FLEXPWM_MCTRL_RUN(MMASK);
  FLEXPWM1_SM3CAPTCTRLA = FLEXPWM_SMCAPTCTRLA_EDGA0(2) | FLEXPWM_SMCAPTCTRLA_ARMA;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00 = 6 | 0x10;  // ALT6
  IOMUXC_FLEXPWM1_PWMA3_SELECT_INPUT = 4;   // daisy
  attachInterruptVector(IRQ_FLEXPWM1_3, pwm1_3_isr);
  FLEXPWM1_SM3STS = FLEXPWM_SMSTS_CFA0 | FLEXPWM_SMSTS_RF ;
  FLEXPWM1_SM3INTEN = FLEXPWM_SMINTEN_CA0IE | FLEXPWM_SMINTEN_RIE;
  NVIC_SET_PRIORITY(IRQ_FLEXPWM4_0, 48);
  NVIC_ENABLE_IRQ(IRQ_FLEXPWM1_3);
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
