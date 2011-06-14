/*
 * API to interface the ADCs over IPC.
 */

#include "assert.h"
#include "api_measure.h"
#include "api_watchdog.h"

#include "FreeRTOS.h"
#include "queue.h"

xQueueHandle ipc_measure;
static xQueueHandle irq_transfer;

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
  irq_transfer = xQueueCreate(
      CONFIG_API_MEASURE_TRANSFER_SIZE,
      sizeof(measure_data_t));
}

void ipc_measure_get_data(measure_data_t *data)
{
  if(pdFALSE == xQueueReceive(
        irq_transfer,
        data,
        CONFIG_IPC_WAIT))
  {
    ipc_watchdog_signal_error(0);
  }
}

void ipc_measure_put_data(const measure_data_t *data)
{
  if(pdFALSE == xQueueSendToBack(
        irq_transfer,
        data,
        CONFIG_IPC_WAIT))
  {
    ipc_watchdog_signal_error(0);
  }
}
