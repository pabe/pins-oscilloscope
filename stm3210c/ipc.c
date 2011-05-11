
#include "config.h"
#include "ipc.h"
#include "ipc_msg.h"

xQueueHandle ipc_queue[ipc_mod_LAST+1];

int ipc_init(void)
{
  int i;

  ipc_queue[ipc_mod_display] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, MSG_MAX_SIZE);
  ipc_queue[ipc_mod_input] =
    xQueueCreate(IPC_QUEUE_LEN_INPUT, MSG_MAX_SIZE);
  ipc_queue[ipc_mod_measuring] =
    xQueueCreate(IPC_QUEUE_LEN_MEASURING, MSG_MAX_SIZE);

  /* check if any of the queues fail to init */
  for(i=0; i < sizeof(ipc_queue)/sizeof(xQueueHandle); i++)
  {
    if(!ipc_queue[i])
    {
      ipc_finalizer();
      return !0;
    }
  }
  return 0;
}

void ipc_finalizer(void)
{
  int i;
  for(i=0; i < sizeof(ipc_queue)/sizeof(xQueueHandle); i++)
  {
    if(ipc_queue[i])
    {
      vQueueDelete(ipc_queue[i]);
    }
  }
}
