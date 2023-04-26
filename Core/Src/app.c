/*
 * app.c
 *
 *  Created on: Feb 28, 2023
 *      Author: Chris
 */

#include "main.h"

extern uint8_t userMsg[20];
extern int msgSize;
QueueHandle_t cmd_queue;


void app_init(void)
{
	//display beginning message
	msgSize = sprintf((char *)userMsg, "\r\nStart program.\r\n");
	USART_Write(USART2, userMsg, msgSize);
	receive_task_init();
	channel_task_init();
	ring_task_init();
	cmd_queue = xQueueCreate(99, sizeof(COMMAND_c));
}
