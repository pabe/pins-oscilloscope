/**
 * task_watchdog:
 *
 * TASK responsible for status leds and HW-watchdog reset circuts.
 */

#include <stdint.h>

/* Firmware */
#include "setup.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"


#include "ipc.h"

//#include "task_input_gpio.h"

#define CFG_TASK_INPUT_GPIO__POLLING_PERIOD 300


/* private functions */
static portBASE_TYPE init(ipc_io_t *io);
static portBASE_TYPE timeout(ipc_io_t *io);
static portBASE_TYPE msg(ipc_io_t *io, ipc_msg_id_t *id_t, ipc_msg_t *msg);

/* private variables */
static int we_got_error = 0;

void task_watchdog_signal_error(void)
{
  we_got_error = 1;
}

void task_watchdog(void *p)
{
  ipc_io_t io;

  if(pdTRUE == init(&io) &&
    (pdTRUE == ipc_loop(&io, CFG_TASK_INPUT_GPIO__POLLING_PERIOD / portTICK_RATE_MS)))
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
#define LED_4 0x08
#define LED_3 0x04
#define LED_2 0x02

#define LED_BLUE   LED_4
#define LED_RED    LED_3
#define LED_ORANGE LED_2
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
  LED_out (led);
  i = !i;

  /* TODO: Add some watchdog reset. */
  return pdTRUE;
}

static portBASE_TYPE msg(ipc_io_t *io, ipc_msg_id_t *id_t, ipc_msg_t *msg)
{
  return pdFALSE;
}
