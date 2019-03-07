##  Teensy 4 sketches and such 

Files | Description
---|---
acmpdac.ino  |    ACMP3 and DAC example
adcdma.ino   |    continuous ADC and DMA
adcdmapit.ino|    try to add DMA to PIT+XBAR+ADC_ETC+ADC  not working
cachetst     |    cache/no-cache from stack, OCRAM, PROGMEM
dcptst.ino   |    DCP proof-of-principle, SHA256, CRC32, and AES 
flexiopwm.ino |    flexio PWM 400mhz clock?
gpsgpt.ino   |    measure crystal drift of 24mhz and 32khz crystal with GPT
gpt_capture.ino | GPT1 input capture of GPS PPS signal, drift check 24mhz or 32khz
gpt_count.ino   | GPT1 clocked from pin 25, like FreqCount
gpt_micros.ino |  GPT1 micros (core micros only 10 us res)
pitxbaradc.ino |  clock ADC reads with PIT via XBAR
qtmr_capture.ino |quad timer capture
qtmrtst.ino    |  quad timer counting and PWM tests
rtc.ino        |  RTC off of 32 khz crystal
rtchp.ino      |  HP RTC off of 32 khz crystal, periodic sub-second alarm
spidma.ino     |  SPI DMA transmit
spidma2.ino    |  SPI DMA transmit and receive
sysinfo.ino    |  teensy 4 core registers
trng.ino       |  hardware TRNG example, 512 bits in 52 ms

--------
Some performance comparisons at

   https://github.com/manitou48/DUEZoo

   https://github.com/manitou48/crystals   teensy 3 RTC tuning
