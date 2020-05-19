// fnet tftp  SPIFFS external flash

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <spiffs_t4.h>
#include <spiffs.h>

#include "tftp_server.h"
static spiffs_t4 fs; //filesystem
static spiffs_file fd;

//  SPIFFS interface  open close read write
static uint32_t nbytes, us;

void* tftp_fs_open(const char *fname, const char *mode, uint8_t write)
{
  char  *f = (char *)"xx";

  nbytes = 0;
  Serial.printf("opening %s  %d \n", fname, write);
  us = micros();
  if (write == 0) {
    Serial.println("opening for read");
    fs.f_open(fd, fname, SPIFFS_RDWR);
  }
  else fs.f_open(fd, fname, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR);
  if (fd) Serial.println("opened");
  else return NULL;

  return f;
}

void tftp_fs_close(void *handle)
{
  us = micros() - us;
  fs.f_close( fd);
  Serial.printf("closed %d bytes %d us\n", nbytes, us);
}

int tftp_fs_read(void *handle, void *buf, int bytes)
{
  int ret = 0;

  ret = fs.f_read( fd, (u8_t *)buf, bytes);
  if (ret > 0)  nbytes += ret;
  else ret = 0;
  return ret;
}

int tftp_fs_write(void *handle, void *buf, int bytes)
{
  int ret;

  nbytes += bytes;
  ret = fs.f_write(fd, buf, bytes);
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

  Serial.println("Mount SPIFFS:");
  fs.begin();
  fs.fs_mount();
  const float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
  const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / (float)(((CCM_CBCMR >> 29) & 7) + 1);
  Serial.printf("CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);

  Ethernet.setStackHeap(1024 * 64);
  Ethernet.setSocketSize(1460 * 4); //Set buffer size
  Ethernet.setSocketNum(6); //Change number of allowed sockets to 6

  uint8_t mac[6];
  teensyMAC(mac);
  Ethernet.begin(mac);
  Serial.print("IP  address:");
  Serial.println(Ethernet.localIP());
  Serial.println("tftp server");
  tftp_init(&tftp_ctx);
}

void loop() {
}
