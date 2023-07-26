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
 * @brief Read and compile translation tables
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

#define QUOTESUB 28 /* Stand-in for double quotes in strings */

/* needed to make debuggin easier */
#ifdef DEBUG
wchar_t wchar;
#endif

/* The following variables and functions make it possible to specify the
 * path on which all tables for liblouis and all files for liblouisutdml,
 * in their proper directories, will be found.
 */

static char *dataPathPtr;

char *EXPORT_CALL
lou_setDataPath(const char *path) {
	static char dataPath[MAXSTRING];
	dataPathPtr = NULL;
	if (path == NULL) return NULL;
	strcpy(dataPath, path);
	dataPathPtr = dataPath;
	return dataPathPtr;
}

char *EXPORT_CALL
lou_getDataPath(void) {
	return dataPathPtr;
}

/* End of dataPath code. */

static int
eqasc2uni(const unsigned char *a, const widechar *b, const int len) {
	int k;
	for (k = 0; k < len; k++)
		if ((widechar)a[k] != b[k]) return 0;
	return 1;
}

typedef struct CharsString {
	widechar length;
	widechar chars[MAXSTRING];
} CharsString;

static int errorCount;
static int warningCount;

typedef struct TranslationTableChainEntry {
	struct TranslationTableChainEntry *next;
	TranslationTableHeader *table;
	int tableListLength;
	char tableList[1];
} TranslationTableChainEntry;

static TranslationTableChainEntry *translationTableChain = NULL;

typedef struct DisplayTableChainEntry {
	struct DisplayTableChainEntry *next;
	DisplayTableHeader *table;
	int tableListLength;
	char tableList[1];
} DisplayTableChainEntry;

static DisplayTableChainEntry *displayTableChain = NULL;

/* predefined character classes */
static const char *characterClassNames[] = {
	"space",
	"letter",
	"digit",
	"punctuation",
	"uppercase",
	"lowercase",
	"math",
	"sign",
	"litdigit",
	NULL,
};

// names that may not be used for custom attributes
static const char *reservedAttributeNames[] = {
	"numericnocontchars",
	"numericnocontchar",
	"numericnocont",
	"midendnumericmodechars",
	"midendnumericmodechar",
	"midendnumericmode",
	"numericmodechars",
	"numericmodechar",
	"numericmode",
	"capsmodechars",
	"capsmodechar",
	"capsmode",
	"emphmodechars",
	"emphmodechar",
	"emphmode",
	"noemphchars",
	"noemphchar",
	"noemph",
	"seqdelimiter",
	"seqbeforechars",
	"seqbeforechar",
	"seqbefore",
	"seqafterchars",
	"seqafterchar",
	"seqafter",
	"noletsign",
	"noletsignbefore",
	"noletsignafter",
	NULL,
};

static const char *opcodeNames[CTO_None] = {
	"include",
	"locale",
	"undefined",
	"capsletter",
	"begcapsword",
	"endcapsword",
	"begcaps",
	"endcaps",
	"begcapsphrase",
	"endcapsphrase",
	"lencapsphrase",
	"modeletter",
	"begmodeword",
	"endmodeword",
	"begmode",
	"endmode",
	"begmodephrase",
	"endmodephrase",
	"lenmodephrase",
	"letsign",
	"noletsignbefore",
	"noletsign",
	"noletsignafter",
	"numsign",
	"nonumsign",
	"numericmodechars",
	"midendnumericmodechars",
	"numericnocontchars",
	"seqdelimiter",
	"seqbeforechars",
	"seqafterchars",
	"seqafterpattern",
	"seqafterexpression",
	"emphclass",
	"emphletter",
	"begemphword",
	"endemphword",
	"begemph",
	"endemph",
	"begemphphrase",
	"endemphphrase",
	"lenemphphrase",
	"capsmodechars",
	"emphmodechars",
	"noemphchars",
	"begcomp",
	"endcomp",
	"nocontractsign",
	"multind",
	"compdots",
	"comp6",
	"class",
	"after",
	"before",
	"noback",
	"nofor",
	"empmatchbefore",
	"empmatchafter",
	"swapcc",
	"swapcd",
	"swapdd",
	"space",
	"digit",
	"punctuation",
	"math",
	"sign",
	"letter",
	"uppercase",
	"lowercase",
	"grouping",
	"uplow",
	"litdigit",
	"display",
	"replace",
	"context",
	"correct",
	"pass2",
	"pass3",
	"pass4",
	"repeated",
	"repword",
	"rependword",
	"capsnocont",
	"always",
	"exactdots",
	"nocross",
	"syllable",
	"nocont",
	"compbrl",
	"literal",
	"largesign",
	"word",
	"partword",
	"joinnum",
	"joinword",
	"lowword",
	"contraction",
	"sufword",
	"prfword",
	"begword",
	"begmidword",
	"midword",
	"midendword",
	"endword",
	"prepunc",
	"postpunc",
	"begnum",
	"midnum",
	"endnum",
	"decpoint",
	"hyphen",
	// "apostrophe",
	// "initial",
	"nobreak",
	"match",
	"backmatch",
	"attribute",
	"base",
	"macro",
};

static short opcodeLengths[CTO_None] = { 0 };

static void
compileError(const FileInfo *file, const char *format, ...);

static void
free_tablefiles(char **tables);

static int
getAChar(FileInfo *file) {
	/* Read a big endian, little endian or ASCII 8 file and convert it to
	 * 16- or 32-bit unsigned integers */
	int ch1 = 0, ch2 = 0;
	widechar character;
	if (file->encoding == ascii8)
		if (file->status == 2) {
			file->status++;
			return file->checkencoding[1];
		}
	while ((ch1 = fgetc(file->in)) != EOF) {
		if (file->status < 2) file->checkencoding[file->status] = ch1;
		file->status++;
		if (file->status == 2) {
			if (file->checkencoding[0] == 0xfe && file->checkencoding[1] == 0xff)
				file->encoding = bigEndian;
			else if (file->checkencoding[0] == 0xff && file->checkencoding[1] == 0xfe)
				file->encoding = littleEndian;
			else if (file->checkencoding[0] < 128 && file->checkencoding[1] < 128) {
				file->encoding = ascii8;
				return file->checkencoding[0];
			} else {
				compileError(file,
						"encoding is neither big-endian, little-endian nor ASCII 8.");
				ch1 = EOF;
				break;
				;
			}
			continue;
		}
		switch (file->encoding) {
		case noEncoding:
			break;
		case ascii8:
			return ch1;
			break;
		case bigEndian:
			ch2 = fgetc(file->in);
			if (ch2 == EOF) break;
			character = (widechar)(ch1 << 8) | ch2;
			return (int)character;
			break;
		case littleEndian:
			ch2 = fgetc(file->in);
			if (ch2 == EOF) break;
			character = (widechar)(ch2 << 8) | ch1;
			return (int)character;
			break;
		}
		if (ch1 == EOF || ch2 == EOF) break;
	}
	return EOF;
}

int EXPORT_CALL
_lou_getALine(FileInfo *file) {
	/* Read a line of widechar's from an input file */
	int ch;
	int pch = 0;
	file->linelen = 0;
	while ((ch = getAChar(file)) != EOF) {
		if (ch == 13) continue;
		if (pch == '\\' && ch == 10) {
			file->linelen--;
			pch = ch;
			continue;
		}
		if (ch == 10 || file->linelen >= MAXSTRING - 1) break;
		file->line[file->linelen++] = (widechar)ch;
		pch = ch;
	}
	file->line[file->linelen] = 0;
	file->linepos = 0;
	if (ch == EOF && !file->linelen) return 0;
	file->lineNumber++;
	return 1;
}

static inline int
atEndOfLine(const FileInfo *file) {
	return file->linepos >= file->linelen;
}

static inline int
atTokenDelimiter(const FileInfo *file) {
	return file->line[file->linepos] <= 32;
}

static int
getToken(FileInfo *file, CharsString *result, const char *description) {
	/* Find the next string of contiguous non-whitespace characters. If this
	 * is the last token on the line, return 2 instead of 1. */
	while (!atEndOfLine(file) && atTokenDelimiter(file)) file->linepos++;
	result->length = 0;
	while (!atEndOfLine(file) && !atTokenDelimiter(file)) {
		int maxlen = MAXSTRING;
		if (result->length >= maxlen) {
			compileError(file, "more than %d characters (bytes)", maxlen);
			return 0;
		} else
			result->chars[result->length++] = file->line[file->linepos++];
	}
	if (!result->length) {
		/* Not enough tokens */
		if (description) compileError(file, "%s not specified.", description);
		return 0;
	}
	result->chars[result->length] = 0;
	while (!atEndOfLine(file) && atTokenDelimiter(file)) file->linepos++;
	return 1;
}

static void
compileError(const FileInfo *file, const char *format, ...) {
#ifndef __SYMBIAN32__
	char buffer[MAXSTRING];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(buffer, sizeof(buffer), format, arguments);
	va_end(arguments);
	if (file)
		_lou_logMessage(LOU_LOG_ERROR, "%s:%d: error: %s", file->fileName,
				file->lineNumber, buffer);
	else
		_lou_logMessage(LOU_LOG_ERROR, "error: %s", buffer);
	errorCount++;
#endif
}

static void
compileWarning(const FileInfo *file, const char *format, ...) {
#ifndef __SYMBIAN32__
	char buffer[MAXSTRING];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(buffer, sizeof(buffer), format, arguments);
	va_end(arguments);
	if (file)
		_lou_logMessage(LOU_LOG_WARN, "%s:%d: warning: %s", file->fileName,
				file->lineNumber, buffer);
	else
		_lou_logMessage(LOU_LOG_WARN, "warning: %s", buffer);
	warningCount++;
#endif
}

static int
allocateSpaceInTranslationTable(const FileInfo *file, TranslationTableOffset *offset,
		int size, TranslationTableHeader **table) {
	/* allocate memory for table and expand previously allocated memory if necessary */
	int spaceNeeded = ((size + OFFSETSIZE - 1) / OFFSETSIZE) * OFFSETSIZE;
	TranslationTableOffset newTableSize = (*table)->bytesUsed + spaceNeeded;
	TranslationTableOffset tableSize = (*table)->tableSize;
	if (newTableSize > tableSize) {
		TranslationTableHeader *newTable;
		newTableSize += (newTableSize / OFFSETSIZE);
		newTable = realloc(*table, newTableSize);
		if (!newTable) {
			compileError(file, "Not enough memory for translation table.");
			_lou_outOfMemory();
		}
		memset(((unsigned char *)newTable) + tableSize, 0, newTableSize - tableSize);
		/* update references to the old table */
		{
			TranslationTableChainEntry *entry;
			for (entry = translationTableChain; entry != NULL; entry = entry->next)
				if (entry->table == *table)
					entry->table = (TranslationTableHeader *)newTable;
		}
		newTable->tableSize = newTableSize;
		*table = newTable;
	}
	if (offset != NULL) {
		*offset = ((*table)->bytesUsed - sizeof(**table)) / OFFSETSIZE;
		(*table)->bytesUsed += spaceNeeded;
	}
	return 1;
}

static int
allocateSpaceInDisplayTable(const FileInfo *file, TranslationTableOffset *offset,
		int size, DisplayTableHeader **table) {
	/* allocate memory for table and expand previously allocated memory if necessary */
	int spaceNeeded = ((size + OFFSETSIZE - 1) / OFFSETSIZE) * OFFSETSIZE;
	TranslationTableOffset newTableSize = (*table)->bytesUsed + spaceNeeded;
	TranslationTableOffset tableSize = (*table)->tableSize;
	if (newTableSize > tableSize) {
		DisplayTableHeader *newTable;
		newTableSize += (newTableSize / OFFSETSIZE);
		newTable = realloc(*table, newTableSize);
		if (!newTable) {
			compileError(file, "Not enough memory for display table.");
			_lou_outOfMemory();
		}
		memset(((unsigned char *)newTable) + tableSize, 0, newTableSize - tableSize);
		/* update references to the old table */
		{
			DisplayTableChainEntry *entry;
			for (entry = displayTableChain; entry != NULL; entry = entry->next)
				if (entry->table == *table) entry->table = (DisplayTableHeader *)newTable;
		}
		newTable->tableSize = newTableSize;
		*table = newTable;
	}
	if (offset != NULL) {
		*offset = ((*table)->bytesUsed - sizeof(**table)) / OFFSETSIZE;
		(*table)->bytesUsed += spaceNeeded;
	}
	return 1;
}

static int
allocateTranslationTable(const FileInfo *file, TranslationTableHeader **table) {
	/* Allocate memory for the table and a guess on the number of rules */
	const TranslationTableOffset startSize = 2 * sizeof(**table);
	if (*table) return 1;
	TranslationTableOffset bytesUsed =
			sizeof(**table) + OFFSETSIZE; /* So no offset is ever zero */
	if (!(*table = malloc(startSize))) {
		compileError(file, "Not enough memory");
		if (*table != NULL) free(*table);
		*table = NULL;
		_lou_outOfMemory();
	}
	memset(*table, 0, startSize);
	(*table)->tableSize = startSize;
	(*table)->bytesUsed = bytesUsed;
	return 1;
}

static int
allocateDisplayTable(const FileInfo *file, DisplayTableHeader **table) {
	/* Allocate memory for the table and a guess on the number of rules */
	const TranslationTableOffset startSize = 2 * sizeof(**table);
	if (*table) return 1;
	TranslationTableOffset bytesUsed =
			sizeof(**table) + OFFSETSIZE; /* So no offset is ever zero */
	if (!(*table = malloc(startSize))) {
		compileError(file, "Not enough memory");
		if (*table != NULL) free(*table);
		*table = NULL;
		_lou_outOfMemory();
	}
	memset(*table, 0, startSize);
	(*table)->tableSize = startSize;
	(*table)->bytesUsed = bytesUsed;
	return 1;
}

/* Look up a character or dot pattern. Although the algorithms are almost identical,
 * different tables are needed for characters and dots because of the possibility of
 * conflicts. */

static TranslationTableCharacter *
getChar(widechar c, TranslationTableHeader *table,
		TranslationTableOffset *characterOffset) {
	const TranslationTableOffset bucket = table->characters[_lou_charHash(c)];
	TranslationTableOffset offset = bucket;
	while (offset) {
		TranslationTableCharacter *character =
				(TranslationTableCharacter *)&table->ruleArea[offset];
		if (character->value == c) {
			if (characterOffset) *characterOffset = offset;
			return character;
		}
		offset = character->next;
	}
	return NULL;
}

static TranslationTableCharacter *
getDots(widechar d, TranslationTableHeader *table) {
	const TranslationTableOffset bucket = table->dots[_lou_charHash(d)];
	TranslationTableOffset offset = bucket;
	while (offset) {
		TranslationTableCharacter *character =
				(TranslationTableCharacter *)&table->ruleArea[offset];
		if (character->value == d) return character;
		offset = character->next;
	}
	return NULL;
}

static TranslationTableCharacter *
putChar(const FileInfo *file, widechar c, TranslationTableHeader **table,
		TranslationTableOffset *characterOffset) {
	/* See if a character is in the appropriate table. If not, insert it. In either case,
	 * return a pointer to it. */
	TranslationTableCharacter *character;
	TranslationTableOffset offset;
	if ((character = getChar(c, *table, characterOffset))) return character;
	if (!allocateSpaceInTranslationTable(file, &offset, sizeof(*character), table))
		return NULL;
	character = (TranslationTableCharacter *)&(*table)->ruleArea[offset];
	memset(character, 0, sizeof(*character));
	character->sourceFile = file->sourceFile;
	character->sourceLine = file->lineNumber;
	character->value = c;
	const unsigned long int charHash = _lou_charHash(c);
	const TranslationTableOffset bucket = (*table)->characters[charHash];
	if (!bucket)
		(*table)->characters[charHash] = offset;
	else {
		TranslationTableCharacter *oldchar =
				(TranslationTableCharacter *)&(*table)->ruleArea[bucket];
		while (oldchar->next)
			oldchar = (TranslationTableCharacter *)&(*table)->ruleArea[oldchar->next];
		oldchar->next = offset;
	}
	if (characterOffset) *characterOffset = offset;
	return character;
}

static TranslationTableCharacter *
putDots(const FileInfo *file, widechar d, TranslationTableHeader **table) {
	/* See if a dot pattern is in the appropriate table. If not, insert it. In either
	 * case, return a pointer to it. */
	TranslationTableCharacter *character;
	TranslationTableOffset offset;
	if ((character = getDots(d, *table))) return character;
	if (!allocateSpaceInTranslationTable(file, &offset, sizeof(*character), table))
		return NULL;
	character = (TranslationTableCharacter *)&(*table)->ruleArea[offset];
	memset(character, 0, sizeof(*character));
	character->sourceFile = file->sourceFile;
	character->sourceLine = file->lineNumber;
	character->value = d;
	const unsigned long int charHash = _lou_charHash(d);
	const TranslationTableOffset bucket = (*table)->dots[charHash];
	if (!bucket)
		(*table)->dots[charHash] = offset;
	else {
		TranslationTableCharacter *oldchar =
				(TranslationTableCharacter *)&(*table)->ruleArea[bucket];
		while (oldchar->next)
			oldchar = (TranslationTableCharacter *)&(*table)->ruleArea[oldchar->next];
		oldchar->next = offset;
	}
	return character;
}

/* Look up a character-dots mapping in a display table. */

static CharDotsMapping *
getDotsForChar(widechar c, const DisplayTableHeader *table) {
	CharDotsMapping *cdPtr;
	const TranslationTableOffset bucket = table->charToDots[_lou_charHash(c)];
	TranslationTableOffset offset = bucket;
	while (offset) {
		cdPtr = (CharDotsMapping *)&table->ruleArea[offset];
		if (cdPtr->lookFor == c) return cdPtr;
		offset = cdPtr->next;
	}
	return NULL;
}

static CharDotsMapping *
getCharForDots(widechar d, const DisplayTableHeader *table) {
	CharDotsMapping *cdPtr;
	const TranslationTableOffset bucket = table->dotsToChar[_lou_charHash(d)];
	TranslationTableOffset offset = bucket;
	while (offset) {
		cdPtr = (CharDotsMapping *)&table->ruleArea[offset];
		if (cdPtr->lookFor == d) return cdPtr;
		offset = cdPtr->next;
	}
	return NULL;
}

widechar EXPORT_CALL
_lou_getDotsForChar(widechar c, const DisplayTableHeader *table) {
	CharDotsMapping *cdPtr = getDotsForChar(c, table);
	if (cdPtr) return cdPtr->found;
	return LOU_DOTS;
}

widechar EXPORT_CALL
_lou_getCharForDots(widechar d, const DisplayTableHeader *table) {
	CharDotsMapping *cdPtr = getCharForDots(d, table);
	if (cdPtr) return cdPtr->found;
	return '\0';
}

static int
putCharDotsMapping(
		const FileInfo *file, widechar c, widechar d, DisplayTableHeader **table) {
	if (!getDotsForChar(c, *table)) {
		CharDotsMapping *cdPtr;
		TranslationTableOffset offset;
		if (!allocateSpaceInDisplayTable(file, &offset, sizeof(*cdPtr), table)) return 0;
		cdPtr = (CharDotsMapping *)&(*table)->ruleArea[offset];
		cdPtr->next = 0;
		cdPtr->lookFor = c;
		cdPtr->found = d;
		const unsigned long int charHash = _lou_charHash(c);
		const TranslationTableOffset bucket = (*table)->charToDots[charHash];
		if (!bucket)
			(*table)->charToDots[charHash] = offset;
		else {
			CharDotsMapping *oldcdPtr = (CharDotsMapping *)&(*table)->ruleArea[bucket];
			while (oldcdPtr->next)
				oldcdPtr = (CharDotsMapping *)&(*table)->ruleArea[oldcdPtr->next];
			oldcdPtr->next = offset;
		}
	}
	if (!getCharForDots(d, *table)) {
		CharDotsMapping *cdPtr;
		TranslationTableOffset offset;
		if (!allocateSpaceInDisplayTable(file, &offset, sizeof(*cdPtr), table)) return 0;
		cdPtr = (CharDotsMapping *)&(*table)->ruleArea[offset];
		cdPtr->next = 0;
		cdPtr->lookFor = d;
		cdPtr->found = c;
		const unsigned long int charHash = _lou_charHash(d);
		const TranslationTableOffset bucket = (*table)->dotsToChar[charHash];
		if (!bucket)
			(*table)->dotsToChar[charHash] = offset;
		else {
			CharDotsMapping *oldcdPtr = (CharDotsMapping *)&(*table)->ruleArea[bucket];
			while (oldcdPtr->next)
				oldcdPtr = (CharDotsMapping *)&(*table)->ruleArea[oldcdPtr->next];
			oldcdPtr->next = offset;
		}
	}
	return 1;
}

static inline const char *
getPartName(int actionPart) {
	return actionPart ? "action" : "test";
}

static int
passFindCharacters(const FileInfo *file, widechar *instructions, int end,
		widechar **characters, int *length) {
	int IC = 0;
	int lookback = 0;

	*characters = NULL;
	*length = 0;

	while (IC < end) {
		widechar instruction = instructions[IC];

		switch (instruction) {
		case pass_string:
		case pass_dots: {
			int count = instructions[IC + 1];
			IC += 2;
			if (count > lookback) {
				*characters = &instructions[IC + lookback];
				*length = count - lookback;
				return 1;
			} else {
				lookback -= count;
			}
			IC += count;
			continue;
		}

		case pass_attributes:
			IC += 7;
			if (instructions[IC - 2] == instructions[IC - 1] &&
					instructions[IC - 1] <= lookback) {
				lookback -= instructions[IC - 1];
				continue;
			}
			goto NO_CHARACTERS;

		case pass_swap:
			IC += 2;
			/* fall through */

		case pass_groupstart:
		case pass_groupend:
		case pass_groupreplace:
			IC += 3;

		NO_CHARACTERS : { return 1; }

		case pass_eq:
		case pass_lt:
		case pass_gt:
		case pass_lteq:
		case pass_gteq:
			IC += 3;
			continue;

		case pass_lookback:
			lookback += instructions[IC + 1];
			IC += 2;
			continue;

		case pass_not:
		case pass_startReplace:
		case pass_endReplace:
		case pass_first:
		case pass_last:
		case pass_copy:
		case pass_omit:
		case pass_plus:
		case pass_hyphen:
			IC += 1;
			continue;

		case pass_endTest:
			goto NO_CHARACTERS;

		default:
			compileError(file, "unhandled test suboperand: \\x%02x", instruction);
			return 0;
		}
	}
	goto NO_CHARACTERS;
}

