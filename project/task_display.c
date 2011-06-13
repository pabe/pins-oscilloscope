#include <stdio.h>
#include "api_controller.h"
#include "api_display.h"
#include "api_measure.h"
#include "api_watchdog.h"
#include "ipc.h"
#include "task_display.h"
#include "oscilloscope.h"
#include "GLCD.h"

uint16_t display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS] = {0};
int display_buffer_index[NUMBER_OF_CHANNELS] = {0};
button buttons[NUM_BUTTONS];
char display_buffer_enable[NUMBER_OF_CHANNELS] = {0};
int lastWrittenTimeStamp;


/* private functions */
static portBASE_TYPE handle_msg_subscribe_mode(msg_id_t id, msg_data_t *data);
static portBASE_TYPE handle_msg_subscribe_measure_data(msg_id_t id, msg_data_t *data);
// Is this old and should be removed?
static portBASE_TYPE handle_msg_subscribe_measure_rate(msg_id_t id, msg_data_t *data);
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
  ipc_measure_subscribe(ipc_display,    ipc_measure_variable_data_ch0);
  ipc_measure_subscribe(ipc_display,    ipc_measure_variable_data_ch1);

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
#define display_index(channel) (display_buffer_index[channel] % DISPLAY_BUFF_SIZE)
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

	static	uint16_t data2[CONFIG_SAMPLE_BUFFER_SIZE];
