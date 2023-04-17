/* liblouis Braille Translation and Back-Translation Library

   Copyright (C) 2017 Bert Frees

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "internal.h"

static const TranslationTableHeader *table;
static const DisplayTableHeader *displayTable;

extern void
loadTable(const char *tableList) {
	_lou_getTable(tableList, tableList, &table, &displayTable);
}

extern int
hyphenationEnabled() {
	return table->hyphenStatesArray;
}

extern int
isLetter(widechar c) {
	static unsigned long int hash;
	static TranslationTableOffset offset;
	static TranslationTableCharacter *character;
	hash = _lou_charHash(c);
	offset = table->characters[hash];
	while (offset) {
		character = (TranslationTableCharacter *)&table->ruleArea[offset];
		if (character->value == c) return character->attributes & CTC_Letter;
		offset = character->next;
	}
	return 0;
}

extern widechar
toLowercase(widechar c) {
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

extern void
toDotPattern(widechar *braille, char *pattern) {
	int length;
	widechar *dots;
	int i;
	for (length = 0; braille[length]; length++)
		;
	dots = (widechar *)malloc((length + 1) * sizeof(widechar));
	for (i = 0; i < length; i++) dots[i] = _lou_getDotsForChar(braille[i], displayTable);
	strcpy(pattern, _lou_showDots(dots, length));
	free(dots);
}

extern int
printRule(TranslationTableRule *rule, widechar *rule_string) {
	switch (rule->opcode) {
	case CTO_Context:
	case CTO_Correct:
	case CTO_SwapCd:
	case CTO_SwapDd:
	case CTO_Pass2:
	case CTO_Pass3:
	case CTO_Pass4:
		return 0;
	default: {
		int l = 0;
		if (rule->nocross)
			for (char *c = "nocross "; *c; c++) rule_string[l++] = *c;
		const char *opcode = _lou_findOpcodeName(rule->opcode);
		for (size_t k = 0; k < strlen(opcode); k++) rule_string[l++] = opcode[k];
		rule_string[l++] = '\t';
		for (int k = 0; k < rule->charslen; k++) rule_string[l++] = rule->charsdots[k];
		rule_string[l++] = '\t';
		for (int k = 0; k < rule->dotslen; k++) {
			rule_string[l] = _lou_getCharForDots(
					rule->charsdots[rule->charslen + k], displayTable);
			if (rule_string[l] == '\0') {
				// if a dot pattern can not be displayed, print an error message
				char *message = (char *)malloc(50 * sizeof(char));
				sprintf(message, "ERROR: provide a display rule for dots %s",
						_lou_showDots(&rule->charsdots[rule->charslen + k], 1));
				l = 0;
				while (message[l]) {
					rule_string[l] = message[l];
					l++;
				}
				rule_string[l++] = '\0';
				free(message);
				return 1;
			}
			l++;
		}
		rule_string[l++] = '\0';
		return 1;
	}
	}
}

#define DEBUG 0

#if DEBUG
#define debug(fmt, ...)                                                     \
	do {                                                                    \
		if (DEBUG) printf("%*s" fmt "\n", debug_indent, "", ##__VA_ARGS__); \
	} while (0)
#else
#define debug(fmt, ...)
#endif

static int
find_matching_rules(widechar *text, int text_len, widechar *braille, int braille_len,
		char *data, int clear_data) {
	TranslationTableOffset offset;
	TranslationTableRule *rule;
	TranslationTableCharacter *character;
	char *data_save;
	int hash_len, k;
#if DEBUG
	static int initial_text_len = 0;
	int debug_indent = 0;
	if (data[-1] == '^') {
		initial_text_len = text_len;
		for (k = 0; k < text_len; k++) printf("%c", text[k]);
		printf(" <=> ");
		for (k = 0; k < braille_len; k++) printf("%c", braille[k]);
		printf("\n");
	} else
		debug_indent = initial_text_len - text_len;
#endif

	/* finish */
	if (text_len == 0 && braille_len == 0) {
		data[-1] = '$';
		return 1;
	}

	/* save data */
	data_save = (char *)malloc(text_len * sizeof(char));
	memcpy(data_save, data, text_len);

	for (k = 0; k < text_len; k++)
		if (data[k] == ')')
			data[k] = '>';
		else if (clear_data)
			data[k] = '-';
	debug("%s", data);

	/* iterate over rules */
	for (hash_len = 2; hash_len >= 1; hash_len--) {
		offset = 0;
		switch (hash_len) {
		case 2:
			if (text_len < 2) break;
			offset = table->forRules[_lou_stringHash(text, 1, table)];
			break;
		case 1:
			offset = table->characters[_lou_charHash(text[0])];
			while (offset) {
				character = (TranslationTableCharacter *)&table->ruleArea[offset];
				if (character->value == text[0]) {
					offset = character->otherRules;
					break;
				} else
					offset = character->next;
			}
		}
		while (offset) {
			rule = (TranslationTableRule *)&table->ruleArea[offset];
#if DEBUG
			widechar print_string[128];
			printRule(rule, print_string);
			printf("%*s=> ", debug_indent, "");
			for (k = 0; print_string[k]; k++) printf("%c", print_string[k]);
			printf("\n");
#endif

			/* select rule */
			if (rule->charslen == 0 || rule->dotslen == 0) goto next_rule;
			if (rule->charslen > text_len) goto next_rule;
			switch (rule->opcode) {
			case CTO_WholeWord:
				if (data[-1] == '^' && rule->charslen == text_len) break;
				goto next_rule;
			case CTO_SuffixableWord:
				if (data[-1] == '^') break;
				goto next_rule;
			case CTO_PrefixableWord:
				if (rule->charslen == text_len) break;
				goto next_rule;
			case CTO_BegWord:
				if (data[-1] == '^' && rule->charslen < text_len) break;
				goto next_rule;
			case CTO_BegMidWord:
				if (rule->charslen < text_len) break;
				goto next_rule;
			case CTO_MidWord:
				if (data[-1] != '^' && rule->charslen < text_len) break;
				goto next_rule;
			case CTO_MidEndWord:
				if (data[-1] != '^') break;
				goto next_rule;
			case CTO_EndWord:
				if (data[-1] != '^' && rule->charslen == text_len) break;
				goto next_rule;
			case CTO_Letter:
			case CTO_UpperCase:
			case CTO_LowerCase:
			case CTO_Punctuation:
			case CTO_Always:
				break;
			default:
				goto next_rule;
			}
			for (k = 0; k < rule->charslen; k++)
				if (rule->charsdots[k] != text[k]) goto next_rule;
			debug("** rule selected **");

			/* inhibit rule */
			if (rule->dotslen > braille_len ||
					(rule->charslen == text_len && rule->dotslen < braille_len) ||
					(rule->dotslen == braille_len && rule->charslen < text_len))
				goto inhibit;
			for (k = 0; k < rule->dotslen; k++)
				if (_lou_getCharForDots(rule->charsdots[rule->charslen + k],
							displayTable) != braille[k])
					goto inhibit;

			/* don't let this rule be inhibited by an earlier rule */
			int inhibit_all = 0;
			if (rule->nocross)
				for (k = 0; k < rule->charslen - 1; k++)
					if (data[k + 1] == '>') {
						if (data[-1] == 'x')
							inhibit_all = 1;
						else
							goto next_rule;
					}

			/* fill data */
			if (rule->nocross)
				;  // deferred: see success
			else {
				k = 0;
				while (k < rule->charslen - 1) {
					if (data[k + 1] == '>') {
						data[k++] = '1';
						memset(&data[k], '-', text_len - k);
					} else
						data[k++] = 'x';
				}
			}
			if (data[rule->charslen] == '>' || data[rule->charslen] == ')') {
				data[rule->charslen - 1] = '1';
				memset(&data[rule->charslen], '-', text_len - rule->charslen);
			} else
				data[rule->charslen - 1] = 'x';
			debug("%s", data);

			/* recur */
			if (find_matching_rules(&text[rule->charslen], text_len - rule->charslen,
						&braille[rule->dotslen], braille_len - rule->dotslen,
						&data[rule->charslen], inhibit_all))
				goto success;

		inhibit:
			debug("** rule inhibited **");
			if (rule->nocross) {
				if (rule->charslen < 2) goto abort;
				/* inhibited by earlier rule */
				for (k = 0; k < rule->charslen - 1; k++)
					if (data[k + 1] == '>' && data[-1] != 'x') goto next_rule;
				data[rule->charslen - 1] = ')';
				debug("%s", data);
				goto next_rule;
			} else {
				goto abort;
			}

		success:
			/* fill data (deferred) */
			if (inhibit_all) data[-1] = '1';
			if (rule->nocross) {
				memset(data, '0', rule->charslen - 1);
				debug("%s", data);
			}
			free(data_save);
			return 1;

		next_rule:
			offset = rule->charsnext;
		}
	}

