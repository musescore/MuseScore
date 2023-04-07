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
 * @brief Translate to braille
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"

/* additional bits in typebuf */
#define SYLLABLE_MARKER_1 0x2000
#define SYLLABLE_MARKER_2 0x4000
#define CAPSEMPH 0x8000

#define EMPHASIS 0x3fff	 // all typeform bits that can be used

/* bits for wordBuffer */
#define WORD_CHAR 0x00000001
#define WORD_RESET 0x00000002
#define WORD_WHOLE 0x00000004

typedef struct {
	int size;
	widechar **buffers;
	int *inUse;
	widechar *(*alloc)(int index, int length);
	void (*free)(widechar *);
} StringBufferPool;

static widechar *
allocStringBuffer(int index, int length) {
	return _lou_allocMem(alloc_passbuf, index, 0, length);
}

static const StringBufferPool *stringBufferPool = NULL;

static void
initStringBufferPool() {
	static widechar *stringBuffers[MAXPASSBUF] = { NULL };
	static int stringBuffersInUse[MAXPASSBUF] = { 0 };
	StringBufferPool *pool = malloc(sizeof(StringBufferPool));
	pool->size = MAXPASSBUF;
	pool->buffers = stringBuffers;
	pool->inUse = stringBuffersInUse;
	pool->alloc = &allocStringBuffer;
	pool->free = NULL;
	stringBufferPool = pool;
}

static int
getStringBuffer(int length) {
	int i;
	for (i = 0; i < stringBufferPool->size; i++) {
		if (!stringBufferPool->inUse[i]) {
			stringBufferPool->buffers[i] = stringBufferPool->alloc(i, length);
			stringBufferPool->inUse[i] = 1;
			return i;
		}
	}
	_lou_outOfMemory();
	return -1;
}

static int
releaseStringBuffer(int idx) {
	if (idx >= 0 && idx < stringBufferPool->size) {
		int inUse = stringBufferPool->inUse[idx];
		if (inUse && stringBufferPool->free)
			stringBufferPool->free(stringBufferPool->buffers[idx]);
		stringBufferPool->inUse[idx] = 0;
		return inUse;
	}
	return 0;
}

typedef struct {
	int bufferIndex;
	const widechar *chars;
	int length;
} InString;

typedef struct {
	int bufferIndex;
	widechar *chars;
	int maxlength;
	int length;
} OutString;

typedef struct {
	int startMatch;
	int startReplace;
	int endReplace;
	int endMatch;
} PassRuleMatch;

static int
putCharacter(widechar c, const TranslationTableHeader *table, int pos,
		const InString *input, OutString *output, int *posMapping, int *cursorPosition,
		int *cursorStatus, int mode);
static int
passDoTest(const TranslationTableHeader *table, int pos, const InString *input,
		int transOpcode, const TranslationTableRule *transRule, int *passCharDots,
		const widechar **passInstructions, int *passIC, PassRuleMatch *match,
		TranslationTableRule **groupingRule, widechar *groupingOp);
static int
passDoAction(const TranslationTableHeader *table, const InString **input,
		OutString *output, int *posMapping, int transOpcode,
		const TranslationTableRule **transRule, int passCharDots,
		const widechar *passInstructions, int passIC, int *pos, PassRuleMatch match,
		int *cursorPosition, int *cursorStatus, TranslationTableRule *groupingRule,
		widechar groupingOp, int mode);

static const TranslationTableRule **appliedRules;
static int maxAppliedRules;
static int appliedRulesCount;

static TranslationTableCharacter *
getChar(widechar c, const TranslationTableHeader *table) {
	static TranslationTableCharacter notFound = { NULL, -1, 0, 0, 0, CTC_Space, 0, 0, 32,
		0, 0 };
	const TranslationTableOffset bucket = table->characters[_lou_charHash(c)];
	TranslationTableOffset offset = bucket;
	while (offset) {
		TranslationTableCharacter *character =
				(TranslationTableCharacter *)&table->ruleArea[offset];
		if (character->value == c) return character;
		offset = character->next;
	}
	notFound.value = c;
	return &notFound;
}

static TranslationTableCharacter *
getDots(widechar c, const TranslationTableHeader *table) {
	static TranslationTableCharacter notFound = { NULL, -1, 0, 0, 0, CTC_Space, 0, 0,
		LOU_DOTS, 0, 0 };
	const TranslationTableOffset bucket = table->dots[_lou_charHash(c)];
	TranslationTableOffset offset = bucket;
	while (offset) {
		TranslationTableCharacter *character =
				(TranslationTableCharacter *)&table->ruleArea[offset];
		if (character->value == c) return character;
		offset = character->next;
	}
	notFound.value = c;
	return &notFound;
}

static int
checkCharAttr(const widechar c, const TranslationTableCharacterAttributes a,
		const TranslationTableHeader *table) {
	return (((getChar(c, table))->attributes & a) ? 1 : 0);
}

static int
checkDotsAttr(const widechar c, const TranslationTableCharacterAttributes a,
		const TranslationTableHeader *table) {
	return (((getDots(c, table))->attributes & a) ? 1 : 0);
}

static int
checkCharAttr_safe(const InString *input, int pos,
		const TranslationTableCharacterAttributes a,
		const TranslationTableHeader *table) {
	return ((pos < input->length) ? checkCharAttr(input->chars[pos], a, table) : 0);
}

static int
findForPassRule(const TranslationTableHeader *table, int pos, int currentPass,
		const InString *input, int *transOpcode, const TranslationTableRule **transRule,
		int *transCharslen, int *passCharDots, widechar const **passInstructions,
		int *passIC, PassRuleMatch *match, TranslationTableRule **groupingRule,
		widechar *groupingOp) {
	int save_transCharslen = *transCharslen;
	const TranslationTableRule *save_transRule = *transRule;
	TranslationTableOpcode save_transOpcode = *transOpcode;
	TranslationTableOffset ruleOffset;
	ruleOffset = table->forPassRules[currentPass];
	*transCharslen = 0;
	while (ruleOffset) {
		*transRule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
		*transOpcode = (*transRule)->opcode;
		if (passDoTest(table, pos, input, *transOpcode, *transRule, passCharDots,
					passInstructions, passIC, match, groupingRule, groupingOp))
			return 1;
		ruleOffset = (*transRule)->charsnext;
	}
	*transCharslen = save_transCharslen;
	*transRule = save_transRule;
	*transOpcode = save_transOpcode;
	return 0;
}

