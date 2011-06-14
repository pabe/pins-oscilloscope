#include <stdio.h>

#include "setup.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "assert.h"
#include "misc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "api_controller.h"
#include "api_measure.h"
#include "api_watchdog.h"
#include "ipc.h"
#include "task_measure.h"
  
/* private functions */
static portBASE_TYPE handle_msg_subscribe(msg_id_t id, msg_data_t *cmd);
static void timer_cfg(uint16_t prescaler, uint16_t period);
static int ch_generic2internal(oscilloscope_input_t ch);

/* private variables */
static xQueueHandle irq_transfer;
static ipc_subscribe_msg_t data[NUMBER_OF_CHANNELS];
static ipc_subscribe_msg_t rate;
static uint16_t read_channel(oscilloscope_input_t ch);
static const ipc_loop_t msg_handle_table[] =
{
  { msg_id_measure_subscribe, handle_msg_subscribe }
};

static const ipc_subscribe_table_t ipc_subscribe_table[] =
{
  IPC_SUBSCRIBE_TABLE_INIT(data+0, ipc_measure_variable_data_ch0, measure_subscribe),
  IPC_SUBSCRIBE_TABLE_INIT(data+1, ipc_measure_variable_data_ch1, measure_subscribe),
  IPC_SUBSCRIBE_TABLE_INIT(&rate,  ipc_measure_variable_rate,     measure_subscribe)
};

/* public functions */
portBASE_TYPE task_measure_init(void)
{
  int i;
  assert(!irq_transfer);

  irq_transfer = xQueueCreate(CONFIG_MEASURE_IRQ_QUEUE_LEN, sizeof(measure_data_t));
  if(!irq_transfer)
    return pdFALSE;

  /* subscribe related initializing */
  for(i=0; i<sizeof(data)/sizeof(data[0]); i++)
  {
    ipc_subscribe_init(data + i, msg_id_subscribe_measure_data);
  }
  ipc_subscribe_init(&rate, msg_id_subscribe_measure_rate);
  rate.msg.data.subscribe_measure_rate.rate = 0;

  /* this needs manual update if number if channels change */
  assert(sizeof(data)/sizeof(data[0]) == 2);

  data[0].msg.data.subscribe_measure_data.ch = input_channel0;
  data[1].msg.data.subscribe_measure_data.ch = input_channel1;


  /* lets init the adc! */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC calibration */
  while(ADC_GetCalibrationStatus(ADC1));
//  TimerInit(5,5000);
  timer_cfg(60,5000);

  return pdTRUE;
}

void task_measure_cmd(void* params)
{
  assert(ipc_measure);

  while(1)
  {
    if(pdTRUE == ipc_get(
          ipc_measure,
          portMAX_DELAY,
          msg_handle_table,
          sizeof(msg_handle_table)/sizeof(msg_handle_table[0])))
    {
      /* never timeout */
      ipc_watchdog_signal_error(0);
    }
    else
    {    
      ipc_watchdog_signal_error(0);
    }
  }
}

void task_measure(void* params)
{
  static measure_data_t buffer;
  /* 
   * buffer is stack so we do not waste stack,
   * so no more than one instance of task_measure() please!
   */
  
  assert(irq_transfer);
  while(1)
  {
    xQueueReceive(irq_transfer, &buffer, portMAX_DELAY);
    ipc_measure_put_data(&buffer);
    if(pdFALSE == ipc_subscribe_execute(
          data + ch_generic2internal(buffer.ch)))
    {
      ipc_watchdog_signal_error(0);
    }
  }
}

/* public (unofficial) functions */
void TIM2_IRQHandler(void)
{
  int i;
  portBASE_TYPE xTaskWokenByPost = pdFALSE;
  static measure_data_t buffer[] =
  {
    { { 0 }, input_channel0, 0 },
    { { 0 }, input_channel1, 0 }
  };
  static unsigned timestamp = 0;

  TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
  
  for(i=0; i<sizeof(buffer)/sizeof(buffer[0]); i++)
  {
    buffer[i].data[timestamp] = read_channel(buffer[i].ch);
  }
  timestamp++;

  if(timestamp == API_MEASURE_DATA_CHUNK_SIZE)
  {
    timestamp = 0;
    for(i=0; i<sizeof(buffer)/sizeof(buffer[0]); i++)
    {
      xQueueSendToFrontFromISR(
          irq_transfer,
          &buffer+i,
          &xTaskWokenByPost);
      buffer[i].timestamp += API_MEASURE_DATA_CHUNK_SIZE;
    }

    if(xTaskWokenByPost)
      taskYIELD();
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

static void timer_cfg(uint16_t prescaler, uint16_t period)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
  
  /* Initialise data. */
  TIM_DeInit( TIM2 );
  TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );

  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );
  TIM_ARRPreloadConfig( TIM2, ENABLE );

  /* Configuration of the interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure );       
  
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  TIM_Cmd(TIM2, ENABLE);
}

static int ch_generic2internal(oscilloscope_input_t ch)
{
  switch(ch)
  {
    case input_channel0:
      return 0;

    case input_channel1:
      return 1;

    default:
      ipc_watchdog_signal_error(0);
      return 0;
  }
}

/*
 * NOTE: No protection, should only be called from ISR
 */
static uint16_t read_channel(oscilloscope_input_t ch)
{
  uint8_t real_ch;
  switch(ch)
  {
    case input_channel0:
      real_ch = ADC_Channel_7;
      break;

    case input_channel1:
      real_ch = ADC_Channel_8;
      break;

    default:
      ipc_watchdog_signal_error(0);
      return UINT16_MAX;
  }
  
  ADC_RegularChannelConfig(ADC1, real_ch, 1, ADC_SampleTime_239Cycles5);
  ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

  /* spin until we have data */
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));			

  return ADC_GetConversionValue(ADC1);
}	

