// stepl's lwIP 2.0.2, for IDE add -I to boards.txt
// https://forum.pjrc.com/threads/45647-k6x-LAN8720(A)-amp-lwip
// lwiperf

#include "lwip_t41.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
#include "lwip/apps/lwiperf.h"


#define LOG Serial.printf
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
    LOG("netif status changed: ip %s, mask %s, gw %s\n", ip4addr_ntoa_r(netif_ip_addr4(netif), str1, IP4ADDR_STRLEN_MAX), ip4addr_ntoa_r(netif_ip_netmask4(netif), str2, IP4ADDR_STRLEN_MAX), ip4addr_ntoa_r(netif_ip_gw4(netif), str3, IP4ADDR_STRLEN_MAX));
}

static void link_status_callback(struct netif *netif)
{
    LOG("enet link status: %s\n", netif_is_link_up(netif) ? "up" : "down");
}

static void
lwiperf_report(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(local_addr);
  LWIP_UNUSED_ARG(local_port);

  Serial.printf("IPERF report: type=%d, remote: %s:%d, total bytes: %lu, duration in ms: %lu, kbits/s: %lu\n",
    (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port, bytes_transferred, ms_duration, bandwidth_kbitpsec);
}


void setup()
{
  
    Serial.begin(9600);
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

    lwiperf_start_tcp_server_default(lwiperf_report, NULL);
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
