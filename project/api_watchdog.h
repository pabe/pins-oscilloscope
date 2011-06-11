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

#define API_TASK_QUOTEME_(x) #x
#define API_TASK_QUOTEME(x) API_TASK_QUOTEME_(x)
#define API_TASK_SRC_CURRENT_POS __FILE__":"API_TASK_QUOTEME(__LINE__)

#define ipc_watchdog_signal_error(ERROR) \
  _ipc_watchdog_signal_error(API_TASK_SRC_CURRENT_POS, ERROR)

extern xQueueHandle ipc_watchdog;
typedef enum   msg_watchdog_cmd msg_watchdog_cmd_t;

void _ipc_watchdog_signal_error(const char* path, int err);

portBASE_TYPE ipc_watchdog_set_led_aux(portBASE_TYPE value);

#endif /* __API_WATCHDOG__H_ */
