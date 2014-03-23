/*
 * USART_atmega328p.c
 *
 *  Created on: Mar 22, 2014
 *      Author: eprasetio
 */
#include "USART_atmega328p.h"

void usart_init(unsigned int baudrate) {
	//set baud rate
	UBRR0H = (uint8_t) (baudrate >> 8);
	UBRR0L = (uint8_t) (baudrate);

	//enable receiver and transmitter
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

uint8_t usart_getc() {
	//wait until receiver buffer is filled up
	while (!(UCSR0A & (1 << RXC0)))
		;
	//return the data inside the buffer
	return UDR0 ;

}

void usart_putc(uint8_t data) {
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
		;

	/* Put data into buffer, sends the data */
	UDR0 = data;
}

int usart_isWriteable() {
	int status = 0; //return false by default

	//if transmit register is empty, return true
	if ((UCSR0A & (1 << UDRE0))) {
		status = 1;
	}

	return status;
}

int usart_isReadable() {
	int status = 0; //return false by default

	//if there is data in receive buffer, return true
	if ((UCSR0A & (1 << RXC0))) {
		status = 1;
	}

	return status;
}

void usart_putString(char* StringPtr) {
	while (*StringPtr != 0x00) { //Here we check if there is still more chars to send, this is done checking the actual char and see if it is different from the null char
		usart_putc(*StringPtr); //Using the simple send function we send one char at a time
		StringPtr++;
	}        //We increment the pointer so we can read the next char

}
