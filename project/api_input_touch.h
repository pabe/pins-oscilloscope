/*
 * Thread-safe API-interface for the touchscreen.
 */

#ifndef __API_INPUT_TOUCH__H__
#define __API_INPUT_TOUCH__H__

#include "FreeRTOS.h"
#include "queue.h"

extern xQueueHandle ipc_input_touch;

#endif /* __API_INPUT_TOUCH__H__ */
