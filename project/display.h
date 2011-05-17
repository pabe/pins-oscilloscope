#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#define DISPLAY_X_RES 320
#define DISPLAY_Y_RES 240

void task_display(void *args);
void display_redraw();
void display_sample(portBASE_TYPE channel, portBASE_TYPE sample);

static portBASE_TYPE display_mode; //This should really be global

#endif /* __DISPLAY_H_ */
