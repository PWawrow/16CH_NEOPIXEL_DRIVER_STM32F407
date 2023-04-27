/*
 * nonvmem.c
 *
 *  Created on: Mar 15, 2023
 *      Author: notif
 */
#include "nonvmem.h"
/*DEFINE ADDRESES*/
uint16_t IR_ADDR = 20;
uint16_t ELE_INPUT_ADDR = 500;
uint16_t TIME_ADDR = 1000;

uint8_t IR_CODES_FLAGS[MAX_IR_CODES][BYTES_PER_IR_CODE] 	= {0};
uint8_t IR_CODES_MACROS[MAX_IR_CODES][BYTES_PER_EFFECT]  	= {0};
uint8_t ELE_INPUT_MACROS[MAX_ELE_INPUTS][BYTES_PER_EFFECT] 	= {0};
uint8_t TIME_FLAGS[MAX_TIME_MACROS][BYTES_PER_TIME_FLAG]	= {0};
uint8_t TIME_MACROS[MAX_TIME_MACROS][BYTES_PER_EFFECT] 		= {0};

void purgeSaved(int range){
	for(int i = 0; i < range; i++)
	{
		*(__IO uint8_t *) (BKPSRAM_BASE +i) = 0;
	}
}

void save2DArr(uint8_t* array, int rows, int cols, int* startId){

	for(int i = 0; i<rows; i++)
		{
			for(int j = 0; j<cols; j++){
				*(__IO uint8_t *) (BKPSRAM_BASE+*startId) = *(array + i * cols + j);
				*startId+=1;
			}
		}
}

void read2DArr(uint8_t* array, int rows, int cols, int* startId){

	for(int i = 0; i<rows; i++)
		{
			for(int j = 0; j<cols; j++){
				*(array + i * cols + j) = (*(__IO uint8_t *) (BKPSRAM_BASE + *startId));
				*startId+=1;
			}
		}
}

void saveMem(){
	//SET 6 FIRTS BYTES TO STARTING ADDRESSES OF INDIVIDUAL DATA
	uint8_t hbyte = 0,lbyte = 0;

	hbyte = (IR_ADDR>>8) & 0xFF;
	lbyte = IR_ADDR & 0xFF;
	*(__IO uint8_t *) (BKPSRAM_BASE) = hbyte;
	*(__IO uint8_t *) (BKPSRAM_BASE+1) = lbyte;
	hbyte = 0,lbyte = 0;

	hbyte = (ELE_INPUT_ADDR>>8) & 0xFF;
	lbyte = ELE_INPUT_ADDR & 0xFF;
	*(__IO uint8_t *) (BKPSRAM_BASE+2) = hbyte;
	*(__IO uint8_t *) (BKPSRAM_BASE+3) = lbyte;
	hbyte = 0,lbyte = 0;

	hbyte = (TIME_ADDR>>8) & 0xFF;
	lbyte = TIME_ADDR & 0xFF;
	*(__IO uint8_t *) (BKPSRAM_BASE+4) = hbyte;
	*(__IO uint8_t *) (BKPSRAM_BASE+5) = lbyte;
	hbyte = 0,lbyte = 0;

	//SAVE IR ARRAYs
	int currAddr = IR_ADDR;
	save2DArr(&IR_CODES_FLAGS[0][0], MAX_IR_CODES, BYTES_PER_IR_CODE, &currAddr);
	save2DArr(&IR_CODES_MACROS[0][0], MAX_IR_CODES, BYTES_PER_EFFECT, &currAddr);

	currAddr = ELE_INPUT_ADDR;
	save2DArr(&ELE_INPUT_MACROS[0][0], MAX_ELE_INPUTS, BYTES_PER_EFFECT, &currAddr);

	currAddr = TIME_ADDR;
	save2DArr(&TIME_FLAGS[0][0], MAX_IR_CODES, BYTES_PER_TIME_FLAG, &currAddr);
	save2DArr(&TIME_MACROS[0][0], MAX_IR_CODES, BYTES_PER_EFFECT, &currAddr);
}


void readMem(){
	//READ ADDRESES
	uint8_t hbyte = 0,lbyte = 0;

	hbyte = (*(__IO uint8_t *) (BKPSRAM_BASE));
	lbyte = (*(__IO uint8_t *) (BKPSRAM_BASE+1));
	IR_ADDR = ((uint16_t) hbyte << 8) | lbyte;
	hbyte = 0,lbyte = 0;

	hbyte = (*(__IO uint8_t *) (BKPSRAM_BASE+2));
	lbyte = (*(__IO uint8_t *) (BKPSRAM_BASE+3));
	ELE_INPUT_ADDR = ((uint16_t) hbyte << 8) | lbyte;
	hbyte = 0,lbyte = 0;

	hbyte = (*(__IO uint8_t *) (BKPSRAM_BASE+4));
	lbyte = (*(__IO uint8_t *) (BKPSRAM_BASE+5));
	TIME_ADDR = ((uint16_t) hbyte << 8) | lbyte;
	hbyte = 0,lbyte = 0;
	//READ ARRAYS
	int currAddr = IR_ADDR;
	read2DArr(&IR_CODES_FLAGS[0][0], MAX_IR_CODES, BYTES_PER_IR_CODE, &currAddr);
	read2DArr(&IR_CODES_MACROS[0][0], MAX_IR_CODES, BYTES_PER_EFFECT, &currAddr);

	currAddr = ELE_INPUT_ADDR;
	read2DArr(&ELE_INPUT_MACROS[0][0], MAX_ELE_INPUTS, BYTES_PER_EFFECT, &currAddr);

	currAddr = TIME_ADDR;
	read2DArr(&TIME_FLAGS[0][0], MAX_IR_CODES, BYTES_PER_TIME_FLAG, &currAddr);
	read2DArr(&TIME_MACROS[0][0], MAX_IR_CODES, BYTES_PER_EFFECT, &currAddr);

}