static widechar
toLowercase(
		const TranslationTableHeader *table, const TranslationTableCharacter *character) {
	if (character->mode & CTC_UpperCase) {
		const TranslationTableCharacter *c = character;
		if (c->basechar) c = (TranslationTableCharacter *)&table->ruleArea[c->basechar];
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

static int
compareChars(const widechar *address1, const widechar *address2, int count,
		const TranslationTableHeader *table) {
	int k;
	if (!count) return 0;
	for (k = 0; k < count; k++)
		if (toLowercase(table, getChar(address1[k], table)) !=
				toLowercase(table, getChar(address2[k], table)))
			return 0;
	return 1;
}

static int
makeCorrections(const TranslationTableHeader *table, const InString *input,
		OutString *output, int *posMapping, formtype *typebuf, int *realInlen,
		int *cursorPosition, int *cursorStatus, int mode) {
	int pos;
	int transOpcode;
	const TranslationTableRule *transRule;
	int transCharslen;
	int passCharDots;
	const widechar *passInstructions;
	int passIC; /* Instruction counter */
	PassRuleMatch patternMatch;
	TranslationTableRule *groupingRule;
	widechar groupingOp;
	const InString *origInput = input;
	if (!table->corrections) return 1;
	pos = 0;
	output->length = 0;
	int posIncremented = 1;
	_lou_resetPassVariables();
	while (pos < input->length) {
		int length = input->length - pos;
		int tryThis = 0;
		// check posIncremented to avoid endless loop
		if (!(posIncremented &&
					findForPassRule(table, pos, 0, input, &transOpcode, &transRule,
							&transCharslen, &passCharDots, &passInstructions, &passIC,
							&patternMatch, &groupingRule, &groupingOp)))
			while (tryThis < 3) {
				TranslationTableOffset ruleOffset = 0;
				switch (tryThis) {
				case 0:
					if (!(length >= 2)) break;
					ruleOffset = table->forRules[_lou_stringHash(
							&input->chars[pos], 1, table)];
					break;
				case 1:
					if (!(length >= 1)) break;
					length = 1;
					ruleOffset = getChar(input->chars[pos], table)->otherRules;
					break;
				case 2: /* No rule found */
					transOpcode = CTO_Always;
					ruleOffset = 0;
					break;
				}
				while (ruleOffset) {
					transRule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
					transOpcode = transRule->opcode;
					transCharslen = transRule->charslen;
					if (tryThis == 1 ||
							(transCharslen <= length &&
									compareChars(&transRule->charsdots[0],
											&input->chars[pos], transCharslen, table))) {
						if (posIncremented && transOpcode == CTO_Correct &&
								passDoTest(table, pos, input, transOpcode, transRule,
										&passCharDots, &passInstructions, &passIC,
										&patternMatch, &groupingRule, &groupingOp)) {
							tryThis = 4;
							break;
						}
					}
					ruleOffset = transRule->charsnext;
				}
				tryThis++;
			}
		posIncremented = 1;

		switch (transOpcode) {
		case CTO_Always:
			if (output->length >= output->maxlength) goto failure;
			posMapping[output->length] = pos;
			output->chars[output->length++] = input->chars[pos++];
			break;
		case CTO_Correct: {
			const InString *inputBefore = input;
			int posBefore = pos;
			if (appliedRules != NULL && appliedRulesCount < maxAppliedRules)
				appliedRules[appliedRulesCount++] = transRule;
			if (!passDoAction(table, &input, output, posMapping, transOpcode, &transRule,
						passCharDots, passInstructions, passIC, &pos, patternMatch,
						cursorPosition, cursorStatus, groupingRule, groupingOp, mode))
				goto failure;
			if (input->bufferIndex != inputBefore->bufferIndex &&
					inputBefore->bufferIndex != origInput->bufferIndex)
				releaseStringBuffer(inputBefore->bufferIndex);
			if (pos == posBefore) posIncremented = 0;
			break;
		}
		default:
			break;
		}
	}

	{  // We have to transform typebuf accordingly
		int k;
		formtype *typebuf_temp;
		if ((typebuf_temp = malloc(output->length * sizeof(formtype))) == NULL)
			_lou_outOfMemory();
		for (k = 0; k < output->length; k++)
			// posMapping will never be < 0 but in theory it could
			if (posMapping[k] < 0)
				typebuf_temp[k] = typebuf[0];  // prepend to next
			else if (posMapping[k] >= input->length)
				typebuf_temp[k] = typebuf[input->length - 1];  // append to previous
			else
				typebuf_temp[k] = typebuf[posMapping[k]];
		memcpy(typebuf, typebuf_temp, output->length * sizeof(formtype));
		free(typebuf_temp);
	}

failure:
	*realInlen = pos;
	if (input->bufferIndex != origInput->bufferIndex)
		releaseStringBuffer(input->bufferIndex);
	return 1;
}

static int
matchCurrentInput(
		const InString *input, int pos, const widechar *passInstructions, int passIC) {
	int k;
	int kk = pos;
	for (k = passIC + 2;
			((k < passIC + 2 + passInstructions[passIC + 1]) && (kk < input->length));
			k++)
		if (input->chars[kk] == LOU_ENDSEGMENT ||
				passInstructions[k] != input->chars[kk++])
			return 0;
	return 1;
}

static int
swapTest(int swapIC, int *pos, const TranslationTableHeader *table, const InString *input,
		const widechar *passInstructions) {
	int p = *pos;
	TranslationTableOffset swapRuleOffset;
	TranslationTableRule *swapRule;
	swapRuleOffset = (passInstructions[swapIC + 1] << 16) | passInstructions[swapIC + 2];
	swapRule = (TranslationTableRule *)&table->ruleArea[swapRuleOffset];
	while (p - *pos < passInstructions[swapIC + 3]) {
		int test;
		if (p >= input->length) return 0;
		if (swapRule->opcode == CTO_SwapDd) {
			for (test = 1; test < swapRule->charslen; test += 2) {
				if (input->chars[p] == swapRule->charsdots[test]) break;
			}
		} else {
			for (test = 0; test < swapRule->charslen; test++) {
				if (input->chars[p] == swapRule->charsdots[test]) break;
			}
		}
		if (test >= swapRule->charslen) return 0;
		p++;
	}
	if (passInstructions[swapIC + 3] == passInstructions[swapIC + 4]) {
		*pos = p;
		return 1;
	}
	while (p - *pos < passInstructions[swapIC + 4]) {
		int test;
		if (p >= input->length) {
			*pos = p;
			return 1;
		}
		if (swapRule->opcode == CTO_SwapDd) {
			for (test = 1; test < swapRule->charslen; test += 2) {
				if (input->chars[p] == swapRule->charsdots[test]) break;
			}
		} else {
			for (test = 0; test < swapRule->charslen; test++) {
				if (input->chars[p] == swapRule->charsdots[test]) break;
			}
		}
		if (test >= swapRule->charslen) {
			*pos = p;
			return 1;
		}
		p++;
	}
	*pos = p;
	return 1;
}

static int
swapReplace(int start, int end, const TranslationTableHeader *table,
		const InString *input, OutString *output, int *posMapping,
		const widechar *passInstructions, int passIC) {
	TranslationTableOffset swapRuleOffset;
	TranslationTableRule *swapRule;
	widechar *replacements;
	int p;
	swapRuleOffset = (passInstructions[passIC + 1] << 16) | passInstructions[passIC + 2];
	swapRule = (TranslationTableRule *)&table->ruleArea[swapRuleOffset];
	replacements = &swapRule->charsdots[swapRule->charslen];
	for (p = start; p < end; p++) {
		int rep;
		int test;
		int k;
		if (swapRule->opcode == CTO_SwapDd) {
			// A sequence of dot patterns is encoded as the length of the first dot
			// pattern (single widechar) followed by the contents of the first dot pattern
			// (one widechar per cell) followed by the length of the second dot pattern,
			// etc. See the function `compileSwapDots'. Because the third operand of a
			// swapdd rule can only contain single-cell dot patterns, the elements at
			// index 0, 2, ... are "1" and the elements at index 1, 3, ... are the dot
			// patterns.
			for (test = 0; test * 2 + 1 < swapRule->charslen; test++)
				if (input->chars[p] == swapRule->charsdots[test * 2 + 1]) break;
			if (test * 2 == swapRule->charslen) continue;
		} else {
			for (test = 0; test < swapRule->charslen; test++)
				if (input->chars[p] == swapRule->charsdots[test]) break;
			if (test == swapRule->charslen) continue;
		}
		k = 0;
		for (rep = 0; rep < test; rep++)
			if (swapRule->opcode == CTO_SwapCc)
				k++;
			else
				k += replacements[k];
		if (swapRule->opcode == CTO_SwapCc) {
			if ((output->length + 1) > output->maxlength) return 0;
			posMapping[output->length] = p;
			output->chars[output->length++] = replacements[k];
		} else {
			int l = replacements[k] - 1;
			int d = output->length + l;
			if (d > output->maxlength) return 0;
			while (--d >= output->length) posMapping[d] = p;
			memcpy(&output->chars[output->length], &replacements[k + 1],
					l * sizeof(*output->chars));
			output->length += l;
		}
	}
	return 1;
}

static int
replaceGrouping(const TranslationTableHeader *table, const InString **input,
		OutString *output, int transOpcode, int passCharDots,
		const widechar *passInstructions, int passIC, int startReplace,
		TranslationTableRule *groupingRule, widechar groupingOp) {
	widechar startCharDots = groupingRule->charsdots[2 * passCharDots];
	widechar endCharDots = groupingRule->charsdots[2 * passCharDots + 1];
	int p;
	int level = 0;
	TranslationTableOffset replaceOffset =
			passInstructions[passIC + 1] << 16 | (passInstructions[passIC + 2] & 0xff);
	TranslationTableRule *replaceRule =
			(TranslationTableRule *)&table->ruleArea[replaceOffset];
	widechar replaceStart = replaceRule->charsdots[2 * passCharDots];
	widechar replaceEnd = replaceRule->charsdots[2 * passCharDots + 1];
	if (groupingOp == pass_groupstart) {
		for (p = startReplace + 1; p < (*input)->length; p++) {
			if ((*input)->chars[p] == startCharDots) level--;
			if ((*input)->chars[p] == endCharDots) level++;
			if (level == 1) break;
		}
		if (p == (*input)->length)
			return 0;
		else {
			// Create a new string instead of modifying it. This is slightly less
			// efficient, but makes the code more readable. Grouping is not a much used
			// feature anyway.
			int idx = getStringBuffer((*input)->length);
			widechar *chars = stringBufferPool->buffers[idx];
			memcpy(chars, (*input)->chars, (*input)->length * sizeof(widechar));
			chars[startReplace] = replaceStart;
			chars[p] = replaceEnd;
			static InString stringStore;
			stringStore = (InString){
				.chars = chars, .length = (*input)->length, .bufferIndex = idx
			};
			*input = &stringStore;
		}
	} else {
		if (transOpcode == CTO_Context) {
			startCharDots = groupingRule->charsdots[2];
			endCharDots = groupingRule->charsdots[3];
			replaceStart = replaceRule->charsdots[2];
			replaceEnd = replaceRule->charsdots[3];
		}
		output->chars[output->length] = replaceEnd;
		for (p = output->length - 1; p >= 0; p--) {
			if (output->chars[p] == endCharDots) level--;
			if (output->chars[p] == startCharDots) level++;
			if (level == 1) break;
		}
		if (p < 0) return 0;
		output->chars[p] = replaceStart;
		output->length++;
	}
	return 1;
}

static int
removeGrouping(const InString **input, OutString *output, int passCharDots,
		int startReplace, TranslationTableRule *groupingRule, widechar groupingOp) {
	widechar startCharDots = groupingRule->charsdots[2 * passCharDots];
	widechar endCharDots = groupingRule->charsdots[2 * passCharDots + 1];
	int p;
	int level = 0;
	if (groupingOp == pass_groupstart) {
		for (p = startReplace + 1; p < (*input)->length; p++) {
			if ((*input)->chars[p] == startCharDots) level--;
			if ((*input)->chars[p] == endCharDots) level++;
			if (level == 1) break;
		}
		if (p == (*input)->length)
			return 0;
		else {
			// Create a new string instead of modifying it. This is slightly less
			// efficient, but makes the code more readable. Grouping is not a much used
			// feature anyway.
			int idx = getStringBuffer((*input)->length);
			widechar *chars = stringBufferPool->buffers[idx];
			int len = 0;
			int k;
			for (k = 0; k < (*input)->length; k++) {
				if (k == p) continue;
				chars[len++] = (*input)->chars[k];
			}
			static InString stringStore;
			stringStore = (InString){ .chars = chars, .length = len, .bufferIndex = idx };
			*input = &stringStore;
		}
	} else {
		for (p = output->length - 1; p >= 0; p--) {
			if (output->chars[p] == endCharDots) level--;
			if (output->chars[p] == startCharDots) level++;
			if (level == 1) break;
		}
		if (p < 0) return 0;
		p++;
		for (; p < output->length; p++) output->chars[p - 1] = output->chars[p];
		output->length--;
	}
	return 1;
}

static int
doPassSearch(const TranslationTableHeader *table, const InString *input,
		const TranslationTableRule *transRule, int passCharDots, int pos,
		const widechar *passInstructions, int passIC, int *searchIC, int *searchPos,
		TranslationTableRule *groupingRule, widechar groupingOp) {
	int level = 0;
	int k, kk;
	int notOperator = 0;  // whether next operand should be reversed
	TranslationTableOffset ruleOffset;
	TranslationTableRule *rule;
	TranslationTableCharacterAttributes attributes;
	while (pos < input->length) {
		*searchIC = passIC + 1;
		*searchPos = pos;
		while (*searchIC < transRule->dotslen) {
			int itsTrue = 1;  // whether we have a match or not
			if (*searchPos > input->length) return 0;
			switch (passInstructions[*searchIC]) {
			case pass_lookback:
				*searchPos -= passInstructions[*searchIC + 1];
				if (*searchPos < 0) {
					*searchPos = 0;
					itsTrue = 0;
				}
				*searchIC += 2;
				break;
			case pass_not:
				notOperator = !notOperator;
				(*searchIC)++;
				continue;
			case pass_string:
			case pass_dots:
				kk = *searchPos;
				for (k = *searchIC + 2;
						k < *searchIC + 2 + passInstructions[*searchIC + 1]; k++)
					if (input->chars[kk] == LOU_ENDSEGMENT ||
							passInstructions[k] != input->chars[kk++]) {
						itsTrue = 0;
						break;
					}
				*searchPos += passInstructions[*searchIC + 1];
				*searchIC += passInstructions[*searchIC + 1] + 2;
				break;
			case pass_startReplace:
				(*searchIC)++;
				break;
			case pass_endReplace:
				(*searchIC)++;
				break;
			case pass_attributes:
				attributes = passInstructions[*searchIC + 1];
				attributes <<= 16;
				attributes |= passInstructions[*searchIC + 2];
				attributes <<= 16;
				attributes |= passInstructions[*searchIC + 3];
				attributes <<= 16;
				attributes |= passInstructions[*searchIC + 4];
				for (k = 0; k < passInstructions[*searchIC + 5]; k++) {
					if (input->chars[*searchPos] == LOU_ENDSEGMENT)
						itsTrue = 0;
					else {
						itsTrue = (passCharDots ? getDots(input->chars[(*searchPos)++],
														  table)
												: getChar(input->chars[(*searchPos)++],
														  table))
										  ->attributes &
								attributes;
						if (notOperator) itsTrue = !itsTrue;
					}
					if (!itsTrue) break;
				}
				if (itsTrue) {
					for (k = passInstructions[*searchIC + 5];
							k < passInstructions[*searchIC + 6]; k++) {
						if (input->chars[*searchPos] == LOU_ENDSEGMENT) {
							itsTrue = 0;
							break;
						}
						if (!((passCharDots ? getDots(input->chars[*searchPos], table)
											: getChar(input->chars[*searchPos], table))
											->attributes &
									attributes)) {
							if (!notOperator) break;
						} else if (notOperator)
							break;
						(*searchPos)++;
					}
				}
				notOperator = 0;
				*searchIC += 7;
				break;
			case pass_groupstart:
			case pass_groupend:
				ruleOffset = (passInstructions[*searchIC + 1] << 16) |
						passInstructions[*searchIC + 2];
				rule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
				if (passInstructions[*searchIC] == pass_groupstart)
					itsTrue = (input->chars[*searchPos] ==
									  rule->charsdots[2 * passCharDots])
							? 1
							: 0;
				else
					itsTrue = (input->chars[*searchPos] ==
									  rule->charsdots[2 * passCharDots + 1])
							? 1
							: 0;
				if (groupingRule != NULL && groupingOp == pass_groupstart &&
						rule == groupingRule) {
					if (input->chars[*searchPos] == rule->charsdots[2 * passCharDots])
						level--;
					else if (input->chars[*searchPos] ==
							rule->charsdots[2 * passCharDots + 1])
						level++;
				}
				(*searchPos)++;
				*searchIC += 3;
				break;
			case pass_swap:
				itsTrue = swapTest(*searchIC, searchPos, table, input, passInstructions);
				*searchIC += 5;
				break;
			case pass_endTest:
				if (itsTrue) {
					if ((groupingRule && level == 1) || !groupingRule) return 1;
				}
				*searchIC = transRule->dotslen;
				break;
			default:
				if (_lou_handlePassVariableTest(passInstructions, searchIC, &itsTrue))
					break;
				break;
			}
			if ((!notOperator && !itsTrue) || (notOperator && itsTrue)) break;
			notOperator = 0;
		}
		pos++;
	}
	return 0;
}

static int
passDoTest(const TranslationTableHeader *table, int pos, const InString *input,
		int transOpcode, const TranslationTableRule *transRule, int *passCharDots,
		widechar const **passInstructions, int *passIC, PassRuleMatch *match,
		TranslationTableRule **groupingRule, widechar *groupingOp) {
	int searchIC, searchPos;
	int k;
	int notOperator = 0;  // whether next operand should be reversed
	TranslationTableOffset ruleOffset = 0;
	TranslationTableRule *rule = NULL;
	TranslationTableCharacterAttributes attributes = 0;
	int startMatch = pos;
	int endMatch = pos;
	int startReplace = -1;
	int endReplace = -1;
	*groupingRule = NULL;
	*passInstructions = &transRule->charsdots[transRule->charslen];
	*passIC = 0;
	if (transOpcode == CTO_Context || transOpcode == CTO_Correct)
		*passCharDots = 0;
	else
		*passCharDots = 1;
	while (*passIC < transRule->dotslen) {
		int itsTrue = 1;  // whether we have a match or not
		if (pos > input->length) return 0;
		switch ((*passInstructions)[*passIC]) {
		case pass_first:
			if (pos != 0) itsTrue = 0;
			(*passIC)++;
			break;
		case pass_last:
			if (pos != input->length) itsTrue = 0;
			(*passIC)++;
			break;
		case pass_lookback:
			pos -= (*passInstructions)[*passIC + 1];
			if (pos < 0) {
				searchPos = 0;
				itsTrue = 0;
			}
			*passIC += 2;
			break;
		case pass_not:
			notOperator = !notOperator;
			(*passIC)++;
			continue;
		case pass_string:
		case pass_dots:
			itsTrue = matchCurrentInput(input, pos, *passInstructions, *passIC);
			pos += (*passInstructions)[*passIC + 1];
			*passIC += (*passInstructions)[*passIC + 1] + 2;
			break;
		case pass_startReplace:
			startReplace = pos;
			(*passIC)++;
			break;
		case pass_endReplace:
			endReplace = pos;
			(*passIC)++;
			break;
		case pass_attributes:
			attributes = (*passInstructions)[*passIC + 1];
			attributes <<= 16;
			attributes |= (*passInstructions)[*passIC + 2];
			attributes <<= 16;
			attributes |= (*passInstructions)[*passIC + 3];
			attributes <<= 16;
			attributes |= (*passInstructions)[*passIC + 4];
			for (k = 0; k < (*passInstructions)[*passIC + 5]; k++) {
				if (pos >= input->length) {
					itsTrue = 0;
					break;
				}
				if (input->chars[pos] == LOU_ENDSEGMENT) {
					itsTrue = 0;
					break;
				}
				if (!((*passCharDots ? getDots(input->chars[pos], table)
									 : getChar(input->chars[pos], table))
									->attributes &
							attributes)) {
					if (!notOperator) {
						itsTrue = 0;
						break;
					}
				} else if (notOperator) {
					itsTrue = 0;
					break;
				}
				pos++;
			}
			if (itsTrue) {
				for (k = (*passInstructions)[*passIC + 5];
						k < (*passInstructions)[*passIC + 6] && pos < input->length;
						k++) {
					if (input->chars[pos] == LOU_ENDSEGMENT) {
						itsTrue = 0;
						break;
					}
					if (!((*passCharDots ? getDots(input->chars[pos], table)
										 : getChar(input->chars[pos], table))
										->attributes &
								attributes)) {
						if (!notOperator) break;
					} else if (notOperator)
						break;
					pos++;
				}
			}
			notOperator = 0;
			*passIC += 7;
			break;
		case pass_groupstart:
		case pass_groupend:
			ruleOffset = ((*passInstructions)[*passIC + 1] << 16) |
					(*passInstructions)[*passIC + 2];
			rule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
			if (*passIC == 0 ||
					(*passIC > 0 &&
							(*passInstructions)[*passIC - 1] == pass_startReplace)) {
				*groupingRule = rule;
				*groupingOp = (*passInstructions)[*passIC];
			}
			if ((*passInstructions)[*passIC] == pass_groupstart)
				itsTrue =
						(input->chars[pos] == rule->charsdots[2 * *passCharDots]) ? 1 : 0;
			else
				itsTrue = (input->chars[pos] == rule->charsdots[2 * *passCharDots + 1])
						? 1
						: 0;
			pos++;
			*passIC += 3;
			break;
		case pass_swap:
			itsTrue = swapTest(*passIC, &pos, table, input, *passInstructions);
			*passIC += 5;
			break;
		case pass_search:
			itsTrue = doPassSearch(table, input, transRule, *passCharDots, pos,
					*passInstructions, *passIC, &searchIC, &searchPos, *groupingRule,
					*groupingOp);
			if ((!notOperator && !itsTrue) || (notOperator && itsTrue)) return 0;
			*passIC = searchIC;
			pos = searchPos;
		case pass_endTest:
			(*passIC)++;
			endMatch = pos;
			if (startReplace == -1) {
				startReplace = startMatch;
				endReplace = endMatch;
			}
			if (startReplace < startMatch)
				return 0;
			else {
				*match = (PassRuleMatch){ .startMatch = startMatch,
					.startReplace = startReplace,
					.endReplace = endReplace,
					.endMatch = endMatch };
				return 1;
			}
			break;
		default:
			if (_lou_handlePassVariableTest(*passInstructions, passIC, &itsTrue)) break;
			return 0;
		}
		if ((!notOperator && !itsTrue) || (notOperator && itsTrue)) return 0;
		notOperator = 0;
	}
	return 0;
}

static int
copyCharacters(int from, int to, const TranslationTableHeader *table,
		const InString *input, OutString *output, int *posMapping, int transOpcode,
		int *cursorPosition, int *cursorStatus, int mode) {
	if (transOpcode == CTO_Context) {
		while (from < to) {
			if (!putCharacter(input->chars[from], table, from, input, output, posMapping,
						cursorPosition, cursorStatus, mode))
				return 0;
			from++;
		}
	} else {
		if (to > from) {
			if ((output->length + to - from) > output->maxlength) return 0;
			while (to > from) {
				posMapping[output->length] = from;
				output->chars[output->length] = input->chars[from];
				output->length++;
				from++;
			}
		}
	}

	return 1;
}

static int
passDoAction(const TranslationTableHeader *table, const InString **input,
		OutString *output, int *posMapping, int transOpcode,
		const TranslationTableRule **transRule, int passCharDots,
		const widechar *passInstructions, int passIC, int *pos, PassRuleMatch match,
		int *cursorPosition, int *cursorStatus, TranslationTableRule *groupingRule,
		widechar groupingOp, int mode) {
	int k;
	TranslationTableOffset ruleOffset = 0;
	TranslationTableRule *rule = NULL;
	int destStartMatch = output->length;
	int destStartReplace;
	int newPos = match.endReplace;

	if (!copyCharacters(match.startMatch, match.startReplace, table, *input, output,
				posMapping, transOpcode, cursorPosition, cursorStatus, mode))
		return 0;
	destStartReplace = output->length;

	while (passIC < (*transRule)->dotslen) switch (passInstructions[passIC]) {
		case pass_string:
		case pass_dots:
			if ((output->length + passInstructions[passIC + 1]) > output->maxlength)
				return 0;
			for (k = 0; k < passInstructions[passIC + 1]; ++k)
				posMapping[output->length + k] = match.startReplace;
			memcpy(&output->chars[output->length], &passInstructions[passIC + 2],
					passInstructions[passIC + 1] * CHARSIZE);
			output->length += passInstructions[passIC + 1];
			passIC += passInstructions[passIC + 1] + 2;
			break;
		case pass_groupstart:
			ruleOffset =
					(passInstructions[passIC + 1] << 16) | passInstructions[passIC + 2];
			rule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
			posMapping[output->length] = match.startMatch;
			output->chars[output->length++] = rule->charsdots[2 * passCharDots];
			passIC += 3;
			break;
		case pass_groupend:
			ruleOffset =
					(passInstructions[passIC + 1] << 16) | passInstructions[passIC + 2];
			rule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
			posMapping[output->length] = match.startMatch;
			output->chars[output->length++] = rule->charsdots[2 * passCharDots + 1];
			passIC += 3;
			break;
		case pass_swap:
			if (!swapReplace(match.startReplace, match.endReplace, table, *input, output,
						posMapping, passInstructions, passIC))
				return 0;
			passIC += 3;
			break;
		case pass_groupreplace:
			if (!groupingRule ||
					!replaceGrouping(table, input, output, transOpcode, passCharDots,
							passInstructions, passIC, match.startReplace, groupingRule,
							groupingOp))
				return 0;
			passIC += 3;
			break;
		case pass_omit:
			if (groupingRule)
				removeGrouping(input, output, passCharDots, match.startReplace,
						groupingRule, groupingOp);
			passIC++;
			break;
		case pass_copy: {
			int count = destStartReplace - destStartMatch;
			if (count > 0) {
				memmove(&output->chars[destStartMatch], &output->chars[destStartReplace],
						count * sizeof(*output->chars));
				output->length -= count;
				destStartReplace = destStartMatch;
			}
		}

			if (!copyCharacters(match.startReplace, match.endReplace, table, *input,
						output, posMapping, transOpcode, cursorPosition, cursorStatus,
						mode))
				return 0;
			newPos = match.endMatch;
			passIC++;
			break;
		default:
			if (_lou_handlePassVariableAction(passInstructions, &passIC)) break;
			return 0;
		}
	*pos = newPos;
	return 1;
}

static void
passSelectRule(const TranslationTableHeader *table, int pos, int currentPass,
		const InString *input, int *transOpcode, const TranslationTableRule **transRule,
		int *transCharslen, int *passCharDots, widechar const **passInstructions,
		int *passIC, PassRuleMatch *match, TranslationTableRule **groupingRule,
		widechar *groupingOp) {
	if (!findForPassRule(table, pos, currentPass, input, transOpcode, transRule,
				transCharslen, passCharDots, passInstructions, passIC, match,
				groupingRule, groupingOp)) {
		*transOpcode = CTO_Always;
	}
}

static int
translatePass(const TranslationTableHeader *table, int currentPass, const InString *input,
		OutString *output, int *posMapping, int *realInlen, int *cursorPosition,
		int *cursorStatus, int mode) {
	int pos;
	int transOpcode;
	const TranslationTableRule *transRule;
	int transCharslen;
	int passCharDots;
	const widechar *passInstructions;
	int passIC; /* Instruction counter */
	PassRuleMatch patternMatch;
	TranslationTableRule *groupingRule;
	widechar groupingOp;
	const InString *origInput = input;
	pos = output->length = 0;
	int posIncremented = 1;
	_lou_resetPassVariables();
	while (pos < input->length) { /* the main multipass translation loop */
		// check posIncremented to avoid endless loop
		if (!posIncremented)
			transOpcode = CTO_Always;
		else
			passSelectRule(table, pos, currentPass, input, &transOpcode, &transRule,
					&transCharslen, &passCharDots, &passInstructions, &passIC,
					&patternMatch, &groupingRule, &groupingOp);
		posIncremented = 1;
		switch (transOpcode) {
		case CTO_Context:
		case CTO_Pass2:
		case CTO_Pass3:
		case CTO_Pass4: {
			const InString *inputBefore = input;
			int posBefore = pos;
			if (appliedRules != NULL && appliedRulesCount < maxAppliedRules)
				appliedRules[appliedRulesCount++] = transRule;
			if (!passDoAction(table, &input, output, posMapping, transOpcode, &transRule,
						passCharDots, passInstructions, passIC, &pos, patternMatch,
						cursorPosition, cursorStatus, groupingRule, groupingOp, mode))
				goto failure;
			if (input->bufferIndex != inputBefore->bufferIndex &&
					inputBefore->bufferIndex != origInput->bufferIndex)
				releaseStringBuffer(inputBefore->bufferIndex);
			if (pos == posBefore) posIncremented = 0;
			break;
		}
		case CTO_Always:
			if ((output->length + 1) > output->maxlength) goto failure;
			posMapping[output->length] = pos;
			output->chars[output->length++] = input->chars[pos++];
			break;
		default:
			goto failure;
		}
	}
failure:
	if (pos < input->length) {
		while (checkDotsAttr(input->chars[pos], CTC_Space, table))
			if (++pos == input->length) break;
	}
	*realInlen = pos;
	if (input->bufferIndex != origInput->bufferIndex)
		releaseStringBuffer(input->bufferIndex);
	return 1;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static int
translateString(const TranslationTableHeader *table, int mode, int currentPass,
		const InString *input, OutString *output, int *posMapping, formtype *typebuf,
		unsigned char *srcSpacing, unsigned char *destSpacing, unsigned int *wordBuffer,
		EmphasisInfo *emphasisBuffer, int haveEmphasis, int *realInlen,
		int *cursorPosition, int *cursorStatus, int compbrlStart, int compbrlEnd);

int EXPORT_CALL
lou_translateString(const char *tableList, const widechar *inbufx, int *inlen,
		widechar *outbuf, int *outlen, formtype *typeform, char *spacing, int mode) {
	return lou_translate(tableList, inbufx, inlen, outbuf, outlen, typeform, spacing,
			NULL, NULL, NULL, mode);
}

int EXPORT_CALL
lou_translate(const char *tableList, const widechar *inbufx, int *inlen, widechar *outbuf,
		int *outlen, formtype *typeform, char *spacing, int *outputPos, int *inputPos,
		int *cursorPos, int mode) {
	return _lou_translate(tableList, tableList, inbufx, inlen, outbuf, outlen, typeform,
			spacing, outputPos, inputPos, cursorPos, mode, NULL, NULL);
}

int EXPORT_CALL
_lou_translate(const char *tableList, const char *displayTableList,
		const widechar *inbufx, int *inlen, widechar *outbuf, int *outlen,
		formtype *typeform, char *spacing, int *outputPos, int *inputPos, int *cursorPos,
		int mode, const TranslationTableRule **rules, int *rulesLen) {
	// int i;
	// for(i = 0; i < *inlen; i++)
	// {
	// 	outbuf[i] = inbufx[i];
	// 	if(inputPos)
	// 		inputPos[i] = i;
	// 	if(outputPos)
	// 		outputPos[i] = i;
	// }
	// *inlen = i;
	// *outlen = i;
	// return 1;
	const TranslationTableHeader *table;
	const DisplayTableHeader *displayTable;
	InString input;
	OutString output;
	// posMapping contains position mapping info between the initial input and the output
	// of the current pass. It is 1 longer than the output. The values are monotonically
	// increasing and can range between -1 and the (consumed) input length. At the end the
	// position info is passed to the user as an inputPos and outputPos array. inputPos
	// has the length of the final output and has values ranging from 0 to inlen-1.
	// outputPos has the length of the (consumed) initial input and has values ranging
	// from 0 to outlen-1.
	int *posMapping;
	int *posMapping1;
	int *posMapping2;
	int *posMapping3;
	formtype *typebuf;
	unsigned char *srcSpacing;
	unsigned char *destSpacing;
	unsigned int *wordBuffer;
	EmphasisInfo *emphasisBuffer;
	int cursorPosition;
	int cursorStatus;
	int haveEmphasis;
	int compbrlStart = -1;
	int compbrlEnd = -1;
	int k;
	int goodTrans = 1;
	if (tableList == NULL || inbufx == NULL || inlen == NULL || outbuf == NULL ||
			outlen == NULL)
		return 0;
	_lou_logMessage(LOU_LOG_ALL, "Performing translation: tableList=%s, inlen=%d",
			tableList, *inlen);
	_lou_logWidecharBuf(LOU_LOG_ALL, "Inbuf=", inbufx, *inlen);

	if (!_lou_isValidMode(mode))
		_lou_logMessage(LOU_LOG_ERROR, "Invalid mode parameter: %d", mode);

	if (displayTableList == NULL) displayTableList = tableList;
	_lou_getTable(tableList, displayTableList, &table, &displayTable);
	if (table == NULL || *inlen < 0 || *outlen < 0) return 0;
	k = 0;
	while (k < *inlen && inbufx[k]) k++;
	input = (InString){ .chars = inbufx, .length = k, .bufferIndex = -1 };
	haveEmphasis = 0;
	if (!(typebuf = _lou_allocMem(alloc_typebuf, 0, input.length, *outlen))) return 0;
	if (typeform != NULL) {
		for (k = 0; k < input.length; k++) {
			typebuf[k] = typeform[k];
			if (typebuf[k] & EMPHASIS) haveEmphasis = 1;
		}
	} else
		memset(typebuf, 0, input.length * sizeof(formtype));

	if (!(spacing == NULL || *spacing == 'X'))
		srcSpacing = (unsigned char *)spacing;
	else
		srcSpacing = NULL;
	if (outputPos != NULL)
		for (k = 0; k < input.length; k++) outputPos[k] = -1;
	if (cursorPos != NULL && *cursorPos >= 0) {
		cursorStatus = 0;
		cursorPosition = *cursorPos;
		if ((mode & (compbrlAtCursor | compbrlLeftCursor))) {
			compbrlStart = cursorPosition;
			if (checkCharAttr(input.chars[compbrlStart], CTC_Space, table))
				/* It would have been simpler to just set compbrlStart and compbrlEnd to
				 * -1 (i.e. disable compbrlAtCursor/compbrlLeftCursor mode) if the cursor
				 * is set on a space. But maybe there are cases where a space in computer
				 * braille does not map to a blank cell, and the user expects to see the
				 * computer braille representation when the space is under the cursor, so
				 * we better leave it as it is.
				 */
				compbrlEnd = compbrlStart + 1;
			else {
				while (compbrlStart >= 0 &&
						!checkCharAttr(input.chars[compbrlStart], CTC_Space, table))
					compbrlStart--;
				compbrlStart++;
				compbrlEnd = cursorPosition;
				if (!(mode & compbrlLeftCursor))
					while (compbrlEnd < input.length &&
							!checkCharAttr(input.chars[compbrlEnd], CTC_Space, table))
						compbrlEnd++;
			}
		}
	} else {
		cursorPosition = -1;
		cursorStatus = 1; /* so it won't check cursor position */
	}
	if (!(posMapping1 = _lou_allocMem(alloc_posMapping1, 0, input.length, *outlen)))
		return 0;
	if (table->numPasses > 1 || table->corrections) {
		if (!(posMapping2 = _lou_allocMem(alloc_posMapping2, 0, input.length, *outlen)))
			return 0;
		if (!(posMapping3 = _lou_allocMem(alloc_posMapping3, 0, input.length, *outlen)))
			return 0;
	}
	if (srcSpacing != NULL) {
		if (!(destSpacing = _lou_allocMem(alloc_destSpacing, 0, input.length, *outlen)))
			goodTrans = 0;
		else
			memset(destSpacing, '*', *outlen);
	} else
		destSpacing = NULL;
	appliedRulesCount = 0;
	if (rules != NULL && rulesLen != NULL) {
		appliedRules = rules;
		maxAppliedRules = *rulesLen;
	} else {
		appliedRules = NULL;
		maxAppliedRules = 0;
	}
	{
		int idx;
		if (!stringBufferPool) initStringBufferPool();
		for (idx = 0; idx < stringBufferPool->size; idx++) releaseStringBuffer(idx);
		idx = getStringBuffer(*outlen);
		output = (OutString){ .chars = stringBufferPool->buffers[idx],
			.maxlength = *outlen,
			.length = 0,
			.bufferIndex = idx };
	}
	posMapping = posMapping1;

	int currentPass = table->corrections ? 0 : 1;
	int *passPosMapping = posMapping;
	while (1) {
		int realInlen;
		switch (currentPass) {
		case 0:
			goodTrans = makeCorrections(table, &input, &output, passPosMapping, typebuf,
					&realInlen, &cursorPosition, &cursorStatus, mode);
			break;
		case 1: {
			if (!(wordBuffer = _lou_allocMem(alloc_wordBuffer, 0, input.length, *outlen)))
				return 0;
			if (!(emphasisBuffer = _lou_allocMem(
						  alloc_emphasisBuffer, 0, input.length, *outlen)))
				return 0;
			goodTrans = translateString(table, mode, currentPass, &input, &output,
					passPosMapping, typebuf, srcSpacing, destSpacing, wordBuffer,
					emphasisBuffer, haveEmphasis, &realInlen, &cursorPosition,
					&cursorStatus, compbrlStart, compbrlEnd);
			break;
		}
		default:
			goodTrans = translatePass(table, currentPass, &input, &output, passPosMapping,
					&realInlen, &cursorPosition, &cursorStatus, mode);
			break;
		}
		passPosMapping[output.length] = realInlen;
		if (passPosMapping == posMapping) {
			passPosMapping = posMapping2;
		} else {
			int *prevPosMapping = posMapping3;
			memcpy((int *)prevPosMapping, posMapping, (*outlen + 1) * sizeof(int));
			for (k = 0; k <= output.length; k++)
				if (passPosMapping[k] < 0)
					posMapping[k] = prevPosMapping[0];
				else
					posMapping[k] = prevPosMapping[passPosMapping[k]];
		}
		currentPass++;
		if (currentPass <= table->numPasses && goodTrans) {
			int idx;
			releaseStringBuffer(input.bufferIndex);
			input = (InString){ .chars = output.chars,
				.length = output.length,
				.bufferIndex = output.bufferIndex };
			idx = getStringBuffer(*outlen);
			output = (OutString){ .chars = stringBufferPool->buffers[idx],
				.maxlength = *outlen,
				.length = 0,
				.bufferIndex = idx };
			continue;
		}
		break;
	}
	if (goodTrans) {
		for (k = 0; k < output.length; k++) {
			if (typeform != NULL) {
				if ((output.chars[k] & (LOU_DOT_7 | LOU_DOT_8)))
					typeform[k] = '8';
				else
					typeform[k] = '0';
			}
			if ((mode & dotsIO)) {
				if ((mode & ucBrl))
					outbuf[k] = ((output.chars[k] & 0xff) | LOU_ROW_BRAILLE);
				else
					outbuf[k] = output.chars[k];
			} else {
				outbuf[k] = _lou_getCharForDots(output.chars[k], displayTable);
				if (!outbuf[k]) {
					// assume that if NUL character is returned, it's because the display
					// table has no mapping for the dot pattern (not because it maps to
					// NUL)
					_lou_logMessage(LOU_LOG_ERROR,
							"%s: no mapping for dot pattern %s in display table",
							displayTableList, _lou_showDots(&output.chars[k], 1));
					return 0;
				}
			}
		}
		*inlen = posMapping[output.length];
		*outlen = output.length;
		// Compute inputPos and outputPos from posMapping. The value at the last index of
		// posMapping is currectly not used.
		if (inputPos != NULL) {
			for (k = 0; k < *outlen; k++)
				if (posMapping[k] < 0)
					inputPos[k] = 0;
				else if (posMapping[k] > *inlen - 1)
					inputPos[k] = *inlen - 1;
				else
					inputPos[k] = posMapping[k];
		}
		if (outputPos != NULL) {
			int inpos = -1;
			int outpos = -1;
			for (k = 0; k < *outlen; k++)
				if (posMapping[k] > inpos) {
					while (inpos < posMapping[k]) {
						if (inpos >= 0 && inpos < *inlen)
							outputPos[inpos] = outpos < 0 ? 0 : outpos;
						inpos++;
					}
					outpos = k;
				}
			if (inpos < 0) inpos = 0;
			while (inpos < *inlen) outputPos[inpos++] = outpos;
		}
	}
	if (destSpacing != NULL) {
		memcpy(srcSpacing, destSpacing, input.length);
		srcSpacing[input.length] = 0;
	}
	if (cursorPos != NULL && *cursorPos != -1) {
		if (outputPos != NULL)
			*cursorPos = outputPos[*cursorPos];
		else
			*cursorPos = cursorPosition;
	}
	if (rulesLen != NULL) *rulesLen = appliedRulesCount;
	_lou_logMessage(LOU_LOG_ALL, "Translation complete: outlen=%d", *outlen);
	_lou_logWidecharBuf(LOU_LOG_ALL, "Outbuf=", (const widechar *)outbuf, *outlen);

	return goodTrans;
}

int EXPORT_CALL
lou_translatePrehyphenated(const char *tableList, const widechar *inbufx, int *inlen,
		widechar *outbuf, int *outlen, formtype *typeform, char *spacing, int *outputPos,
		int *inputPos, int *cursorPos, char *inputHyphens, char *outputHyphens,
		int mode) {
	int rv = 1;
	int *alloc_inputPos = NULL;
	if (inputHyphens != NULL) {
		if (outputHyphens == NULL) return 0;
		if (inputPos == NULL) {
			if ((alloc_inputPos = malloc(*outlen * sizeof(int))) == NULL)
				_lou_outOfMemory();
			inputPos = alloc_inputPos;
		}
	}
	if (lou_translate(tableList, inbufx, inlen, outbuf, outlen, typeform, spacing,
				outputPos, inputPos, cursorPos, mode)) {
		if (inputHyphens != NULL) {
			int inpos = 0;
			int outpos;
			for (outpos = 0; outpos < *outlen; outpos++) {
				int new_inpos = inputPos[outpos];
				if (new_inpos < inpos) {
					rv = 0;
					break;
				}
				if (new_inpos > inpos)
					outputHyphens[outpos] = inputHyphens[new_inpos];
				else
					outputHyphens[outpos] = '0';
				inpos = new_inpos;
			}
		}
	}
	if (alloc_inputPos != NULL) free(alloc_inputPos);
	return rv;
}

static int
hyphenateWord(const widechar *word, int wordSize, char *hyphens,
		const TranslationTableHeader *table) {
	widechar *prepWord;
	int i, k, limit;
	int stateNum;
	widechar ch;
	HyphenationState *statesArray =
			(HyphenationState *)&table->ruleArea[table->hyphenStatesArray];
	HyphenationState *currentState;
	HyphenationTrans *transitionsArray;
	char *hyphenPattern;
	int patternOffset;
	if (!table->hyphenStatesArray || (wordSize + 3) > MAXSTRING) return 0;
	prepWord = (widechar *)calloc(wordSize + 3, sizeof(widechar));
	/* prepWord is of the format ".hello."
	 * hyphens is the length of the word "hello" "00000" */
	prepWord[0] = '.';
	for (i = 0; i < wordSize; i++) {
		prepWord[i + 1] = toLowercase(table, getChar(word[i], table));
		hyphens[i] = '0';
	}
	prepWord[wordSize + 1] = '.';

	/* now, run the finite state machine */
	stateNum = 0;

	// we need to walk all of ".hello."
	for (i = 0; i < wordSize + 2; i++) {
		ch = prepWord[i];
		while (1) {
			if (stateNum == 0xffff) {
				stateNum = 0;
				goto nextLetter;
			}
			currentState = &statesArray[stateNum];
			if (currentState->trans.offset) {
				transitionsArray =
						(HyphenationTrans *)&table->ruleArea[currentState->trans.offset];
				for (k = 0; k < currentState->numTrans; k++) {
					if (transitionsArray[k].ch == ch) {
						stateNum = transitionsArray[k].newState;
						goto stateFound;
					}
				}
			}
			stateNum = currentState->fallbackState;
		}
	stateFound:
		currentState = &statesArray[stateNum];
		if (currentState->hyphenPattern) {
			hyphenPattern = (char *)&table->ruleArea[currentState->hyphenPattern];
			patternOffset = i + 1 - (int)strlen(hyphenPattern);

			/* Need to ensure that we don't overrun hyphens,
			 * in some cases hyphenPattern is longer than the remaining letters,
			 * and if we write out all of it we would have overshot our buffer. */
			limit = MIN((int)strlen(hyphenPattern), wordSize - patternOffset);
			for (k = 0; k < limit; k++) {
				if (hyphens[patternOffset + k] < hyphenPattern[k])
					hyphens[patternOffset + k] = hyphenPattern[k];
			}
		}
	nextLetter:;
	}
	hyphens[wordSize] = 0;
	free(prepWord);
	return 1;
}

static int
doCompTrans(int start, int end, const TranslationTableHeader *table, int *pos,
		const InString *input, OutString *output, int *posMapping,
		EmphasisInfo *emphasisBuffer, const TranslationTableRule **transRule,
		int *cursorPosition, int *cursorStatus, int mode);

// The `shift' argument should be used with care because it can mess up the positions
// array which is supposed to be monotonically increasing. It is set to -1 in order to
//  append certain indicators (endemphword, endemph, endemphphrase after, endcapsword,
// endcaps, endcapsphrase after) to the preceding character.
static int
for_updatePositions(const widechar *outChars, int inLength, int outLength, int shift,
		int pos, const InString *input, OutString *output, int *posMapping,
		int *cursorPosition, int *cursorStatus) {
	int k;
	if ((output->length + outLength) > output->maxlength ||
			(pos + inLength) > input->length)
		return 0;
	memcpy(&output->chars[output->length], outChars, outLength * CHARSIZE);
	if (!*cursorStatus) {
		if (*cursorPosition >= pos && *cursorPosition < (pos + inLength)) {
			*cursorPosition = output->length;
			*cursorStatus = 1;
		} else if (input->chars[*cursorPosition] == 0 &&
				*cursorPosition == (pos + inLength)) {
			*cursorPosition = output->length + outLength / 2 + 1;
			*cursorStatus = 1;
		}
	} else if (*cursorStatus == 2 && *cursorPosition == pos)
		*cursorPosition = output->length;
	for (k = 0; k < outLength; k++) posMapping[output->length + k] = pos + shift;
	output->length += outLength;
	return 1;
}

static int
syllableBreak(const TranslationTableHeader *table, int pos, const InString *input,
		int transCharslen) {
	int wordStart = 0;
	int wordEnd = 0;
	int wordSize = 0;
	int k = 0;
	char *hyphens = NULL;
	for (wordStart = pos; wordStart >= 0; wordStart--)
		if (!((getChar(input->chars[wordStart], table))->attributes & CTC_Letter)) {
			wordStart++;
			break;
		}
	if (wordStart < 0) wordStart = 0;
	for (wordEnd = pos; wordEnd < input->length; wordEnd++)
		if (!((getChar(input->chars[wordEnd], table))->attributes & CTC_Letter)) {
			wordEnd--;
			break;
		}
	if (wordEnd == input->length) wordEnd--;
	/* At this stage wordStart is the 0 based index of the first letter in the word,
	 * wordEnd is the 0 based index of the last letter in the word.
	 * example: "hello" wordstart=0, wordEnd=4. */
	wordSize = wordEnd - wordStart + 1;
	hyphens = (char *)calloc(wordSize + 1, sizeof(char));
	if (!hyphenateWord(&input->chars[wordStart], wordSize, hyphens, table)) {
		free(hyphens);
		return 0;
	}
	for (k = pos - wordStart + 1; k < (pos - wordStart + transCharslen); k++)
		if (hyphens[k] & 1) {
			free(hyphens);
			return 1;
		}
	free(hyphens);
	return 0;
}

static void
setBefore(const TranslationTableHeader *table, int pos, const InString *input,
		TranslationTableCharacterAttributes *beforeAttributes) {
	widechar before;
	if (pos >= 2 && input->chars[pos - 1] == LOU_ENDSEGMENT)
		before = input->chars[pos - 2];
	else
		before = (pos == 0) ? ' ' : input->chars[pos - 1];
	*beforeAttributes = (getChar(before, table))->attributes;
}

static void
setAfter(int length, const TranslationTableHeader *table, int pos, const InString *input,
		TranslationTableCharacterAttributes *afterAttributes) {
	widechar after;
	if ((pos + length + 2) < input->length && input->chars[pos + 1] == LOU_ENDSEGMENT)
		after = input->chars[pos + 2];
	else
		after = (pos + length < input->length) ? input->chars[pos + length] : ' ';
	*afterAttributes = (getChar(after, table))->attributes;
}

static int
brailleIndicatorDefined(TranslationTableOffset offset,
		const TranslationTableHeader *table, const TranslationTableRule **indicRule) {
	if (!offset) return 0;
	*indicRule = (TranslationTableRule *)&table->ruleArea[offset];
	return 1;
}

/**
 * Return 1 if both `indicator1` and `indicator2` are defined and use the same dot
 * pattern. Otherwise return 0.
 */
static int
isIndicatorEqual(TranslationTableOffset indicator1, TranslationTableOffset indicator2,
		const TranslationTableHeader *table) {
	const TranslationTableRule *indicatorRule1;
	const TranslationTableRule *indicatorRule2;

	if (brailleIndicatorDefined(indicator1, table, &indicatorRule1) &&
			brailleIndicatorDefined(indicator2, table, &indicatorRule2) &&
			indicatorRule1->dotslen == indicatorRule2->dotslen &&
			!memcmp(&indicatorRule1->charsdots[0], &indicatorRule2->charsdots[0],
					indicatorRule1->dotslen * CHARSIZE)) {
		return 1;
	} else {
		return 0;
	}
}

static int
capsletterDefined(const TranslationTableHeader *table) {
	return table->emphRules[MAX_EMPH_CLASSES][letterOffset];
}

static int
validMatch(const TranslationTableHeader *table, int pos, const InString *input,
		formtype *typebuf, const TranslationTableRule *transRule, int transCharslen) {
	/* Analyze the typeform parameter and also check for capitalization */
	TranslationTableCharacter *inputChar;
	TranslationTableCharacter *ruleChar;
	TranslationTableCharacterAttributes prevAttr = 0;
	int k;
	int kk = 0;
	if (!transCharslen) return 0;
	for (k = pos; k < pos + transCharslen; k++) {
		if (input->chars[k] == LOU_ENDSEGMENT) {
			if (k == pos && transCharslen == 1)
				return 1;
			else
				return 0;
		}
		inputChar = getChar(input->chars[k], table);
		if (k == pos) prevAttr = inputChar->attributes;
		ruleChar = getChar(transRule->charsdots[kk++], table);
		if (toLowercase(table, inputChar) != toLowercase(table, ruleChar)) return 0;
		if (typebuf != NULL && (typebuf[pos] & CAPSEMPH) == 0 &&
				(typebuf[k] | typebuf[pos]) != typebuf[pos])
			return 0;
		if (inputChar->attributes != CTC_Letter) {
			if (k != (pos + 1) && (prevAttr & CTC_Letter) &&
					(inputChar->attributes & CTC_Letter) &&
					((inputChar->attributes &
							 (CTC_LowerCase | CTC_UpperCase | CTC_Letter)) !=
							(prevAttr & (CTC_LowerCase | CTC_UpperCase | CTC_Letter))))
				return 0;
		}
		prevAttr = inputChar->attributes;
	}
	return 1;
}

static int
insertNumberSign(const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, int prevTransOpcode, int *cursorPosition,
		int *cursorStatus, TranslationTableCharacterAttributes beforeAttributes) {
	const TranslationTableRule *numberSign;
	if (brailleIndicatorDefined(table->numberSign, table, &numberSign) &&
			checkCharAttr_safe(input, pos, CTC_Digit, table) &&
			(prevTransOpcode == CTO_ExactDots ||
					(!(beforeAttributes & CTC_Digit) && prevTransOpcode != CTO_MidNum))) {
		if (!for_updatePositions(&numberSign->charsdots[0], 0, numberSign->dotslen, 0,
					pos, input, output, posMapping, cursorPosition, cursorStatus))
			return 0;
	}
	return 1;
}

static int
isNoLetsign(widechar c, const TranslationTableHeader *table) {
	for (int k = 0; k < table->noLetsignCount; k++)
		if (c == table->noLetsign[k]) return 1;
	return 0;
}

static int
isNoLetsignBefore(widechar c, const TranslationTableHeader *table) {
	for (int k = 0; k < table->noLetsignBeforeCount; k++)
		if (c == table->noLetsignBefore[k]) return 1;
	return 0;
}

static int
isNoLetsignAfter(widechar c, const TranslationTableHeader *table) {
	for (int k = 0; k < table->noLetsignAfterCount; k++)
		if (c == table->noLetsignAfter[k]) return 1;
	return 0;
}

static int
insertLetterSign(const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, int transOpcode, int *cursorPosition,
		int *cursorStatus, TranslationTableCharacterAttributes beforeAttributes) {
	const TranslationTableRule *letterSign;
	if (brailleIndicatorDefined(table->letterSign, table, &letterSign)) {
		if (transOpcode == CTO_Contraction) {
			if (!for_updatePositions(&letterSign->charsdots[0], 0, letterSign->dotslen, 0,
						pos, input, output, posMapping, cursorPosition, cursorStatus))
				return 0;
		} else if ((checkCharAttr_safe(input, pos, CTC_Letter, table) &&
						   !(beforeAttributes & CTC_Letter)) &&
				(!checkCharAttr_safe(input, pos + 1, CTC_Letter, table) ||
						(beforeAttributes & CTC_Digit))) {
			if (pos > 0 && isNoLetsignBefore(input->chars[pos - 1], table)) return 1;
			if (isNoLetsign(input->chars[pos], table)) return 1;
			if (pos + 1 < input->length && isNoLetsignAfter(input->chars[pos + 1], table))
				return 1;
			if (!for_updatePositions(&letterSign->charsdots[0], 0, letterSign->dotslen, 0,
						pos, input, output, posMapping, cursorPosition, cursorStatus))
				return 0;
		}
	}
	return 1;
}

static int
onlyLettersBehind(const TranslationTableHeader *table, int pos, const InString *input,
		TranslationTableCharacterAttributes beforeAttributes) {
	/* Actually, spaces, then letters */
	int k;
	if (!(beforeAttributes & CTC_Space)) return 0;
	for (k = pos - 2; k >= 0; k--) {
		TranslationTableCharacterAttributes attr =
				(getChar(input->chars[k], table))->attributes;
		if ((attr & CTC_Space)) continue;
		if ((attr & CTC_Letter))
			return 1;
		else
			return 0;
	}
	return 1;
}

static int
onlyLettersAhead(const TranslationTableHeader *table, int pos, const InString *input,
		int transCharslen, TranslationTableCharacterAttributes afterAttributes) {
	/* Actullly, spaces, then letters */
	int k;
	if (!(afterAttributes & CTC_Space)) return 0;
	for (k = pos + transCharslen + 1; k < input->length; k++) {
		TranslationTableCharacterAttributes attr =
				(getChar(input->chars[k], table))->attributes;
		if ((attr & CTC_Space)) continue;
		if ((attr & (CTC_Letter | CTC_LitDigit)))
			return 1;
		else
			return 0;
	}
	return 0;
}

static int
noCompbrlAhead(const TranslationTableHeader *table, int pos, int mode,
		const InString *input, int transOpcode, int transCharslen, int cursorPosition) {
	int start = pos + transCharslen;
	int end;
	int p;
	if (start >= input->length) return 1;
	while (start < input->length && checkCharAttr(input->chars[start], CTC_Space, table))
		start++;
	if (start == input->length ||
			(transOpcode == CTO_JoinableWord &&
					(!checkCharAttr(input->chars[start], CTC_Letter | CTC_Digit, table) ||
							!checkCharAttr(input->chars[start - 1], CTC_Space, table))))
		return 1;
	end = start;
	while (end < input->length && !checkCharAttr(input->chars[end], CTC_Space, table))
		end++;
	if ((mode & (compbrlAtCursor | compbrlLeftCursor)) && cursorPosition >= start &&
			cursorPosition < end)
		return 0;
	/* Look ahead for rules with CTO_CompBrl */
	for (p = start; p < end; p++) {
		int length = input->length - p;
		int tryThis;
		int k;
		for (tryThis = 0; tryThis < 2; tryThis++) {
			TranslationTableOffset ruleOffset = 0;
			TranslationTableRule *testRule;
			switch (tryThis) {
			case 0:
				if (!(length >= 2)) break;
				ruleOffset = table->forRules[_lou_stringHash(&input->chars[p], 1, table)];
				break;
			case 1:
				if (!(length >= 1)) break;
				length = 1;
				ruleOffset = getChar(input->chars[p], table)->otherRules;
				break;
			}
			while (ruleOffset) {
				const TranslationTableCharacter *character1;
				const TranslationTableCharacter *character2;
				testRule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
				for (k = 0; k < testRule->charslen && k < length; k++) {
					character1 = getChar(testRule->charsdots[k], table);
					character2 = getChar(input->chars[p + k], table);
					if (toLowercase(table, character1) != toLowercase(table, character2))
						break;
				}
				if (tryThis == 1 || k == testRule->charslen) {
					if (testRule->opcode == CTO_CompBrl ||
							testRule->opcode == CTO_Literal)
						return 0;
				}
				ruleOffset = testRule->charsnext;
			}
		}
	}
	return 1;
}

static int
checkEmphasisChange(int pos, int len, const EmphasisInfo *emphasisBuffer) {
	int i;
	for (i = pos + 1; i < pos + len; i++)
		if (emphasisBuffer[i].begin || emphasisBuffer[i].end || emphasisBuffer[i].word ||
				emphasisBuffer[i].symbol)
			return 1;
	return 0;
}

static int
isRepeatedWord(const TranslationTableHeader *table, int pos, const InString *input,
		const EmphasisInfo *emphasisBuffer, int outputLength, const int *posMapping,
		int transCharslen, int *repwordLength) {
	/* transCharslen is the length of the character sequence that separates the repeated
	 * parts */
	int len;
	/* maximum length that the repeated part can have is determined by how many letters
	 * there are before and after the separator */
	for (len = 1; pos - len >= 0 && pos + transCharslen + len - 1 < input->length &&
			checkCharAttr(input->chars[pos - len], CTC_Letter, table) &&
			checkCharAttr(input->chars[pos + transCharslen + len - 1], CTC_Letter, table);
			len++)
		;
	len--;
	/* now actually compare the parts, starting with the maximal length and making them
	 * shorter if they don't match */
	while (len > 0) {
		int start = pos - len;
		if (compareChars(&input->chars[start], &input->chars[pos + transCharslen], len,
					table)) {
			/* part must not start within a contraction */
			for (int k = outputLength - 1; k >= 0; k--)
				if (posMapping[k] == start)
					break;
				else if (posMapping[k] < start)
					return 0;
			/* capitalisation and emphasis may not change except at the beginning of the
			 * parts */
			if (checkEmphasisChange(start, len + transCharslen, emphasisBuffer) ||
					checkEmphasisChange(pos + transCharslen, len, emphasisBuffer))
				return 0;
			*repwordLength = len;
			return 1;
		}
		len--;
	}
	return 0;
}

static int
inSequence(const TranslationTableHeader *table, int pos, const InString *input,
		const TranslationTableRule *transRule) {
	int i, j, s, match;
	// TODO: all caps words
	// const TranslationTableCharacter *c = NULL;

	/* check before sequence */
	for (i = pos - 1; i >= 0; i--) {
		if (checkCharAttr(input->chars[i], CTC_SeqBefore, table)) continue;
		if (!(checkCharAttr(input->chars[i], CTC_Space | CTC_SeqDelimiter, table)))
			return 0;
		break;
	}

	/* check after sequence */
	for (i = pos + transRule->charslen; i < input->length; i++) {
		/* check sequence after patterns */
		if (table->seqPatternsCount) {
			match = 0;
			for (j = i, s = 0; j <= input->length && s < table->seqPatternsCount;
					j++, s++) {
				/* matching */
				if (match == 1) {
					if (table->seqPatterns[s]) {
						if (input->chars[j] == table->seqPatterns[s])
							match = 1;
						else {
							match = -1;
							j = i - 1;
						}
					}

					/* found match */
					else {
						/* pattern at end of input */
						if (j >= input->length) return 1;

						i = j;
						break;
					}
				}

				/* looking for match */
				else if (match == 0) {
					if (table->seqPatterns[s]) {
						if (input->chars[j] == table->seqPatterns[s])
							match = 1;
						else {
							match = -1;
							j = i - 1;
						}
					}
				}

				/* next pattarn */
				else if (match == -1) {
					if (!table->seqPatterns[s]) {
						match = 0;
						j = i - 1;
					}
				}
			}
		}

		if (checkCharAttr(input->chars[i], CTC_SeqAfter, table)) continue;
		if (!(checkCharAttr(input->chars[i], CTC_Space | CTC_SeqDelimiter, table)))
			return 0;
		break;
	}

	return 1;
}

static void
for_selectRule(const TranslationTableHeader *table, int pos, OutString output,
		const int *posMapping, int mode, const InString *input, formtype *typebuf,
		EmphasisInfo *emphasisBuffer, int *transOpcode, int prevTransOpcode,
		const TranslationTableRule **transRule, int *transCharslen, int *passCharDots,
		widechar const **passInstructions, int *passIC, PassRuleMatch *patternMatch,
		int posIncremented, int cursorPosition, int *repwordLength, int dontContract,
		int compbrlStart, int compbrlEnd,
		TranslationTableCharacterAttributes beforeAttributes,
		TranslationTableCharacter **curCharDef, TranslationTableRule **groupingRule,
		widechar *groupingOp) {
	/* check for valid Translations. Return value is in transRule. */
	static TranslationTableRule pseudoRule = { 0 };
	int length = ((pos < compbrlStart) ? compbrlStart : input->length) - pos;
	int tryThis;
	int k;
	TranslationTableOffset ruleOffset = 0;
	*curCharDef = getChar(input->chars[pos], table);
	for (tryThis = 0; tryThis < 3; tryThis++) {
		switch (tryThis) {
		case 0:
			if (!(length >= 2)) break;
			ruleOffset = table->forRules[_lou_stringHash(&input->chars[pos], 1, table)];
			break;
		case 1:
			if (!(length >= 1)) break;
			length = 1;
			ruleOffset = (*curCharDef)->otherRules;
			break;
		case 2: /* No rule found */
			*transRule = &pseudoRule;
			*transOpcode = pseudoRule.opcode = CTO_None;
			*transCharslen = pseudoRule.charslen = 1;
			pseudoRule.charsdots[0] = input->chars[pos];
			pseudoRule.dotslen = 0;
			return;
		}
		while (ruleOffset) {
			*transRule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
			*transOpcode = (*transRule)->opcode;
			*transCharslen = (*transRule)->charslen;
			if (tryThis == 1 ||
					((*transCharslen <= length) &&
							validMatch(table, pos, input, typebuf, *transRule,
									*transCharslen))) {
				TranslationTableCharacterAttributes afterAttributes;
				/* check before emphasis match */
				if ((*transRule)->before & CTC_EmpMatch) {
					if (emphasisBuffer[pos].begin || emphasisBuffer[pos].end ||
							emphasisBuffer[pos].word || emphasisBuffer[pos].symbol)
						break;
				}

				/* check after emphasis match */
				if ((*transRule)->after & CTC_EmpMatch) {
					if (emphasisBuffer[pos + *transCharslen].begin ||
							emphasisBuffer[pos + *transCharslen].end ||
							emphasisBuffer[pos + *transCharslen].word ||
							emphasisBuffer[pos + *transCharslen].symbol)
						break;
				}

				/* check this rule */
				setAfter(*transCharslen, table, pos, input, &afterAttributes);
				if ((!((*transRule)->after & ~CTC_EmpMatch) ||
							(beforeAttributes & (*transRule)->after)) &&
						(!((*transRule)->before & ~CTC_EmpMatch) ||
								(afterAttributes & (*transRule)->before)))
					/* check nocross */
					if (!((*transRule)->nocross &&
								syllableBreak(table, pos, input, *transCharslen))) {
						switch (*transOpcode) { /* check validity of this Translation */
						case CTO_Space:
						case CTO_Letter:
						case CTO_UpperCase:
						case CTO_LowerCase:
						case CTO_Digit:
						case CTO_LitDigit:
						case CTO_Punctuation:
						case CTO_Math:
						case CTO_Sign:
						case CTO_Hyphen:
						case CTO_Replace:
						case CTO_CompBrl:
						case CTO_Literal:
							return;
						case CTO_Repeated:
							if (dontContract || (mode & noContractions)) break;
							if ((mode & (compbrlAtCursor | compbrlLeftCursor)) &&
									pos >= compbrlStart && pos <= compbrlEnd)
								break;
							return;
						case CTO_RepWord:
						case CTO_RepEndWord:
							if (dontContract || (mode & noContractions)) break;
							if (isRepeatedWord(table, pos, input, emphasisBuffer,
										output.length, posMapping, *transCharslen,
										repwordLength)) {
								if ((pos > *repwordLength &&
											checkCharAttr(input->chars[pos -
																  *repwordLength - 1],
													CTC_Letter, table)) ==
										(*transOpcode == CTO_RepEndWord)) {
									return;
								}
							}
							break;
						case CTO_NoCont:
							if (dontContract || (mode & noContractions)) break;
							return;
						case CTO_Syllable:
							*transOpcode = CTO_Always;
						case CTO_Always:
							if (checkEmphasisChange(pos, *transCharslen, emphasisBuffer))
								break;
							if (dontContract || (mode & noContractions)) break;
							return;
						case CTO_ExactDots:
							return;
						case CTO_Context:
							// check posIncremented to avoid endless loop
							if (!posIncremented ||
									!passDoTest(table, pos, input, *transOpcode,
											*transRule, passCharDots, passInstructions,
											passIC, patternMatch, groupingRule,
											groupingOp))
								break;
							return;
						case CTO_LargeSign:
							if (dontContract || (mode & noContractions)) break;
							if (!((beforeAttributes & (CTC_Space | CTC_Punctuation)) ||
										onlyLettersBehind(
												table, pos, input, beforeAttributes)) ||
									!((afterAttributes & CTC_Space) ||
											prevTransOpcode == CTO_LargeSign) ||
									(afterAttributes & CTC_Letter) ||
									!noCompbrlAhead(table, pos, mode, input, *transOpcode,
											*transCharslen, cursorPosition))
								*transOpcode = CTO_Always;
							return;
						case CTO_WholeWord:
							if (dontContract || (mode & noContractions)) break;
							if (checkEmphasisChange(pos, *transCharslen, emphasisBuffer))
								break;
						case CTO_Contraction:
							if (table->usesSequences) {
								if (inSequence(table, pos, input, *transRule)) return;
							} else {
								if ((beforeAttributes & (CTC_Space | CTC_Punctuation)) &&
										(afterAttributes & (CTC_Space | CTC_Punctuation)))
									return;
							}
							break;
						case CTO_PartWord:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes & CTC_Letter) ||
									(afterAttributes & CTC_Letter))
								return;
							break;
						case CTO_JoinNum:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes & (CTC_Space | CTC_Punctuation)) &&
									(afterAttributes & CTC_Space) &&
									(output.length + (*transRule)->dotslen <
											output.maxlength)) {
								int p = pos + *transCharslen + 1;
								while (p < input->length) {
									if (!checkCharAttr(
												input->chars[p], CTC_Space, table)) {
										if (checkCharAttr(
													input->chars[p], CTC_Digit, table))
											return;
										break;
									}
									p++;
								}
							}
							break;
						case CTO_LowWord:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes & CTC_Space) &&
									(afterAttributes & CTC_Space) &&
									(prevTransOpcode != CTO_JoinableWord))
								return;
							break;
						case CTO_JoinableWord:
							if (dontContract || (mode & noContractions)) break;
							if (beforeAttributes & (CTC_Space | CTC_Punctuation) &&
									onlyLettersAhead(table, pos, input, *transCharslen,
											afterAttributes) &&
									noCompbrlAhead(table, pos, mode, input, *transOpcode,
											*transCharslen, cursorPosition))
								return;
							break;
						case CTO_SuffixableWord:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes & (CTC_Space | CTC_Punctuation)) &&
									(afterAttributes &
											(CTC_Space | CTC_Letter | CTC_Punctuation)))
								return;
							break;
						case CTO_PrefixableWord:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes &
										(CTC_Space | CTC_Letter | CTC_Punctuation)) &&
									(afterAttributes & (CTC_Space | CTC_Punctuation)))
								return;
							break;
						case CTO_BegWord:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes & (CTC_Space | CTC_Punctuation)) &&
									(afterAttributes & CTC_Letter))
								return;
							break;
						case CTO_BegMidWord:
							if (dontContract || (mode & noContractions)) break;
							if ((beforeAttributes &
										(CTC_Letter | CTC_Space | CTC_Punctuation)) &&
									(afterAttributes & CTC_Letter))
								return;
							break;
						case CTO_MidWord:
							if (dontContract || (mode & noContractions)) break;
							if (beforeAttributes & CTC_Letter &&
									afterAttributes & CTC_Letter)
								return;
							break;
						case CTO_MidEndWord:
							if (dontContract || (mode & noContractions)) break;
							if (beforeAttributes & CTC_Letter &&
									afterAttributes &
											(CTC_Letter | CTC_Space | CTC_Punctuation))
								return;
							break;
						case CTO_EndWord:
							if (dontContract || (mode & noContractions)) break;
							if (beforeAttributes & CTC_Letter &&
									afterAttributes & (CTC_Space | CTC_Punctuation))
								return;
							break;
						case CTO_BegNum:
							if (beforeAttributes & (CTC_Space | CTC_Punctuation) &&
									afterAttributes & CTC_Digit)
								return;
							break;
						case CTO_MidNum:
							if (prevTransOpcode != CTO_ExactDots &&
									beforeAttributes & CTC_Digit &&
									afterAttributes & CTC_Digit)
								return;
							break;
						case CTO_EndNum:
							if (beforeAttributes & CTC_Digit &&
									prevTransOpcode != CTO_ExactDots)
								return;
							break;
						case CTO_DecPoint:
							if (!(afterAttributes & CTC_Digit)) break;
							if (beforeAttributes & CTC_Digit) *transOpcode = CTO_MidNum;
							return;
						case CTO_PrePunc:
							if (!checkCharAttr(
										input->chars[pos], CTC_Punctuation, table) ||
									(pos > 0 &&
											checkCharAttr(input->chars[pos - 1],
													CTC_Letter, table)))
								break;
							for (k = pos + *transCharslen; k < input->length; k++) {
								if (checkCharAttr(input->chars[k],
											(CTC_Letter | CTC_Digit), table))
									return;
								if (checkCharAttr(input->chars[k], CTC_Space, table))
									break;
							}
							break;
						case CTO_PostPunc:
							if (!checkCharAttr(
										input->chars[pos], CTC_Punctuation, table) ||
									(pos < (input->length - 1) &&
											checkCharAttr(input->chars[pos + 1],
													CTC_Letter, table)))
								break;
							for (k = pos; k >= 0; k--) {
								if (checkCharAttr(input->chars[k],
											(CTC_Letter | CTC_Digit), table))
									return;
								if (checkCharAttr(input->chars[k], CTC_Space, table))
									break;
							}
							break;

						case CTO_Match: {
							widechar *patterns, *pattern;

							if (dontContract || (mode & noContractions)) break;
							if (checkEmphasisChange(pos, *transCharslen, emphasisBuffer))
								break;

							patterns =
									(widechar *)&table->ruleArea[(*transRule)->patterns];

							/* check before pattern */
							pattern = &patterns[1];
							if (!_lou_pattern_check(
										input->chars, pos - 1, -1, -1, pattern, table))
								break;

							/* check after pattern */
							pattern = &patterns[patterns[0]];
							if (!_lou_pattern_check(input->chars,
										pos + (*transRule)->charslen, input->length, 1,
										pattern, table))
								break;

							return;
						}

						default:
							break;
						}
					}
			}
			/* Done with checking this rule */
			ruleOffset = (*transRule)->charsnext;
		}
	}
}

