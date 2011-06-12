/*
 * Some generica IPC helper functions.
 */

#ifndef __IPC__H_
#define __IPC__H_

#include <stdint.h>
#include "config.h"
#include "api.h"

/* internal IPC macros start */
/* DO NOT use outside ipc.{h|c} */
#define IPC_OFFSET(MEMBER) \
  (&(((msg_data_t*)0)->MEMBER))

#define IPC_SET_OFFSET(TYPE, SUB, VAR) \
  IPC_OFFSET(TYPE ## . ## SUB), IPC_OFFSET(TYPE ## . ## VAR)
/* internal IPC macros end */

typedef struct
{
  msg_t msg;
  xQueueHandle queues[CONFIG_SUBSCRIBE_MSG_HOOKS];
} ipc_subscribe_msg_t;

typedef struct 
{
  enum msg_id id;
  portBASE_TYPE (*handler)(msg_id_t id, msg_data_t *data);
} ipc_loop_t;

typedef struct
{
  ipc_subscribe_msg_t* var;
  int variable_var;
  void *subscriber;
  void *variable;
} ipc_subscribe_table_t;
#define IPC_SUBSCRIBE_TABLE_INIT(MSG_PTR, VALUE, TYPE) \
  { MSG_PTR, VALUE, IPC_SET_OFFSET(TYPE, subscriber,variable) }

portBASE_TYPE ipc_init(void);
/*
 * return:
 * true = timeout
 * false = error
 */
portBASE_TYPE ipc_get(
    ipc_addr_t addr,
    portTickType xTicksToWait,
    const ipc_loop_t handlers[],
    size_t n);
portBASE_TYPE ipc_handle_msg_subscribe(
    msg_data_t *msg,
    const ipc_subscribe_table_t table[],
    size_t size);
void ipc_subscribe_init(ipc_subscribe_msg_t *sub, msg_id_t head_id);
portBASE_TYPE ipc_subscribe_execute(ipc_subscribe_msg_t *v);
portBASE_TYPE ipc_subscribe_add(
    ipc_subscribe_msg_t *v,
    ipc_addr_t subscriber);

#endif /* __IPC__H_ */