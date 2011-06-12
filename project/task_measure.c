#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "assert.h"
#include "queue.h"
#include "stm3210c_eval_ioe.h"
#include "stm32f10x_adc.h"

#include "api_controller.h"
#include "api_measure.h"
#include "api_watchdog.h"
#include "ipc.h"
#include "task_measure.h"

int samplerate = 50;  //FIX!
OscilloscopeChannel oChan[NUMBER_OF_CHANNELS];

/* private functions */
static portBASE_TYPE handle_msg_subscribe(msg_id_t id, msg_data_t *cmd);
portBASE_TYPE send_data(
    oscilloscope_input_t ch,
    uint16_t data,
    int timestamp);

/* private variables */
static ipc_subscribe_msg_t data[NUMBER_OF_CHANNELS];
static ipc_subscribe_msg_t rate[NUMBER_OF_CHANNELS];
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_measure_subscribe, handle_msg_subscribe }
};

static const ipc_subscribe_table_t ipc_subscribe_table[] =
{
  IPC_SUBSCRIBE_TABLE_INIT(data+0, ipc_measure_variable_data_ch0, measure_subscribe),
  IPC_SUBSCRIBE_TABLE_INIT(data+1, ipc_measure_variable_data_ch1, measure_subscribe),
  IPC_SUBSCRIBE_TABLE_INIT(rate+0, ipc_measure_variable_rate_ch0, measure_subscribe),
  IPC_SUBSCRIBE_TABLE_INIT(rate+1, ipc_measure_variable_rate_ch1, measure_subscribe)
};


portBASE_TYPE setSampleRate(int rate, oscilloscope_input_t channel){
	int cntr = 0;
 	while(cntr < NUMBER_OF_CHANNELS){
		if(oChan[cntr].inputChannel == channel)
			oChan[cntr].rate=rate;
		cntr++;
 	}
 	return pdTRUE;;
} 

int getSampleRate(oscilloscope_input_t channel){
	int cntr = 0;
 	while(cntr < NUMBER_OF_CHANNELS){
		if(oChan[cntr].inputChannel == channel)
			 return oChan[cntr].rate;
		cntr++;
 	}
 	return -1;
} 

portBASE_TYPE setSubscribe(int subscribe, oscilloscope_input_t channel){
	
	int cntr = 0;
	while(cntr < NUMBER_OF_CHANNELS){
		if(oChan[cntr].inputChannel == channel)
			oChan[cntr].subscribed=subscribe;
		cntr++;
 	}
 	return pdTRUE;;
} 

int getSubscribe(oscilloscope_input_t channel){
	
	int cntr = 0;
	while(cntr < NUMBER_OF_CHANNELS){
		if(oChan[cntr].inputChannel == channel)
			return oChan[cntr].subscribed;
		cntr++;
 	}
 	return -1;
} 

double voltageConversion(uint16_t val){
	double maxAdcBits = 4095.0; // Should be > 0 
	double maxVolts = 3.3;      
	double voltsPerBit = (maxVolts / (maxAdcBits));
	return (double)val * voltsPerBit;
}	


portTickType herzToTicks(int samplerate){
		 double period_in_MS;
		 if (samplerate == 0)
		 	assert(0); //Division by zero
		 period_in_MS = 1000.0/(double)samplerate ;
		 return (portTickType)period_in_MS / portTICK_RATE_MS;
}

uint16_t readChannel(OscilloscopeChannel OChannel){
		ADC_RegularChannelConfig(OChannel.ADC,OChannel.ADC_Channel, 1, ADC_SampleTime_239Cycles5);
		ADC_ClearFlag(OChannel.ADC, ADC_FLAG_EOC);
		ADC_SoftwareStartConvCmd(OChannel.ADC, ENABLE);
		while (!ADC_GetFlagStatus(OChannel.ADC, ADC_FLAG_EOC));			
		return ADC_GetConversionValue(OChannel.ADC);
	
}	

