/*
 * oscilloscope.h:
 *
 * Structures for the project.
 */

#ifndef __OSCILLOSCOPE__H_
#define __OSCILLOSCOPE__H_

typedef enum
{
  oscilloscope_mode_oscilloscope,
  oscilloscope_mode_multimeter
} oscilloscope_mode_t;

typedef enum
{
  input_channel0,
  input_channel1
} oscilloscope_input_t;

#endif /* __OSCILLOSCOPE__H_ */
