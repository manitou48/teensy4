// lwip multicast chat
// to use IDE hack -I into boards.txt
#include "lwip_t41.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/udp.h"
#include "lwip/igmp.h"

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

#define MGROUP "224.7.8.9"
#define MPORT 7654

// reverse of crc32 ethernet (04C11DB7)  0xEDB88320
// used by k64 for multicast mac hash, could use hardware CRC

uint32_t crc32(uint8_t *address, uint32_t bytes) {
  uint32_t crc = 0xFFFFFFFFU;
  uint32_t count1 = 0;
  uint32_t count2 = 0;

  /* Calculates the CRC-32 polynomial on the multicast group address. */
  for (count1 = 0; count1 < bytes; count1++)
  {
    uint8_t c = address[count1];
    for (count2 = 0; count2 < 0x08U; count2++)
    {
      if ((c ^ crc) & 1U)
      {
        crc >>= 1U;
        c >>= 1U;
        crc ^= 0xEDB88320U;
      }
      else
      {
        crc >>= 1U;
        c >>= 1U;
      }
    }
  }
  return crc;
}

void mcast_init() {
  // do igmp and MAC input filter
  uint8_t mac[6];
  uint8_t *ip;
  uint32_t crc;
  ip_addr_t mgroup;

  inet_aton(MGROUP, &mgroup);
  ip = (uint8_t *) &mgroup.addr;
  mac[0] = 1;
  mac[1] = 0;
  mac[2] = 0x5e;
  mac[3] = ip[1] & 0x7f;
  mac[4] = ip[2];
  mac[5] = ip[3];
  crc = crc32(mac, 6);
  // join group, set bit in 64-bit hash (2 regs)
  igmp_joingroup(IP_ADDR_ANY, &mgroup);
  // leave group need to clear bit
  int hash = (crc >> 26) & 0x3f;   // 6 bit hash
  Serial.print("crc "); Serial.print(crc, HEX); Serial.print(" hash ");
  Serial.println(hash, HEX);
  if (hash & 0x20) ENET_GAUR |= 1 << (hash & 0x1f);
  else ENET_GALR |= 1 << (hash & 0x1f);

  //  ENET_RCR |= ENET_RCR_PROM; // hack in promisuous
  //    ENET_GAUR = ENET_GALR = 0xffffffff;   // hack all hashes
  //  ENET_GAUR = 0; ENET_GALR = 0x00000040;  // hack works for 224.7.8.9
}

// UDP callbacks
//  fancy would be pbuf recv_q per UDP pcb, if q full, drop and free pbuf
void udp_callback(void * arg, struct udp_pcb * upcb, struct pbuf * p, const ip_addr_t * addr, u16_t port)
{
  if (p == NULL) return;
  Serial.print(">>>>from ");
  Serial.print(ipaddr_ntoa(addr));
  Serial.print(": ");
  Serial.print(p->tot_len); Serial.println(" bytes");
  Serial.println((char *)(p->payload));
  // dump((uint8_t *)p->payload,64);
  pbuf_free(p);
}


void udp_listen() {
  struct udp_pcb *pcb;

  pcb = udp_new();
  udp_bind(pcb, IP_ADDR_ANY, MPORT);    // local port
  udp_recv(pcb, udp_callback, NULL);  // do once?
  // fall into loop  ether_poll
}

void udp_chirp(char *msg, int bytes) {
  struct udp_pcb *pcb;
  pbuf *p;
  ip_addr_t server;

  inet_aton(MGROUP, &server);
  pcb = udp_new();
  p = pbuf_alloc(PBUF_TRANSPORT, bytes, PBUF_RAM);
  memcpy(p->payload, msg, bytes);
  udp_sendto(pcb, p, &server, MPORT);
  pbuf_free(p);
}


void setup()
{
  Serial.begin(9600);
  while (!Serial) delay(100);

  Serial.println(); Serial.print(F_CPU); Serial.print(" ");
  Serial.print(__TIME__); Serial.print(" "); Serial.println(__DATE__);
  Serial.println("multicast chat");
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
  mcast_init();
  udp_listen();
  udp_chirp("chirp", 5);  // could schedule a chirp every 5s
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
