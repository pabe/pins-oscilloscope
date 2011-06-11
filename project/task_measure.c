#include <stdio.h>
#define USE_TIMER 1
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "assert.h"
#include "queue.h"
#include "stm3210c_eval_ioe.h"
#include "stm32f10x_adc.h"

#if USE_TIMER
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#endif

#include "api_controller.h"
#include "api_measure.h"
#include "api_watchdog.h"
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
static int testIntForISR;
static subscribe_msg_t data[NUMBER_OF_CHANNELS];
static subscribe_msg_t rate[NUMBER_OF_CHANNELS];
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_measure_subscribe, handle_msg_subscribe }
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
	int testIntForISROld;
	packetCounter = 0;

	testIntForISR = 0;
	
  
  /* subscribe related initializing */
  assert(sizeof(data)/sizeof(data[0]) == sizeof(rate)/sizeof(rate[0]));
  for(i=0; i<sizeof(data)/sizeof(data[0]); i++)
  {
    subscribe_init(data + i, msg_id_subscribe_measure_data);
    subscribe_init(rate + i, msg_id_subscribe_measure_rate);
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
	//if (testIntForISROld != testIntForISR)
	printf("%d", testIntForISR);
	testIntForISROld = testIntForISR;

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
  switch(msg->measure_subscribe.variable)
  {
    case ipc_measure_variable_data_ch0:
      if(pdFALSE == subscribe_add(data+0, msg->measure_subscribe.subscriber))
      {
        ipc_watchdog_signal_error(0);
        return pdFALSE;
      }
      break;

    case ipc_measure_variable_data_ch1:
      if(pdFALSE == subscribe_add(data+1, msg->measure_subscribe.subscriber))
      {
        ipc_watchdog_signal_error(0);
        return pdFALSE;
      }
      break;

    case ipc_measure_variable_rate_ch0:
      if(pdFALSE == subscribe_add(rate+0, msg->measure_subscribe.subscriber))
      {
        ipc_watchdog_signal_error(0);
        return pdFALSE;
      }
      break;

    case ipc_measure_variable_rate_ch1:
      if(pdFALSE == subscribe_add(rate+1, msg->measure_subscribe.subscriber))
      {
        ipc_watchdog_signal_error(0);
        return pdFALSE;
      }
      break;

    default:
      return pdFALSE;
  }
  return pdTRUE;
}

portBASE_TYPE send_data(
    oscilloscope_input_t ch,
    uint16_t value,
    int timestamp)
{
  data[ch].msg.data.subscribe_measure_data.data = value;
  data[ch].msg.data.subscribe_measure_data.timestamp = timestamp;
  if(pdFALSE == subscribe_execute(data + ch))
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

void ADCInit(OscilloscopeChannel oChan){
 /* ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	 // no dual ADC
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;           // read from the channel(s) configured below
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	 // one-shot conversion
  ADC_InitStructure.ADC_ExternalTrigConv =				 // we only trigger conversion internally
  ADC_ExternalTrigConv_None;							 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // store result in least significant bits

  ADC_InitStructure.ADC_NbrOfChannel = 1;                // only read from one channel at a time
  ADC_Init(ADC1, &ADC_InitStructure);*/

  /* Power up the ADC */
  ADC_Cmd(oChan.ADC, ENABLE);

  /* Enable ADC reset calibaration register */   
  ADC_ResetCalibration(oChan.ADC);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(oChan.ADC));

  /* Start ADC calibaration */
  ADC_StartCalibration(oChan.ADC);
  /* Check the end of ADC calibration */
  while(ADC_GetCalibrationStatus(oChan.ADC));


}
#if USE_TIMER


void vTimer2IntHandler(void) {
  portBASE_TYPE higherPrio;



  // Clear pending-bit of interrupt
  TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

  // FreeRTOS macro to signal the end of ISR
  portEND_SWITCHING_ISR(higherPrio);
}



void TimerInit(void){
TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
TIM_OCInitTypeDef         TIM_OCInitStructure;

  RCC_APB1PeriphClockCmd( RCC_APB2Periph_TIM1, ENABLE );
  /* TIM1 configuration ------------------------------------------------------*/
  /* Time Base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = (unsigned portSHORT)0x0FFF;
  TIM_TimeBaseStructure.TIM_Prescaler = 719;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  /* Channel1 Configuration in PWM mode */
  /*TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);	 */

  /* Enable TIM1 */  
  TIM_Cmd(TIM1, ENABLE);
  /* Enable TIM1 outputs */
  //TIM_CtrlPWMOutputs(TIM1, ENABLE);

}
#endif


  void measureInit(void) {
//

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

  
 	 ADCInit(oChan[0]);
#if USE_TIMER
	 TimerInit();
#endif




}
