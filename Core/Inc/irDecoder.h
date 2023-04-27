/*
 * irDecoder.h
 *
 *  Created on: 10 kwi 2023
 *      Author: notif
 */
#include "main.h"
#ifndef INC_IRDECODER_H_
#define INC_IRDECODER_H_
//#define DEBUG




void irSetup(TIM_HandleTypeDef *htim, UART_HandleTypeDef *huart);
void irHandle(void (*callback)(uint16_t, uint16_t));
void irIRQ();


#endif /* INC_IRDECODER_H_ */
