/*
 * api_watchdog:
 *
 * API to interface the controller over IPC.
 */


#ifndef __API_CONTROLLER__H_
#define __API_CONTROLLER__H_
#include "FreeRTOS.h"
#include "queue.h"
#include "assert.h"

#include "api.h"
#include "config.h"
#include "oscilloscope.h"

typedef enum   msg_controller_cmd 
  msg_controller_cmd_t;
typedef struct msg_controller_subscribe
  msg_controller_subscribe_t;
typedef enum   msg_controller_subscribe_variable
  msg_controller_subscribe_variable_t;

extern xQueueHandle ipc_controller;


portBASE_TYPE ipc_controller_mode_set(oscilloscope_mode_t mode);
portBASE_TYPE ipc_controller_toggle_time_axis_increase(void);
portBASE_TYPE ipc_controller_toggle_time_axis_decrease(void);
portBASE_TYPE ipc_controller_toggle_channel0_subbscribe(void);
portBASE_TYPE ipc_controller_toggle_channel1_subbscribe(void);
portBASE_TYPE ipc_controller_toggle_mode(void);
portBASE_TYPE ipc_controller_toggle_freeze_screen(void);

/* TODO should maybe be in a diffrent api... */
portBASE_TYPE ipc_controller_send_data(
    oscilloscope_input_t intput,
    uint16_t data,
    int timestamp);


/* returns 'subscribe_mode' messages */
portBASE_TYPE ipc_controller_subscribe(
    ipc_addr_t subscriber,
    msg_controller_subscribe_variable_t var);


#endif /* __API_CONTROLLER__H_ */
