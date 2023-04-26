/*
 * uart.h
 *
 *  Created on: Feb 19, 2023
 *      Author: Chris
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l4xx_hal.h"

#define UART_BUFFER_SIZE 100
_Bool USART_getline(USART_TypeDef * USARTx);
uint8_t USART_Read (USART_TypeDef * USARTx);
void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes);

#endif /* INC_UART_H_ */
