#include "task_display.h"
#include "oscilloscope.h"
#include "GLCD.h"

portBASE_TYPE display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS] = {0};
portBASE_TYPE display_buffer_index[NUMBER_OF_CHANNELS] = {0};

static oscilloscope_mode_t display_mode = oscilloscope_mode_oscilloscope;

void task_display(void *args) {
	int i=0;
	while(1) {
		// Spin, read IPC, handle IPC
		display_sample(0, i);
		display_sample(1, (i+2048) % 4096);
		i = (i + 1) % 4096;
	}
	//assert(0);
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

void display_show_analog(portBASE_TYPE latitude, portBASE_TYPE longitude) {
	portBASE_TYPE x = DISPLAY_X_RES - latitude;
	portBASE_TYPE y = DISPLAY_Y_RES - longitude / (ADC_MAX / (DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT));
	GLCD_putPixel(x,y);
}
