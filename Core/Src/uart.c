/*
 * uart.c
 *
 *  Created on: Feb 19, 2023
 *      Author: Chris
 */
#include "uart.h"

static int index;
int uart_index = 0;
int n;
uint8_t rxByte;
uint8_t rxBuffer[UART_BUFFER_SIZE];
uint8_t txBuffer[UART_BUFFER_SIZE];
uint8_t rxBytePnt;

_Bool USART_getline(USART_TypeDef * USARTx)
{
	rxByte = USART_Read(USARTx);
	if(rxByte != 255)
	{
		if(rxByte == '\r')
		{
			n = sprintf((char *)txBuffer, "\r\n");
			USART_Write(USART2, txBuffer, n);
			uart_index = 0;
			return 1;
		}
		else if(rxByte == '\177'){
			if(uart_index > 0)
			{
				rxBuffer[uart_index - 1] = '\0';
				uart_index --;
			}
			//USART_Write(USARTx, '\r\n');
			return 0;
		}
		else if(uart_index < UART_BUFFER_SIZE){
			rxBuffer[uart_index] = rxByte;
			uart_index++;
       	    return 0;
		}
	}else{
		return 0;
	}
	return 0;
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes) {
	int i;
	// A byte to be transmitted is written to the TDR (transmit data register), and the TXE (transmit empty) bit is cleared.
	// The TDR is copied to an output shift register for serialization when that register is empty, and the TXE bit is set.
	for (i = 0; i < nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE))
			;   							// wait until TXE (TX empty) bit is set
		USARTx->TDR = buffer[i] & 0xFF;		// writing USART_TDR automatically clears the TXE flag
	}
	while (!(USARTx->ISR & USART_ISR_TC))
		;  									// wait until TC bit is set
	USARTx->ISR &= ~USART_ISR_TC;
}

uint8_t USART_Read (USART_TypeDef * USARTx) {
	// SR_RXNE (Read data register not empty) bit is set by hardware
	if(!(USARTx->ISR & USART_ISR_RXNE))
	{
		return -1;
	} else{
		// Reading USART_DR automatically clears the RXNE flag
		//USART_Write(USARTx, (uint8_t)(USARTx->RDR & 0xFF));
		rxBytePnt = ((uint8_t)(USARTx->RDR & 0xFF));
		n = sprintf((char *)txBuffer, "%c", rxBytePnt);
 		USART_Write(USART2, txBuffer, n);
		//HAL_UART_Transmit(USARTx, (uint8_t*)&rxBytePnt, sizeof(rxBytePnt), 1);
		return rxBytePnt;
	}
}