abort:
	/* restore data */
	memcpy(data, data_save, text_len);
	free(data_save);
	debug("** abort **");
	return 0;
}

/*
 * - begin with all -
 * - set cursor position right before the word
 * - put a ^
 * - match rules
 *   - when a rule has been selected
 *     - if the braille does not match: try inhibiting the rule
 *       - if it's a nocross rule (longer than a single character)
 *         - if there's a > within or right after the rule and there's no x right before
 *           the rule
 *           - already inhibited
 *         - else: put a ) at the position right after the rule
 *       - else: abort this match
 *     - else (the braille does match)
 *       - if it's a nocross rule
 *         - if there's a > within or right after the rule
 *           - if there's a x at the position right before the rule
 *             - put a 1 at that position
 *             - reset all >
 *           - else
 *             - continue with next matched rule
 *         - put a 0 at each position within the rule
 *       - else
 *         - for each position within the rule
 *           - if there's a > at the next position
 *             - put a 1
 *             - reset all >
 *           - else put a x
 *       - move cursor to the position right after the rule
 *       - put a $ if we're at the end of the word
 *       - change all ) to >
 *       - else if there's a > or a ) at the next position
 *         - put a 1
 *         - reset all >
 *         - match rules at the new cursor position
 *           - if match was aborted
 *             - revert changes
 *             - try inhibiting the last rule
 *             - go back to the position before the rule
 *             - continue with next matched rule
 *       - else put a x
 */