static int
undefinedCharacter(widechar c, const TranslationTableHeader *table, int pos,
		const InString *input, OutString *output, int *posMapping, int *cursorPosition,
		int *cursorStatus, int mode) {
	/* Display an undefined character in the output buffer */
	if (table->undefined) {
		TranslationTableRule *rule =
				(TranslationTableRule *)&table->ruleArea[table->undefined];

		return for_updatePositions(&rule->charsdots[rule->charslen], rule->charslen,
				rule->dotslen, 0, pos, input, output, posMapping, cursorPosition,
				cursorStatus);
	}

	const char *text = (mode & noUndefined) ? "" : _lou_showString(&c, 1, 1);
	size_t length = strlen(text);

#ifdef _MSC_VER
	widechar dots[MAXSTRING]; // MSVC doesn't support variable length arrays
#else
	widechar dots[length == 0 ? 1 : length];
#endif

	for (unsigned int k = 0; k < length; k += 1) {
		dots[k] = 0;
		// looking in otherRules and not definitionRule because definitionRule gives us
		// the last occurence of a character definition rule and we are interested in
		// the first
		TranslationTableOffset offset = getChar(text[k], table)->otherRules;
		while (offset) {
			const TranslationTableRule *r =
					(TranslationTableRule *)&table->ruleArea[offset];
			if (r->opcode >= CTO_Space && r->opcode < CTO_UpLow && r->dotslen == 1) {
				dots[k] = r->charsdots[1];
				break;
			}
			offset = r->charsnext;
		}
		if (!dots[k]) dots[k] = _lou_charToFallbackDots(text[k]);
	}

	return for_updatePositions(dots, 1, length, 0, pos, input, output, posMapping,
			cursorPosition, cursorStatus);
}

