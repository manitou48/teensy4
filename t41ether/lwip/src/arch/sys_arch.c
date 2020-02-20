#include <stdint.h>
extern volatile uint32_t systick_millis_count;
/* Returns the current time in mS. This is needed for the LWIP timers */
uint32_t sys_now(void) {
  return (uint32_t) systick_millis_count;
}
