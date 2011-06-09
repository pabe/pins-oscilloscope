
#include "api_controller.h"
#include "api_display.h"
#include "task_display.h"
#include "task_watchdog.h"
#include "oscilloscope.h"
#include "GLCD.h"

portBASE_TYPE display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS] = {0};
portBASE_TYPE display_buffer_index[NUMBER_OF_CHANNELS] = {0};

/* private functions */
static portBASE_TYPE handle_msg_subscribe_mode(msg_id_t id, msg_data_t *data);

/* private variables */
static oscilloscope_mode_t display_mode = oscilloscope_mode_oscilloscope;
static const ipc_loop_t msg_handle_table[] =
{
  { msg_subscribe_mode, handle_msg_subscribe_mode }
};

void task_display(void *args)
{
  ipc_controller_subscribe(ipc_display, ipc_controller_variable_mode);

	while(1)
  {
    if(pdTRUE == ipc_get(
          ipc_display,
          portMAX_DELAY,
          msg_handle_table,
          sizeof(msg_handle_table)/sizeof(msg_handle_table[0])))
    {
      /* with no timeouts this should never happen so kill ourself */
      task_watchdog_signal_error();
      vTaskDelete(NULL);
    }
    else
    {
      task_watchdog_signal_error();
       vTaskDelete(NULL);
    }
  }
}
#if 0 /* orig code */
// Spin, read IPC, handle IPC
int i=0;
xSemaphoreTake(lcdLock, portMAX_DELAY);
display_sample(0, i);
display_sample(1, (i+2048) % 4096);
i = (i + 10) % 4096;
xSemaphoreGive(lcdLock);
vTaskDelay(1/portTICK_RATE_MS);
#endif

/* private functions */
static portBASE_TYPE handle_msg_subscribe_mode(msg_id_t id, msg_data_t *data)
{
  switch(data->subscribe_mode)
  {
    case oscilloscope_mode_oscilloscope:
      printf("|LOL0|");
      break;

    case oscilloscope_mode_multimeter:
      printf("|LOL1)");
      break;

    default:
      return pdFALSE;
  }

  return pdTRUE;
}

// Call when layout has changed
void display_redraw(void) {
	int channel, sample; //C89  ;__;
	switch(display_mode) {
		case oscilloscope_mode_oscilloscope:
			// Draw interface

			// Redraw buffers
			for(channel = 0; channel < NUMBER_OF_CHANNELS; channel++) {
				// Check if channel active?
				for(sample = 0; sample < DISPLAY_BUFF_SIZE; sample++) {
					display_sample(channel, display_buffer[display_buffer_index[channel]][channel]);
				}
			}
			break;
		case oscilloscope_mode_multimeter:
			// Draw interface
			// Output last measurement?
			break;
		default:
			// Whoops?
			break;
	}
}

// New sample to display, regardless of mode
void display_sample(portBASE_TYPE channel, portBASE_TYPE sample) {
	switch(display_mode) {
		// Draw new pixel
		case oscilloscope_mode_oscilloscope:
			// Remove old pixel
			GLCD_setTextColor(Black);
			display_show_analog(display_buffer_index[channel], display_buffer[display_buffer_index[channel]][channel]);

			// Display new pixel
			GLCD_setTextColor(channel ? Green : Magenta);
			display_show_analog(display_buffer_index[channel], sample);

			// Update buffer
			display_buffer[display_buffer_index[channel]][channel] = sample;
			display_buffer_index[channel]++;
			display_buffer_index[channel] %= DISPLAY_BUFF_SIZE;

			break;

		// Update number
		case oscilloscope_mode_multimeter:
			break;

		// Whoops?
		default:
			break;
	}
}

void display_show_analog(portBASE_TYPE x, portBASE_TYPE y) {
	// Stupid display has all messed up coordinate system :/
	// We pretend this is not the case...
	//          <- y
	// +-----------+ x
	// |           | |
	// |           | v
	// |           |
	// +-----------+
	portBASE_TYPE disx = (DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT) - y / (ADC_MAX / (DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT));
	portBASE_TYPE disy = DISPLAY_X_RES - x;
	GLCD_putPixel(disx,disy);
}
