/*
 * owi.c
 *
 *  Created on: 11 мар. 2021 г.
 *      Author: User
 */

#include	"main.h"
#include 	"owi_slave.h"

#include	<avr/interrupt.h>
#include	<util/delay.h>
#include	<string.h>

static uint8_t state;
static uint8_t bit;
static uint8_t pos;
static uint8_t len;
static uint8_t st;
static uint8_t buff[9];

#define	DELAY_US(x)	((F_CPU/1000000*(x))/8)

void owi_init(void) {
	state = OWI_IDLE;
	DDRD &= ~0x04;
	PORTD &= ~0x04;
	TCCR1A = 0;
	TCCR1B = 1 << CS11;
	MCUCR |= (1 << ISC01);
	MCUCR &= ~(1 << ISC00);
	GIFR = (1 << INTF0);
	TIFR = (1 << OCF1A);
	GICR |= (1 << INT0);
}

/* Подсчет CRC8 массива mas длиной Len */
uint8_t owi_calc_crc(uint8_t *mas, uint8_t Len) {
	uint8_t i, dat, crc, fb, st_byt;
	st_byt = 0;
	crc = 0;
	do {
		dat = mas[st_byt];
		for (i = 0; i < 8; i++) {  // счетчик битов в байте
			fb = crc ^ dat;
			fb &= 1;
			crc >>= 1;
			dat >>= 1;
			if (fb == 1)
				crc ^= 0x8c; // полином
		}
		st_byt++;
	} while (st_byt < Len); // счетчик байтов в массиве
	return crc;
}

ISR(TIMER1_COMPA_vect) {
	TIMSK &= ~(1 << OCIE1A);
	if (state == OWI_WF1)
		state = OWI_RESET_WF1;
}

ISR(TIMER1_COMPB_vect) {
	TIMSK &= ~(1 << OCIE1B);
	DDRD &= ~0x04;
}

ISR(INT0_vect) {
	GICR &= ~(1 << INT0);
	if (state == OWI_IDLE) {
		state = OWI_WF1;
		OCR1A = TCNT1 + DELAY_US(480);
		MCUCR |= (1 << ISC00);
		GIFR = (1 << INTF0);
		TIFR = (1 << OCF1A);
		GICR |= (1 << INT0);
		TIMSK |= (1 << OCIE1A);
		if (st == OWI_ST_ANSWER) {
			if ((buff[0] & 1) == 0) {
				DDRD |= 0x04;
				_delay_us(40);
				DDRD &= ~0x04;
			}
			buff[0] >>= 1;
			bit++;
			if (bit == 8) {
				bit = 0;
				pos++;
				if (pos >= len) {
					st = OWI_ST_CMD;
				} else {
					buff[0] = buff[pos];
				}
			}
		} else if (st == OWI_ST_CMD) {
			_delay_us(10);
			buff[0] >>= 1;
			if (PIND & 0x04) {
				buff[0] |= 0x80;
			}
			bit++;
			if (bit >= 8) {
				bit = 0;
				if (buff[0] == 0x33) {
					memcpy(buff, owi_ROM, sizeof(owi_ROM));
					len = sizeof(owi_ROM);
					pos = 0;
					st = OWI_ST_ANSWER;
				} else if (buff[0] == 0x44) {
					ADMUX = (1 << REFS0) | (1);
					ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2)
							| (1 << ADPS1) | (1 << ADPS0);
				} else if (buff[0] == 0xbe) {
					memcpy(buff, owi_SCRATCHPAD, sizeof(owi_SCRATCHPAD));
					len = sizeof(owi_SCRATCHPAD);
					pos = 0;
					st = OWI_ST_ANSWER;
				}
			}
		}
		if (PIND & 0x04) {
			MCUCR &= ~(1 << ISC00);
			TIMSK &= ~(1 << OCIE1A);
			GIFR = (1 << INTF0);
			state = OWI_IDLE;
		}
	} else if (state == OWI_WF1) {
		TIMSK &= ~(1 << OCIE1A);
		state = OWI_IDLE;
		MCUCR &= ~(1 << ISC00);
		GIFR = (1 << INTF0);
		GICR |= (1 << INT0);
	} else if (state == OWI_RESET_WF1) {
		_delay_us(30);
		DDRD |= 0x04;
		_delay_us(150);
		DDRD &= ~0x04;
		state = OWI_IDLE;
		st = OWI_ST_CMD;
		bit = 0;
		MCUCR &= ~(1 << ISC00);
		GIFR = (1 << INTF0);
		GICR |= (1 << INT0);
	}
}
