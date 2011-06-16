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

#include "config.h"
#include "api_controller.h" 
#include "api_measure.h"
#include "api_watchdog.h" 
#include "ipc.h" 
#include "task_measure.h" 
#include "task_display.h" 
#include "task_input_gpio.h" 
#include "task_input_touch.h" 
#include "task_controller.h" 
#include "task_watchdog.h" 

xSemaphoreHandle lcdLock; 

static void initDisplay () { 
  /* LCD Module init */ 

  lcdLock = xSemaphoreCreateMutex(); 

  GLCD_init(); 
  GLCD_clear(White); 
} 
/*-----------------------------------------------------------*/ 

#ifdef CONFIG_ENABLE_PRINTER
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
#endif

/* Retarget printing to the serial port 1 */ 
int fputc(int ch, FILE *f) { 
  unsigned char c = ch; 
#ifdef CONFIG_ENABLE_PRINTER
  xQueueSend(printQueue, &c, 0); 
#endif
  return ch; 
} 

/*-----------------------------------------------------------*/ 

/* 
 * Entry point of program execution 
 */ 
int main( void ) 
{ 
  prvSetupHardware(); 
  IOE_Config(); 

#ifdef CONFIG_ENABLE_PRINTER
  printQueue = xQueueCreate(128, 1);
#endif

  setup_buttons();
  initDisplay();  
  ipc_measure_init();


  if(pdFALSE == ipc_init()) 
  { 
    ipc_watchdog_signal_error(0);
  }

  if(pdFALSE == task_measure_init())
  { 
    ipc_watchdog_signal_error(0);
  }

#ifdef CONFIG_ENABLE_PRINTER
  xTaskCreate(printTask, "print", 100, NULL, 1, NULL); 
#endif

  xTaskCreate(task_controller, "Controller", 100, NULL, 1, NULL); 
  xTaskCreate(task_watchdog, "Watchdog driver", 100, NULL, 1, NULL); 
  xTaskCreate(task_input_gpio, "Input driver for GPIO", 100, NULL, 1, NULL); 
  xTaskCreate(task_input_touch, "Input driver for touchscreen", 200, NULL, 1, NULL); 
  xTaskCreate(task_display, "Display", 300, NULL, 1, NULL); 
  xTaskCreate(task_measure_cmd, "ADC-driver: controller", 100, NULL, 1, NULL); 
  xTaskCreate(task_measure, "ADC-driver: IRQ interface", 100, NULL, 1, NULL); 

  vTaskStartScheduler(); 

  ipc_watchdog_signal_error(0);
  while(1);
} 

/*-----------------------------------------------------------*/ 
