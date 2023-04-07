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

/**
 * @file
 * @brief Common utility functions
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "internal.h"
#include "config.h"

/* Contributed by Michel Such <michel.such@free.fr> */
#ifdef _WIN32

/* Adapted from BRLTTY code (see sys_progs_wihdows.h) */

#include <shlobj.h>

static void *
reallocWrapper(void *address, size_t size) {
	if (!(address = realloc(address, size)) && size) _lou_outOfMemory();
	return address;
}

static char *
strdupWrapper(const char *string) {
	char *address = strdup(string);
	if (!address) _lou_outOfMemory();
	return address;
}

char *EXPORT_CALL
lou_getProgramPath(void) {
	char *path = NULL;
	HMODULE handle;

	if ((handle = GetModuleHandle(NULL))) {
		DWORD size = 0X80;
		char *buffer = NULL;

		while (1) {
			buffer = reallocWrapper(buffer, size <<= 1);

			{
				// As the "UNICODE" Windows define may have been set at compilation,
				// This call must be specifically GetModuleFilenameA as further code
				// expects it to be single byte chars.
				DWORD length = GetModuleFileNameA(handle, buffer, size);

				if (!length) {
					printf("GetModuleFileName\n");
					exit(3);
				}

				if (length < size) {
					buffer[length] = 0;
					path = strdupWrapper(buffer);

					while (length > 0)
						if (path[--length] == '\\') break;

					strncpy(path, path, length + 1);
					path[length + 1] = '\0';
					break;
				}
			}
		}

		free(buffer);
	} else {
		printf("GetModuleHandle\n");
		exit(3);
	}

	return path;
}
#endif
/* End of MS contribution */

static widechar
toLowercase(widechar c, const TranslationTableHeader *table) {
	static TranslationTableOffset offset;
	static TranslationTableCharacter *character;
	offset = table->characters[_lou_charHash(c)];
	while (offset) {
		character = (TranslationTableCharacter *)&table->ruleArea[offset];
		if (character->value == c) {
			if (character->mode & CTC_UpperCase) {
				const TranslationTableCharacter *c = character;
				if (c->basechar)
					c = (TranslationTableCharacter *)&table->ruleArea[c->basechar];
				while (1) {
					if ((c->mode & (character->mode & ~CTC_UpperCase)) ==
							(character->mode & ~CTC_UpperCase))
						return c->value;
					if (!c->linked) break;
					c = (TranslationTableCharacter *)&table->ruleArea[c->linked];
				}
			}
			return character->value;
		}
		offset = character->next;
	}
	return c;
}

unsigned long int EXPORT_CALL
_lou_stringHash(const widechar *c, int lowercase, const TranslationTableHeader *table) {
	if (!lowercase)
		return (((unsigned long int)c[0] << 8) + (unsigned long int)c[1]) % HASHNUM;
	else
		return (((unsigned long int)toLowercase(c[0], table) << 8) +
					   (unsigned long int)toLowercase(c[1], table)) %
				HASHNUM;
}

unsigned long int EXPORT_CALL
_lou_charHash(widechar c) {
	return (unsigned long int)c % HASHNUM;
}

const char *EXPORT_CALL
_lou_showString(widechar const *chars, int length, int forceHex) {
	/* Translate a string of characters to the encoding used in character
	 * operands */
	static char scratchBuf[MAXSTRING];
	int bufPos = 0;
	scratchBuf[bufPos++] = '\'';

	for (int charPos = 0; (charPos < length) && (bufPos < (MAXSTRING - 2));
			charPos += 1) {
		widechar c = chars[charPos];

		if (!forceHex && isASCII(c)) {
			scratchBuf[bufPos++] = (char)c;
		} else {
			char hexbuf[20];
			int hexLength;
			char escapeLetter;

			int leadingZeros;
			int hexPos;
			hexLength = sprintf(hexbuf, "%x", c);
			switch (hexLength) {
			case 1:
			case 2:
			case 3:
			case 4:
				escapeLetter = 'x';
				leadingZeros = 4 - hexLength;
				break;
			case 5:
				escapeLetter = 'y';
				leadingZeros = 0;
				break;
			case 6:
			case 7:
			case 8:
				escapeLetter = 'z';
				leadingZeros = 8 - hexLength;
				break;
			default:
				escapeLetter = '?';
				leadingZeros = 0;
				break;
			}
			if ((bufPos + leadingZeros + hexLength + 4) >= (MAXSTRING - 2)) break;
			scratchBuf[bufPos++] = '\\';
			scratchBuf[bufPos++] = escapeLetter;
			for (hexPos = 0; hexPos < leadingZeros; hexPos++) scratchBuf[bufPos++] = '0';
			for (hexPos = 0; hexPos < hexLength; hexPos++)
				scratchBuf[bufPos++] = hexbuf[hexPos];
		}
	}
	scratchBuf[bufPos++] = '\'';
	scratchBuf[bufPos] = 0;
	return scratchBuf;
}

