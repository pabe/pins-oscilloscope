/**
 * Program skeleton for the course "Programming embedded systems"
 */

#include <stdio.h>
#include "msg.h"
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_cl.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "setup.h"
#include "assert.h"
#include "GLCD.h"

/*-----------------------------------------------------------*/

#define WIDTH 320

xSemaphoreHandle lcdLock;

/**
 * LCD Module init
 */
static void init_display () {
  lcdLock = xSemaphoreCreateMutex();

  GLCD_init();
  GLCD_clear(White);
  GLCD_setTextColor(Blue);
  GLCD_displayStringLn(Line2, " Programming");
  GLCD_displayStringLn(Line3, " Embedded");
  GLCD_displayStringLn(Line4, " Systems");
}

/**
 * Display 1 2 3 with shifting colours
 */
void lcdTask(void *params) {
  unsigned short col1 = Blue, col2 = Red, col3 = Green;
  unsigned short t;

  for (;;) {
    xSemaphoreTake(lcdLock, portMAX_DELAY);
    GLCD_setTextColor(col1);
    GLCD_displayChar(Line7, WIDTH - 40, '1');
    GLCD_setTextColor(col2);
    GLCD_displayChar(Line7, WIDTH - 60, '2');
    GLCD_setTextColor(col3);
    GLCD_displayChar(Line7, WIDTH - 80, '3');
    xSemaphoreGive(lcdLock);
	t = col1; col1 = col2; col2 = col3; col3 = t;
    vTaskDelay(300 / portTICK_RATE_MS);
  }
}

/*-----------------------------------------------------------*/

xQueueHandle printQueue;

/**
 * Redirect stdout to the display
 */
void printTask(void *params) {
  unsigned char str[21] = "                    ";
  portTickType lastWakeTime = xTaskGetTickCount();
  int i;

  for (;;) {
    xSemaphoreTake(lcdLock, portMAX_DELAY);
    GLCD_setTextColor(Black);
    GLCD_displayStringLn(Line9, str);
    xSemaphoreGive(lcdLock);

    for (i = 0; i < 19; ++i)
	  str[i] = str[i+1];

    if (!xQueueReceive(printQueue, str + 19, 0))
	  str[19] = ' ';

	vTaskDelayUntil(&lastWakeTime, 100 / portTICK_RATE_MS);
  }
}

/* Retarget printing */
int fputc(int ch, FILE *f) {
  unsigned char c = ch;
  xQueueSend(printQueue, &c, 0);
  return ch;
}

/*-----------------------------------------------------------*/

/**
 * LED flashing
 */
void ledTask(void *params) {
  const u8 led_val[8] = { 0x01,0x03,0x07,0x0F,0x0E,0x0C,0x08,0x00 };
  int cnt = 0;

  for (;;) {
    LED_out (led_val[cnt]);
    cnt = (cnt + 1) % sizeof(led_val);
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}

/*-----------------------------------------------------------*/

/*
 * Entry point of program execution
 */
int main( void )
{
  prvSetupHardware();

  init_display();
  printQueue = xQueueCreate(128, 1);

  xTaskCreate(lcdTask, "lcd", 100, NULL, 1, NULL);
  xTaskCreate(printTask, "print", 100, NULL, 1, NULL);
  xTaskCreate(ledTask, "led", 100, NULL, 1, NULL);

  printf("Setup complete ");  // this is redirected to the display

  vTaskStartScheduler();

  assert(0);
  return 0;                 // not reachable
}

/*-----------------------------------------------------------*/
