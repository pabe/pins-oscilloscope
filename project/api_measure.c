/*
 * API to interface the ADCs over IPC.
 */

#include "assert.h"
#include "api_measure.h"

xQueueHandle ipc_measure;

portBASE_TYPE ipc_measure_subscribe(
    ipc_addr_t subscriber,
    msg_measure_subscribe_variable_t var)
{
  msg_t msg;
  portBASE_TYPE ret;

  msg.head.id = msg_id_measure_subscribe;
  msg.data.measure_subscribe.variable = var;
  msg.data.measure_subscribe.subscriber = subscriber;

  assert(ipc_measure);
  ret = xQueueSendToBack(ipc_measure, &msg, CONFIG_IPC_WAIT);
  assert(ret == pdTRUE);
  return ret;
}

#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"

struct
{
  uint16_t data[CONFIG_SAMPLE_BUFFER_SIZE];
  int timestamp;
  xSemaphoreHandle lock;
} buffer[CONFIG_SAMPLE_BUFFERS];

/* Known issue: buff_i not protected */
/* This is not a problem for now as only taskMeasure calls put */
static int buff_i = 0;

void ipc_measure_init(void)
{
  int i;
  for(i = 0; i<CONFIG_SAMPLE_BUFFERS; i++)
  {
    buffer[i].lock = xSemaphoreCreateMutex();
  }
}

void ipc_measure_get_data(uint16_t *dest, int *timestamp)
{
  int i = buff_i;
  xSemaphoreTake(buffer[i].lock, portMAX_DELAY);
  memcpy(dest, buffer[i].data, sizeof(uint16_t)*CONFIG_SAMPLE_BUFFER_SIZE);
  *timestamp = buffer[i].timestamp;
  xSemaphoreGive(buffer[i].lock);
}

void ipc_measure_put_data(uint16_t *src, int timestamp)
{
  int i = (buff_i+1)%2;
  
  xSemaphoreTake(buffer[i].lock, portMAX_DELAY);
  memcpy(buffer[i].data, src, sizeof(uint16_t)*CONFIG_SAMPLE_BUFFER_SIZE);
  buffer[i].timestamp = timestamp;
  xSemaphoreGive(buffer[i].lock);
  
  buff_i = i;
}
