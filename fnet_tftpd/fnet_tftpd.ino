// fnet tftp

#include <SD.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include "tftp_server.h"

File file;

// file interface
static uint32_t nbytes, us;

void* tftp_fs_open(const char *fname, const char *mode, uint8_t write)
{
  char  *f = (char *)"xx";

  nbytes = 0;
  Serial.printf("opening %s  %d %d\n", fname, write, O_READ);
  us = micros();
  if (write == 0) {
    Serial.println("opening for read");
    file = SD.open(fname);
  }
  else file = SD.open(fname, FILE_WRITE);
  if (file) Serial.println("opened");
  else return NULL;

  return f;
}

void tftp_fs_close(void *handle)
{
  us = micros() - us;
  file.close();
  Serial.printf("closed %d bytes %d us\n", nbytes, us);
}

int tftp_fs_read(void *handle, void *buf, int bytes)
{
  int ret = 0;

  //Serial.printf("read avail %d\n", file.available());
  if (file.available()) {
    ret = file.read((uint8_t*)buf, bytes);
    nbytes += ret;
    // Serial.printf("read  %d %d\n", bytes, ret);
  }
  return ret;
}

int tftp_fs_write(void *handle, void *buf, int bytes)
{
  int ret;

  nbytes += bytes;
  ret = file.write((char *)buf, bytes);
  return ret;
}

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
  static const tftp_context tftp_ctx = { tftp_fs_open, tftp_fs_close, tftp_fs_read, tftp_fs_write };
  Serial.begin(9600);
  while (!Serial);
  delay(100);
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
  tftp_init(&tftp_ctx);
}

void loop() {
}
