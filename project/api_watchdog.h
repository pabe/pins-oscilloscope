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
#include "api.h"

extern xQueueHandle ipc_watchdog;
typedef enum   msg_watchdog_cmd msg_watchdog_cmd_t;

portBASE_TYPE ipc_watchdog_set_led_aux(portBASE_TYPE value);

#endif /* __API_WATCHDOG__H_ */