static const char *
printSource(const FileInfo *currentFile, const char *sourceFile, int sourceLine) {
	static char scratchBuf[MAXSTRING];
	if (sourceFile) {
		if (currentFile && currentFile->sourceFile &&
				strcmp(currentFile->sourceFile, sourceFile) == 0)
			snprintf(scratchBuf, MAXSTRING, "line %d", sourceLine);
		else
			snprintf(scratchBuf, MAXSTRING, "%s:%d", sourceFile, sourceLine);
	} else
		snprintf(scratchBuf, MAXSTRING, "source unknown");
	return scratchBuf;
}

/* The following functions are called by addRule to handle various cases. */

static void
addForwardRuleWithSingleChar(const FileInfo *file, TranslationTableOffset ruleOffset,
		TranslationTableRule *rule, TranslationTableHeader **table) {
	/* direction = 0, rule->charslen = 1 */
	TranslationTableCharacter *character;
	// get the character from the table, or if the character is not defined yet, define it
	// (without adding attributes)
	if (rule->opcode >= CTO_Pass2 && rule->opcode <= CTO_Pass4) {
		character = putDots(file, rule->charsdots[0], table);
		// putDots may have moved table, so make sure rule is still valid
		rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
	} else if (rule->opcode == CTO_CompDots || rule->opcode == CTO_Comp6) {
		character = putChar(file, rule->charsdots[0], table, NULL);
		// putChar may have moved table, so make sure rule is still valid
		rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
		character->compRule = ruleOffset;
		return;
	} else {
		character = putChar(file, rule->charsdots[0], table, NULL);
		// putChar may have moved table, so make sure rule is still valid
		rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
		// if the new rule is a character definition rule, set the main definition rule of
		// this character to it (possibly overwriting previous definition rules)
		// adding the attributes to the character has already been done elsewhere
		if (rule->opcode >= CTO_Space && rule->opcode < CTO_UpLow) {
			if (character->definitionRule) {
				TranslationTableRule *prevRule =
						(TranslationTableRule *)&(*table)
								->ruleArea[character->definitionRule];
				_lou_logMessage(LOU_LOG_DEBUG,
						"%s:%d: Character already defined (%s). The new definition will "
						"take precedence.",
						file->fileName, file->lineNumber,
						printSource(file, prevRule->sourceFile, prevRule->sourceLine));
			} else if (character->basechar) {
				_lou_logMessage(LOU_LOG_DEBUG,
						"%s:%d: A base rule already exists for this character (%s). The "
						"%s rule will take precedence.",
						file->fileName, file->lineNumber,
						printSource(file, character->sourceFile, character->sourceLine),
						_lou_findOpcodeName(rule->opcode));
				character->basechar = 0;
				character->mode = 0;
			}
			character->definitionRule = ruleOffset;
		}
	}
	// add the new rule to the list of rules associated with this character
	// if the new rule is a character definition rule, it is inserted at the end of the
	// list
	// otherwise it is inserted before the first character definition rule
	TranslationTableOffset *otherRule = &character->otherRules;
	while (*otherRule) {
		TranslationTableRule *r = (TranslationTableRule *)&(*table)->ruleArea[*otherRule];
		if (r->charslen == 0) break;
		if (r->opcode >= CTO_Space && r->opcode < CTO_UpLow)
			if (!(rule->opcode >= CTO_Space && rule->opcode < CTO_UpLow)) break;
		otherRule = &r->charsnext;
	}
	rule->charsnext = *otherRule;
	*otherRule = ruleOffset;
}

static void
addForwardRuleWithMultipleChars(TranslationTableOffset ruleOffset,
		TranslationTableRule *rule, TranslationTableHeader *table) {
	/* direction = 0 rule->charslen > 1 */
	TranslationTableOffset *forRule =
			&table->forRules[_lou_stringHash(&rule->charsdots[0], 0, NULL)];
	while (*forRule) {
		TranslationTableRule *r = (TranslationTableRule *)&table->ruleArea[*forRule];
		if (rule->charslen > r->charslen) break;
		if (rule->charslen == r->charslen)
			if ((r->opcode == CTO_Always) && (rule->opcode != CTO_Always)) break;
		forRule = &r->charsnext;
	}
	rule->charsnext = *forRule;
	*forRule = ruleOffset;
}

static void
addBackwardRuleWithSingleCell(const FileInfo *file, widechar cell,
		TranslationTableOffset ruleOffset, TranslationTableRule *rule,
		TranslationTableHeader **table) {
	/* direction = 1, rule->dotslen = 1 */
	TranslationTableCharacter *dots;
	if (rule->opcode == CTO_SwapCc || rule->opcode == CTO_Repeated)
		return; /* too ambiguous */
	// get the cell from the table, or if the cell is not defined yet, define it (without
	// adding attributes)
	dots = putDots(file, cell, table);
	// putDots may have moved table, so make sure rule is still valid
	rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
	if (rule->opcode >= CTO_Space && rule->opcode < CTO_UpLow)
		dots->definitionRule = ruleOffset;
	TranslationTableOffset *otherRule = &dots->otherRules;
	while (*otherRule) {
		TranslationTableRule *r = (TranslationTableRule *)&(*table)->ruleArea[*otherRule];
		if (rule->charslen > r->charslen || r->dotslen == 0) break;
		if (r->opcode >= CTO_Space && r->opcode < CTO_UpLow)
			if (!(rule->opcode >= CTO_Space && rule->opcode < CTO_UpLow)) break;
		otherRule = &r->dotsnext;
	}
	rule->dotsnext = *otherRule;
	*otherRule = ruleOffset;
}

static void
addBackwardRuleWithMultipleCells(widechar *cells, int dotslen,
		TranslationTableOffset ruleOffset, TranslationTableRule *rule,
		TranslationTableHeader *table) {
	/* direction = 1, dotslen > 1 */
	TranslationTableOffset *backRule = &table->backRules[_lou_stringHash(cells, 0, NULL)];
	if (rule->opcode == CTO_SwapCc) return;
	int ruleLength = dotslen + rule->charslen;
	while (*backRule) {
		TranslationTableRule *r = (TranslationTableRule *)&table->ruleArea[*backRule];
		int rLength = r->dotslen + r->charslen;
		if (ruleLength > rLength) break;
		if (rLength == ruleLength)
			if ((r->opcode == CTO_Always) && (rule->opcode != CTO_Always)) break;
		backRule = &r->dotsnext;
	}
	rule->dotsnext = *backRule;
	*backRule = ruleOffset;
}

static int
addForwardPassRule(TranslationTableOffset ruleOffset, TranslationTableRule *rule,
		TranslationTableHeader *table) {
	TranslationTableOffset *forPassRule;
	switch (rule->opcode) {
	case CTO_Correct:
		forPassRule = &table->forPassRules[0];
		break;
	case CTO_Context:
		forPassRule = &table->forPassRules[1];
		break;
	case CTO_Pass2:
		forPassRule = &table->forPassRules[2];
		break;
	case CTO_Pass3:
		forPassRule = &table->forPassRules[3];
		break;
	case CTO_Pass4:
		forPassRule = &table->forPassRules[4];
		break;
	default:
		return 0;
	}
	while (*forPassRule) {
		TranslationTableRule *r = (TranslationTableRule *)&table->ruleArea[*forPassRule];
		if (rule->charslen > r->charslen) break;
		forPassRule = &r->charsnext;
	}
	rule->charsnext = *forPassRule;
	*forPassRule = ruleOffset;
	return 1;
}

static int
addBackwardPassRule(TranslationTableOffset ruleOffset, TranslationTableRule *rule,
		TranslationTableHeader *table) {
	TranslationTableOffset *backPassRule;
	switch (rule->opcode) {
	case CTO_Correct:
		backPassRule = &table->backPassRules[0];
		break;
	case CTO_Context:
		backPassRule = &table->backPassRules[1];
		break;
	case CTO_Pass2:
		backPassRule = &table->backPassRules[2];
		break;
	case CTO_Pass3:
		backPassRule = &table->backPassRules[3];
		break;
	case CTO_Pass4:
		backPassRule = &table->backPassRules[4];
		break;
	default:
		return 0;
	}
	while (*backPassRule) {
		TranslationTableRule *r = (TranslationTableRule *)&table->ruleArea[*backPassRule];
		if (rule->charslen > r->charslen) break;
		backPassRule = &r->dotsnext;
	}
	rule->dotsnext = *backPassRule;
	*backPassRule = ruleOffset;
	return 1;
}

static int
addRule(const FileInfo *file, TranslationTableOpcode opcode, CharsString *ruleChars,
		CharsString *ruleDots, TranslationTableCharacterAttributes after,
		TranslationTableCharacterAttributes before, TranslationTableOffset *ruleOffset,
		TranslationTableRule **rule, int noback, int nofor,
		TranslationTableHeader **table) {
	/* Add a rule to the table, using the hash function to find the start of
	 * chains and chaining both the chars and dots strings */
	TranslationTableOffset offset;
	int ruleSize = sizeof(TranslationTableRule) - (DEFAULTRULESIZE * CHARSIZE);
	if (ruleChars) ruleSize += CHARSIZE * ruleChars->length;
	if (ruleDots) ruleSize += CHARSIZE * ruleDots->length;
	if (!allocateSpaceInTranslationTable(file, &offset, ruleSize, table)) return 0;
	TranslationTableRule *r = (TranslationTableRule *)&(*table)->ruleArea[offset];
	if (rule) *rule = r;
	if (ruleOffset) *ruleOffset = offset;
	r->sourceFile = file->sourceFile;
	r->sourceLine = file->lineNumber;
	r->opcode = opcode;
	r->after = after;
	r->before = before;
	r->nocross = 0;
	if (ruleChars)
		memcpy(&r->charsdots[0], &ruleChars->chars[0],
				CHARSIZE * (r->charslen = ruleChars->length));
	else
		r->charslen = 0;
	if (ruleDots)
		memcpy(&r->charsdots[r->charslen], &ruleDots->chars[0],
				CHARSIZE * (r->dotslen = ruleDots->length));
	else
		r->dotslen = 0;

	/* link new rule into table. */
	if (opcode == CTO_SwapCc || opcode == CTO_SwapCd || opcode == CTO_SwapDd) return 1;
	if (opcode >= CTO_Context && opcode <= CTO_Pass4)
		if (!(opcode == CTO_Context && r->charslen > 0)) {
			if (!nofor)
				if (!addForwardPassRule(offset, r, *table)) return 0;
			if (!noback)
				if (!addBackwardPassRule(offset, r, *table)) return 0;
			return 1;
		}
	if (!nofor) {
		if (r->charslen == 1) {
			addForwardRuleWithSingleChar(file, offset, r, table);
			// addForwardRuleWithSingleChar may have moved table, so make sure rule is
			// still valid
			r = (TranslationTableRule *)&(*table)->ruleArea[offset];
			if (rule) *rule = r;
		} else if (r->charslen > 1)
			addForwardRuleWithMultipleChars(offset, r, *table);
	}
	if (!noback) {
		widechar *cells;
		int dotslen;

		if (r->opcode == CTO_Context) {
			cells = &r->charsdots[0];
			dotslen = r->charslen;
		} else {
			cells = &r->charsdots[r->charslen];
			dotslen = r->dotslen;
		}
		if (dotslen == 1) {
			addBackwardRuleWithSingleCell(file, *cells, offset, r, table);
			// addBackwardRuleWithSingleCell may have moved table, so make sure rule is
			// still valid
			r = (TranslationTableRule *)&(*table)->ruleArea[offset];
			if (rule) *rule = r;
		} else if (dotslen > 1)
			addBackwardRuleWithMultipleCells(cells, dotslen, offset, r, *table);
	}
	return 1;
}

static const CharacterClass *
findCharacterClass(const CharsString *name, const TranslationTableHeader *table) {
	/* Find a character class, whether predefined or user-defined */
	const CharacterClass *class = table->characterClasses;
	while (class) {
		if ((name->length == class->length) &&
				(memcmp(&name->chars[0], class->name, CHARSIZE * name->length) == 0))
			return class;
		class = class->next;
	}
	return NULL;
}

static TranslationTableCharacterAttributes
getNextNumberedAttribute(TranslationTableHeader *table) {
	/* Get the next attribute value for numbered attributes, or 0 if there is no more
	 * space in the table. */
	TranslationTableCharacterAttributes next = table->nextNumberedCharacterClassAttribute;
	if (next > CTC_UserDefined8) return 0;
	table->nextNumberedCharacterClassAttribute <<= 1;
	return next;
}

static TranslationTableCharacterAttributes
getNextAttribute(TranslationTableHeader *table) {
	/* Get the next attribute value, or 0 if there is no more space in the table. */
	TranslationTableCharacterAttributes next = table->nextCharacterClassAttribute;
	if (next) {
		if (next == CTC_LitDigit)
			table->nextCharacterClassAttribute = CTC_UserDefined9;
		else
			table->nextCharacterClassAttribute <<= 1;
		return next;
	} else
		return getNextNumberedAttribute(table);
}

static CharacterClass *
addCharacterClass(const FileInfo *file, const widechar *name, int length,
		TranslationTableHeader *table, int validate) {
	/* Define a character class, Whether predefined or user-defined */
	if (validate) {
		for (int i = 0; i < length; i++) {
			if (!((name[i] >= 'a' && name[i] <= 'z') ||
						(name[i] >= 'A' && name[i] <= 'Z'))) {
				// don't abort because in some cases (before/after rules)
				// this will work fine, but it will not work in multipass
				// expressions
				compileWarning(file,
						"Invalid attribute name: must be a digit between "
						"0 and 7 or a word containing only letters");
			}
		}
		// check that name is not reserved
		int k = 0;
		while (reservedAttributeNames[k]) {
			if (strlen(reservedAttributeNames[k]) == length) {
				int i;
				for (i = 0; i < length; i++)
					if (reservedAttributeNames[k][i] != name[i]) break;
				if (i == length) {
					compileError(file, "Attribute name is reserved: %s",
							reservedAttributeNames[k]);
					return NULL;
				}
			}
			k++;
		}
	}
	CharacterClass **classes = &table->characterClasses;
	TranslationTableCharacterAttributes attribute = getNextAttribute(table);
	CharacterClass *class;
	if (attribute) {
		if (!(class = malloc(sizeof(*class) + CHARSIZE * (length - 1))))
			_lou_outOfMemory();
		else {
			memset(class, 0, sizeof(*class));
			memcpy(class->name, name, CHARSIZE * (class->length = length));
			class->attribute = attribute;
			class->next = *classes;
			*classes = class;
			return class;
		}
	}
	compileError(file, "character class table overflow.");
	return NULL;
}

static void
deallocateCharacterClasses(TranslationTableHeader *table) {
	CharacterClass **classes = &table->characterClasses;
	while (*classes) {
		CharacterClass *class = *classes;
		*classes = (*classes)->next;
		if (class) free(class);
	}
}

static int
allocateCharacterClasses(TranslationTableHeader *table) {
	/* Allocate memory for predefined character classes */
	int k = 0;
	table->characterClasses = NULL;
	table->nextCharacterClassAttribute = 1;	 // CTC_Space
	table->nextNumberedCharacterClassAttribute = CTC_UserDefined1;
	while (characterClassNames[k]) {
		widechar wname[MAXSTRING];
		int length = (int)strlen(characterClassNames[k]);
		int kk;
		for (kk = 0; kk < length; kk++) wname[kk] = (widechar)characterClassNames[k][kk];
		if (!addCharacterClass(NULL, wname, length, table, 0)) {
			deallocateCharacterClasses(table);
			return 0;
		}
		k++;
	}
	return 1;
}

static TranslationTableOpcode
getOpcode(const FileInfo *file, const CharsString *token) {
	static TranslationTableOpcode lastOpcode = 0;
	TranslationTableOpcode opcode = lastOpcode;

	do {
		if (token->length == opcodeLengths[opcode])
			if (eqasc2uni((unsigned char *)opcodeNames[opcode], &token->chars[0],
						token->length)) {
				lastOpcode = opcode;
				return opcode;
			}
		opcode++;
		if (opcode >= CTO_None) opcode = 0;
	} while (opcode != lastOpcode);
	return CTO_None;
}

TranslationTableOpcode EXPORT_CALL
_lou_findOpcodeNumber(const char *toFind) {
	/* Used by tools such as lou_debug */
	static TranslationTableOpcode lastOpcode = 0;
	TranslationTableOpcode opcode = lastOpcode;
	int length = (int)strlen(toFind);
	do {
		if (length == opcodeLengths[opcode] &&
				strcasecmp(toFind, opcodeNames[opcode]) == 0) {
			lastOpcode = opcode;
			return opcode;
		}
		opcode++;
		if (opcode >= CTO_None) opcode = 0;
	} while (opcode != lastOpcode);
	return CTO_None;
}

const char *EXPORT_CALL
_lou_findOpcodeName(TranslationTableOpcode opcode) {
	static char scratchBuf[MAXSTRING];
	/* Used by tools such as lou_debug */
	if (opcode < 0 || opcode >= CTO_None) {
		sprintf(scratchBuf, "%u", opcode);
		return scratchBuf;
	}
	return opcodeNames[opcode];
}

static widechar
hexValue(const FileInfo *file, const widechar *digits, int length) {
	int k;
	unsigned int binaryValue = 0;
	for (k = 0; k < length; k++) {
		unsigned int hexDigit = 0;
		if (digits[k] >= '0' && digits[k] <= '9')
			hexDigit = digits[k] - '0';
		else if (digits[k] >= 'a' && digits[k] <= 'f')
			hexDigit = digits[k] - 'a' + 10;
		else if (digits[k] >= 'A' && digits[k] <= 'F')
			hexDigit = digits[k] - 'A' + 10;
		else {
			compileError(file, "invalid %d-digit hexadecimal number", length);
			return (widechar)0xffffffff;
		}
		binaryValue |= hexDigit << (4 * (length - 1 - k));
	}
	return (widechar)binaryValue;
}

#define MAXBYTES 7
static const unsigned int first0Bit[MAXBYTES] = { 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC,
	0XFE };

static int
parseChars(const FileInfo *file, CharsString *result, CharsString *token) {
	int in = 0;
	int out = 0;
	int lastOutSize = 0;
	int lastIn;
	unsigned int ch = 0;
	int numBytes = 0;
	unsigned int utf32 = 0;
	int k;
	while (in < token->length) {
		ch = token->chars[in++] & 0xff;
		if (ch < 128) {
			if (ch == '\\') { /* escape sequence */
				switch (ch = token->chars[in]) {
				case '\\':
					break;
				case 'e':
					ch = 0x1b;
					break;
				case 'f':
					ch = 12;
					break;
				case 'n':
					ch = 10;
					break;
				case 'r':
					ch = 13;
					break;
				case 's':
					ch = ' ';
					break;
				case 't':
					ch = 9;
					break;
				case 'v':
					ch = 11;
					break;
				case 'w':
					ch = LOU_ENDSEGMENT;
					break;
				case 34:
					ch = QUOTESUB;
					break;
				case 'X':
					compileWarning(file, "\\Xhhhh (with a capital 'X') is deprecated.");
				case 'x':
					if (token->length - in > 4) {
						ch = hexValue(file, &token->chars[in + 1], 4);
						in += 4;
					}
					break;
				case 'Y':
					compileWarning(file, "\\Yhhhhh (with a capital 'Y') is deprecated.");
				case 'y':
					if (CHARSIZE == 2) {
					not32:
						compileError(file,
								"liblouis has not been compiled for 32-bit Unicode");
						break;
					}
					if (token->length - in > 5) {
						ch = hexValue(file, &token->chars[in + 1], 5);
						in += 5;
					}
					break;
				case 'Z':
					compileWarning(
							file, "\\Zhhhhhhhh (with a capital 'Z') is deprecated.");
				case 'z':
					if (CHARSIZE == 2) goto not32;
					if (token->length - in > 8) {
						ch = hexValue(file, &token->chars[in + 1], 8);
						in += 8;
					}
					break;
				default:
					compileError(file, "invalid escape sequence '\\%c'", ch);
					break;
				}
				in++;
			}
			if (out >= MAXSTRING - 1) {
				compileError(file, "Token too long");
				result->length = MAXSTRING - 1;
				return 1;
			}
			result->chars[out++] = (widechar)ch;
			continue;
		}
		lastOutSize = out;
		lastIn = in;
		for (numBytes = MAXBYTES - 1; numBytes > 0; numBytes--)
			if (ch >= first0Bit[numBytes]) break;
		utf32 = ch & (0XFF - first0Bit[numBytes]);
		for (k = 0; k < numBytes; k++) {
			if (in >= MAXSTRING - 1 || in >= token->length) break;
			if (out >= MAXSTRING - 1) {
				compileError(file, "Token too long");
				result->length = lastOutSize;
				return 1;
			}
			if (token->chars[in] < 128 || (token->chars[in] & 0x0040)) {
				compileWarning(file, "invalid UTF-8. Assuming Latin-1.");
				result->chars[out++] = token->chars[lastIn];
				in = lastIn + 1;
				continue;
			}
			utf32 = (utf32 << 6) + (token->chars[in++] & 0x3f);
		}
		if (out >= MAXSTRING - 1) {
			compileError(file, "Token too long");
			result->length = lastOutSize;
			return 1;
		}
		if (CHARSIZE == 2 && utf32 > 0xffff) utf32 = 0xffff;
		result->chars[out++] = (widechar)utf32;
	}
	result->length = out;
	return 1;
}

