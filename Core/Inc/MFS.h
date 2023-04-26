/*
 * MFS.h
 *
 *  Created on: Aug 31, 2021
 *      Author: rickweil
 */


#ifndef MFS_H_
#define MFS_H_

#include "stm32l476xx.h"

void MFS_init(void);

/// Turns LED `num` to 'on' if non-zero, or off if zero
void MFS_set_led( uint8_t num, uint32_t on );

/// Toggles LED `num`
void MFS_toggle_led( uint8_t num );

/// Returns 1 if button `num if pressed, 0 otherwise
uint8_t MFS_button_pressed( uint8_t num );

// print ascii to 7-segment display
void MFS_print_str(char *ascii);

// print integer to 7-segment display
void MFS_print_int(int integer);

// print float value to 7-segment display
void MFS_print_float(double value);

// call repeatedly to refresh the 7-segment display, 1 char each call
void MFS_7seg_refresh();

#endif /* MFS_H_ */
