/*
 * dac_channel_task.c
 *
 *  Created on: Mar 28, 2023
 *      Author: Chris
 */

#include "main.h"
#include <math.h>
extern QueueHandle_t cmd_queue;

extern DAC_HandleTypeDef hdac1;
extern DMA_HandleTypeDef hdma_dac_ch1;
extern DMA_HandleTypeDef hdma_dac_ch2;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

const char *pc_ch = "Channel Receive\n";

uint16_t dac_LUT_1[LUT_SIZE];
uint16_t dac_LUT_2[LUT_SIZE];
uint16_t adc_LUT_1[ADC_LUT_SIZE];

//LUT for EKG
uint16_t ekg[] = {
	1690, 1680, 1680, 1669, 1648, 1648, 1648, 1680, 1680, 1690, 1680, 1680, 1680, 1680, 1680, 1658,
	1690, 1690, 1712, 1690, 1690, 1680, 1669, 1669, 1680, 1690, 1690, 1680, 1669, 1669, 1669, 1680,
	1680, 1680, 1669, 1669, 1680, 1690, 1690, 1680, 1690, 1690, 1680, 1690, 1690, 1712, 1680, 1680,
	1658, 1648, 1648, 1648, 1669, 1669, 1680, 1690, 1690, 1701, 1680, 1680, 1669, 1680, 1680, 1680,
	1701, 1701, 1701, 1690, 1690, 1701, 1712, 1712, 1722, 1712, 1712, 1690, 1669, 1669, 1680, 1690,
	1690, 1690, 1733, 1733, 1765, 1776, 1861, 1882, 1936, 1936, 1968, 1989, 1989, 2032, 2053, 2053,
	2085, 2149, 2069, 2080, 2058, 2058, 1930, 1930, 1845, 1824, 1792, 1872, 1840, 1754, 1754, 1722,
	1680, 1680, 1680, 1637, 1637, 1637, 1637, 1637, 1626, 1648, 1648, 1637, 1605, 1605, 1616, 1680,
	1680, 1765, 1776, 1861, 2042, 2106, 2021, 1776, 2480, 2400, 2176, 1632, 1637, 1360, 933, 928,
	1962, 1962, 2042, 2149, 3141, 3141, 2320, 1200, 1200, 1392, 1669, 1669, 1658, 1701, 1701, 1701,
	1701, 1701, 1722, 1690, 1690, 1690, 1680, 1680, 1690, 1690, 1690, 1669, 1669, 1669, 1701, 1733,
	1733, 1754, 1744, 1744, 1733, 1733, 1733, 1722, 1765, 1765, 1765, 1733, 1733, 1733, 1722, 1722,
	1701, 1690, 1690, 1701, 1690, 1690, 1701, 1701, 1701, 1701, 1722, 1722, 1712, 1722, 1722, 1733,
	1733, 1733, 1733, 1712, 1712, 1712, 1733, 1733, 1733, 1733, 1733, 1733, 1744, 1744, 1744, 1744,
	1744, 1744, 1733, 1733, 1722, 1722, 1722, 1722, 1722, 1722, 1733, 1722, 1722, 1722, 1722, 1722,
	1701, 1669, 1669, 1680, 1690, 1690, 1690, 1701, 1701, 1712, 1712, 1712, 1690, 1669, 1669, 1680,
};

static void channel_task(void *params);

