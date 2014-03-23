/*
 * USART_atmega328p.h
 *
 *  Created on: Mar 22, 2014
 *      Author: eprasetio
 */

#ifndef USART_ATMEGA328P_H_
#define USART_ATMEGA328P_H_

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>

//#define BAUD 9600
//#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)    //The formula that does all the required maths

void usart_init(unsigned integer);
uint8_t usart_getc();
void usart_putc(uint8_t);
int usart_writeable();
int usart_readable();
void usart_putString(char*);

#endif /* USART_ATMEGA328P_H_ */
