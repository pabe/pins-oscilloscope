/*
 * API to interface the ADCs over IPC.
 */

#ifndef __API_MEASURE__H_
#define __API_MEASURE__H_

#include "FreeRTOS.h"
#include "queue.h"

#include "api.h"

typedef enum msg_measure_subscribe_variable
  msg_measure_subscribe_variable_t;

extern xQueueHandle ipc_measure;

portBASE_TYPE ipc_measure_subscribe(
    ipc_addr_t subscriber,
    msg_measure_subscribe_variable_t var);
#endif /* __API_MEASURE__H_ */
