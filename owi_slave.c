/*
 * owi.c
 *
 *  Created on: 11 мар. 2021 г.
 *      Author: User
 */

#include	"main.h"
#include "owi_slave.h"

#include	<avr/interrupt.h>
#include	<util/delay.h>

volatile static uint8_t state;
volatile static uint16_t tp;
volatile static uint8_t byte;
volatile static uint8_t bit;
volatile static uint8_t pos;
volatile static uint8_t len;
volatile static uint8_t st;
volatile static uint8_t buff[9];

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
        OCR1A = TCNT1 + 480;
        MCUCR |= (1 << ISC00);
        GIFR = (1 << INTF0);
        TIFR = (1 << OCF1A);
        GICR |= (1 << INT0);
        TIMSK |= (1 << OCIE1A);
        if (st == OWI_ST_ANSWER) {
            if ((byte & 1) == 0) {
                DDRD |= 0x04;
                _delay_us(40);
                DDRD &= ~0x04;
            }
            byte >>= 1;
            bit++;
            if (bit == 8) {
                PORTB ^= 0x04;
                bit = 0;
                pos++;
                if (pos >= len) {
                    st = OWI_ST_CMD;
                } else {
                    byte = buff[pos];
                }
            }
        } else if (st == OWI_ST_CMD) {
            _delay_us(10);
            byte >>= 1;
            if (PIND & 0x04) {
                byte |= 0x80;
            }
            bit++;
            PORTB ^= 0x01;
            if (bit >= 8) {
                bit = 0;
                PORTB ^= 0x02;
                if (byte == 0x33) {
                    buff[0] = 0x10;
                    buff[1] = 0x5a;
                    buff[2] = 0xa9;
                    buff[3] = 0x75;
                    buff[4] = 0x03;
                    buff[5] = 0x08;
                    buff[6] = 0x00;
                    buff[7] = 0x61;
                    len = 8;
                    pos = 0;
                    byte = buff[0];
                    st = OWI_ST_ANSWER;
                } else if (byte == 0x44) {
                    ADMUX = (1 << REFS0) | (1);
                    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
                } else if (byte == 0xbe) {
                    buff[0] = temp[0];
                    buff[1] = temp[1];
                    buff[2] = 0x00;
                    buff[3] = 0x00;
                    buff[4] = 0xFF;
                    buff[5] = 0xFF;
                    buff[6] = 0x0C;
                    buff[7] = 0x10;
                    buff[8] = 0xAA;
                    len = 9;
                    pos = 0;
                    byte = buff[0];
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
        MCUCR |= (1 << ISC01);
        MCUCR &= ~(1 << ISC00);
        GIFR = (1 << INTF0);
        GICR |= (1 << INT0);
    }
}

ISR(TIMER1_OVF_vect) {
    PORTD ^= 0x20;
}
