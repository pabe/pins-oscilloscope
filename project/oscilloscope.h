/*
 * Structures for the project.
 */

#ifndef __OSCILLOSCOPE__H__
#define __OSCILLOSCOPE__H__

#include "stm32f10x_tim.h"

#include "FreeRTOSConfig.h" 
#include "task.h"
/* TODO: These should edit the global interrupt flag? */
__inline void interrupt_off(void)
{
  taskDISABLE_INTERRUPTS();
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
}

__inline void interrupt_on(void)
{
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  taskENABLE_INTERRUPTS();
}

typedef enum
{
  oscilloscope_mode_oscilloscope,
  oscilloscope_mode_multimeter
} oscilloscope_mode_t;

typedef enum
{
  input_channel0,
  input_channel1
} oscilloscope_input_t;

#endif /* __OSCILLOSCOPE__H__ */
