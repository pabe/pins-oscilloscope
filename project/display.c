void task_display(void *args) {
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
			break;
		case MODE_MULTIMETER:
			// Update number
			break;
		default:
			// Whoops?
			break;
	}
}