static int
putCharacter(widechar character, const TranslationTableHeader *table, int pos,
		const InString *input, OutString *output, int *posMapping, int *cursorPosition,
		int *cursorStatus, int mode) {
	/* Insert the dots equivalent of a character into the output buffer */
	TranslationTableOffset offset;
	TranslationTableCharacter *chardef = getChar(character, table);
	if (chardef->basechar)
		chardef = (TranslationTableCharacter *)&table->ruleArea[chardef->basechar];
	offset = chardef->definitionRule;
	if (offset) {
		const TranslationTableRule *rule =
				(TranslationTableRule *)&table->ruleArea[offset];
		return for_updatePositions(&rule->charsdots[1], 1, rule->dotslen, 0, pos, input,
				output, posMapping, cursorPosition, cursorStatus);
	}
	return undefinedCharacter(character, table, pos, input, output, posMapping,
			cursorPosition, cursorStatus, mode);
}

static int
putCharacters(const widechar *characters, int count, const TranslationTableHeader *table,
		int pos, const InString *input, OutString *output, int *posMapping,
		int *cursorPosition, int *cursorStatus, int mode) {
	/* Insert the dot equivalents of a series of characters in the output
	 * buffer */
	int k;
	for (k = 0; k < count; k++)
		if (!putCharacter(characters[k], table, pos, input, output, posMapping,
					cursorPosition, cursorStatus, mode))
			return 0;
	return 1;
}

