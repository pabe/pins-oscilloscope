
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ipc.h"
#include "task_ipc_testA.h"

/* private functions */
static portBASE_TYPE task_ipc_testA_timeout(struct ipc_io *io);
static portBASE_TYPE task_ipc_testA_msg(struct ipc_io *io, enum ipc_msg_id *id, union ipc_msg *msg);

void task_ipc_testA(void *p)
{
  struct ipc_addr me;
  struct ipc_io io;
  int error;

  while(1)
  {
    if(pdFALSE == ipc_addr_lookup(ipc_mod_testA, &me))
    {
      error = 0;
      break;
    }

    /* register this address to current task */
    if(pdFALSE == ipc_register(&io, task_ipc_testA_timeout, task_ipc_testA_msg, &me))
    {
      error = 1;
      break;
    }

    if(pdFALSE == ipc_loop(&io, 5000/portTICK_RATE_MS))
    {
      error = 2;
      break;
    }
  }

  /* if we are here then there is an error! */

  while(1)
  {
    printf("testA ERROR %i! |", error);
    vTaskDelay(2000 / portTICK_RATE_MS);
  }

  /* ask the kernel to kill me */
  //  vTaskDelete(NULL);
}

static portBASE_TYPE task_ipc_testA_timeout(struct ipc_io *io)
{
  struct ipc_fullmsg m;
  struct ipc_fullmsg m2;
  struct ipc_addr dest;
  printf("testA work|");
  
  if(pdFALSE == ipc_addr_lookup(ipc_mod_testB, &dest))
    return pdFALSE;
  m.head.Id = ACKNOWLEDGE;

  ipc_put2(io, &dest, &m, &m2);
  return pdTRUE;
}

static portBASE_TYPE task_ipc_testA_msg(struct ipc_io *io, enum ipc_msg_id *id, union ipc_msg *msg)
{
  printf("testA event %i |", id);

  *id = ACKNOWLEDGE;
  return pdFALSE;
}