/**
 * Mapping between braille dot and textual representation as used in dots operands
 */
static const intCharTupple dotMapping[] = {
	{ LOU_DOT_1, '1' },
	{ LOU_DOT_2, '2' },
	{ LOU_DOT_3, '3' },
	{ LOU_DOT_4, '4' },
	{ LOU_DOT_5, '5' },
	{ LOU_DOT_6, '6' },
	{ LOU_DOT_7, '7' },
	{ LOU_DOT_8, '8' },
	{ LOU_DOT_9, '9' },
	{ LOU_DOT_10, 'A' },
	{ LOU_DOT_11, 'B' },
	{ LOU_DOT_12, 'C' },
	{ LOU_DOT_13, 'D' },
	{ LOU_DOT_14, 'E' },
	{ LOU_DOT_15, 'F' },
	{ 0, 0 },
};

/**
 * Print out dot numbers
 *
 * @return a string containing the dot numbers. The longest possible
 * output is "\123456789ABCDEF0/"
 */
const char *EXPORT_CALL
_lou_unknownDots(widechar dots) {
	static char buffer[20];

	int k = 0;
	buffer[k++] = '\\';

	for (int mappingPos = 0; dotMapping[mappingPos].key; mappingPos++) {
		if (dots & dotMapping[mappingPos].key) buffer[k++] = dotMapping[mappingPos].value;
	}

	if (k == 1) buffer[k++] = '0';
	buffer[k++] = '/';
	buffer[k] = 0;
	return buffer;
}

/**
 * Translate a sequence of dots to the encoding used in dots operands.
 */
const char *EXPORT_CALL
_lou_showDots(widechar const *dots, int length) {
	int bufPos = 0;
	static char scratchBuf[MAXSTRING];
	for (int dotsPos = 0; dotsPos < length && bufPos < (MAXSTRING - 1); dotsPos++) {
		for (int mappingPos = 0; dotMapping[mappingPos].key; mappingPos++) {
			if ((dots[dotsPos] & dotMapping[mappingPos].key) &&
					(bufPos < (MAXSTRING - 1)))
				scratchBuf[bufPos++] = dotMapping[mappingPos].value;
		}
		if ((dots[dotsPos] == LOU_DOTS) && (bufPos < (MAXSTRING - 1)))
			scratchBuf[bufPos++] = '0';
		if ((dotsPos != length - 1) && (bufPos < (MAXSTRING - 1)))
			scratchBuf[bufPos++] = '-';
	}
	scratchBuf[bufPos] = 0;
	return scratchBuf;
}

/**
 * Mapping between character attribute and textual representation
 */
static const intCharTupple attributeMapping[] = {
	{ CTC_Space, 's' },
	{ CTC_Letter, 'l' },
	{ CTC_Digit, 'd' },
	{ CTC_Punctuation, 'p' },
	{ CTC_UpperCase, 'U' },
	{ CTC_LowerCase, 'u' },
	{ CTC_Math, 'm' },
	{ CTC_Sign, 'S' },
	{ CTC_LitDigit, 'D' },
	{ CTC_UserDefined9, 'w' },
	{ CTC_UserDefined10, 'x' },
	{ CTC_UserDefined11, 'y' },
	{ CTC_UserDefined12, 'z' },
	{ 0, 0 },
};

/**
 * Show attributes using the letters used after the $ in multipass
 * opcodes.
 */
char *EXPORT_CALL
_lou_showAttributes(TranslationTableCharacterAttributes a) {
	int bufPos = 0;
	static char scratchBuf[MAXSTRING];
	for (int mappingPos = 0; attributeMapping[mappingPos].key; mappingPos++) {
		if ((a & attributeMapping[mappingPos].key) && bufPos < (MAXSTRING - 1))
			scratchBuf[bufPos++] = attributeMapping[mappingPos].value;
	}
	scratchBuf[bufPos] = 0;
	return scratchBuf;
}

void EXPORT_CALL
_lou_outOfMemory(void) {
	_lou_logMessage(LOU_LOG_FATAL, "liblouis: Insufficient memory\n");
	exit(3);
}

#ifdef DEBUG
void EXPORT_CALL
_lou_debugHook(void) {
	char *hook = "debug hook";
	printf("%s\n", hook);
}
#endif

static const int validTranslationModes[] = { noContractions, compbrlAtCursor, dotsIO,
	compbrlLeftCursor, ucBrl, noUndefined, partialTrans };

int EXPORT_CALL
_lou_isValidMode(int mode) {
	// mask out all valid mode bits. If you end up with some bits set
	// then the input isn't valid. See
	// https://en.wikipedia.org/wiki/Material_nonimplication
	for (int i = 0; i < (sizeof(validTranslationModes) / sizeof(*validTranslationModes));
			i++)
		mode &= ~validTranslationModes[i];
	return !mode;
}

