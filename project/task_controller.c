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

#include "task_watchdog.h"
#include "task_controller.h"
#include "api_controller.h"


/* public variables */

/* private variables */

/* private functions */
static void handle_msg_cmd(msg_controller_cmd_t *cmd);

/* public functions */
void task_controller(void *p)
{
  while(1)
  {
    msg_controller_t msg;

    assert(ipc_controller);
    if(pdFALSE == xQueueReceive(ipc_controller, &msg, portMAX_DELAY))
    {
      /* with no timeouts this should never happen so kill ourself */
      task_watchdog_signal_error();
      vTaskDelete(NULL);
    }

    switch(msg.head_msgtype)
    {
      case controller_msgtype_cmd:
        handle_msg_cmd(&msg.data.cmd);
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
  switch(cmd->cmd)
  {
    case controller_cmd_set_mode_oscilloscope:
      printf("mode: osc|");
      break;

    case controller_cmd_set_mode_multimeter:
      printf("mode: multi|");
      break;

    default:
      /* TODO: Output error mesg? */
      task_watchdog_signal_error();
      vTaskDelete(NULL);
  }
}
