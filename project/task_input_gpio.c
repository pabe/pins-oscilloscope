/**
 * task_input_gpio:
 *
 * Driver responsible for input from buttons.
 * (For now only 'Key'.)
 */

#include <stdio.h>
#include <string.h>

/* Firmware */
#include "stm32f10x_gpio.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"

#include "config.h"
#include "ipc.h"
#include "task_input_gpio.h"
#include "task_watchdog.h"

/* private functions */
static portBASE_TYPE init(ipc_io_t *io);
static portBASE_TYPE timeout(ipc_io_t *io);

/* private variables */
static uint8_t pin_state;

void task_input_gpio(void *p)
{
  ipc_io_t io;

  if(pdTRUE == init(&io) &&
    (pdTRUE == ipc_loop(&io, CFG_TASK_INPUT_GPIO__POLLING_PERIOD)))
  {
    /* well frankly, this should never happen...
     * ipc_loop() only returns when handling requests.
     */
  }

  /* 
   * so if we are here (init failed or loop() returned) something is bad.
   */
  task_watchdog_signal_error();

  /* ask the kernel to kill me */
  vTaskDelete(NULL);
}

#if 0
void task_input_gpio(void *p)
{
  portTickType wakeTime;
  uint8_t pin_state;
  
  task_input_gpio_config();

  wakeTime = xTaskGetTickCount();
  pin_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
  while(1)
  {
    uint8_t new_pin_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
    if(pin_state^new_pin_state)
    {
      ipc_msg msg;
      msg.watchdog_cmd = 0;

//      printf("B ");
    }

    pin_state = new_pin_state;
    vTaskDelayUntil(&wakeTime, CFG_TASK_INPUT_GPIO__POLLING_PERIOD / portTICK_RATE_MS);
  }
}

#endif
static portBASE_TYPE init(ipc_io_t *io)
{
  GPIO_InitTypeDef GPIO_InitStructure 
    = {GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_IPU};
  ipc_addr_t me;

  if(pdFALSE == ipc_addr_lookup(ipc_mod_input_gpio, &me))
  {
    return pdFALSE;
  }

  /* register this address to current task */
  if(pdFALSE == ipc_register(io, timeout, ipc_msg_def, &me))
  {
    return pdFALSE;
  }

  GPIO_Init( GPIOB, &GPIO_InitStructure );
  pin_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
  return pdTRUE;
}

static portBASE_TYPE timeout(ipc_io_t *io)
{
  uint8_t new_pin_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
  if(pin_state^new_pin_state)
  {
    ipc_addr_t addr_watchdog;
    ipc_fullmsg_t msg;
    /* the HW actually invert the value so pin_state == 0 means
     * button is pressed and == 1 means released
     */

    /* simple testing indicate that no button FOO is needed */
    if(pdFALSE == ipc_addr_lookup(ipc_mod_watchdog, &addr_watchdog))
    {
      return pdFALSE;
    }
    msg.head.Id = WATCHDOG_CMD;

    if(!new_pin_state)
    {
      msg.payload.watchdog_cmd.cmd = TASK_WATCHDOG_CMD_AUX_LED_LIT;
    }
    else
    {
      msg.payload.watchdog_cmd.cmd = TASK_WATCHDOG_CMD_AUX_LED_QUENCH;
    }

    ipc_put(io, &msg, NULL, & addr_watchdog);
    //      printf("B_%i ", !new_pin_state);
    pin_state = new_pin_state;
  }

  return pdTRUE;
}
