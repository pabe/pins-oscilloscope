#ifndef __IPC_H_
#define __IPC_H_

//#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "ipc_forwards.h"

typedef portBASE_TYPE (ipc_cb_timeout_t)(ipc_io_t *io);
typedef portBASE_TYPE (ipc_cb_msg_t)(ipc_io_t *io, ipc_msg_id_t *id, ipc_msg_t *msg);

typedef enum ipc_modules
{
	ipc_mod_display,
  ipc_mod_testA,
  ipc_mod_testB,
	ipc_mod_input,
	ipc_mod_measuring,

	/* remember to update if head or tail is changed! */
	ipc_mod_FIRST = ipc_mod_display,
	ipc_mod_LAST  = ipc_mod_measuring
} ipc_modules_t;

struct ipc_addr
{
  ipc_modules_t mod;
};

struct ipc_io
{
  ipc_cb_timeout_t *cb_timeout;
  ipc_cb_msg_t     *cb_msg;
  xQueueHandle     qh;
  ipc_addr_t       me;
  ipc_fullmsg_t    *recv_msg;
  struct
  {
    unsigned waiting_for_result:1; //recv_msg == NULL equals this =0
  } flags;
};

/* ipc.h should be the only one included from other subsystems */
#include "ipc_msg.h"


//extern xQueueHandle ipc_queue[ipc_mod_LAST+1];


/* functions return 0 on success, !0 on error if not other stated */
int ipc_init(void);
void ipc_finalizer(void);

portBASE_TYPE ipc_addr_lookup(
    ipc_modules_t mod,
    ipc_addr_t *addr);

portBASE_TYPE ipc_register(
    ipc_io_t *io,
    ipc_cb_timeout_t *cb_timeout,
    ipc_cb_msg_t *cb_msg,
    const ipc_addr_t *addr);

/* fire and forget */
/* will update the header part of msg */
portBASE_TYPE ipc_put(
    ipc_io_t *io,
    ipc_fullmsg_t *msg,
    const ipc_addr_t *dest);

/* fire and block */
/* will update the header part of msg */
portBASE_TYPE ipc_put2(
    ipc_io_t *io,
    const ipc_addr_t *dest,
    ipc_fullmsg_t *msg,
    ipc_fullmsg_t *response);


portBASE_TYPE ipc_loop(
    ipc_io_t *io,
    portTickType xTicksToWait);

#endif /* __IPC_H_ */
