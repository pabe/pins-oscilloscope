/*
 * api_watchdog:
 *
 * API to interface the watchdog over IPC.
 */

#ifndef __API_WATCHDOG__H_
#define __API_WATCHDOG__H_

#include "FreeRTOS.h"
#include "queue.h"

#include "assert.h"
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

__inline portBASE_TYPE ipc_watchdog_init(void);

__inline portBASE_TYPE ipc_watchdog_init(void)
{
  /* we do not init twice! */
  if(ipc_watchdog)
    return pdFALSE;

  ipc_watchdog = xQueueCreate(IPC_QUEUE_LEN_WATCHDOG, sizeof(msg_watchdog_t));

  return ipc_watchdog ? pdTRUE : pdFALSE;
}
#endif /* __API_WATCHDOG__H_ */