static portBASE_TYPE handle_msg_subscribe_measure_data(msg_id_t id, msg_data_t *data)
{
	int channel;
	switch(data->subscribe_measure_data.ch)
	{
#if 0
		case input_channel0:
			display_new_measure(0, data->subscribe_measure_data.data, data->subscribe_measure_data.timestamp);
			break;

		case input_channel1:
			display_new_measure(1, data->subscribe_measure_data.data, data->subscribe_measure_data.timestamp);
			break;
#endif
		case input_channel0:
			channel = 0;
			break;
		case input_channel1:
      return pdTRUE;
			channel = 1;
			break;
		default:
			return pdFALSE;
	}

	// Fall through to here if all is well
	{
		int i;
		int timestamp;
		ipc_measure_get_data(data2, &timestamp);

		for(i=0;i<CONFIG_SAMPLE_BUFFER_SIZE;i++)
		{
      display_new_measure(channel, data2[i], timestamp+i);
		}
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
	char channel;
	uint16_t sample; //C89  ;__;
	xSemaphoreTake(lcdLock, portMAX_DELAY);
	GLCD_clear(White); 	  //Is this the right place to do this?
	xSemaphoreGive(lcdLock);
	switch(display_mode) {
		case oscilloscope_mode_oscilloscope:
			// Draw interface
			display_buttons();

			// Redraw buffers
#if 1
			for(channel = 0; channel < NUMBER_OF_CHANNELS; channel++) {
				// Check if channel active
				if(display_buffer_enable[channel])
					for(sample = 0; sample < DISPLAY_BUFF_SIZE; sample++) {
						display_sample(channel, display_buffer[sample][channel]);
					}
			}
#endif 
			break;
		case oscilloscope_mode_multimeter:
			// Draw interface
			display_buttons();
			// Output last measurement?
			break;
		default:
			// Whoops?
			break;
	}
}

void display_new_measure(char channel, uint16_t sample, int timestamp) {
	//printf("[%d,%d,%d=%d]", channel, sample, timestamp, display_buffer_index[channel]);
	switch(display_mode) {
		case oscilloscope_mode_oscilloscope: 
			display_sample(channel, sample);
			break;
		case oscilloscope_mode_multimeter: 
			
			if(timestamp - lastWrittenTimeStamp > 1000){ //Should probably depend on rate.
				display_sample(channel, sample);
				lastWrittenTimeStamp = timestamp;
			}
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
			xSemaphoreTake(lcdLock, portMAX_DELAY);

			//GLCD_setTextColor(Black);
			GLCD_setTextColor(White);
			display_show_analog(display_index(channel), display_buffer[display_index(channel)][channel]);

			// Display new pixel
			GLCD_setTextColor(channel ? Green : Magenta);
			display_show_analog(display_index(channel), sample);

			xSemaphoreGive(lcdLock);

			// Update buffer
			display_buffer[display_index(channel)][channel] = sample;
			display_buffer_index[channel]++;

			break;

			// Update number
		case oscilloscope_mode_multimeter:
			display_WriteMultimerDigit(channel, sample);
			break;

			// Whoops?
		default:
			break;
	}
}

void display_show_analog(uint16_t x, uint16_t y) {
	// REMEMBER TO GRAB LOCK BEFORE CALLING!
	
	// Stupid display has all messed up coordinate system :/
	// We pretend this is not the case...
	//          <- y
	// +-----------+ x
	// |           | |
	// |           | v
	// |           |
	// +-----------+
	uint16_t disx = (DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT) - y / (ADC_MAX / (DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT));
	uint16_t disy = DISPLAY_X_RES - x;
	GLCD_putPixel(disx,disy);
}

void setup_buttons(void){
	int i;
	char* btn_strings[] = {"M","-","+","A", "B"}; 

	/*Setup screen button*/
		buttons[0].upper = 0+5;
		buttons[0].lower = DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT - 10;// 5 Padding btw menu screen
		buttons[0].left = DISPLAY_X_RES-5; 
		buttons[0].right = 0+5;
		buttons[0].text = "";

	for (i=0; i<NUM_MENU_BUTTONS;i++){ //Button 0 is screen!
		buttons[i+1].upper = DISPLAY_Y_RES - DISPLAY_MENU_HEIGHT+1;
		buttons[i+1].lower = DISPLAY_Y_RES;
		buttons[i+1].left = DISPLAY_X_RES - i *	DISPLAY_X_RES / NUM_MENU_BUTTONS;
		//buttons[i+1].right = DISPLAY_X_RES - DISPLAY_X_RES / NUM_MENU_BUTTONS - i * DISPLAY_X_RES / NUM_MENU_BUTTONS;
		buttons[i+1].right = DISPLAY_X_RES - (1+i) * DISPLAY_X_RES / NUM_MENU_BUTTONS;
		buttons[i+1].text =	btn_strings[i];
	}
};

Pbutton get_button(u16 btn){
	return &buttons[btn];
}

void display_button(int button) {
	xSemaphoreTake(lcdLock, portMAX_DELAY);
	GLCD_setTextColor(Black);
	GLCD_drawRect(buttons[button].upper, buttons[button].right, buttons[button].lower-buttons[button].upper, buttons[button].left-buttons[button].right);
	GLCD_displayChar(buttons[button].upper + 10, buttons[button].right + 20, *(buttons[button].text));
	xSemaphoreGive(lcdLock);
}

void display_buttons(void) {
	int i;
	switch(display_mode) {
		case oscilloscope_mode_oscilloscope:
			for(i = 0; i<NUM_MENU_BUTTONS; i++)
				display_button(i+1);
			break;
		case oscilloscope_mode_multimeter:
			display_button(1);	  //"M"
			display_button(4);	  //"A"
			display_button(5);	  //"B"
			break;
		default:
			//Whoops?
			break;
	}
}

double voltageConversion(uint16_t val){
	double maxAdcBits = 4095.0; // Should be > 0 
	double maxVolts = 3.3;      
	double voltsPerBit = (maxVolts / (maxAdcBits));
	return (double)val * voltsPerBit;
}

void display_WriteMultimerDigit(char channel, uint16_t sample) {
char buffer[15];
int Line = 0;

	switch (channel){
   		case  input_channel0:
			Line = Line3;
			//sprintf (buffer, "Chan A: %.1f", voltageConversion(sample));
			sprintf (buffer, "Chan A: %d", sample);
			break;
   		case  input_channel1:
   			Line = Line4;
			//sprintf (buffer, "Chan B: %.1f", voltageConversion(sample));
			sprintf (buffer, "Chan B: %d", sample);

			break;
		default:
			//Whoops?
		break;
   }
   			
   xSemaphoreTake(lcdLock, portMAX_DELAY);
   GLCD_setTextColor(Black);
   GLCD_displayStringLn(Line, (unsigned char *) buffer); 
   xSemaphoreGive(lcdLock);
}

