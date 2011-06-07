/**
 * task_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 * (For now a pure SW that toogles a LED.)
 */
#include <stdio.h>
#include <stdint.h>

/* Firmware */
#include "setup.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "assert.h"

#include "config.h"
#include "task_watchdog.h"
#include "api_watchdog.h"


#define LED_4 0x08
#define LED_3 0x04
#define LED_2 0x02
#define LED_BLUE   LED_4
#define LED_RED    LED_3
#define LED_ORANGE LED_2

/* private functions */

/* private variables */
static int we_got_error = 0;
static int lit_led_aux = 0;
static int lit_led_watchdog = 0;

/* public variables */
xQueueHandle ipc_watchdog;

/* public functions */
portBASE_TYPE ipc_watchdog_init(void)
{
  assert(!ipc_watchdog);

  ipc_watchdog = xQueueCreate(IPC_QUEUE_LEN_WATCHDOG, sizeof(msg_watchdog_t));

  return ipc_watchdog ? pdTRUE : pdFALSE;
}

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
    msg_watchdog_t msg;

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
      /* recalculate timeout */
      timeout = timeout - (xTaskGetTickCount() - sleep_time);
      switch(msg.cmd)
      {
        case watchdog_cmd_aux_led_lit:
          lit_led_aux = 1;
          break;

        case watchdog_cmd_aux_led_quench:
          lit_led_aux = 0;
          break;

        default:
          assert(0);
      }
    }
  }
}
