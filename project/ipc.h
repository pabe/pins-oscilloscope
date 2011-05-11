#ifndef __IPC_H_
#define __IPC_H_

#include "FreeRTOS.h"
#include "queue.h"

enum ipc_modules
{
	ipc_mod_display,
	ipc_mod_input,
	ipc_mod_measuring,

	/* remember to update if head or tail is changed! */
	ipc_mod_FIRST = ipc_mod_display,
	ipc_mod_LAST  = ipc_mod_measuring
};

//extern xQueueHandle ipc_queue[ipc_mod_LAST+1];

/* forwards */
union ipc_msg;

/* functions return 0 on success, !0 on error if not other stated */
int ipc_init(void);
void ipc_finalizer(void);
int ipc_put(enum ipc_modules dest,
            const union ipc_msg *msg);
int ipc_get(enum ipc_modules src,
            union ipc_msg *msg);

#endif /* __IPC_H_ */
