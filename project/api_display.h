/*
 * API to interface the watchdog over IPC.
 */

#ifndef __API_DISPLAY__H_
#define __API_DISPLAY__H_

#include "FreeRTOS.h"
#include "queue.h"

#include "api.h"

typedef enum msg_display_cmd
  msg_display_cmd_t;

extern xQueueHandle ipc_display;

portBASE_TYPE ipc_display_toggle_channel(oscilloscope_input_t ch);

#endif /* __API_DISPLAY__H_ */