/* Map char to dots according to North American Braille Computer Code (NABCC) */
widechar EXPORT_CALL
_lou_charToFallbackDots(widechar c) {
	static const unsigned char charToDots[] = {
		/* ASCII characters 0X00-0X1F - control characters.
		 * These won't be referenced so we have room for data.
		 * These groups must be in descending order.
		 * Each group contains the following four bytes:
		 * 1) The first character to which this block applies.
		 * 2) The bits to remove from the character.
		 * 3) The bits to add to the character.
		 * 4) The dots to add to the braille pattern.
		 */
		// clang-format off
		0X7F, 0X20, 0X00, LOU_DOT_7,
		0X60, 0X20, 0X00, 0,
		0X5F, 0X00, 0X00, 0,
		0X40, 0X00, 0X00, LOU_DOT_7,
		0X20, 0X00, 0X00, 0,
		0X00, 0X00, 0X40, LOU_DOT_7 | LOU_DOT_8,

		// ASCII characters 0X20-0X3F - digits and common symbols.
		[' '] = 0,
		['!'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_6,
		['"'] = LOU_DOT_5,
		['#'] = LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,
		['$'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_4 | LOU_DOT_6,
		['%'] = LOU_DOT_1 | LOU_DOT_4 | LOU_DOT_6,
		['&'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_6,
		['\''] = LOU_DOT_3,
		['('] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_5 | LOU_DOT_6,
		[')'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,
		['*'] = LOU_DOT_1 | LOU_DOT_6,
		['+'] = LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_6,
		[','] = LOU_DOT_6,
		['-'] = LOU_DOT_3 | LOU_DOT_6,
		['.'] = LOU_DOT_4 | LOU_DOT_6,
		['/'] = LOU_DOT_3 | LOU_DOT_4,
		['0'] = LOU_DOT_3 | LOU_DOT_5 | LOU_DOT_6,
		['1'] = LOU_DOT_2,
		['2'] = LOU_DOT_2 | LOU_DOT_3,
		['3'] = LOU_DOT_2 | LOU_DOT_5,
		['4'] = LOU_DOT_2 | LOU_DOT_5 | LOU_DOT_6,
		['5'] = LOU_DOT_2 | LOU_DOT_6,
		['6'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_5,
		['7'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_5 | LOU_DOT_6,
		['8'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_6,
		['9'] = LOU_DOT_3 | LOU_DOT_5,
		[':'] = LOU_DOT_1 | LOU_DOT_5 | LOU_DOT_6,
		[';'] = LOU_DOT_5 | LOU_DOT_6,
		['<'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_6,
		['='] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,
		['>'] = LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5,
		['?'] = LOU_DOT_1 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,

		// ASCII characters 0X40-0X5F - letters and other symbols.
		['@'] = LOU_DOT_4,
		['A'] = LOU_DOT_1,
		['B'] = LOU_DOT_1 | LOU_DOT_2,
		['C'] = LOU_DOT_1 | LOU_DOT_4,
		['D'] = LOU_DOT_1 | LOU_DOT_4 | LOU_DOT_5,
		['E'] = LOU_DOT_1 | LOU_DOT_5,
		['F'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_4,
		['G'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_4 | LOU_DOT_5,
		['H'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_5,
		['I'] = LOU_DOT_2 | LOU_DOT_4,
		['J'] = LOU_DOT_2 | LOU_DOT_4 | LOU_DOT_5,
		['K'] = LOU_DOT_1 | LOU_DOT_3,
		['L'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3,
		['M'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_4,
		['N'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5,
		['O'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_5,
		['P'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4,
		['Q'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5,
		['R'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_5,
		['S'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4,
		['T'] = LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5,
		['U'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_6,
		['V'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_3 | LOU_DOT_6,
		['W'] = LOU_DOT_2 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,
		['X'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_6,
		['Y'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,
		['Z'] = LOU_DOT_1 | LOU_DOT_3 | LOU_DOT_5 | LOU_DOT_6,
		['['] = LOU_DOT_2 | LOU_DOT_4 | LOU_DOT_6,
		['\\'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_5 | LOU_DOT_6,
		[']'] = LOU_DOT_1 | LOU_DOT_2 | LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6,
		['^'] = LOU_DOT_4 | LOU_DOT_5,
		['_'] = LOU_DOT_4 | LOU_DOT_5 | LOU_DOT_6
		// clang-format on
	};

	if (c >= 0X80) c = '?';
	widechar dots = LOU_DOTS;

	{
		const unsigned char *p = charToDots;
		while (*p > c) p += 4;

		c &= ~*++p;
		c |= *++p;
		dots |= *++p;
	}

	dots |= charToDots[c];
	return dots;
}
