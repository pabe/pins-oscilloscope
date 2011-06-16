/*
 * Thread-safe API-interface for the display.
 */

#ifndef __API_DISPLAY__H__
#define __API_DISPLAY__H__
#include "FreeRTOS.h"
#include "queue.h"

#include "api.h"

typedef enum msg_display_cmd
  msg_display_cmd_t;

extern xQueueHandle ipc_display;

portBASE_TYPE ipc_display_toggle_channel(oscilloscope_input_t ch);
portBASE_TYPE ipc_display_button_highlight(int button);
portBASE_TYPE ipc_display_toggle_freeze_screen(void);

#endif /* __API_DISPLAY__H__ */
