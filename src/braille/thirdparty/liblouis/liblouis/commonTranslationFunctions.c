/* liblouis Braille Translation and Back-Translation Library

   Based on the Linux screenreader BRLTTY, copyright (C) 1999-2006 by The
   BRLTTY Team

   Copyright (C) 2004, 2005, 2006 ViewPlus Technologies, Inc. www.viewplus.com
   Copyright (C) 2004, 2005, 2006 JJB Software, Inc. www.jjb-software.com
   Copyright (C) 2016 Mike Gray, American Printing House for the Blind
   Copyright (C) 2016 Davy Kager, Dedicon

   This file is part of liblouis.

   liblouis is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 2.1 of the License, or
   (at your option) any later version.

   liblouis is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with liblouis. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include "internal.h"

static int passVariables[NUMVAR];

void EXPORT_CALL
_lou_resetPassVariables(void) {
	memset(passVariables, 0, sizeof(passVariables[0]) * NUMVAR);
}

int EXPORT_CALL
_lou_handlePassVariableTest(const widechar *instructions, int *IC, int *itsTrue) {
	switch (instructions[*IC]) {
	case pass_eq:
		if (passVariables[instructions[*IC + 1]] != instructions[*IC + 2]) *itsTrue = 0;
		*IC += 3;
		return 1;

	case pass_lt:
		if (passVariables[instructions[*IC + 1]] >= instructions[*IC + 2]) *itsTrue = 0;
		*IC += 3;
		return 1;

	case pass_gt:
		if (passVariables[instructions[*IC + 1]] <= instructions[*IC + 2]) *itsTrue = 0;
		*IC += 3;
		return 1;

	case pass_lteq:
		if (passVariables[instructions[*IC + 1]] > instructions[*IC + 2]) *itsTrue = 0;
		*IC += 3;
		return 1;

	case pass_gteq:
		if (passVariables[instructions[*IC + 1]] < instructions[*IC + 2]) *itsTrue = 0;
		*IC += 3;
		return 1;

	default:
		return 0;
	}
}

int EXPORT_CALL
_lou_handlePassVariableAction(const widechar *instructions, int *IC) {
	switch (instructions[*IC]) {
	case pass_eq:
		passVariables[instructions[*IC + 1]] = instructions[*IC + 2];
		*IC += 3;
		return 1;

	case pass_hyphen:
		passVariables[instructions[*IC + 1]] -= 1;
		if (passVariables[instructions[*IC + 1]] < 0)
			passVariables[instructions[*IC + 1]] = 0;
		*IC += 2;
		return 1;

	case pass_plus:
		passVariables[instructions[*IC + 1]] += 1;
		*IC += 2;
		return 1;

	default:
		return 0;
	}
}
