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
#include "api_watchdog.h"
#include "api_controller.h"
#include "task_input_gpio.h"
#include "task_watchdog.h"

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
      /* TODO: we should fix a debouncer */
      static int i = 0;
		// Bounce?
		vTaskDelay(50/portTICK_RATE_MS);
		if(new_pin_state == GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9))
      switch(++i)
      {
        case 1:
          ipc_controller_mode_set(oscilloscope_mode_oscilloscope);
          ipc_watchdog_set_led_aux(1);
          break;
        case 3:
          ipc_controller_mode_set(oscilloscope_mode_multimeter);
          ipc_watchdog_set_led_aux(0);
          break;

        case 4:
          i = 0;
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
