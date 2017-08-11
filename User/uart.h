#ifndef _uart_H_
#define _uart_H_

#include <stdio.h>
#include <stdint.h>
#include "stm32f7xx.h"

extern UART_HandleTypeDef huart4;

int fputc(int ch, FILE *f);

#endif /*_uart4_H_*/
