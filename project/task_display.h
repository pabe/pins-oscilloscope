#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#define DISPLAY_X_RES 320
#define DISPLAY_Y_RES 240

#define DISPLAY_MENU_HEIGHT 40

#define MODE_OSCILLOSCOPE (1<<0)
#define MODE_MULTIMETER (1<<1)

#define DISPLAY_BUFF_SIZE DISPLAY_X_RES

//Change this later
#define display_mode MODE_OSCILLOSCOPE

void task_display(void *args);
void display_redraw();
void display_sample(portBASE_TYPE channel, portBASE_TYPE sample);

static portBASE_TYPE display_mode; //This should really be global

extern portBASE_TYPE display_buffer[][]; //This goes to the storage module eventualy
extern portBASE_TYPE display_buffer_index[]; //This goes to the storage module eventualy
#endif /* __DISPLAY_H_ */
