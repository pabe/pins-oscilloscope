/*
 * API to interface the watchdog over IPC.
 */

#ifndef __API_DISPLAY__H_
#define __API_DISPLAY__H_

#include "FreeRTOS.h"
#include "queue.h"

#include "api.h"

extern xQueueHandle ipc_display;

#endif /* __API_DISPLAY__H_ */
