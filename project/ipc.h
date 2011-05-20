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
  ipc_mod_watchdog,
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
};

/* ipc.h should be the only one included from other subsystems */
#include "ipc_msg.h"

/******************************************************************************/

/* if ipc_init() failas some queues may have been inited so a call to
 * ipc_finalizer() shoudl always be done */
portBASE_TYPE ipc_init(void);
void ipc_finalizer(void);

portBASE_TYPE ipc_addr_lookup(
    ipc_modules_t mod,
    ipc_addr_t *addr);

portBASE_TYPE ipc_register(
    ipc_io_t *io,
    ipc_cb_timeout_t *cb_timeout,
    ipc_cb_msg_t *cb_msg,
    const ipc_addr_t *addr);

/* will update the header part of msg */
/* reply == NULL means fire and forget */
portBASE_TYPE ipc_put(
    ipc_io_t *io,
    ipc_fullmsg_t *msg,
    ipc_fullmsg_t *reply,
    const ipc_addr_t *dest);


portBASE_TYPE ipc_loop(
    ipc_io_t *io,
    portTickType xTicksToWait);

#endif /* __IPC_H_ */
