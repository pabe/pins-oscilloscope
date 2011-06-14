/*
 * API to interface the watchdog over IPC.
 */

#include "assert.h"
#include "api_display.h"

/* public variables */
xQueueHandle ipc_display;

/* private variables */

/* private functions */
__inline static
portBASE_TYPE ipc_display_send_cmd(msg_display_cmd_t cmd);

/* public functions */
portBASE_TYPE ipc_display_toggle_channel(oscilloscope_input_t ch)
{
  msg_t msg;
  portBASE_TYPE ret;

  msg.head.id = msg_id_display_toggle_channel;
  msg.data.msg_display_toggle_channel = ch;

  assert(ipc_display);
  ret = xQueueSendToBack(ipc_display, &msg, CONFIG_IPC_WAIT);
  // assert(ret == pdTRUE);
  return ret;
}

portBASE_TYPE ipc_display_button_highlight(int button)
{
  msg_t msg;
  portBASE_TYPE ret;

  msg.head.id = msg_id_display_button_highlight;
  msg.data.msg_display_button_highlight = button;

  assert(ipc_display);
  ret = xQueueSendToBack(ipc_display, &msg, CONFIG_IPC_WAIT);
  //assert(ret == pdTRUE);
  return ret;
}

portBASE_TYPE ipc_display_toggle_freeze_screen(void)
{
  return ipc_display_send_cmd(display_cmd_toggle_freeze_screen);
}

/* private functions */
__inline static
portBASE_TYPE ipc_display_send_cmd(msg_display_cmd_t cmd)
{
  msg_t msg;
  portBASE_TYPE ret;

  msg.head.id = msg_id_display_cmd;
  msg.data.display_cmd = cmd;

  assert(ipc_display);
  ret = xQueueSendToBack(ipc_display, &msg, CONFIG_IPC_WAIT);
  //assert(ret == pdTRUE);
  return ret;
}
