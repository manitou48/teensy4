// lwip perf
// to use IDE hack -I into boards.txt
#include "lwip_t41.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/stats.h"

#define swap2 __builtin_bswap16
#define swap4 __builtin_bswap32

uint32_t rtt;

#define PHY_ADDR 0 /*for read/write PHY registers (check link status,...)*/
#define DHCP 0
#define IP "192.168.1.19"
#define MASK "255.255.255.0"
#define GW "192.168.1.1"

// debug stats stuff
extern "C" {
#if LWIP_STATS
  struct stats_ lwip_stats;
#endif
}

void print_stats() {
  // lwip stats_display() needed printf
#if LWIP_STATS
  char str[128];

  // my  LINK stats
  sprintf(str, "LINK in %d out %d drop %d memerr %d",
          lwip_stats.link.recv, lwip_stats.link.xmit, lwip_stats.link.drop, lwip_stats.link.memerr);
  Serial.println(str);
  sprintf(str, "TCP in %d out %d drop %d memerr %d",
          lwip_stats.tcp.recv, lwip_stats.tcp.xmit, lwip_stats.tcp.drop, lwip_stats.tcp.memerr);
  Serial.println(str);
  sprintf(str, "UDP in %d out %d drop %d memerr %d",
          lwip_stats.udp.recv, lwip_stats.udp.xmit, lwip_stats.udp.drop, lwip_stats.udp.memerr);
  Serial.println(str);
  sprintf(str, "ICMP in %d out %d",
          lwip_stats.icmp.recv, lwip_stats.icmp.xmit);
  Serial.println(str);
  sprintf(str, "ARP in %d out %d",
          lwip_stats.etharp.recv, lwip_stats.etharp.xmit);
  Serial.println(str);
#if MEM_STATS
  sprintf(str, "HEAP avail %d used %d max %d err %d",
          lwip_stats.mem.avail, lwip_stats.mem.used, lwip_stats.mem.max, lwip_stats.mem.err);
  Serial.println(str);
#endif
#endif
}

#define PRREG(x) Serial.printf(#x" 0x%x\n",x)

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



static void teensyMAC(uint8_t *mac)
{
  uint8_t serial[4];
  
  mac[0] = 0x04;
  mac[1] = 0xE9;
  mac[2] = 0xE5;
  mac[3] = 1;
  mac[4] = 2;
  mac[5] = 3;
}



static void netif_status_callback(struct netif *netif)
{
  static char str1[IP4ADDR_STRLEN_MAX], str2[IP4ADDR_STRLEN_MAX], str3[IP4ADDR_STRLEN_MAX];
  Serial.printf("netif status changed: ip %s, mask %s, gw %s\n", ip4addr_ntoa_r(netif_ip_addr4(netif), str1, IP4ADDR_STRLEN_MAX), ip4addr_ntoa_r(netif_ip_netmask4(netif), str2, IP4ADDR_STRLEN_MAX), ip4addr_ntoa_r(netif_ip_gw4(netif), str3, IP4ADDR_STRLEN_MAX));
}

static void link_status_callback(struct netif *netif)
{
  Serial.printf("enet link status: %s\n", netif_is_link_up(netif) ? "up" : "down");
}

// UDP callbacks
//  fancy would be pbuf recv_q per UDP pcb, if q full, drop and free pbuf

static volatile int udp_bytes, udp_pkts;

// udp recv callback
static void udp_callback(void * arg, struct udp_pcb * upcb, struct pbuf * p, const  ip_addr_t * addr, u16_t port)
{
  if (p == NULL) return;
  udp_bytes +=  p->tot_len;
  udp_pkts++;
  pbuf_free(p);
}

void ntp_callback(void * arg, struct udp_pcb * upcb, struct pbuf * p,  const ip_addr_t * addr, u16_t port)
{
  uint32_t secs, nus, us;
  static uint32_t us0 = 0, secs0, nus0;

  us = micros();
  if (p == NULL) return;
  rtt = us - rtt;
  if (p->tot_len == 48) {
    uint32_t secs = swap4(((uint32_t *) p->payload)[10]); // NTP secs
    uint64_t x = swap4(((uint32_t *) p->payload)[11]);  // NTP frac secs
    nus = (1000000 * x) >> 32;
    if (us0 == 0) {
      us0 = us;
      secs0 = secs;
      nus0 = nus;
    }
    float t = 1000000 * (secs - secs0) + (nus - nus0); // elapsed remote us
    Serial.printf("ntp %u.%06d  rtt %d us   %.0f ppm over %d s\n",
                  secs, nus, rtt, (1000000. * ((us - us0) - t)) / t, secs - secs0);
  }
  pbuf_free(p);
}

