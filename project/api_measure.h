/*
 * API to interface the ADCs over IPC.
 */

#ifndef __API_MEASURE__H_
#define __API_MEASURE__H_

#include "FreeRTOS.h"
#include "queue.h"

#include "api.h"

extern xQueueHandle ipc_measure;

#endif /* __API_MEASURE__H_ */
