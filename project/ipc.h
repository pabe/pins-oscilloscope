#ifndef __IPC_H_
#define __IPC_H_

//#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "ipc_forwards.h"
typedef portBASE_TYPE (ipc_cb_timeout_t)(struct ipc_io *io);
typedef portBASE_TYPE (ipc_cb_msg_t)(struct ipc_io *io, enum ipc_msg_id *id, union ipc_msg *msg);

enum ipc_modules
{
	ipc_mod_display,
  ipc_mod_testA,
  ipc_mod_testB,
	ipc_mod_input,
	ipc_mod_measuring,

	/* remember to update if head or tail is changed! */
	ipc_mod_FIRST = ipc_mod_display,
	ipc_mod_LAST  = ipc_mod_measuring
};

struct ipc_addr
{
  enum ipc_modules mod;
};

struct ipc_io
{
  ipc_cb_timeout_t *cb_timeout;
  ipc_cb_msg_t     *cb_msg;
  xQueueHandle qh;
  struct ipc_addr me;
  struct ipc_fullmsg *recv_msg;
  struct
  {
    unsigned waiting_for_result:1; //recv_msg == NULL equals this =0
  } flags;
};

//extern xQueueHandle ipc_queue[ipc_mod_LAST+1];


/* functions return 0 on success, !0 on error if not other stated */
int ipc_init(void);
void ipc_finalizer(void);

portBASE_TYPE ipc_addr_lookup(
    enum ipc_modules mod,
    struct ipc_addr *addr);
portBASE_TYPE ipc_register(
    struct ipc_io *io,
    ipc_cb_timeout_t *cb_timeout,
    ipc_cb_msg_t *cb_msg,
    const struct ipc_addr *addr);

/* fire and forget */
/* will update the header part of msg */
portBASE_TYPE ipc_put(
    struct ipc_io *io,
    struct ipc_fullmsg *msg,
    const struct ipc_addr *dest);

/* fire and block */
/* will update the header part of msg */
portBASE_TYPE ipc_put2(
    struct ipc_io *io,
    const struct ipc_addr *dest,
    struct ipc_fullmsg *msg,
    struct ipc_fullmsg *response);


portBASE_TYPE ipc_loop(
    struct ipc_io *io,
    portTickType xTicksToWait);

#endif /* __IPC_H_ */
