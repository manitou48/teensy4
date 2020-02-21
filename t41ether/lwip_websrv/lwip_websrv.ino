// lwip webserv
// to use IDE hack -I into boards.txt
#include "lwip_t41.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#define swap2 __builtin_bswap16
#define swap4 __builtin_bswap32

#define PHY_ADDR 0 /*for read/write PHY registers (check link status,...)*/
#define DHCP 0
#define IP "192.168.1.19"
#define MASK "255.255.255.0"
#define GW "192.168.1.1"

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

#define LEDpin 13

void tcperr_callback(void * arg, err_t err)
{
  // set with tcp_err()
  Serial.print("TCP err "); Serial.println(err);
  *(int *)arg = err;
}


static  struct tcp_pcb * pcbl;   // listen pcb
static int sendqlth;


// web page
char *msg1 = "HTTP/1.0 200 OK\nContent-type:text/html\n\n"
             "<H1>K64/K66 web server</H1>\n";
char *msg2 = "<br>Click <a href=\"/LEDON\">here</a> turn the LED on"
             "<br>Click <a href=\"/LEDOFF\">here</a> turn the LED off\n";

void web_close(struct tcp_pcb * tpcb) {
  Serial.println("web close");
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_close(tpcb);
}

void listen_err_callback(void * arg, err_t err)
{
  // set with tcp_err()
  Serial.print("TCP listen err "); Serial.println(err);
  *(int *)arg = err;
}

err_t sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len) {
  // invoked before last send, await ACKs from sent segments

  if (tcp_sndbuf(tpcb) == sendqlth) web_close(tpcb);  // nothing q'd close
  return 0;
}

void tcp_sendstr(struct tcp_pcb * tpcb, char *str) {
  // we're not expecting write to fail for our small web reply < sendqlth

  tcp_write(tpcb, str, strlen(str), TCP_WRITE_FLAG_COPY); // PUSH
  tcp_output(tpcb);  // force ? necessary
}

err_t recv_callback(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err)
{
  char str[128];
  int i;
  static int pkts = 0;

  if (p == NULL) {
    // other end closed
    Serial.println("remote closed");

    web_close(tpcb);
    return 0;
  }

  pkts++;
  String line = "";
  Serial.print(pkts); Serial.print(" pkts  "); Serial.println(p->tot_len);
  Serial.println();
  for (i = 0; i < p->tot_len; i++) line += ((char *)p->payload)[i];
  Serial.println(line);
  // parse query
  if (line.indexOf("GET /LEDON") != -1) digitalWrite(LEDpin, HIGH);
  if (line.indexOf("GET /LEDOFF") != -1) digitalWrite(LEDpin, LOW);

  tcp_recved(tpcb, p->tot_len);  // data processed
  pbuf_free(p);

  tcp_sendstr(tpcb, msg1);
  tcp_sendstr(tpcb, msg2);
  sprintf(str, "<hr>\n uptime %lu ms\n\n", millis());
  tcp_sent(tpcb, sent_callback);   // wait for last then close
  tcp_sendstr(tpcb, str);
  return 0;
}

err_t accept_callback(void * arg, struct tcp_pcb * newpcb, err_t err) {
  if (err || !newpcb) {
    Serial.print("accept err "); Serial.println(err);
    delay(10);
    return 1;
  }
  Serial.println("accepted");
  sendqlth = tcp_sndbuf(newpcb) + 1;     //5840
  tcp_recv(newpcb, recv_callback);
  tcp_err(newpcb, tcperr_callback);
  //  tcp_accepted(pcbl);   // ref says the listen pcb
  return 0;
}

void websrv() {
  struct tcp_pcb * pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 80); // server port
  pcbl = tcp_listen(pcb);   // pcb deallocated
  tcp_err(pcbl, listen_err_callback);

  Serial.println("web server listening on 80");

  tcp_accept(pcbl, accept_callback);

  // fall through to main ether_poll loop ....
}


void setup()
{
  Serial.begin(9600);
  while (!Serial) delay(100);
  pinMode(LEDpin,OUTPUT);

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
  websrv();
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
