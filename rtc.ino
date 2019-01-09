// T4 RTC  32khz crystal  LPSRTC   start and sync HPRTC too ?

#define SNVS_DEFAULT_PGD_VALUE (0x41736166U)
#define SNVS_LPSR_PGD_MASK                       (0x8U)
#define SNVS_LPSRTCMR      (IMXRT_SNVS.offset050)
#define SNVS_LPSRTCLR      (IMXRT_SNVS.offset054)

void rtc_init() {
  CCM_CCGR2 |= CCM_CCGR2_IOMUXC_SNVS(CCM_CCGR_ON);
  SNVS_LPGPR = SNVS_DEFAULT_PGD_VALUE;
  SNVS_LPSR = SNVS_LPSR_PGD_MASK;
  // ? calibration
  // ? tamper pins

  SNVS_LPCR |= 1;             // start RTC
  while (!(SNVS_LPCR & 1));
}

void rtc_set_time() {
  uint32_t secs = 1547051415;
  SNVS_LPCR &= ~1;   // stop RTC
  while (SNVS_LPCR & 1);
  SNVS_LPSRTCMR = (uint32_t)(secs >> 17U);
  SNVS_LPSRTCLR = (uint32_t)(secs << 15U);
  SNVS_LPCR |= 1;             // start RTC
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

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(2000);
  rtc_init();
  rtc_set_time();   //LPSRTC will start at 0 otherwise
}

void loop() {
  Serial.println(rtc_secs());
  delay(1000);

}
