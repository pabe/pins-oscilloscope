#ifndef __MAIN_H_
#define __MAIN_H_

#define NUMBER_OF_CHANNELS 2  //Number of probes on the oscilloscope

typedef struct {
  ADC_TypeDef* ADC;		  // ADC to use
  uint8_t ADC_Channel;	  // ADC Channel to use.. (ADC_Channel_0, ADC_Channel_1 ... 17
  int rate;               // Sample rate in Herz
  int subscribed;		  // 1 if has channel has subscribers.
  int inputChannel;		  // Input number "seen" on the oscilloscope.
} OscilloscopeChannel;

OscilloscopeChannel oChan[NUMBER_OF_CHANNELS];

int setSampleRate(int rate, int channel);
int getSampleRate(int channel);
int setSubscribe(int subscribe, int channel);
int getSubscribe(int channel);

#endif /* __MAIN_H_ */