//creates task to take in gen commands
int channel_task_init(){
	//create task
	BaseType_t err = xTaskCreate(channel_task, "Channel_Task", 1024, (void *) pc_ch, 3, NULL);
	assert(err == pdPASS);
	return 0;
}
static void channel_task(void* params){
	COMMAND_c * rec_cmd; //pointer to received command
	int channel_sel; //variable to hold selected DAC channel
	uint32_t req_freq; //variable for required frequency of waveform
	uint32_t req_freq_div; //variable for required ARR value
	int noise_bits; //number of noise bits to unmask
	uint16_t prescaler = 0; //prescaler value that the timers will use
	char gen_cmd[] = "gen\0";
	char cap_cmd[] = "cap\0";
	while(1)
	{
		if(uxQueueMessagesWaiting(cmd_queue))
		{
			//receive pointer to command struct
			BaseType_t rec_error = xQueueReceive(cmd_queue, &rec_cmd, 0);
			assert(rec_error == pdPASS);
			if(strcasecmp((char *) rec_cmd->name, gen_cmd) == 0)
			{
				channel_sel = rec_cmd->channel;
				noise_bits = rec_cmd->noise;
				TIM2->PSC = prescaler;
				TIM5->PSC = prescaler;
				if(rec_cmd->freq == 0)
				{
					//generate DC signal
					generate_DC(channel_sel, rec_cmd->dac_minv, rec_cmd->noise);
				} else{
					//calculate register values to achieve correct frequency
					req_freq = rec_cmd->freq * LUT_SIZE;
					req_freq_div = ((80000000/(prescaler + 1))/req_freq);


					//switch statement to generate the selected waveform
					switch(rec_cmd->type)
					{
						case 's':
							generate_sine(channel_sel, rec_cmd->dac_minv, rec_cmd->dac_maxv, rec_cmd->noise);
							break;
						case 'r':
							generate_rectangle(channel_sel, rec_cmd->dac_minv, rec_cmd->dac_maxv, rec_cmd->noise);
							break;
						case 't':
							generate_triangular(channel_sel, rec_cmd->dac_minv, rec_cmd->dac_maxv, rec_cmd->noise);
							break;
						case 'a':
							generate_arbitrary(channel_sel, rec_cmd->noise);
							break;
					}
				}
				//start the DMA for the data
				switch(channel_sel)
				{
				case 1:
					//turn off DAC
					HAL_DAC_Stop_DMA(&hdac1, DAC1_CHANNEL_1);
					//turn off noise
					DAC->CR &= ~(DAC_CR_MAMP1_0 + DAC_CR_MAMP1_1 + DAC_CR_MAMP1_2 + DAC_CR_MAMP1_3);
					DAC->CR &= ~DAC_CR_WAVE1_0;
					//if there is noise bits, enable noise generator and unmask that number of bits
					if(noise_bits > 0)
					{
						DAC->CR |= DAC_CR_WAVE1_0;
						DAC->CR |= (noise_bits << DAC_CR_MAMP1_Pos);
					}
					//start timer 2 and DMA for channel 1
					if(rec_cmd->type == 'c')
					{
						req_freq = 10000; //put entire buffer into DAC in 2 seconds (20k/2 = 10kHz)
						req_freq_div = ((80000000/(prescaler + 1))/req_freq);
						TIM2->ARR = req_freq_div;
						TIM2->EGR |= TIM_EGR_UG;
						HAL_TIM_Base_Start(&htim2);
						HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_1, (uint32_t *)adc_LUT_1, ADC_LUT_SIZE, DAC_ALIGN_12B_R);
					} else{
						TIM2->ARR = req_freq_div;
						TIM2->EGR |= TIM_EGR_UG;
						HAL_TIM_Base_Start(&htim2);
						HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_1, (uint32_t *)dac_LUT_1, LUT_SIZE , DAC_ALIGN_12B_R);
					}
					break;
				case 2:
					//turn off DAC
					HAL_DAC_Stop_DMA(&hdac1, DAC1_CHANNEL_2);
					//turn off noise
					DAC->CR &= ~(DAC_CR_MAMP2_0 + DAC_CR_MAMP2_1 + DAC_CR_MAMP2_2 + DAC_CR_MAMP2_3);
					DAC->CR &= ~DAC_CR_WAVE2_0;
					//if there is noise bits, enable noise generator and unmask that number of bits
					if(noise_bits > 0)
					{
						DAC->CR |= DAC_CR_WAVE2_0;
						DAC->CR |= (noise_bits << DAC_CR_MAMP2_Pos);
					}

					//start timer 5 and DMA for channel 1
					if(rec_cmd->type == 'c')
					{
						//output to DAC at 10kHz
						HAL_ADC_Stop_DMA(&hadc1);
						req_freq = 10000;
						req_freq_div = ((80000000/(prescaler + 1))/req_freq);
						TIM5->ARR = req_freq_div; //calculated for a frequency of 0.5 Hz (2s)
						TIM5->EGR |= TIM_EGR_UG;
						HAL_TIM_Base_Start(&htim5);
						HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_2, (uint32_t *)adc_LUT_1, ADC_LUT_SIZE, DAC_ALIGN_12B_R);
					} else{
						TIM5->ARR = req_freq_div;
						TIM5->EGR |= TIM_EGR_UG;
						HAL_TIM_Base_Start(&htim5);
						HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_2, (uint32_t *)dac_LUT_2, LUT_SIZE, DAC_ALIGN_12B_R);
					}
					break;
				}
			} else if (strcasecmp((char *) rec_cmd->name, cap_cmd) == 0)
			{
				TIM4->EGR |= TIM_EGR_UG;
				HAL_TIM_Base_Start(&htim4);
				HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adc_LUT_1, ADC_LUT_SIZE);
			}
		}
		vTaskDelay(1);
	}
}


