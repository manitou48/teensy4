// fnet tftp client   UDP port 69

#include <SD.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include "tftp_client.h"

IPAddress server(192, 168, 1, 10);
File file;

// file interface
static uint32_t nbytes, us;


static void teensyMAC(uint8_t *mac)
{
  uint32_t m1 = HW_OCOTP_MAC1;
  uint32_t m2 = HW_OCOTP_MAC0;
  mac[0] = m1 >> 8;
  mac[1] = m1 >> 0;
  mac[2] = m2 >> 24;
  mac[3] = m2 >> 16;
  mac[4] = m2 >> 8;
  mac[5] = m2 >> 0;
}

void setup() {
  uint8_t buff[512];

  Serial.begin(9600);
  while (!Serial);
  delay(100);
  Serial.println("tftp client");
  Serial.print("Initializing SD card...");

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  Ethernet.setStackHeap(1024 * 64);
  Ethernet.setSocketSize(1460 * 4); //Set buffer size
  Ethernet.setSocketNum(6); //Change number of allowed sockets to 6

  uint8_t mac[6];
  teensyMAC(mac);
  Ethernet.begin(mac);
  Serial.print("IP  address:");
  Serial.println(Ethernet.localIP());

  us = micros();
  int ret = tftp_open(server,"test.dat",1);
  tftp_write(buff,sizeof(buff);
  nbytes += sizeof(buff);
  tftp_write(buff,sizeof(buff);
  nbytes += sizeof(buff);
  tftp_close();
  us = micros() - us;
  Serial.printf("write %d bytes %d us\n",nbytes,us);
  int n;
  us = micros();
  ret = tftp_open(server,"test.dat",0);
  while(ret > 0) {
  	ret = tftp_read(buff);
	if (ret > 0) nbytes += ret;
  }
  us = micros() - us;
  Serial.printf("read %d bytes %d us\n",nbytes,us);

  ret = tftp_open(server,"test.dat",0);
  while(ret > 0) {
  	ret = tftp_read(buff);
	if (ret >0) Serial.write(buff,ret);
  } 
}

void loop() {
}