int EXPORT_CALL
_lou_extParseChars(const char *inString, widechar *outString) {
	/* Parse external character strings */
	CharsString wideIn;
	CharsString result;
	int k;
	for (k = 0; inString[k] && k < MAXSTRING - 1; k++) wideIn.chars[k] = inString[k];
	wideIn.chars[k] = 0;
	wideIn.length = k;
	parseChars(NULL, &result, &wideIn);
	if (errorCount) {
		errorCount = 0;
		return 0;
	}
	for (k = 0; k < result.length; k++) outString[k] = result.chars[k];
	return result.length;
}

static int
parseDots(const FileInfo *file, CharsString *cells, const CharsString *token) {
	/* get dot patterns */
	widechar cell = 0; /* assembly place for dots */
	int cellCount = 0;
	int index;
	int start = 0;

	for (index = 0; index < token->length; index++) {
		int started = index != start;
		widechar character = token->chars[index];
		switch (character) { /* or dots to make up Braille cell */
			{
				int dot;
			case '1':
				dot = LOU_DOT_1;
				goto haveDot;
			case '2':
				dot = LOU_DOT_2;
				goto haveDot;
			case '3':
				dot = LOU_DOT_3;
				goto haveDot;
			case '4':
				dot = LOU_DOT_4;
				goto haveDot;
			case '5':
				dot = LOU_DOT_5;
				goto haveDot;
			case '6':
				dot = LOU_DOT_6;
				goto haveDot;
			case '7':
				dot = LOU_DOT_7;
				goto haveDot;
			case '8':
				dot = LOU_DOT_8;
				goto haveDot;
			case '9':
				dot = LOU_DOT_9;
				goto haveDot;
			case 'a':
			case 'A':
				dot = LOU_DOT_10;
				goto haveDot;
			case 'b':
			case 'B':
				dot = LOU_DOT_11;
				goto haveDot;
			case 'c':
			case 'C':
				dot = LOU_DOT_12;
				goto haveDot;
			case 'd':
			case 'D':
				dot = LOU_DOT_13;
				goto haveDot;
			case 'e':
			case 'E':
				dot = LOU_DOT_14;
				goto haveDot;
			case 'f':
			case 'F':
				dot = LOU_DOT_15;
			haveDot:
				if (started && !cell) goto invalid;
				if (cell & dot) {
					compileError(file, "dot specified more than once.");
					return 0;
				}
				cell |= dot;
				break;
			}
		case '0': /* blank */
			if (started) goto invalid;
			break;
		case '-': /* got all dots for this cell */
			if (!started) {
				compileError(file, "missing cell specification.");
				return 0;
			}
			cells->chars[cellCount++] = cell | LOU_DOTS;
			cell = 0;
			start = index + 1;
			break;
		default:
		invalid:
			compileError(
					file, "invalid dot number %s.", _lou_showString(&character, 1, 0));
			return 0;
		}
	}
	if (index == start) {
		compileError(file, "missing cell specification.");
		return 0;
	}
	cells->chars[cellCount++] = cell | LOU_DOTS; /* last cell */
	cells->length = cellCount;
	return 1;
}

int EXPORT_CALL
_lou_extParseDots(const char *inString, widechar *outString) {
	/* Parse external dot patterns */
	CharsString wideIn;
	CharsString result;
	int k;
	for (k = 0; inString[k] && k < MAXSTRING - 1; k++) wideIn.chars[k] = inString[k];
	wideIn.chars[k] = 0;
	wideIn.length = k;
	parseDots(NULL, &result, &wideIn);
	if (errorCount) {
		errorCount = 0;
		return 0;
	}
	for (k = 0; k < result.length; k++) outString[k] = result.chars[k];
	outString[k] = 0;
	return result.length;
}

static int
getCharacters(FileInfo *file, CharsString *characters) {
	/* Get ruleChars string */
	CharsString token;
	if (!getToken(file, &token, "characters")) return 0;
	return parseChars(file, characters, &token);
}

static int
getRuleCharsText(FileInfo *file, CharsString *ruleChars) {
	CharsString token;
	if (!getToken(file, &token, "Characters operand")) return 0;
	return parseChars(file, ruleChars, &token);
}

static int
getRuleDotsText(FileInfo *file, CharsString *ruleDots) {
	CharsString token;
	if (!getToken(file, &token, "characters")) return 0;
	return parseChars(file, ruleDots, &token);
}

static int
getRuleDotsPattern(FileInfo *file, CharsString *ruleDots) {
	/* Interpret the dets operand */
	CharsString token;
	if (!getToken(file, &token, "Dots operand")) return 0;
	if (token.length == 1 && token.chars[0] == '=') {
		ruleDots->length = 0;
		return 1;
	} else
		return parseDots(file, ruleDots, &token);
}

static int
includeFile(const FileInfo *file, CharsString *includedFile,
		TranslationTableHeader **table, DisplayTableHeader **displayTable);

static TranslationTableOffset
findRuleName(const CharsString *name, const TranslationTableHeader *table) {
	const RuleName *ruleName = table->ruleNames;
	while (ruleName) {
		if ((name->length == ruleName->length) &&
				(memcmp(&name->chars[0], ruleName->name, CHARSIZE * name->length) == 0))
			return ruleName->ruleOffset;
		ruleName = ruleName->next;
	}
	return 0;
}

static int
addRuleName(const FileInfo *file, CharsString *name, TranslationTableOffset ruleOffset,
		TranslationTableHeader *table) {
	int k;
	RuleName *ruleName;
	if (!(ruleName = malloc(sizeof(*ruleName) + CHARSIZE * (name->length - 1)))) {
		compileError(file, "not enough memory");
		_lou_outOfMemory();
	}
	memset(ruleName, 0, sizeof(*ruleName));
	// a name is a sequence of characters in the ranges 'a'..'z' and 'A'..'Z'
	for (k = 0; k < name->length; k++) {
		widechar c = name->chars[k];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
			ruleName->name[k] = c;
		else {
			compileError(file, "a name may contain only letters");
			free(ruleName);
			return 0;
		}
	}
	ruleName->length = name->length;
	ruleName->ruleOffset = ruleOffset;
	ruleName->next = table->ruleNames;
	table->ruleNames = ruleName;
	return 1;
}

static void
deallocateRuleNames(TranslationTableHeader *table) {
	RuleName **ruleName = &table->ruleNames;
	while (*ruleName) {
		RuleName *rn = *ruleName;
		*ruleName = rn->next;
		free(rn);
	}
}

static int
compileSwapDots(const FileInfo *file, CharsString *source, CharsString *dest) {
	int k = 0;
	int kk = 0;
	CharsString dotsSource;
	CharsString dotsDest;
	dest->length = 0;
	dotsSource.length = 0;
	while (k <= source->length) {
		if (source->chars[k] != ',' && k != source->length)
			dotsSource.chars[dotsSource.length++] = source->chars[k];
		else {
			if (!parseDots(file, &dotsDest, &dotsSource)) return 0;
			dest->chars[dest->length++] = dotsDest.length + 1;
			for (kk = 0; kk < dotsDest.length; kk++)
				dest->chars[dest->length++] = dotsDest.chars[kk];
			dotsSource.length = 0;
		}
		k++;
	}
	return 1;
}

static int
compileSwap(FileInfo *file, TranslationTableOpcode opcode, int noback, int nofor,
		TranslationTableHeader **table) {
	CharsString ruleChars;
	CharsString ruleDots;
	CharsString name;
	CharsString matches;
	CharsString replacements;
	TranslationTableOffset ruleOffset;
	if (!getToken(file, &name, "name operand")) return 0;
	if (!getToken(file, &matches, "matches operand")) return 0;
	if (!getToken(file, &replacements, "replacements operand")) return 0;
	if (opcode == CTO_SwapCc || opcode == CTO_SwapCd) {
		if (!parseChars(file, &ruleChars, &matches)) return 0;
	} else {
		if (!compileSwapDots(file, &matches, &ruleChars)) return 0;
	}
	if (opcode == CTO_SwapCc) {
		if (!parseChars(file, &ruleDots, &replacements)) return 0;
	} else {
		if (!compileSwapDots(file, &replacements, &ruleDots)) return 0;
	}
	if (!addRule(file, opcode, &ruleChars, &ruleDots, 0, 0, &ruleOffset, NULL, noback,
				nofor, table))
		return 0;
	if (!addRuleName(file, &name, ruleOffset, *table)) return 0;
	return 1;
}

static int
getNumber(widechar *string, widechar *number) {
	/* Convert a string of wide character digits to an integer */
	int k = 0;
	*number = 0;
	while (string[k] >= '0' && string[k] <= '9')
		*number = 10 * *number + (string[k++] - '0');
	return k;
}

/* Start of multipass compiler */

static int
passGetAttributes(CharsString *passLine, int *passLinepos,
		TranslationTableCharacterAttributes *attributes, const FileInfo *file) {
	int more = 1;
	*attributes = 0;
	while (more) {
		switch (passLine->chars[*passLinepos]) {
		case pass_any:
			*attributes = 0xffffffff;
			break;
		case pass_digit:
			*attributes |= CTC_Digit;
			break;
		case pass_litDigit:
			*attributes |= CTC_LitDigit;
			break;
		case pass_letter:
			*attributes |= CTC_Letter;
			break;
		case pass_math:
			*attributes |= CTC_Math;
			break;
		case pass_punctuation:
			*attributes |= CTC_Punctuation;
			break;
		case pass_sign:
			*attributes |= CTC_Sign;
			break;
		case pass_space:
			*attributes |= CTC_Space;
			break;
		case pass_uppercase:
			*attributes |= CTC_UpperCase;
			break;
		case pass_lowercase:
			*attributes |= CTC_LowerCase;
			break;
		case pass_class1:
			*attributes |= CTC_UserDefined9;
			break;
		case pass_class2:
			*attributes |= CTC_UserDefined10;
			break;
		case pass_class3:
			*attributes |= CTC_UserDefined11;
			break;
		case pass_class4:
			*attributes |= CTC_UserDefined12;
			break;
		default:
			more = 0;
			break;
		}
		if (more) (*passLinepos)++;
	}
	if (!*attributes) {
		compileError(file, "missing attribute");
		(*passLinepos)--;
		return 0;
	}
	return 1;
}

static int
passGetDots(CharsString *passLine, int *passLinepos, CharsString *dots,
		const FileInfo *file) {
	CharsString collectDots;
	collectDots.length = 0;
	while (*passLinepos < passLine->length &&
			(passLine->chars[*passLinepos] == '-' ||
					(passLine->chars[*passLinepos] >= '0' &&
							passLine->chars[*passLinepos] <= '9') ||
					((passLine->chars[*passLinepos] | 32) >= 'a' &&
							(passLine->chars[*passLinepos] | 32) <= 'f')))
		collectDots.chars[collectDots.length++] = passLine->chars[(*passLinepos)++];
	if (!parseDots(file, dots, &collectDots)) return 0;
	return 1;
}

static int
passGetString(CharsString *passLine, int *passLinepos, CharsString *string,
		const FileInfo *file) {
	string->length = 0;
	while (1) {
		if ((*passLinepos >= passLine->length) || !passLine->chars[*passLinepos]) {
			compileError(file, "unterminated string");
			return 0;
		}
		if (passLine->chars[*passLinepos] == 34) break;
		if (passLine->chars[*passLinepos] == QUOTESUB)
			string->chars[string->length++] = 34;
		else
			string->chars[string->length++] = passLine->chars[*passLinepos];
		(*passLinepos)++;
	}
	string->chars[string->length] = 0;
	(*passLinepos)++;
	return 1;
}

static int
passGetNumber(CharsString *passLine, int *passLinepos, widechar *number) {
	/* Convert a string of wide character digits to an integer */
	*number = 0;
	while ((*passLinepos < passLine->length) && (passLine->chars[*passLinepos] >= '0') &&
			(passLine->chars[*passLinepos] <= '9'))
		*number = 10 * (*number) + (passLine->chars[(*passLinepos)++] - '0');
	return 1;
}

static int
passGetVariableNumber(
		const FileInfo *file, CharsString *passLine, int *passLinepos, widechar *number) {
	if (!passGetNumber(passLine, passLinepos, number)) {
		compileError(file, "missing variable number");
		return 0;
	}
	if ((*number >= 0) && (*number < NUMVAR)) return 1;
	compileError(file, "variable number out of range");
	return 0;
}

static int
passGetName(CharsString *passLine, int *passLinepos, CharsString *name) {
	name->length = 0;
	// a name is a sequence of characters in the ranges 'a'..'z' and 'A'..'Z'
	do {
		widechar c = passLine->chars[*passLinepos];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
			name->chars[name->length++] = c;
			(*passLinepos)++;
		} else {
			break;
		}
	} while (*passLinepos < passLine->length);
	return 1;
}

static inline int
wantsString(TranslationTableOpcode opcode, int actionPart, int nofor) {
	if (opcode == CTO_Correct) return 1;
	if (opcode != CTO_Context) return 0;
	return !nofor == !actionPart;
}

static int
verifyStringOrDots(const FileInfo *file, TranslationTableOpcode opcode, int isString,
		int actionPart, int nofor) {
	if (!wantsString(opcode, actionPart, nofor) == !isString) return 1;

	compileError(file, "%s are not allowed in the %s part of a %s translation %s rule.",
			isString ? "strings" : "dots", getPartName(actionPart),
			nofor ? "backward" : "forward", _lou_findOpcodeName(opcode));

	return 0;
}

static int
appendInstructionChar(
		const FileInfo *file, widechar *passInstructions, int *passIC, widechar ch) {
	if (*passIC >= MAXSTRING) {
		compileError(file, "multipass operand too long");
		return 0;
	}
	passInstructions[(*passIC)++] = ch;
	return 1;
}

static int
compilePassOpcode(const FileInfo *file, TranslationTableOpcode opcode, int noback,
		int nofor, TranslationTableHeader **table) {
	static CharsString passRuleChars;
	static CharsString passRuleDots;
	/* Compile the operands of a pass opcode */
	widechar passSubOp;
	const CharacterClass *class;
	TranslationTableRule *rule = NULL;
	int k;
	int kk = 0;
	int endTest = 0;
	widechar *passInstructions = passRuleDots.chars;
	int passIC = 0; /* Instruction counter */
	passRuleChars.length = 0;
	CharsString passHoldString;
	widechar passHoldNumber;
	CharsString passLine;
	int passLinepos = 0;
	TranslationTableCharacterAttributes passAttributes;
	int replacing = 0;
	passHoldString.length = 0;
	for (k = file->linepos; k < file->linelen; k++)
		passHoldString.chars[passHoldString.length++] = file->line[k];
#define SEPCHAR 0x0001
	for (k = 0; k < passHoldString.length && passHoldString.chars[k] > 32; k++)
		;
	if (k < passHoldString.length)
		passHoldString.chars[k] = SEPCHAR;
	else {
		compileError(file, "Invalid multipass operands");
		return 0;
	}
	parseChars(file, &passLine, &passHoldString);
	/* Compile test part */
	for (k = 0; k < passLine.length && passLine.chars[k] != SEPCHAR; k++)
		;
	endTest = k;
	passLine.chars[endTest] = pass_endTest;
	passLinepos = 0;
	while (passLinepos <= endTest) {
		switch ((passSubOp = passLine.chars[passLinepos])) {
		case pass_lookback:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_lookback))
				return 0;
			passLinepos++;
			passGetNumber(&passLine, &passLinepos, &passHoldNumber);
			if (passHoldNumber == 0) passHoldNumber = 1;
			if (!appendInstructionChar(file, passInstructions, &passIC, passHoldNumber))
				return 0;
			break;
		case pass_not:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_not))
				return 0;
			passLinepos++;
			break;
		case pass_first:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_first))
				return 0;
			passLinepos++;
			break;
		case pass_last:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_last))
				return 0;
			passLinepos++;
			break;
		case pass_search:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_search))
				return 0;
			passLinepos++;
			break;
		case pass_string:
			if (!verifyStringOrDots(file, opcode, 1, 0, nofor)) {
				return 0;
			}
			passLinepos++;
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_string))
				return 0;
			passGetString(&passLine, &passLinepos, &passHoldString, file);
			if (passHoldString.length == 0) {
				compileError(file, "empty string in test part");
				return 0;
			}
			goto testDoCharsDots;
		case pass_dots:
			if (!verifyStringOrDots(file, opcode, 0, 0, nofor)) {
				return 0;
			}
			passLinepos++;
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_dots))
				return 0;
			passGetDots(&passLine, &passLinepos, &passHoldString, file);
			if (passHoldString.length == 0) {
				compileError(file, "expected dot pattern after @ operand in test part");
				return 0;
			}
		testDoCharsDots:
			if (passIC >= MAXSTRING) {
				compileError(
						file, "@ operand in test part of multipass operand too long");
				return 0;
			}
			if (!appendInstructionChar(
						file, passInstructions, &passIC, passHoldString.length))
				return 0;
			for (kk = 0; kk < passHoldString.length; kk++) {
				if (passIC >= MAXSTRING) {
					compileError(
							file, "@ operand in test part of multipass operand too long");
					return 0;
				}
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldString.chars[kk]))
					return 0;
			}
			break;
		case pass_startReplace:
			if (replacing) {
				compileError(file, "nested replacement statements");
				return 0;
			}
			if (!appendInstructionChar(
						file, passInstructions, &passIC, pass_startReplace))
				return 0;
			replacing = 1;
			passLinepos++;
			break;
		case pass_endReplace:
			if (!replacing) {
				compileError(file, "unexpected end of replacement");
				return 0;
			}
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_endReplace))
				return 0;
			replacing = 0;
			passLinepos++;
			break;
		case pass_variable:
			passLinepos++;
			if (!passGetVariableNumber(file, &passLine, &passLinepos, &passHoldNumber))
				return 0;
			switch (passLine.chars[passLinepos]) {
			case pass_eq:
				if (!appendInstructionChar(file, passInstructions, &passIC, pass_eq))
					return 0;
				goto doComp;
			case pass_lt:
				if (passLine.chars[passLinepos + 1] == pass_eq) {
					passLinepos++;
					if (!appendInstructionChar(
								file, passInstructions, &passIC, pass_lteq))
						return 0;
				} else if (!appendInstructionChar(
								   file, passInstructions, &passIC, pass_lt))
					return 0;
				goto doComp;
			case pass_gt:
				if (passLine.chars[passLinepos + 1] == pass_eq) {
					passLinepos++;
					if (!appendInstructionChar(
								file, passInstructions, &passIC, pass_gteq))
						return 0;
				} else if (!appendInstructionChar(
								   file, passInstructions, &passIC, pass_gt))
					return 0;
			doComp:
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldNumber))
					return 0;
				passLinepos++;
				passGetNumber(&passLine, &passLinepos, &passHoldNumber);
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldNumber))
					return 0;
				break;
			default:
				compileError(file, "incorrect comparison operator");
				return 0;
			}
			break;
		case pass_attributes:
			passLinepos++;
			if (!passGetAttributes(&passLine, &passLinepos, &passAttributes, file))
				return 0;
		insertAttributes:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_attributes))
				return 0;
			if (!appendInstructionChar(
						file, passInstructions, &passIC, (passAttributes >> 48) & 0xffff))
				return 0;
			if (!appendInstructionChar(
						file, passInstructions, &passIC, (passAttributes >> 32) & 0xffff))
				return 0;
			if (!appendInstructionChar(
						file, passInstructions, &passIC, (passAttributes >> 16) & 0xffff))
				return 0;
			if (!appendInstructionChar(
						file, passInstructions, &passIC, passAttributes & 0xffff))
				return 0;
		getRange:
			if (passLine.chars[passLinepos] == pass_until) {
				passLinepos++;
				if (!appendInstructionChar(file, passInstructions, &passIC, 1)) return 0;
				if (!appendInstructionChar(file, passInstructions, &passIC, 0xffff))
					return 0;
				break;
			}
			passGetNumber(&passLine, &passLinepos, &passHoldNumber);
			if (passHoldNumber == 0) {
				if (!appendInstructionChar(file, passInstructions, &passIC, 1)) return 0;
				if (!appendInstructionChar(file, passInstructions, &passIC, 1)) return 0;
				break;
			}
			if (!appendInstructionChar(file, passInstructions, &passIC, passHoldNumber))
				return 0;
			if (passLine.chars[passLinepos] != pass_hyphen) {
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldNumber))
					return 0;
				break;
			}
			passLinepos++;
			passGetNumber(&passLine, &passLinepos, &passHoldNumber);
			if (passHoldNumber == 0) {
				compileError(file, "invalid range");
				return 0;
			}
			if (!appendInstructionChar(file, passInstructions, &passIC, passHoldNumber))
				return 0;
			break;
		case pass_groupstart:
		case pass_groupend: {
			passLinepos++;
			passGetName(&passLine, &passLinepos, &passHoldString);
			TranslationTableOffset ruleOffset = findRuleName(&passHoldString, *table);
			if (ruleOffset)
				rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
			if (rule && rule->opcode == CTO_Grouping) {
				if (!appendInstructionChar(file, passInstructions, &passIC, passSubOp))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset >> 16))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset & 0xffff))
					return 0;
				break;
			} else {
				compileError(file, "%s is not a grouping name",
						_lou_showString(
								&passHoldString.chars[0], passHoldString.length, 0));
				return 0;
			}
			break;
		}
		case pass_swap: {
			passLinepos++;
			passGetName(&passLine, &passLinepos, &passHoldString);
			if ((class = findCharacterClass(&passHoldString, *table))) {
				passAttributes = class->attribute;
				goto insertAttributes;
			}
			TranslationTableOffset ruleOffset = findRuleName(&passHoldString, *table);
			if (ruleOffset)
				rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
			if (rule &&
					(rule->opcode == CTO_SwapCc || rule->opcode == CTO_SwapCd ||
							rule->opcode == CTO_SwapDd)) {
				if (!appendInstructionChar(file, passInstructions, &passIC, pass_swap))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset >> 16))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset & 0xffff))
					return 0;
				goto getRange;
			}
			compileError(file, "%s is neither a class name nor a swap name.",
					_lou_showString(&passHoldString.chars[0], passHoldString.length, 0));
			return 0;
		}
		case pass_endTest:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_endTest))
				return 0;
			if (replacing) {
				compileError(file, "expected end of replacement");
				return 0;
			}
			passLinepos++;
			break;
		default:
			compileError(file, "incorrect operator '%c ' in test part",
					passLine.chars[passLinepos]);
			return 0;
		}

	} /* Compile action part */

	/* Compile action part */
	while (passLinepos < passLine.length && passLine.chars[passLinepos] <= 32)
		passLinepos++;
	while (passLinepos < passLine.length && passLine.chars[passLinepos] > 32) {
		if (passIC >= MAXSTRING) {
			compileError(file, "Action part in multipass operand too long");
			return 0;
		}
		switch ((passSubOp = passLine.chars[passLinepos])) {
		case pass_string:
			if (!verifyStringOrDots(file, opcode, 1, 1, nofor)) {
				return 0;
			}
			passLinepos++;
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_string))
				return 0;
			passGetString(&passLine, &passLinepos, &passHoldString, file);
			goto actionDoCharsDots;
		case pass_dots:
			if (!verifyStringOrDots(file, opcode, 0, 1, nofor)) {
				return 0;
			}
			passLinepos++;
			passGetDots(&passLine, &passLinepos, &passHoldString, file);
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_dots))
				return 0;
			if (passHoldString.length == 0) {
				compileError(file, "expected dot pattern after @ operand in action part");
				return 0;
			}
		actionDoCharsDots:
			if (passIC >= MAXSTRING) {
				compileError(
						file, "@ operand in action part of multipass operand too long");
				return 0;
			}
			if (!appendInstructionChar(
						file, passInstructions, &passIC, passHoldString.length))
				return 0;
			for (kk = 0; kk < passHoldString.length; kk++) {
				if (passIC >= MAXSTRING) {
					compileError(file,
							"@ operand in action part of multipass operand too long");
					return 0;
				}
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldString.chars[kk]))
					return 0;
			}
			break;
		case pass_variable:
			passLinepos++;
			if (!passGetVariableNumber(file, &passLine, &passLinepos, &passHoldNumber))
				return 0;
			switch (passLine.chars[passLinepos]) {
			case pass_eq:
				if (!appendInstructionChar(file, passInstructions, &passIC, pass_eq))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldNumber))
					return 0;
				passLinepos++;
				passGetNumber(&passLine, &passLinepos, &passHoldNumber);
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldNumber))
					return 0;
				break;
			case pass_plus:
			case pass_hyphen:
				if (!appendInstructionChar(file, passInstructions, &passIC,
							passLine.chars[passLinepos++]))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, passHoldNumber))
					return 0;
				break;
			default:
				compileError(file, "incorrect variable operator in action part");
				return 0;
			}
			break;
		case pass_copy:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_copy))
				return 0;
			passLinepos++;
			break;
		case pass_omit:
			if (!appendInstructionChar(file, passInstructions, &passIC, pass_omit))
				return 0;
			passLinepos++;
			break;
		case pass_groupreplace:
		case pass_groupstart:
		case pass_groupend: {
			passLinepos++;
			passGetName(&passLine, &passLinepos, &passHoldString);
			TranslationTableOffset ruleOffset = findRuleName(&passHoldString, *table);
			if (ruleOffset)
				rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
			if (rule && rule->opcode == CTO_Grouping) {
				if (!appendInstructionChar(file, passInstructions, &passIC, passSubOp))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset >> 16))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset & 0xffff))
					return 0;
				break;
			}
			compileError(file, "%s is not a grouping name",
					_lou_showString(&passHoldString.chars[0], passHoldString.length, 0));
			return 0;
		}
		case pass_swap: {
			passLinepos++;
			passGetName(&passLine, &passLinepos, &passHoldString);
			TranslationTableOffset ruleOffset = findRuleName(&passHoldString, *table);
			if (ruleOffset)
				rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
			if (rule &&
					(rule->opcode == CTO_SwapCc || rule->opcode == CTO_SwapCd ||
							rule->opcode == CTO_SwapDd)) {
				if (!appendInstructionChar(file, passInstructions, &passIC, pass_swap))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset >> 16))
					return 0;
				if (!appendInstructionChar(
							file, passInstructions, &passIC, ruleOffset & 0xffff))
					return 0;
				break;
			}
			compileError(file, "%s is not a swap name.",
					_lou_showString(&passHoldString.chars[0], passHoldString.length, 0));
			return 0;
			break;
		}
		default:
			compileError(file, "incorrect operator in action part");
			return 0;
		}
	}

	/* Analyze and add rule */
	passRuleDots.length = passIC;

	{
		widechar *characters;
		int length;
		int found = passFindCharacters(
				file, passInstructions, passRuleDots.length, &characters, &length);

		if (!found) return 0;

		if (characters) {
			for (k = 0; k < length; k += 1) passRuleChars.chars[k] = characters[k];
			passRuleChars.length = k;
		}
	}

	if (!addRule(file, opcode, &passRuleChars, &passRuleDots, 0, 0, NULL, NULL, noback,
				nofor, table))
		return 0;
	return 1;
}

