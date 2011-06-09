/*
 * Input driver for touchscreen.
 */

#include <stdio.h>

/* Firmware */
#include "assert.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include "api_input_touch.h"
#include "api_controller.h"
#include "task_input_touch.h"
#include "task_watchdog.h"

/* public variables */

/* private variables */

/* private functions */

/* public functions */
void task_input_touch(void *p)
{
  portTickType timeout = CFG_TASK_INPUT_TOUCH__POLLING_PERIOD;

  /* subscribe to mode variable in the controller, returns pd(TRUE|FALSE) */
  ipc_controller_subscribe(ipc_input_touch, ipc_controller_variable_mode);

  while(1)
  {
    portTickType sleep_time;
    msg_t msg;

    sleep_time = xTaskGetTickCount();
    
    assert(ipc_input_touch);
    if(pdFALSE == xQueueReceive(ipc_input_touch, &msg, timeout))
    {
      /* timeout work */
    }
    else
    {
      switch(msg.head.id)
      {
        case msg_id_subscribe_mode:
          printf("| MODE: %i |", msg.data.subscribe_mode);
          break;

        default:
          /* TODO: Output error mesg? */
          task_watchdog_signal_error();
      }

      /* recalculate timeout */
      timeout = timeout - (xTaskGetTickCount() - sleep_time);
    }
  }
}

/* private functions */
