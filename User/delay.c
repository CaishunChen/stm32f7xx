#include "delay.h"
#include "stm32f7xx.h"

extern HAL_GetTick();

void delay(uint32_t tick )
{
	uint32_t start_tick;
	uint32_t end_tick;
	start_tick=HAL_GetTick();
	end_tick=start_tick+tick;
	while(HAL_GetTick()<end_tick)
	{
		;
	}
}