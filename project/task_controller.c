/*
 * (TODO: WRITE ME!)
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
#include "ipc.h"
#include "task_controller.h"


/* private functions */
static void update_mode(oscilloscope_mode_t new_mode);
static void update_zoom(void);
static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *cmd);
static portBASE_TYPE handle_msg_subscribe(msg_id_t id, msg_data_t *msg);

/* public variables */
/* private variables */
static uint8_t zoom = 9;
static ipc_subscribe_msg_t mode;
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_controller_cmd,         handle_msg_cmd },
  { msg_id_controller_subscribe,   handle_msg_subscribe }
};

static const ipc_subscribe_table_t ipc_subscribe_table[] =
{
  IPC_SUBSCRIBE_TABLE_INIT(&mode, ipc_controller_variable_mode, controller_subscribe)
};

/* public functions */
void task_controller(void *p)
{
  ipc_subscribe_init(&mode, msg_id_subscribe_mode);
  update_mode(oscilloscope_mode_multimeter);
  update_zoom();

  ipc_measure_subscribe(ipc_controller, ipc_measure_variable_rate);

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
static void update_mode(oscilloscope_mode_t new_mode)
{
  if(mode.msg.data.subscribe_mode != new_mode)
  {
    switch(new_mode)
    {
      case oscilloscope_mode_oscilloscope:
        ipc_watchdog_set_led_aux(0);
        break;

      case oscilloscope_mode_multimeter:
        ipc_watchdog_set_led_aux(1);
        break;
    }
    mode.msg.data.subscribe_mode = new_mode;
    if(NULL != ipc_subscribe_execute(&mode))
    {
      ipc_watchdog_signal_error(0);
    }
  }
}

static void update_zoom(void)
{
  int prescaler;
  int period;

  /* yes, ugly case for this... */
  switch(zoom)
  {
    case 0:
      prescaler = 0;
      period    = 5000;
      break;

    case 1:
      prescaler = 2;
      period    = 2500;
      break;

    case 2:
      prescaler = 4;
      period    = 2500;
      break;

    case 3:
      prescaler = 6;
      period    = 2500;
      break;

    case 4:
      prescaler = 8;
      period    = 2500;
      break;

    case 5:
      prescaler = 10;
      period    = 2500;
      break;

    case 6:
      prescaler = 12;
      period    = 2500;
      break;

    case 7:
      prescaler = 14;
      period    = 2500;
      break;

    case 8:
      prescaler = 16;
      period    = 2500;
      break;

    case 9:
      prescaler = 18;
      period    = 2500;
      break;

    default:
      ipc_watchdog_signal_error(0);
      return;
  }
  
  ipc_measure_cfg_timer(prescaler, period);
}

static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *data)
{
  switch(data->controller_cmd)
  {
    case controller_cmd_mode_set_oscilloscope:
      update_mode(oscilloscope_mode_oscilloscope);
      break;

    case controller_cmd_mode_set_multimeter:
      update_mode(oscilloscope_mode_multimeter);
      break;

    case controller_cmd_mode_do_toggle:
      if(mode.msg.data.subscribe_mode == oscilloscope_mode_multimeter)
      {
        update_mode(oscilloscope_mode_oscilloscope);
      }
      else
      {
        update_mode(oscilloscope_mode_multimeter);
      }
      break;

    case controller_cmd_time_axis_increase:
      if(zoom < 9)
      {
        zoom++;
        update_zoom();
      }
      break;
      
    case controller_cmd_time_axis_decrease:
      if(zoom > 0)
      {
        zoom--;
        update_zoom();
      }

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
