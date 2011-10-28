/* special_functions.h */
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

#ifndef SPECIAL_FUNCTIONS_H_
#define SPECIAL_FUNCTIONS_H_

#include <stdint.h>

void exec_spm(uint16_t z, uint16_t r0r1, void* dest, void* src, uint8_t len);
void soft_reset(uint8_t delay);

#endif /* SPECIAL_FUNCTIONS_H_ */
