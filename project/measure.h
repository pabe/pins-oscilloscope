#ifndef __MEASURE_H_
#define __MEASURE_H_

#define NUMBER_OF_CHANNELS 2  //Number of probes on the oscilloscope


typedef struct {
  ADC_TypeDef* ADC;		  // ADC to use
  uint8_t ADC_Channel;	  //Channel to use.. (ADC_Channel_0, ADC_Channel_1 ...

  int rate;               // Sample rate in Herz
  int subscribed;		  // 1 if has subscribers.
  int inputChannel;		  // Input number "seen" on the oscilloscope.
} OscilloscopeChannel;

OscilloscopeChannel oChan[NUMBER_OF_CHANNELS] ;


#endif /* __MEASURE_H_ */
