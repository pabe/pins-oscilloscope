/*
 * task_controller:
 *
 * TODO
 */

#ifndef __API_CONTROLLER__H_
#define __API_CONTROLLER__H_

#include "FreeRTOS.h"
#include "queue.h"
#include "assert.h"

#include "api.h"
#include "config.h"
#include "oscilloscope.h"

typedef enum   msg_controller_cmd msg_controller_cmd_t;

extern xQueueHandle ipc_controller;

__inline portBASE_TYPE ipc_controller_set_mode(oscilloscope_mode_t mode);
__inline portBASE_TYPE ipc_controller_send_cmd(msg_controller_cmd_t cmd);

__inline portBASE_TYPE ipc_controller_set_mode(oscilloscope_mode_t mode)
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

__inline portBASE_TYPE ipc_controller_send_cmd(msg_controller_cmd_t cmd)
{
  msg_t msg;
  msg.head.id = msg_id_controller_cmd;
  msg.data.controller_cmd = cmd;

  assert(ipc_controller);
  return xQueueSendToBack(ipc_controller, &msg, portMAX_DELAY);
}

#endif /* __API_CONTROLLER__H_ */
