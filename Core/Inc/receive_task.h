/*
 * receive_task.h
 *
 *  Created on: Mar 21, 2023
 *      Author: Chris
 */

#ifndef INC_RECEIVE_TASK_H_
#define INC_RECEIVE_TASK_H_

typedef struct {
    // some parameters to track
	unsigned char name[5];
	int channel;
	unsigned char type;
	float freq;
	uint16_t dac_minv;
	uint16_t dac_maxv;
	int noise;
} COMMAND_c;

extern COMMAND_c cmd;
int receive_task_init();

#endif /* INC_RECEIVE_TASK_H_ */
