/*
 * ring.c
 *
 *  Created on: Apr 20, 2023
 *      Author: Chris
 */

#include "main.h"
extern uint8_t txBuffer2[UART_BUFFER_SIZE]; //extern to send message to uart.c
extern uint8_t txBuffer3[UART_BUFFER_SIZE]; //extern to send message to uart.c
extern UART_HandleTypeDef huart3;
extern uint8_t rxBuffer2[UART_BUFFER_SIZE];
extern uint8_t rxBuffer3[UART_BUFFER_SIZE];
extern uint8_t uart3_line_flag;
extern uint8_t rxByte3; //char and buffer for USART3

COMMAND_c cmd;
extern QueueHandle_t cmd_queue;
RING_r ring;

const char *pc_r = "Ring\n";

static void ring_task(void* params);

int ring_task_init(void)
{
	int msgSize;
	//clear memory at ring
	RING_r  * r = &ring;
	memset(r, 0, sizeof(RING_r));

	//prompt for source ID
	msgSize = sprintf((char *)txBuffer2, "Enter source name (letters only):\r\n");
	USART_Write(USART2, txBuffer2, msgSize);

	//wait until a user name is entered
	while(!USART_getline(USART2));
	sscanf((char *)rxBuffer2, "%s", r->ringID);

	//create ring task
	BaseType_t err = xTaskCreate(ring_task, "Ring_Task", 1024, (void *) r, 2, NULL);
	assert(err == pdPASS);

	MFS_init();
	//enable interrupt
	HAL_UART_Receive_IT(&huart3, &rxByte3, 1);
	return 0;
}

static void ring_task(void* params){
	RING_r * r = (RING_r *)params; //pointer to received command
	char gen_cmd[] = "gen\0";
	char cap_cmd[] = "cap\0";
	char msg_cmd[] = "msg\0";
	char led_cmd[] = "led\0";
	char on[] = "on\0";
	char off[] = "off\0";
	int msgSize;
	unsigned char temp_ringID[10];
	//enable HAL UART interrupts
	HAL_UART_Receive_IT(&huart3, &rxByte3, 1);
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
				//received command
				msgSize = sprintf((char *)txBuffer2, "Received command: %s\r\n", r->command);
				USART_Write(USART2, txBuffer2, msgSize);
				memset(txBuffer2, '\0', sizeof(txBuffer2)); //reset buffer to all null terminators
				//if command matches gen or cap
				if((strcmp((char *) r->command, gen_cmd) == 0) || strcmp((char *) r->destID, cap_cmd) == 0)
				{
					parse_channel_cmd(r);
				}

				//if received command is msg, print out message
				if((strcmp((char *) r->command, msg_cmd) == 0))
				{
					msgSize = sprintf((char *)txBuffer2, "%s: %s\r\n", r->sourceID, r->param_1);
					USART_Write(USART2, txBuffer2, msgSize);
				}

				//if received command is LED, turn on LED
				if((strcmp((char *) r->command, led_cmd) == 0))
				{
					msgSize = sprintf((char *)txBuffer2, "%s: LED %s turned %s\r\n", r->sourceID, r->param_1, r->param_2);
					USART_Write(USART2, txBuffer2, msgSize);
					if((strcmp((char *) r->param_2, on) == 0))
					{
						int led = atoi(r->param_1);
						MFS_set_led(led, 1);
					}else if((strcmp((char *) r->param_2, off) == 0))
					{
						int led = atoi(r->param_1);
						MFS_set_led(led, 0);
					}
				}
			} else{
				//if not intended destination, send message out
				msgSize = sprintf((char *)txBuffer2, "Not intended target\r\n");
				USART_Write(USART2, txBuffer2, msgSize);
				//USART_Write(USART3, rxBuffer3, sizeof(rxBuffer3));
			}
			uart3_line_flag = 0;
			memset(rxBuffer3, '\0', sizeof(rxBuffer3)); //reset buffer to all null terminators
			memset(txBuffer2, '\0', sizeof(txBuffer2)); //reset buffer to all null terminators

			//store ring ID and clear ring command
			// copying ring ID to temporary
			strcpy((char*)temp_ringID, (char*) r->ringID);
			memset(r, '\0', sizeof(RING_r));
			strcpy((char*) r->ringID, (char*)temp_ringID);

			//enable interrupt
			HAL_UART_Receive_IT(&huart3, &rxByte3, 1);
		}

		vTaskDelay(1);
	}
	/*
	//read in command and calculate min and max DAC values required
				sscanf((char *)rxBuffer2, "%s %u %c %f %f %f %u", c->name, &(c->channel), &(c->type), &(c->freq), &minv, &maxv, &(c->noise));
				c->dac_minv = (float) (4095/3.3) * minv;
				c->dac_maxv = (float) (4095/3.3) * maxv;

				memset(rxBuffer2, '\0', sizeof(rxBuffer2)); //reset buffer to all null terminators
				BaseType_t err = xQueueSendToFront(cmd_queue, &c, 0);
				assert(err == pdPASS);
				//send command to the queue if command is okay
				if(print_command(c))
				{
					BaseType_t err = xQueueSendToFront(cmd_queue, &c, 0);
					assert(err == pdPASS);
				} else{
					msgSize = sprintf((char *)userMsg, "Command Error!\r\n");
					USART_Write(USART2, userMsg, msgSize);
				}
	*/
}

//Use ring command parameters to create command for channel tasks
void parse_channel_cmd(RING_r * r)
{
	double minv, maxv;
	COMMAND_c * c = &cmd;
	memset(c, 0, sizeof(COMMAND_c));
	strcpy((char*) c->name, (char*) r->command);
	c->channel = atoi((char *)r->param_1);
	c->type = (unsigned char) r->param_2[0];
	c->freq = atof((char *)r->param_3);

	minv = atof((char *)r->param_4);
	maxv = atof((char *)r->param_5);
	c->dac_minv = (float) (4095/3.3) * minv;
	c->dac_maxv = (float) (4095/3.3) * maxv;

	c->noise = atoi((char *)r->param_6);
	BaseType_t err = xQueueSendToFront(cmd_queue, &c, 0);
	assert(err == pdPASS);
}
