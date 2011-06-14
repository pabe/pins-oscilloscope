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

typedef struct
{
  uint16_t data[CONFIG_SAMPLE_BUFFER_SIZE];
  oscilloscope_input_t ch;
  int timestamp;
} measure_data_t;

extern xQueueHandle ipc_measure;

portBASE_TYPE ipc_measure_subscribe(
    ipc_addr_t subscriber,
    msg_measure_subscribe_variable_t var);

void ipc_measure_init(void);
void ipc_measure_get_data(measure_data_t *data);
void ipc_measure_put_data(const measure_data_t *data);
#endif /* __API_MEASURE__H_ */
