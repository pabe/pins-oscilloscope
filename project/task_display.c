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
			// Redraw buffers?
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
		case MODE_OSCILLOSCOPE:
			// Draw new pixel
			portBASE_TYPE x, y;

			x = DISPLAY_X_RES - display_buffer_index[channel] - 1;
			y = DISPLAY_Y_RES - sample / (ADC_MAX / DISPLAY_Y_RES);

			GLCD_setTextColor(channel ? Green : Magenta);
			GLCD_putPixel(y,x);

			display_buffer[display_buffer_index[channel]][channel] = sample;
			display_buffer_index[channel]++;
			display_buffer_index[channel] %= DISPLAY_BUFF_SIZE;

			break;
		case MODE_MULTIMETER:
			// Update number
			break;
		default:
			// Whoops?
			break;
	}
}
