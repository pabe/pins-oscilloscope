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
