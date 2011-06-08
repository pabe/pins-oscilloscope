#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "FreeRTOS.h"
#include "oscilloscope.h"
#include "measure.h"
#include "semphr.h"
#include "task.h"

#define DISPLAY_X_RES 320
#define DISPLAY_Y_RES 240

#define DISPLAY_MENU_HEIGHT 40

#define MODE_OSCILLOSCOPE (1<<0)
#define MODE_MULTIMETER (1<<1)

#define DISPLAY_BUFF_SIZE DISPLAY_X_RES

void task_display(void *args);
void display_redraw(void);
void display_sample(portBASE_TYPE channel, portBASE_TYPE sample);
void display_show_analog(portBASE_TYPE latitude, portBASE_TYPE longitude);

extern portBASE_TYPE display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS]; //This goes to the storage module eventualy
extern portBASE_TYPE display_buffer_index[]; //This goes to the storage module eventualy

extern xSemaphoreHandle lcdLock;
#endif /* __DISPLAY_H_ */
