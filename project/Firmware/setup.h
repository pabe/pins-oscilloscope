

#ifndef SETUP_H
#define SETUP_H

// #include <stm32f10x_type.h>
#include <stm32f10x.h>

/*
 * Configure the clocks, GPIO and other peripherals as required by the demo.
 */
void prvSetupHardware( void );

void LED_out (u32 val);

#endif
