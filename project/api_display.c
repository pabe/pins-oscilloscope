/*
 * API to interface the watchdog over IPC.
 */

#include "assert.h"
#include "api_display.h"

xQueueHandle ipc_display;

portBASE_TYPE ipc_display_toggle_channel(oscilloscope_input_t ch)
{
  msg_t msg;
  msg.head.id = msg_id_display_toggle_channel;
  msg.data.msg_display_toggle_channel = ch;

  assert(ipc_display);
  return xQueueSendToBack(ipc_display, &msg, portMAX_DELAY);
}

portBASE_TYPE ipc_display_button_highlight(int button)
{
  msg_t msg;
  msg.head.id = msg_id_display_button_highlight;
  msg.data.msg_display_button_highlight = button;

  assert(ipc_display);
  return xQueueSendToBack(ipc_display, &msg, portMAX_DELAY);
}
