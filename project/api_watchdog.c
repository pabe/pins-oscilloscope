/*
 * api_watchdog:
 *
 * API to interface the watchdog over IPC.
 */

#include "api_watchdog.h"

xQueueHandle ipc_watchdog;

/* shared memory with the task */
extern void task_watchdog_signal_error(void);

void _ipc_watchdog_signal_error(const char* path, int err)
{
  printf("ERROR(%i)@%s   ", err, path);
  task_watchdog_signal_error();
}

portBASE_TYPE ipc_watchdog_set_led_aux(portBASE_TYPE value)
{
  msg_t msg;
  portBASE_TYPE ret;

  msg.head.id = msg_id_watchdog_cmd;
  msg.data.watchdog_cmd = value ? watchdog_cmd_aux_led_lit : watchdog_cmd_aux_led_quench;
  
  assert(ipc_watchdog);
  ret = xQueueSendToBack(ipc_watchdog, &msg, CONFIG_IPC_WAIT);
  assert(ret == pdTRUE);
  return ret;
}
