
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ipc.h"
#include "task_ipc_testB.h"

/* private functions */
static portBASE_TYPE task_ipc_testB_timeout(struct ipc_io *io);
static portBASE_TYPE task_ipc_testB_msg(struct ipc_io *io, enum ipc_msg_id *id, union ipc_msg *msg);

void task_ipc_testB(void *p)
{
  struct ipc_addr me;
  struct ipc_io io;
  int error;

  while(1)
  {
    if(pdFALSE == ipc_addr_lookup(ipc_mod_testB, &me))
    {
      error = 0;
      break;
    }

    /* register this address to current task */
    if(pdFALSE == ipc_register(&io, task_ipc_testB_timeout, task_ipc_testB_msg, &me))
    {
      error = 1;
      break;
    }

    if(pdFALSE == ipc_loop(&io, 10000/portTICK_RATE_MS))
    {
      error = 2;
      break;
    }
  }

  /* if we are here then there is an error! */

  while(1)
  {
    printf("testB ERROR %i! |", error);
    vTaskDelay(2000 / portTICK_RATE_MS);
  }

  /* ask the kernel to kill me */
  //  vTaskDelete(NULL);
}

static portBASE_TYPE task_ipc_testB_timeout(struct ipc_io *io)
{
  printf("testB work|");
  return pdTRUE;
}

static portBASE_TYPE task_ipc_testB_msg(struct ipc_io *io, enum ipc_msg_id *id, union ipc_msg *msg)
{
  printf("testB event %i |", id);

  *id = ACKNOWLEDGE;
  return pdFALSE;
}

