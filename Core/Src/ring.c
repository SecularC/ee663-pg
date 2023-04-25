/*
 * ring.c
 *
 *  Created on: Apr 20, 2023
 *      Author: Chris
 */

#include "main.h"
extern uint8_t userMsg[200]; //extern to send message to uart.c
extern UART_HandleTypeDef huart3;
extern uint8_t rxBuffer2[UART_BUFFER_SIZE];
extern uint8_t rxBuffer3[UART_BUFFER_SIZE];
extern uint8_t uart3_line_flag;
RING_r ring;

static void ring_task(void* params);

int ring_task_init(void)
{
	int msgSize;
	//clear memory at ring
	RING_r  * r = &ring;
	memset(r, 0, sizeof(RING_r));

	//prompt for source ID
	msgSize = sprintf((char *)userMsg, "Enter source name (letters only):\r\n");
	USART_Write(USART2, userMsg, msgSize);

	//wait until a user name is entered
	while(!USART_getline(USART2));
	sscanf((char *)rxBuffer2, "%s", r->ringID);

	//create ring task
	BaseType_t err = xTaskCreate(ring_task, "Ring_Task", 1024, (void *) r, 2, NULL);
	assert(err == pdPASS);
	return 0;
}

static void ring_task(void* params){
	RING_r * r = (RING_r *)params; //pointer to received command
	char gen_cmd[] = "gen\0";
	char cap_cmd[] = "cap\0";
	char msg_cmd[] = "msg\0";
	char led_cmd[] = "led\0";
	while(1)
	{
		//if flag is raised (UART3 has received a line
		if(uart3_line_flag)
		{
			//read in command
			sscanf((char *)rxBuffer3, "%s %s %s %s %s %s %s %s %s", r->sourceID, r->destID, r->command, r->param_1, r->param_2, r->param_3, r->param_4, r->param_5, r->param_6);

			//check if destination ID matches ring ID
			if(strcmp((char *) r->destID, (char *) r->ringID) == 0)
			{
				//if command matches gen or cap
				if((strcmp((char *) r->command, gen_cmd) == 0) || strcmp((char *) r->destID, cap_cmd) == 0)
				{

				}
			}
		}
	}
}
