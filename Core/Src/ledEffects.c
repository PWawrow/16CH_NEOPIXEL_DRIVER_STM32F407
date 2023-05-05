/*
 * ledEffects.c
 *
 *  Created on: 23 kwi 2023
 *      Author: notif
 */

#include "ledEffects.h"

#define newColor(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) <<  8) | (b))
#define Red(c) ((uint8_t)((c >> 16) & 0xFF))
#define Green(c) ((uint8_t)((c >> 8) & 0xFF))
#define Blue(c) ((uint8_t)(c & 0xFF))


uint8_t frameBufferArray[WS2812_BUFFER_COUNT][WS2812B_NUMBER_OF_LEDS*3] = {0};
uint8_t ACTIVE_EFFECT_TYPE = 0;
uint8_t ACTIVE_EFFECT_NB = 0;


void effectsLoop()
{
	//CHECK ACTIVE STRIP
	switch(ACTIVE_EFFECT_TYPE)
	{
		//HANDLE ELE
		uint16_t strips;
		case 0:
			setAllBlack();
		case 1:
			strips = (ELE_INPUT_MACROS[ACTIVE_EFFECT_NB][1]<<8) | ELE_INPUT_MACROS[ACTIVE_EFFECT_NB][0];
			handleEffects(strips, &ELE_INPUT_MACROS[0][0], ACTIVE_EFFECT_NB, BYTES_PER_EFFECT);
		break;
		//HANDLE IR
		case 2:
			strips = (IR_CODES_MACROS[ACTIVE_EFFECT_NB][1]<<8) | IR_CODES_MACROS[ACTIVE_EFFECT_NB][0];
			handleEffects(strips, &IR_CODES_MACROS[0][0], ACTIVE_EFFECT_NB, BYTES_PER_EFFECT);
		break;
		//HANDLE TIME
		case 3:

		break;
	}
	if(ws2812b.transferComplete)
	{
		ws2812b.startTransfer = 1;
		ws2812b_handle();
	}
}
void handleEffects(uint16_t strips, uint8_t *arr, uint8_t start, uint8_t size)
{
	switch(*(arr+start*size+2))
	{
		case 0:
			setAllBlack();
			break;
		case 1:
			for(uint8_t i = 0; i < WS2812_BUFFER_COUNT; i++)
			{
				if(strips & (1<<i)) // Effect on strip
				{
					calcRainbow(i, *(arr+start*size+3));
				}else //strip off
				{
					setBlack(i);
				}
			}
			break;
	}
}

void setAllBlack()
{
	for(uint8_t j = 0; j < WS2812_BUFFER_COUNT; j++)
		for(uint16_t i = 0; i < (WS2812B_NUMBER_OF_LEDS*3); i++)
			frameBufferArray[j][i] = 0;
}
void setBlack(uint8_t id)
{
	for(uint16_t i = 0; i<(WS2812B_NUMBER_OF_LEDS*3); i++)
		frameBufferArray[id][i] = 0;
}
void calcRainbow(uint8_t id, uint8_t effectLength)
{
		uint32_t i;
		static uint8_t x[WS2812_BUFFER_COUNT] = {0};

		x[id] += 1;

		if(x[id] == 256*5)
			x[id] = 0;


		for( i = 0; i < WS2812B_NUMBER_OF_LEDS; i++)
		{
			uint32_t color = Wheel(((i * 256) / effectLength + x[id]) & 0xFF);

			frameBufferArray[id][i*3 + 0] = (color & 0xFF);
			frameBufferArray[id][i*3 + 1] = (color >> 8 & 0xFF);
			frameBufferArray[id][i*3 + 2] = (color >> 16 & 0xFF);
		}

}

uint32_t Wheel(uint8_t WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return newColor(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return newColor(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return newColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}
void visRainbow(uint8_t *frameBuffer, uint32_t frameBufferSize, uint32_t effectLength)
{
	uint32_t i;
	static uint8_t x = 0;

	x += 1;

	if(x == 256*5)
		x = 0;

	for( i = 0; i < frameBufferSize / 3; i++)
	{
		uint32_t color = Wheel(((i * 256) / effectLength + x) & 0xFF);

		frameBuffer[i*3 + 0] = (color & 0xFF);
		frameBuffer[i*3 + 1] = (color >> 8 & 0xFF);
		frameBuffer[i*3 + 2] = (color >> 16 & 0xFF);
	}
}

