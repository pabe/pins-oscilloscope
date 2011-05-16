#ifndef __IPC_H_
#define __IPC_H_

//#include <stdint.h>

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

struct ipc_addr
{
  enum ipc_modules mod;
};

struct ipc_loop_opt
{
  xQueueHandle q;
  struct
  {
    unsigned waiting_for_result:1;
  } flags;
};

//extern xQueueHandle ipc_queue[ipc_mod_LAST+1];

/* forwards */
struct ipc_msg;

/* functions return 0 on success, !0 on error if not other stated */
int ipc_init(void);
void ipc_finalizer(void);

xQueueHandle ipc_resolv_addr(const struct ipc_addr* addr);

portBASE_TYPE ipc_put(
    const struct ipc_addr *dest,
    const struct ipc_msg *msg,
    portTickType xTicksToWait);
portBASE_TYPE ipc_get(
    const struct ipc_addr *dest,
    struct ipc_msg *msg,
    portTickType xTicksToWait);


#if 0
/*
 0 -- signals normal exit
 x -- signal errors and will exit the loop and return x
 */
typedef int (ipc_timeout)(
    struct ipc_loop_opt *opt,
    void *p);

/*
 0 -> normal exit
 x -> abnormal exit
 */
typedef int (ipc_msg)(
    struct ipc_msg *msg,
    struct ipc_loop_opt *opt,
    void *p);

//int ipc_loop(int (*worker(void)), int (*irq(void)));
int ipc_loop(ipc_timeout *timeout,
             ipc_msg *msg,
             struct ipc_loop_opt *opt,
             void *p);
#endif
#endif /* __IPC_H_ */
