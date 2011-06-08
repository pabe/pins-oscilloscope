/*
 * api:
 *
 * Base API.
 */

#include "FreeRTOS.h"
#include "queue.h"
#include "api.h"
#include "config.h"
#include "api_watchdog.h"
#include "api_controller.h"

/* public variables */

/* private variables */

/* private functions */
static portBASE_TYPE ipc_init_module(xQueueHandle* h, unsigned portBASE_TYPE uxQueueLength);

/* public functions */
portBASE_TYPE ipc_init(void)
{
  return ipc_init_module(&ipc_watchdog, IPC_QUEUE_LEN_WATCHDOG)
    && ipc_init_module(&ipc_controller, IPC_QUEUE_LEN_CONTROLLER);
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

void subscribe_execute(subscribe_msg_t *v)
{
  int i;
  assert(v);

  for(i=0; i<sizeof(v->queues)/sizeof(v->queues[0]); i++)
  {
    if(!v->queues[i])
      break;

    /* ignore fails */
    xQueueSendToBack(v->queues[i], &v->msg, portMAX_DELAY);
  }
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
  return *h ? pdTRUE : pdFALSE;
}
