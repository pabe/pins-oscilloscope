

#include <string.h>

#include "config.h"
#include "ipc.h"
#include "ipc_msg.h"

#include "setup.h"
#include "assert.h"

//#warning removing asserts...
//#define assert(x)

xQueueHandle ipc_queue[ipc_mod_LAST+1];

static xQueueHandle ipc_addr_resolve(const struct ipc_addr* addr);

int ipc_init(void)
{
  int i;

  ipc_queue[ipc_mod_display] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(struct ipc_fullmsg));
  ipc_queue[ipc_mod_testA] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(struct ipc_fullmsg));
  ipc_queue[ipc_mod_testB] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(struct ipc_fullmsg));
  ipc_queue[ipc_mod_input] =
    xQueueCreate(IPC_QUEUE_LEN_INPUT, sizeof(struct ipc_fullmsg));
  ipc_queue[ipc_mod_measuring] =
    xQueueCreate(IPC_QUEUE_LEN_MEASURING, sizeof(struct ipc_fullmsg));

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

static xQueueHandle ipc_addr_resolve(const struct ipc_addr* addr)
{
  assert(addr);

  return ipc_queue[addr->mod];
}

portBASE_TYPE ipc_addr_lookup(
    enum ipc_modules mod,
    struct ipc_addr* addr)
{
  assert(addr);

  addr->mod = mod;
  return pdTRUE;
}

portBASE_TYPE ipc_register(
    struct ipc_io *io,
    ipc_cb_timeout_t *cb_timeout,
    ipc_cb_msg_t *cb_msg,
    const struct ipc_addr *addr)
{
  assert(io);
  assert(cb_timeout);
  assert(cb_msg);
  assert(addr);

  /* TODO: add support to see if an other task owns this one */
  memset(io, 0, sizeof(struct ipc_io));

  io->qh         = ipc_addr_resolve(addr);
  io->cb_timeout = cb_timeout;
  io->cb_msg     = cb_msg;
  io->me         = *addr;
  //memcpy(io->me, addr, sizeof(struct ipc_addr));
  return pdTRUE;
}

portBASE_TYPE ipc_put(
    struct ipc_io *io,
    struct ipc_fullmsg *msg,
    const struct ipc_addr *dest)
{
  assert(dest);
  assert(msg);

  msg->head.src = *dest;
  return xQueueSendToBack(ipc_addr_resolve(dest), msg, 0);
}

portBASE_TYPE ipc_put2(
    struct ipc_io *io,
    const struct ipc_addr *dest,
    struct ipc_fullmsg *msg,
    struct ipc_fullmsg *response)
{
  portBASE_TYPE ret;
  assert(io);
  assert(dest);
  assert(msg);
  assert(response);

  
  /* TODO: use put().... */
  msg->head.src = io->me;
  if(pdFALSE == xQueueSendToBack(ipc_addr_resolve(dest), msg, portMAX_DELAY))
  {
    return pdFALSE;
  }

  /* now have we sent our control msg, setup the recv buffer and call loop*/
  io->flags.waiting_for_result = 1;
  io->recv_msg = response;

  /* TODO: read timeout from io struct */
  ret = ipc_loop(io, portMAX_DELAY);

  /* clear the control struct */
  io->flags.waiting_for_result = 0;
  io->recv_msg = NULL;
  return ret;
}

portBASE_TYPE ipc_loop(
    struct ipc_io *io,
    portTickType xTicksToWait)
{
  while(1)
  {
    portBASE_TYPE ret;
    struct ipc_fullmsg msg;
    struct ipc_addr msg_src;

    /* TODO: for now wait forever */
    ret = xQueueReceive(io->qh, &msg, xTicksToWait);

    /* timeout? */
    if(ret == pdFALSE)
    {
      if(!io->flags.waiting_for_result)
      {
        /* if there was a timeout and we not already in worker() */
        /* TODO: check return */
        if(pdFALSE == io->cb_timeout(io))
        {
          return pdFALSE;
        }
      }

      continue;
    }

    /* check if this is a reply */
    if(msg.head.reply)
    {
      /* are we expecting a reply? */
      if(io->recv_msg)
      {
        return pdFALSE;
      }

      /* we are called recursive here so it is ok to return */
      *io->recv_msg = msg;
      return pdTRUE;
    }

    /* TODO: no need anymore... */
    /* save src address as the bad user may edit the head */
    msg_src = msg.head.src;
    if(pdFALSE == io->cb_msg(io, &msg.head.Id, &msg.payload))
    {
      return pdFALSE;
    }

    msg.head.reply = 1;
    if(pdFALSE == ipc_put(io, &msg, &msg_src))
    {
      return pdFALSE;
    }
  }
}