/* End of multipass compiler */

static int
compileBrailleIndicator(FileInfo *file, const char *ermsg, TranslationTableOpcode opcode,
		TranslationTableOffset *ruleOffset, int noback, int nofor,
		TranslationTableHeader **table) {
	CharsString token;
	CharsString cells;
	if (!getToken(file, &token, ermsg)) return 0;
	if (!parseDots(file, &cells, &token)) return 0;
	return addRule(
			file, opcode, NULL, &cells, 0, 0, ruleOffset, NULL, noback, nofor, table);
}

static int
compileNumber(FileInfo *file) {
	CharsString token;
	widechar number;
	if (!getToken(file, &token, "number")) return 0;
	getNumber(&token.chars[0], &number);
	if (!(number > 0)) {
		compileError(file, "a nonzero positive number is required");
		return 0;
	}
	return number;
}

static int
compileGrouping(FileInfo *file, int noback, int nofor, TranslationTableHeader **table,
		DisplayTableHeader **displayTable) {
	int k;
	CharsString name;
	CharsString groupChars;
	CharsString groupDots;
	CharsString dotsParsed;
	if (!getToken(file, &name, "name operand")) return 0;
	if (!getRuleCharsText(file, &groupChars)) return 0;
	if (!getToken(file, &groupDots, "dots operand")) return 0;
	for (k = 0; k < groupDots.length && groupDots.chars[k] != ','; k++)
		;
	if (k == groupDots.length) {
		compileError(file, "Dots operand must consist of two cells separated by a comma");
		return 0;
	}
	groupDots.chars[k] = '-';
	if (!parseDots(file, &dotsParsed, &groupDots)) return 0;
	if (groupChars.length != 2 || dotsParsed.length != 2) {
		compileError(file,
				"two Unicode characters and two cells separated by a comma are needed.");
		return 0;
	}
	if (table) {
		TranslationTableOffset ruleOffset;
		TranslationTableCharacter *charsDotsPtr;
		charsDotsPtr = putChar(file, groupChars.chars[0], table, NULL);
		charsDotsPtr->attributes |= CTC_Math;
		charsDotsPtr = putChar(file, groupChars.chars[1], table, NULL);
		charsDotsPtr->attributes |= CTC_Math;
		charsDotsPtr = putDots(file, dotsParsed.chars[0], table);
		charsDotsPtr->attributes |= CTC_Math;
		charsDotsPtr = putDots(file, dotsParsed.chars[1], table);
		charsDotsPtr->attributes |= CTC_Math;
		if (!addRule(file, CTO_Grouping, &groupChars, &dotsParsed, 0, 0, &ruleOffset,
					NULL, noback, nofor, table))
			return 0;
		if (!addRuleName(file, &name, ruleOffset, *table)) return 0;
	}
	if (displayTable) {
		putCharDotsMapping(file, groupChars.chars[0], dotsParsed.chars[0], displayTable);
		putCharDotsMapping(file, groupChars.chars[1], dotsParsed.chars[1], displayTable);
	}
	if (table) {
		widechar endChar;
		widechar endDots;
		endChar = groupChars.chars[1];
		endDots = dotsParsed.chars[1];
		groupChars.length = dotsParsed.length = 1;
		if (!addRule(file, CTO_Math, &groupChars, &dotsParsed, 0, 0, NULL, NULL, noback,
					nofor, table))
			return 0;
		groupChars.chars[0] = endChar;
		dotsParsed.chars[0] = endDots;
		if (!addRule(file, CTO_Math, &groupChars, &dotsParsed, 0, 0, NULL, NULL, noback,
					nofor, table))
			return 0;
	}
	return 1;
}

/* Functions for compiling hyphenation tables */

typedef struct HyphenDict { /* hyphenation dictionary: finite state machine */
	int numStates;
	HyphenationState *states;
} HyphenDict;

#define DEFAULTSTATE 0xffff
#define HYPHENHASHSIZE 8191

typedef struct HyphenHashEntry {
	struct HyphenHashEntry *next;
	CharsString *key;
	int val;
} HyphenHashEntry;

typedef struct HyphenHashTab {
	HyphenHashEntry *entries[HYPHENHASHSIZE];
} HyphenHashTab;

/* a hash function from ASU - adapted from Gtk+ */
static unsigned int
hyphenStringHash(const CharsString *s) {
	int k;
	unsigned int h = 0, g;
	for (k = 0; k < s->length; k++) {
		h = (h << 4) + s->chars[k];
		if ((g = h & 0xf0000000)) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
	}
	return h;
}

static HyphenHashTab *
hyphenHashNew(void) {
	HyphenHashTab *hashTab;
	if (!(hashTab = malloc(sizeof(HyphenHashTab)))) _lou_outOfMemory();
	memset(hashTab, 0, sizeof(HyphenHashTab));
	return hashTab;
}

static void
hyphenHashFree(HyphenHashTab *hashTab) {
	int i;
	HyphenHashEntry *e, *next;
	for (i = 0; i < HYPHENHASHSIZE; i++)
		for (e = hashTab->entries[i]; e; e = next) {
			next = e->next;
			free(e->key);
			free(e);
		}
	free(hashTab);
}

/* assumes that key is not already present! */
static void
hyphenHashInsert(HyphenHashTab *hashTab, const CharsString *key, int val) {
	int i, j;
	HyphenHashEntry *e;
	i = hyphenStringHash(key) % HYPHENHASHSIZE;
	if (!(e = malloc(sizeof(HyphenHashEntry)))) _lou_outOfMemory();
	e->next = hashTab->entries[i];
	e->key = malloc((key->length + 1) * CHARSIZE);
	if (!e->key) _lou_outOfMemory();
	e->key->length = key->length;
	for (j = 0; j < key->length; j++) e->key->chars[j] = key->chars[j];
	e->val = val;
	hashTab->entries[i] = e;
}

/* return val if found, otherwise DEFAULTSTATE */
static int
hyphenHashLookup(HyphenHashTab *hashTab, const CharsString *key) {
	int i, j;
	HyphenHashEntry *e;
	if (key->length == 0) return 0;
	i = hyphenStringHash(key) % HYPHENHASHSIZE;
	for (e = hashTab->entries[i]; e; e = e->next) {
		if (key->length != e->key->length) continue;
		for (j = 0; j < key->length; j++)
			if (key->chars[j] != e->key->chars[j]) break;
		if (j == key->length) return e->val;
	}
	return DEFAULTSTATE;
}

static int
hyphenGetNewState(HyphenDict *dict, HyphenHashTab *hashTab, const CharsString *string) {
	hyphenHashInsert(hashTab, string, dict->numStates);
	/* predicate is true if dict->numStates is a power of two */
	if (!(dict->numStates & (dict->numStates - 1)))
		dict->states =
				realloc(dict->states, (dict->numStates << 1) * sizeof(HyphenationState));
	if (!dict->states) _lou_outOfMemory();
	dict->states[dict->numStates].hyphenPattern = 0;
	dict->states[dict->numStates].fallbackState = DEFAULTSTATE;
	dict->states[dict->numStates].numTrans = 0;
	dict->states[dict->numStates].trans.pointer = NULL;
	return dict->numStates++;
}

/* add a transition from state1 to state2 through ch - assumes that the
 * transition does not already exist */
static void
hyphenAddTrans(HyphenDict *dict, int state1, int state2, widechar ch) {
	int numTrans;
	numTrans = dict->states[state1].numTrans;
	if (numTrans == 0)
		dict->states[state1].trans.pointer = malloc(sizeof(HyphenationTrans));
	else if (!(numTrans & (numTrans - 1)))
		dict->states[state1].trans.pointer = realloc(dict->states[state1].trans.pointer,
				(numTrans << 1) * sizeof(HyphenationTrans));
	dict->states[state1].trans.pointer[numTrans].ch = ch;
	dict->states[state1].trans.pointer[numTrans].newState = state2;
	dict->states[state1].numTrans++;
}

static int
compileHyphenation(
		FileInfo *file, CharsString *encoding, TranslationTableHeader **table) {
	CharsString hyph;
	HyphenationTrans *holdPointer;
	HyphenHashTab *hashTab;
	CharsString word;
	char pattern[MAXSTRING + 1];
	unsigned int stateNum = 0, lastState = 0;
	int i, j, k = encoding->length;
	widechar ch;
	int found;
	HyphenHashEntry *e;
	HyphenDict dict;
	TranslationTableOffset holdOffset;
	/* Set aside enough space for hyphenation states and transitions in
	 * translation table. Must be done before anything else */
	allocateSpaceInTranslationTable(file, NULL, 250000, table);
	hashTab = hyphenHashNew();
	dict.numStates = 1;
	dict.states = malloc(sizeof(HyphenationState));
	if (!dict.states) _lou_outOfMemory();
	dict.states[0].hyphenPattern = 0;
	dict.states[0].fallbackState = DEFAULTSTATE;
	dict.states[0].numTrans = 0;
	dict.states[0].trans.pointer = NULL;
	do {
		if (encoding->chars[0] == 'I') {
			if (!getToken(file, &hyph, NULL)) continue;
		} else {
			/* UTF-8 */
			if (!getToken(file, &word, NULL)) continue;
			parseChars(file, &hyph, &word);
		}
		if (hyph.length == 0 || hyph.chars[0] == '#' || hyph.chars[0] == '%' ||
				hyph.chars[0] == '<')
			continue; /* comment */
		j = 0;
		pattern[j] = '0';
		for (i = 0; i < hyph.length; i++) {
			if (hyph.chars[i] >= '0' && hyph.chars[i] <= '9')
				pattern[j] = (char)hyph.chars[i];
			else {
				word.chars[j] = hyph.chars[i];
				pattern[++j] = '0';
			}
		}
		word.chars[j] = 0;
		word.length = j;
		pattern[j + 1] = 0;
		for (i = 0; pattern[i] == '0'; i++)
			;
		found = hyphenHashLookup(hashTab, &word);
		if (found != DEFAULTSTATE)
			stateNum = found;
		else
			stateNum = hyphenGetNewState(&dict, hashTab, &word);
		k = j + 2 - i;
		if (k > 0) {
			allocateSpaceInTranslationTable(
					file, &dict.states[stateNum].hyphenPattern, k, table);
			memcpy(&(*table)->ruleArea[dict.states[stateNum].hyphenPattern], &pattern[i],
					k);
		}
		/* now, put in the prefix transitions */
		while (found == DEFAULTSTATE) {
			lastState = stateNum;
			ch = word.chars[word.length-- - 1];
			found = hyphenHashLookup(hashTab, &word);
			if (found != DEFAULTSTATE)
				stateNum = found;
			else
				stateNum = hyphenGetNewState(&dict, hashTab, &word);
			hyphenAddTrans(&dict, stateNum, lastState, ch);
		}
	} while (_lou_getALine(file));
	/* put in the fallback states */
	for (i = 0; i < HYPHENHASHSIZE; i++) {
		for (e = hashTab->entries[i]; e; e = e->next) {
			for (j = 1; j <= e->key->length; j++) {
				word.length = 0;
				for (k = j; k < e->key->length; k++)
					word.chars[word.length++] = e->key->chars[k];
				stateNum = hyphenHashLookup(hashTab, &word);
				if (stateNum != DEFAULTSTATE) break;
			}
			if (e->val) dict.states[e->val].fallbackState = stateNum;
		}
	}
	hyphenHashFree(hashTab);
	/* Transfer hyphenation information to table */
	for (i = 0; i < dict.numStates; i++) {
		if (dict.states[i].numTrans == 0)
			dict.states[i].trans.offset = 0;
		else {
			holdPointer = dict.states[i].trans.pointer;
			allocateSpaceInTranslationTable(file, &dict.states[i].trans.offset,
					dict.states[i].numTrans * sizeof(HyphenationTrans), table);
			memcpy(&(*table)->ruleArea[dict.states[i].trans.offset], holdPointer,
					dict.states[i].numTrans * sizeof(HyphenationTrans));
			free(holdPointer);
		}
	}
	allocateSpaceInTranslationTable(
			file, &holdOffset, dict.numStates * sizeof(HyphenationState), table);
	(*table)->hyphenStatesArray = holdOffset;
	/* Prevents segmentation fault if table is reallocated */
	memcpy(&(*table)->ruleArea[(*table)->hyphenStatesArray], &dict.states[0],
			dict.numStates * sizeof(HyphenationState));
	free(dict.states);
	return 1;
}

static int
compileCharDef(FileInfo *file, TranslationTableOpcode opcode,
		TranslationTableCharacterAttributes attributes, int noback, int nofor,
		TranslationTableHeader **table, DisplayTableHeader **displayTable) {
	CharsString ruleChars;
	CharsString ruleDots;
	if (!getRuleCharsText(file, &ruleChars)) return 0;
	if (!getRuleDotsPattern(file, &ruleDots)) return 0;
	if (ruleChars.length != 1) {
		compileError(file, "Exactly one character is required.");
		return 0;
	}
	if (ruleDots.length < 1) {
		compileError(file, "At least one cell is required.");
		return 0;
	}
	if (table) {
		TranslationTableCharacter *character;
		TranslationTableCharacter *cell = NULL;
		int k;
		if (attributes & (CTC_UpperCase | CTC_LowerCase)) attributes |= CTC_Letter;
		character = putChar(file, ruleChars.chars[0], table, NULL);
		character->attributes |= attributes;
		for (k = ruleDots.length - 1; k >= 0; k -= 1) {
			cell = getDots(ruleDots.chars[k], *table);
			if (!cell) cell = putDots(file, ruleDots.chars[k], table);
		}
		if (ruleDots.length == 1) cell->attributes |= attributes;
	}
	if (displayTable && ruleDots.length == 1)
		putCharDotsMapping(file, ruleChars.chars[0], ruleDots.chars[0], displayTable);
	if (table)
		if (!addRule(file, opcode, &ruleChars, &ruleDots, 0, 0, NULL, NULL, noback, nofor,
					table))
			return 0;
	return 1;
}

static int
compileBeforeAfter(FileInfo *file) {
	/* 1=before, 2=after, 0=error */
	CharsString token;
	CharsString tmp;
	if (!getToken(file, &token, "last word before or after")) return 0;
	if (!parseChars(file, &tmp, &token)) return 0;
	if (eqasc2uni((unsigned char *)"before", tmp.chars, 6))
		return 1;
	else if (eqasc2uni((unsigned char *)"after", tmp.chars, 5))
		return 2;
	return 0;
}

/**
 * Macro
 */
typedef struct {
	const char *name;
	const widechar *definition;	 // fixed part
	int definition_length;
	const int *substitutions;  // variable part: position and argument index of each
							   // variable substitution
	int substitution_count;
	int argument_count;	 // number of expected arguments
} Macro;

/**
 * List of in-scope macros
 */
typedef struct MacroList {
	const Macro *head;
	const struct MacroList *tail;
} MacroList;

/**
 * Create new macro.
 */
static const Macro *
create_macro(const char *name, const widechar *definition, int definition_length,
		const int *substitutions, int substitution_count, int argument_count) {
	Macro *m = malloc(sizeof(Macro));
	m->name = strdup(name);
	widechar *definition_copy = malloc(definition_length * sizeof(widechar));
	memcpy(definition_copy, definition, definition_length * sizeof(widechar));
	m->definition = definition_copy;
	m->definition_length = definition_length;
	int *substitutions_copy = malloc(2 * substitution_count * sizeof(int));
	memcpy(substitutions_copy, substitutions, 2 * substitution_count * sizeof(int));
	m->substitutions = substitutions_copy;
	m->substitution_count = substitution_count;
	m->argument_count = argument_count;
	return m;
}

/**
 * Create new macro list from "head" macro and "tail" list.
 */
static const MacroList *
cons_macro(const Macro *head, const MacroList *tail) {
	MacroList *list = malloc(sizeof(MacroList));
	list->head = head;
	list->tail = tail;
	return list;
}

/**
 * Free macro returned by create_macro.
 */
static void
free_macro(const Macro *macro) {
	if (macro) {
		free((char *)macro->name);
		free((char *)macro->definition);
		free((int *)macro->substitutions);
		free((Macro *)macro);
	}
}

/**
 * Free macro list returned by cons_macro.
 */
static void
free_macro_list(const MacroList *list) {
	if (list) {
		free_macro((Macro *)list->head);
		free_macro_list((MacroList *)list->tail);
		free((MacroList *)list);
	}
}

