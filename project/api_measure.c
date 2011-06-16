/*
 * Thread-safe API-interface for the ADCs.
 */

#include "assert.h"
#include "api_measure.h"
#include "api_watchdog.h"

#include "FreeRTOS.h"
#include "queue.h"

xQueueHandle ipc_measure;
static xQueueHandle data_queue;

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

void ipc_measure_init(void)
{
  data_queue = xQueueCreate(
      CONFIG_API_MEASURE_TRANSFER_SIZE,
      sizeof(measure_data_t));
}

void ipc_measure_get_data(measure_data_t *data)
{
  xQueueReceive(
      data_queue,
      data,
      CONFIG_IPC_WAIT);
}

void ipc_measure_put_data(const measure_data_t *data)
{
  xQueueSendToBack(
      data_queue,
      data,
      CONFIG_IPC_WAIT);
}

portBASE_TYPE ipc_measure_cfg_timer(
    uint16_t prescaler,
    uint16_t period)
{
  msg_t msg;
  portBASE_TYPE ret;

  msg.head.id = msg_id_measure_cfg_timer;
  msg.data.measure_cfg_timer.prescaler = prescaler;
  msg.data.measure_cfg_timer.period    = period;

  assert(ipc_measure);
  ret = xQueueSendToBack(ipc_measure, &msg, CONFIG_IPC_WAIT);

  return ret;
}
