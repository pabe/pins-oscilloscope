#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "assert.h"
#include "measure.h"
#include "queue.h"
//#include "stm3210c_eval_ioe.h"
#include "stm32f10x_adc.h"


	

 void channelConfig(OscilloscopeChannel OChannel) {
/*	ADC_RegularChannelConfig(OChannel.ADC,
 	OChannel.ADC_Channel,
  	1,		//Rank
  	ADC_SampleTime_239Cycles5); //Fix calculation from herz to ADC_Sampletime */
}  


uint16_t readChannel(OscilloscopeChannel OChannel){
		channelConfig(OChannel);
	/*	ADC_ClearFlag(OChannel.ADC, ADC_FLAG_EOC);
		ADC_SoftwareStartConvCmd(OChannel.ADC, ENABLE);
		while (!ADC_GetFlagStatus(OChannel.ADC, ADC_FLAG_EOC));	
			
		
		return ADC_GetConversionValue(OChannel.ADC);
	*/	
		return 0; //Sholud be a value FIX !! 
	
	
	}



int setSampleRate(int rate, int channel){
	int cntr = 0;
 	while(cntr < NUMBER_OF_CHANNELS){
		if(oChan[cntr].inputChannel == channel);
			oChan[cntr].rate=rate;
		cntr++;
 	}
 	return 0;
} 

int setSubscribe(int subscribe, int channel){
	
	int cntr = 0;
	while(cntr < NUMBER_OF_CHANNELS){
		if(oChan[cntr].inputChannel == channel);
			oChan[cntr].subscribed=subscribe;
		cntr++;
 	}
 	return 0;
} 



static void measureTask(void *params) {
        
	int i;
	int voltage;
	while (1){

		//Get things from queues
		//Probably somthing with ipc_get here..  FIX
		


		//Maybe this is something that don't have to be done that often FIX
		for (i = 0; i < NUMBER_OF_CHANNELS;i++){
			setSubscribe(1, i);  //Should be set by value from ipc FIX
			setSampleRate(50, i); //Should be set by value from ipc FIX
		
			//Reconfigure Channels
			//channelConfig(oChan[i]);
			//Read data from ADCs
			voltage = readChannel(oChan[i]);
		}
	}
}

  void measureInit(unsigned portBASE_TYPE uxPriority) {
	///ALOT OF ADC-stuff goes here...
	 if(NUMBER_OF_CHANNELS != 2)
		assert(0); //this need to be generalized if other amount of chans is needed;
									   
	 oChan[0].ADC=ADC1;
	 oChan[0].ADC_Channel=ADC_Channel_0;
     oChan[0].subscribed=	0;
	 oChan[0].inputChannel = 1;
     oChan[0].rate=50;

	 oChan[1].ADC=ADC1;
	 oChan[1].ADC_Channel=ADC_Channel_1;
     oChan[1].subscribed=	0;
	 oChan[1].inputChannel = 2;
     oChan[1].rate=50;
	 xTaskCreate(measureTask, "measure", 100, NULL, uxPriority, NULL);
}
