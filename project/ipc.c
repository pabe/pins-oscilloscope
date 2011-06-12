/*
 * Some generica IPC helper functions.
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "api.h"
#include "config.h"
#include "api_watchdog.h"
#include "api_controller.h"
#include "api_input_touch.h"
#include "api_display.h"
#include "api_measure.h"

/* public variables */

/* private variables */

/* private functions */
#include "ipc.h"
static portBASE_TYPE ipc_init_module(xQueueHandle* h, unsigned portBASE_TYPE uxQueueLength);

/* public functions */
portBASE_TYPE ipc_init(void)
{
  return ipc_init_module(&ipc_watchdog,  IPC_QUEUE_LEN_WATCHDOG)
    && ipc_init_module(&ipc_controller,  IPC_QUEUE_LEN_CONTROLLER)
    && ipc_init_module(&ipc_input_touch, IPC_QUEUE_LEN_INPUT_TOUCH)
    && ipc_init_module(&ipc_display,     IPC_QUEUE_LEN_DISPLAY)
    && ipc_init_module(&ipc_measure,     IPC_QUEUE_LEN_MEASURE);
}

portBASE_TYPE ipc_get(
    ipc_addr_t addr,
    portTickType xTicksToWait,
    const ipc_loop_t handlers[],
    size_t n)
{
  msg_t msg;
  portTickType sleep_time;
  int i;

  assert(addr);

  while(1)
  {
    sleep_time = xTaskGetTickCount();
    if(pdFALSE == xQueueReceive(addr, &msg, xTicksToWait))
      return pdTRUE;

    for(i=0; i<n; i++)
    {
      if(handlers[i].id == msg.head.id)
      {
        if(pdFALSE == handlers[i].handler(msg.head.id, &msg.data))
        {
          return pdFALSE;
        }
        break;
      }
    }
    xTicksToWait -= xTaskGetTickCount() - sleep_time;
  }
}

portBASE_TYPE ipc_handle_msg_subscribe(
    msg_data_t *msg,
    const ipc_subscribe_table_t table[],
    size_t size)
{
  int i;
  for(i = 0; i < size; i++)
  {
    int *variable = (int*)((int8_t*)msg + (int)table[i].variable);
    if(*variable == table[i].variable_var)
    {
      ipc_addr_t *subscriber = (ipc_addr_t*)((int8_t*)msg + (int)table[i].subscriber);
      if(pdFALSE == subscribe_add(table[i].var, *subscriber))
      {
        ipc_watchdog_signal_error(0);
        return pdFALSE;
      }
    }
  }
  return pdTRUE;
}

void subscribe_init(subscribe_msg_t *sub, msg_id_t head_id)
{
  int i;
  assert(sub);
  sub->msg.head.id = head_id;

  for(i=0; i<sizeof(sub->queues)/sizeof(sub->queues[0]); i++)
  {
    sub->queues[i] = NULL;
  }
}

portBASE_TYPE subscribe_execute(subscribe_msg_t *v)
{
  int i;
  assert(v);

  for(i=0; i<sizeof(v->queues)/sizeof(v->queues[0]); i++)
  {
    if(!v->queues[i])
      break;

    /* ignore fails */
    if(pdFALSE == xQueueSendToBack(v->queues[i], &v->msg, CONFIG_IPC_WAIT))
      return pdFALSE;

  }

  return pdTRUE;
}

portBASE_TYPE subscribe_add(subscribe_msg_t *v, ipc_addr_t subscriber)
{
  int i;
  assert(v);
  assert(subscriber);

  for(i=0; i<sizeof(v->queues)/sizeof(v->queues[0]); i++)
  {
    if(!v->queues[i])
    {
      v->queues[i] = subscriber;

      /* ignore fails */
      xQueueSendToBack(v->queues[i], &v->msg, portMAX_DELAY);
      return pdTRUE;
    }
  }

  return pdFALSE;
}

/* private functions */
static portBASE_TYPE ipc_init_module(xQueueHandle* h, unsigned portBASE_TYPE uxQueueLength)
{
  /* we do not init twice! */
  if(*h)
    return pdFALSE;

  *h = xQueueCreate(uxQueueLength, sizeof(msg_t));
  assert(*h);
  return *h ? pdTRUE : pdFALSE;
}
