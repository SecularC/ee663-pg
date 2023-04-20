/*
 * receive_task.c
 *
 *  Created on: Mar 28, 2023
 *      Author: Christian Secular
 */

#include "main.h"
uint8_t userMsg[200]; //extern to send message to uart.c
int msgSize;
COMMAND_c cmd;

extern uint8_t rxBuffer[UART_BUFFER_SIZE];
extern QueueHandle_t cmd_queue;
static void receive_task(void *params);
static int print_command(COMMAND_c * c_print);

//create receive task
int receive_task_init(void)
{
	COMMAND_c * c = &cmd;
	memset(c, 0, sizeof(COMMAND_c));
	BaseType_t err = xTaskCreate(receive_task, "ReceiveTask", 1024, &cmd, 1, NULL);
	assert(err == pdPASS);
	return 0;
}

static void receive_task(void *params){
	COMMAND_c *c = (COMMAND_c *)params;
	float minv; //stores the min/max voltages
	float maxv;
	while(1)
	{
		//check for a completed line
		if(USART_getline(USART2))
		{
			//read in command and calculate min and max DAC values required
			sscanf((char *)rxBuffer, "%s %u %c %f %f %f %u", c->name, &(c->channel), &(c->type), &(c->freq), &minv, &maxv, &(c->noise));
			c->dac_minv = (float) (4095/3.3) * minv;
			c->dac_maxv = (float) (4095/3.3) * maxv;

			memset(rxBuffer, '\0', sizeof(rxBuffer)); //reset buffer to all null terminators
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
		}
		vTaskDelay(1);
	}
}

/*
 * print out command
 * Return: 0 if command error, 1 if command is valid
 */
static int print_command(COMMAND_c * c_print)
{
	uint8_t * cmd_type[20];
	uint32_t req_freq;
	char cap_cmd[] = "cap\0";
	if (strcmp((char *) c_print->name, cap_cmd) == 0)
	{
		msgSize = sprintf((char *)userMsg, "Capturing\r\n");
		USART_Write(USART2, userMsg, msgSize);
		return 1;
	}
	//print out type of output signal
	switch(tolower(c_print->type))
	{
		case 's':
			strncpy((char *)cmd_type, "Sine", 20);
			break;
		case 'r':
			strncpy((char *)cmd_type, "Rectangle", 20);
			break;
		case 't':
			strncpy((char *)cmd_type, "Triangular", 20);
			break;
		case 'a':
			strncpy((char *)cmd_type, "Arbitrary", 20);
			break;
		case 'c':
			msgSize = sprintf((char *)userMsg, "Printing Copied Signal\r\n");
			USART_Write(USART2, userMsg, msgSize);
			return(1);
		default:
			//return error if wave type is not valid
			return 0;
			break;
	}

	if(c_print->freq != 0)
	{
		req_freq = c_print->freq * LUT_SIZE;
	}

	msgSize = sprintf((char *)userMsg, "Waveform: %s\r\n", (char *)cmd_type);
	USART_Write(USART2, userMsg, msgSize);

	msgSize = sprintf((char *)userMsg, "Frequency: %.2f\tUpdate Rate: %lu\r\n", c_print->freq, req_freq);
	USART_Write(USART2, userMsg, msgSize);

	msgSize = sprintf((char *)userMsg, "Number of Samples Per Waveform: %u\r\n", LUT_SIZE);
	USART_Write(USART2, userMsg, msgSize);

	//if signal was DC, only display minv. If not, display max DAC and min DAC voltage
	if(c_print->freq != 0)
	{
		if((c_print->dac_maxv >= 4096) || (c_print->dac_maxv < c_print->dac_minv) || (c_print->dac_minv < 0))
		{
			return 0;
		}

		msgSize = sprintf((char *)userMsg, "Max DAC Value: %u\tMin DAC Value:: %u\r\n", c_print->dac_maxv, c_print->dac_minv);
		USART_Write(USART2, userMsg, msgSize);
	} else{
		//return error is DC voltage is less than 0 or greaater than 3.3V
		if(c_print->dac_minv < 0 || c_print->dac_minv > 4096)
		{
			return 0;
		}
		req_freq = 0;
		msgSize = sprintf((char *)userMsg, "DC DAC Value: %u\r\n", c_print->dac_minv);
		USART_Write(USART2, userMsg, msgSize);
	}
	return 1;
}
