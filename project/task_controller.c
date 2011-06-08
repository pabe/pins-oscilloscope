/**
 * task_controller:
 *
 * TASK (TODO: WRITE ME!)
 */

#include <stdio.h>

/* Firmware */
#include "assert.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"

#include "task_watchdog.h"
#include "task_controller.h"
#include "api_controller.h"


/* public variables */

/* private variables */
static subscribe_msg_t mode;

/* private functions */
static void handle_msg_cmd(msg_controller_cmd_t *cmd);
static void handle_msg_subscribe(msg_controller_subscribe_t *msg);

/* public functions */
void task_controller(void *p)
{
  subscribe_init(&mode, msg_subscribe_mode);
  mode.msg.data.subscribe_mode = oscilloscope_mode_multimeter;

  while(1)
  {
    msg_t msg;

    assert(ipc_controller);
    if(pdFALSE == xQueueReceive(ipc_controller, &msg, portMAX_DELAY))
    {
      /* with no timeouts this should never happen so kill ourself */
      task_watchdog_signal_error();
      vTaskDelete(NULL);
    }

    switch(msg.head.id)
    {
      case msg_id_controller_cmd:
        handle_msg_cmd(&msg.data.controller_cmd);
        break;

      case msg_id_controller_subscribe:
        handle_msg_subscribe(&msg.data.controller_subscribe);
        break;

      default:
        /* TODO: Output error mesg? */
        task_watchdog_signal_error();
        vTaskDelete(NULL);
    }
  }
}

/* private functions */
static void handle_msg_cmd(msg_controller_cmd_t *cmd)
{
  switch(*cmd)
  {
    case controller_cmd_set_mode_oscilloscope:
      mode.msg.data.subscribe_mode = oscilloscope_mode_oscilloscope;
      subscribe_execute(&mode);
      break;

    case controller_cmd_set_mode_multimeter:
      mode.msg.data.subscribe_mode = oscilloscope_mode_multimeter;
      subscribe_execute(&mode);
      break;

    default:
      /* TODO: Output error mesg? */
      task_watchdog_signal_error();
      vTaskDelete(NULL);
  }
}

static void handle_msg_subscribe(msg_controller_subscribe_t *msg)
{
  switch(msg->variable)
  {
    case ipc_controller_variable_mode:
      if(pdFALSE == subscribe_add(&mode, msg->subscriber))
      {
        /* TODO: Output error mesg? */
      }
      break;

    default:
      /* TODO: Output error mesg? */
      task_watchdog_signal_error();
      vTaskDelete(NULL);
  }
}
