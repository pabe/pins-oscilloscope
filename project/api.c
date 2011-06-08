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

/* private functions */
static portBASE_TYPE ipc_init_module(xQueueHandle* h, unsigned portBASE_TYPE uxQueueLength)
{
  /* we do not init twice! */
  if(*h)
    return pdFALSE;

  *h = xQueueCreate(uxQueueLength, sizeof(msg_t));
  return *h ? pdTRUE : pdFALSE;
}
