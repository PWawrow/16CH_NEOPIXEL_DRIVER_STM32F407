/*
 * irDecoder.c
 *
 *  Created on: 10 kwi 2023
 *      Author: notif
 */
#include "irDecoder.h"

uint8_t irReceived = 0, irPulseNb = 0;
uint16_t irPulseError = 200;
uint16_t irPulses[32];
uint8_t irDevice = 0, irCommand = 0;
TIM_HandleTypeDef *irHtim;
UART_HandleTypeDef *irHuart;
#ifdef DEBUG

char ascii[20];

#endif


void irSetup(TIM_HandleTypeDef *htim, UART_HandleTypeDef *huart)
{
	irHtim = htim;
	irHuart = huart;
//	HAL_TIM_Base_Start_IT(htim);
	HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
}
void irHandle(void (*callback)(uint16_t, uint16_t))
{
	if(irReceived == 1){
			  for(int i = 0; i<8; i++)
			  {
				  if(irPulses[i]<(1000+irPulseError) && irPulses[i]>(1000-irPulseError))
					  irDevice &= ~(1 << (i));
				  else if(irPulses[i]<(2000+irPulseError) && irPulses[i]>(2000-irPulseError))
					  irDevice |= (1 << (i));
			  }
			  for(int i = 17; i<25; i++)
			  {
				  if(irPulses[i]<(1000+irPulseError) && irPulses[i]>(1000-irPulseError))
					  irCommand &= ~(1 << (i-17));
				  else if(irPulses[i]<(2000+irPulseError) && irPulses[i]>(2000-irPulseError))
					  irCommand |= (1 << (i-17));
			  }
#ifdef DEBUG
			  sprintf(ascii, "%d, c: %d\r\n", irDevice, irCommand);
			  HAL_UART_Transmit(&irHuart, ascii, sizeof(ascii), 100);
#endif
			  callback(irDevice, irCommand);
			  irReceived = 0;

	}
}


void irIRQ()
{
	uint16_t pulse_duration = 0;

	  if (irHtim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	    {

	      pulse_duration = (uint16_t)HAL_TIM_ReadCapturedValue(irHtim, TIM_CHANNEL_1);
	      __HAL_TIM_SET_COUNTER(irHtim, 0);
	      if(irPulseNb > 0 && irPulseNb <= 33){
	          	irPulses[irPulseNb-1] = pulse_duration;
	          	irPulseNb++;
	          	if(irPulseNb == 33)
	          	{
	          		irReceived = 1;
	          		irPulseNb = 0;
	          	}
	          }
	      if(pulse_duration<(8600+irPulseError) && pulse_duration>(8600-irPulseError))
	          	irPulseNb = 1;
	    }
}
