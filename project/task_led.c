/**
 * TASK: Blink the LEDs to show that we are alive
 */

#include <stdint.h>

#include "setup.h"
#include "FreeRTOS.h"
#include "task.h"


#include "task_input_gpio.h"

#define CFG_TASK_INPUT_GPIO__POLLING_PERIOD 1000


static void task_led_config(void);

void task_led(void *p)
{

  const uint8_t led_val[8] = { 0x01,0x03,0x07,0x0F,0x0E,0x0C,0x08,0x00 };
  int cnt = 0;

  task_led_config();
  while(1)
  {
    LED_out (led_val[cnt]);
    cnt = (cnt + 1) % sizeof(led_val);
    vTaskDelay(CFG_TASK_INPUT_GPIO__POLLING_PERIOD / portTICK_RATE_MS);
  }
}

static void task_led_config(void)
{
}
