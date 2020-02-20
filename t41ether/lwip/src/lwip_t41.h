#ifndef __LWIP_T41__
#define __LWIP_T41__

#if defined(ARDUINO_TEENSY41)

#include "include/lwip/ip_addr.h"
#include "include/lwip/pbuf.h"

#ifdef __cplusplus
extern "C" {
#endif
   
typedef void (*rx_frame_fn)(void*);

void enet_init(uint32_t phy_addr, uint8_t *mac, ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void enet_set_rx_callback(rx_frame_fn rx_cb);
struct pbuf* enet_rx_next();
void enet_input(struct pbuf* p_frame);
void enet_proc_input(void);
void enet_poll();

#ifdef __cplusplus
}
#endif

#endif

#endif
