#include "ipc_task.h"
#include "ipc.h"
#include "ipc_msg.h"
#include <stdio.h>

void task_ipc_testA(void *p)
{
  struct ipc_addr dest;

  printf("test");
  
  if(0 != ipc_addr_lookup(ipc_mod_display, &dest))
  {
    printf("testA: ERROR");
  }


  while(1)
  {
    struct ipc_msg msg;
    vTaskDelay(2000 / portTICK_RATE_MS);
    ipc_put(&dest, &msg, 0);
  }
}

void task_ipc_testB(void *p)
{
  struct ipc_addr me;
  
  if(0 != ipc_addr_lookup(ipc_mod_display, &me))
  {
    printf("testB ERROR0 |");
    return;
  }

  /* register this address to current task */
  if(0 != ipc_register(&me))
  {
    printf("testB ERROR1 |");
    return;
  }

  while(1)
  {
    struct ipc_msg msg;
    if(ipc_get(&me, &msg, 1000/portTICK_RATE_MS) == pdFALSE)
    {
      printf("0 ");
    }
    else
    {
      printf("1 ");
    }
  }

}
