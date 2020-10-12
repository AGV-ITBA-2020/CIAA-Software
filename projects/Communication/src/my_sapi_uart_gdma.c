/*
 * my_sapi_uart_gdma.c
 *
 *  Created on: Oct 7, 2020
 *      Author: Javier
 */


#include "../inc/my_sapi_uart_gdma.h"
#include "my_sapi_uart.h"
#include "chip.h"

static uint8_t dmaChannelNumRx;

void uartDMAInit( uartMap_t uart, uint32_t baudRate, bool_t loopback )
{
	if(uart != UART_232) //Se diseño solo para rs232, sino hay que cambiar los defines
		assert(0);
	uartInit(uart, baudRate,loopback ); //Inicializa la uart deseada, se puede poner en modo loopback
	uartSetDMAMode(uart);
	Chip_GPDMA_Init(LPC_GPDMA);

	dmaChannelNumRx = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, GPDMA_CONN_UART0_Rx );
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, 5 ); // FreeRTOS Requiere prioridad >= 5 (numero mas alto, mas baja prioridad)
	NVIC_EnableIRQ(DMA_IRQn);

}