/**
 * Compile macro
 */
static int
compileMacro(FileInfo *file, const Macro **macro) {

	// parse name
	CharsString token;
	if (!getToken(file, &token, "macro name")) return 0;
	switch (getOpcode(file, &token)) {
	case CTO_UpLow:	 // deprecated so "uplow" may be used as macro name
	case CTO_None:
		break;
	default:
		compileError(file, "Invalid macro name: already taken by an opcode");
		return 0;
	}
	for (int i = 0; i < token.length; i++) {
		if (!((token.chars[i] >= 'a' && token.chars[i] <= 'z') ||
					(token.chars[i] >= 'A' && token.chars[i] <= 'Z') ||
					(token.chars[i] >= '0' && token.chars[i] <= '9'))) {
			compileError(file,
					"Invalid macro name: must be a word containing only letters and "
					"digits");
			return 0;
		}
	}
	static char name[MAXSTRING + 1];
	int name_length;
	for (name_length = 0; name_length < token.length;
			name_length++)	// we know token can not be longer than MAXSTRING
		name[name_length] = (char)token.chars[name_length];
	name[name_length] = '\0';

	// parse body
	static widechar definition[MAXSTRING];
	static int substitutions[2 * MAX_MACRO_VAR];
	int definition_length = 0;
	int substitution_count = 0;
	int argument_count = 0;
	int dollar = 0;

	// ignore rest of line after name and read lines until "eom" is encountered
	while (_lou_getALine(file)) {
		if (file->linelen >= 3 && file->line[0] == 'e' && file->line[1] == 'o' &&
				file->line[2] == 'm') {
			*macro = create_macro(name, definition, definition_length, substitutions,
					substitution_count, argument_count);
			return 1;
		}
		while (!atEndOfLine(file)) {
			widechar c = file->line[file->linepos++];
			if (dollar) {
				dollar = 0;
				if (c >= '0' && c <= '9') {
					if (substitution_count >= MAX_MACRO_VAR) {
						compileError(file,
								"Macro can not have more than %d variable substitutions",
								MAXSTRING);
						return 0;
					}
					substitutions[2 * substitution_count] = definition_length;
					int arg = c - '0';
					substitutions[2 * substitution_count + 1] = arg;
					if (arg > argument_count) argument_count = arg;
					substitution_count++;
					continue;
				}
			} else if (c == '$') {
				dollar = 1;
				continue;
			}
			if (definition_length >= MAXSTRING) {
				compileError(file, "Macro exceeds %d characters", MAXSTRING);
				return 0;
			} else
				definition[definition_length++] = c;
		}
		dollar = 0;
		if (definition_length >= MAXSTRING) {
			compileError(file, "Macro exceeds %d characters", MAXSTRING);
			return 0;
		}
		definition[definition_length++] = '\n';
	}
	compileError(file, "macro must be terminated with 'eom'");
	return 0;
}