void measureTask (void* params)
{
	int  packetCounter, i;
	uint16_t adc_value;
	packetCounter = 0;
  
  /* subscribe related initializing */
  assert(sizeof(data)/sizeof(data[0]) == sizeof(rate)/sizeof(rate[0]));
  for(i=0; i<sizeof(data)/sizeof(data[0]); i++)
  {
    ipc_subscribe_init(data + i, msg_id_subscribe_measure_data);
    ipc_subscribe_init(rate + i, msg_id_subscribe_measure_rate);
    data[i].msg.data.subscribe_measure_data.data = 0;
    data[i].msg.data.subscribe_measure_data.timestamp = 0;
    rate[i].msg.data.subscribe_measure_rate.rate = 0;
  }

  /* this needs manual update */
  assert(sizeof(data)/sizeof(data[0]) == 2);

  data[0].msg.data.subscribe_measure_data.ch = input_channel0;
  data[1].msg.data.subscribe_measure_data.ch = input_channel1;
  rate[0].msg.data.subscribe_measure_rate.ch = input_channel0;
  rate[1].msg.data.subscribe_measure_rate.ch = input_channel1;

  while(1)
  {
    if(pdTRUE == ipc_get(
          ipc_measure,
          (5000 / portTICK_RATE_MS) /*herzToTicks(samplerate)*/,
          msg_handle_table,
          sizeof(msg_handle_table)/sizeof(msg_handle_table[0])))
    {
      static uint16_t fakedata = 0;
      static int fakedata_counter = 0;
      send_data(input_channel0,fakedata,fakedata_counter);
      fakedata++;
      fakedata_counter+=2;

#if 0
		for (i = 0; i < NUMBER_OF_CHANNELS;i++){
			setSubscribe(1, oChan[i].inputChannel);  //Should be set by value from ipc FIX
			setSampleRate(samplerate, oChan[i].inputChannel); //Should be set by value from ipc FIX

			if(oChan[i].active){ 

				adc_value = readChannel(oChan[i]);
				//printf("%.2f ", voltageConversion(adc_value));
				send_data(oChan[i].inputChannel,adc_value,packetCounter);
				packetCounter++;
			}
		}
#endif



    }
    else
    {    
      ipc_watchdog_signal_error(0);
    }
  }
}

/* private functions */
static portBASE_TYPE handle_msg_subscribe(msg_id_t id, msg_data_t *msg)
{
  return ipc_handle_msg_subscribe(
      msg,
      ipc_subscribe_table,
      sizeof(ipc_subscribe_table)/sizeof(ipc_subscribe_table[0]));
}

portBASE_TYPE send_data(
    oscilloscope_input_t ch,
    uint16_t value,
    int timestamp)
{
  data[ch].msg.data.subscribe_measure_data.data = value;
  data[ch].msg.data.subscribe_measure_data.timestamp = timestamp;
  if(pdFALSE == ipc_subscribe_execute(data + ch))
  {
    ipc_watchdog_signal_error(0);
    return pdFALSE;
  }

  return pdTRUE;
}

#if 0
void measureTaskOld (void* params) {
	int  packetCounter, i;
	//oscilloscope_input_t i;
    uint16_t adc_value;
    portTickType xLastWakeTime;
    portTickType xFrequency = herzToTicks(samplerate);
	packetCounter = 0;

	for(;;){
		xLastWakeTime = xTaskGetTickCount();
		for (i = 0; i < NUMBER_OF_CHANNELS;i++){
			setSubscribe(1, oChan[i].inputChannel);  //Should be set by value from ipc FIX
			setSampleRate(samplerate, oChan[i].inputChannel); //Should be set by value from ipc FIX

			if(oChan[i].active){ 

				adc_value = readChannel(oChan[i]);
				//printf("%.2f ", voltageConversion(adc_value));
				ipc_controller_send_data(oChan[i].inputChannel,adc_value,packetCounter);
				packetCounter++;
			}
		}
	//printf("xLastWakeTime %d \n", xLastWakeTime);
	vTaskDelayUntil( &xLastWakeTime, xFrequency );	
	}
}
#endif
  void measureInit(void) {
  //void measureInit(unsigned portBASE_TYPE uxPriority) {
 
 /* ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	 // no dual ADC
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;           // read from the channel(s) configured below
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	 // one-shot conversion
  ADC_InitStructure.ADC_ExternalTrigConv =				 // we only trigger conversion internally
  ADC_ExternalTrigConv_None;							 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // store result in least significant bits

  ADC_InitStructure.ADC_NbrOfChannel = 1;                // only read from one channel at a time
  ADC_Init(ADC1, &ADC_InitStructure);*/

  /* Power up the ADC */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));


 if(NUMBER_OF_CHANNELS != 2){
		assert(0);} //this need to be generalized if other amount of chans is needed;
									   
	 oChan[0].ADC=ADC1;
	 oChan[0].ADC_Channel=ADC_Channel_7;
     oChan[0].subscribed=	0;
	 oChan[0].inputChannel = input_channel0;
     oChan[0].rate=50;
	 oChan[0].active=1;

	 oChan[1].ADC=ADC1;
	 oChan[1].ADC_Channel=ADC_Channel_8;
     oChan[1].subscribed=	0;
	 oChan[1].inputChannel = input_channel1;
     oChan[1].rate=50;
	 oChan[1].active=1;

}
