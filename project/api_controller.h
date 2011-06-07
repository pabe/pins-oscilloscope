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

#include "config.h"
#include "oscilloscope.h"

typedef enum
{
  controller_cmd_set_mode_oscilloscope,
  controller_cmd_set_mode_multimeter
} controller_cmd_t;

typedef enum
{
  controller_msgtype_cmd
} controller_msgtypes_t;

typedef struct
{
  controller_cmd_t cmd;
} msg_controller_cmd_t;

typedef struct
{
  controller_msgtypes_t head_msgtype;
  union
  {
    msg_controller_cmd_t cmd;
  } data;
} msg_controller_t;

extern xQueueHandle ipc_controller;

__inline portBASE_TYPE ipc_controller_init(void);
__inline portBASE_TYPE ipc_controller_set_mode(oscilloscope_mode_t mode);
__inline portBASE_TYPE ipc_controller_send_cmd(controller_cmd_t cmd);

__inline portBASE_TYPE ipc_controller_init(void)
{
  /* we do not init twice! */
  if(ipc_controller)
    return pdFALSE;

  ipc_controller = xQueueCreate(IPC_QUEUE_LEN_CONTROLLER, sizeof(msg_controller_t));
  return ipc_controller ? pdTRUE : pdFALSE;
}

__inline portBASE_TYPE ipc_controller_set_mode(oscilloscope_mode_t mode)
{
  controller_cmd_t cmd;
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

__inline portBASE_TYPE ipc_controller_send_cmd(controller_cmd_t cmd)
{
  msg_controller_t msg;
  msg.head_msgtype = controller_msgtype_cmd;
  msg.data.cmd.cmd = cmd;

  assert(ipc_controller);
  return xQueueSendToBack(ipc_controller, &msg, portMAX_DELAY);
}

#endif /* __API_CONTROLLER__H_ */
