
//#include "FreeRTOS.h"
//#include "queue.h"

#include "config.h"
#include "ipc.h"
#include "ipc_msg.h"

#warning removing asserts...
#define assert(x)

xQueueHandle ipc_queue[ipc_mod_LAST+1];

int ipc_init(void)
{
  int i;

  ipc_queue[ipc_mod_display] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(struct ipc_msg));
  ipc_queue[ipc_mod_input] =
    xQueueCreate(IPC_QUEUE_LEN_INPUT, sizeof(struct ipc_msg));
  ipc_queue[ipc_mod_measuring] =
    xQueueCreate(IPC_QUEUE_LEN_MEASURING, sizeof(struct ipc_msg));

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

xQueueHandle ipc_resolv_addr(const struct ipc_addr* addr)
{
  return ipc_queue[addr->mod];
}

portBASE_TYPE ipc_put(
    const struct ipc_addr *dest,
    const struct ipc_msg *msg,
    portTickType xTicksToWait)
{
  assert(dest);
  assert(msg);

  return xQueueSendToBack(ipc_resolv_addr(dest), msg, xTicksToWait);
}

portBASE_TYPE ipc_get(
    const struct ipc_addr *dest,
    struct ipc_msg *msg,
    portTickType xTicksToWait)
{
  assert(dest);
  assert(msg);

  return xQueueReceive(ipc_resolv_addr(dest), msg, xTicksToWait);
}

#if 0
int ipc_loop(ipc_timeout *cb_timeout,
             ipc_msg *cb_msg,
             struct ipc_loop_opt *opt,
             void *p)
{
  while(1)
  {
    portBASE_TYPE ret;
    struct ipc_msg msg;
    struct ipc_msg_head msg_header;

    /* TODO: for now wait forever */
    ret = xQueueReceive(opt->q, &msg, portMAX_DELAY);

    /* timeout? */
    if(ret == pdFALSE)
    {
      if(!opt->flags.waiting_for_result)
      {
        /* if there was a timeout and we not already in worker() */
        /* TODO: check return */
        cb_timeout(opt, p);
      }

      continue;
    }

    /* check if this is a reply */
    if(msg.head.reply)
    {
      if(!opt->flags.waiting_for_result)
      {
        /* TODO: signal errir */
      }
  
      /* store result */
      /* return with code got_result */
      return 0;
    }

    /* TODO: based on return we should maybe send a reply */
    msg_header = msg.head;
    if(cb_msg(&msg, opt, p) != 0)
    {
      /* signal error */
#if 0
      /* send error */
      case 0:
      case 1:
        /* send msg */

      default:
        /* TODO: DEBUG PRINT: Error */
        return -1;
#endif
    }

    msg.head.reply = 1;
    msg.datagram = 1;
//    msg.head.src = opt->q;


  }
}
#endif
