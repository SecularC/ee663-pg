/*
 * ring.h
 *
 *  Created on: Apr 19, 2023
 *      Author: Chris
 */

#ifndef INC_RING_H_
#define INC_RING_H_

typedef struct {
    // some parameters to track
	unsigned char sourceID[10];
	unsigned char destID[10];
	unsigned char command[10];
	unsigned char parameters[20];
} RING_r;

int ring_task_init(void);

#endif /* INC_RING_H_ */