static int
compileRule(FileInfo *file, TranslationTableHeader **table,
		DisplayTableHeader **displayTable, const MacroList **inScopeMacros) {
	CharsString token;
	TranslationTableOpcode opcode;
	CharsString ruleChars;
	CharsString ruleDots;
	CharsString cells;
	CharsString scratchPad;
	CharsString emphClass;
	TranslationTableCharacterAttributes after = 0;
	TranslationTableCharacterAttributes before = 0;
	int noback, nofor, nocross;
	noback = nofor = nocross = 0;
doOpcode:
	if (!getToken(file, &token, NULL)) return 1;				  /* blank line */
	if (token.chars[0] == '#' || token.chars[0] == '<') return 1; /* comment */
	if (file->lineNumber == 1 &&
			(eqasc2uni((unsigned char *)"ISO", token.chars, 3) ||
					eqasc2uni((unsigned char *)"UTF-8", token.chars, 5))) {
		if (table)
			compileHyphenation(file, &token, table);
		else
			/* ignore the whole file */
			while (_lou_getALine(file))
				;
		return 1;
	}
	opcode = getOpcode(file, &token);
	switch (opcode) {
	case CTO_Macro: {
		const Macro *macro;
#ifdef ENABLE_MACROS
		if (!inScopeMacros) {
			compileError(file, "Defining macros only allowed in table files.");
			return 0;
		}
		if (compileMacro(file, &macro)) {
			*inScopeMacros = cons_macro(macro, *inScopeMacros);
			return 1;
		}
		return 0;
#else
		compileError(file, "Macro feature is disabled.");
		return 0;
#endif
	}
	case CTO_IncludeFile: {
		CharsString includedFile;
		if (!getToken(file, &token, "include file name")) return 0;
		if (!parseChars(file, &includedFile, &token)) return 0;
		return includeFile(file, &includedFile, table, displayTable);
	}
	case CTO_NoBack:
		if (nofor) {
			compileError(file, "%s already specified.", _lou_findOpcodeName(CTO_NoFor));
			return 0;
		}
		noback = 1;
		goto doOpcode;
	case CTO_NoFor:
		if (noback) {
			compileError(file, "%s already specified.", _lou_findOpcodeName(CTO_NoBack));
			return 0;
		}
		nofor = 1;
		goto doOpcode;
	case CTO_Space:
		return compileCharDef(
				file, opcode, CTC_Space, noback, nofor, table, displayTable);
	case CTO_Digit:
		return compileCharDef(
				file, opcode, CTC_Digit, noback, nofor, table, displayTable);
	case CTO_LitDigit:
		return compileCharDef(
				file, opcode, CTC_LitDigit, noback, nofor, table, displayTable);
	case CTO_Punctuation:
		return compileCharDef(
				file, opcode, CTC_Punctuation, noback, nofor, table, displayTable);
	case CTO_Math:
		return compileCharDef(file, opcode, CTC_Math, noback, nofor, table, displayTable);
	case CTO_Sign:
		return compileCharDef(file, opcode, CTC_Sign, noback, nofor, table, displayTable);
	case CTO_Letter:
		return compileCharDef(
				file, opcode, CTC_Letter, noback, nofor, table, displayTable);
	case CTO_UpperCase:
		return compileCharDef(
				file, opcode, CTC_UpperCase, noback, nofor, table, displayTable);
	case CTO_LowerCase:
		return compileCharDef(
				file, opcode, CTC_LowerCase, noback, nofor, table, displayTable);
	case CTO_Grouping:
		return compileGrouping(file, noback, nofor, table, displayTable);
	case CTO_Display:
		if (!displayTable) return 1;  // ignore
		if (!getRuleCharsText(file, &ruleChars)) return 0;
		if (!getRuleDotsPattern(file, &ruleDots)) return 0;
		if (ruleChars.length != 1 || ruleDots.length != 1) {
			compileError(file, "Exactly one character and one cell are required.");
			return 0;
		}
		return putCharDotsMapping(
				file, ruleChars.chars[0], ruleDots.chars[0], displayTable);
	case CTO_UpLow:
	case CTO_None: {
		// check if token is a macro name
		if (inScopeMacros) {
			const MacroList *macros = *inScopeMacros;
			while (macros) {
				const Macro *m = macros->head;
				if (token.length == strlen(m->name) &&
						eqasc2uni((unsigned char *)m->name, token.chars, token.length)) {
					if (!inScopeMacros) {
						compileError(file, "Calling macros only allowed in table files.");
						return 0;
					}
					FileInfo tmpFile;
					memset(&tmpFile, 0, sizeof(tmpFile));
					tmpFile.fileName = file->fileName;
					tmpFile.sourceFile = file->sourceFile;
					tmpFile.lineNumber = file->lineNumber;
					tmpFile.encoding = noEncoding;
					tmpFile.status = 0;
					tmpFile.linepos = 0;
					tmpFile.linelen = 0;
					int argument_count = 0;
					CharsString *arguments =
							malloc(m->argument_count * sizeof(CharsString));
					while (argument_count < m->argument_count) {
						if (getToken(file, &token, "macro argument"))
							arguments[argument_count++] = token;
						else
							break;
					}
					if (argument_count < m->argument_count) {
						compileError(file, "Expected %d arguments", m->argument_count);
						return 0;
					}
					int i = 0;
					int subst = 0;
					int next = subst < m->substitution_count ? m->substitutions[2 * subst]
															 : m->definition_length;
					for (;;) {
						while (i < next) {
							widechar c = m->definition[i++];
							if (c == '\n') {
								if (!compileRule(&tmpFile, table, displayTable,
											inScopeMacros)) {
									_lou_logMessage(LOU_LOG_ERROR,
											"result of macro expansion was: %s",
											_lou_showString(
													tmpFile.line, tmpFile.linelen, 0));
									return 0;
								}
								tmpFile.linepos = 0;
								tmpFile.linelen = 0;
							} else if (tmpFile.linelen >= MAXSTRING) {
								compileError(file,
										"Line exceeds %d characters (post macro "
										"expansion)",
										MAXSTRING);
								return 0;
							} else
								tmpFile.line[tmpFile.linelen++] = c;
						}
						if (subst < m->substitution_count) {
							CharsString arg =
									arguments[m->substitutions[2 * subst + 1] - 1];
							for (int j = 0; j < arg.length; j++)
								tmpFile.line[tmpFile.linelen++] = arg.chars[j];
							subst++;
							next = subst < m->substitution_count
									? m->substitutions[2 * subst]
									: m->definition_length;
						} else {
							if (!compileRule(
										&tmpFile, table, displayTable, inScopeMacros)) {
								_lou_logMessage(LOU_LOG_ERROR,
										"result of macro expansion was: %s",
										_lou_showString(
												tmpFile.line, tmpFile.linelen, 0));
								return 0;
							}
							break;
						}
					}
					return 1;
				}
				macros = macros->tail;
			}
		}
		if (opcode == CTO_UpLow) {
			compileError(file, "The uplow opcode is deprecated.");
			return 0;
		}
		compileError(file, "opcode %s not defined.",
				_lou_showString(token.chars, token.length, 0));
		return 0;
	}

	/* now only opcodes follow that don't modify the display table */
	default:
		if (!table) return 1;
		switch (opcode) {
		case CTO_Locale:
			compileWarning(file,
					"The locale opcode is not implemented. Use the locale meta data "
					"instead.");
			return 1;
		case CTO_Undefined: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->undefined;
			if (!compileBrailleIndicator(file, "undefined character opcode",
						CTO_Undefined, &ruleOffset, noback, nofor, table))
				return 0;
			(*table)->undefined = ruleOffset;
			return 1;
		}
		case CTO_Match: {
			int ok = 0;
			widechar *patterns = NULL;
			TranslationTableRule *rule;
			TranslationTableOffset ruleOffset;
			CharsString ptn_before, ptn_after;
			TranslationTableOffset patternsOffset;
			int len, mrk;
			size_t patternsByteSize = sizeof(*patterns) * 27720;
			patterns = (widechar *)malloc(patternsByteSize);
			if (!patterns) _lou_outOfMemory();
			memset(patterns, 0xffff, patternsByteSize);
			noback = 1;
			getCharacters(file, &ptn_before);
			getRuleCharsText(file, &ruleChars);
			getCharacters(file, &ptn_after);
			getRuleDotsPattern(file, &ruleDots);
			if (!addRule(file, opcode, &ruleChars, &ruleDots, after, before, &ruleOffset,
						&rule, noback, nofor, table))
				goto CTO_Match_cleanup;
			if (ptn_before.chars[0] == '-' && ptn_before.length == 1)
				len = _lou_pattern_compile(
						&ptn_before.chars[0], 0, &patterns[1], 13841, *table, file);
			else
				len = _lou_pattern_compile(&ptn_before.chars[0], ptn_before.length,
						&patterns[1], 13841, *table, file);
			if (!len) goto CTO_Match_cleanup;
			mrk = patterns[0] = len + 1;
			_lou_pattern_reverse(&patterns[1]);
			if (ptn_after.chars[0] == '-' && ptn_after.length == 1)
				len = _lou_pattern_compile(
						&ptn_after.chars[0], 0, &patterns[mrk], 13841, *table, file);
			else
				len = _lou_pattern_compile(&ptn_after.chars[0], ptn_after.length,
						&patterns[mrk], 13841, *table, file);
			if (!len) goto CTO_Match_cleanup;
			len += mrk;
			if (!allocateSpaceInTranslationTable(
						file, &patternsOffset, len * sizeof(widechar), table))
				goto CTO_Match_cleanup;
			// allocateSpaceInTranslationTable may have moved table, so make sure rule is
			// still valid
			rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
			memcpy(&(*table)->ruleArea[patternsOffset], patterns, len * sizeof(widechar));
			rule->patterns = patternsOffset;
			ok = 1;
		CTO_Match_cleanup:
			free(patterns);
			return ok;
		}

		case CTO_BackMatch: {
			int ok = 0;
			widechar *patterns = NULL;
			TranslationTableRule *rule;
			TranslationTableOffset ruleOffset;
			CharsString ptn_before, ptn_after;
			TranslationTableOffset patternOffset;
			int len, mrk;
			size_t patternsByteSize = sizeof(*patterns) * 27720;
			patterns = (widechar *)malloc(patternsByteSize);
			if (!patterns) _lou_outOfMemory();
			memset(patterns, 0xffff, patternsByteSize);
			nofor = 1;
			getCharacters(file, &ptn_before);
			getRuleCharsText(file, &ruleChars);
			getCharacters(file, &ptn_after);
			getRuleDotsPattern(file, &ruleDots);
			if (!addRule(file, opcode, &ruleChars, &ruleDots, 0, 0, &ruleOffset, &rule,
						noback, nofor, table))
				goto CTO_BackMatch_cleanup;
			if (ptn_before.chars[0] == '-' && ptn_before.length == 1)
				len = _lou_pattern_compile(
						&ptn_before.chars[0], 0, &patterns[1], 13841, *table, file);
			else
				len = _lou_pattern_compile(&ptn_before.chars[0], ptn_before.length,
						&patterns[1], 13841, *table, file);
			if (!len) goto CTO_BackMatch_cleanup;
			mrk = patterns[0] = len + 1;
			_lou_pattern_reverse(&patterns[1]);
			if (ptn_after.chars[0] == '-' && ptn_after.length == 1)
				len = _lou_pattern_compile(
						&ptn_after.chars[0], 0, &patterns[mrk], 13841, *table, file);
			else
				len = _lou_pattern_compile(&ptn_after.chars[0], ptn_after.length,
						&patterns[mrk], 13841, *table, file);
			if (!len) goto CTO_BackMatch_cleanup;
			len += mrk;
			if (!allocateSpaceInTranslationTable(
						file, &patternOffset, len * sizeof(widechar), table))
				goto CTO_BackMatch_cleanup;
			// allocateSpaceInTranslationTable may have moved table, so make sure rule is
			// still valid
			rule = (TranslationTableRule *)&(*table)->ruleArea[ruleOffset];
			memcpy(&(*table)->ruleArea[patternOffset], patterns, len * sizeof(widechar));
			rule->patterns = patternOffset;
			ok = 1;
		CTO_BackMatch_cleanup:
			free(patterns);
			return ok;
		}

		case CTO_CapsLetter:
		case CTO_BegCapsWord:
		case CTO_EndCapsWord:
		case CTO_BegCaps:
		case CTO_EndCaps:
		case CTO_BegCapsPhrase:
		case CTO_EndCapsPhrase:
		case CTO_LenCapsPhrase:
		/* these 8 general purpose opcodes are compiled further down to more specific
		 * internal opcodes:
		 * - modeletter
		 * - begmodeword
		 * - endmodeword
		 * - begmode
		 * - endmode
		 * - begmodephrase
		 * - endmodephrase
		 * - lenmodephrase
		 */
		case CTO_ModeLetter:
		case CTO_BegModeWord:
		case CTO_EndModeWord:
		case CTO_BegMode:
		case CTO_EndMode:
		case CTO_BegModePhrase:
		case CTO_EndModePhrase:
		case CTO_LenModePhrase: {
			TranslationTableCharacterAttributes mode;
			int i;
			switch (opcode) {
			case CTO_CapsLetter:
			case CTO_BegCapsWord:
			case CTO_EndCapsWord:
			case CTO_BegCaps:
			case CTO_EndCaps:
			case CTO_BegCapsPhrase:
			case CTO_EndCapsPhrase:
			case CTO_LenCapsPhrase:
				mode = CTC_UpperCase;
				i = 0;
				opcode += (CTO_ModeLetter - CTO_CapsLetter);
				break;
			default:
				if (!getToken(file, &token, "attribute name")) return 0;
				if (!(*table)->characterClasses && !allocateCharacterClasses(*table)) {
					return 0;
				}
				const CharacterClass *characterClass = findCharacterClass(&token, *table);
				if (!characterClass) {
					characterClass =
							addCharacterClass(file, token.chars, token.length, *table, 1);
					if (!characterClass) return 0;
				}
				mode = characterClass->attribute;
				if (!(mode == CTC_UpperCase || mode == CTC_Digit) && mode >= CTC_Space &&
						mode <= CTC_LitDigit) {
					compileError(file,
							"mode must be \"uppercase\", \"digit\", or a custom "
							"attribute name.");
					return 0;
				}
				/* check if this mode is already defined and if the number of modes does
				 * not exceed the maximal number */
				if (mode == CTC_UpperCase)
					i = 0;
				else {
					for (i = 1; i < MAX_MODES && (*table)->modes[i].value; i++) {
						if ((*table)->modes[i].mode == mode) {
							break;
						}
					}
					if (i == MAX_MODES) {
						compileError(file, "Max number of modes (%i) reached", MAX_MODES);
						return 0;
					}
				}
			}
			if (!(*table)->modes[i].value)
				(*table)->modes[i] = (EmphasisClass){ plain_text, mode,
					0x1 << (MAX_EMPH_CLASSES + i), MAX_EMPH_CLASSES + i };
			switch (opcode) {
			case CTO_BegModePhrase: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[MAX_EMPH_CLASSES + i][begPhraseOffset];
				if (!compileBrailleIndicator(file, "first word capital sign",
							// when mode is not caps (i != 0), provide enough information
							// for back-translator to be able to recognize and ignore the
							// indicator (but it won't be able to determine the mode)
							i == 0 ? CTO_BegCapsPhrase : CTO_BegModePhrase, &ruleOffset,
							noback, nofor, table))
					return 0;
				(*table)->emphRules[MAX_EMPH_CLASSES + i][begPhraseOffset] = ruleOffset;
				return 1;
			}
			case CTO_EndModePhrase: {
				TranslationTableOffset ruleOffset;
				switch (compileBeforeAfter(file)) {
				case 1:	 // before
					if ((*table)->emphRules[MAX_EMPH_CLASSES + i][endPhraseAfterOffset]) {
						compileError(
								file, "Capital sign after last word already defined.");
						return 0;
					}
					// not passing pointer because compileBrailleIndicator may reallocate
					// table
					ruleOffset = (*table)->emphRules[MAX_EMPH_CLASSES + i]
													[endPhraseBeforeOffset];
					if (!compileBrailleIndicator(file, "capital sign before last word",
								i == 0 ? CTO_EndCapsPhraseBefore : CTO_EndModePhrase,
								&ruleOffset, noback, nofor, table))
						return 0;
					(*table)->emphRules[MAX_EMPH_CLASSES + i][endPhraseBeforeOffset] =
							ruleOffset;
					return 1;
				case 2:	 // after
					if ((*table)->emphRules[MAX_EMPH_CLASSES + i]
										   [endPhraseBeforeOffset]) {
						compileError(
								file, "Capital sign before last word already defined.");
						return 0;
					}
					// not passing pointer because compileBrailleIndicator may reallocate
					// table
					ruleOffset = (*table)->emphRules[MAX_EMPH_CLASSES + i]
													[endPhraseAfterOffset];
					if (!compileBrailleIndicator(file, "capital sign after last word",
								i == 0 ? CTO_EndCapsPhraseAfter : CTO_EndModePhrase,
								&ruleOffset, noback, nofor, table))
						return 0;
					(*table)->emphRules[MAX_EMPH_CLASSES + i][endPhraseAfterOffset] =
							ruleOffset;
					return 1;
				default:  // error
					compileError(file, "Invalid lastword indicator location.");
					return 0;
				}
				return 0;
			}
			case CTO_BegMode: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[MAX_EMPH_CLASSES + i][begOffset];
				if (!compileBrailleIndicator(file, "first letter capital sign",
							i == 0 ? CTO_BegCaps : CTO_BegMode, &ruleOffset, noback,
							nofor, table))
					return 0;
				(*table)->emphRules[MAX_EMPH_CLASSES + i][begOffset] = ruleOffset;
				return 1;
			}
			case CTO_EndMode: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[MAX_EMPH_CLASSES + i][endOffset];
				if (!compileBrailleIndicator(file, "last letter capital sign",
							i == 0 ? CTO_EndCaps : CTO_EndMode, &ruleOffset, noback,
							nofor, table))
					return 0;
				(*table)->emphRules[MAX_EMPH_CLASSES + i][endOffset] = ruleOffset;
				return 1;
			}
			case CTO_ModeLetter: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[MAX_EMPH_CLASSES + i][letterOffset];
				if (!compileBrailleIndicator(file, "single letter capital sign",
							i == 0 ? CTO_CapsLetter : CTO_ModeLetter, &ruleOffset, noback,
							nofor, table))
					return 0;
				(*table)->emphRules[MAX_EMPH_CLASSES + i][letterOffset] = ruleOffset;
				return 1;
			}
			case CTO_BegModeWord: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[MAX_EMPH_CLASSES + i][begWordOffset];
				if (!compileBrailleIndicator(file, "capital word",
							i == 0 ? CTO_BegCapsWord : CTO_BegModeWord, &ruleOffset,
							noback, nofor, table))
					return 0;
				(*table)->emphRules[MAX_EMPH_CLASSES + i][begWordOffset] = ruleOffset;
				return 1;
			}
			case CTO_EndModeWord: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[MAX_EMPH_CLASSES + i][endWordOffset];
				if (!compileBrailleIndicator(file, "capital word stop",
							i == 0 ? CTO_EndCapsWord : CTO_EndModeWord, &ruleOffset,
							noback, nofor, table))
					return 0;
				(*table)->emphRules[MAX_EMPH_CLASSES + i][endWordOffset] = ruleOffset;
				return 1;
			}
			case CTO_LenModePhrase:
				return (*table)->emphRules[MAX_EMPH_CLASSES + i][lenPhraseOffset] =
							   compileNumber(file);
			default:
				break;
			}
			break;
		}

		/* these 8 general purpose emphasis opcodes are compiled further down to more
		 * specific internal opcodes:
		 * - emphletter
		 * - begemphword
		 * - endemphword
		 * - begemph
		 * - endemph
		 * - begemphphrase
		 * - endemphphrase
		 * - lenemphphrase
		 */
		case CTO_EmphClass:
			if (!getToken(file, &emphClass, "emphasis class")) {
				compileError(file, "emphclass must be followed by a valid class name.");
				return 0;
			}
			int k, i;
			char *s = malloc(sizeof(char) * (emphClass.length + 1));
			for (k = 0; k < emphClass.length; k++) s[k] = (char)emphClass.chars[k];
			s[k++] = '\0';
			for (i = 0; i < MAX_EMPH_CLASSES && (*table)->emphClassNames[i]; i++)
				if (strcmp(s, (*table)->emphClassNames[i]) == 0) {
					_lou_logMessage(LOU_LOG_WARN, "Duplicate emphasis class: %s", s);
					warningCount++;
					free(s);
					return 1;
				}
			if (i == MAX_EMPH_CLASSES) {
				_lou_logMessage(LOU_LOG_ERROR,
						"Max number of emphasis classes (%i) reached", MAX_EMPH_CLASSES);
				errorCount++;
				free(s);
				return 0;
			}
			switch (i) {
			/* For backwards compatibility (i.e. because programs will assume
			 * the first 3 typeform bits are `italic', `underline' and `bold')
			 * we require that the first 3 emphclass definitions are (in that
			 * order):
			 *
			 *   emphclass italic
			 *   emphclass underline
			 *   emphclass bold
			 *
			 * While it would be possible to use the emphclass opcode only for
			 * defining _additional_ classes (not allowing for them to be called
			 * italic, underline or bold), thereby reducing the amount of
			 * boilerplate, we deliberately choose not to do that in order to
			 * not give italic, underline and bold any special status. The
			 * hope is that eventually all programs will use liblouis for
			 * emphasis the recommended way (i.e. by looking up the supported
			 * typeforms in the documentation or API) so that we can drop this
			 * restriction.
			 */
			case 0:
				if (strcmp(s, "italic") != 0) {
					_lou_logMessage(LOU_LOG_ERROR,
							"First emphasis class must be \"italic\" but got "
							"%s",
							s);
					errorCount++;
					free(s);
					return 0;
				}
				break;
			case 1:
				if (strcmp(s, "underline") != 0) {
					_lou_logMessage(LOU_LOG_ERROR,
							"Second emphasis class must be \"underline\" but "
							"got "
							"%s",
							s);
					errorCount++;
					free(s);
					return 0;
				}
				break;
			case 2:
				if (strcmp(s, "bold") != 0) {
					_lou_logMessage(LOU_LOG_ERROR,
							"Third emphasis class must be \"bold\" but got "
							"%s",
							s);
					errorCount++;
					free(s);
					return 0;
				}
				break;
			}
			(*table)->emphClassNames[i] = s;
			(*table)->emphClasses[i] = (EmphasisClass){ emph_1
						<< i, /* relies on the order of typeforms emph_1..emph_10 */
				0, 0x1 << i, i };
			return 1;
		case CTO_EmphLetter:
		case CTO_BegEmphWord:
		case CTO_EndEmphWord:
		case CTO_BegEmph:
		case CTO_EndEmph:
		case CTO_BegEmphPhrase:
		case CTO_EndEmphPhrase:
		case CTO_LenEmphPhrase:
		case CTO_EmphModeChars:
		case CTO_NoEmphChars: {
			if (!getToken(file, &token, "emphasis class")) return 0;
			if (!parseChars(file, &emphClass, &token)) return 0;
			char *s = malloc(sizeof(char) * (emphClass.length + 1));
			int k, i;
			for (k = 0; k < emphClass.length; k++) s[k] = (char)emphClass.chars[k];
			s[k++] = '\0';
			for (i = 0; i < MAX_EMPH_CLASSES && (*table)->emphClassNames[i]; i++)
				if (strcmp(s, (*table)->emphClassNames[i]) == 0) break;
			if (i == MAX_EMPH_CLASSES || !(*table)->emphClassNames[i]) {
				_lou_logMessage(LOU_LOG_ERROR, "Emphasis class %s not declared", s);
				errorCount++;
				free(s);
				return 0;
			}
			int ok = 0;
			switch (opcode) {
			case CTO_EmphLetter: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset = (*table)->emphRules[i][letterOffset];
				// provide enough information for back-translator to be able to recognize
				// and ignore the indicator (but it won't be able to determine the
				// emphasis class)
				if (!compileBrailleIndicator(file, "single letter", CTO_EmphLetter,
							&ruleOffset, noback, nofor, table))
					break;
				(*table)->emphRules[i][letterOffset] = ruleOffset;
				ok = 1;
				break;
			}
			case CTO_BegEmphWord: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset = (*table)->emphRules[i][begWordOffset];
				if (!compileBrailleIndicator(file, "word", CTO_BegEmphWord, &ruleOffset,
							noback, nofor, table))
					break;
				(*table)->emphRules[i][begWordOffset] = ruleOffset;
				ok = 1;
				break;
			}
			case CTO_EndEmphWord: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset = (*table)->emphRules[i][endWordOffset];
				if (!compileBrailleIndicator(file, "word stop", CTO_EndEmphWord,
							&ruleOffset, noback, nofor, table))
					break;
				(*table)->emphRules[i][endWordOffset] = ruleOffset;
				ok = 1;
				break;
			}
			case CTO_BegEmph: {
				/* fail if both begemph and any of begemphphrase or begemphword are
				 * defined */
				if ((*table)->emphRules[i][begWordOffset] ||
						(*table)->emphRules[i][begPhraseOffset]) {
					compileError(file,
							"Cannot define emphasis for both no context and word or "
							"phrase context, i.e. cannot have both begemph and "
							"begemphword or begemphphrase.");
					break;
				}
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset = (*table)->emphRules[i][begOffset];
				if (!compileBrailleIndicator(file, "first letter", CTO_BegEmph,
							&ruleOffset, noback, nofor, table))
					break;
				(*table)->emphRules[i][begOffset] = ruleOffset;
				ok = 1;
				break;
			}
			case CTO_EndEmph: {
				if ((*table)->emphRules[i][endWordOffset] ||
						(*table)->emphRules[i][endPhraseBeforeOffset] ||
						(*table)->emphRules[i][endPhraseAfterOffset]) {
					compileError(file,
							"Cannot define emphasis for both no context and word or "
							"phrase context, i.e. cannot have both endemph and "
							"endemphword or endemphphrase.");
					break;
				}
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset = (*table)->emphRules[i][endOffset];
				if (!compileBrailleIndicator(file, "last letter", CTO_EndEmph,
							&ruleOffset, noback, nofor, table))
					break;
				(*table)->emphRules[i][endOffset] = ruleOffset;
				ok = 1;
				break;
			}
			case CTO_BegEmphPhrase: {
				// not passing pointer because compileBrailleIndicator may reallocate
				// table
				TranslationTableOffset ruleOffset =
						(*table)->emphRules[i][begPhraseOffset];
				if (!compileBrailleIndicator(file, "first word", CTO_BegEmphPhrase,
							&ruleOffset, noback, nofor, table))
					break;
				(*table)->emphRules[i][begPhraseOffset] = ruleOffset;
				ok = 1;
				break;
			}
			case CTO_EndEmphPhrase:
				switch (compileBeforeAfter(file)) {
				case 1: {  // before
					if ((*table)->emphRules[i][endPhraseAfterOffset]) {
						compileError(file, "last word after already defined.");
						break;
					}
					// not passing pointer because compileBrailleIndicator may reallocate
					// table
					TranslationTableOffset ruleOffset =
							(*table)->emphRules[i][endPhraseBeforeOffset];
					if (!compileBrailleIndicator(file, "last word before",
								CTO_EndEmphPhrase, &ruleOffset, noback, nofor, table))
						break;
					(*table)->emphRules[i][endPhraseBeforeOffset] = ruleOffset;
					ok = 1;
					break;
				}
				case 2: {  // after
					if ((*table)->emphRules[i][endPhraseBeforeOffset]) {
						compileError(file, "last word before already defined.");
						break;
					}
					// not passing pointer because compileBrailleIndicator may reallocate
					// table
					TranslationTableOffset ruleOffset =
							(*table)->emphRules[i][endPhraseAfterOffset];
					if (!compileBrailleIndicator(file, "last word after",
								CTO_EndEmphPhrase, &ruleOffset, noback, nofor, table))
						break;
					(*table)->emphRules[i][endPhraseAfterOffset] = ruleOffset;
					ok = 1;
					break;
				}
				default:  // error
					compileError(file, "Invalid lastword indicator location.");
					break;
				}
				break;
			case CTO_LenEmphPhrase:
				if (((*table)->emphRules[i][lenPhraseOffset] = compileNumber(file)))
					ok = 1;
				break;
			case CTO_EmphModeChars: {
				if (!getRuleCharsText(file, &ruleChars)) break;
				widechar *emphmodechars = (*table)->emphModeChars[i];
				int len;
				for (len = 0; len < EMPHMODECHARSSIZE && emphmodechars[len]; len++)
					;
				if (len + ruleChars.length > EMPHMODECHARSSIZE) {
					compileError(file, "More than %d characters", EMPHMODECHARSSIZE);
					break;
				}
				ok = 1;
				for (int k = 0; k < ruleChars.length; k++) {
					if (!getChar(ruleChars.chars[k], *table, NULL)) {
						compileError(file, "Emphasis mode character undefined");
						ok = 0;
						break;
					}
					emphmodechars[len++] = ruleChars.chars[k];
				}
				break;
			}
			case CTO_NoEmphChars: {
				if (!getRuleCharsText(file, &ruleChars)) break;
				widechar *noemphchars = (*table)->noEmphChars[i];
				int len;
				for (len = 0; len < NOEMPHCHARSSIZE && noemphchars[len]; len++)
					;
				if (len + ruleChars.length > NOEMPHCHARSSIZE) {
					compileError(file, "More than %d characters", NOEMPHCHARSSIZE);
					break;
				}
				ok = 1;
				for (int k = 0; k < ruleChars.length; k++) {
					if (!getChar(ruleChars.chars[k], *table, NULL)) {
						compileError(file, "Character undefined");
						ok = 0;
						break;
					}
					noemphchars[len++] = ruleChars.chars[k];
				}
				break;
			}
			default:
				break;
			}
			free(s);
			return ok;
		}
		case CTO_LetterSign: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->letterSign;
			if (!compileBrailleIndicator(file, "letter sign", CTO_LetterSign, &ruleOffset,
						noback, nofor, table))
				return 0;
			(*table)->letterSign = ruleOffset;
			return 1;
		}
		case CTO_NoLetsignBefore:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (((*table)->noLetsignBeforeCount + ruleChars.length) > LETSIGNBEFORESIZE) {
				compileError(file, "More than %d characters", LETSIGNBEFORESIZE);
				return 0;
			}
			for (int k = 0; k < ruleChars.length; k++)
				(*table)->noLetsignBefore[(*table)->noLetsignBeforeCount++] =
						ruleChars.chars[k];
			return 1;
		case CTO_NoLetsign:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (((*table)->noLetsignCount + ruleChars.length) > LETSIGNSIZE) {
				compileError(file, "More than %d characters", LETSIGNSIZE);
				return 0;
			}
			for (int k = 0; k < ruleChars.length; k++)
				(*table)->noLetsign[(*table)->noLetsignCount++] = ruleChars.chars[k];
			return 1;
		case CTO_NoLetsignAfter:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (((*table)->noLetsignAfterCount + ruleChars.length) > LETSIGNAFTERSIZE) {
				compileError(file, "More than %d characters", LETSIGNAFTERSIZE);
				return 0;
			}
			for (int k = 0; k < ruleChars.length; k++)
				(*table)->noLetsignAfter[(*table)->noLetsignAfterCount++] =
						ruleChars.chars[k];
			return 1;
		case CTO_NumberSign: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->numberSign;
			if (!compileBrailleIndicator(file, "number sign", CTO_NumberSign, &ruleOffset,
						noback, nofor, table))
				return 0;
			(*table)->numberSign = ruleOffset;
			return 1;
		}
		case CTO_NoNumberSign: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->noNumberSign;
			if (!compileBrailleIndicator(file, "no number sign", CTO_NoNumberSign,
						&ruleOffset, noback, nofor, table))
				return 0;
			(*table)->noNumberSign = ruleOffset;
			return 1;
		}

		case CTO_NumericModeChars:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Numeric mode character undefined: %s",
							_lou_showString(&ruleChars.chars[k], 1, 0));
					return 0;
				}
				c->attributes |= CTC_NumericMode;
				(*table)->usesNumericMode = 1;
			}
			return 1;

		case CTO_MidEndNumericModeChars:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Midendnumeric mode character undefined");
					return 0;
				}
				c->attributes |= CTC_MidEndNumericMode;
				(*table)->usesNumericMode = 1;
			}
			return 1;

		case CTO_NumericNoContractChars:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Numeric no contraction character undefined");
					return 0;
				}
				c->attributes |= CTC_NumericNoContract;
				(*table)->usesNumericMode = 1;
			}
			return 1;

		case CTO_NoContractSign: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->noContractSign;
			if (!compileBrailleIndicator(file, "no contractions sign", CTO_NoContractSign,
						&ruleOffset, noback, nofor, table))
				return 0;
			(*table)->noContractSign = ruleOffset;
			return 1;
		}
		case CTO_SeqDelimiter:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Sequence delimiter character undefined");
					return 0;
				}
				c->attributes |= CTC_SeqDelimiter;
				(*table)->usesSequences = 1;
			}
			return 1;

		case CTO_SeqBeforeChars:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Sequence before character undefined");
					return 0;
				}
				c->attributes |= CTC_SeqBefore;
			}
			return 1;

		case CTO_SeqAfterChars:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Sequence after character undefined");
					return 0;
				}
				c->attributes |= CTC_SeqAfter;
			}
			return 1;

		case CTO_SeqAfterPattern:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (((*table)->seqPatternsCount + ruleChars.length + 1) > SEQPATTERNSIZE) {
				compileError(file, "More than %d characters", SEQPATTERNSIZE);
				return 0;
			}
			for (int k = 0; k < ruleChars.length; k++)
				(*table)->seqPatterns[(*table)->seqPatternsCount++] = ruleChars.chars[k];
			(*table)->seqPatterns[(*table)->seqPatternsCount++] = 0;
			return 1;

		case CTO_SeqAfterExpression:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if ((ruleChars.length + 1) > SEQPATTERNSIZE) {
				compileError(file, "More than %d characters", SEQPATTERNSIZE);
				return 0;
			}
			for (int k = 0; k < ruleChars.length; k++)
				(*table)->seqAfterExpression[k] = ruleChars.chars[k];
			(*table)->seqAfterExpression[ruleChars.length] = 0;
			(*table)->seqAfterExpressionLength = ruleChars.length;
			return 1;

		case CTO_CapsModeChars:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!c) {
					compileError(file, "Capital mode character undefined");
					return 0;
				}
				c->attributes |= CTC_CapsMode;
				(*table)->hasCapsModeChars = 1;
			}
			return 1;

		case CTO_BegComp: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->begComp;
			if (!compileBrailleIndicator(file, "begin computer braille", CTO_BegComp,
						&ruleOffset, noback, nofor, table))
				return 0;
			(*table)->begComp = ruleOffset;
			return 1;
		}
		case CTO_EndComp: {
			// not passing pointer because compileBrailleIndicator may reallocate table
			TranslationTableOffset ruleOffset = (*table)->endComp;
			if (!compileBrailleIndicator(file, "end computer braslle", CTO_EndComp,
						&ruleOffset, noback, nofor, table))
				return 0;
			(*table)->endComp = ruleOffset;
			return 1;
		}
		case CTO_NoCross:
			if (nocross) {
				compileError(
						file, "%s already specified.", _lou_findOpcodeName(CTO_NoCross));
				return 0;
			}
			nocross = 1;
			goto doOpcode;
		case CTO_Syllable:
			(*table)->syllables = 1;
		case CTO_Always:
		case CTO_LargeSign:
		case CTO_WholeWord:
		case CTO_PartWord:
		case CTO_JoinNum:
		case CTO_JoinableWord:
		case CTO_LowWord:
		case CTO_SuffixableWord:
		case CTO_PrefixableWord:
		case CTO_BegWord:
		case CTO_BegMidWord:
		case CTO_MidWord:
		case CTO_MidEndWord:
		case CTO_EndWord:
		case CTO_PrePunc:
		case CTO_PostPunc:
		case CTO_BegNum:
		case CTO_MidNum:
		case CTO_EndNum:
		case CTO_Repeated:
		case CTO_RepWord:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (!getRuleDotsPattern(file, &ruleDots)) return 0;
			if (ruleDots.length == 0)
				// check that all characters in a rule with `=` as second operand are
				// defined (or based on another character)
				for (int k = 0; k < ruleChars.length; k++) {
					TranslationTableCharacter *c =
							getChar(ruleChars.chars[k], *table, NULL);
					if (!(c && (c->definitionRule || c->basechar))) {
						compileError(file, "Character %s is not defined",
								_lou_showString(&ruleChars.chars[k], 1, 0));
						return 0;
					}
				}
			TranslationTableRule *r;
			if (!addRule(file, opcode, &ruleChars, &ruleDots, after, before, NULL, &r,
						noback, nofor, table))
				return 0;
			if (nocross) r->nocross = 1;
			return 1;
			// if (opcode == CTO_MidNum)
			// {
			//   TranslationTableCharacter *c = getChar(ruleChars.chars[0]);
			//   if(c)
			//     c->attributes |= CTC_NumericMode;
			// }
		case CTO_RepEndWord:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			CharsString dots;
			if (!getToken(file, &dots, "dots,dots operand")) return 0;
			int len = dots.length;
			for (int k = 0; k < len - 1; k++) {
				if (dots.chars[k] == ',') {
					dots.length = k;
					if (!parseDots(file, &ruleDots, &dots)) return 0;
					ruleDots.chars[ruleDots.length++] = ',';
					k++;
					if (k == len - 1 && dots.chars[k] == '=') {
						// check that all characters are defined (or based on another
						// character)
						for (int l = 0; l < ruleChars.length; l++) {
							TranslationTableCharacter *c =
									getChar(ruleChars.chars[l], *table, NULL);
							if (!(c && (c->definitionRule || c->basechar))) {
								compileError(file, "Character %s is not defined",
										_lou_showString(&ruleChars.chars[l], 1, 0));
								return 0;
							}
						}
					} else {
						CharsString x, y;
						x.length = 0;
						while (k < len) x.chars[x.length++] = dots.chars[k++];
						if (parseDots(file, &y, &x))
							for (int l = 0; l < y.length; l++)
								ruleDots.chars[ruleDots.length++] = y.chars[l];
					}
					return addRule(file, opcode, &ruleChars, &ruleDots, after, before,
							NULL, NULL, noback, nofor, table);
				}
			}
			return 0;
		case CTO_CompDots:
		case CTO_Comp6: {
			TranslationTableOffset ruleOffset;
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (ruleChars.length != 1) {
				compileError(file, "first operand must be 1 character");
				return 0;
			}
			if (nofor || noback) {
				compileWarning(file, "nofor and noback not allowed on comp6 rules");
			}
			if (!getRuleDotsPattern(file, &ruleDots)) return 0;
			if (!addRule(file, opcode, &ruleChars, &ruleDots, after, before, &ruleOffset,
						NULL, noback, nofor, table))
				return 0;
			return 1;
		}
		case CTO_ExactDots:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (ruleChars.chars[0] != '@') {
				compileError(file, "The operand must begin with an at sign (@)");
				return 0;
			}
			for (int k = 1; k < ruleChars.length; k++)
				scratchPad.chars[k - 1] = ruleChars.chars[k];
			scratchPad.length = ruleChars.length - 1;
			if (!parseDots(file, &ruleDots, &scratchPad)) return 0;
			return addRule(file, opcode, &ruleChars, &ruleDots, before, after, NULL, NULL,
					noback, nofor, table);
		case CTO_CapsNoCont: {
			TranslationTableOffset ruleOffset;
			ruleChars.length = 1;
			ruleChars.chars[0] = 'a';
			if (!addRule(file, opcode, &ruleChars, NULL, after, before, &ruleOffset, NULL,
						noback, nofor, table))
				return 0;
			(*table)->capsNoCont = ruleOffset;
			return 1;
		}
		case CTO_Replace:
			if (getRuleCharsText(file, &ruleChars)) {
				if (atEndOfLine(file))
					ruleDots.length = ruleDots.chars[0] = 0;
				else {
					getRuleDotsText(file, &ruleDots);
					if (ruleDots.chars[0] == '#')
						ruleDots.length = ruleDots.chars[0] = 0;
					else if (ruleDots.chars[0] == '\\' && ruleDots.chars[1] == '#')
						memmove(&ruleDots.chars[0], &ruleDots.chars[1],
								ruleDots.length-- * CHARSIZE);
				}
			}
			for (int k = 0; k < ruleChars.length; k++)
				putChar(file, ruleChars.chars[k], table, NULL);
			for (int k = 0; k < ruleDots.length; k++)
				putChar(file, ruleDots.chars[k], table, NULL);
			return addRule(file, opcode, &ruleChars, &ruleDots, after, before, NULL, NULL,
					noback, nofor, table);
		case CTO_Correct:
			(*table)->corrections = 1;
			goto doPass;
		case CTO_Pass2:
			if ((*table)->numPasses < 2) (*table)->numPasses = 2;
			goto doPass;
		case CTO_Pass3:
			if ((*table)->numPasses < 3) (*table)->numPasses = 3;
			goto doPass;
		case CTO_Pass4:
			if ((*table)->numPasses < 4) (*table)->numPasses = 4;
		doPass:
		case CTO_Context:
			if (!(nofor || noback)) {
				compileError(file, "%s or %s must be specified.",
						_lou_findOpcodeName(CTO_NoFor), _lou_findOpcodeName(CTO_NoBack));
				return 0;
			}
			return compilePassOpcode(file, opcode, noback, nofor, table);
		case CTO_Contraction:
		case CTO_NoCont:
		case CTO_CompBrl:
		case CTO_Literal:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			// check that all characters in a compbrl, contraction,
			// nocont or literal rule are defined (or based on another
			// character)
			for (int k = 0; k < ruleChars.length; k++) {
				TranslationTableCharacter *c = getChar(ruleChars.chars[k], *table, NULL);
				if (!(c && (c->definitionRule || c->basechar))) {
					compileError(file, "Character %s is not defined",
							_lou_showString(&ruleChars.chars[k], 1, 0));
					return 0;
				}
			}
			return addRule(file, opcode, &ruleChars, NULL, after, before, NULL, NULL,
					noback, nofor, table);
		case CTO_MultInd: {
			ruleChars.length = 0;
			if (!getToken(file, &token, "multiple braille indicators") ||
					!parseDots(file, &cells, &token))
				return 0;
			while (getToken(file, &token, "multind opcodes")) {
				opcode = getOpcode(file, &token);
				if (opcode == CTO_None) {
					compileError(file, "opcode %s not defined.",
							_lou_showString(token.chars, token.length, 0));
					return 0;
				}
				if (!(opcode >= CTO_CapsLetter && opcode < CTO_MultInd)) {
					compileError(file, "Not a braille indicator opcode.");
					return 0;
				}
				ruleChars.chars[ruleChars.length++] = (widechar)opcode;
				if (atEndOfLine(file)) break;
			}
			return addRule(file, CTO_MultInd, &ruleChars, &cells, after, before, NULL,
					NULL, noback, nofor, table);
		}

		case CTO_Class:
			compileWarning(file, "class is deprecated, use attribute instead");
		case CTO_Attribute: {
			if (nofor || noback) {
				compileWarning(
						file, "nofor and noback not allowed before class/attribute");
			}
			if ((opcode == CTO_Class && (*table)->usesAttributeOrClass == 1) ||
					(opcode == CTO_Attribute && (*table)->usesAttributeOrClass == 2)) {
				compileError(file,
						"attribute and class rules must not be both present in a table");
				return 0;
			}
			if (opcode == CTO_Class)
				(*table)->usesAttributeOrClass = 2;
			else
				(*table)->usesAttributeOrClass = 1;
			if (!getToken(file, &token, "attribute name")) {
				compileError(file, "Expected %s", "attribute name");
				return 0;
			}
			if (!(*table)->characterClasses && !allocateCharacterClasses(*table)) {
				return 0;
			}

			TranslationTableCharacterAttributes attribute = 0;
			{
				int attrNumber = -1;
				switch (token.chars[0]) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					attrNumber = token.chars[0] - '0';
					break;
				}
				if (attrNumber >= 0) {
					if (opcode == CTO_Class) {
						compileError(file,
								"Invalid class name: may not contain digits, use "
								"attribute instead of class");
						return 0;
					}
					if (token.length > 1 || attrNumber > 7) {
						compileError(file,
								"Invalid attribute name: must be a digit between 0 and 7 "
								"or a word containing only letters");
						return 0;
					}
					if (!(*table)->numberedAttributes[attrNumber])
						// attribute not used before yet: assign it a value
						(*table)->numberedAttributes[attrNumber] =
								getNextNumberedAttribute(*table);
					attribute = (*table)->numberedAttributes[attrNumber];
				} else {
					const CharacterClass *namedAttr = findCharacterClass(&token, *table);
					if (!namedAttr) {
						// no class with that name: create one
						namedAttr = addCharacterClass(
								file, &token.chars[0], token.length, *table, 1);
						if (!namedAttr) return 0;
					}
					// there is a class with that name or a new class was successfully
					// created
					attribute = namedAttr->attribute;
					if (attribute == CTC_UpperCase || attribute == CTC_LowerCase)
						attribute |= CTC_Letter;
				}
			}
			CharsString characters;
			if (!getCharacters(file, &characters)) return 0;
			for (int i = 0; i < characters.length; i++) {
				// get the character from the table, or if it is not defined yet,
				// define it
				TranslationTableCharacter *character =
						putChar(file, characters.chars[i], table, NULL);
				// set the attribute
				character->attributes |= attribute;
				// also set the attribute on the associated dots (if any)
				if (character->basechar)
					character = (TranslationTableCharacter *)&(*table)
										->ruleArea[character->basechar];
				if (character->definitionRule) {
					TranslationTableRule *defRule =
							(TranslationTableRule *)&(*table)
									->ruleArea[character->definitionRule];
					if (defRule->dotslen == 1) {
						TranslationTableCharacter *dots =
								getDots(defRule->charsdots[defRule->charslen], *table);
						if (dots) dots->attributes |= attribute;
					}
				}
			}
			return 1;
		}

			{
				TranslationTableCharacterAttributes *attributes;
				const CharacterClass *class;
			case CTO_After:
				attributes = &after;
				goto doBeforeAfter;
			case CTO_Before:
				attributes = &before;
			doBeforeAfter:
				if (!(*table)->characterClasses) {
					if (!allocateCharacterClasses(*table)) return 0;
				}
				if (!getToken(file, &token, "attribute name")) return 0;
				if (!(class = findCharacterClass(&token, *table))) {
					compileError(file, "attribute not defined");
					return 0;
				}
				*attributes |= class->attribute;
				goto doOpcode;
			}
		case CTO_Base:
			if (nofor || noback) {
				compileWarning(file, "nofor and noback not allowed before base");
			}
			if (!getToken(file, &token, "attribute name")) {
				compileError(
						file, "base opcode must be followed by a valid attribute name.");
				return 0;
			}
			if (!(*table)->characterClasses && !allocateCharacterClasses(*table)) {
				return 0;
			}
			const CharacterClass *mode = findCharacterClass(&token, *table);
			if (!mode) {
				mode = addCharacterClass(file, token.chars, token.length, *table, 1);
				if (!mode) return 0;
			}
			if (!(mode->attribute == CTC_UpperCase || mode->attribute == CTC_Digit) &&
					mode->attribute >= CTC_Space && mode->attribute <= CTC_LitDigit) {
				compileError(file,
						"base opcode must be followed by \"uppercase\", \"digit\", or a "
						"custom attribute name.");
				return 0;
			}
			if (!getRuleCharsText(file, &token)) return 0;
			if (token.length != 1) {
				compileError(file,
						"Exactly one character followed by one base character is "
						"required.");
				return 0;
			}
			TranslationTableOffset characterOffset;
			TranslationTableCharacter *character =
					putChar(file, token.chars[0], table, &characterOffset);
			if (!getRuleCharsText(file, &token)) return 0;
			if (token.length != 1) {
				compileError(file, "Exactly one base character is required.");
				return 0;
			}
			if (character->definitionRule) {
				TranslationTableRule *prevRule =
						(TranslationTableRule *)&(*table)
								->ruleArea[character->definitionRule];
				_lou_logMessage(LOU_LOG_DEBUG,
						"%s:%d: Character already defined (%s). The base rule will take "
						"precedence.",
						file->fileName, file->lineNumber,
						printSource(file, prevRule->sourceFile, prevRule->sourceLine));
				character->definitionRule = 0;
			}
			TranslationTableOffset basechar;
			putChar(file, token.chars[0], table, &basechar);
			// putChar may have moved table, so make sure character is still valid
			character = (TranslationTableCharacter *)&(*table)->ruleArea[characterOffset];
			if (character->basechar) {
				if (character->basechar == basechar &&
						character->mode == mode->attribute) {
					_lou_logMessage(LOU_LOG_DEBUG, "%s:%d: Duplicate base rule.",
							file->fileName, file->lineNumber);
				} else {
					_lou_logMessage(LOU_LOG_DEBUG,
							"%s:%d: A different base rule already exists for this "
							"character (%s). The new rule will take precedence.",
							file->fileName, file->lineNumber,
							printSource(
									file, character->sourceFile, character->sourceLine));
				}
			}
			character->basechar = basechar;
			character->mode = mode->attribute;
			character->sourceFile = file->sourceFile;
			character->sourceLine = file->lineNumber;
			/* some other processing is done at the end of the compilation, in
			 * finalizeTable() */
			return 1;
		case CTO_EmpMatchBefore:
			before |= CTC_EmpMatch;
			goto doOpcode;
		case CTO_EmpMatchAfter:
			after |= CTC_EmpMatch;
			goto doOpcode;

		case CTO_SwapCc:
		case CTO_SwapCd:
		case CTO_SwapDd:
			return compileSwap(file, opcode, noback, nofor, table);
		case CTO_Hyphen:
		case CTO_DecPoint:
			//	case CTO_Apostrophe:
			//	case CTO_Initial:
			if (!getRuleCharsText(file, &ruleChars)) return 0;
			if (!getRuleDotsPattern(file, &ruleDots)) return 0;
			if (ruleChars.length != 1 || ruleDots.length < 1) {
				compileError(file,
						"One Unicode character and at least one cell are "
						"required.");
				return 0;
			}
			return addRule(file, opcode, &ruleChars, &ruleDots, after, before, NULL, NULL,
					noback, nofor, table);
			// if (opcode == CTO_DecPoint)
			// {
			//   TranslationTableCharacter *c =
			//   getChar(ruleChars.chars[0]);
			//   if(c)
			//     c->attributes |= CTC_NumericMode;
			// }
		default:
			compileError(file, "unimplemented opcode.");
			return 0;
		}
	}
	return 0;
}

