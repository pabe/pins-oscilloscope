/*
 * api:
 *
 * Base API.
 */

#ifndef __API__H_
#define __API__H_

#include <stdint.h>
#include "config.h"
#include "oscilloscope.h"

typedef xQueueHandle ipc_addr_t;

typedef struct
{
  struct
  {
    enum msg_id
    {
      /* 
       * template:
       * msg_id_<name of corresponding variable name in the union>
       */
      msg_id_controller_cmd,
      msg_id_controller_subscribe,
      msg_id_display_toggle_channel,
      msg_id_display_button_highlight,
      msg_id_display_cmd,
      msg_id_watchdog_cmd,
      msg_id_measure_subscribe,
      msg_id_subscribe_mode,
      msg_id_subscribe_measure_data,
      msg_id_subscribe_measure_rate
    } id;
  } head;
  union msg_data
  {
    enum msg_controller_cmd
    {
      controller_cmd_mode_set_oscilloscope,
      controller_cmd_mode_set_multimeter,
      controller_cmd_mode_do_toggle,
      controller_cmd_time_axis_increase,
      controller_cmd_time_axis_decrease
    } controller_cmd;

    struct msg_controller_subscribe
    {
      enum msg_controller_subscribe_variable
      {
        ipc_controller_variable_mode
      } variable;
      ipc_addr_t subscriber;
    } controller_subscribe;
    
    oscilloscope_input_t msg_display_toggle_channel;
    
    int msg_display_button_highlight;

    enum msg_display_cmd
    {
      display_cmd_toggle_freeze_screen
    } display_cmd;

    enum msg_watchdog_cmd
    {
      watchdog_cmd_aux_led_lit,
      watchdog_cmd_aux_led_quench
    } watchdog_cmd;

    struct msg_measure_subscribe
    {
      enum msg_measure_subscribe_variable
      {
        ipc_measure_variable_data_ch0,
        ipc_measure_variable_data_ch1,
        ipc_measure_variable_rate_ch0,
        ipc_measure_variable_rate_ch1
      } variable;
      ipc_addr_t subscriber;
    } measure_subscribe;

    oscilloscope_mode_t subscribe_mode;
    
    struct msg_subscribe_measure_data
    {
      oscilloscope_input_t ch;
      int timestamp;
      uint16_t data;
    } subscribe_measure_data;

    unsigned subscribe_measure_rate;
  } data;
} msg_t;

typedef enum  msg_id   msg_id_t;
typedef union msg_data msg_data_t;

typedef struct
{
  msg_t msg;
  xQueueHandle queues[CONFIG_SUBSCRIBE_MSG_HOOKS];
} subscribe_msg_t;

typedef struct 
{
  enum msg_id id;
  portBASE_TYPE (*handler)(msg_id_t id, msg_data_t *data);
} ipc_loop_t;


portBASE_TYPE ipc_init(void);

/*
 * return:
 * true = timeout
 * false = error
 */
portBASE_TYPE ipc_get(
    ipc_addr_t addr,
    portTickType xTicksToWait,
    const ipc_loop_t handlers[],
    size_t n);
void subscribe_init(subscribe_msg_t *sub, msg_id_t head_id);
void subscribe_execute(subscribe_msg_t *v);
portBASE_TYPE subscribe_add(subscribe_msg_t *v, ipc_addr_t subscriber);

#endif /* __API__H_ */
