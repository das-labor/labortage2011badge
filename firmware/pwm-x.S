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

.global reverse10

reverse10:
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
	bld r24, 7
	bst r23, 1
	movw r24, r22
	ret

