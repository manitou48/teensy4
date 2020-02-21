// stepl's lwIP 2.0.2, for IDE add -I to boards.txt
// https://forum.pjrc.com/threads/45647-k6x-LAN8720(A)-amp-lwip


#include "lwip_t41.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"

#define PHY_ADDR 0 /*for read/write PHY registers (check link status,...)*/
#define DHCP 0
#define IP "192.168.1.19"
#define MASK "255.255.255.0"
#define GW "192.168.1.1"

#define DNS1 "192.168.1.1"

#define LOG Serial.printf



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



#pragma region lwip

static void netif_status_callback(struct netif *netif)
{
  static char str1[IP4ADDR_STRLEN_MAX], str2[IP4ADDR_STRLEN_MAX], str3[IP4ADDR_STRLEN_MAX];
  LOG("netif status changed: ip %s, mask %s, gw %s\n", ip4addr_ntoa_r(netif_ip_addr4(netif), str1, IP4ADDR_STRLEN_MAX), ip4addr_ntoa_r(netif_ip_netmask4(netif), str2, IP4ADDR_STRLEN_MAX), ip4addr_ntoa_r(netif_ip_gw4(netif), str3, IP4ADDR_STRLEN_MAX));
}

static void link_status_callback(struct netif *netif)
{
  LOG("enet link status: %s\n", netif_is_link_up(netif) ? "up" : "down");
}

#pragma endregion

static  ip_addr_t dnsaddr;

static void sntp_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
  LWIP_UNUSED_ARG(hostname);
  LWIP_UNUSED_ARG(arg);

  if (ipaddr != NULL) {
    /* Address resolved, do something */
    //LOG("resolved %s\n", inet_ntoa(*ipaddr));
    dnsaddr = *ipaddr;

  } else {
    /* DNS resolving failed  */
    Serial.println("DNS failed");
  }
}

char * hosts[] = {"tnlandforms.us", "google.com", NULL};
void do_dns() {
  err_t err;
  uint32_t t;
  static int cnt = 0;

  while (1) {
    LOG("host %s\n", hosts[cnt]);
    ip_addr_set_zero(&dnsaddr);

    err = dns_gethostbyname(hosts[cnt], &dnsaddr, sntp_dns_found, NULL);
    if (err == ERR_INPROGRESS) {
      /* DNS request sent, wait for sntp_dns_found being called */
      while ( dnsaddr.addr == 0) loop();  // timeout ?
      LOG("DNS x %s\n", inet_ntoa(dnsaddr));
    } else if (err == ERR_OK) {
      LOG("DNS ok %s\n", inet_ntoa(dnsaddr));
    }
    t = millis();
    while ( millis() - t < 5000); loop(); // ether friendly delay
    cnt++;
    if (hosts[cnt] == NULL) cnt = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(100);

  LOG("PHY_ADDR %d\n", PHY_ADDR);
  uint8_t mac[6];
  teensyMAC(mac);
  LOG("MAC_ADDR %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  LOG("DHCP is %s\n", DHCP == 1 ? "on" : "off");

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

#ifdef DNS1
  //  need LWIP_DNS and LWIP_UDP
  static ip_addr_t dns1;
  inet_aton(DNS1, &dns1);
  dns_setserver(0, &dns1);
#endif
  do_dns();
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
