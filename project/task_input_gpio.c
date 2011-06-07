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
#include "task_input_gpio.h"
#include "task_watchdog.h"
#include "api_watchdog.h"

/* private functions */
static portBASE_TYPE init(void);

/* private variables */

/* public functions */
void task_input_gpio(void *p)
{
  portTickType wakeTime;
  uint8_t pin_state;

  init();
  wakeTime = xTaskGetTickCount();
  pin_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
  while(1)
  {
    uint8_t new_pin_state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
    if(pin_state^new_pin_state)
    {
      msg_watchdog_t msg;

      msg.cmd = pin_state ? watchdog_cmd_aux_led_lit : watchdog_cmd_aux_led_quench;
      if(pdFALSE == xQueueSendToBack(ipc_watchdog, &msg, portMAX_DELAY))
      {
        /* TODO: handle error */
      }
    }

    pin_state = new_pin_state;
    vTaskDelayUntil(&wakeTime, CFG_TASK_INPUT_GPIO__POLLING_PERIOD / portTICK_RATE_MS);
  }
}

/* private functions */
static portBASE_TYPE init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure 
    = {GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_IPU};

  GPIO_Init( GPIOB, &GPIO_InitStructure );
  return pdTRUE;
}