void generate_DC(int channel, uint16_t dac_minv, int noise)
{
	//generate lookup table for DC signal

	switch(channel)
	{
	case 1:
		for(int i = 0; i < LUT_SIZE; i++)
		{
			//add noise
			dac_LUT_1[i] = dac_minv;
		}
		break;
	case 2:
		for(int i = 0; i < LUT_SIZE; i++)
		{
			//add noise
			dac_LUT_2[i] = dac_minv;
		}
		break;
	}
}

/*
 * Generate an rectangular signal with a high of maxv and low of minv
 *
 * Param: int channel - # of DAC channel
 * 		  uint16_t dac_minv - DAC value of minimum voltage
 * 		  uint16_t dac_maxv - DAC value of max voltage
 * 		  int noise - # of noise bits
 */
void generate_rectangle(int channel, uint16_t dac_minv, uint16_t dac_maxv, int noise)
{
	//generate lookup table for rectangle signal where half is minv and 2nd half is maxv
	switch(channel)
	{
	case 1:
		for(int i = 0; i < LUT_SIZE/2; i++)
		{
			//set low part of rectangle
			dac_LUT_1[i] = dac_minv;
		}
		for(int i = LUT_SIZE/2; i < LUT_SIZE; i++)
		{
			//set high part of rectangle
			dac_LUT_1[i] = dac_maxv;
		}
		break;
	case 2:
		for(int i = 0; i < LUT_SIZE/2; i++)
		{
			//set low part of rectangle
			dac_LUT_2[i] = dac_minv;
		}
		for(int i = LUT_SIZE/2; i < LUT_SIZE; i++)
		{
			//set high part of rectangle
			dac_LUT_2[i] = dac_maxv;
		}
		break;
	}
}

/*
 * Generate an sine wave signal
 *
 * Param: int channel - # of DAC channel
 * 		  uint16_t dac_minv - DAC value of minimum voltage
 * 		  uint16_t dac_maxv - DAC value of max voltage
 * 		  int noise - # of noise bits
 */
void generate_sine(int channel, uint16_t dac_minv, uint16_t dac_maxv, int noise)
{
	uint16_t midv = (dac_minv + dac_maxv) / 2;
	float angle = (float)(2*M_PI)/LUT_SIZE;
	switch(channel)
	{
	case 1:
		for(int i = 0; i < LUT_SIZE; i++)
		{
			//set LUT to be a sine wave and add noise
			dac_LUT_1[i] = midv + ((dac_maxv - midv) * sin((float)(i * angle)));
		}
		break;
	case 2:
		for(int i = 0; i < LUT_SIZE; i++)
		{
			//set LUT to be a sine wave and add noise
			dac_LUT_2[i] = midv + ((dac_maxv - midv) * sin((float)(i * angle)));
		}
		break;
	}
}

/*
 * Generate an triangular signal
 *
 * Param: int channel - # of DAC channel
 * 		  uint16_t dac_minv - DAC value of minimum voltage
 * 		  uint16_t dac_maxv - DAC value of max voltage
 * 		  int noise - # of noise bits
 */
void generate_triangular(int channel, uint16_t dac_minv, uint16_t dac_maxv, int noise)
{
	uint16_t slope = (dac_maxv - dac_minv)/(LUT_SIZE/2);
	switch(channel)
	{
	case 1:
		for(int i = 0; i < (LUT_SIZE / 2); i++)
		{
			//set positive slope part of triangular and add noise
			dac_LUT_1[i] = dac_minv + (i * slope);
		}
		for(int i = 0; i < (LUT_SIZE / 2); i++)
		{
			//set negative slope part of triangular and add noise
			dac_LUT_1[i + (LUT_SIZE / 2)] = dac_maxv - (i * slope);
		}
		break;
	case 2:
		for(int i = 0; i < (LUT_SIZE / 2); i++)
		{
			dac_LUT_2[i] = dac_minv + (i * slope);
		}
		for(int i = 0; i < (LUT_SIZE / 2); i++)
		{
			//set negative part of the triangular wave
			dac_LUT_2[i + (LUT_SIZE / 2)] = dac_maxv - (i * slope);
		}
		break;
	}
}

/*
 * Generate an arbitrary signal by copying over the EKG signal
 *
 * Param: int channel - # of DAC channel
 * 		  int noise - # of noise bits
 */
void generate_arbitrary(int channel, int noise)
{

	switch(channel)
	{
	case 1:
		for(int i = 0; i < LUT_SIZE; i++)
		{
			//set equal to EKG part of triangular and add noise
			dac_LUT_1[i] = ekg[i];
		}
		break;
	case 2:
		for(int i = 0; i < LUT_SIZE; i++)
		{
			//set equal to EKG part of triangular and add noise
			dac_LUT_2[i] = ekg[i];
		}
		break;
	}
}
