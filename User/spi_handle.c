#include "spi_handle.h"
#include "spi.h"

#ifdef DEBUG
#define DBG_LOG(x) printf x
#else
#define DBG_LOG(x) 
#endif

#define SPI2_RX_BUFFER_size 1024
#define NULL 0

enum spi_rx_state{spi_idle=0,spi_rx_cmd,spi_rx_dat,spi_reply};//global

/*global variable*/
uint8_t g_spi2_state;
uint8_t g_spi2_rx_buffer[SPI2_RX_BUFFER_size]={"0"};

static void __handle_spi2_idle(void);
static void __handle_spi2_rx_cmd(void);
static void __handle_spi2_rx_dat(void);
static void __handle_spi2_reply(void);

void spi2_isr_config(void)
{
	
	hspi2.RxISR=spi2_rx_isr;
	hspi2.RxXferCount=0;
	hspi2.RxXferSize=0;
	hspi2.pRxBuffPtr= NULL;
}

void spi2_rx_isr(SPI_HandleTypeDef *hspi)
{
	hspi=hspi;//
	//to add rx data from spi2
	DBG_LOG(("enter spi2 int\r\n"));
	switch (g_spi2_state)
		{
		case spi_idle:
			__handle_spi2_idle();
			break;
		case spi_rx_cmd:
			__handle_spi2_rx_cmd();
			break;
		case spi_rx_dat:
			__handle_spi2_rx_dat();
			break;
		case spi_reply:
			__handle_spi2_reply();
			break;
		default:break;
	}
	;
}


void __handle_spi2_idle(void)
{

}

void __handle_spi2_rx_cmd(void)
{

}

void __handle_spi2_rx_dat(void)
{

}

void __handle_spi2_reply(void)
{

}




