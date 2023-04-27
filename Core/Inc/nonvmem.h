/*
 * nonvmem.h
 *
 *  Created on: Mar 15, 2023
 *      Author: notif
 */

#ifndef INC_NONVMEM_H_
#define INC_NONVMEM_H_

#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_pwr.h"

/*DEFINE MAXIMUMS*/
#define BYTES_PER_EFFECT 6
#define MAX_IR_CODES 20
#define BYTES_PER_IR_CODE 2
#define MAX_ELE_INPUTS 10
#define MAX_TIME_MACROS 20
#define BYTES_PER_TIME_FLAG 4

extern uint8_t IR_CODES_FLAGS[MAX_IR_CODES][BYTES_PER_IR_CODE];
extern uint8_t IR_CODES_MACROS[MAX_IR_CODES][BYTES_PER_EFFECT];
extern uint8_t ELE_INPUT_MACROS[MAX_ELE_INPUTS][BYTES_PER_EFFECT];
extern uint8_t TIME_FLAGS[MAX_TIME_MACROS][BYTES_PER_TIME_FLAG];
extern uint8_t TIME_MACROS[MAX_TIME_MACROS][BYTES_PER_EFFECT];

/*FUNCTIONS*/
void purgeSaved();
void saveMem();
void readMem();

#endif /* INC_NONVMEM_H_ */