void udp_echo(int pkts, int bytes) {
  int i, prev = 0;
  struct udp_pcb *pcb;
  pbuf *p;
  uint32_t t, ms;
  ip_addr_t server;

  inet_aton("192.168.1.4", &server);
  pcb = udp_new();
  udp_bind(pcb, IP_ADDR_ANY, 4444);    // local port
  udp_recv(pcb, udp_callback, NULL /* *arg */);

  for (i = 0; i < pkts; i++) {
    p = pbuf_alloc(PBUF_TRANSPORT, bytes, PBUF_RAM);
    t = micros();
    udp_sendto(pcb, p, &server, 7654);
    while (udp_bytes <= prev) loop(); // wait for reply
    t = micros() - t;
    prev = udp_bytes;
    pbuf_free(p);
    Serial.print(t); Serial.print(" us  "); Serial.println(udp_bytes);
    ms = millis(); // ether delay
    while (millis() - ms < 2000) loop();
  }

  pbuf_free(p);
  udp_remove(pcb);
}

void udp_ntp(int pkts) {
  int i;
  struct udp_pcb *pcb;
  pbuf *p;
  uint32_t ms;
  ip_addr_t server;

  inet_aton("192.168.1.4", &server);
  pcb = udp_new();
  udp_bind(pcb, IP_ADDR_ANY, 4444);    // local port
  udp_recv(pcb, ntp_callback, NULL /* *arg */);  // do once?
  for (i = 0; i < pkts; i++) {
    p = pbuf_alloc(PBUF_TRANSPORT, 48, PBUF_RAM);  // need each time?
    *(uint8_t *)p->payload = 0x1b;    // NTP query
    rtt = micros();
    udp_sendto(pcb, p, &server, 123);
    pbuf_free(p);
    ms = millis(); // ether delay
    while (millis() - ms < 10000) loop();
  }

  pbuf_free(p);
  udp_remove(pcb);
}

void udp_sink() {
  int pkts = 0, prev = 0;
  struct udp_pcb *pcb;
  uint32_t t, t0 = 0, ms = 0;

  Serial.println("udp sink on port 8888");  //use udpsrc for rate-based send
  pcb = udp_new();
  udp_bind(pcb, IP_ADDR_ANY, 8888);    // local port
  udp_recv(pcb, udp_callback, NULL);  // do once?
  while (1) {
    while (udp_bytes <= prev) {
      loop();  // wait for incoming
      if (ms && (millis() - ms > 2000)) { // timeout
        char str[64];
        t = t - t0;
        sprintf(str, "%d pkts %d bytes %d us ", pkts, udp_bytes, t);
        Serial.print(str);
        Serial.println(8.*udp_bytes / t);
        udp_remove(pcb);
        return;
      }
    }
    t = micros();
    if (t0 == 0) {
      t0 = t;
      ms = millis();
    }
    pkts++;
    prev = udp_bytes;
  }

}

// blast needs to be primed with a few echo pkts, since doesn't poll so ARP will fail
//  use udpsink on desktop
void udp_blast(int pkts, int bytes) {
  int i;
  struct udp_pcb *pcb;
  pbuf *p;
  uint32_t t;
  ip_addr_t server;

  Serial.println("UDP blast");
  delay(1000);
  inet_aton("192.168.1.4", &server);
  pcb = udp_new();
  udp_bind(pcb, IP_ADDR_ANY, 3333);    // local port
  t = micros();
  for (i = 0; i < pkts; i++) {
    p = pbuf_alloc(PBUF_TRANSPORT, bytes, PBUF_RAM);  // ? in the loop
    *(uint32_t *)(p->payload) = swap4(i);  // seq number
    udp_sendto(pcb, p, &server, 2000);
    pbuf_free(p);
    //    delay(100);  // rate limit, need poll/delay ?
  }
  t = micros() - t;
  Serial.println(t);

  pbuf_free(p);
  udp_remove(pcb);
}


//   ----- TCP ------

void tcperr_callback(void * arg, err_t err)
{
  // set with tcp_err()
  Serial.print("TCP err "); Serial.println(err);
  *(int *)arg = err;
}

err_t connect_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
  Serial.print("connected "); Serial.println(tcp_sndbuf(tpcb));
  *(int *)arg = 1;
  return 0;
}

void tcptx(int pkts) {
  // send to ttcp -r -s
  char buff[1000];
  ip_addr_t server;
  struct tcp_pcb * pcb;
  int i, connected = 0;
  err_t err;
  uint32_t t, sendqlth;

  Serial.println("tcptx");
  inet_aton("192.168.1.4", &server);
  pcb = tcp_new();
  tcp_err(pcb, tcperr_callback);
  tcp_arg(pcb, &connected);
  tcp_bind(pcb, IP_ADDR_ANY, 3333);   // local port
  sendqlth = tcp_sndbuf(pcb);
  Serial.println(sendqlth);
  do {
    err = tcp_connect(pcb, &server, 5001, connect_callback);
    //Serial.print("err ");Serial.println(err);
    loop();
  } while (err < 0);
  while (!connected) loop();
  if (connected < 0) {
    Serial.println("connect error");
    return;  // err
  }
  t = micros();
  for (i = 0; i < pkts; i++) {
    do {
      err = tcp_write(pcb, buff, sizeof(buff), TCP_WRITE_FLAG_COPY);
      loop();   // keep checkin while we blast
    } while ( err < 0);  // -1 is ERR_MEM
    tcp_output(pcb);
  }
  while (tcp_sndbuf(pcb) != sendqlth) loop(); // wait til sent
  tcp_close(pcb);
  t = micros() - t;
  Serial.print(t); Serial.print(" us  "); Serial.println(8.*pkts * sizeof(buff) / t);
}

