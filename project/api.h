/*
 * api:
 *
 * Base API.
 */

#ifndef __API__H_
#define __API__H_

typedef struct
{
  struct
  {
    enum msg_id
    {
      msg_id_controller_cmd,
      msg_id_watchdog_cmd
    } id;
  } head;
  union
  {
    enum msg_controller_cmd {
      controller_cmd_set_mode_oscilloscope,
      controller_cmd_set_mode_multimeter,
    } controller_cmd;
    enum msg_watchdog_cmd {
      watchdog_cmd_aux_led_lit,
      watchdog_cmd_aux_led_quench
    } watchdog_cmd;
  } data;
} msg_t;

portBASE_TYPE ipc_init(void);
#endif /* __API__H_ */
