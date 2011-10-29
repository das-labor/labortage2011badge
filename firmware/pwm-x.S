/* pwm-x.S */
/*
    This file is part of the Labotage2011_Badge.
    Copyright (C) 2011 Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>

.global reverse10

#define RED_BIT   4
#define GREEN_BIT 3
#define BLUE_BIT  1

.macro reverse10
	clr r23
	lsr r25
	ror r25
	ror r22 ; 1
	lsl r25
	ror r22 ; 2
	lsl r24
	ror r22 ; 3
	lsl r24
	ror r22 ; 4
	lsl r24
	ror r22 ; 5
	lsl r24
	ror r22 ; 6
	lsl r24
	ror r22 ; 7
	lsl r24
	ror r22 ; 8
	lsl r24
	rol r23
	bst r24, 7
	bld r23, 1
	movw r24, r22
/*	ret */
.endm

.comm counter, 2

.global update_pwm
update_pwm:
	ldi r30, lo8(counter)
	ldi r31, hi8(counter)
	ldd r24, Z+0
	ldd r25, Z+1
	adiw r24, 1
	std Z+0, r24
	std Z+1, r25
/*	rcall reverse10 */
	reverse10
	in r20, _SFR_IO_ADDR(TIFR)
	bst r20, 4	/* if OCF0A */
	brtc 20f
	andi r20, 0x10
	out _SFR_IO_ADDR(TIFR), r20 /* reset OCF0A */
;	rjmp 5f
	lds r26, fade_counter+0
	lds r27, fade_counter+1
	sbiw r26, 1
	sts fade_counter+0, r26
	sts fade_counter+1, r27
	brne 10f
5:
	out _SFR_IO_ADDR(TCCR0B), r1 /* deactivate timer */
10:	ldi r26, lo8(color_delta)
	ldi r27, hi8(color_delta)
20:	ldi r30, lo8(color)
	ldi r31, hi8(color)
check_red:
	ldd r22, Z+0
	ldd r23, Z+1
	cp  r24, r22
	cpc r25, r23
	brge red_off
	cbi _SFR_IO_ADDR(PORTB), RED_BIT
	rjmp add_red
red_off:
	sbi _SFR_IO_ADDR(PORTB), RED_BIT
add_red:
	brtc check_green
	ld r20, X+
	ld r21, X+
	add r22, r20
	adc r23, r21
	std Z+0, r22
	std Z+1, r23
check_green:
	ldd r22, Z+2
	ldd r23, Z+3
	cp  r24, r22
	cpc r25, r23
	brge green_off
	cbi _SFR_IO_ADDR(PORTB), GREEN_BIT
	rjmp add_green
green_off:
	sbi _SFR_IO_ADDR(PORTB), GREEN_BIT
add_green:
	brtc check_blue
	ld r20, X+
	ld r21, X+
	add r22, r20
	adc r23, r21
	std Z+2, r22
	std Z+3, r23
check_blue:
	ldd r22, Z+4
	ldd r23, Z+5
	cp  r24, r22
	cpc r25, r23
	brge blue_off
	cbi _SFR_IO_ADDR(PORTB), BLUE_BIT
	rjmp add_blue
blue_off:
	sbi _SFR_IO_ADDR(PORTB), BLUE_BIT
add_blue:
	brtc 1f
	ld r20, X+
	ld r21, X+
	add r22, r20
	adc r23, r21
	std Z+4, r22
	std Z+5, r23
1:
	ret

