#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "FreeRTOS.h"
#include "oscilloscope.h"
#include "task_measure.h"
#include "semphr.h"
#include "task.h"

#define DISPLAY_X_RES 320
#define DISPLAY_Y_RES 240

#define DISPLAY_MENU_HEIGHT 50
#define NUM_BUTTONS 6
#define NUM_MENU_BUTTONS 5

#define MODE_OSCILLOSCOPE (1<<0)
#define MODE_MULTIMETER (1<<1)

#define DISPLAY_BUFF_SIZE DISPLAY_X_RES

typedef struct
{
  uint16_t 	left;
  uint16_t 	right;
  uint16_t 	upper;
  uint16_t 	lower;
  char		*text;
} button, *Pbutton;

void setup_buttons(void);
Pbutton get_button(u16 button);

void task_display(void *args);
void display_redraw(void);
void display_sample(char channel, uint16_t sample);
void display_show_analog(uint16_t x, uint16_t y);
void display_new_measure(char channel, uint16_t sample, int timestamp);
void display_button(int button);
void display_buttons(void);

extern uint16_t display_buffer[DISPLAY_BUFF_SIZE][NUMBER_OF_CHANNELS]; //This goes to the storage module eventualy
extern int display_buffer_index[]; //This goes to the storage module eventualy

extern xSemaphoreHandle lcdLock;





#endif /* __DISPLAY_H_ */
