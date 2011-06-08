/**
 * task_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 * (For now a pure SW that toogles a LED.)
 */

/* Firmware */
#include "assert.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include "task_watchdog.h"
#include "api_watchdog.h"


#define LED_4 0x08
#define LED_3 0x04
#define LED_2 0x02
#define LED_BLUE   LED_4
#define LED_RED    LED_3
#define LED_ORANGE LED_2

/* public variables */

/* private variables */
static int we_got_error = 0;
static int lit_led_aux = 0;
static int lit_led_watchdog = 0;

/* private functions */
static void handle_msg_cmd(msg_watchdog_cmd_t *cmd);

/* public functions */

void task_watchdog_signal_error(void)
{
  we_got_error = 1;
}

void task_watchdog(void *p)
{
  portTickType timeout = CFG_TASK_WATCHDOG__POLLING_PERIOD;
  while(1)
  {
    portTickType sleep_time;
    msg_t msg;

    sleep_time = xTaskGetTickCount();
    
    assert(ipc_watchdog);
    if(pdFALSE == xQueueReceive(ipc_watchdog, &msg, timeout))
    {
      u32 led;
      timeout = CFG_TASK_WATCHDOG__POLLING_PERIOD;

      led  = lit_led_watchdog ? LED_BLUE   : 0;
      led |= we_got_error     ? LED_RED    : 0;
      led |= lit_led_aux      ? LED_ORANGE : 0;

      LED_out(led);
      lit_led_watchdog = !lit_led_watchdog;
    }
    else
    {
      switch(msg.head.id)
      {
        case msg_id_watchdog_cmd:
          handle_msg_cmd(&msg.data.watchdog_cmd);
          break;

        case msg_subscribe_mode:
          printf("watchdog: mode: %i.\n", msg.data.subscribe_mode);
          break;

        default:
          /* TODO: Output error mesg? */
          task_watchdog_signal_error();
      }

      /* recalculate timeout */
      timeout = timeout - (xTaskGetTickCount() - sleep_time);
    }
  }
}

/* private functions */
static void handle_msg_cmd(msg_watchdog_cmd_t *cmd)
{
  switch(*cmd)
  {
    case watchdog_cmd_aux_led_lit:
      {
        static int i = 0;
        if(!i)
        {
          i=1;
          ipc_controller_subscribe(ipc_watchdog, ipc_controller_variable_mode);
        }
        else
        {
          ipc_controller_mode_set(oscilloscope_mode_oscilloscope);
        }
      }
      lit_led_aux = 1;
      break;

    case watchdog_cmd_aux_led_quench:
      ipc_controller_mode_set(oscilloscope_mode_multimeter);
      lit_led_aux = 0;
      break;
    default:
      /* TODO: Output error mesg? */
      task_watchdog_signal_error();
  }
}
