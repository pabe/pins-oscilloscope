/**
 * Program skeleton for the course "Programming embedded systems"
 */

#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "setup.h"
#include "assert.h"
#include "GLCD.h"
#include "stm3210c_eval_ioe.h"

/*-----------------------------------------------------------*/

#include "task_input_gpio.h"
#include "task_controller.h"
#include "task_watchdog.h"
#include "api_controller.h"
#include "api_watchdog.h"
#include "measure.h"
#include "task_display.h"
#define WIDTH 320

xSemaphoreHandle lcdLock;

static void initDisplay () {
  /* LCD Module init */

  lcdLock = xSemaphoreCreateMutex();

  GLCD_init();
  GLCD_clear(White);
  GLCD_setTextColor(Blue);
  GLCD_displayStringLn(Line2, " Programming");
  GLCD_displayStringLn(Line3, " Embedded");
  GLCD_displayStringLn(Line4, " Systems");
}

static void lcdTask(void *params) {
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

/**
 * Display stdout on the display
 */

xQueueHandle printQueue;

static void printTask(void *params) {
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

/* Retarget printing to the serial port 1 */
int fputc(int ch, FILE *f) {
  unsigned char c = ch;
  xQueueSend(printQueue, &c, 0);
  return ch;
}

/*-----------------------------------------------------------*/

/**
 * Register a callback that will be invoked when a touch screen
 * event occurs within a given rectangle
 *
 * NB: the callback function should have a short execution time,
 * since long-running callbacks will prevent further events from
 * being generated
 */

typedef struct {
  u16 lower, upper, left, right;
  void *data;
  void (*callback)(u16 x, u16 y, u16 pressure, void *data);
} TSCallback;

static TSCallback callbacks[16];
static u8 callbackNum = 0;

void registerTSCallback(u16 left, u16 right, u16 lower, u16 upper,
                        void (*callback)(u16 x, u16 y, u16 pressure, void *data),
						void *data) {
  callbacks[callbackNum].lower    = lower;
  callbacks[callbackNum].upper    = upper;
  callbacks[callbackNum].left     = left;
  callbacks[callbackNum].right    = right;
  callbacks[callbackNum].callback = callback;
  callbacks[callbackNum].data     = data;
  callbackNum++;
}

static void touchScreenTask(void *params) {
  portTickType lastWakeTime = xTaskGetTickCount();
  TS_STATE *ts_state;
  u8 pressed = 0;
  u8 i;

  for (;;) {
    ts_state = IOE_TS_GetState();

	if (pressed) {
	  if (!ts_state->TouchDetected)
	    pressed = 0;
	} else if (ts_state->TouchDetected) {
	  for (i = 0; i < callbackNum; ++i) {
		if (callbacks[i].left  <= ts_state->X &&
		    callbacks[i].right >= ts_state->X &&
		    callbacks[i].lower >= ts_state->Y &&
		    callbacks[i].upper <= ts_state->Y)
		  callbacks[i].callback(ts_state->X, ts_state->Y, ts_state->Z,
		                        callbacks[i].data);
	  }													
	  pressed = 1;
	}

    if (ts_state->TouchDetected) {
	  printf("%d,%d,%d ", ts_state->X, ts_state->Y, ts_state->Z);
	}

	vTaskDelayUntil(&lastWakeTime, 100 / portTICK_RATE_MS);
  }
}

/*-----------------------------------------------------------*/

xQueueHandle buttonQueue;

static void highlightButton(u16 x, u16 y, u16 pressure, void *data) {
  u16 d = (int)data;
  xQueueSend(buttonQueue, &d, 0);
}

static void setupButtons(void) {
  u16 i;
  buttonQueue = xQueueCreate(4, sizeof(u16));
  
  for (i = 0; i < 3; ++i) {
    GLCD_drawRect(30 + 60*i, 30, 40, 40);
	registerTSCallback(WIDTH - 30 - 40, WIDTH - 30, 30 + 60*i + 40, 30 + 60*i,
	                   &highlightButton, (void*)i);
  }
}

static void highlightButtonsTask(void *params) {
  u16 d;

  for (;;) {
    xQueueReceive(buttonQueue, &d, portMAX_DELAY);

    xSemaphoreTake(lcdLock, portMAX_DELAY);
    GLCD_setTextColor(Red);
    GLCD_fillRect(31 + 60*d, 31, 38, 38);
    GLCD_setTextColor(Blue);
    xSemaphoreGive(lcdLock);

	vTaskDelay(500 / portTICK_RATE_MS);

    xSemaphoreTake(lcdLock, portMAX_DELAY);
    GLCD_setTextColor(White);
    GLCD_fillRect(31 + 60*d, 31, 38, 38);
    GLCD_setTextColor(Blue);
    xSemaphoreGive(lcdLock);
  }
}

/*-----------------------------------------------------------*/

/*
 * Entry point of program execution
 */
int main( void )
{
  prvSetupHardware();
  IOE_Config();

  printQueue = xQueueCreate(128, 1);

  initDisplay();
  setupButtons();
  
  if(pdFALSE == ipc_init())
  {
    /* TODO: Output error mesg? */
    task_watchdog_signal_error();
  }

  xTaskCreate(lcdTask, "lcd", 100, NULL, 1, NULL);
  xTaskCreate(printTask, "print", 100, NULL, 1, NULL);
  xTaskCreate(touchScreenTask, "touchScreen", 100, NULL, 1, NULL);
  xTaskCreate(highlightButtonsTask, "highlighter", 100, NULL, 1, NULL);
  xTaskCreate(task_controller, "Controller", 100, NULL, 1, NULL);
  xTaskCreate(task_watchdog, "Watchdog driver", 100, NULL, 1, NULL);
  xTaskCreate(task_input_gpio, "Input driver for GPIO", 100, NULL, 1, NULL);
  xTaskCreate(task_display, "Display", 100, NULL, 1, NULL);


  printf("Setup complete ");  // this is redirected to the display

  vTaskStartScheduler();

  assert(0);
  return 0;                 // not reachable
}

/*-----------------------------------------------------------*/