int EXPORT_CALL
lou_readCharFromFile(const char *fileName, int *mode) {
	/* Read a character from a file, whether big-endian, little-endian or
	 * ASCII8 */
	int ch;
	static FileInfo file;
	if (fileName == NULL) return 0;
	if (*mode == 1) {
		*mode = 0;
		file.fileName = fileName;
		file.encoding = noEncoding;
		file.status = 0;
		file.lineNumber = 0;
		if (!(file.in = fopen(file.fileName, "r"))) {
			_lou_logMessage(LOU_LOG_ERROR, "Cannot open file '%s'", file.fileName);
			*mode = 1;
			return EOF;
		}
	}
	if (file.in == NULL) {
		*mode = 1;
		return EOF;
	}
	ch = getAChar(&file);
	if (ch == EOF) {
		fclose(file.in);
		file.in = NULL;
		*mode = 1;
	}
	return ch;
}

static int
finalizeTable(TranslationTableHeader *table) {
	if (table->finalized) return 1;
	// normalize basechar and mode of all characters
	for (int i = 0; i < HASHNUM; i++) {
		TranslationTableOffset characterOffset = table->characters[i];
		while (characterOffset) {
			TranslationTableCharacter *character =
					(TranslationTableCharacter *)&table->ruleArea[characterOffset];
			if (character->basechar) {
				TranslationTableOffset basecharOffset = 0;
				TranslationTableCharacter *basechar = character;
				TranslationTableCharacterAttributes mode = 0;
				int detect_loop = 0;
				while (basechar->basechar) {
					if (basechar->basechar == characterOffset ||
							detect_loop++ > MAX_MODES) {
						_lou_logMessage(LOU_LOG_ERROR,
								"%s: error: Character can not be (indirectly) based on "
								"itself.",
								printSource(NULL, character->sourceFile,
										character->sourceLine));
						errorCount++;
						return 0;
					}
					// inherit basechar mode
					mode |= basechar->mode;
					// compute basechar recursively
					basecharOffset = basechar->basechar;
					basechar =
							(TranslationTableCharacter *)&table->ruleArea[basecharOffset];
					if (character->mode & (basechar->attributes | basechar->mode)) {
						char *attributeName = NULL;
						const CharacterClass *class = table->characterClasses;
						while (class) {
							if (class->attribute == character->mode) {
								attributeName = strdup(
										_lou_showString(class->name, class->length, 0));
								break;
							}
							class = class->next;
						}
						_lou_logMessage(LOU_LOG_ERROR,
								"%s: error: Base character %s can not have the %s "
								"attribute.",
								printSource(NULL, character->sourceFile,
										character->sourceLine),
								_lou_showString(&basechar->value, 1, 0),
								attributeName != NULL ? attributeName : "?");
						errorCount++;
						free(attributeName);
						return 0;
					}
				}
				character->mode = mode;
				character->basechar = basecharOffset;
				// add mode to attributes
				character->attributes |= character->mode;
				if (character->attributes & (CTC_UpperCase | CTC_LowerCase))
					character->attributes |= CTC_Letter;
				// also set the new attributes on the associated dots of the base
				// character
				TranslationTableRule *defRule =
						(TranslationTableRule *)&table
								->ruleArea[basechar->definitionRule];
				if (defRule->dotslen == 1) {
					TranslationTableCharacter *dots =
							getDots(defRule->charsdots[defRule->charslen], table);
					if (dots) {
						dots->attributes |= character->mode;
						if (dots->attributes & (CTC_UpperCase | CTC_LowerCase))
							dots->attributes |= CTC_Letter;
					}
				}
				// store all characters that are based on a base character in list
				if (basechar->linked) character->linked = basechar->linked;
				basechar->linked = characterOffset;
			}
			characterOffset = character->next;
		}
	}
	// add noletsign rules from single-letter word and largesign rules
	for (int i = 0; i < HASHNUM; i++) {
		TranslationTableOffset characterOffset = table->characters[i];
		while (characterOffset) {
			TranslationTableCharacter *character =
					(TranslationTableCharacter *)&table->ruleArea[characterOffset];
			if (character->attributes & CTC_Letter) {
				TranslationTableOffset *otherRule = &character->otherRules;
				while (*otherRule) {
					TranslationTableRule *rule =
							(TranslationTableRule *)&table->ruleArea[*otherRule];
					if (rule->opcode == CTO_WholeWord || rule->opcode == CTO_LargeSign)
						if (table->noLetsignCount < LETSIGNSIZE)
							table->noLetsign[table->noLetsignCount++] =
									rule->charsdots[0];
					otherRule = &rule->charsnext;
				}
			}
			characterOffset = character->next;
		}
	}
	table->finalized = 1;
	return 1;
}

static int
compileString(const char *inString, TranslationTableHeader **table,
		DisplayTableHeader **displayTable) {
	/* This function can be used to make changes to tables on the fly. */
	int k;
	FileInfo file;
	if (inString == NULL) return 0;
	memset(&file, 0, sizeof(file));
	file.fileName = inString;
	file.encoding = noEncoding;
	file.lineNumber = 1;
	file.status = 0;
	file.linepos = 0;
	for (k = 0; inString[k]; k++) file.line[k] = inString[k];
	file.line[k] = 0;
	file.linelen = k;
	if (table && *table && (*table)->finalized) {
		compileError(&file, "Table is finalized");
		return 0;
	}
	return compileRule(&file, table, displayTable, NULL);
}

static int
setDefaults(TranslationTableHeader *table) {
	for (int i = 0; i < 3; i++)
		if (!table->emphRules[i][lenPhraseOffset])
			table->emphRules[i][lenPhraseOffset] = 4;
	if (table->numPasses == 0) table->numPasses = 1;
	return 1;
}

/* =============== *
 * TABLE RESOLVING *
 * =============== *
 *
 * A table resolver is a function that resolves a `tableList` path against a
 * `base` path, and returns the resolved table(s) as a list of absolute file
 * paths.
 *
 * The function must have the following signature:
 *
 *     char ** (const char * tableList, const char * base)
 *
 * In general, `tableList` is a path in the broad sense. The default
 * implementation accepts only *file* paths. But another implementation could
 * for instance handle URI's. `base` is always a file path however.
 *
 * The idea is to give other programs that use liblouis the ability to define
 * their own table resolver (in C, Java, Python, etc.) when the default
 * resolver is not satisfying. (see also lou_registerTableResolver)
 *
 */

/**
 * Resolve a single (sub)table.
 *
 * Tries to resolve `table` against `base` if base is an absolute path. If
 * that fails, searches `searchPath`.
 *
 */
static char *
resolveSubtable(const char *table, const char *base, const char *searchPath) {
	char *tableFile;
	static struct stat info;

	if (table == NULL || table[0] == '\0') return NULL;
	tableFile = (char *)malloc(MAXSTRING * sizeof(char) * 2);

	//
	// First try to resolve against base
	//
	if (base) {
		int k;
		strcpy(tableFile, base);
		k = (int)strlen(tableFile);
		while (k >= 0 && tableFile[k] != '/' && tableFile[k] != '\\') k--;
		tableFile[++k] = '\0';
		strcat(tableFile, table);
		if (stat(tableFile, &info) == 0 && !(info.st_mode & S_IFDIR)) {
			_lou_logMessage(LOU_LOG_DEBUG, "found table %s", tableFile);
			return tableFile;
		}
	}

	//
	// It could be an absolute path, or a path relative to the current working
	// directory
	//
	strcpy(tableFile, table);
	if (stat(tableFile, &info) == 0 && !(info.st_mode & S_IFDIR)) {
		_lou_logMessage(LOU_LOG_DEBUG, "found table %s", tableFile);
		return tableFile;
	}

	//
	// Then search `LOUIS_TABLEPATH`, `dataPath` and `programPath`
	//
	if (searchPath[0] != '\0') {
		char *dir;
		int last;
		char *cp;
		char *searchPath_copy = strdup(searchPath);
		for (dir = searchPath_copy;; dir = cp + 1) {
			for (cp = dir; *cp != '\0' && *cp != ','; cp++)
				;
			last = (*cp == '\0');
			*cp = '\0';
			if (dir == cp) dir = ".";
			sprintf(tableFile, "%s%c%s", dir, DIR_SEP, table);
			if (stat(tableFile, &info) == 0 && !(info.st_mode & S_IFDIR)) {
				_lou_logMessage(LOU_LOG_DEBUG, "found table %s", tableFile);
				free(searchPath_copy);
				return tableFile;
			}
			if (last) break;
			sprintf(tableFile, "%s%c%s%c%s%c%s", dir, DIR_SEP, "liblouis", DIR_SEP,
					"tables", DIR_SEP, table);
			if (stat(tableFile, &info) == 0 && !(info.st_mode & S_IFDIR)) {
				_lou_logMessage(LOU_LOG_DEBUG, "found table %s", tableFile);
				free(searchPath_copy);
				return tableFile;
			}
			if (last) break;
		}
		free(searchPath_copy);
	}
	free(tableFile);
	return NULL;
}

char *EXPORT_CALL
_lou_getTablePath(void) {
	char searchPath[MAXSTRING];
	char *path;
	char *cp;
	int envset = 0;
	cp = searchPath;
	path = getenv("LOUIS_TABLEPATH");
	if (path != NULL && path[0] != '\0') {
		envset = 1;
		cp += sprintf(cp, ",%s", path);
	}
	path = lou_getDataPath();
	if (path != NULL && path[0] != '\0')
		cp += sprintf(cp, ",%s%c%s%c%s", path, DIR_SEP, "liblouis", DIR_SEP, "tables");
	if (!envset) {
#ifdef _WIN32
		path = lou_getProgramPath();
		if (path != NULL) {
			if (path[0] != '\0')
				cp += sprintf(cp, ",%s%s", path, "\\share\\liblouis\\tables");
			free(path);
		}
#else
		cp += sprintf(cp, ",%s", TABLESDIR);
#endif
	}
	if (searchPath[0] != '\0')
		return strdup(&searchPath[1]);
	else
		return strdup(".");
}

/**
 * The default table resolver
 *
 * Tries to resolve tableList against base. The search path is set to
 * `LOUIS_TABLEPATH`, `dataPath` and `programPath` (in that order).
 *
 * @param table A file path, may be absolute or relative. May be a list of
 *              tables separated by comma's. In that case, the first table
 *              is used as the base for the other subtables.
 * @param base A file path or directory path, or NULL.
 * @return The file paths of the resolved subtables, or NULL if the table
 *         could not be resolved.
 *
 */
char **EXPORT_CALL
_lou_defaultTableResolver(const char *tableList, const char *base) {
	char *searchPath;
	char **tableFiles;
	char *subTable;
	char *tableList_copy;
	char *cp;
	int last;
	int k;

	/* Set up search path */
	searchPath = _lou_getTablePath();

	/* Count number of subtables in table list */
	k = 0;
	for (cp = (char *)tableList; *cp != '\0'; cp++)
		if (*cp == ',') k++;
	tableFiles = (char **)calloc(k + 2, sizeof(char *));
	if (!tableFiles) _lou_outOfMemory();

	/* Resolve subtables */
	k = 0;
	tableList_copy = strdup(tableList);
	for (subTable = tableList_copy;; subTable = cp + 1) {
		for (cp = subTable; *cp != '\0' && *cp != ','; cp++)
			;
		last = (*cp == '\0');
		*cp = '\0';
		if (!(tableFiles[k++] = resolveSubtable(subTable, base, searchPath))) {
			char *path;
			_lou_logMessage(LOU_LOG_ERROR, "Cannot resolve table '%s'", subTable);
			path = getenv("LOUIS_TABLEPATH");
			if (path != NULL && path[0] != '\0')
				_lou_logMessage(LOU_LOG_ERROR, "LOUIS_TABLEPATH=%s", path);
			free(searchPath);
			free(tableList_copy);
			free_tablefiles(tableFiles);
			return NULL;
		}
		if (k == 1) base = subTable;
		if (last) break;
	}
	free(searchPath);
	free(tableList_copy);
	tableFiles[k] = NULL;
	return tableFiles;
}