// state at the beginning of the current word, used for back-tracking and also for the
// nocont and compbrl rules
typedef struct {
	int inPos;			// begin position of the current word in the input
	int outPos;			// begin position of the current word in the output
	int emphasisInPos;	// position of the next character in the input for which to insert
						// emphasis marks
} LastWord;

static int
doCompbrl(const TranslationTableHeader *table, int *pos, const InString *input,
		OutString *output, int *posMapping, EmphasisInfo *emphasisBuffer,
		const TranslationTableRule **transRule, int *cursorPosition, int *cursorStatus,
		const LastWord *lastWord, int *insertEmphasesFrom, int mode) {
	/* Handle strings containing substrings defined by the compbrl opcode */
	int stringStart, stringEnd;
	if (checkCharAttr(input->chars[*pos], CTC_Space, table)) return 1;
	if (lastWord->outPos) {
		*pos = lastWord->inPos;
		output->length = lastWord->outPos;
	} else {
		*pos = 0;
		output->length = 0;
	}
	*insertEmphasesFrom = lastWord->emphasisInPos;
	// just in case word starts with space
	while (checkCharAttr(input->chars[*pos], CTC_Space, table)) (*pos)++;
	stringStart = *pos;
	while (stringStart > 0 &&
			!checkCharAttr(input->chars[stringStart - 1], CTC_Space, table))
		stringStart--;
	stringEnd = *pos;
	while (stringEnd < input->length &&
			!checkCharAttr(input->chars[stringEnd], CTC_Space, table))
		stringEnd++;
	return doCompTrans(stringStart, stringEnd, table, pos, input, output, posMapping,
			emphasisBuffer, transRule, cursorPosition, cursorStatus, mode);
}

static int
doCompTrans(int start, int end, const TranslationTableHeader *table, int *pos,
		const InString *input, OutString *output, int *posMapping,
		EmphasisInfo *emphasisBuffer, const TranslationTableRule **transRule,
		int *cursorPosition, int *cursorStatus, int mode) {
	const TranslationTableRule *indicRule;
	int k;
	int haveEndsegment = 0;
	if (*cursorStatus != 2 && brailleIndicatorDefined(table->begComp, table, &indicRule))
		if (!for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, *pos,
					input, output, posMapping, cursorPosition, cursorStatus))
			return 0;
	for (k = start; k < end; k++) {
		TranslationTableOffset compdots = 0;
		/* HACK: computer braille is one-to-one so it
		 * can't have any emphasis indicators.
		 * A better solution is to treat computer braille as its own mode. */
		emphasisBuffer[k] = (EmphasisInfo){ 0 };
		if (input->chars[k] == LOU_ENDSEGMENT) {
			haveEndsegment = 1;
			continue;
		}
		*pos = k;
		compdots = getChar(input->chars[k], table)->compRule;
		if (compdots != 0) {
			*transRule = (TranslationTableRule *)&table->ruleArea[compdots];
			if (!for_updatePositions(&(*transRule)->charsdots[(*transRule)->charslen],
						(*transRule)->charslen, (*transRule)->dotslen, 0, *pos, input,
						output, posMapping, cursorPosition, cursorStatus))
				return 0;
		} else if (!putCharacter(input->chars[k], table, *pos, input, output, posMapping,
						   cursorPosition, cursorStatus, mode))
			return 0;
	}
	if (*cursorStatus != 2 && brailleIndicatorDefined(table->endComp, table, &indicRule))
		if (!for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, *pos,
					input, output, posMapping, cursorPosition, cursorStatus))
			return 0;
	*pos = end;
	if (haveEndsegment) {
		widechar endSegment = LOU_ENDSEGMENT;
		if (!for_updatePositions(&endSegment, 0, 1, 0, *pos, input, output, posMapping,
					cursorPosition, cursorStatus))
			return 0;
	}
	return 1;
}

static int
doNocont(const TranslationTableHeader *table, int *pos, OutString *output, int mode,
		const InString *input, const LastWord *lastWord, int *dontContract,
		int *insertEmphasesFrom) {
	/* Handle strings containing substrings defined by the nocont opcode */
	if (checkCharAttr(input->chars[*pos], CTC_Space, table) || *dontContract ||
			(mode & noContractions))
		return 1;
	if (lastWord->outPos) {
		*pos = lastWord->inPos;
		output->length = lastWord->outPos;
	} else {
		*pos = 0;
		output->length = 0;
	}
	*insertEmphasesFrom = lastWord->emphasisInPos;
	*dontContract = 1;
	return 1;
}

static int
markSyllables(
		const TranslationTableHeader *table, const InString *input, formtype *typebuf) {
	int pos;
	int k;
	int currentMark = 0;
	int const syllable_marks[] = { SYLLABLE_MARKER_1, SYLLABLE_MARKER_2 };
	int syllable_mark_selector = 0;
	const TranslationTableRule *transRule;
	int transOpcode;
	int transCharslen;

	if (typebuf == NULL || !table->syllables) return 1;
	pos = 0;
	while (pos < input->length) { /* the main multipass translation loop */
		int length = input->length - pos;
		int tryThis = 0;
		while (tryThis < 3) {
			TranslationTableOffset ruleOffset = 0;
			switch (tryThis) {
			case 0:
				if (!(length >= 2)) break;
				// memory overflow when pos == input->length - 1
				ruleOffset =
						table->forRules[_lou_stringHash(&input->chars[pos], 1, table)];
				break;
			case 1:
				if (!(length >= 1)) break;
				length = 1;
				ruleOffset = getChar(input->chars[pos], table)->otherRules;
				break;
			case 2: /* No rule found */
				transOpcode = CTO_Always;
				ruleOffset = 0;
				break;
			}
			while (ruleOffset) {
				transRule = (TranslationTableRule *)&table->ruleArea[ruleOffset];
				transOpcode = transRule->opcode;
				transCharslen = transRule->charslen;
				if (tryThis == 1 ||
						(transCharslen <= length &&
								compareChars(&transRule->charsdots[0], &input->chars[pos],
										transCharslen, table))) {
					if (transOpcode == CTO_Syllable) {
						tryThis = 4;
						break;
					}
				}
				ruleOffset = transRule->charsnext;
			}
			tryThis++;
		}
		switch (transOpcode) {
		case CTO_Always:
			if (pos >= input->length) return 0;
			typebuf[pos++] |= currentMark;
			break;
		case CTO_Syllable:
			/* cycle between SYLLABLE_MARKER_1 and SYLLABLE_MARKER_2 so
			 * we can distinguinsh two consequtive syllables */
			currentMark = syllable_marks[syllable_mark_selector];
			syllable_mark_selector = (syllable_mark_selector + 1) % 2;

			if ((pos + transCharslen) > input->length) return 0;
			for (k = 0; k < transCharslen; k++) typebuf[pos++] |= currentMark;
			break;
		default:
			break;
		}
	}
	return 1;
}

static int
resetsEmphMode(
		widechar c, const TranslationTableHeader *table, const EmphasisClass *emphClass) {
	/* Whether a character cancels word emphasis mode or not. */
	if (emphClass->mode) {
		const TranslationTableCharacter *chardef = getChar(c, table);
		/* the base character of a character belonging to a mode can never cancel the mode
		 */
		if (chardef->attributes & emphClass->mode)
			return 0;
		else {
			const TranslationTableCharacter *ch = chardef;
			if (ch->basechar)
				ch = (TranslationTableCharacter *)&table->ruleArea[ch->basechar];
			while (ch->linked) {
				ch = (TranslationTableCharacter *)&table->ruleArea[ch->linked];
				if ((ch->mode & chardef->mode) == chardef->mode &&
						ch->attributes & emphClass->mode) {
					return 0;
				}
			}
		}
		if (emphClass->mode == CTC_UpperCase) {
			/* characters that are not letter and not capsmodechars cancel capsword mode
			 */
			return !checkCharAttr(c, CTC_Letter | CTC_CapsMode, table);
		} else if (emphClass->mode == CTC_Digit) {
			/* characters that are not digit or litdigit or numericmodechars cancel
			 * numeric mode */
			return !checkCharAttr(c,
					CTC_Digit | CTC_LitDigit | CTC_NumericMode | CTC_MidEndNumericMode,
					table);
		} else {
			/* characters that are not letter cancel other word modes */
			return !checkCharAttr(c, CTC_Letter, table);
		}
	} else {
		if (checkCharAttr(c, CTC_Letter, table)) /* a letter never cancels emphasis */
			return 0;
		const widechar *emphmodechars = table->emphModeChars[emphClass->rule];
		/* by default (if emphmodechars is not declared) only space cancels emphasis */
		if (!emphmodechars[0]) return checkCharAttr(c, CTC_Space, table);
		for (int k = 0; emphmodechars[k]; k++)
			if (c == emphmodechars[k]) return 0;
		return 1;
	}
}

static int
isEmphasizable(
		widechar c, const TranslationTableHeader *table, const EmphasisClass *emphClass) {
	/* Whether emphasis is indicated on a character or not. */
	if (emphClass->mode) {
		/* a character is emphasizable if it belongs to the mode or if it has the same
		 * base as a character that belongs to the mode */
		const TranslationTableCharacter *chardef = getChar(c, table);
		if (chardef->basechar)
			chardef = (TranslationTableCharacter *)&table->ruleArea[chardef->basechar];
		if (chardef->attributes & emphClass->mode) return 1;
		while (chardef->linked) {
			chardef = (TranslationTableCharacter *)&table->ruleArea[chardef->linked];
			if (chardef->attributes & emphClass->mode) return 1;
		}
		return 0;
	} else {
		const widechar *noemphchars = table->noEmphChars[emphClass->rule];
		/* if noemphchars is not declared emphasis is indicated on all characters except
		 * spaces */
		if (!noemphchars[0]) return !checkCharAttr(c, CTC_Space, table);
		for (int k = 0; noemphchars[k]; k++)
			if (c == noemphchars[k]) return 0;
		return 1;
	}
}

static int
isEmphasized(widechar c, const TranslationTableHeader *table,
		const EmphasisClass *emphClass, formtype typeform) {
	/* Whether a character is emphasized or not. */
	if (!isEmphasizable(c, table, emphClass)) return 0;
	if (emphClass->mode)
		return checkCharAttr(c, emphClass->mode, table);
	else
		return typeform & emphClass->typeform;
}

