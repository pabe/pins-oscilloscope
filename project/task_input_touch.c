/*
 * Input driver for touchscreen.
 */




#include <stdio.h>

/* Firmware */
#include "assert.h"

/* FreeRTOS */
#include "FreeRTOS.h"
#include "GLCD.h"
#include "stm3210c_eval_ioe.h"
#include "task.h"

#include "api_input_touch.h"
#include "api_controller.h"
#include "task_input_touch.h"
#include "task_watchdog.h"
#include "oscilloscope.h"

 /*--- ONLY for testing purposes remove for release Drawing in this task---*/
#include "semphr.h"

 /*-------------------------------------*/

#define WIDTH 320 

/* public variables */

/* private variables */

/* private functions */
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
  void (*callback)(void *data);
} TSCallback;

static TSCallback callbacks[16];
static u8 callbackNum = 0;

void registerTSCallback(u16 left, u16 right, u16 lower, u16 upper,
                        void (*callback)(void *data),
						void *data) {
  callbacks[callbackNum].lower    = lower;
  callbacks[callbackNum].upper    = upper;
  callbacks[callbackNum].left     = left;
  callbacks[callbackNum].right    = right;
  callbacks[callbackNum].callback = callback;
  callbacks[callbackNum].data     = data;
  callbackNum++;
}

static void printButton(void *data){
  u16 button;
  button = (int)data;
  printf("I am button:%d ", button);
}

static void setupButtons(void) { 
  extern xSemaphoreHandle lcdLock; 
  u16 i; 
  xSemaphoreTake(lcdLock, portMAX_DELAY);
  for (i = 0; i < 3; ++i) { 
    GLCD_drawRect(30 + 60*i, 30, 40, 40); 
	registerTSCallback(WIDTH - 30 - 40, WIDTH - 30, 30 + 60*i + 40, 30 + 60*i, 
	                   &printButton, (void*)i); 
  }
  xSemaphoreGive(lcdLock); 
} 

/* public functions */
void task_input_touch(void *p)
{
  TS_STATE *ts_state;
  static oscilloscope_mode_t mode;
  static u8 pressed = 0;
  static u8 i;
  portTickType timeout = CFG_TASK_INPUT_TOUCH__POLLING_PERIOD;

  /* subscribe to mode variable in the controller, returns pd(TRUE|FALSE) */
  ipc_controller_subscribe(ipc_input_touch, ipc_controller_variable_mode);
  setupButtons();

  while(1)
  {
    portTickType sleep_time;
    msg_t msg;

    sleep_time = xTaskGetTickCount();
    
    assert(ipc_input_touch);
    if(pdFALSE == xQueueReceive(ipc_input_touch, &msg, timeout))
    {
      /* timeout work */
	ts_state = IOE_TS_GetState();

	if (pressed) {
	  if (!ts_state->TouchDetected)
	    pressed = 0;
	} else if (ts_state->TouchDetected) {
	  for (i = 0; i < callbackNum; ++i) {
		if (callbacks[i].left  <= ts_state->X &&
		    callbacks[i].right >= ts_state->X &&
		    callbacks[i].lower >= ts_state->Y &&
		    callbacks[i].upper <= ts_state->Y){
		       callbacks[i].callback(callbacks[i].data);
					 }
	  }													
	  pressed = 1;
	}

    if (ts_state->TouchDetected) {
	  //printf("%d,%d,%d ", ts_state->X, ts_state->Y, ts_state->Z);
	}


    }
    else
    {
      switch(msg.head.id)
      {
        case msg_id_subscribe_mode:
          //printf("| MODE: %i |", msg.data.subscribe_mode);
		  mode = msg.data.subscribe_mode;
		  printf("mymode:%d ", mode);
          break;

        default:
          /* TODO: Output error mesg? */
          task_watchdog_signal_error();
      }

      /* recalculate timeout */
      timeout = timeout - (xTaskGetTickCount() - sleep_time);
    }
  }
}

/* private functions */
