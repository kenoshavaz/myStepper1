// Lab    : Stepper Motor Control
// Project: myStepper.c

// Name   : Kenosha Vaz
// Date   : 25 February 2018

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

static DAC_HandleTypeDef hdac;
static TIM_HandleTypeDef tim17;

/* Initialise the GPIO pins */

ParserReturnVal_t CmdStpInit(int action)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  DAC_ChannelConfTypeDef DacConfig;
  uint32_t rc;

  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  for(rc=1;rc<4;rc++){
    switch(rc){
    case 1:
      __HAL_RCC_GPIOC_CLK_ENABLE();
      GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      GPIO_InitStruct.Alternate = 0;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
      break;
    case 2:
      __HAL_RCC_GPIOA_CLK_ENABLE();
      GPIO_InitStruct.Pin = GPIO_PIN_4;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      GPIO_InitStruct.Alternate = 0;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      break;
    case 3:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      GPIO_InitStruct.Pin = GPIO_PIN_14;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      GPIO_InitStruct.Alternate = 0;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    }
  }

  __HAL_RCC_DAC1_CLK_ENABLE();

  /* Initialize DAC */

  hdac.Instance=DAC1;

  rc = HAL_DAC_Init(&hdac);

  if(rc != HAL_OK) {
    printf("Unable to initialize DAC, rc=%lu\n",rc);
    return CmdReturnOk;
  }

  DacConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  DacConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  rc = HAL_DAC_ConfigChannel(&hdac, &DacConfig,DAC_CHANNEL_1);

  if(rc != HAL_OK) {
    printf("Unable to configure DAC channel 1, rc=%lu\n",rc);
    return CmdReturnOk;
  }

  /* Enable the output */ 

  __HAL_DAC_ENABLE(&hdac,DAC_CHANNEL_1);

  /*Initialise the Timer Delay*/
  
  __HAL_RCC_TIM17_CLK_ENABLE();
  tim17.Instance = TIM17;
  tim17.Init.Prescaler     = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
  tim17.Init.CounterMode   = TIM_COUNTERMODE_UP;
  tim17.Init.Period        = 0xffff;
  tim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim17.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&tim17);

  HAL_NVIC_SetPriority(TIM17_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM17_IRQn);

  HAL_TIM_Base_Start(&tim17);
  TIM17->DIER |= 0x01;

 return CmdReturnOk;
}

ADD_CMD("stepperinit",CmdStpInit,"                Initialise Stepper Motor");

void TIM17_IRQHandler(void){
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
  TIM17->SR &= ~0x01;
}


ParserReturnVal_t CmdStpEnable(int action)
{
  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  uint16_t rc, pin;

  rc=fetch_uint16_arg(&pin);

  if(rc==1){
     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3|GPIO_PIN_8, 1);
     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, 0);
  }else if(rc==0){
     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3|GPIO_PIN_8, 0);
     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, 1);
  }else{
    printf("\nPlease Enter 1 or 0 to Toggle pin\n");
  }
  
 return CmdReturnOk;
}

ADD_CMD("stprenable",CmdStpEnable," <0|1>          Enable Stepper Motor");

/* Set Steps and Delay */

ParserReturnVal_t CmdStepper(int action)
{
  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  uint32_t rc,step,delay;

  rc=fetch_uint32_arg(&step);
  if(rc){
    printf("\nERROR: No value for steps!\n");
    return CmdReturnBadParameter1;
  }

  rc=fetch_uint32_arg(&delay);
  if(rc<0){
    printf("\nERROR: Time delay must be positive or null");
    return CmdReturnBadParameter1;
  }
  for(rc=0;rc<=step;rc++)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
    TIM17->CNT = 0;     /* Reset counter */
    while(TIM17->CNT < delay) {
      asm volatile ("nop\n");
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
  }
  return CmdReturnOk;
}
ADD_CMD("step",CmdStepper,"<step> <delay>  Control Stepper Motor");
