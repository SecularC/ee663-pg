/*
 * uart.c
 *
 *  Created on: Feb 19, 2023
 *      Author: Chris
 */
#include "uart.h"


int uart2_index = 0;
int uart3_index = 0;
uint8_t uart3_line_flag = 0;
int n;
uint8_t rxByte2; //char and buffer for USART2
uint8_t rxBuffer2[UART_BUFFER_SIZE];
uint8_t txBuffer2[UART_BUFFER_SIZE];
uint8_t rxBytePnt;



uint8_t rxByte3; //char and buffer for USART3
uint8_t rxBuffer3[UART_BUFFER_SIZE];
uint8_t txBuffer3[UART_BUFFER_SIZE];

_Bool USART_getline(USART_TypeDef * USARTx)
{
	rxByte2 = USART_Read(USARTx);
	//if character isn't null
	if(rxByte2 != 255)
	{
		if(rxByte2 == '\r')
		{
			n = sprintf((char *)txBuffer2, "\r\n");
			USART_Write(USART2, txBuffer2, n);
			uart2_index = 0;
			return 1;
		}
		else if(rxByte2 == '\177'){
			if(uart2_index > 0)
			{
				rxBuffer2[uart2_index - 1] = '\0';
				uart2_index --;
			}
			//USART_Write(USARTx, '\r\n');
			return 0;
		}
		else if(uart2_index < UART_BUFFER_SIZE){
			rxBuffer2[uart2_index] = rxByte2;
			uart2_index++;
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
		n = sprintf((char *)txBuffer2, "%c", rxBytePnt);
 		USART_Write(USART2, txBuffer2, n);
		//HAL_UART_Transmit(USARTx, (uint8_t*)&rxBytePnt, sizeof(rxBytePnt), 1);
		return rxBytePnt;
	}
}

//interrupt handler for USART3
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//if character isn't null
	if(rxByte3 != 255)
	{
		if(rxByte3 == '\r')
		{
			n = sprintf((char *)txBuffer3, "\r\n");
			USART_Write(USART3, txBuffer3, n);
			uart3_index = 0;
			uart3_line_flag = 0;
		}
		else if(rxByte3 == '\177'){
			if(uart3_index > 0)
			{
				rxBuffer3[uart3_index - 1] = '\0';
				uart3_index --;
			}
			//USART_Write(USARTx, '\r\n');
		}
		else if(uart3_index < UART_BUFFER_SIZE){
			rxBuffer3[uart3_index] = rxByte3;
			uart3_index++;
		}
	}
	HAL_UART_Receive_IT(huart, (uint8_t *)rxByte3, 1);
}
