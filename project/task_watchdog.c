/*
 * TASK responsible for status leds and HW-watchdog reset circuts.
 * (For now a pure SW that toogles a LED.)
 */
#include <stdio.h>

/* Firmware */
#include "assert.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include "task_watchdog.h"
#include "ipc.h"
#include "api_watchdog.h"


#define LED_4 0x08
#define LED_3 0x04
#define LED_2 0x02
#define LED_BLUE   LED_4
#define LED_RED    LED_3
#define LED_ORANGE LED_2


/* private functions */
static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *msg);
static void execute_led(void);

/* public variables */
/* private variables */
static int we_got_error = 0;
static int lit_led_aux = 0;
static int lit_led_watchdog = 0;
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_watchdog_cmd, handle_msg_cmd }
};
/* public functions */

void task_watchdog_signal_error(void)
{
  we_got_error = 1;
}

void task_watchdog(void *p)
{
  while(1)
  {
    if(pdTRUE == ipc_get(
          ipc_watchdog,
          CFG_TASK_WATCHDOG__POLLING_PERIOD,
          msg_handle_table,
          sizeof(msg_handle_table)/sizeof(msg_handle_table[0])))
    {
      execute_led();
      lit_led_watchdog = !lit_led_watchdog;
    }
    else
    {
      task_watchdog_signal_error();
      execute_led();
    }
  }
}

/* private functions */
static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *msg)
{
  switch(msg->watchdog_cmd)
  {
    case watchdog_cmd_aux_led_lit:
      lit_led_aux = 1;
      execute_led();
      break;

    case watchdog_cmd_aux_led_quench:
      lit_led_aux = 0;
      execute_led();
      break;

    default:
      return pdFALSE;
  }

  return pdTRUE;
}

static void execute_led(void)
{
  u32 led;

  led  = lit_led_watchdog ? LED_BLUE   : 0;
  led |= we_got_error     ? LED_RED    : 0;
  led |= lit_led_aux      ? LED_ORANGE : 0;

  LED_out(led);
}

