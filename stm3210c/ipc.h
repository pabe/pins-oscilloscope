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

extern xQueueHandle ipc_queue[ipc_mod_LAST+1];

/* return 0 on success, !0 on error */
int ipc_init(void);

void ipc_finalizer(void);

#endif /* __IPC_H_ */
