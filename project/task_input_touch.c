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
#include "semphr.h"

#include "api_input_touch.h"
#include "api_controller.h"
#include "api_watchdog.h"
#include "api_display.h"
#include "task_input_touch.h"
#include "oscilloscope.h"
#include "task_display.h"



/*-------------------------------------*/

#define WIDTH 320 

/* public variables */

/* private variables */
static oscilloscope_mode_t mode;
extern xSemaphoreHandle lcdLock;
/* private functions */
static void btnPressHandler(void *data);
static void registerButtonsCallback(void);
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
        void (*callback)(void *data), void *data) {
    callbacks[callbackNum].lower = lower;
    callbacks[callbackNum].upper = upper;
    callbacks[callbackNum].left = left;
    callbacks[callbackNum].right = right;
    callbacks[callbackNum].callback = callback;
    callbacks[callbackNum].data = data;
    callbackNum++;
}

static void btnPressOscMode(u16 btn) {
    //printf("btn:%d Mode:%d", btn, mode);
    switch (btn) {
        case 0:
            ipc_display_toggle_freeze_screen();
            break;

        case 1:
            ipc_controller_mode_toggle();
            break;

        case 2:
            ipc_controller_time_axis_decrease();
            break;

        case 3:
            ipc_controller_time_axis_increase();
            break;
        case 4:
            ipc_display_toggle_channel(input_channel0);
            break;

        case 5:
            ipc_display_toggle_channel(input_channel1);
            break;

        default:
            printf("unhandled button");
            ipc_watchdog_signal_error(0);
    }
}

static void btnPressVmMode(u16 btn) {
    //printf("btn:%d Mode:%d", btn, mode);
    switch (btn) {
        case 0:
            ipc_display_toggle_freeze_screen();
            break;
        case 1:
            ipc_controller_mode_toggle();
            break;

        case 2:

            break;
        case 3:

            break;
        case 4:

            break;
        case 5:

            break;
        default:
            printf("unhandled button");
            ipc_watchdog_signal_error(0);
    }
}

static void registerButtonsCallback(void) {
    u16 i;
    Pbutton btn = NULL;

    xSemaphoreTake(lcdLock, portMAX_DELAY);
    for (i = 0; i < 6; ++i) {
        btn = get_button(i);
        //GLCD_drawRect(0 + 40 * i, 30, 40, 40);
        //registerTSCallback(WIDTH - 30 - 40, WIDTH - 30, 0 + 40 * i + 40, 0 + 40 * i, &btnPressHandler, (void*) i);
        //printf("btn%d   l%d     r%d     low%d     up%d     ", i,btn->left, btn->right, btn->lower, btn->upper);
        registerTSCallback(WIDTH - btn->left, WIDTH - btn->right, btn->lower, btn->upper, &btnPressHandler, (void*) i);
    }
    xSemaphoreGive(lcdLock);
}

static void btnPressHandler(void *data) {
    u16 button;
    button = (int) data;
    //printf("btn:%d Mode:%d", button, mode);
    switch (mode) {
        case oscilloscope_mode_oscilloscope:
            btnPressOscMode(button);
            break;

        case oscilloscope_mode_multimeter:
            btnPressVmMode(button);
            break;
    }
}

/* public functions */
void task_input_touch(void *p) {
    TS_STATE *ts_state;
    static u8 pressed = 0;
    static u8 i;
    portTickType timeout = CFG_TASK_INPUT_TOUCH__POLLING_PERIOD;

    /* subscribe to mode variable in the controller, returns pd(TRUE|FALSE) */
    ipc_controller_subscribe(ipc_input_touch, ipc_controller_variable_mode);
    registerButtonsCallback();

    while (1) {
        portTickType sleep_time;
        msg_t msg;

        sleep_time = xTaskGetTickCount();

        assert(ipc_input_touch);
        if (pdFALSE == xQueueReceive(ipc_input_touch, &msg, timeout)) {
            /* timeout work */
            xSemaphoreTake(lcdLock, portMAX_DELAY);
            ts_state = IOE_TS_GetState();
            xSemaphoreGive(lcdLock);

            if (pressed) {
                if (!ts_state->TouchDetected)
                    pressed = 0;
            } else if (ts_state->TouchDetected) {
                for (i = 0; i < callbackNum; ++i) {
                    if (callbacks[i].left <= ts_state->X &&
                            callbacks[i].right >= ts_state->X &&
                            callbacks[i].lower >= ts_state->Y &&
                            callbacks[i].upper <= ts_state->Y) {
                        callbacks[i].callback(callbacks[i].data);
                    }
                }
                pressed = 1;
            }

            /*if (ts_state->TouchDetected) {
                } */

        } else {
            switch (msg.head.id) {
                case msg_id_subscribe_mode:
                    //printf("| MODE: %i |", msg.data.subscribe_mode);
                    mode = msg.data.subscribe_mode;
                    //printf("mymode:%d ", mode);
                    break;

                default:
                    ipc_watchdog_signal_error(0);
            }

            /* recalculate timeout */
            timeout = timeout - (xTaskGetTickCount() - sleep_time);
        }
    }
}

/* private functions */
