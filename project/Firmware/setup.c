

#include "setup.h"

#include <stdio.h>

#include <stm32f10x.h>
#include <system_stm32f10x_cl.h>

/*
 * Configure the clocks, GPIO and other peripherals.
 */
void prvSetupHardware( void )
{
  /* Set the clocks. */
  SystemInit();
  RCC->APB2ENR |= 0x00000261;

  /* Configure the GPIO for LEDs. */
  GPIOD->CRL &= 0xFFF00FFF;
  GPIOD->CRL |= 0x00033000;
  GPIOD->CRH &= 0xFF0FFFFF;
  GPIOD->CRH |= 0x00300000;
  GPIOE->CRH &= 0xF0FFFFFF;
  GPIOE->CRH |= 0x03000000;

  /* Configure UART2 for 115200 baud. */
  AFIO->MAPR |= 0x00000008;
  GPIOD->CRL &= 0xF00FFFFF;
  GPIOD->CRL |= 0x04B00000;

  RCC->APB1ENR |= 0x00020000;
  USART2->BRR = 0x0135;
  USART2->CR3 = 0x0000;
  USART2->CR2 = 0x0000;
  USART2->CR1 = 0x200C;

  /* Configure ADC.14 input. */
  GPIOC->CRL &= 0xFFF0FFFF;
  ADC1->SQR1  = 0x00000000;
  ADC1->SMPR1 = (5<<12);
  ADC1->SQR3  = (14<<0);
  ADC1->CR1   = 0x00000100;
  ADC1->CR2   = 0x000E0003;

  /* Reset calibration */
  ADC1->CR2  |= 0x00000008;
  while (ADC1->CR2 & 0x00000008);

  /* Start calibration */
  ADC1->CR2  |= 0x00000004;
  while (ADC1->CR2 & 0x00000004);
  ADC1->CR2  |= 0x00500000;

  /* Setup and enable the SysTick timer for 100ms. */
  SysTick->LOAD = (SystemFrequency / 100) - 1;
  SysTick->CTRL = 0x05;


  /* Set the Vector Table base address at 0x08000000 */
//  NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

//  NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* setup a serial port so that we can print to it */
//    xPort = xSerialPortInitMinimal(115200, 64);
}
/*-----------------------------------------------------------*/

/**
 * Currently does not seem to support LED1?
 */
void LED_out (u32 val) {
  u32 rv;

  rv = 0;
  if (val & 0x01) rv |= 0x00004000;
  GPIOE->BSRR = rv;
  GPIOE->BRR  = rv ^ 0x00004000;

  rv = 0;
  if (val & 0x02) rv |= 0x00002000;	// bit 13
  if (val & 0x04) rv |= 0x00000008;	// bit 3
  if (val & 0x08) rv |= 0x00000010;	// bit 4
  GPIOD->BSRR = rv;
  GPIOD->BRR  = rv ^ 0x0002098;
}

/* Function to execute if an assertion failed */
void assert_failed(u8* file, u32 line) {
  printf("ASSERTION FAILURE: %s:%d\n", file, line);
}
