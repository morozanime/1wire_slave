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

	owi_ROM[0] = 0x10;
	owi_ROM[1] = 0x5a;
	owi_ROM[2] = 0xa9;
	owi_ROM[3] = 0x75;
	owi_ROM[4] = 0x03;
	owi_ROM[5] = 0x08;
	owi_ROM[6] = 0x00;
	owi_ROM[7] = owi_calc_crc(owi_ROM, 7);

	owi_SCRATCHPAD[0] = 0xAA;
	owi_SCRATCHPAD[1] = 0x00;
	owi_SCRATCHPAD[2] = 0x00;
	owi_SCRATCHPAD[3] = 0x00;
	owi_SCRATCHPAD[4] = 0xff;
	owi_SCRATCHPAD[5] = 0xff;
	owi_SCRATCHPAD[6] = 0x0c;
	owi_SCRATCHPAD[7] = 0x10;
	owi_SCRATCHPAD[8] = owi_calc_crc(owi_SCRATCHPAD, 8);

	owi_init();
	sei();
	while (1) {
		if (ADCSRA & (1 << ADIF)) {
			ADCSRA |= (1 << ADIF);
			uint16_t x = ADC;
			if (x < 512) {
				x /= 6;
				owi_SCRATCHPAD[0] = ((uint8_t) x) - (uint8_t) (512 / 6);
				owi_SCRATCHPAD[1] = 0xff;
			} else {
				x = (x - 512) / 4;
				owi_SCRATCHPAD[0] = (uint8_t) x;
				owi_SCRATCHPAD[1] = 0x00;
			}
			owi_SCRATCHPAD[8] = owi_calc_crc(owi_SCRATCHPAD, 8);
		}
	}
}
