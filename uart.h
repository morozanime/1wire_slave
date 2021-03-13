/*
 * uart.h
 *
 *  Created on: 11 мар. 2021 г.
 *      Author: User
 */

#ifndef UART_H_
#define UART_H_

#define	BAUD(x)	((F_CPU/8UL/(x))-1UL)

void uart_init(uint16_t baud);
uint8_t uart_read_len(void);
uint8_t uart_read_byte(void);
uint8_t uart_read_bytes(uint8_t *buff, uint8_t len);
void uart_puts(uint8_t *str);
void uart_putsn(uint8_t *str, uint8_t len);
void uart_putc(uint8_t c);

#endif /* UART_H_ */
