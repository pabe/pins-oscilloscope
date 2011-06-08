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
portBASE_TYPE ipc_controller_subscribe(
    ipc_addr_t subscriber,
    msg_controller_subscribe_variable_t var);
portBASE_TYPE ipc_controller_send_cmd(msg_controller_cmd_t cmd);


#endif /* __API_CONTROLLER__H_ */
