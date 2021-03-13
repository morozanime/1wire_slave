#include	"main.h"

#include	<avr/interrupt.h>
#include	<inttypes.h>
#include	<util/delay.h>
#include	<string.h>
#include	<stdio.h>

#include	"owi_slave.h"
#include 	"uart.h"

//-------------------------------------------------------------------
int main(void) {
	DDRB = 0x3F;
	PORTB = 0x00;

	DDRD = 0xFE;
	PORTD = 0xFD;

	DDRC = 0x00;
	PORTC = 0x3E;

	uart_init(BAUD(9600));
	owi_init();
	sei();
	while (1) {
		if (ADCSRA & (1 << ADIF)) {
			ADCSRA |= (1 << ADIF);
			uint16_t x = ADC;
			if (x < 512) {
				x /= 6;
				temp[0] = ((uint8_t) x) - (uint8_t) (512 / 6);
				temp[1] = 0xff;
			} else {
				x = (x - 512) / 4;
				temp[0] = (uint8_t) x;
				temp[1] = 0x00;
			}
//				temp[0] = ADCL;
//				temp[1] = ADCH;
		}
	}
}
