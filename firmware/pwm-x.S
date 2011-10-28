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
	ror r25
	ror r25
	ror r22
	lsl r25
	ror r22
	lsl r24
	ror r22
	lsl r24
	ror r22
	lsl r24
	ror r22
	lsl r24
	ror r22
	lsl r24
	ror r22
	lsl r24
	ror r22
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
	ldD r25, Z+1
	adiw r24, 1
	std Z+0, r24
	std Z+1, r25
/*	rcall reverse10 */
	reverse10
	ldi r30, lo8(color)
	ldi r31, hi8(color)
check_red:
	ld r22, Z+
	ld r23, Z+
	cp  r24, r22
	cpc r25, r23
	brge red_off
	cbi _SFR_IO_ADDR(PORTB), RED_BIT
	rjmp check_green
red_off:
	sbi _SFR_IO_ADDR(PORTB), RED_BIT
check_green:
	ld r22, Z+
	ld r23, Z+
	cp  r24, r22
	cpc r25, r23
	brge green_off
	cbi _SFR_IO_ADDR(PORTB), GREEN_BIT
	rjmp check_blue
green_off:
	sbi _SFR_IO_ADDR(PORTB), GREEN_BIT
check_blue:
	ld r22, Z+
	ld r23, Z
	cp  r24, r22
	cpc r25, r23
	brge blue_off
	cbi _SFR_IO_ADDR(PORTB), BLUE_BIT
	ret
blue_off:
	sbi _SFR_IO_ADDR(PORTB), BLUE_BIT
	ret

