/*
 * receive_task.c
 *
 *  Created on: Mar 28, 2023
 *      Author: Christian Secular
 */

#include "main.h"
uint8_t userMsg[200]; //extern to send message to uart.c
int msgSize;


extern uint8_t txBuffer2[UART_BUFFER_SIZE];
extern uint8_t rxBuffer2[UART_BUFFER_SIZE];
extern UART_HandleTypeDef huart3;
extern uint8_t rxByte3; //char and buffer for USART3
extern RING_r ring;
static void receive_task(void *params);
static int print_command(COMMAND_c * c_print);

//create receive task
int receive_task_init(void)
{
	COMMAND_c * c = &cmd;
	memset(c, 0, sizeof(COMMAND_c));
	BaseType_t err = xTaskCreate(receive_task, "Receive_Task", 1024, &cmd, 1, NULL);
	assert(err == pdPASS);
	return 0;
}

static void receive_task(void *params){
	memset(txBuffer2, '\0', sizeof(txBuffer2));
	while(1)
	{
		//check for a completed line
		if(USART_getline(USART2))
		{
			//append source ID and write to USART3
			msgSize = sprintf((char *)txBuffer2, "%s %s\r", ring.ringID, rxBuffer2);
			HAL_UART_Transmit_IT(&huart3, txBuffer2, msgSize);
			memset(rxBuffer2, '\0',  sizeof(rxBuffer2));
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
			msgSize = sprintf((char *)userMsg, "Number of Samples Per Waveform: %u\r\n", ADC_LUT_SIZE);
			USART_Write(USART2, userMsg, msgSize);
			msgSize = sprintf((char *)userMsg, "Update Rate: 10000Hz\r\n");
			return(1);
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
