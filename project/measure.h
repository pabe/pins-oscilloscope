#ifndef __MEASURE_H_
#define __MEASURE_H_
#include "stm32f10x_adc.h"
#include "oscilloscope.h"
#define NUMBER_OF_CHANNELS 2  //Number of probes on the oscilloscope

#define ADC_MAX ((1<<12)-1)

typedef struct {
  ADC_TypeDef* ADC;		  // ADC to use
  uint8_t ADC_Channel;	  // ADC Channel to use.. (ADC_Channel_0, ADC_Channel_1 ... 17
  int rate;               // Sample rate in Herz
  int subscribed;		  // 1 if has channel has subscribers.
  oscilloscope_input_t inputChannel;		  // Input number "seen" on the oscilloscope.
} OscilloscopeChannel;

void measureInit(void);
void measureTask (void* params);
int setSampleRate(int rate, oscilloscope_input_t channel);
int getSampleRate(oscilloscope_input_t channel);
int setSubscribe(int subscribe, oscilloscope_input_t channel);
int getSubscribe(oscilloscope_input_t channel);

#endif /* __MEASURE_H_ */
