
#include <stdio.h>
#include <string.h>

#include "stm32f10x_gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "task_input_gpio.h"

#define CFG_TASK_INPUT_GPIO__POLLING_PERIOD 1000


static void task_input_gpio_config(void);

void task_input_gpio(void *p)
{
  portTickType wakeTime;
  task_input_gpio_config();

  wakeTime = xTaskGetTickCount();
  while(1)
  {
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9))
    {
//      printf("|Nej");
    }
    else
    {
//      printf("|Ja");
    }
    vTaskDelayUntil(&wakeTime, CFG_TASK_INPUT_GPIO__POLLING_PERIOD / portTICK_RATE_MS);
  }
}

static void task_input_gpio_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure 
    = {GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_IPU};

  GPIO_Init( GPIOB, &GPIO_InitStructure );
}