static  struct tcp_pcb * pcbl;   // listen
static struct tcp_pcb * pcba;   // accepted pcb
volatile bool tcprx_running = true;

void listen_err_callback(void * arg, err_t err)
{
  // set with tcp_err()
  Serial.print("TCP listen err "); Serial.println(err);
  *(int *)arg = err;
}

err_t accept_callback(void * arg, struct tcp_pcb * newpcb, err_t err) {
  if (err || !newpcb) {
    Serial.print("accept err "); Serial.println(err);
    delay(100);
    return 1;
  }
  Serial.println("accepted");
  tcp_accepted(pcbl);    // ref says use listen pcb
  pcba = newpcb;    // let tcprx proceed
  return 0;
}

err_t recv_callback(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err)
{
  static uint32_t t0 = 0, t, bytes = 0;

  t = micros();
  if (!t0) t0 = t;
  if (p == NULL) {
    // other end closed
    t = t - t0;
    Serial.println("remote closed");
    Serial.print(bytes); Serial.print(" ");
    Serial.print(t); Serial.print(" us   ");
    Serial.println(8.*bytes / t);

    tcp_close(tpcb);
    tcprx_running = false;
    return 0;
  }
  tcp_recved(tpcb, p->tot_len);  // data processed
  bytes += p->tot_len;
  pbuf_free(p);
  return 0;
}

void tcprx() {
  struct tcp_pcb * pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 5001); // server port
  pcbl = tcp_listen(pcb);   // pcb deallocated
  tcp_err(pcbl, listen_err_callback);
  while (1) {
    Serial.println("server listening on 5001");
    pcba = NULL;    // accept PCB
    tcp_accept(pcbl, accept_callback);
    // uint32_t prev = inpkts;
    uint32_t ms = millis();
    while (pcba == NULL)  {
      if (millis() - ms > 60000) {
        Serial.println("accept timedout");
        tcp_close(pcbl);
        return;
      }
      //   if (inpkts != prev) { Serial.print("inpkts "); Serial.println(inpkts);
      //     prev=inpkts;}
      loop();   // waiting connection
    }
    tcp_err(pcba, tcperr_callback);
    tcp_recv(pcba, recv_callback);  // all the action is now in callback
    ms = millis();
    while (tcprx_running) {
      if (millis() - ms > 5000) {
        Serial.println(" timedout");
        return;
      }
      loop();  // wait til close
    }
    tcprx_running = true;
  }
  Serial.println("fall through");
  tcp_close(pcbl);
  // fall through to main ether_poll loop ....
}

void setup()
{
  Serial.begin(9600);
  while (!Serial) delay(100);

  Serial.println(); Serial.print(F_CPU); Serial.print(" ");

  Serial.print(__TIME__); Serial.print(" "); Serial.println(__DATE__);
  Serial.printf("PHY_ADDR %d\n", PHY_ADDR);
  uint8_t mac[6];
  teensyMAC(mac);
  Serial.printf("MAC_ADDR %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.printf("DHCP is %s\n", DHCP == 1 ? "on" : "off");

  ip_addr_t ip, mask, gateway;
  if (DHCP == 1)
  {
    ip = IPADDR4_INIT(IPADDR_ANY);
    mask = IPADDR4_INIT(IPADDR_ANY);
    gateway = IPADDR4_INIT(IPADDR_ANY);
  }
  else
  {
    inet_aton(IP, &ip);
    inet_aton(MASK, &mask);
    inet_aton(GW, &gateway);
  }
  enet_init(PHY_ADDR, mac, &ip, &mask, &gateway);
  netif_set_status_callback(netif_default, netif_status_callback);
  netif_set_link_callback(netif_default, link_status_callback);
  netif_set_up(netif_default);

  if (DHCP == 1)
    dhcp_start(netif_default);

  while (!netif_is_link_up(netif_default)) loop(); // await on link up
  prregs();

  //  pick a test
  //udp_sink();
  // udp_echo(10, 8);
  //udp_echo(2, 8); udp_blast(20, 1000); // blast needs echo to run first ?
  udp_ntp(100);   // NTP query and drift check
  //tcptx(100);
  //tcprx();

#if 0
  // optional stats every 5 s, need LWIP_STATS 1 lwipopts.h
  while (1) {
    static uint32_t ms = millis();
    if (millis() - ms > 5000) {
      ms = millis();
      print_stats();
    }
    loop();  // poll
  }
#endif
}

void loop()
{
  static uint32_t last_ms;
  uint32_t ms;

  enet_proc_input();

  ms = millis();
  if (ms - last_ms > 100)
  {
    last_ms = ms;
    enet_poll();
  }
}
