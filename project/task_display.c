#include "api_controller.h"
#include "api_display.h"
#include "api_measure.h"
#include "api_watchdog.h"
#include "task_display.h"
#include "oscilloscope.h"
#include "GLCD.h"

uint16_t display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS] = {0};
int display_buffer_index[NUMBER_OF_CHANNELS] = {0};
portBASE_TYPE display_buffer_enable[NUMBER_OF_CHANNELS] = {0};

/* private functions */
static portBASE_TYPE handle_msg_subscribe_mode(msg_id_t id, msg_data_t *data);
static portBASE_TYPE handle_msg_subscribe_measure_data(msg_id_t id, msg_data_t *data);
static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *data);
static portBASE_TYPE handle_msg_toggle_channel(msg_id_t id, msg_data_t *data);

/* private variables */
static oscilloscope_mode_t display_mode = oscilloscope_mode_oscilloscope;
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_subscribe_mode,         handle_msg_subscribe_mode },
  { msg_id_subscribe_measure_data, handle_msg_subscribe_measure_data },
  { msg_id_display_cmd,            handle_msg_cmd },
  { msg_id_display_toggle_channel, handle_msg_toggle_channel }
};

void task_display(void *args)
{
  ipc_controller_subscribe(ipc_display, ipc_controller_variable_mode);
  ipc_measure_subscribe(ipc_display, ipc_measure_variable_data_ch0);
  ipc_measure_subscribe(ipc_display, ipc_measure_variable_data_ch1);

	while(1)
  {
    if(pdTRUE == ipc_get(
          ipc_display,
          portMAX_DELAY,
          msg_handle_table,
          sizeof(msg_handle_table)/sizeof(msg_handle_table[0])))
    {
      /* with no timeouts this should never happen so kill ourself */
      ipc_watchdog_signal_error(0);
    }
    else
    {
      ipc_watchdog_signal_error(0);
    }
  }
}

/* private functions */
static portBASE_TYPE handle_msg_subscribe_mode(msg_id_t id, msg_data_t *data)
{
	display_mode = data->subscribe_mode;
	display_redraw();
#if 0
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
#endif

  return pdTRUE;
}

static portBASE_TYPE handle_msg_subscribe_measure_data(msg_id_t id, msg_data_t *data)
{
  switch(data->subscribe_measure_data.ch)
  {
    case input_channel0:
		display_new_measure(0, data->subscribe_measure_data.data, data->subscribe_measure_data.timestamp);
      break;

    case input_channel1:
		display_new_measure(1, data->subscribe_measure_data.data, data->subscribe_measure_data.timestamp);
      break;

    default:
      return pdFALSE;
  }

  return pdTRUE;
}

static portBASE_TYPE handle_msg_cmd(msg_id_t id, msg_data_t *data)
{
  switch(data->display_cmd)
  {
    case display_cmd_toggle_freeze_screen:
      ipc_watchdog_signal_error(0);
      break;

    default:
      return pdFALSE;
  }
  return pdTRUE;
}
static portBASE_TYPE handle_msg_toggle_channel(msg_id_t id, msg_data_t *data)
{
  switch(data->msg_display_toggle_channel)
  {
    case input_channel0:
		display_buffer_enable[0] ^= 1;
      break;
    case input_channel1:
		display_buffer_enable[1] ^= 1;
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

void display_new_measure(char channel, uint16_t sample, int timestamp) {
	switch(display_mode) {
		case oscilloscope_mode_oscilloscope: 
			do {
				if((display_buffer_index[channel] +1) == timestamp)
					display_sample(channel, sample);
				else
					display_sample(channel, 0);
			} while(++display_buffer_index[channel] != timestamp);
			break;
		case oscilloscope_mode_multimeter: 
			display_sample(channel, sample);
			break;
		default:
			ipc_watchdog_signal_error(0);
			break;
	}
}


// New sample to display, regardless of mode
void display_sample(char channel, uint16_t sample) {
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

void display_show_analog(uint16_t x, uint16_t y) {
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
	xSemaphoreTake(lcdLock, portMAX_DELAY);
	GLCD_putPixel(disx,disy);
	xSemaphoreGive(lcdLock);
}
