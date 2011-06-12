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

#include "api_watchdog.h"
#include "api_controller.h"
#include "api_measure.h"
#include "task_controller.h"


/* private functions */
static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *cmd);
static portBASE_TYPE handle_msg_subscribe_measure_rate(msg_id_t id, msg_data_t *cmd);
static portBASE_TYPE handle_msg_subscribe(msg_id_t id, msg_data_t *msg);

/* public variables */
/* private variables */
static subscribe_msg_t mode;
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_controller_cmd,         handle_msg_cmd },
  { msg_id_subscribe_measure_rate, handle_msg_subscribe_measure_rate },
  { msg_id_controller_subscribe,   handle_msg_subscribe }
};

static const ipc_subscribe_table_t ipc_subscribe_table[] =
{
  IPC_SUBSCRIBE_TABLE_INIT(&mode, ipc_controller_variable_mode, controller_subscribe)
};

/* public functions */
void task_controller(void *p)
{
  subscribe_init(&mode, msg_id_subscribe_mode);
  mode.msg.data.subscribe_mode = oscilloscope_mode_multimeter;

  ipc_measure_subscribe(ipc_controller, ipc_measure_variable_rate_ch0);
  ipc_measure_subscribe(ipc_controller, ipc_measure_variable_rate_ch1);

  while(1)
  {
    if(pdTRUE == ipc_get(
          ipc_controller,
          portMAX_DELAY,
          msg_handle_table,
          sizeof(msg_handle_table)/sizeof(msg_handle_table[0])))
    {
      /* with no timeouts this should never happen so kill ourself */
      ipc_watchdog_signal_error(0);
    }
    else
    {    
      ipc_watchdog_signal_error(0);
    }
  }
}

/* private functions */
static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *data)
{
  switch(data->controller_cmd)
  {
    case controller_cmd_mode_set_oscilloscope:
      if(mode.msg.data.subscribe_mode != oscilloscope_mode_oscilloscope)
      {
        mode.msg.data.subscribe_mode = oscilloscope_mode_oscilloscope;
        subscribe_execute(&mode);
      }
      break;

    case controller_cmd_mode_set_multimeter:
      if(mode.msg.data.subscribe_mode != oscilloscope_mode_multimeter)
      {
        mode.msg.data.subscribe_mode = oscilloscope_mode_multimeter;
        subscribe_execute(&mode);
      }
      break;

    case controller_cmd_mode_do_toggle:
      if(mode.msg.data.subscribe_mode == oscilloscope_mode_multimeter)
      {
        mode.msg.data.subscribe_mode = oscilloscope_mode_oscilloscope;
      }
      else
      {
        mode.msg.data.subscribe_mode = oscilloscope_mode_multimeter;
      }
      subscribe_execute(&mode);
      break;

    case controller_cmd_time_axis_increase:
      printf("|C: AXIS_INC| ");
      break;
      
    case controller_cmd_time_axis_decrease:
      printf("|C: AXIS_DEC| ");
      break;
      
    default:
      return pdFALSE;
  }
  return pdTRUE;
}

static portBASE_TYPE handle_msg_subscribe_measure_rate(msg_id_t id, msg_data_t *data)
{
  switch(data->subscribe_measure_rate.ch)
  {
    case input_channel0:
    case input_channel1:
      printf("|C: RATE(%i)=%u| ",
          data->subscribe_measure_rate.ch,
          data->subscribe_measure_rate.rate);
      break;

    default:
      return pdFALSE;
  }

  return pdTRUE;
}

static portBASE_TYPE handle_msg_subscribe(msg_id_t id, msg_data_t *msg)
{
  return ipc_handle_msg_subscribe(
      msg,
      ipc_subscribe_table, 
      sizeof(ipc_subscribe_table)/sizeof(ipc_subscribe_table[0]));
}
