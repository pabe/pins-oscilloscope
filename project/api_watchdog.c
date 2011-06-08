/*
 * api_watchdog:
 *
 * API to interface the watchdog over IPC.
 */

#include "api_watchdog.h"

xQueueHandle ipc_watchdog;

portBASE_TYPE ipc_watchdog_set_led_aux(portBASE_TYPE value)
{
  msg_t msg;
  msg.head.id = msg_id_watchdog_cmd;
  
  msg.data.watchdog_cmd = value ? watchdog_cmd_aux_led_lit : watchdog_cmd_aux_led_quench;
  return xQueueSendToBack(ipc_watchdog, &msg, portMAX_DELAY);
}
