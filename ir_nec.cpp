/*
 * Copyright (C) 2010 Andreas Wetzel
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <wiring.h>

#include "ir_nec.h"


#define PULSEWIDTH  560
#define NEC_ZERO	PULSEWIDTH
#define NEC_ONE		(3*PULSEWIDTH)
#define NEC_REPEAT  (4*PULSEWIDTH)
#define NEC_START   (8*PULSEWIDTH)
#define NEC_STARTPULSE (16*PULSEWIDTH)

#define AROUND_10(x, tgt)	((x < 0.9*tgt) ? 0 : (x > 1.1*tgt) ? 0 : 1)
#define AROUND_20(x, tgt)	((x < 0.8*tgt) ? 0 : (x > 1.2*tgt) ? 0 : 1)

IRemoteNEC *g_ir = 0;

void ir_isr() {
	if (g_ir) {
		g_ir->isr();
	}
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static byte irqpins[] = {
		2, 3,
#if defined(__AVR_ATmega1280__)
		21, 20, 19, 18,
#endif
};

IRemoteNEC::IRemoteNEC( int irpin, int myremote)
{
	m_irpin = irpin;
	m_iraddress = myremote;
	m_outkey = KEY_NONE;

	pinMode( m_irpin, INPUT);
	// find interrupt for pin
	unsigned int i;
	for (i=0; i<ARRAY_SIZE( irqpins); i++) {
		if (irpin == irqpins[i]) {
			g_ir = this;
			attachInterrupt( i, ir_isr, CHANGE);
		}
	}
}

inline char IRemoteNEC::bswap( char v) {
	v = ( ( v & 0xaa ) >> 1 ) | ( ( v & 0x55 ) << 1 );
	v = ( ( v & 0xcc ) >> 2 ) | ( ( v & 0x33 ) << 2 );
	return ( ( v & 0xf0 ) >> 4 ) | ( ( v & 0x0f ) << 4 );
}

void IRemoteNEC::isr() {
	static int had_startpulse = 0;
	static unsigned long t_fall, t_rise;
	static unsigned long cur_value, num_bits;
	int newval = digitalRead( m_irpin);
	unsigned long duration;
	if (newval == HIGH) {
		t_rise = micros();
		duration = t_rise - t_fall;

		had_startpulse = AROUND_20( duration, NEC_STARTPULSE);
	}
	else {
		t_fall = micros();
		duration = t_fall - t_rise;

		if (AROUND_10( duration, NEC_REPEAT) && had_startpulse) {
			m_outkey = KEY_REPEAT;
			num_bits = 0;
		}
		if (AROUND_20( duration, NEC_ZERO)) {
			cur_value <<= 1;
			num_bits++;
		}
		else if (AROUND_20( duration, NEC_ONE)) {
			cur_value <<= 1;
			cur_value |= 1;
			num_bits++;
		}
		else if (AROUND_20( duration, NEC_START)) {
			cur_value = 0;
			num_bits = 0;
		}

		if (num_bits == 32) {
			int address = cur_value >> 16;
			if (address == m_iraddress) {
				char val = (cur_value & 0xff00) >> 8;
				if ((val ^ (cur_value & 0xff)) == 0xff) {
					m_outkey = bswap( val);
				}
			}
			num_bits = 0;
		}
	}
}

int IRemoteNEC::read() {
	int v = m_outkey;
	m_outkey = KEY_NONE;
	return v;
}
