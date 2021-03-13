/*
 * uart.c
 *
 *  Created on: 11 мар. 2021 г.
 *      Author: User
 */

#include	"main.h"
#include	<avr/interrupt.h>
#include	<stddef.h>
#include	"uart.h"

static uint8_t tx_buff[256];
static uint8_t tx_pos_put;
volatile static uint8_t tx_pos_get;

static uint8_t rx_buff[256];
volatile static uint8_t rx_pos_put;
static uint8_t rx_pos_get;

void uart_init(uint16_t baud) {
	UBRRH = (uint8_t) (baud >> 8);
	UBRRL = (uint8_t) baud;
	UCSRA = 1 << U2X;
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}

uint8_t uart_read_len(void) {
	return rx_pos_put - rx_pos_get;
}

uint8_t uart_read_byte(void) {
	if (rx_pos_put != rx_pos_get)
		return rx_buff[rx_pos_get++];
	else
		return 0;
}

uint8_t uart_read_bytes(uint8_t *buff, uint8_t len) {
	uint8_t l = rx_pos_put - rx_pos_get;
	if (l > 0) {
		if (buff == NULL) {
			rx_pos_get = rx_pos_put;
		} else {
			if (len < l)
				l = len;
			for (uint8_t i = 0; i < l; i++)
				buff[i] = rx_buff[rx_pos_get++];
		}
	}
	return l;
}

void uart_puts(uint8_t *str) {
	uint8_t i = 0;
	for (; i < sizeof(tx_buff); i++) {
		uint8_t c = str[i];
		if (c == 0)
			break;
		tx_buff[tx_pos_put++] = c;
	}
	if (i > 0)
		UCSRB |= (1 << UDRIE);
}

void uart_putsn(uint8_t *str, uint8_t len) {
	uint8_t i = 0;
	for (; i < sizeof(tx_buff) && i < len; i++)
		tx_buff[tx_pos_put++] = str[i];
	if (i > 0)
		UCSRB |= (1 << UDRIE);
}

void uart_putc(uint8_t c) {
	tx_buff[tx_pos_put++] = c;
	UCSRB |= (1 << UDRIE);
}

ISR(USART_UDRE_vect) {
	UDR = tx_buff[tx_pos_get++];
	if (tx_pos_put == tx_pos_get)
		UCSRB &= ~(1 << UDRIE);
}

ISR(USART_RXC_vect) {
	static uint8_t pos = 0;
	uint8_t c = UDR;
	if (c == '\n' || c == '\r')
		rx_pos_put = pos;
	else
		rx_buff[pos++] = c;
}
