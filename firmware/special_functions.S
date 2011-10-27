/* special_functions.S */
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

.global exec_spm
/* void exec_spm(uint16_t z, uint16_t r0r1, void* dest, void* src, uint8_t len); */
exec_spm:
	push r28
	push r29
	movw r30, r24
	movw r0, r22
	movw r28, r20
	movw r26, r18
	movw r24, r16
	in r23, _SFR_IO_ADDR(SREG)
	cli
	tst r24
	breq spm_cmd
	dec r24
	breq last_store
copy_loop:
	ld r22, X+
	st Y+, r22
	dec r24
	brne copy_loop
last_store:
	ld r22, X+
	st Y+, r22
spm_cmd:
	spm
	out _SFR_IO_ADDR(SREG), r23
	clr r1
	pop r29
	pop r28
	ret
