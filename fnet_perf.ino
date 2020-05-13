/*
  fnet perf  arduino API wrapper
  from wizperf
   udp_echo  roundtrip latency 8 byte uechosrv
   udp_send  sink test 1000-byte 10 reps to udpsink
   udp_recv  input test 1000-byte pkts  udpsrc
   uechosrv  from uvdelay
   uvdelay to uechosrv
   udp_ntp
   tcp_send  ttcp client
   tcp_recv  ttcp server
*/

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <time.h>

#define PRREG(x) Serial.printf(#x" 0x%x\n",x)
#define swap4 __builtin_bswap32


IPAddress ip(192, 168, 1, 17);

#define REPS 10

#define NBYTES 1000000
#define RECLTH 1000
#define TTCP_PORT 5001
EthernetServer server(TTCP_PORT);
EthernetClient client;


unsigned int localPort = 8888;      // local port to listen for UDP packets
unsigned int dstport = 7654;      //  dst port

IPAddress MyServer(192, 168, 1, 4);

#define PACKET_SIZE 1024

byte packetBuffer[ PACKET_SIZE]; //buffer to hold incoming and outgoing packets

uint8_t mac[6];
static void teensyMAC(uint8_t *mac)
{
  uint32_t m1 = HW_OCOTP_MAC1;   // T4 MAC
  uint32_t m2 = HW_OCOTP_MAC0;
  mac[0] = m1 >> 8;
  mac[1] = m1 >> 0;
  mac[2] = m2 >> 24;
  mac[3] = m2 >> 16;
  mac[4] = m2 >> 8;
  mac[5] = m2 >> 0;
}


// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void udp_echo() {
  uint32_t  t1, t2;
  static int lost = 0; // for static ip, first pkt not sent?
  t1 = micros();
  Udp.beginPacket(MyServer, dstport);   //uechosrv
  Udp.write(packetBuffer, 8);
  Udp.endPacket();

  while (!Udp.parsePacket()) {
    // wait to see if a reply is available
    if (micros() - t1 > 1000000) {
      lost++;
      Serial.print("lost "); Serial.println(lost);
      return;
    }
  }
  // We've received a packet, read the data from it
  Udp.read(packetBuffer, 8); // read the packet into the buffer
  t2 = micros() - t1;
  Serial.println(t2);
}

void udp_send(int reps, int nbytes) {
  uint32_t  i, t1, t2;
  float mbs;

  t1 = micros();
  for (i = 0; i < reps; i++) {
    Udp.beginPacket(MyServer, 2000);   // to udpsink
    Udp.write(packetBuffer, nbytes);
    Udp.endPacket();
  }
  t2 = micros() - t1;
  mbs = (8.*nbytes * reps) / t2;
  Serial.printf("UDP blast %d reps %d bytes %d us %f mbs\n", reps, nbytes, t2, mbs);
}

void udp_recv() {
  uint32_t i, t1, t2, pkts, bytes, ipaddr;
  char buff[128];
  double mbs;

  Serial.print(Ethernet.localIP());
  while (1) {
    sprintf(buff, " port %d, hit key to stop", localPort);
    Serial.println(buff);
    pkts = bytes = 0;
    while (!Serial.available()) {
      int n = Udp.parsePacket();
      if (n) {
        if (!bytes) t1 = micros();
        t2 = micros();
        bytes += Udp.read(packetBuffer, 1000);
        pkts++;
      }
    }
    t1 = t2 - t1;
    mbs = (8.*bytes) / t1;
    Serial.print(mbs);
    sprintf(buff, " mbs %d pkts %d bytes on port %d ", pkts, bytes, Udp.remotePort());
    Serial.print(buff);
    IPAddress remote = Udp.remoteIP();
    Serial.println(remote);
    while (Serial.available()) Serial.read(); // consume
  }
}

void uechosrv() {
  Serial.print(Ethernet.localIP());
  Serial.printf(" uechosrv on %d\n", localPort);
  while (1) {
    while (!Udp.parsePacket()); // wait  ? timeout
    int n = Udp.read(packetBuffer, sizeof(packetBuffer));
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(packetBuffer, n);
    Udp.endPacket();
  }
}

void uvdelay(int cnt, int lth) {
  unsigned int t1, t2, i, n, tmin = 99999, tmax = 0, times[cnt];
  float avrg = 0;

  for (i = 0; i < cnt; i++) {
    t1 = micros();
    Udp.beginPacket(MyServer, 7654);   // to uechosrv
    Udp.write(packetBuffer, lth);
    Udp.endPacket();
    while ((n = Udp.parsePacket()) == 0) ;  // spin, timeout ?
    Udp.read(packetBuffer, n);
    t2 = micros() - t1;
    times[i] = t2;
  }
  for (i = 0; i < cnt; i++) {
    int v = times[i];
    if (v < tmin) tmin = v;
    if (v > tmax) tmax = v;
    avrg += v;
    Serial.print(v); Serial.print(" ");
  }
  Serial.println();
  avrg /= cnt;
  Serial.print(tmin); Serial.print(" ");
  Serial.print(tmax); Serial.print(" ");
  Serial.println(avrg, 2);
}


