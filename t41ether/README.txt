T41 ether files

 etherraw/  hacked version of Paul's low-level Ether sketch, hand-crafted UDP

 lwip/   lwIP library file, put in sketchbook/libraries

 lwip test sketches
  lwip_iperf/  lwip_perf/ lwip_echosrv/  lwip_mcast/  lwip_sntp/

you need to alter boards.txt to inject include path for compile

teensy41.build.flags.common=-g -Wall -ffunction-sections -fdata-sections -nostdlib -I/home/dunigan/sketchbook/libraries/lwip/src/include

  (alter for your home directory ...)

TODO: fetch MAC address from internal fuses