extern int
suggestChunks(widechar *text, widechar *braille, char *hyphen_string) {
	int text_len, braille_len;
	for (text_len = 0; text[text_len]; text_len++)
		;
	for (braille_len = 0; braille[braille_len]; braille_len++)
		;
	if (text_len == 0 || braille_len == 0) return 0;
	hyphen_string[0] = '^';
	hyphen_string[text_len + 1] = '\0';
	memset(&hyphen_string[1], '-', text_len);
	return find_matching_rules(
			text, text_len, braille, braille_len, &hyphen_string[1], 0);
}

extern void
findRelevantRules(widechar *text, widechar **rules_str) {
	int text_len, rules_len;
	TranslationTableOffset offset;
	TranslationTableCharacter *character;
	TranslationTableRule *rule;
	TranslationTableRule **rules;
	int hash_len, k, m, n;
	for (text_len = 0; text[text_len]; text_len++)
		;
	for (rules_len = 0; rules_str[rules_len]; rules_len++)
		;
	rules = (TranslationTableRule **)malloc(
			(rules_len + 1) * sizeof(TranslationTableRule *));
	m = n = 0;
	while (text[n]) {
		for (hash_len = 2; hash_len >= 1; hash_len--) {
			offset = 0;
			switch (hash_len) {
			case 2:
				if (text_len - n < 2) break;
				offset = table->forRules[_lou_stringHash(&text[n], 1, table)];
				break;
			case 1:
				offset = table->characters[_lou_charHash(text[n])];
				while (offset) {
					character = (TranslationTableCharacter *)&table->ruleArea[offset];
					if (character->value == text[0]) {
						offset = character->otherRules;
						break;
					} else
						offset = character->next;
				}
			}
			while (offset) {
				rule = (TranslationTableRule *)&table->ruleArea[offset];
				switch (rule->opcode) {
				case CTO_Always:
				case CTO_WholeWord:
				case CTO_SuffixableWord:
				case CTO_PrefixableWord:
				case CTO_BegWord:
				case CTO_BegMidWord:
				case CTO_MidWord:
				case CTO_MidEndWord:
				case CTO_EndWord:
					break;
				default:
					goto next_rule;
				}
				if (rule->charslen == 0 || rule->dotslen == 0 ||
						rule->charslen > text_len - n)
					goto next_rule;
				for (k = 0; k < rule->charslen; k++)
					if (rule->charsdots[k] != text[n + k]) goto next_rule;
				rules[m++] = rule;
				if (m == rules_len) goto finish;
			next_rule:
				offset = rule->charsnext;
			}
		}
		n++;
	}
finish:
	rules_str[m--] = NULL;
	for (; m >= 0; m--) printRule(rules[m], rules_str[m]);
	free(rules);
}