static char **(EXPORT_CALL *tableResolver)(
		const char *tableList, const char *base) = &_lou_defaultTableResolver;

static char **
copyStringArray(char **array) {
	int len;
	char **copy;
	if (!array) return NULL;
	len = 0;
	while (array[len]) len++;
	copy = malloc((len + 1) * sizeof(char *));
	copy[len] = NULL;
	while (len) {
		len--;
		copy[len] = strdup(array[len]);
	}
	return copy;
}

char **EXPORT_CALL
_lou_resolveTable(const char *tableList, const char *base) {
	char **tableFiles = (*tableResolver)(tableList, base);
	char **result = copyStringArray(tableFiles);
	if (tableResolver == &_lou_defaultTableResolver) free_tablefiles(tableFiles);
	return result;
}

/**
 * Register a new table resolver. Overrides the default resolver.
 *
 * @param resolver The new resolver as a function pointer.
 *
 */
void EXPORT_CALL
lou_registerTableResolver(
		char **(EXPORT_CALL *resolver)(const char *tableList, const char *base)) {
	tableResolver = resolver;
}

static int fileCount = 0;

/**
 * Compile a single file
 *
 */
static int
compileFile(const char *fileName, TranslationTableHeader **table,
		DisplayTableHeader **displayTable) {
	FileInfo file;
	fileCount++;
	file.fileName = fileName;
	if (table) {
		int i;
		for (i = 0; (*table)->sourceFiles[i]; i++)
			;
		if (i >= MAX_SOURCE_FILES) {
			_lou_logMessage(LOU_LOG_WARN, "Max number of source files (%i) reached",
					MAX_SOURCE_FILES);
			file.sourceFile = NULL;
		} else {
			file.sourceFile = (*table)->sourceFiles[i] = strdup(fileName);
		}
	}
	file.encoding = noEncoding;
	file.status = 0;
	file.lineNumber = 0;
	if ((file.in = fopen(file.fileName, "rb"))) {
		// the scope of a macro is the current file (after the macro definition)
		const MacroList *inscopeMacros = NULL;
		while (_lou_getALine(&file))
			if (!compileRule(&file, table, displayTable, &inscopeMacros)) {
				if (!errorCount) compileError(&file, "Rule could not be compiled");
				break;
			}
		fclose(file.in);
		free_macro_list(inscopeMacros);
	} else {
		_lou_logMessage(LOU_LOG_ERROR, "Cannot open table '%s'", file.fileName);
		errorCount++;
	}
	return !errorCount;
}

static void
freeTranslationTable(TranslationTableHeader *t) {
	for (int i = 0; i < MAX_EMPH_CLASSES && t->emphClassNames[i]; i++)
		free(t->emphClassNames[i]);
	for (int i = 0; t->sourceFiles[i]; i++) free(t->sourceFiles[i]);
	if (t->characterClasses) deallocateCharacterClasses(t);
	if (t->ruleNames) deallocateRuleNames(t);
	free(t);
}

/**
 * Free a char** array
 */
static void
free_tablefiles(char **tables) {
	char **table;
	if (!tables) return;
	for (table = tables; *table; table++) free(*table);
	free(tables);
}

/**
 * Implement include opcode
 *
 */
static int
includeFile(const FileInfo *file, CharsString *includedFile,
		TranslationTableHeader **table, DisplayTableHeader **displayTable) {
	int k;
	char includeThis[MAXSTRING];
	char **tableFiles;
	int rv;
	for (k = 0; k < includedFile->length; k++)
		includeThis[k] = (char)includedFile->chars[k];
	if (k >= MAXSTRING) {
		compileError(file, "Include statement too long: 'include %s'", includeThis);
		return 0;
	}
	includeThis[k] = 0;
	tableFiles = _lou_resolveTable(includeThis, file->fileName);
	if (tableFiles == NULL) {
		errorCount++;
		return 0;
	}
	if (tableFiles[1] != NULL) {
		free_tablefiles(tableFiles);
		compileError(file, "Table list not supported in include statement: 'include %s'",
				includeThis);
		return 0;
	}
	rv = compileFile(*tableFiles, table, displayTable);
	free_tablefiles(tableFiles);
	if (!rv)
		_lou_logMessage(LOU_LOG_ERROR, "%s:%d: Error in included file", file->fileName,
				file->lineNumber);
	return rv;
}

/**
 * Compile source tables into a table in memory
 *
 */
static int
compileTable(const char *tableList, const char *displayTableList,
		TranslationTableHeader **translationTable, DisplayTableHeader **displayTable) {
	char **tableFiles;
	char **subTable;
	if (translationTable && !tableList) return 0;
	if (displayTable && !displayTableList) return 0;
	if (!translationTable && !displayTable) return 0;
	if (translationTable) *translationTable = NULL;
	if (displayTable) *displayTable = NULL;
	errorCount = warningCount = fileCount = 0;
	if (!opcodeLengths[0]) {
		TranslationTableOpcode opcode;
		for (opcode = 0; opcode < CTO_None; opcode++)
			opcodeLengths[opcode] = (short)strlen(opcodeNames[opcode]);
	}
	if (translationTable) allocateTranslationTable(NULL, translationTable);
	if (displayTable) allocateDisplayTable(NULL, displayTable);

	if (translationTable) {
		(*translationTable)->emphClassNames[0] = NULL;
		(*translationTable)->characterClasses = NULL;
		(*translationTable)->ruleNames = NULL;
	}

	/* Compile things that are necesary for the proper operation of
	 * liblouis or liblouisxml or liblouisutdml */
	/* TODO: These definitions seem to be necessary for proper functioning of
	   liblouisutdml. Find a way to satisfy those requirements without hard coding
	   some characters in every table notably behind the users back */
	compileString("space \\xffff 123456789abcdef LOU_ENDSEGMENT", translationTable,
			displayTable);

	if (displayTable && translationTable && strcmp(tableList, displayTableList) == 0) {
		/* Compile the display and translation tables in one go */

		/* Compile all subtables in the list */
		if (!(tableFiles = _lou_resolveTable(tableList, NULL))) {
			errorCount++;
			goto cleanup;
		}
		for (subTable = tableFiles; *subTable; subTable++)
			if (!compileFile(*subTable, translationTable, displayTable)) goto cleanup;
	} else {
		/* Compile the display and translation tables separately */

		if (displayTable) {
			if (!(tableFiles = _lou_resolveTable(displayTableList, NULL))) {
				errorCount++;
				goto cleanup;
			}
			for (subTable = tableFiles; *subTable; subTable++)
				if (!compileFile(*subTable, NULL, displayTable)) goto cleanup;
			free_tablefiles(tableFiles);
			tableFiles = NULL;
		}
		if (translationTable) {
			if (!(tableFiles = _lou_resolveTable(tableList, NULL))) {
				errorCount++;
				goto cleanup;
			}
			for (subTable = tableFiles; *subTable; subTable++)
				if (!compileFile(*subTable, translationTable, NULL)) goto cleanup;
		}
	}

/* Clean up after compiling files */
cleanup:
	free_tablefiles(tableFiles);
	if (warningCount) _lou_logMessage(LOU_LOG_WARN, "%d warnings issued", warningCount);
	if (!errorCount) {
		if (translationTable) setDefaults(*translationTable);
		return 1;
	} else {
		_lou_logMessage(LOU_LOG_ERROR, "%d errors found.", errorCount);
		if (translationTable) {
			if (*translationTable) freeTranslationTable(*translationTable);
			*translationTable = NULL;
		}
		if (displayTable) {
			if (*displayTable) free(*displayTable);
			*displayTable = NULL;
		}
		return 0;
	}
}

/* Return the emphasis classes declared in tableList. */
char const **EXPORT_CALL
lou_getEmphClasses(const char *tableList) {
	const char *names[MAX_EMPH_CLASSES + 1];
	unsigned int count = 0;
	const TranslationTableHeader *table = _lou_getTranslationTable(tableList);
	if (!table) return NULL;

	while (count < MAX_EMPH_CLASSES) {
		char const *name = table->emphClassNames[count];
		if (!name) break;
		names[count++] = name;
	}
	names[count++] = NULL;

	{
		unsigned int size = count * sizeof(names[0]);
		char const **result = malloc(size);
		if (!result) return NULL;
		/* The void* cast is necessary to stop MSVC from warning about
		 * different 'const' qualifiers (C4090). */
		memcpy((void *)result, names, size);
		return result;
	}
}

void
getTable(const char *tableList, const char *displayTableList,
		TranslationTableHeader **translationTable, DisplayTableHeader **displayTable);

void EXPORT_CALL
_lou_getTable(const char *tableList, const char *displayTableList,
		const TranslationTableHeader **translationTable,
		const DisplayTableHeader **displayTable) {
	TranslationTableHeader *newTable;
	DisplayTableHeader *newDisplayTable;
	getTable(tableList, displayTableList, &newTable, &newDisplayTable);
	if (newTable)
		if (!finalizeTable(newTable)) newTable = NULL;
	*translationTable = newTable;
	*displayTable = newDisplayTable;
}

/* Checks and loads tableList. */
const void *EXPORT_CALL
lou_getTable(const char *tableList) {
	const TranslationTableHeader *table;
	const DisplayTableHeader *displayTable;
	_lou_getTable(tableList, tableList, &table, &displayTable);
	if (!table || !displayTable) return NULL;
	return table;
}

const TranslationTableHeader *EXPORT_CALL
_lou_getTranslationTable(const char *tableList) {
	TranslationTableHeader *table;
	getTable(tableList, NULL, &table, NULL);
	if (table)
		if (!finalizeTable(table)) table = NULL;
	return table;
}

const DisplayTableHeader *EXPORT_CALL
_lou_getDisplayTable(const char *tableList) {
	DisplayTableHeader *table;
	getTable(NULL, tableList, NULL, &table);
	return table;
}

void
getTable(const char *translationTableList, const char *displayTableList,
		TranslationTableHeader **translationTable, DisplayTableHeader **displayTable) {
	/* Keep track of which tables have already been compiled */
	int translationTableListLen, displayTableListLen = 0;
	if (translationTableList == NULL || *translationTableList == 0)
		translationTable = NULL;
	if (displayTableList == NULL || *displayTableList == 0) displayTable = NULL;
	/* See if translation table has already been compiled */
	if (translationTable) {
		translationTableListLen = (int)strlen(translationTableList);
		*translationTable = NULL;
		TranslationTableChainEntry *currentEntry = translationTableChain;
		TranslationTableChainEntry *prevEntry = NULL;
		while (currentEntry != NULL) {
			if (translationTableListLen == currentEntry->tableListLength &&
					(memcmp(&currentEntry->tableList[0], translationTableList,
							translationTableListLen)) == 0) {
				/* Move the table to the top of the table chain. */
				if (prevEntry != NULL) {
					prevEntry->next = currentEntry->next;
					currentEntry->next = translationTableChain;
					translationTableChain = currentEntry;
				}
				*translationTable = currentEntry->table;
				break;
			}
			prevEntry = currentEntry;
			currentEntry = currentEntry->next;
		}
	}
	/* See if display table has already been compiled */
	if (displayTable) {
		displayTableListLen = (int)strlen(displayTableList);
		*displayTable = NULL;
		DisplayTableChainEntry *currentEntry = displayTableChain;
		DisplayTableChainEntry *prevEntry = NULL;
		while (currentEntry != NULL) {
			if (displayTableListLen == currentEntry->tableListLength &&
					(memcmp(&currentEntry->tableList[0], displayTableList,
							displayTableListLen)) == 0) {
				/* Move the table to the top of the table chain. */
				if (prevEntry != NULL) {
					prevEntry->next = currentEntry->next;
					currentEntry->next = displayTableChain;
					displayTableChain = currentEntry;
				}
				*displayTable = currentEntry->table;
				break;
			}
			prevEntry = currentEntry;
			currentEntry = currentEntry->next;
		}
	}
	if ((translationTable && *translationTable == NULL) ||
			(displayTable && *displayTable == NULL)) {
		TranslationTableHeader *newTranslationTable = NULL;
		DisplayTableHeader *newDisplayTable = NULL;
		if (compileTable(translationTableList, displayTableList,
					(translationTable && *translationTable == NULL) ? &newTranslationTable
																	: NULL,
					(displayTable && *displayTable == NULL) ? &newDisplayTable : NULL)) {
			/* Add a new entry to the top of the table chain. */
			if (newTranslationTable != NULL) {
				int entrySize =
						sizeof(TranslationTableChainEntry) + translationTableListLen;
				TranslationTableChainEntry *newEntry = malloc(entrySize);
				if (!newEntry) _lou_outOfMemory();
				newEntry->next = translationTableChain;
				newEntry->table = newTranslationTable;
				newEntry->tableListLength = translationTableListLen;
				memcpy(&newEntry->tableList[0], translationTableList,
						translationTableListLen);
				translationTableChain = newEntry;
				*translationTable = newTranslationTable;
			}
			if (newDisplayTable != NULL) {
				int entrySize = sizeof(DisplayTableChainEntry) + displayTableListLen;
				DisplayTableChainEntry *newEntry = malloc(entrySize);
				if (!newEntry) _lou_outOfMemory();
				newEntry->next = displayTableChain;
				newEntry->table = newDisplayTable;
				newEntry->tableListLength = displayTableListLen;
				memcpy(&newEntry->tableList[0], displayTableList, displayTableListLen);
				displayTableChain = newEntry;
				*displayTable = newDisplayTable;
			}
		} else {
			_lou_logMessage(
					LOU_LOG_ERROR, "%s could not be compiled", translationTableList);
			return;
		}
	}
}

int EXPORT_CALL
lou_checkTable(const char *tableList) {
	if (lou_getTable(tableList)) return 1;
	return 0;
}

formtype EXPORT_CALL
lou_getTypeformForEmphClass(const char *tableList, const char *emphClass) {
	const TranslationTableHeader *table = _lou_getTranslationTable(tableList);
	if (!table) return 0;
	for (int i = 0; i < MAX_EMPH_CLASSES && table->emphClassNames[i]; i++)
		if (strcmp(emphClass, table->emphClassNames[i]) == 0) return italic << i;
	return 0;
}

static unsigned char *destSpacing = NULL;
static int sizeDestSpacing = 0;
static formtype *typebuf = NULL;
static unsigned int *wordBuffer = NULL;
static EmphasisInfo *emphasisBuffer = NULL;
static int sizeTypebuf = 0;
static widechar *passbuf[MAXPASSBUF] = { NULL };
static int sizePassbuf[MAXPASSBUF] = { 0 };
static int *posMapping1 = NULL;
static int sizePosMapping1 = 0;
static int *posMapping2 = NULL;
static int sizePosMapping2 = 0;
static int *posMapping3 = NULL;
static int sizePosMapping3 = 0;
void *EXPORT_CALL
_lou_allocMem(AllocBuf buffer, int index, int srcmax, int destmax) {
	if (srcmax < 1024) srcmax = 1024;
	if (destmax < 1024) destmax = 1024;
	switch (buffer) {
	case alloc_typebuf:
		if (destmax > sizeTypebuf) {
			if (typebuf != NULL) free(typebuf);
			// TODO: should this be srcmax?
			typebuf = malloc((destmax + 4) * sizeof(formtype));
			if (!typebuf) _lou_outOfMemory();
			sizeTypebuf = destmax;
		}
		return typebuf;

	case alloc_wordBuffer:

		if (wordBuffer != NULL) free(wordBuffer);
		wordBuffer = calloc(srcmax + 4, sizeof(unsigned int));
		if (wordBuffer == NULL) _lou_outOfMemory();
		return wordBuffer;

	case alloc_emphasisBuffer:

		if (emphasisBuffer != NULL) free(emphasisBuffer);
		emphasisBuffer = calloc(srcmax + 4, sizeof(EmphasisInfo));
		if (emphasisBuffer == NULL) _lou_outOfMemory();
		return emphasisBuffer;

	case alloc_destSpacing:
		if (destmax > sizeDestSpacing) {
			if (destSpacing != NULL) free(destSpacing);
			destSpacing = malloc(destmax + 4);
			if (!destSpacing) _lou_outOfMemory();
			sizeDestSpacing = destmax;
		}
		return destSpacing;
	case alloc_passbuf:
		if (index < 0 || index >= MAXPASSBUF) {
			_lou_logMessage(LOU_LOG_FATAL, "Index out of bounds: %d\n", index);
			exit(3);
		}
		if (destmax > sizePassbuf[index]) {
			if (passbuf[index] != NULL) free(passbuf[index]);
			passbuf[index] = malloc((destmax + 4) * CHARSIZE);
			if (!passbuf[index]) _lou_outOfMemory();
			sizePassbuf[index] = destmax;
		}
		return passbuf[index];
	case alloc_posMapping1: {
		int mapSize;
		if (srcmax >= destmax)
			mapSize = srcmax;
		else
			mapSize = destmax;
		if (mapSize > sizePosMapping1) {
			if (posMapping1 != NULL) free(posMapping1);
			posMapping1 = malloc((mapSize + 4) * sizeof(int));
			if (!posMapping1) _lou_outOfMemory();
			sizePosMapping1 = mapSize;
		}
	}
		return posMapping1;
	case alloc_posMapping2: {
		int mapSize;
		if (srcmax >= destmax)
			mapSize = srcmax;
		else
			mapSize = destmax;
		if (mapSize > sizePosMapping2) {
			if (posMapping2 != NULL) free(posMapping2);
			posMapping2 = malloc((mapSize + 4) * sizeof(int));
			if (!posMapping2) _lou_outOfMemory();
			sizePosMapping2 = mapSize;
		}
	}
		return posMapping2;
	case alloc_posMapping3: {
		int mapSize;
		if (srcmax >= destmax)
			mapSize = srcmax;
		else
			mapSize = destmax;
		if (mapSize > sizePosMapping3) {
			if (posMapping3 != NULL) free(posMapping3);
			posMapping3 = malloc((mapSize + 4) * sizeof(int));
			if (!posMapping3) _lou_outOfMemory();
			sizePosMapping3 = mapSize;
		}
	}
		return posMapping3;
	default:
		return NULL;
	}
}

void EXPORT_CALL
lou_free(void) {
	TranslationTableChainEntry *currentEntry;
	TranslationTableChainEntry *previousEntry;
	lou_logEnd();
	if (translationTableChain != NULL) {
		currentEntry = translationTableChain;
		while (currentEntry) {
			freeTranslationTable(currentEntry->table);
			previousEntry = currentEntry;
			currentEntry = currentEntry->next;
			free(previousEntry);
		}
		translationTableChain = NULL;
	}
	if (typebuf != NULL) free(typebuf);
	typebuf = NULL;
	if (wordBuffer != NULL) free(wordBuffer);
	wordBuffer = NULL;
	if (emphasisBuffer != NULL) free(emphasisBuffer);
	emphasisBuffer = NULL;
	sizeTypebuf = 0;
	if (destSpacing != NULL) free(destSpacing);
	destSpacing = NULL;
	sizeDestSpacing = 0;
	{
		int k;
		for (k = 0; k < MAXPASSBUF; k++) {
			if (passbuf[k] != NULL) free(passbuf[k]);
			passbuf[k] = NULL;
			sizePassbuf[k] = 0;
		}
	}
	if (posMapping1 != NULL) free(posMapping1);
	posMapping1 = NULL;
	sizePosMapping1 = 0;
	if (posMapping2 != NULL) free(posMapping2);
	posMapping2 = NULL;
	sizePosMapping2 = 0;
	if (posMapping3 != NULL) free(posMapping3);
	posMapping3 = NULL;
	sizePosMapping3 = 0;
	opcodeLengths[0] = 0;
}

const char *EXPORT_CALL
lou_version(void) {
	static const char *version = PACKAGE_VERSION;
	return version;
}

int EXPORT_CALL
lou_charSize(void) {
	return CHARSIZE;
}

int EXPORT_CALL
lou_compileString(const char *tableList, const char *inString) {
	TranslationTableHeader *table;
	DisplayTableHeader *displayTable;
	getTable(tableList, tableList, &table, &displayTable);
	if (!table) return 0;
	if (!compileString(inString, &table, &displayTable)) return 0;
	return 1;
}

int EXPORT_CALL
_lou_compileTranslationRule(const char *tableList, const char *inString) {
	TranslationTableHeader *table;
	getTable(tableList, NULL, &table, NULL);
	return compileString(inString, &table, NULL);
}

int EXPORT_CALL
_lou_compileDisplayRule(const char *tableList, const char *inString) {
	DisplayTableHeader *table;
	getTable(NULL, tableList, NULL, &table);
	return compileString(inString, NULL, &table);
}

/**
 * This procedure provides a target for cals that serve as breakpoints
 * for gdb.
 */
// char *EXPORT_CALL
// lou_getTablePaths (void)
// {
//   static char paths[MAXSTRING];
//   static char scratchBuf[MAXSTRING];
//   char *pathList;
//   strcpy (paths, tablePath);
//   strcat (paths, ",");
//   pathList = getenv ("LOUIS_TABLEPATH");
//   if (pathList)
//     {
//       strcat (paths, pathList);
//       strcat (paths, ",");
//     }
//   pathList = getcwd (scratchBuf, MAXSTRING);
//   if (pathList)
//     {
//       strcat (paths, pathList);
//       strcat (paths, ",");
//     }
//   pathList = lou_getDataPath ();
//   if (pathList)
//     {
//       strcat (paths, pathList);
//       strcat (paths, ",");
//     }
// #ifdef _WIN32
//   strcpy (paths, lou_getProgramPath ());
//   strcat (paths, "\\share\\liblouss\\tables\\");
// #else
//   strcpy (paths, TABLESDIR);
// #endif
//   return paths;
// }
