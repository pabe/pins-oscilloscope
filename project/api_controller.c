/*
 * api_watchdog:
 *
 * API to interface the controller over IPC.
 */

#include "assert.h"
#include "api_controller.h"

/* public variables */
xQueueHandle ipc_controller;

/* private variables */

/* private functions */
__inline static 
portBASE_TYPE ipc_controller_send_cmd(msg_controller_cmd_t cmd);

/* public functions */
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

portBASE_TYPE ipc_controller_toggle_mode(void)
{
  return ipc_controller_send_cmd(controller_cmd_toggle_mode);
}

portBASE_TYPE ipc_controller_toggle_time_axis_increase(void)
{
  return pdFALSE;
}

portBASE_TYPE ipc_controller_toggle_time_axis_decrease(void)
{
  return pdFALSE;
}

portBASE_TYPE ipc_controller_toggle_channel0_subbscribe(void)
{
  return pdFALSE;
}

portBASE_TYPE ipc_controller_toggle_channel1_subbscribe(void)
{
  return pdFALSE;
}


portBASE_TYPE ipc_controller_toggle_freeze_screen(void)
{
  return pdFALSE;
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

/* private functions */
__inline static
portBASE_TYPE ipc_controller_send_cmd(msg_controller_cmd_t cmd)
{
  msg_t msg;
  msg.head.id = msg_id_controller_cmd;
  msg.data.controller_cmd = cmd;

  assert(ipc_controller);
  return xQueueSendToBack(ipc_controller, &msg, portMAX_DELAY);
}
