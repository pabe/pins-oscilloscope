#ifndef __MEASURE_H_
#define __MEASURE_H_
#include "stm32f10x_adc.h"
#define NUMBER_OF_CHANNELS 2  //Number of probes on the oscilloscope

#define ADC_MAX ((1<<12)-1)

typedef struct {
  ADC_TypeDef* ADC;		  // ADC to use
  uint8_t ADC_Channel;	  // ADC Channel to use.. (ADC_Channel_0, ADC_Channel_1 ... 17
  int rate;               // Sample rate in Herz
  int subscribed;		  // 1 if has channel has subscribers.
  int inputChannel;		  // Input number "seen" on the oscilloscope.
} OscilloscopeChannel;

void measureInit(void);

int setSampleRate(int rate, int channel);
int getSampleRate(int channel);
int setSubscribe(int subscribe, int channel);
int getSubscribe(int channel);

#endif /* __MEASURE_H_ */
