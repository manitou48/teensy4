// T4 RTC  32khz crystal  HPSRTC   start and sync HPRTC
//  subsecond periodic alarm 1/32768 to 1 second
#include <time.h>
extern void *__rtc_localtime; // Arduino build process sets this, boards.txt
#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define SNVS_DEFAULT_PGD_VALUE (0x41736166U)
#define SNVS_LPSR_PGD_MASK                       (0x8U)
#define SNVS_LPSRTCMR      (IMXRT_SNVS.offset050)
#define SNVS_LPSRTCLR      (IMXRT_SNVS.offset054)
#define SNVS_HPCR_PI_FREQ_MASK                   (0xF0U)
#define SNVS_HPCR_PI_FREQ_SHIFT                  (4U)
#define SNVS_HPCR_PI_FREQ(x)                     (((uint32_t)(((uint32_t)(x)) << SNVS_HPCR_PI_FREQ_SHIFT)) & SNVS_HPCR_PI_FREQ_MASK)
#define SNVS_HPCR_PI_EN_MASK                     (0x8U)
#define SNVS_HPCR_PI_EN_SHIFT                    (3U)
#define SNVS_HPCR_PI_EN(x)                       (((uint32_t)(((uint32_t)(x)) << SNVS_HPCR_PI_EN_SHIFT)) & SNVS_HPCR_PI_EN_MASK)
#define SNVS_HPCR_HP_TS_MASK                     (0x10000U)

void rtc_stopAlarm()
{
  SNVS_HPSR |= 2;
  SNVS_HPCR &= ~SNVS_HPCR_PI_EN_MASK;
  while (SNVS_HPCR & SNVS_HPCR_PI_EN_MASK);
}

uint32_t ticks;
void rtc_isr(void)
{
  SNVS_HPSR |= 3;  // clear period
  ticks++;
  asm("dsb"); // needed? barrier or wait for clear
}

void rtc_initAlarm()
{
  attachInterruptVector(IRQ_SNVS_IRQ, rtc_isr);
  //  NVIC_SET_PRIORITY(IRQ_SNVS_IRQ, prio*16); // 8 is normal priority
  NVIC_DISABLE_IRQ(IRQ_SNVS_IRQ);
}

void rtc_setPeriodicAlarm(uint32_t period)
{
  /* disable SRTC alarm interrupt */
  rtc_stopAlarm();

  SNVS_HPCR &= ~SNVS_HPCR_PI_FREQ_MASK;  // clear period
  SNVS_HPCR |=  SNVS_HPCR_PI_FREQ(period);
  //   while(SNVS_LPTAR != alarmSeconds);
  asm("dsb");
  NVIC_ENABLE_IRQ(IRQ_SNVS_IRQ);

  SNVS_HPCR |=  SNVS_HPCR_PI_EN_MASK; // restore control register and set alarm
  while (!(SNVS_HPCR & SNVS_HPCR_PI_EN_MASK));
}

void rtc_init() {
  CCM_CCGR2 |= CCM_CCGR2_IOMUXC_SNVS(CCM_CCGR_ON);
  SNVS_LPGPR = SNVS_DEFAULT_PGD_VALUE;
  SNVS_LPSR = SNVS_LPSR_PGD_MASK;
  // ? calibration
  // ? tamper pins

  SNVS_LPCR |= 1;             // start RTC
  while (!(SNVS_LPCR & 1));

  SNVS_HPCR |= 1 | SNVS_HPCR_HP_TS_MASK;             // start and sync HP RTC
  while (!(SNVS_HPCR & 1));
}

void rtc_set_time(uint32_t secs) {

  SNVS_LPCR &= ~1;   // stop RTC
  SNVS_HPCR &= ~1;   // stop HP RTC
  while (SNVS_LPCR & 1);
  SNVS_LPSRTCMR = (uint32_t)(secs >> 17U);
  SNVS_LPSRTCLR = (uint32_t)(secs << 15U);
  SNVS_LPCR |= 1;             // start RTC
  SNVS_HPCR |= 1 | SNVS_HPCR_HP_TS_MASK;             // start and sync HP RTC
  while (!(SNVS_LPCR & 1));
}

uint32_t rtc_secs() {
  uint32_t seconds = 0;
  uint32_t tmp = 0;

  /* Do consecutive reads until value is correct */
  do
  {
    seconds = tmp;
    tmp = (SNVS_LPSRTCMR << 17U) | (SNVS_LPSRTCLR >> 15U);
  } while (tmp != seconds);

  return seconds;

}

uint32_t rtchp_secs() {
  uint32_t seconds = 0;
  uint32_t tmp = 0;

  /* Do consecutive reads until value is correct */
  do
  {
    seconds = tmp;
    tmp = (SNVS_HPRTCMR << 17U) | (SNVS_HPRTCLR >> 15U);
  } while (tmp != seconds);

  return seconds;

}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  rtc_init();
  rtc_set_time((uint32_t)&__rtc_localtime);   //LPSRTC will start at 0 otherwise  1547051415;
  time_t tt = rtc_secs();
  Serial.printf("time set to %s\n", ctime(&tt));
  rtc_initAlarm();
  rtc_setPeriodicAlarm(4);   // 0 to 15
  PRREG(SNVS_HPCR);
}

void loop() {
  Serial.printf("%d  %d  %d ticks\n", rtc_secs(), rtchp_secs(), ticks);
  delay(4000);
}
