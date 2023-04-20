/*
 * dac_channel_task.h
 *
 *  Created on: Mar 28, 2023
 *      Author: Chris
 */

#ifndef INC_CHANNEL_TASK_H_
#define INC_CHANNEL_TASK_H_

#define DAC_RESOLUTION 12
#define NUM_CHANNELS 2
#define LUT_SIZE 256
#define ADC_LUT_SIZE 20000

int channel_task_init(void);

//functions that generate pointers to the DAC after it has been populated with variables
void generate_DC(int channel, uint16_t dac_minv, int noise);

void generate_rectangle(int channel, uint16_t dac_minv, uint16_t dac_maxv, int noise);

void generate_sine(int channel, uint16_t dac_minv, uint16_t dac_maxv, int noise);

void generate_triangular(int channel, uint16_t dac_minv, uint16_t dac_maxv, int noise);

void generate_arbitrary(int channel, int noise);

#endif /* INC_CHANNEL_TASK_H_ */
