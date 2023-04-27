/*
 * ring.h
 *
 *  Created on: Apr 19, 2023
 *      Author: Chris
 */

#ifndef INC_RING_H_
#define INC_RING_H_

typedef struct {
    //ID of own Nucleo
	unsigned char ringID[10];

	//ID and command of message
	unsigned char sourceID[10];
	unsigned char destID[10];
	unsigned char command[10];

	//params for commands
	unsigned char params[50]; //longer for the message
	unsigned char param_1[50]; //longer for the message
	unsigned char param_2[10];
	unsigned char param_3[10];
	unsigned char param_4[10];
	unsigned char param_5[10];
	unsigned char param_6[10];

} RING_r;

extern RING_r ring;

int ring_task_init(void);
void parse_channel_cmd(RING_r * r);

#endif /* INC_RING_H_ */
