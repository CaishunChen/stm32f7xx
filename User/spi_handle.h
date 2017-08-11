#ifndef _spi_handle_H_
#define _spi_handle_H_

#include <stdint.h>
#include "stm32f7xx.h"

void spi2_rx_isr(SPI_HandleTypeDef *hspi);

#endif

