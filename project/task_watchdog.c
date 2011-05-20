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

#include "config.h"
#include "ipc.h"
#include "task_watchdog.h"


#define LED_4 0x08
#define LED_3 0x04
#define LED_2 0x02
#define LED_BLUE   LED_4
#define LED_RED    LED_3
#define LED_ORANGE LED_2

/* private functions */
static portBASE_TYPE init(ipc_io_t *io);
static portBASE_TYPE timeout(ipc_io_t *io);
static portBASE_TYPE msg(ipc_io_t *io, ipc_msg_id_t *id, ipc_msg_t *msg);

/* private variables */
static int we_got_error = 0;
static int lit_aux_led = 0;

void task_watchdog_signal_error(void)
{
  we_got_error = 1;
}

void task_watchdog(void *p)
{
  ipc_io_t io;

  if(pdTRUE == init(&io) &&
    (pdTRUE == ipc_loop(&io, CFG_TASK_WATCHDOG__POLLING_PERIOD)))
  {
    /* well frankly, this should never happen...
     * ipc_loop() only returns when handling requests.
     */
  }

  /* 
   * So if we are here (init failed or loop() returned) something is bad
   * and yes, if we (watchdog) fail nooooooone will care :(
   */
  task_watchdog_signal_error();

  /* ask the kernel to kill me */
  vTaskDelete(NULL);
}

static portBASE_TYPE init(ipc_io_t *io)
{
  ipc_addr_t me;

  if(pdFALSE == ipc_addr_lookup(ipc_mod_watchdog, &me))
  {
    return pdFALSE;
  }

  /* register this address to current task */
  if(pdFALSE == ipc_register(io, timeout, msg, &me))
  {
    return pdFALSE;
  }

  return pdTRUE;
}

static portBASE_TYPE timeout(ipc_io_t *io)
{
  static int i = 0;
  u32 led;

  if(i)
  {
    led = (LED_BLUE | (we_got_error?LED_RED:0) );
  }
  else
  {
    led = 0;
  }

  if(lit_aux_led)
  {
    led |= LED_ORANGE;
  }

  LED_out(led);
  i = !i;

  return pdTRUE;
}

static portBASE_TYPE msg(ipc_io_t *io, ipc_msg_id_t *id, ipc_msg_t *_msg)
{
  switch(*id)
  {
    case WATCHDOG_CMD:
      {
        struct ipc_msg_watchdog_cmd *msg = &_msg->watchdog_cmd;
        switch(msg->cmd)
        {
          case TASK_WATCHDOG_CMD_AUX_LED_LIT:
            lit_aux_led = 1;
            break;

          case TASK_WATCHDOG_CMD_AUX_LED_QUENCH:
            lit_aux_led = 0;
            break;

          default:
            /* we silient ignore bad cmds */
            break;
        }
        return pdTRUE;
      }

    default:
      break;
  }
  return pdFALSE;
}
