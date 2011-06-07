/**
 * api_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 * (For now a pure SW that toogles a LED.)
 */

#ifndef __API_WATCHDOG_H_
#define __API_WATCHDOG_H_

#include "queue.h"

#include "config.h"

typedef enum
{
  watchdog_cmd_aux_led_lit,
  watchdog_cmd_aux_led_quench
} watchdog_cmd_t;

typedef struct
{
  watchdog_cmd_t cmd;
} msg_watchdog_t;

extern xQueueHandle ipc_watchdog;

portBASE_TYPE ipc_watchdog_init(void);

#endif /* __API_WATCHDOG_H_ */
