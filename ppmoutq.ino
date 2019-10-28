//  PPM out on pin 10
// QTIMER1  test  qtmr 1 ch 0  pin 10 B0_00, F_BUS_ACTUAL 150 mhz
//  v2  switch CTRL OUTMODE between clear and set
// timer clock 150mhz/4    PCS 8+2  TMR_SCTRL_OPS  ouput polarity

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define CLOCKS_PER_MICROSECOND (150./4)  // pcs 8+2

#define TX_PULSE_WIDTH      100.0
#define TX_DEFAULT_SIGNAL  1500.0
#define TX_MINIMUM_SPACE   5000.0
#define TX_MINIMUM_FRAME  20000.0
#define TX_MINIMUM_SIGNAL   300.0
#define TX_MAXIMUM_SIGNAL  2500.0
#define TX_MINIMUM_SPACE_CLOCKS   (uint32_t)(TX_MINIMUM_SPACE * CLOCKS_PER_MICROSECOND)
#define TX_MINIMUM_FRAME_CLOCKS   (uint32_t)(TX_MINIMUM_FRAME * CLOCKS_PER_MICROSECOND)
#define TX_PULSE_WIDTH_CLOCKS     (uint32_t)(TX_PULSE_WIDTH * CLOCKS_PER_MICROSECOND)
#define TX_DEFAULT_SIGNAL_CLOCKS  (uint32_t)(TX_DEFAULT_SIGNAL * CLOCKS_PER_MICROSECOND)

#define CTRL_SET TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 2) | TMR_CTRL_LENGTH |TMR_CTRL_OUTMODE(2)
#define CTRL_CLEAR TMR_CTRL_CM(1) | TMR_CTRL_PCS(8 + 2) | TMR_CTRL_LENGTH |TMR_CTRL_OUTMODE(1)
#define PULSEPOSITION_MAXCHANNELS 16
uint32_t pulse_width[PULSEPOSITION_MAXCHANNELS + 1];
uint32_t pulse_buffer[PULSEPOSITION_MAXCHANNELS + 1];

uint32_t state, total_channels, total_channels_buffer, pulse_remaining,
         current_channel, framePin = 255;

volatile uint32_t ticks;
void my_isr() {
  TMR1_CSCTRL0 &= ~(TMR_CSCTRL_TCF1);  // clear compare interrupt
  ticks++;

  if (state == 0) {
    // pin was just set high, schedule it to go low
    TMR1_COMP10 = TMR1_CMPLD10 = TX_PULSE_WIDTH_CLOCKS;
    TMR1_CTRL0 =  CTRL_CLEAR;
    state = 1;
  } else {
    // pin just went low
    uint32_t width, channel;
    if (state == 1) {
      channel = current_channel;
      if (channel == 0) {
        total_channels_buffer = total_channels;
        for (uint32_t i = 0; i <= total_channels_buffer; i++) {
          pulse_buffer[i] = pulse_width[i];
        }
      }
      width = pulse_buffer[channel] - TX_PULSE_WIDTH_CLOCKS;
      if (++channel > total_channels_buffer) {
        channel = 0;
      }
      if (framePin < NUM_DIGITAL_PINS) {
        if (channel == 1) {
          digitalWrite(framePin,HIGH);
        } else {
          digitalWrite(framePin,LOW);
        }
      }
      current_channel = channel;
    } else {
      width = pulse_remaining;
    }
    if (width <= 60000) {
      TMR1_COMP10 = TMR1_CMPLD10 = width;
      TMR1_CTRL0 =  CTRL_SET; // set on compare match & interrupt
      state = 0;
    } else {
      TMR1_COMP10 = TMR1_CMPLD10 = 58000;
      TMR1_CTRL0 =  CTRL_CLEAR; // clear on compare match & interrupt
      pulse_remaining = width - 58000;
      state = 2;
    }
  }

  asm volatile ("dsb");  // wait for clear  memory barrier
}

void qtmr_init() {
  TMR1_CTRL0 = 0; // stop
  TMR1_CNTR0 = 0;
  TMR1_LOAD0 = 0;

 //  framePin = 2;   // optional select a framePin
  if (framePin < NUM_DIGITAL_PINS) {
    pinMode(framePin,OUTPUT);
    digitalWrite(framePin,HIGH);
  }
  TMR1_COMP10 = 200;  // first time
  state = 0;
  pulse_width[0] = TX_MINIMUM_FRAME_CLOCKS;
  for (int i = 1; i <= PULSEPOSITION_MAXCHANNELS; i++) {
    pulse_width[i] = TX_DEFAULT_SIGNAL_CLOCKS;
  }
  TMR1_CMPLD10 = TX_PULSE_WIDTH_CLOCKS;
  TMR1_SCTRL0 = TMR_SCTRL_OEN ;
  TMR1_CSCTRL0 = TMR_CSCTRL_CL1(1);
  attachInterruptVector(IRQ_QTIMER1, my_isr);
  TMR1_CSCTRL0 &= ~(TMR_CSCTRL_TCF1);  // clear
  TMR1_CSCTRL0 |= TMR_CSCTRL_TCF1EN;  // enable interrupt
  NVIC_SET_PRIORITY(IRQ_QTIMER1, 32);
  NVIC_ENABLE_IRQ(IRQ_QTIMER1);
  TMR1_CTRL0 =  CTRL_SET;
  *(portConfigRegister(10)) = 1;  // ALT 1
}

bool ppmOut_write(uint8_t channel, float microseconds)
{
  uint32_t i, sum, space, clocks, num_channels;

  if (channel < 1 || channel > PULSEPOSITION_MAXCHANNELS) return false;
  if (microseconds < TX_MINIMUM_SIGNAL || microseconds > TX_MAXIMUM_SIGNAL) return false;
  clocks = microseconds * CLOCKS_PER_MICROSECOND;
  num_channels = total_channels;
  if (channel > num_channels) num_channels = channel;
  sum = clocks;
  for (i = 1; i < channel; i++) sum += pulse_width[i];
  for (i = channel + 1; i <= num_channels; i++) sum += pulse_width[i];
  if (sum < TX_MINIMUM_FRAME_CLOCKS - TX_MINIMUM_SPACE_CLOCKS) {
    space = TX_MINIMUM_FRAME_CLOCKS - sum;
  } else {
    if (framePin < NUM_DIGITAL_PINS) {
      space = TX_PULSE_WIDTH_CLOCKS;
    } else {
      space = TX_MINIMUM_SPACE_CLOCKS;
    }
  }
  __disable_irq();
  pulse_width[0] = space;
  pulse_width[channel] = clocks;
  total_channels = num_channels;
  __enable_irq();
  return true;
}

void setup()   {

  CCM_CCGR6 |= CCM_CCGR6_QTIMER1(CCM_CCGR_ON);

  qtmr_init();  //  pin 10

  ppmOut_write(1, 600.03);
  ppmOut_write(2, 1500);
  ppmOut_write(3, 759.24);
  // slots 4 and 5 will default to 1500 us
  ppmOut_write(6, 1234.56);
}

void loop() {

}
