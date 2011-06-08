/*
 * api_watchdog:
 *
 * API to interface the controller over IPC.
 */

#include "api_controller.h"

xQueueHandle ipc_controller;

portBASE_TYPE ipc_controller_mode_set(oscilloscope_mode_t mode)
{
  msg_controller_cmd_t cmd;
  switch(mode)
  {
    case oscilloscope_mode_oscilloscope:
      cmd = controller_cmd_set_mode_oscilloscope;
      break;

    case oscilloscope_mode_multimeter:
      cmd = controller_cmd_set_mode_multimeter;
      break;

    default:
      return pdFALSE;
  }

  return ipc_controller_send_cmd(cmd);
}

portBASE_TYPE ipc_controller_subscribe(
    ipc_addr_t subscriber,
    msg_controller_subscribe_variable_t var)
{
  msg_t msg;

  msg.head.id = msg_id_controller_subscribe;
  msg.data.controller_subscribe.variable = var;
  msg.data.controller_subscribe.subscriber = subscriber;

  return xQueueSendToBack(ipc_controller, &msg, portMAX_DELAY);
}

portBASE_TYPE ipc_controller_send_cmd(msg_controller_cmd_t cmd)
{
  msg_t msg;
  msg.head.id = msg_id_controller_cmd;
  msg.data.controller_cmd = cmd;

  assert(ipc_controller);
  return xQueueSendToBack(ipc_controller, &msg, portMAX_DELAY);
}

