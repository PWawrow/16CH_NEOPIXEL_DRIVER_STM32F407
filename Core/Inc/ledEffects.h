#ifndef INC_LEDEFFECTS_H_
#define INC_LEDEFFECTS_H_
#include "ws2812b.h"
#include "nonvmem.h"
#include "main.h"


extern uint8_t frameBufferArray[WS2812_BUFFER_COUNT][WS2812B_NUMBER_OF_LEDS*3];
extern uint8_t ACTIVE_EFFECT_TYPE;
extern uint8_t ACTIVE_EFFECT_NB;


void effectsSetup();
void effectsLoop();

void handleEffects(uint16_t strips, uint8_t *arr, uint8_t start, uint8_t size);
void setBlack(uint8_t id);
void setAllBlack();
void setBlack(uint8_t id);
void calcRainbow(uint8_t id, uint8_t effectLength);
uint32_t Wheel(uint8_t WheelPos);

#endif
