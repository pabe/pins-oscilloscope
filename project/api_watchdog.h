/*
 * Thread-safe API-interface for the watchdog.
 */

#ifndef __API_WATCHDOG__H__
#define __API_WATCHDOG__H__

#include "FreeRTOS.h"
#include "queue.h"

#include "assert.h"
#include "config.h"
#include "api.h"

/* private macros, these should not be used outside watchdog */
#if CONFIG_ENABLE_PRINTER
# define API_WATCHDOG_QUOTEME_(x) #x
# define API_WATCHDOG_QUOTEME(x) API_WATCHDOG_QUOTEME_(x)
# define API_WATCHDOG_SRC_CURRENT_POS __FILE__":"API_WATCHDOG_QUOTEME(__LINE__)
#endif

/* public macros */
#if CONFIG_ENABLE_PRINTER
# define ipc_watchdog_signal_error(ERROR) \
  _ipc_watchdog_signal_error(API_WATCHDOG_SRC_CURRENT_POS, ERROR)
#else
# define ipc_watchdog_signal_error(ERROR) \
  _ipc_watchdog_signal_error("<unknown>", ERROR)
#endif

extern xQueueHandle ipc_watchdog;
typedef enum   msg_watchdog_cmd msg_watchdog_cmd_t;

void _ipc_watchdog_signal_error(const char* path, int err);

portBASE_TYPE ipc_watchdog_set_led_aux(portBASE_TYPE value);

#endif /* __API_WATCHDOG__H__ */
