/*
 * Thread-safe API-interface for the controller.
 */

#ifndef __API_CONTROLLER__H__
#define __API_CONTROLLER__H__
#include "FreeRTOS.h"
#include "queue.h"

#include "api.h"
#include "oscilloscope.h"

typedef enum   msg_controller_cmd 
  msg_controller_cmd_t;
typedef struct msg_controller_subscribe
  msg_controller_subscribe_t;
typedef enum   msg_controller_subscribe_variable
  msg_controller_subscribe_variable_t;

extern xQueueHandle ipc_controller;

portBASE_TYPE ipc_controller_mode_set(oscilloscope_mode_t mode);
portBASE_TYPE ipc_controller_mode_toggle(void);
portBASE_TYPE ipc_controller_time_axis_increase(void);
portBASE_TYPE ipc_controller_time_axis_decrease(void);

portBASE_TYPE ipc_controller_subscribe(
    ipc_addr_t subscriber,
    msg_controller_subscribe_variable_t var);

#endif /* __API_CONTROLLER__H__ */
