#include "uart.h"

static uint8_t g_usart_tmp;

int fputc(int ch,FILE *f)
{
	g_usart_tmp=(uint8_t)(ch);
	HAL_UART_Transmit_IT(&huart4,&g_usart_tmp,1);
	return ch;
}