static int
isEmphSpace(
		widechar c, const TranslationTableHeader *table, const EmphasisClass *emphClass) {
	/* For determining word boundaries. */
	/* Note that this is not the only function that is used for this purpose. In
	 * resolveEmphasisWords the beginning and end of words are further refined based on
	 * the isEmphasizable function. */
	const int word_enabled = table->emphRules[emphClass->rule][begWordOffset];
	if (emphClass->mode == CTC_UpperCase) {
		/* The old behavior was that words are determined by spaces. However for some
		 * tables it is a requirement that words are determined based on letters and
		 * capsmodechars. While the latter probably makes most sense, we don't want to
		 * break the old behavior because there is no easy way to achieve it using
		 * table rules. A good middle ground is to let the behavior depend on the
		 * presence of a capsmodechars rule. */
		if (!(word_enabled && table->hasCapsModeChars))
			return checkCharAttr(c, CTC_Space, table);
	}
	return !isEmphasizable(c, table, emphClass) &&
			(!word_enabled || resetsEmphMode(c, table, emphClass));
}

static void
resolveEmphasisBeginEnd(EmphasisInfo *buffer, const EmphasisClass *class,
		const TranslationTableHeader *table, const InString *input,
		const formtype *typebuf, const unsigned int *wordBuffer) {
	/* mark emphasized (capitalized) sections, i.e. sections that */
	/* - start with an emphasized (uppercase) character, */
	/* - extend as long as no unemphasized (lowercase) character is encountered, and */
	/* - do not end with a word that contains no emphasized (uppercase) characters */
	/* in addition, if phrase rules are present, sections are split up as needed so that
	 * they do not end in the middle of a word */

	int last_space = -1;  // position of the last encountered space
	int emph_start = -1;  // position of the first emphasized (uppercase) character after
						  // which no unemphasized (lowercase) character was encountered
	int last_word = -1;	  // position of the first space following the last encountered
						  // character if that character was emphasized (uppercase)
	int emph = 0;		  // whether or not the last encountered character was emphasized
						  // (uppercase) and happened in the current word
	int phrase_enabled = table->emphRules[class->rule][begPhraseOffset];

	for (int i = 0; i < input->length; i++) {
		int isSpace = !(wordBuffer[i] & WORD_CHAR);
		if (isSpace) {
			/* character is a space */
			last_space = i;
			if (emph) {
				last_word = i;
				emph = 0;
			}
		}
		/* if character is an emphasized (uppercase) character, emphasis mode begins or
		 * continues */
		if (!isSpace && isEmphasized(input->chars[i], table, class, typebuf[i])) {
			if (emph_start < 0) emph_start = i;
			emph = 1;
		} else {
			/* else if emphasis mode has begun, it should continue if there are no
			 * unemphasized (lowercase) characters before the next emphasized (uppercase)
			 * character */
			/* characters that cancel emphasis mode are handled later in
			 * resolveEmphasisResets (note that letters that are neither uppercase nor
			 * lowercase do not cancel caps mode) */
			if (!isSpace && isEmphasizable(input->chars[i], table, class)) {
				if (emph_start >= 0) {
					buffer[emph_start].begin |= class->value;
					if (emph) {
						/* a passage can not end on a word without emphasized (uppercase)
						 * characters, so if emphasis did not start inside the current
						 * word, end it after the last word that contained an emphasized
						 * (uppercase) character, and start over from the beginning of the
						 * current word */
						if (phrase_enabled && emph_start < last_space) {
							buffer[last_word].end |= class->value;
							emph_start = -1;
							last_word = -1;
							emph = 0;
							i = last_space;
							continue;
						} else
							/* don't split into two sections if no phrase rules are
							 * present or emphasis started inside the current word */
							buffer[i].end |= class->value;
					} else
						/* current word had no emphasis yet */
						buffer[last_word].end |= class->value;
					emph_start = -1;
					last_word = -1;
					emph = 0;
				}
			}
		}
	}

	/* clean up input->length */
	if (emph_start >= 0) {
		buffer[emph_start].begin |= class->value;
		if (emph)
			buffer[input->length].end |= class->value;
		else
			buffer[last_word].end |= class->value;
	}
}

static void
resolveEmphasisWords(EmphasisInfo *buffer, const EmphasisClass *class,
		const TranslationTableHeader *table, const InString *input,
		unsigned int *wordBuffer) {
	int in_word = 0, in_emp = 0;
	int word_start = -1;  // start position of the current emphasized word section
	int char_cnt = 0;  // number of emphasizable characters within the current emphasized
					   // word section
	int last_char = -1;	 // position of the last emphasizable character
	const TranslationTableOffset *emphRule = table->emphRules[class->rule];
	int letter_defined = emphRule[letterOffset];
	int endphraseafter_defined = emphRule[begPhraseOffset] &&
			(emphRule[endPhraseAfterOffset] || emphRule[endOffset]);

	for (int i = 0; i < input->length; i++) {

		/* check if at beginning of emphasis */
		if (!in_emp)
			if (buffer[i].begin & class->value) {
				in_emp = 1;
				buffer[i].begin &= ~class->value;

				/* emphasis started inside word (and is therefore not a whole word) */
				if (in_word) word_start = i;

				/* emphasis started on space */
				if (!(wordBuffer[i] & WORD_CHAR)) word_start = -1;
			}

		/* check if at end of emphasis */
		if (in_emp)
			if (buffer[i].end & class->value) {
				in_emp = 0;
				buffer[i].end &= ~class->value;
				if (in_word && word_start >= 0) {
					/* if word is one symbol, turn it into a symbol (unless emphletter is
					 * not defined) */
					if (letter_defined && char_cnt == 1)
						buffer[word_start].symbol |= class->value;
					else {
						/* else mark the word start point and, if emphasis ended inside a
						 * word, also mark the end point */
						buffer[word_start].word |= class->value;
						if (wordBuffer[i] & WORD_CHAR) {
							buffer[i].end |= class->value;
							buffer[i].word |= class->value;
						}
					}
				}
			}

		/* check if at beginning of word (first character that is not a space) */
		if (!in_word)
			if (wordBuffer[i] & WORD_CHAR) {
				/* check if word started on a character that is not emphasizable */
				if (isEmphasizable(input->chars[i], table, class)) {
					in_word = 1;
					if (in_emp) word_start = i;
					/* remove WORD_CHAR marks at the end of the previous word */
					for (int j = last_char + 1; j < i; j++) wordBuffer[j] &= ~WORD_CHAR;
					/* also delete possible word end point */
					if (last_char >= 0 && !(buffer[last_char].symbol & class->value)) {
						if ((buffer[last_char].word & class->value) &&
								!(buffer[last_char].end & class->value))
							buffer[last_char].symbol |= class->value;
						for (int j = last_char; j < i - 1; j++)
							if (buffer[j + 1].end & class->value) {
								buffer[j + 1].end &= ~class->value;
								buffer[j + 1].word &= ~class->value;
								break;
							}
					}
				}
			}

		/* check if at end of word (last character that is not a space) */
		if (in_word)
			if (!(wordBuffer[i] & WORD_CHAR)) {
				/* made it through whole word */
				if (in_emp && word_start >= 0) {
					/* if word is one symbol, turn it into a symbol (unless emphletter is
					 * not defined) */
					if (letter_defined && char_cnt == 1)
						buffer[word_start].symbol |= class->value;
					else
						/* else mark it as a word */
						buffer[word_start].word |= class->value;
				}
				in_word = 0;
				word_start = -1;
			}

		/* count characters within the current emphasized word (section) that are
		 * emphasizable */
		if (i == word_start) {
			last_char = i;
			char_cnt = 1;
		} else if (in_word &&
				(endphraseafter_defined /* hack to achieve old behavior of endemphphrase
										 * after: if the last word of the passage ends
										 * with unemphasizable characters, the indicator
										 * is inserted after them  */
						|| isEmphasizable(input->chars[i], table, class))) {
			last_char = i;
			if (in_emp) char_cnt++;
		}
	}

	/* clean up end */
	if (in_emp) {
		buffer[input->length].end &= ~class->value;

		if (in_word)
			if (word_start >= 0) {
				/* if word is one symbol, turn it into a symbol (unless emphletter is not
				 * defined) */
				if (letter_defined && char_cnt == 1)
					buffer[word_start].symbol |= class->value;
				else
					/* else mark it as a word */
					buffer[word_start].word |= class->value;
			}
	}

	/* remove WORD_CHAR marks at the end of the previous word */
	for (int j = last_char + 1; j < input->length; j++) wordBuffer[j] &= ~WORD_CHAR;
	/* also delete possible word end point */
	if (last_char >= 0 && !(buffer[last_char].symbol & class->value)) {
		if ((buffer[last_char].word & class->value) &&
				!(buffer[last_char].end & class->value))
			buffer[last_char].symbol |= class->value;
		for (int j = last_char; j < input->length - 1; j++)
			if (buffer[j + 1].end & class->value) {
				buffer[j + 1].end &= ~class->value;
				buffer[j + 1].word &= ~class->value;
				break;
			}
	}

	/* mark whole words */
	word_start = -1;
	for (int i = 0; i < input->length; i++) {
		if (buffer[i].symbol & class->value) {
			if ((i == 0 || !(wordBuffer[i - 1] & WORD_CHAR)) &&
					(i + 1 == input->length || !(wordBuffer[i + 1] & WORD_CHAR)))
				wordBuffer[i] |= WORD_WHOLE;
		} else if (buffer[i].word & class->value) {
			if (buffer[i].end & class->value) {
				if (word_start >= 0 && wordBuffer[i] & WORD_CHAR)
					wordBuffer[word_start] &= ~WORD_WHOLE;
				word_start = -1;
			} else {
				if (i == 0 || !(wordBuffer[i - 1] & WORD_CHAR))
					wordBuffer[i] |= WORD_WHOLE;
				word_start = i;
			}
		}
	}
}

static void
convertToPassage(const int pass_start, const int pass_end, const int word_start,
		EmphasisInfo *buffer, const EmphasisClass *class,
		const TranslationTableHeader *table, unsigned int *wordBuffer) {
	int i;
	const TranslationTableOffset *emphRule = table->emphRules[class->rule];
	const TranslationTableRule *indicRule;

	for (i = pass_start; i <= pass_end; i++)
		if (wordBuffer[i] & WORD_WHOLE) {
			buffer[i].symbol &= ~class->value;
			buffer[i].word &= ~class->value;
			wordBuffer[i] &= ~WORD_WHOLE;
		}

	buffer[pass_start].begin |= class->value;
	if (brailleIndicatorDefined(emphRule[endOffset], table, &indicRule) ||
			brailleIndicatorDefined(emphRule[endPhraseAfterOffset], table, &indicRule))
		buffer[pass_end].end |= class->value;
	else if (brailleIndicatorDefined(
					 emphRule[endPhraseBeforeOffset], table, &indicRule)) {
		/* if the phrase end indicator is the same as the word indicator, mark it as a
		 * word so that the resolveEmphasisResets code applies */
		const TranslationTableRule *begwordRule;
		if (brailleIndicatorDefined(emphRule[begWordOffset], table, &begwordRule) &&
				indicRule->dotslen == begwordRule->dotslen &&
				!memcmp(&indicRule->charsdots[0], &begwordRule->charsdots[0],
						begwordRule->dotslen * CHARSIZE)) {
			buffer[word_start].word |= class->value;
			/* a passage has only whole emphasized words */
			wordBuffer[word_start] |= WORD_WHOLE;
		} else {
			buffer[word_start].end |= class->value;
		}
	}
}

static void
resolveEmphasisPassages(EmphasisInfo *buffer, const EmphasisClass *class,
		const TranslationTableHeader *table, const InString *input,
		unsigned int *wordBuffer) {
	const TranslationTableOffset *emphRule = table->emphRules[class->rule];
	unsigned int word_cnt = 0;
	int pass_start = -1, pass_end = -1, word_start = -1, in_word = 0, in_pass = 0;
	int i;

	for (i = 0; i < input->length; i++) {
		/* check if at beginning of word (first character that is not a space) */
		if (!in_word)
			if (wordBuffer[i] & WORD_CHAR) {
				in_word = 1;
				/* only whole emphasized words can be part of a passage (in case of caps,
				 * this also includes words without letters, but only if the next word
				 * with letters is a whole word) */
				if (wordBuffer[i] & WORD_WHOLE) {
					if (!in_pass) {
						in_pass = 1;
						pass_start = i;
						pass_end = -1;
						word_cnt = 1;
					} else
						word_cnt++;
					word_start = i;
					continue;
				} else if (in_pass) {
					/* it is a passage only if the number of words is greater than or
					 * equal to the minimum length (lencapsphrase / lenemphphrase) */
					if (word_cnt >= emphRule[lenPhraseOffset])
						if (pass_end >= 0) {
							convertToPassage(pass_start, pass_end, word_start, buffer,
									class, table, wordBuffer);
						}
					in_pass = 0;
				}
			}

		/* check if at end of word */
		if (in_word)
			if (!(wordBuffer[i] & WORD_CHAR)) {
				in_word = 0;
				if (in_pass) pass_end = i;
			}

		if (in_pass)
			if ((buffer[i].begin | buffer[i].end | buffer[i].word | buffer[i].symbol) &
					class->value) {
				if (word_cnt >= emphRule[lenPhraseOffset])
					if (pass_end >= 0) {
						convertToPassage(pass_start, pass_end, word_start, buffer, class,
								table, wordBuffer);
					}
				in_pass = 0;
			}
	}

	if (in_pass) {
		if (word_cnt >= emphRule[lenPhraseOffset]) {
			if (pass_end >= 0) {
				if (in_word) {
					convertToPassage(
							pass_start, i, word_start, buffer, class, table, wordBuffer);
				} else {
					convertToPassage(pass_start, pass_end, word_start, buffer, class,
							table, wordBuffer);
				}
			}
		}
	}
}

static void
resolveEmphasisSingleSymbols(
		EmphasisInfo *buffer, const EmphasisClass *class, const InString *input) {
	int i;

	for (i = 0; i < input->length; i++) {
		if (buffer[i].begin & class->value)
			if (buffer[i + 1].end & class->value) {
				buffer[i].begin &= ~class->value;
				buffer[i + 1].end &= ~class->value;
				buffer[i].symbol |= class->value;
			}
	}
}

static void
resolveEmphasisAllSymbols(EmphasisInfo *buffer, const EmphasisClass *class,
		const TranslationTableHeader *table, formtype *typebuf, const InString *input,
		unsigned int *wordBuffer) {

	/* Mark every emphasized character individually with symbol if begemphword is not
	 * defined (assumes resolveEmphasisWords has not been run) */
	/* Mark every emphasized character individually with symbol if endemphword is not
	 * defined
	 * and emphasis ends within a word (assumes resolveEmphasisWords has been run) */
	/* Note that it is possible that emphletter is also not defined, in which case the
	 * emphasis will not be marked at all. */

	const TranslationTableOffset *emphRule = table->emphRules[class->rule];
	const int begword_enabled = emphRule[begWordOffset];
	const int endword_enabled = emphRule[endWordOffset];

	if (!begword_enabled) {
		int in_emph = 0;
		for (int i = 0; i < input->length; i++) {
			if (in_emph) {
				if (buffer[i].end & class->value) {
					in_emph = 0;
					buffer[i].end &= ~class->value;
				}
			} else {
				if (buffer[i].begin & class->value) {
					in_emph = 1;
					buffer[i].begin &= ~class->value;
				}
			}
			if (in_emph) {
				buffer[i].symbol |= class->value;
			}
		}
	} else if (!endword_enabled) {
		int in_pass = 0, in_word = 0, word_start = -1;
		for (int i = 0; i < input->length; i++) {
			if (in_pass)
				if (buffer[i].end & class->value || buffer[i].word & class->value)
					in_pass = 0;
			if (!in_pass) {
				if (buffer[i].begin & class->value)
					in_pass = 1;
				else {
					if (!in_word)
						if (buffer[i].word & class->value) {
							in_word = 1;
							word_start = i;
						}
					if (in_word) {
						if (buffer[i].word & class->value &&
								buffer[i].end & class->value) {
							in_word = 0;
							if (begword_enabled && !endword_enabled) {
								buffer[i].end &= ~class->value;
								buffer[i].word &= ~class->value;
								buffer[word_start].word &= ~class->value;
								for (int j = word_start; j < i; j++)
									buffer[j].symbol |= class->value;
							}
						} else if (!(wordBuffer[i] & WORD_CHAR)) {
							in_word = 0;
						}
					}
				}
			}
		}
	}
}

