/*
 * MFS.c
 *
 *  Created on: Aug 31, 2021
 *      Author: rickweil, modified by Christian Secular to operate only LEDs
 */
#include "stm32l476xx.h"
#include "MFS.h"
#include "ctype.h"
#include "stdio.h"
#include "string.h"

//////////////////////////////////////////
// private variables
//#define FreeRTOS_REFRESH

typedef struct
{
	GPIO_TypeDef *port;
	uint8_t pin;
	uint8_t otype;
	uint8_t ospeed;
	uint8_t init_value;
} GPIO_OUT_t;

/// GPIO stuff
static GPIO_OUT_t leds[] =
{
	{ .port=GPIOA, .pin=5, .otype=0, .ospeed=0, .init_value=1},	// place holder
	{ .port=GPIOA, .pin=5, .otype=0, .ospeed=0, .init_value=1},	// LED 1 d13 PA5
	{ .port=GPIOA, .pin=6, .otype=0, .ospeed=0, .init_value=1},	// LED 2 d12 PA6
	{ .port=GPIOA, .pin=7, .otype=0, .ospeed=0, .init_value=1},	// LED 3 d11 PA7
	{ .port=GPIOB, .pin=6, .otype=0, .ospeed=0, .init_value=1},	// LED 4 d10 PB6
};


//////////////////////////////////////////
// private functions

static void gpio_config_output(GPIO_OUT_t *gpio)
{
	GPIO_TypeDef *port = gpio->port;
	uint32_t pin = gpio->pin;

	// First, configure as an output
    port->MODER &= ~(0x3 << (pin*2)) ;      // clear the two MODE bits for this pin
    port->MODER |=  1 << (pin*2)  ;        	// 1 => output

	// ...and then the selected drive
	port->OTYPER &= ~(0x1 << pin) ;
	port->OTYPER |= (gpio->otype << pin) ;

	// ...with selected speed
	port->OSPEEDR &= ~(0x3 << (pin*2)) ; 	// clear the two OSPEED bits for this pin
	port->OSPEEDR |= gpio->ospeed << (pin*2) ;

	// ...set initial value
	port->ODR &= ~(0x1 << pin);
	port->ODR |= (gpio->init_value << pin);
}

//////////////////////////////////////////
// public functions

void MFS_init(void)
{
    // Enable the clock to GPIO Ports A, and B
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;    // enable clock some MFS buttons / LEDs
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;    // enable clock some MFS buttons / LEDs

	// configure the LEDs as GPIO outputs
	for(int ii=1; ii<=4; ii++) {
		gpio_config_output(&leds[ii]);
	}
}

// Turns LED `num` to 'on' if non-zero, or off if zero
void MFS_set_led( uint8_t num, uint32_t on )
{
	while(num > 4);	// hang if invalid input

	GPIO_TypeDef *port = leds[num].port;
	uint32_t bit = 1 << leds[num].pin;

	if ( on )
		port->ODR &= ~bit ;	// active low
	else
		port->ODR |= bit ;
}

// Toggles LED `num`
void MFS_toggle_led( uint8_t num )
{
	while(num > 4);	// hang if invalid input

	GPIO_TypeDef *port = leds[num].port;
	uint32_t bit = 1 << leds[num].pin;

	if ( port->IDR & bit )
		port->ODR &= ~bit ;
	else
		port->ODR |= bit ;
}
