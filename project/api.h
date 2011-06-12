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

    struct msg_subscribe_measure_rate
    {
      oscilloscope_input_t ch;
      unsigned rate;
    } subscribe_measure_rate;
  } data;
} msg_t;

typedef enum  msg_id   msg_id_t;
typedef union msg_data msg_data_t;

typedef struct
{
  msg_t msg;
  xQueueHandle queues[CONFIG_SUBSCRIBE_MSG_HOOKS];
} subscribe_msg_t;


/* IPC stugg */
#define IPC_OFFSET(MEMBER) \
  (&(((msg_data_t*)0)->MEMBER))

#define IPC_SET_OFFSET(TYPE, SUB, VAR) \
  IPC_OFFSET(TYPE ## . ## SUB), IPC_OFFSET(TYPE ## . ## VAR)

typedef struct 
{
  enum msg_id id;
  portBASE_TYPE (*handler)(msg_id_t id, msg_data_t *data);
} ipc_loop_t;


portBASE_TYPE ipc_init(void);
typedef struct
{
  subscribe_msg_t* var;
  int variable_var;
  void *subscriber;
  void *variable;
} ipc_subscribe_table_t;
#define IPC_SUBSCRIBE_TABLE_INIT(MSG_PTR, VALUE, TYPE) \
  { MSG_PTR, VALUE, IPC_SET_OFFSET(TYPE, subscriber,variable) }

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
portBASE_TYPE ipc_handle_msg_subscribe(
    msg_data_t *msg,
    const ipc_subscribe_table_t table[],
    size_t size);
void subscribe_init(subscribe_msg_t *sub, msg_id_t head_id);
portBASE_TYPE subscribe_execute(subscribe_msg_t *v);
portBASE_TYPE subscribe_add(subscribe_msg_t *v, ipc_addr_t subscriber);

#endif /* __API__H_ */