static void
resolveEmphasisResets(EmphasisInfo *buffer, const EmphasisClass *class,
		const TranslationTableHeader *table, const InString *input,
		unsigned int *wordBuffer) {
	int in_word = 0, in_pass = 0, word_start = -1, word_reset = 0, letter_cnt = 0,
		pass_end = -1;
	int i;
	int letter_defined = table->emphRules[class->rule][letterOffset];

	for (i = 0; i < input->length; i++) {
		if (in_pass) {
			if (buffer[i].end & class->value)
				in_pass = 0;
			else if (buffer[i].word & class->value) {
				/* the passage is ended with a "endphrase before" indicator and this
				 * indicator is the same as the "begword" indicator (see convertToPassage)
				 */
				in_pass = 0;
				/* remember this position so that if there is a reset later in this word,
				 * we can remove this indicator */
				pass_end = i;
			}
		}
		if (!in_pass) {
			if (buffer[i].begin & class->value) {
				in_pass = 1;
			} else {
				if (!in_word) {
					if (buffer[i].word & class->value) {
						/* deal with case when reset was at beginning of word */
						if (wordBuffer[i] & WORD_RESET ||
								resetsEmphMode(input->chars[i], table, class)) {
							if (!letter_defined)
								/* if emphletter is not defined, use the word indicator */
								;
							else if (pass_end == i)
								/* also use the word indicator if the reset marks the end
								 * of a passage */
								;
							else {
								/* use the symbol indicator symbol for the current
								 * character */
								buffer[i].symbol |= class->value;
								/* move the word indicator to the next character or remove
								 * it altogether if the next character is a space */
								if (wordBuffer[i + 1] & WORD_CHAR) {
									buffer[i + 1].word |= class->value;
									if (wordBuffer[i] & WORD_WHOLE)
										wordBuffer[i + 1] |= WORD_WHOLE;
									if (pass_end == i) pass_end++;
								}
								buffer[i].word &= ~class->value;
								wordBuffer[i] &= ~WORD_WHOLE;
								continue;
							}
						}

						in_word = 1;
						word_start = i;
						letter_cnt = 0;
						word_reset = 0;
					}

					/* it is possible for a character to have been marked as a symbol when
					 * it should not be one */
					else if (buffer[i].symbol & class->value) {
						if (wordBuffer[i] & WORD_RESET ||
								resetsEmphMode(input->chars[i], table, class))
							buffer[i].symbol &= ~class->value;
					}
				}

				if (in_word) {

					/* at end of word */
					if (!(wordBuffer[i] & WORD_CHAR) ||
							(buffer[i].word & class->value &&
									buffer[i].end & class->value)) {
						in_word = 0;

						/* check if symbol */
						if (letter_defined && letter_cnt == 1 && word_start != pass_end) {
							buffer[word_start].symbol |= class->value;
							buffer[word_start].word &= ~class->value;
							wordBuffer[word_start] &= ~WORD_WHOLE;
							buffer[i].end &= ~class->value;
							buffer[i].word &= ~class->value;
						}

						/* if word ended on a reset or last char was a reset, get rid of
						 * end bits */
						if (word_reset || wordBuffer[i] & WORD_RESET ||
								resetsEmphMode(input->chars[i], table, class)) {
							buffer[i].end &= ~class->value;
							buffer[i].word &= ~class->value;
						}

						/* if word ended when it began, get rid of all bits */
						if (i == word_start) {
							wordBuffer[word_start] &= ~WORD_WHOLE;
							buffer[i].end &= ~class->value;
							buffer[i].word &= ~class->value;
						}
					} else {
						/* hit reset */
						if (wordBuffer[i] & WORD_RESET ||
								resetsEmphMode(input->chars[i], table, class)) {

							/* check if symbol is not already resetting */
							if (letter_defined && letter_cnt == 1 &&
									word_start != pass_end) {
								buffer[word_start].symbol |= class->value;
								buffer[word_start].word &= ~class->value;
								wordBuffer[word_start] &= ~WORD_WHOLE;
							}

							/* if reset is a letter or emphmodechar, make it the new
							 * word_start */
							if (!resetsEmphMode(input->chars[i], table, class)) {
								if (word_start == pass_end)
									/* move the word marker that ends the passage to the
									 * current position */
									buffer[pass_end].word &= ~class->value;
								pass_end = -1;
								word_reset = 0;
								word_start = i;
								letter_cnt = 1;
								buffer[i].word |= class->value;
							} else
								word_reset = 1;

							continue;
						}

						if (word_reset) {
							if (word_start == pass_end)
								/* move the word marker that ends the passage to the
								 * current position */
								buffer[pass_end].word &= ~class->value;
							pass_end = -1;
							word_reset = 0;
							word_start = i;
							letter_cnt = 0;
							buffer[i].word |= class->value;
						}

						letter_cnt++;
					}
				}
			}
		}
	}

	/* clean up end */
	if (in_word) {
		/* check if symbol */
		if (letter_defined && letter_cnt == 1 && word_start != pass_end) {
			buffer[word_start].symbol |= class->value;
			buffer[word_start].word &= ~class->value;
			wordBuffer[word_start] &= ~WORD_WHOLE;
			buffer[i].end &= ~class->value;
			buffer[i].word &= ~class->value;
		}

		if (word_reset) {
			buffer[i].end &= ~class->value;
			buffer[i].word &= ~class->value;
		}
	}
}

static void
markEmphases(const TranslationTableHeader *table, const InString *input,
		formtype *typebuf, unsigned int *wordBuffer, EmphasisInfo *emphasisBuffer) {

	/* handle capsnocont */
	if (table->capsNoCont) {
		int caps_cnt = 0;  // number of consecutive characters ending with the current
						   // that are uppercase letters
		for (int i = 0; i < input->length; i++) {
			if (checkCharAttr(input->chars[i], CTC_UpperCase, table)) {
				/* mark two or more consecutive caps with nocont */
				caps_cnt++;
				if (caps_cnt >= 2) {
					typebuf[i] |= no_contract;
					/* also mark the previous one */
					if (caps_cnt == 2) typebuf[i - 1] |= no_contract;
				}
			} else {
				caps_cnt = 0;
			}
		}
	}

	for (int j = 0; j < MAX_EMPH_CLASSES + MAX_MODES; j++) {
		const EmphasisClass *emphClass = j < MAX_EMPH_CLASSES
				? &table->emphClasses[j]
				: &table->modes[j - MAX_EMPH_CLASSES];
		if (!emphClass->value) continue;
		const TranslationTableOffset *emphRule = table->emphRules[emphClass->rule];

		/* clear out previous word markings and mark non-space characters in word buffer
		 */
		for (int i = 0; i < input->length; i++) {
			if (isEmphSpace(input->chars[i], table, emphClass))
				wordBuffer[i] &= ~WORD_CHAR;
			else
				wordBuffer[i] |= WORD_CHAR;
			wordBuffer[i] &= ~WORD_WHOLE;
		}

		/* mark beginning and end points */
		resolveEmphasisBeginEnd(
				emphasisBuffer, emphClass, table, input, typebuf, wordBuffer);

		if (emphRule[begWordOffset]) {
			/* mark word beginning and end points, whole words, and symbols (single
			 * characters) */
			resolveEmphasisWords(emphasisBuffer, emphClass, table, input, wordBuffer);
			if (emphRule[lenPhraseOffset])
				/* remove markings of words that form a passage, and mark the begin and
				 * end of these passages instead */
				resolveEmphasisPassages(
						emphasisBuffer, emphClass, table, input, wordBuffer);
			/* mark where emphasis in a word needs to be retriggered after it was reset */
			resolveEmphasisResets(emphasisBuffer, emphClass, table, input, wordBuffer);
			if (!emphRule[endWordOffset])
				/* if endword is not defined and emphasis ends within a word, mark every
				 * emphasised character individually as symbol */
				resolveEmphasisAllSymbols(
						emphasisBuffer, emphClass, table, typebuf, input, wordBuffer);
		} else if (emphRule[letterOffset]) {
			if (emphRule[begOffset])
				resolveEmphasisSingleSymbols(emphasisBuffer, emphClass, input);
			else
				resolveEmphasisAllSymbols(
						emphasisBuffer, emphClass, table, typebuf, input, wordBuffer);
		}
		if (emphClass->mode) {
			/* only mark if actually a capital letter (don't mark spaces or punctuation).
			 */
			for (int i = 0; i < input->length; i++) {
				if (emphasisBuffer[i].symbol & emphClass->value) {
					if (emphClass->mode == CTC_UpperCase) {
						if (!(typebuf[i] & CAPSEMPH))
							emphasisBuffer[i].symbol &= ~emphClass->value;
					} else {
						if (!checkCharAttr(input->chars[i], emphClass->mode, table))
							emphasisBuffer[i].symbol &= ~emphClass->value;
					}
				}
			}
		}
	}
}

static void
insertEmphasisSymbol(const EmphasisInfo *buffer, const int at, const EmphasisClass *class,
		const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, int *cursorPosition, int *cursorStatus) {
	if (buffer[at].symbol & class->value) {
		const TranslationTableRule *indicRule;
		if (brailleIndicatorDefined(
					table->emphRules[class->rule][letterOffset], table, &indicRule))
			for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
	}
}

static void
insertEmphasisBegin(const EmphasisInfo *buffer, const int at, const EmphasisClass *class,
		const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, int *cursorPosition, int *cursorStatus) {
	const TranslationTableOffset *emphRule = table->emphRules[class->rule];
	const TranslationTableRule *indicRule;
	if (buffer[at].begin & class->value) {
		if (brailleIndicatorDefined(emphRule[begPhraseOffset], table, &indicRule))
			for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
		else if (brailleIndicatorDefined(emphRule[begOffset], table, &indicRule))
			for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
	}

	if (buffer[at].word & class->value
			// && !(buffer[at].begin & class->value)
			&& !(buffer[at].end & class->value)) {
		if (brailleIndicatorDefined(emphRule[begWordOffset], table, &indicRule))
			for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
	}
}

static void
insertEmphasisEnd(const EmphasisInfo *buffer, const int at, const EmphasisClass *class,
		const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, int *cursorPosition, int *cursorStatus) {
	const TranslationTableOffset *emphRule = table->emphRules[class->rule];
	if (buffer[at].end & class->value) {
		const TranslationTableRule *indicRule;
		if (buffer[at].word & class->value) {
			if (brailleIndicatorDefined(emphRule[endWordOffset], table, &indicRule))
				for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, -1,
						pos, input, output, posMapping, cursorPosition, cursorStatus);
		} else {
			if (brailleIndicatorDefined(emphRule[endOffset], table, &indicRule))
				for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, -1,
						pos, input, output, posMapping, cursorPosition, cursorStatus);
			else if (brailleIndicatorDefined(
							 emphRule[endPhraseAfterOffset], table, &indicRule))
				for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, -1,
						pos, input, output, posMapping, cursorPosition, cursorStatus);
			else if (brailleIndicatorDefined(
							 emphRule[endPhraseBeforeOffset], table, &indicRule))
				for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0,
						pos, input, output, posMapping, cursorPosition, cursorStatus);
		}
	}
}

static int
endCount(const EmphasisInfo *buffer, const int at, const EmphasisClass *class) {
	int i, cnt = 1;
	if (!(buffer[at].end & class->value)) return 0;
	for (i = at - 1; i >= 0; i--)
		if (buffer[i].begin & class->value || buffer[i].word & class->value)
			break;
		else
			cnt++;
	return cnt;
}

static int
beginCount(const EmphasisInfo *buffer, const int at, const EmphasisClass *class,
		const TranslationTableHeader *table, const InString *input) {
	if (buffer[at].begin & class->value) {
		int i, cnt = 1;
		for (i = at + 1; i < input->length; i++)
			if (buffer[i].end & class->value)
				break;
			else
				cnt++;
		return cnt;
	} else if (buffer[at].word & class->value) {
		int i, cnt = 1;
		for (i = at + 1; i < input->length; i++)
			if (buffer[i].end & class->value)
				break;
			else if (checkCharAttr(input->chars[i], CTC_SeqDelimiter, table))
				break;
			else if (isEmphSpace(input->chars[i], table, class))
				break;
			else
				cnt++;
		return cnt;
	}
	return 0;
}

static void
insertEmphasesAt(int begin, int end, int caps, int other, const int at,
		const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, const EmphasisInfo *emphasisBuffer,
		int *cursorPosition, int *cursorStatus) {

	/* The order of inserting the end symbols must be the reverse
	 * of the insertions of the begin symbols so that they will
	 * nest properly when multiple emphases start and end at
	 * the same place */
	// TODO: ordering with partial word

	if (end && caps)
		for (int i = 0; i < MAX_MODES; i++) {
			const EmphasisClass *emphClass = &table->modes[i];
			if (!emphClass->value) continue;
			if ((emphasisBuffer[at].begin | emphasisBuffer[at].end |
						emphasisBuffer[at].word | emphasisBuffer[at].symbol) &
					emphClass->value)
				insertEmphasisEnd(emphasisBuffer, at, emphClass, table, pos, input,
						output, posMapping, cursorPosition, cursorStatus);
		}

	if (end && other) {
		int type_counts[MAX_EMPH_CLASSES];

		/* end bits */
		for (int i = 0; i < MAX_EMPH_CLASSES; i++) {
			const EmphasisClass *emphClass = &table->emphClasses[i];
			if (!emphClass->value)
				type_counts[i] = 0;
			else
				type_counts[i] = endCount(emphasisBuffer, at, emphClass);
		}

		while (1) {
			int min = -1;
			for (int i = 0; i < MAX_EMPH_CLASSES; i++)
				if (type_counts[i] > 0)
					if (min < 0 || type_counts[i] < type_counts[min]) min = i;
			if (min < 0) break;
			type_counts[min] = 0;
			insertEmphasisEnd(emphasisBuffer, at, &table->emphClasses[min], table, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
		}
	}

	if (begin && other) {
		int type_counts[MAX_EMPH_CLASSES];

		/* begin and word bits */
		for (int i = 0; i < MAX_EMPH_CLASSES; i++) {
			const EmphasisClass *emphClass = &table->emphClasses[i];
			if (!emphClass->value)
				type_counts[i] = 0;
			else
				type_counts[i] = beginCount(emphasisBuffer, at, emphClass, table, input);
		}

		while (1) {
			int max = MAX_EMPH_CLASSES - 1;
			for (int i = MAX_EMPH_CLASSES - 1; i >= 0; i--)
				if (type_counts[max] < type_counts[i]) max = i;
			if (!type_counts[max]) break;
			type_counts[max] = 0;
			insertEmphasisBegin(emphasisBuffer, at, &table->emphClasses[max], table, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
		}

		/* symbol bits */
		for (int i = MAX_EMPH_CLASSES - 1; i >= 0; i--)
			if ((emphasisBuffer[at].begin | emphasisBuffer[at].end |
						emphasisBuffer[at].word | emphasisBuffer[at].symbol) &
					table->emphClasses[i].value)
				insertEmphasisSymbol(emphasisBuffer, at, &table->emphClasses[i], table,
						pos, input, output, posMapping, cursorPosition, cursorStatus);
	}

	if (begin && caps) {

		/* insert capitalization last so it will be closest to word */
		/* other mode indicators are inserted so that those who are defined first are
		 * closest to word */
		for (int i = MAX_MODES - 1; i >= 0; i--) {
			const EmphasisClass *emphClass = &table->modes[i];
			if (!emphClass->value) continue;
			if ((emphasisBuffer[at].begin | emphasisBuffer[at].end |
						emphasisBuffer[at].word | emphasisBuffer[at].symbol) &
					emphClass->value) {
				insertEmphasisBegin(emphasisBuffer, at, emphClass, table, pos, input,
						output, posMapping, cursorPosition, cursorStatus);
				insertEmphasisSymbol(emphasisBuffer, at, emphClass, table, pos, input,
						output, posMapping, cursorPosition, cursorStatus);
			}
		}
	}
}

static void
checkNumericMode(const TranslationTableHeader *table, int pos, const InString *input,
		OutString *output, int *posMapping, int *cursorPosition, int *cursorStatus,
		int *dontContract, int *numericMode) {
	/* check if numeric mode is active and insert number sign and nocontract sign when
	 * needed */

	int i;
	const TranslationTableRule *indicRule;
	if (!brailleIndicatorDefined(table->numberSign, table, &indicRule)) return;

	/* not in numeric mode */
	if (!*numericMode) {
		if (checkCharAttr(input->chars[pos], CTC_Digit | CTC_LitDigit, table)) {
			*numericMode = 1;
			/* if the nocontractsign is defined and it is the same as the nonumsign then
			   disable contraction */
			if (isIndicatorEqual(table->noContractSign, table->noNumberSign, table))
				*dontContract = 1;
			for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0, pos,
					input, output, posMapping, cursorPosition, cursorStatus);
		} else if (checkCharAttr(input->chars[pos], CTC_NumericMode, table)) {
			for (i = pos + 1; i < input->length; i++) {
				if (checkCharAttr(input->chars[i], CTC_Digit | CTC_LitDigit, table)) {
					*numericMode = 1;
					for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen,
							0, pos, input, output, posMapping, cursorPosition,
							cursorStatus);
					break;
				} else if (!checkCharAttr(input->chars[i], CTC_NumericMode, table))
					break;
			}
		}
	}

	/* in numeric mode */
	else {
		if (!checkCharAttr(input->chars[pos],
					CTC_Digit | CTC_LitDigit | CTC_NumericMode | CTC_MidEndNumericMode,
					table)) {
			*numericMode = 0;
			if (brailleIndicatorDefined(table->noNumberSign, table, &indicRule))
				if (checkCharAttr(input->chars[pos], CTC_NumericNoContract, table))
					for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen,
							0, pos, input, output, posMapping, cursorPosition,
							cursorStatus);
		}
	}
}