void udp_ntp() {
  uint32_t secs, nus, us, rtt;
  static uint32_t us0 = 0, secs0, nus0;
  while (1) {
    packetBuffer[0] = 0x1b;   // ntp query
    Udp.beginPacket(MyServer, 123);
    Udp.write(packetBuffer, 48);
    Udp.endPacket();
    rtt = micros();
    while (!Udp.parsePacket()) ; // poll wait
    us = micros();
    rtt = us - rtt;
    Udp.read(packetBuffer, sizeof(packetBuffer));
    secs = swap4(*(uint32_t *) (packetBuffer + 40));
    //nus = swap4(*(uint32_t *) (packetBuffer + 44)) / 4295; // fract sec
    uint64_t x = swap4(*(uint32_t *) (packetBuffer + 44));
    nus = (1000000 * x) >> 32;
    if (us0 == 0) {
      us0 = us;
      secs0 = secs;
      nus0 = nus;
      time_t edt = secs - 2208988800 - 4 * 3600; // NTP 1990 to unix 1970 and EDT
      Serial.print(ctime(&edt));
    }
    double t = 1000000 * (secs - secs0) + (nus - nus0); // elapsed remote us
    Serial.printf("ntp %u.%06d  rtt %d us   %.0f ppm over %d s\n",
                  secs, nus, rtt, (1000000. * ((us - us0) - t)) / t, secs - secs0);
    delay(10000);
  }
}

void tcp_send() {
  long t1, i, bytes = 0, n, sndlth;
  float mbs;

  if (!client.connect(MyServer, TTCP_PORT)) {
    Serial.println("connect failed");
    return;
  }
  t1 = millis();
  while (bytes < NBYTES) {
    sndlth = NBYTES - bytes;
    if (sndlth > RECLTH) sndlth = RECLTH;
    n = client.write(packetBuffer, sndlth);
    bytes += n;
  }
  client.stop();
  t1 = millis() - t1;
  mbs = 8 * NBYTES * .001 / t1;
  Serial.printf( "send  %ld bytes %ld ms  %f mbs\n", bytes, t1, mbs);

}

void tcp_recv() {
  Serial.printf("TCP server listening port %d\n", TTCP_PORT);
  server.begin();
  while (1) {
    long t1, n, bytes = 0;;
    float mbs;

    EthernetClient sender = server.available();
    if (sender) {
      t1 = millis();
      while (sender.connected()) {
        if ((n = sender.available()) > 0) {

          if (n > RECLTH)  n = RECLTH;
          sender.read(packetBuffer, n);
          bytes += n;
        }
      }
      t1 = millis() - t1;
      mbs = 8 * bytes * .001 / t1;
      Serial.printf("recv  %ld bytes %ld ms %f  mbs ", bytes, t1, mbs);
      Serial.print("from "); Serial.println(sender.remoteIP());
      Serial.println("awaiting connect");
      delay(10);
      sender.stop();
    }
  }
}

void tcp_echo(int eport) {
  EthernetServer eserver(eport);
  Serial.printf("TCP echo server listening port %d\n", eport);
  eserver.begin();
  while (1) {
    long records = 0, n, bytes = 0;;


    EthernetClient sender = eserver.available();
    if (sender) {
      while (sender.connected()) {
        if ((n = sender.available()) > 0) {
          if (n > RECLTH)  n = RECLTH;
          sender.read(packetBuffer, n);
          sender.write(packetBuffer, n); //  echo
          bytes += n;
          records++;
        }
      }

      Serial.printf("%ld records   %ld bytes ", records, bytes);
      Serial.print("from "); Serial.println(sender.remoteIP());
      Serial.println("awaiting connect");
      delay(10);
      sender.stop();
    }
  }
}

void hexdump(uint8_t *p, int lth) {
  char str[16];
  int i, j, k, n;

  if (lth <= 0) return;
  sprintf(str, "addr"); Serial.print(str);
  for (i = 0; i < 16; i++) {
    sprintf(str, " %02x", i);
    Serial.print(str);
  }
  Serial.println("");
  k = i = 0;
  do {
    sprintf(str, "%04x", i); Serial.print(str);
    i += 16;
    if (lth > 16) n = 16;  else n = lth;
    for (j = 0; j < n; j++) {
      sprintf(str, " %02x", p[k++]);
      Serial.print(str);
    }
    Serial.println("");
    lth -= n;
  } while (lth);
}

void prregs() {
  PRREG(ENET_PALR);
  PRREG(ENET_PAUR);
  PRREG(ENET_EIR);
  PRREG(ENET_EIMR);
  PRREG(ENET_ECR);
  PRREG(ENET_MSCR);
  PRREG(ENET_MRBR);
  PRREG(ENET_RCR);
  PRREG(ENET_TCR);
  PRREG(ENET_TACC);
  PRREG(ENET_RACC);
  PRREG(ENET_MMFR);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  teensyMAC(mac);
  // start Ethernet and UDP
  Ethernet.begin(mac);
  prregs();
  Udp.begin(localPort);

  Serial.print("IP  address:");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  // udp_echo();
  //udp_send(20,1000);   // blast  pps 1000,8
  // udp_recv();
  // uechosrv();
  //uvdelay(10, 8);
  // udp_ntp();
  //tcp_send();
  tcp_recv();
  // tcp_echo(7);

  delay(5000);   // wait
}
