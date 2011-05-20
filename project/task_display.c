#include "task_display.h"

portBASE_TYPE display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS] = {0};
portBASE_TYPE display_buffer_index[NUMBER_OF_CHANNELS] = {0};

void task_display(void *args) {
	struct ipc_msg message;
	while(1) {
		// Spin, read IPC, handle IPC
	}
	assert(0);
}

// Call when layout has changed
void display_redraw() {
	switch(display_mode) {
		case MODE_OSCILLOSCOPE:
			// Draw interface

			// Redraw buffers
			for(int channel = 0; channel < NUMBER_OF_CHANNELS; channel++) {
				// Check if channel active?
				for(int sample = 0; i < DISPLAY_BUFF_SIZE; i++) {
					display_sample(channel, display_buffer_index[channel]);
				}
			}
			break;
		case MODE_MULTIMETER:
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
		case MODE_OSCILLOSCOPE:
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
		case MODE_MULTIMETER:
			break;

		// Whoops?
		default:
			break;
	}
}

void display_show_analog(portBASE_TYPE latitude, portBASE_TYPE longitude) {
	portBASE_TYPE x = DISPLAY_X_RES - latitude
	portBASE_TYPE y = DISPLAY_Y_RES - longitude / (ADC_MAX / (DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT));
	GLCD_putPixel(x,y);
}