static int
translateString(const TranslationTableHeader *table, int mode, int currentPass,
		const InString *input, OutString *output, int *posMapping, formtype *typebuf,
		unsigned char *srcSpacing, unsigned char *destSpacing, unsigned int *wordBuffer,
		EmphasisInfo *emphasisBuffer, int haveEmphasis, int *realInlen,
		int *cursorPosition, int *cursorStatus, int compbrlStart, int compbrlEnd) {
	int pos;
	int transOpcode;
	int prevTransOpcode;
	const TranslationTableRule *transRule;
	int transCharslen;
	int passCharDots;
	const widechar *passInstructions;
	int passIC; /* Instruction counter */
	PassRuleMatch patternMatch;
	TranslationTableRule *groupingRule;
	widechar groupingOp;
	int numericMode;
	int dontContract;
	LastWord lastWord;
	int insertEmphasesFrom;
	TranslationTableCharacter *curCharDef;
	int repwordStart;
	int repwordLength;
	const InString *origInput = input;
	/* Main translation routine */
	int k;
	translation_direction = 1;
	markSyllables(table, input, typebuf);
	numericMode = 0;
	lastWord = (LastWord){ 0, 0, 0 };
	dontContract = 0;
	prevTransOpcode = CTO_None;
	pos = output->length = 0;
	int posIncremented = 1;
	insertEmphasesFrom = 0;
	_lou_resetPassVariables();
	if (typebuf && capsletterDefined(table))
		for (k = 0; k < input->length; k++)
			if (checkCharAttr(input->chars[k], CTC_UpperCase, table))
				typebuf[k] |= CAPSEMPH;

	markEmphases(table, input, typebuf, wordBuffer, emphasisBuffer);

	while (pos < input->length) { /* the main translation loop */
		if (pos >= compbrlStart && pos < compbrlEnd) {
			int cs = 2;	 // cursor status for this call
			if (!doCompTrans(pos, compbrlEnd, table, &pos, input, output, posMapping,
						emphasisBuffer, &transRule, cursorPosition, &cs, mode))
				goto failure;
			if (pos > 0 && checkCharAttr(input->chars[pos - 1], CTC_Space, table))
				lastWord = (LastWord){ pos, output->length, insertEmphasesFrom };
			continue;
		}
		TranslationTableCharacterAttributes beforeAttributes;
		setBefore(table, pos, input, &beforeAttributes);
		if (pos >= input->length) break;

		if (!dontContract) dontContract = typebuf[pos] & no_contract;
		if (typebuf[pos] & no_translate) {
			if (input->chars[pos] < 32 || input->chars[pos] > 126) goto failure;
			widechar d = LOU_DOTS;
			TranslationTableOffset offset = getChar(input->chars[pos], table)->otherRules;
			while (offset) {
				const TranslationTableRule *r =
						(TranslationTableRule *)&table->ruleArea[offset];
				if (r->opcode >= CTO_Space && r->opcode < CTO_UpLow && r->dotslen == 1) {
					d = r->charsdots[1];
					break;
				}
				offset = r->charsnext;
			}
			if (!for_updatePositions(&d, 1, 1, 0, pos, input, output, posMapping,
						cursorPosition, cursorStatus))
				goto failure;
			pos++;
			posIncremented = 1;
			insertEmphasesFrom = pos;
			continue;
		}
		repwordLength = 0;
		for_selectRule(table, pos, *output, posMapping, mode, input, typebuf,
				emphasisBuffer, &transOpcode, prevTransOpcode, &transRule, &transCharslen,
				&passCharDots, &passInstructions, &passIC, &patternMatch, posIncremented,
				*cursorPosition, &repwordLength, dontContract, compbrlStart, compbrlEnd,
				beforeAttributes, &curCharDef, &groupingRule, &groupingOp);

		switch (transOpcode) /* Rules that pre-empt context and swap */
		{
		case CTO_CompBrl:
		case CTO_Literal:
			if (!doCompbrl(table, &pos, input, output, posMapping, emphasisBuffer,
						&transRule, cursorPosition, cursorStatus, &lastWord,
						&insertEmphasesFrom, mode))
				goto failure;
			continue;
		default:
			break;
		}

		/* Skip repword separator to make caps/emph indicators appear before repword
		 * indicator */
		if (repwordLength) pos += transCharslen;

		for (int at = insertEmphasesFrom; at <= pos; at++) {
			/* insert caps end indicator */
			insertEmphasesAt(0, 1, 1, 0, at, table, pos, input, output, posMapping,
					emphasisBuffer, cursorPosition, cursorStatus);
			if (haveEmphasis) {
				/* insert emphasis end indicator */
				insertEmphasesAt(0, 1, 0, 1, at, table, pos, input, output, posMapping,
						emphasisBuffer, cursorPosition, cursorStatus);
				/* insert emphasis start indicator */
				insertEmphasesAt(1, 0, 0, 1, at, table, pos, input, output, posMapping,
						emphasisBuffer, cursorPosition, cursorStatus);
			}
			if (at < pos)
				insertEmphasesAt(1, 0, 1, 0, at, table, pos, input, output, posMapping,
						emphasisBuffer, cursorPosition, cursorStatus);
		}
		insertEmphasesFrom = pos + 1;
		/* insert grade 1 mode indicator (nocontractsign) before contraction */
		if (transOpcode == CTO_Contraction) {
			const TranslationTableRule *indicRule;
			if (brailleIndicatorDefined(table->noContractSign, table, &indicRule))
				for_updatePositions(&indicRule->charsdots[0], 0, indicRule->dotslen, 0,
						pos, input, output, posMapping, cursorPosition, cursorStatus);
		}
		/* insert letter sign */
		if (!insertLetterSign(table, pos, input, output, posMapping, transOpcode,
					cursorPosition, cursorStatus, beforeAttributes))
			goto failure;
		/* insert caps start indicator */
		insertEmphasesAt(1, 0, 1, 0, pos, table, pos, input, output, posMapping,
				emphasisBuffer, cursorPosition, cursorStatus);
		/* insert number sign (not if numericmodechars, midnumericmodechars or
		 * numericnocontchars has been defined) */
		if (!table->usesNumericMode)
			if (!insertNumberSign(table, pos, input, output, posMapping, prevTransOpcode,
						cursorPosition, cursorStatus, beforeAttributes))
				goto failure;
		/* insert number sign and number cancel sign (nocontractsign) (only if
		 * numericmodechars, midnumericmodechars or numericnocontchars has been defined)
		 */
		if (table->usesNumericMode)
			checkNumericMode(table, pos, input, output, posMapping, cursorPosition,
					cursorStatus, &dontContract, &numericMode);

		if (transOpcode == CTO_Context ||
				(posIncremented &&
						findForPassRule(table, pos, currentPass, input, &transOpcode,
								&transRule, &transCharslen, &passCharDots,
								&passInstructions, &passIC, &patternMatch, &groupingRule,
								&groupingOp))) {
			posIncremented = 1;
			switch (transOpcode) {
			case CTO_Context: {
				const InString *inputBefore = input;
				int posBefore = pos;
				if (appliedRules != NULL && appliedRulesCount < maxAppliedRules)
					appliedRules[appliedRulesCount++] = transRule;
				if (!passDoAction(table, &input, output, posMapping, transOpcode,
							&transRule, passCharDots, passInstructions, passIC, &pos,
							patternMatch, cursorPosition, cursorStatus, groupingRule,
							groupingOp, mode))
					goto failure;
				if (input->bufferIndex != inputBefore->bufferIndex &&
						inputBefore->bufferIndex != origInput->bufferIndex)
					releaseStringBuffer(inputBefore->bufferIndex);
				if (pos == posBefore) posIncremented = 0;
				continue;
			}
			default:
				break;
			}
		} else {
			if (appliedRules != NULL && appliedRulesCount < maxAppliedRules)
				appliedRules[appliedRulesCount++] = transRule;
			posIncremented = 1;
		}

		/* Processing before replacement */

		/* check if leaving no contraction (grade 1) mode */
		if (checkCharAttr(input->chars[pos], CTC_SeqDelimiter | CTC_Space, table))
			dontContract = 0;

		switch (transOpcode) {
		case CTO_EndNum:
			if (table->letterSign && checkCharAttr(input->chars[pos], CTC_Letter, table))
				output->length--;
			break;
		case CTO_Repeated:
		case CTO_Space:
			dontContract = 0;
			break;
		case CTO_LargeSign:
			if (prevTransOpcode == CTO_LargeSign) {
				int hasEndSegment = 0;
				while (output->length > 0 &&
						checkDotsAttr(
								output->chars[output->length - 1], CTC_Space, table)) {
					if (output->chars[output->length - 1] == LOU_ENDSEGMENT) {
						hasEndSegment = 1;
					}
					output->length--;
				}
				if (hasEndSegment != 0) {
					output->chars[output->length] = 0xffff;
					output->length++;
				}
			}
			break;
		case CTO_DecPoint:
			if (!table->usesNumericMode && table->numberSign) {
				TranslationTableRule *numRule =
						(TranslationTableRule *)&table->ruleArea[table->numberSign];
				if (!for_updatePositions(&numRule->charsdots[numRule->charslen],
							numRule->charslen, numRule->dotslen, 0, pos, input, output,
							posMapping, cursorPosition, cursorStatus))
					goto failure;
			}
			transOpcode = CTO_MidNum;
			break;
		case CTO_NoCont:
			if (!dontContract)
				doNocont(table, &pos, output, mode, input, &lastWord, &dontContract,
						&insertEmphasesFrom);
			continue;
		case CTO_RepWord:
		case CTO_RepEndWord:
			repwordStart = pos - transCharslen - repwordLength;
			break;
		default:
			break;
		} /* end of action */

		/* replacement processing */
		switch (transOpcode) {
		case CTO_Replace:
			pos += transCharslen;
			if (!putCharacters(&transRule->charsdots[transCharslen], transRule->dotslen,
						table, pos, input, output, posMapping, cursorPosition,
						cursorStatus, mode))
				goto failure;
			break;
		case CTO_None:
			/* no definition or translation rules found for this character, but it may be
			 * based on another character */
			if (!putCharacter(input->chars[pos], table, pos, input, output, posMapping,
						cursorPosition, cursorStatus, mode))
				goto failure;
			pos++;
			break;
		default: {
			const widechar *dots = &transRule->charsdots[transCharslen];
			int dotslen = transRule->dotslen;
			if (transOpcode == CTO_RepEndWord) {
				int k;
				for (k = 1; dots[k] != ','; k++)
					;
				k++;
				dots = &dots[k];
				dotslen -= k;
			}
			if (dotslen) {
				if (repwordLength) {
					/* repword sepatator is already skipped */
					if (!for_updatePositions(dots, 0, dotslen, 0, pos, input, output,
								posMapping, cursorPosition, cursorStatus))
						goto failure;
				} else {
					if (!for_updatePositions(dots, transCharslen, dotslen, 0, pos, input,
								output, posMapping, cursorPosition, cursorStatus))
						goto failure;
					pos += transCharslen;
				}
			} else {
				for (k = 0; k < transCharslen; k++) {
					if (!putCharacter(input->chars[pos], table, pos, input, output,
								posMapping, cursorPosition, cursorStatus, mode))
						goto failure;
					pos++;
				}
			}
			break;
		}
		}

		/* processing after replacement */
		switch (transOpcode) {
		case CTO_Repeated: {
			/* Skip repeated characters. */
			int srclim = input->length - transCharslen;
			if (mode & (compbrlAtCursor | compbrlLeftCursor) && compbrlStart < srclim)
				/* Don't skip characters from compbrlStart onwards. */
				srclim = compbrlStart - 1;
			while ((pos <= srclim) &&
					compareChars(&transRule->charsdots[0], &input->chars[pos],
							transCharslen, table)) {
				if (!*cursorStatus && pos <= *cursorPosition &&
						*cursorPosition < pos + transCharslen) {
					*cursorStatus = 1;
					*cursorPosition = output->length - 1;
				}
				pos += transCharslen;
			}
			break;
		}
		case CTO_RepEndWord: {
			/* Go back and insert dots at repwordStart and update posMapping accordingly
			 */
			const widechar *dots = &transRule->charsdots[transCharslen];
			int dotslen;
			for (dotslen = 1; dots[dotslen] != ','; dotslen++)
				;
			if ((output->length + dotslen) > output->maxlength) goto failure;
			int k;
			for (k = output->length - 1; k >= 0; k--)
				if (posMapping[k] >= repwordStart) {
					output->chars[k + dotslen] = output->chars[k];
					posMapping[k + dotslen] = posMapping[k];
				} else
					break;
			k++;
			memcpy(&output->chars[k], dots, dotslen * sizeof(*output->chars));
			for (int l = 0; l < dotslen; l++) posMapping[k + l] = posMapping[k];
			output->length += dotslen;
			if (*cursorStatus && *cursorPosition >= k) *cursorPosition += dotslen;
		}
		case CTO_RepWord: {
			/* Skip repeated characters. */
			int srclim = input->length;
			if (mode & (compbrlAtCursor | compbrlLeftCursor) && compbrlStart < srclim)
				/* Don't skip characters from compbrlStart onwards. */
				srclim = compbrlStart;
			/* Skip first and subsequent repetitions */
			/* Loop body is be executed at least once. */
			int firstRep = 1;
			while (pos + repwordLength <= srclim &&
					compareChars(&input->chars[repwordStart], &input->chars[pos],
							repwordLength, table)) {
				/* Check that capitalisation and emphasis do not change within or in
				 * between subsequent repetitions. It is allowed to change right before
				 * the first repetition because that can be indicated. That it does not
				 * change within the first repetition is already checked in
				 * isRepeatedWord. */
				if (!firstRep &&
						checkEmphasisChange(pos - 1, repwordLength, emphasisBuffer))
					break;
				if (!*cursorStatus && *cursorPosition >= pos - transCharslen &&
						*cursorPosition < pos + repwordLength) {
					*cursorStatus = 1;
					*cursorPosition = output->length - 1;
				}
				pos += repwordLength;
				if (pos + transCharslen <= srclim &&
						!memcmp(transRule->charsdots, &input->chars[pos],
								transCharslen * sizeof(*transRule->charsdots)))
					pos += transCharslen;
				else {
					pos += transCharslen;
					break;
				}
				firstRep = 0;
			}
			pos -= transCharslen;
			break;
		}
		case CTO_JoinNum:
		case CTO_JoinableWord:
			while (pos < input->length &&
					checkCharAttr(input->chars[pos], CTC_Space, table) &&
					input->chars[pos] != LOU_ENDSEGMENT)
				pos++;
			break;
		default:
			break;
		}
		if (((pos > 0) && checkCharAttr(input->chars[pos - 1], CTC_Space, table) &&
					(transOpcode != CTO_JoinableWord))) {
			lastWord = (LastWord){ pos, output->length, insertEmphasesFrom };
		}
		if (srcSpacing != NULL && srcSpacing[pos] >= '0' && srcSpacing[pos] <= '9')
			destSpacing[output->length] = srcSpacing[pos];
		if ((transOpcode >= CTO_Always && transOpcode <= CTO_None) ||
				(transOpcode >= CTO_Digit && transOpcode <= CTO_LitDigit))
			prevTransOpcode = transOpcode;
	}

	for (int at = insertEmphasesFrom; at <= pos; at++) {
		/* insert caps end indicator */
		insertEmphasesAt(0, 1, 1, 0, at, table, pos, input, output, posMapping,
				emphasisBuffer, cursorPosition, cursorStatus);
		if (haveEmphasis) {
			/* insert emphasis end indicator */
			insertEmphasesAt(0, 1, 0, 1, at, table, pos, input, output, posMapping,
					emphasisBuffer, cursorPosition, cursorStatus);
			/* insert emphasis start indicator */
			insertEmphasesAt(1, 0, 0, 1, at, table, pos, input, output, posMapping,
					emphasisBuffer, cursorPosition, cursorStatus);
		}
		/* insert caps start indicator */
		insertEmphasesAt(1, 0, 1, 0, at, table, pos, input, output, posMapping,
				emphasisBuffer, cursorPosition, cursorStatus);
	}

failure:
	if (lastWord.outPos != 0 && pos < input->length &&
			!checkCharAttr(input->chars[pos], CTC_Space, table)) {
		pos = lastWord.inPos;
		output->length = lastWord.outPos;
	}
	if (pos < input->length) {
		while (checkCharAttr(input->chars[pos], CTC_Space, table))
			if (++pos == input->length) break;
	}
	*realInlen = pos;
	if (input->bufferIndex != origInput->bufferIndex)
		releaseStringBuffer(input->bufferIndex);
	return 1;
} /* first pass translation completed */

static int
isHyphen(const TranslationTableHeader *table, widechar c) {
	TranslationTableRule *rule;
	TranslationTableOffset offset = getChar(c, table)->otherRules;
	while (offset) {
		rule = (TranslationTableRule *)&table->ruleArea[offset];
		if (rule->opcode == CTO_Hyphen) return 1;
		offset = rule->dotsnext;
	}
	return 0;
}

/**
 * Hyphenate an input string which can either be text (mode = 0) or braille (mode = 1). If
 * the input is braille, back-translation will be performed with `tableList'. The input
 * string can contain any character (even space), but only break points within words
 * (between letters) are considered. If the string can not be broken before the character
 * at index k, the value of `hyphens[k]' is '0'. If it can be broken by inserting a hyphen
 * at the break point, the value is '1'. If it can be broken without adding a hyphen, the
 * value is '2'.
 */
int EXPORT_CALL
lou_hyphenate(const char *tableList, const widechar *inbuf, int inlen, char *hyphens,
		int mode) {
#define HYPHSTRING 100
	const TranslationTableHeader *table;
	widechar textBuffer[HYPHSTRING];
	char *textHyphens;
	int *inputPos;
	int k;
	int textLen;
	int wordStart;
	table = lou_getTable(tableList);
	if (table == NULL || inbuf == NULL || hyphens == NULL ||
			table->hyphenStatesArray == 0 || inlen >= HYPHSTRING)
		return 0;
	if (mode != 0) {
		int brailleLen = inlen;
		textLen = HYPHSTRING;
		inputPos = malloc(textLen * sizeof(int));
		if (!lou_backTranslate(tableList, inbuf, &brailleLen, textBuffer, &textLen, NULL,
					NULL, NULL, inputPos, NULL, 0)) {
			free(inputPos);
			return 0;
		}
		textHyphens = malloc((textLen + 1) * sizeof(char));
	} else {
		memcpy(textBuffer, inbuf, CHARSIZE * inlen);
		textLen = inlen;
		textHyphens = hyphens;
	}

	// initialize hyphens array
	for (k = 0; k < textLen; k++) textHyphens[k] = '0';
	textHyphens[k] = 0;

	// for every word part
	for (wordStart = 0;;) {
		int wordEnd;
		// find start of word
		for (; wordStart < textLen; wordStart++)
			if ((getChar(textBuffer[wordStart], table))->attributes & CTC_Letter) break;
		if (wordStart == textLen) break;
		// find end of word
		for (wordEnd = wordStart + 1; wordEnd < textLen; wordEnd++)
			if (!((getChar(textBuffer[wordEnd], table))->attributes & CTC_Letter)) break;
		// hyphenate
		if (!hyphenateWord(&textBuffer[wordStart], wordEnd - wordStart,
					&textHyphens[wordStart], table))
			return 0;
		// normalize to '0', '1' or '2'
		if (wordStart >= 2 && isHyphen(table, textBuffer[wordStart - 1]) &&
				((getChar(textBuffer[wordStart - 2], table))->attributes & CTC_Letter))
			textHyphens[wordStart] = '2';
		else
			textHyphens[wordStart] = '0';
		for (k = wordStart + 1; k < wordEnd; k++)
			if (textHyphens[k] & 1)
				textHyphens[k] = '1';
			else
				textHyphens[k] = '0';
		if (wordEnd == textLen) break;
		textHyphens[wordEnd] = '0';	 // because hyphenateWord sets it to 0
		wordStart = wordEnd + 1;
	}

	// map hyphen positions if the input was braille
	if (mode != 0) {
		for (k = 0; k < inlen; k++) hyphens[k] = '0';
		hyphens[k] = 0;
		int prevPos = -1;
		for (k = 0; k < textLen; k++) {
			int braillePos = inputPos[k];
			if (braillePos > inlen || braillePos < 0) break;
			if (braillePos > prevPos) {
				hyphens[braillePos] = textHyphens[k];
				prevPos = braillePos;
			}
		}
		free(textHyphens);
		free(inputPos);
	}
	return 1;
}

int EXPORT_CALL
lou_dotsToChar(
		const char *tableList, widechar *inbuf, widechar *outbuf, int length, int mode) {
	const DisplayTableHeader *table;
	int k;
	widechar dots;
	if (tableList == NULL || inbuf == NULL || outbuf == NULL) return 0;

	table = _lou_getDisplayTable(tableList);
	if (table == NULL || length <= 0) return 0;
	for (k = 0; k < length; k++) {
		dots = inbuf[k];
		if (!(dots & LOU_DOTS) &&
				(dots & 0xff00) == LOU_ROW_BRAILLE) /* Unicode braille */
			dots = (dots & 0x00ff) | LOU_DOTS;
		outbuf[k] = _lou_getCharForDots(dots, table);
		// assume that if NUL character is returned, it's because the display table has no
		// mapping for the dot pattern (not because it maps to NUL)
		if (outbuf[k] == '\0') outbuf[k] = ' ';
	}
	return 1;
}

int EXPORT_CALL
lou_charToDots(const char *tableList, const widechar *inbuf, widechar *outbuf, int length,
		int mode) {
	const DisplayTableHeader *table;
	int k;
	if (tableList == NULL || inbuf == NULL || outbuf == NULL) return 0;

	table = _lou_getDisplayTable(tableList);
	if (table == NULL || length <= 0) return 0;
	for (k = 0; k < length; k++)
		if ((mode & ucBrl))
			outbuf[k] = ((_lou_getDotsForChar(inbuf[k], table) & 0xff) | LOU_ROW_BRAILLE);
		else
			outbuf[k] = _lou_getDotsForChar(inbuf[k], table);
	return 1;
}
