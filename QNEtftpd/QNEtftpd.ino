// fnet tftp

#include <SD.h>
#include <QNEthernet.h>

using namespace qindesign::network;
#include "tftp_server.h"
// QNEthernet links this variable with lwIP's `printf` calls for
// assertions and debugging. User code can also use `printf`.
extern Print *stdPrint;

// IP configuration
IPAddress staticIP{192, 168, 1, 19}; // Set to non-zero for a static IP
IPAddress subnetMask{255, 255, 255, 0}; // Used if staticIP != 0
IPAddress gateway{192, 168, 1, 1};  // Used if staticIP != 0

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


void setup() {
  static const tftp_context tftp_ctx = { tftp_fs_open, tftp_fs_close, tftp_fs_read, tftp_fs_write };
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  stdPrint = &Serial;
  printf("Starting...\n");


  Serial.print("Initializing SD card...");

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  uint8_t mac[6];
  Ethernet.macAddress(mac);
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.println("Server: Starting Ethernet (Static)...");
  Ethernet.begin(staticIP, subnetMask, gateway);
  Ethernet.setDNSServerIP(gateway);

  Serial.print("tftpd server IP  address:");
  Serial.println(Ethernet.localIP());
  tftp_init(&tftp_ctx);
}

void loop() {
}
