

#include <string.h>

/* Firmware */
#include "setup.h"
#include "assert.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include "config.h"
#include "ipc.h"

xQueueHandle ipc_queue[ipc_mod_LAST+1];

static xQueueHandle ipc_addr_resolve(const ipc_addr_t* addr);

portBASE_TYPE ipc_timeout_def(ipc_io_t *io)
{
  return pdTRUE;
}

portBASE_TYPE ipc_msg_def(ipc_io_t *io, ipc_msg_id_t *id, ipc_msg_t *msg)
{
  *id = ACKNOWLEDGE;
  return pdTRUE;
}

portBASE_TYPE ipc_init(void)
{
  int i;

  ipc_queue[ipc_mod_display] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_watchdog] =
    xQueueCreate(IPC_QUEUE_LEN_WATCHDOG, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_input_gpio] =
    xQueueCreate(IPC_QUEUE_LEN_INPUT_GPIO, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_controller] =
    xQueueCreate(IPC_QUEUE_LEN_CONTROLLER, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_testA] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_testB] =
    xQueueCreate(IPC_QUEUE_LEN_DISPLAY, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_input] =
    xQueueCreate(IPC_QUEUE_LEN_INPUT, sizeof(ipc_fullmsg_t));
  ipc_queue[ipc_mod_measuring] =
    xQueueCreate(IPC_QUEUE_LEN_MEASURING, sizeof(ipc_fullmsg_t));

  /* check if any of the queues fail to init */
  for(i=0; i < sizeof(ipc_queue)/sizeof(xQueueHandle); i++)
  {
    if(!ipc_queue[i])
    {
      return pdFALSE;
    }
  }
  return pdTRUE;
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

static xQueueHandle ipc_addr_resolve(const ipc_addr_t* addr)
{
  assert(addr);

  return ipc_queue[addr->mod];
}

portBASE_TYPE ipc_addr_lookup(
    ipc_modules_t mod,
    ipc_addr_t* addr)
{
  assert(addr);

  addr->mod = mod;
  return pdTRUE;
}

portBASE_TYPE ipc_register(
    ipc_io_t *io,
    ipc_cb_timeout_t *cb_timeout,
    ipc_cb_msg_t *cb_msg,
    const ipc_addr_t *addr)
{
  xQueueHandle q;
  assert(io);
  assert(cb_timeout);
  assert(cb_msg);
  assert(addr);

  q = ipc_addr_resolve(addr);
  if(!q)
    return pdFALSE;;
  
  /* TODO: add support to see if an other task owns this one */
  memset(io, 0, sizeof(ipc_io_t));

  io->qh         = q;
  io->cb_timeout = cb_timeout;
  io->cb_msg     = cb_msg;
  io->me         = *addr;
  return pdTRUE;
}

portBASE_TYPE ipc_put(
    ipc_io_t *io,
    ipc_fullmsg_t *msg,
    ipc_fullmsg_t *reply,
    const ipc_addr_t *dest)
{
  portBASE_TYPE ret;
  xQueueHandle q;
  assert(io);
  assert(dest);
  assert(msg);

  q = ipc_addr_resolve(dest);
  if(!q)
  {
    return pdFALSE;
  }
  
  msg->head.src = io->me;
  if(pdFALSE == xQueueSendToBack(q, msg, portMAX_DELAY))
  {
    return pdFALSE;
  }

  /* no reply means fire and forget */
  if(!reply)
  {
    return pdTRUE;
  }

  /* connect a buffer to store respone
   * this also indicate to ipc_loop() that we are expecting a response
   */
  io->recv_msg = reply;

  /* requests block forever,
   * ie. if no one is answering tough luck
   */
  ret = ipc_loop(io, portMAX_DELAY);

  /* clear the control struct */
  io->recv_msg = NULL;
  return ret;
}

portBASE_TYPE ipc_loop(
    ipc_io_t *io,
    portTickType xTicksToWait)
{
  portTickType timeout = xTicksToWait;

  while(1)
  {
    portTickType sleep_time;
    portBASE_TYPE ret;
    ipc_fullmsg_t msg;
    ipc_addr_t msg_src;

    sleep_time = xTaskGetTickCount();
    ret = xQueueReceive(io->qh, &msg, timeout);

    /* timeout? */
    if(ret == pdFALSE)
    {
      /* are we expecting a reply? */
      if(!io->recv_msg)
      {
        /* we where not expecting a reply alas we are not executing
         * the timeout routine
         */
        if(pdFALSE == io->cb_timeout(io))
        {
          return pdFALSE;
        }
      }

      timeout = xTicksToWait;
      continue;
    }
    
    /* TODO: Recalculate timeout */
    timeout = timeout - (xTaskGetTickCount() - sleep_time);

    /* is this a reply? */
    if(msg.head.reply)
    {
      /* are we expecting a reply? */
      if(!io->recv_msg)
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
    
    if(io->recv_msg)
    {
      msg.head.reply = 1;
      if(pdFALSE == ipc_put(io, &msg, NULL, &msg_src))
      {
        return pdFALSE;
      }
    }
  }
}
