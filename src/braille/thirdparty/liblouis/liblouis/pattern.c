/* liblouis Braille Translation and Back-Translation Library

   Copyright (C) 2016 Mike Gray, American Printing House for the Blind

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "internal.h"

//#define CHECK_OUTPUT_DEFINED

/////

// TODO: these functions are static and copied serveral times

int translation_direction = 1;

static TranslationTableCharacter *
findCharOrDots(widechar c, int m, const TranslationTableHeader *table) {
	/* Look up character or dot pattern in the appropriate
	 * table. */
	static TranslationTableCharacter noChar = { NULL, -1, 0, 0, 0, CTC_Space, 0, 0, 32, 0,
		0 };
	static TranslationTableCharacter noDots = { NULL, -1, 0, 0, 0, CTC_Space, 0, 0,
		LOU_DOTS, 0, 0 };
	TranslationTableCharacter *notFound;
	TranslationTableCharacter *character;
	TranslationTableOffset bucket;
	unsigned long int makeHash = _lou_charHash(c);
	if (m == 0) {
		bucket = table->characters[makeHash];
		notFound = &noChar;
	} else {
		bucket = table->dots[makeHash];
		notFound = &noDots;
	}
	while (bucket) {
		character = (TranslationTableCharacter *)&table->ruleArea[bucket];
		if (character->value == c) return character;
		bucket = character->next;
	}
	notFound->value = c;
	return notFound;
}

static int
checkAttr(const widechar c, const TranslationTableCharacterAttributes a,
		const TranslationTableHeader *table) {
	return (((findCharOrDots(c, translation_direction ? 0 : 1, table))->attributes & a)
					? 1
					: 0);
}

/////

enum pattern_type {
	PTN_ERROR,

	PTN_START,
	PTN_GROUP,
	PTN_NOT,

	PTN_ONE_MORE,
	PTN_ZERO_MORE,
	PTN_OPTIONAL,

	PTN_ALTERNATE,

	PTN_ANY,
	PTN_ATTRIBUTES,
	PTN_CHARS,
	PTN_HOOK,
	PTN_END_OF_INPUT,

	PTN_END = 0xffff,
};

#define EXPR_TYPE_IN(at, buffer) (buffer[(at) + 0])
#define EXPR_PRV_IN(at, buffer) (buffer[(at) + 1])
#define EXPR_NXT_IN(at, buffer) (buffer[(at) + 2])
#define EXPR_DATA_0_IN(at, buffer) (buffer[(at) + 3])
#define EXPR_DATA_1_IN(at, buffer) (buffer[(at) + 4])
#define EXPR_DATA_2_IN(at, buffer) (buffer[(at) + 5])
#define EXPR_DATA_IN(at, buffer) ((widechar *)&buffer[(at) + 3])
#define EXPR_CONST_DATA_IN(at, buffer) ((const widechar *)&buffer[(at) + 3])

#define EXPR_TYPE(at) EXPR_TYPE_IN((at), expr_data)
#define EXPR_PRV(at) EXPR_PRV_IN((at), expr_data)
#define EXPR_NXT(at) EXPR_NXT_IN((at), expr_data)
#define EXPR_DATA_0(at) EXPR_DATA_0_IN((at), expr_data)
#define EXPR_DATA_1(at) EXPR_DATA_1_IN((at), expr_data)
#define EXPR_DATA_2(at) EXPR_DATA_2_IN((at), expr_data)
#define EXPR_DATA(at) EXPR_DATA_IN((at), expr_data)
#define EXPR_CONST_DATA(at) EXPR_CONST_DATA_IN((at), expr_data)

#ifdef CHECK_OUTPUT_DEFINED

#ifndef DEBUG
#define DEBUG

#endif

#define START 0
#define CALL 1
#define RETURN 2
#define SHOW 3

#define CHECK_OUTPUT(type, ret, line, msg)                                              \
	{                                                                                   \
		do_output(type, ret, line, input[*input_crs], input_minmax, *input_crs,         \
				input_dir, expr_data, expr_crs, notOperator, loop_crs, loop_cnts, msg); \
	}

#else

#define CHECK_OUTPUT(type, ret, line, msg) \
	{ ; }

#endif

struct expression {
	widechar type;
	widechar prv;
	widechar nxt;
	widechar data[1];
};

/* gdb won't know what this is unless it is actually used */
#ifdef DEBUG
static struct expression *expr_debug;
#endif

////////////////////////////////////////////////////////////////////////////////

static char spaces[] = "..............................";
static int space = 30;

static void
pattern_output_expression(
		const widechar *expr_data, int expr_crs, const TranslationTableHeader *table) {
	int i;

	if (expr_crs == PTN_END) return;

	while (EXPR_TYPE(expr_crs) != PTN_END) {
		printf("%s%d", &spaces[space], expr_crs);
		if (expr_crs < 100) printf(" ");
		if (expr_crs < 10) printf(" ");
		for (i = 0; i < 13 - (30 - space); i++) printf(" ");

		switch (EXPR_TYPE(expr_crs)) {
		case PTN_START:

			printf("START\t%d\t%d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
			break;

		case PTN_GROUP:

			printf("(    \t%d\t%d\t-> %d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs),
					EXPR_DATA_0(expr_crs));
			space--;
			if (space < 0) space = 0;
			pattern_output_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			space++;
			if (space > 30) space = 30;
			break;

		case PTN_NOT:

			printf("!    \t%d\t%d\t-> %d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs),
					EXPR_DATA_0(expr_crs));
			space--;
			if (space < 0) space = 0;
			pattern_output_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			space++;
			if (space > 30) space = 30;
			break;

		case PTN_ONE_MORE:

			printf("+    \t%d\t%d\t-> %d\t#%d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs),
					EXPR_DATA_0(expr_crs), EXPR_DATA_1(expr_crs));
			space--;
			if (space < 0) space = 0;
			pattern_output_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			space++;
			if (space > 30) space = 30;
			break;

		case PTN_ZERO_MORE:

			printf("*    \t%d\t%d\t-> %d\t#%d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs),
					EXPR_DATA_0(expr_crs), EXPR_DATA_1(expr_crs));
			space--;
			if (space < 0) space = 0;
			pattern_output_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			space++;
			if (space > 30) space = 30;
			break;

		case PTN_OPTIONAL:

			printf("?    \t%d\t%d\t-> %d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs),
					EXPR_DATA_0(expr_crs));
			space--;
			if (space < 0) space = 0;
			pattern_output_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			space++;
			if (space > 30) space = 30;
			break;

		case PTN_ALTERNATE:

			printf("|    \t%d\t%d\t-> %d\t-> %d\n", EXPR_PRV(expr_crs),
					EXPR_NXT(expr_crs), EXPR_DATA_0(expr_crs), EXPR_DATA_1(expr_crs));
			space--;
			if (space < 0) space = 0;
			pattern_output_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			pattern_output_expression(expr_data, EXPR_DATA_1(expr_crs), table);
			space++;
			if (space > 30) space = 30;
			break;

		case PTN_ANY:

			printf(".    \t%d\t%d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
			break;

		case PTN_ATTRIBUTES:

			printf("%%    \t%d\t%d\t", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[0] >> 16)) printf("0");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[1] >> 16)) printf("1");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[2] >> 16)) printf("2");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[3] >> 16)) printf("3");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[4] >> 16)) printf("4");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[5] >> 16)) printf("5");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[6] >> 16)) printf("6");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[7] >> 16)) printf("7");
			if (EXPR_DATA_0(expr_crs) & (CTC_EndOfInput >> 16)) printf("^");
			if (EXPR_DATA_1(expr_crs) & CTC_Space) printf("_");
			if (EXPR_DATA_1(expr_crs) & CTC_Digit) printf("#");
			if (EXPR_DATA_1(expr_crs) & CTC_Letter) printf("a");
			if (EXPR_DATA_1(expr_crs) & CTC_UpperCase) printf("u");
			if (EXPR_DATA_1(expr_crs) & CTC_LowerCase) printf("l");
			if (EXPR_DATA_1(expr_crs) & CTC_Punctuation) printf(".");
			if (EXPR_DATA_1(expr_crs) & CTC_Sign) printf("$");
			if (EXPR_DATA_1(expr_crs) & CTC_SeqDelimiter) printf("~");
			if (EXPR_DATA_1(expr_crs) & CTC_SeqBefore) printf("<");
			if (EXPR_DATA_1(expr_crs) & CTC_SeqAfter) printf(">");
			puts("");
			break;

		case PTN_CHARS:

			printf("[]   \t%d\t%d\t", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
			for (i = 0; i < EXPR_DATA_0(expr_crs); i++)
				printf("%c", EXPR_CONST_DATA(expr_crs)[i + 1]);
			puts("");
			break;

		case PTN_HOOK:

			printf("@    \t%d\t%d\t", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
			for (i = 0; i < EXPR_DATA_0(expr_crs); i++)
				printf("%c", EXPR_CONST_DATA(expr_crs)[i + 1]);
			puts("");
			break;

		case PTN_END_OF_INPUT:

			printf("^    \t%d\t%d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
			break;

		default:

			printf("%d?    \t%d\t%d\n", EXPR_TYPE(expr_crs), EXPR_PRV(expr_crs),
					EXPR_NXT(expr_crs));
			break;
		}

		expr_crs = EXPR_NXT(expr_crs);
	}

	printf("%s%d", &spaces[space], expr_crs);
	if (expr_crs < 100) printf(" ");
	if (expr_crs < 10) printf(" ");
	for (i = 0; i < 13 - (30 - space); i++) printf(" ");
	printf("END\t%d\t%d\n", EXPR_PRV(expr_crs), EXPR_NXT(expr_crs));
	fflush(stdout);
	return;
}

static void
pattern_output(const widechar *expr_data, const TranslationTableHeader *table) {
	printf("%d    \tlength\n", expr_data[0]);
	printf("%d    \tloops\n", expr_data[1]);
	if (expr_data[0] > 0 && expr_data[0] != PTN_END)
		pattern_output_expression(expr_data, 2, table);
}

static void
pattern_print_expression(
		const widechar *expr_data, int expr_crs, const TranslationTableHeader *table) {
	int i;

	if (expr_crs == PTN_END) return;

	while (EXPR_TYPE(expr_crs) != PTN_END) {
		switch (EXPR_TYPE(expr_crs)) {
		case PTN_START:
			break;

		case PTN_GROUP:

			printf(" (");
			pattern_print_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			printf(") ");
			break;

		case PTN_NOT:

			printf("!");
			pattern_print_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			break;

		case PTN_ONE_MORE:

			pattern_print_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			printf("+");
			break;

		case PTN_ZERO_MORE:

			pattern_print_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			printf("*");
			break;

		case PTN_OPTIONAL:

			pattern_print_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			printf("?");
			break;

		case PTN_ALTERNATE:

			pattern_print_expression(expr_data, EXPR_DATA_0(expr_crs), table);
			printf(" | ");
			pattern_print_expression(expr_data, EXPR_DATA_1(expr_crs), table);
			break;

		case PTN_ANY:

			printf(".");
			break;

		case PTN_ATTRIBUTES:

			printf("%%[");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[0] >> 16)) printf("0");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[1] >> 16)) printf("1");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[2] >> 16)) printf("2");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[3] >> 16)) printf("3");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[4] >> 16)) printf("4");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[5] >> 16)) printf("5");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[6] >> 16)) printf("6");
			if (EXPR_DATA_0(expr_crs) & (table->numberedAttributes[7] >> 16)) printf("7");
			if (EXPR_DATA_0(expr_crs) & (CTC_EndOfInput >> 16)) printf("^");
			if (EXPR_DATA_1(expr_crs) & CTC_Space) printf("_");
			if (EXPR_DATA_1(expr_crs) & CTC_Digit) printf("#");
			if (EXPR_DATA_1(expr_crs) & CTC_Letter) printf("a");
			if (EXPR_DATA_1(expr_crs) & CTC_UpperCase) printf("u");
			if (EXPR_DATA_1(expr_crs) & CTC_LowerCase) printf("l");
			if (EXPR_DATA_1(expr_crs) & CTC_Punctuation) printf(".");
			if (EXPR_DATA_1(expr_crs) & CTC_Sign) printf("$");
			if (EXPR_DATA_1(expr_crs) & CTC_SeqDelimiter) printf("~");
			if (EXPR_DATA_1(expr_crs) & CTC_SeqBefore) printf("<");
			if (EXPR_DATA_1(expr_crs) & CTC_SeqAfter) printf(">");
			printf("]");
			break;

		case PTN_CHARS:

			if (EXPR_DATA_0(expr_crs) == 1)
				printf("%c", EXPR_DATA_1(expr_crs));
			else {
				printf("[");
				for (i = 0; i < EXPR_DATA_0(expr_crs); i++)
					printf("%c", EXPR_CONST_DATA(expr_crs)[i + 1]);
				printf("]");
			}
			break;

		case PTN_HOOK:

			printf("@[");
			for (i = 0; i < EXPR_DATA_0(expr_crs); i++)
				printf("%c", EXPR_CONST_DATA(expr_crs)[i + 1]);
			printf("]");
			break;

		case PTN_END_OF_INPUT:

			printf("^");
			break;

			// default:  printf("%d?\n", EXPR_TYPE(expr_crs));  break;
		}

		expr_crs = EXPR_NXT(expr_crs);
	}

	return;
}

static void
pattern_print(const widechar *expr_data, const TranslationTableHeader *table) {
	if (expr_data[0] > 0 && expr_data[0] != PTN_END)
		pattern_print_expression(expr_data, 2, table);
	puts("");
}

#ifdef CHECK_OUTPUT_DEFINED

static void
do_padd(const int value) {
	if (value < 100000) printf(" ");
	if (value < 10000) printf(" ");
	if (value < 1000) printf(" ");
	if (value < 100) printf(" ");
	if (value < 10) printf(" ");
}

static void
do_pad(const int value) {
	if (value < 100) printf(" ");
	if (value < 10) printf(" ");
}

static void
do_output(const int type, const int ret, const int line,

		const int input, const int input_minmax, const int input_crs, const int input_dir,
		const widechar *expr_data, const int expr_crs, const int notOperator,
		const int loop_crs, const int *loop_cnts,

		const char *msg) {
	switch (type) {
	case START:

		space--;
		if (space < 0) space = 0;
		printf("|%s()  ", &spaces[space]);
		break;

	case CALL:

		printf("|%s>   ", &spaces[space]);
		break;

	case RETURN:

		printf("|%s<%d  ", &spaces[space], ret);
		space++;
		if (space > 31) space = 31;
		break;

	case SHOW:

		printf("|%s    ", &spaces[space]);
		break;
	}

	printf("%d ", line);
	do_padd(line);

	switch (expr_data[expr_crs]) {
	case PTN_ERROR:
		printf("# ");
		break;
	case PTN_START:
		printf("> ");
		break;
	case PTN_END_OF_INPUT:
		printf("^ ");
		break;
	case PTN_ALTERNATE:
		printf("| ");
		break;
	case PTN_OPTIONAL:
		printf("? ");
		break;
	case PTN_ONE_MORE:
		printf("+ ");
		break;
	case PTN_ZERO_MORE:
		printf("* ");
		break;
	case PTN_NOT:
		printf("! ");
		break;
	case PTN_GROUP:
		printf("( ");
		break;
	case PTN_ANY:
		printf(". ");
		break;
	case PTN_ATTRIBUTES:
		printf("%% ");
		break;
	case PTN_CHARS:
		printf("[ ");
		break;
	case PTN_HOOK:
		printf("@ ");
		break;
	case PTN_END:
		printf("< ");
		break;
	default:
		printf("  ");
		break;
	}
	printf("%d ", expr_crs);
	do_padd(expr_crs);

	if (input > 31 && input < 127)
		printf("%c ", input);
	else
		printf("_ ");

	if (input_crs * input_dir >= input_minmax * input_dir)
		printf("#   ");
	else {
		printf("%d ", input_crs);
		do_pad(input_crs);
	}

	if (input_dir > 0)
		printf("<");
	else
		printf(">");
	printf("%d ", input_minmax);
	do_pad(input_minmax);

	if (notOperator)
		printf("!   ");
	else
		printf("    ");

	if (loop_crs) {
		printf("%d ", loop_crs);
		do_pad(loop_crs);
		printf("%d ", loop_cnts[EXPR_DATA_1(loop_crs)]);
		do_pad(loop_cnts[EXPR_DATA_1(loop_crs)]);
	} else
		printf("-   -   ");
	if (EXPR_TYPE(expr_crs) == PTN_ONE_MORE || EXPR_TYPE(expr_crs) == PTN_ZERO_MORE) {
		printf("%d ", loop_cnts[EXPR_DATA_1(expr_crs)]);
		do_pad(loop_cnts[EXPR_DATA_1(expr_crs)]);
	} else
		printf("-   ");

	if (msg) printf("%s", msg);
	puts("");
}

#endif

////////////////////////////////////////////////////////////////////////////////

static int
pattern_compile_1(const widechar *input, const int input_max, int *input_crs,
		widechar *expr_data, const int expr_max, widechar *expr_crs, widechar *loop_cnts,
		TranslationTableHeader *table, const FileInfo *nested);

static int
pattern_compile_expression(const widechar *input, const int input_max, int *input_crs,
		widechar *expr_data, const int expr_max, widechar *expr_crs, widechar *loop_cnts,
		TranslationTableHeader *table, const FileInfo *nested) {
	widechar *data;
	int expr_start, expr_end, expr_sub, expr_crs_prv;
	int input_end;
	int attrs0, attrs1;
	int set, esc, nest, i;

	switch (input[*input_crs]) {
	case '(':

		if (*expr_crs + 10 >= expr_max) return 0;

		(*input_crs)++;
		if (*input_crs >= input_max) return 0;

		/* find closing parenthesis */
		nest = esc = 0;
		for (input_end = *input_crs; input_end < input_max; input_end++) {
			if (input[input_end] == '\\' && !esc) {
				esc = 1;
				continue;
			}

			if (input[input_end] == '(' && !esc)
				nest++;
			else if (input[input_end] == ')' && !esc) {
				if (nest)
					nest--;
				else
					break;
			}

			esc = 0;
		}
		if (input_end >= input_max) return 0;

		EXPR_TYPE(*expr_crs) = PTN_GROUP;

		/* compile sub expressions */
		expr_crs_prv = *expr_crs;
		*expr_crs += 4;
		EXPR_DATA_0(expr_crs_prv) = *expr_crs;
		expr_sub = *expr_crs;
		EXPR_TYPE(expr_sub) = PTN_ERROR;
		EXPR_PRV(expr_sub) = PTN_END;
		EXPR_NXT(expr_sub) = PTN_END;
		if (!pattern_compile_1(input, input_end, input_crs, expr_data, expr_max, expr_crs,
					loop_cnts, table, nested))
			return 0;
		(*input_crs)++;

		/* reset end expression */
		expr_end = *expr_crs;
		EXPR_NXT(expr_end) = expr_crs_prv;

		return *expr_crs += 3;

	case '!':

		if (*expr_crs + 10 >= expr_max) return 0;

		(*input_crs)++;
		EXPR_TYPE(*expr_crs) = PTN_NOT;
		expr_crs_prv = *expr_crs;
		*expr_crs += 4;
		EXPR_DATA_0(expr_crs_prv) = *expr_crs;

		/* create start expression */
		expr_start = *expr_crs;
		EXPR_TYPE(expr_start) = PTN_START;
		EXPR_PRV(expr_start) = PTN_END;
		*expr_crs += 3;
		EXPR_NXT(expr_start) = *expr_crs;

		/* compile sub expression */
		expr_sub = *expr_crs;
		EXPR_TYPE(expr_sub) = PTN_ERROR;
		EXPR_PRV(expr_sub) = expr_start;
		EXPR_NXT(expr_sub) = PTN_END;

		if (!pattern_compile_expression(input, input_max, input_crs, expr_data, expr_max,
					expr_crs, loop_cnts, table, nested))
			return 0;

		if (*expr_crs + 3 >= expr_max) return 0;

		EXPR_NXT(expr_sub) = *expr_crs;

		/* create end expression */
		expr_end = *expr_crs;
		EXPR_TYPE(expr_end) = PTN_END;
		EXPR_PRV(expr_end) = expr_sub;
		EXPR_NXT(expr_end) = expr_crs_prv;

		return *expr_crs += 3;

	case '+':

		if (*expr_crs + 5 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_ONE_MORE;
		EXPR_DATA_1(*expr_crs) = (*loop_cnts)++;
		(*input_crs)++;
		return *expr_crs += 5;

	case '*':

		if (*expr_crs + 5 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_ZERO_MORE;
		EXPR_DATA_1(*expr_crs) = (*loop_cnts)++;
		(*input_crs)++;
		return *expr_crs += 5;

	case '?':

		if (*expr_crs + 4 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_OPTIONAL;
		(*input_crs)++;
		return *expr_crs += 4;

	case '|':

		if (*expr_crs + 5 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_ALTERNATE;
		(*input_crs)++;
		return *expr_crs += 5;

	case '.':

		if (*expr_crs + 3 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_ANY;
		(*input_crs)++;
		return *expr_crs += 3;

	case '%':

		if (*expr_crs + 5 >= expr_max) return 0;

		(*input_crs)++;
		if (*input_crs >= input_max) return 0;

		/* find closing bracket */
		if (input[*input_crs] == '[') {
			set = 1;
			(*input_crs)++;
			for (input_end = *input_crs; input_end < input_max; input_end++)
				if (input[input_end] == ']') break;
			if (input_end >= input_max) return 0;
		} else {
			set = 0;
			input_end = *input_crs + 1;
		}

		EXPR_TYPE(*expr_crs) = PTN_ATTRIBUTES;

		attrs0 = attrs1 = 0;
		for (; (*input_crs) < input_end; (*input_crs)++) {
			switch (input[*input_crs]) {
			case '_':
				attrs0 |= CTC_Space;
				break;
			case '#':
				attrs0 |= CTC_Digit;
				break;
			case 'a':
				attrs0 |= CTC_Letter;
				break;
			case 'u':
				attrs0 |= CTC_UpperCase;
				break;
			case 'l':
				attrs0 |= CTC_LowerCase;
				break;
			case '.':
				attrs0 |= CTC_Punctuation;
				break;
			case '$':
				attrs0 |= CTC_Sign;
				break;
			case '~':
				attrs0 |= CTC_SeqDelimiter;
				break;
			case '<':
				attrs0 |= CTC_SeqBefore;
				break;
			case '>':
				attrs0 |= CTC_SeqAfter;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7': {
				int k = input[*input_crs] - '0';
				TranslationTableCharacterAttributes a = table->numberedAttributes[k];
				if (!a) {
					// attribute not used before yet: assign it a value
					a = table->numberedAttributes[k] =
							table->nextNumberedCharacterClassAttribute;
					if (a > CTC_UserDefined8) {
						_lou_logMessage(LOU_LOG_ERROR,
								"%s:%d: error: Too many character attributes defined",
								nested->fileName, nested->lineNumber);
						return 0;
					}
					table->nextNumberedCharacterClassAttribute <<= 1;
				}
				attrs1 |= (a >> 16);
				break;
			}
			case '^':
				attrs1 |= (CTC_EndOfInput >> 16);
				break;

			default:
				return 0;
			}
		}
		EXPR_DATA_0(*expr_crs) = attrs1;
		EXPR_DATA_1(*expr_crs) = attrs0;

		if (set) (*input_crs)++;
		return *expr_crs += 5;

	case '[':

		(*input_crs)++;
		if (*input_crs >= input_max) return 0;

		/* find closing bracket */
		esc = 0;
		for (input_end = *input_crs; input_end < input_max; input_end++) {
			if (input[input_end] == '\\' && !esc) {
				esc = 1;
				continue;
			}

			if (input[input_end] == ']' && !esc) break;
			esc = 0;
		}
		if (input_end >= input_max) return 0;

		if (*expr_crs + 4 + (input_end - *input_crs) >= expr_max) return 0;

		EXPR_TYPE(*expr_crs) = PTN_CHARS;

		esc = 0;
		data = EXPR_DATA(*expr_crs);
		for (i = 1; *input_crs < input_end; (*input_crs)++) {
			if (input[*input_crs] == '\\' && !esc) {
				esc = 1;
				continue;
			}

			esc = 0;
			data[i++] = (widechar)input[*input_crs];
		}
		data[0] = i - 1;
		(*input_crs)++;
		return *expr_crs += 4 + data[0];

	case '@':

		(*input_crs)++;
		if (*input_crs >= input_max) return 0;

		/* find closing bracket */
		if (input[*input_crs] == '[') {
			set = 1;
			(*input_crs)++;
			for (input_end = *input_crs; input_end < input_max; input_end++)
				if (input[input_end] == ']') break;
			if (input_end >= input_max) return 0;
		} else {
			set = 0;
			input_end = *input_crs + 1;
		}

		if (*expr_crs + 4 + (input_end - *input_crs) >= expr_max) return 0;

		EXPR_TYPE(*expr_crs) = PTN_HOOK;

		esc = 0;
		data = EXPR_DATA(*expr_crs);
		for (i = 1; *input_crs < input_end; (*input_crs)++) {
			if (input[*input_crs] == '\\' && !esc) {
				esc = 1;
				continue;
			}

			esc = 0;
			data[i++] = (widechar)input[*input_crs];
		}
		data[0] = i - 1;
		if (set) (*input_crs)++;
		return *expr_crs += 4 + data[0];

	case '^':
	case '$':

		if (*expr_crs + 3 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_END_OF_INPUT;
		(*input_crs)++;
		return *expr_crs += 3;

	case '\\':

		(*input_crs)++;
		if (*input_crs >= input_max) return 0;

	default:

		if (*expr_crs + 5 >= expr_max) return 0;
		EXPR_TYPE(*expr_crs) = PTN_CHARS;
		EXPR_DATA_0(*expr_crs) = 1;
		EXPR_DATA_1(*expr_crs) = (widechar)input[*input_crs];
		(*input_crs)++;
		return *expr_crs += 5;
	}
}

static int
pattern_insert_alternate(const widechar *input, const int input_max, int *input_crs,
		widechar *expr_data, const int expr_max, widechar *expr_crs, widechar *loop_cnts,
		int expr_insert, TranslationTableHeader *table, const FileInfo *nested) {
	int expr_group, expr_alt, expr_end;

	if (EXPR_TYPE(*expr_crs) == PTN_START) return 0;

	if (*expr_crs + 12 >= expr_max) return 0;

	/* setup alternate expression */
	expr_alt = *expr_crs;
	EXPR_TYPE(expr_alt) = PTN_ALTERNATE;
	EXPR_PRV(expr_alt) = PTN_END;
	EXPR_NXT(expr_alt) = PTN_END;
	*expr_crs += 5;

	/* setup group expression */
	expr_group = *expr_crs;
	EXPR_TYPE(expr_group) = PTN_GROUP;
	EXPR_PRV(expr_group) = PTN_END;
	EXPR_NXT(expr_group) = PTN_END;
	*expr_crs += 4;
	EXPR_DATA_0(expr_group) = *expr_crs;

	EXPR_TYPE(*expr_crs) = PTN_ERROR;
	EXPR_PRV(*expr_crs) = PTN_END;
	EXPR_NXT(*expr_crs) = PTN_END;
	if (!pattern_compile_1(input, input_max, input_crs, expr_data, expr_max, expr_crs,
				loop_cnts, table, nested))
		return 0;
	expr_end = *expr_crs;
	EXPR_NXT(expr_end) = expr_group;

	/* setup last end expression */
	if (*expr_crs + 3 >= expr_max) return 0;
	*expr_crs += 3;
	EXPR_TYPE(*expr_crs) = PTN_END;
	EXPR_NXT(*expr_crs) = PTN_END;

	/* replace insert expression with group expression using last end expression */
	EXPR_NXT(EXPR_PRV(expr_insert)) = expr_group;
	EXPR_PRV(expr_group) = EXPR_PRV(expr_insert);

	EXPR_NXT(expr_group) = *expr_crs;
	EXPR_PRV(*expr_crs) = expr_group;

	/* link alternate and insert expressions before group end expression */
	EXPR_NXT(EXPR_PRV(expr_end)) = expr_alt;
	EXPR_PRV(expr_alt) = EXPR_PRV(expr_end);

	EXPR_NXT(expr_alt) = expr_insert;
	EXPR_PRV(expr_insert) = expr_alt;

	EXPR_NXT(expr_insert) = expr_end;
	EXPR_PRV(expr_end) = expr_insert;

	return *expr_crs;
}

/* Compile all expression sequences, resolving character sets, attributes,
 * groups, nots, and hooks.  Note that unlike the other compile functions, on
 * returning the expr_crs is set to the last end expression, not after it.
 */
static int
pattern_compile_1(const widechar *input, const int input_max, int *input_crs,
		widechar *expr_data, const int expr_max, widechar *expr_crs, widechar *loop_cnts,
		TranslationTableHeader *table, const FileInfo *nested) {
	int expr_crs_prv;

	if (*expr_crs + 6 >= expr_max) return 0;

	expr_crs_prv = *expr_crs;

	/* setup start expression */
	EXPR_TYPE(*expr_crs) = PTN_START;
	EXPR_PRV(*expr_crs) = PTN_END;
	*expr_crs += 3;
	EXPR_NXT(expr_crs_prv) = *expr_crs;

	/* setup end expression */
	EXPR_TYPE(*expr_crs) = PTN_END;
	EXPR_PRV(*expr_crs) = expr_crs_prv;
	EXPR_NXT(*expr_crs) = PTN_END;

	while (*input_crs < input_max) {
		expr_crs_prv = *expr_crs;
		if (!pattern_compile_expression(input, input_max, input_crs, expr_data, expr_max,
					expr_crs, loop_cnts, table, nested))
			return 0;

		/* setup end expression */
		if (*expr_crs + 3 >= expr_max) return 0;
		EXPR_NXT(expr_crs_prv) = *expr_crs;
		EXPR_TYPE(*expr_crs) = PTN_END;
		EXPR_PRV(*expr_crs) = expr_crs_prv;
		EXPR_NXT(*expr_crs) = PTN_END;

		/* insert seqafterexpression before attributes of seqafterchars */
		// if(EXPR_TYPE(expr_crs_prv) == PTN_ATTRIBUTES)
		// if(EXPR_DATA_1(expr_crs_prv) & CTC_SeqAfter)
		// {
		// 	i = 0;
		// 	pattern_insert_alternate(table->seqAfterExpression,
		// 		table->seqAfterExpressionLength, &i, expr_data, expr_max,
		// 		expr_crs, loop_cnts, expr_crs_prv);
		// }
	}

	return *expr_crs;
}

/* Resolve optional and loop expressions.
 */
static int
pattern_compile_2(
		widechar *expr_data, int expr_at, const int expr_max, widechar *expr_crs) {
	int expr_start, expr_end, expr_prv, expr_sub;

	while (EXPR_TYPE(expr_at) != PTN_END) {
		if (EXPR_TYPE(expr_at) == PTN_GROUP || EXPR_TYPE(expr_at) == PTN_NOT) {
			if (!pattern_compile_2(expr_data, EXPR_DATA_0(expr_at), expr_max, expr_crs))
				return 0;
		}

		if (EXPR_TYPE(expr_at) == PTN_ZERO_MORE || EXPR_TYPE(expr_at) == PTN_ONE_MORE ||
				EXPR_TYPE(expr_at) == PTN_OPTIONAL) {
			if (*expr_crs + 6 >= expr_max) return 0;

			/* get previous expressions, there must
			 * be at least something and a PTN_START */
			expr_sub = EXPR_PRV(expr_at);
			if (EXPR_TYPE(expr_sub) == PTN_START) return 0;
			expr_prv = EXPR_PRV(expr_sub);

			/* create start expression */
			expr_start = *expr_crs;
			EXPR_TYPE(expr_start) = PTN_START;
			EXPR_PRV(expr_start) = PTN_END;
			EXPR_NXT(expr_start) = expr_sub;
			*expr_crs += 3;

			/* create end expression */
			expr_end = *expr_crs;
			EXPR_TYPE(expr_end) = PTN_END;
			EXPR_PRV(expr_end) = expr_sub;
			EXPR_NXT(expr_end) = expr_at;
			*expr_crs += 3;

			/* relink previous expression before sub expression */
			EXPR_DATA_0(expr_at) = expr_start;
			EXPR_NXT(expr_prv) = expr_at;
			EXPR_PRV(expr_at) = expr_prv;

			/* relink sub expression to start and end */
			EXPR_PRV(expr_sub) = expr_start;
			EXPR_NXT(expr_sub) = expr_end;
		}

		expr_at = EXPR_NXT(expr_at);
	}

	return 1;
}

/* Resolves alternative expressions.
 */
static int
pattern_compile_3(
		widechar *expr_data, int expr_at, const int expr_max, widechar *expr_crs) {
	int expr_mrk, expr_start, expr_end, expr_sub_start, expr_sub_end;

	while (EXPR_TYPE(expr_at) != PTN_END) {
		if (EXPR_TYPE(expr_at) == PTN_GROUP || EXPR_TYPE(expr_at) == PTN_NOT ||
				EXPR_TYPE(expr_at) == PTN_OPTIONAL ||
				EXPR_TYPE(expr_at) == PTN_ZERO_MORE ||
				EXPR_TYPE(expr_at) == PTN_ONE_MORE) {
			if (!pattern_compile_3(expr_data, EXPR_DATA_0(expr_at), expr_max, expr_crs))
				return 0;
		}

		if (EXPR_TYPE(expr_at) == PTN_ALTERNATE) {
			if (*expr_crs + 12 >= expr_max) return 0;

			/* get previous start expression,
			 * can include alternate expressions */
			expr_mrk = EXPR_PRV(expr_at);
			if (EXPR_TYPE(expr_mrk) == PTN_START) return 0;
			expr_sub_end = expr_mrk;
			while (EXPR_TYPE(expr_mrk) != PTN_START) expr_mrk = EXPR_PRV(expr_mrk);
			expr_sub_start = EXPR_NXT(expr_mrk);

			/* create first start expression */
			expr_start = *expr_crs;
			EXPR_TYPE(expr_start) = PTN_START;
			EXPR_PRV(expr_start) = PTN_END;
			EXPR_NXT(expr_start) = expr_sub_start;
			*expr_crs += 3;

			/* create first end expression */
			expr_end = *expr_crs;
			EXPR_TYPE(expr_end) = PTN_END;
			EXPR_PRV(expr_end) = expr_sub_end;
			EXPR_NXT(expr_end) = expr_at;
			*expr_crs += 3;

			/* relink previous expression before sub expression */
			EXPR_DATA_0(expr_at) = expr_start;
			EXPR_NXT(expr_mrk) = expr_at;
			EXPR_PRV(expr_at) = expr_mrk;

			/* relink sub expression to start and end */
			EXPR_PRV(expr_sub_start) = expr_start;
			EXPR_NXT(expr_sub_end) = expr_end;

			/* get following PTN_END or PTN_ALTERNATE expression */
			expr_mrk = EXPR_NXT(expr_at);
			if (EXPR_TYPE(expr_mrk) == PTN_END || EXPR_TYPE(expr_mrk) == PTN_ALTERNATE)
				return 0;
			expr_sub_start = expr_mrk;
			while (EXPR_TYPE(expr_mrk) != PTN_END && EXPR_TYPE(expr_mrk) != PTN_ALTERNATE)
				expr_mrk = EXPR_NXT(expr_mrk);
			expr_sub_end = EXPR_PRV(expr_mrk);

			/* create first start expression */
			expr_start = *expr_crs;
			EXPR_TYPE(expr_start) = PTN_START;
			EXPR_PRV(expr_start) = PTN_END;
			EXPR_NXT(expr_start) = expr_sub_start;
			*expr_crs += 3;

			/* create first end expression */
			expr_end = *expr_crs;
			EXPR_TYPE(expr_end) = PTN_END;
			EXPR_PRV(expr_end) = expr_sub_end;
			EXPR_NXT(expr_end) = expr_at;
			*expr_crs += 3;

			/* relink following expression before sub expression */
			EXPR_DATA_1(expr_at) = expr_start;
			EXPR_PRV(expr_mrk) = expr_at;
			EXPR_NXT(expr_at) = expr_mrk;

			/* relink sub expression to start and end */
			EXPR_PRV(expr_sub_start) = expr_start;
			EXPR_NXT(expr_sub_end) = expr_end;

			/* check expressions were after alternate and got moved into
			 * a sub expression, previous expressions already checked */
			if (!pattern_compile_3(expr_data, EXPR_DATA_1(expr_at), expr_max, expr_crs))
				return 0;
		}

		expr_at = EXPR_NXT(expr_at);
	}

	return 1;
}

int EXPORT_CALL
_lou_pattern_compile(const widechar *input, const int input_max, widechar *expr_data,
		const int expr_max, TranslationTableHeader *table, const FileInfo *nested) {
	int input_crs;

	input_crs = 0;
	expr_data[0] = 2;
	expr_data[1] = 0;

	if (!pattern_compile_1(input, input_max, &input_crs, expr_data, expr_max,
				&expr_data[0], &expr_data[1], table, nested))
		return 0;

	/* shift past the last end */
	expr_data[0] += 3;

	if (!pattern_compile_2(expr_data, 2, expr_max, &expr_data[0])) return 0;

	if (!pattern_compile_3(expr_data, 2, expr_max, &expr_data[0])) return 0;

	return expr_data[0];
}

////////////////////////////////////////////////////////////////////////////////

static void
pattern_reverse_expression(widechar *expr_data, const int expr_start);

static void
pattern_reverse_branch(widechar *expr_data, const int expr_at) {
	widechar expr_swap;

	switch (EXPR_TYPE(expr_at)) {
	case PTN_ALTERNATE:

		pattern_reverse_expression(expr_data, EXPR_DATA_0(expr_at));
		expr_swap = EXPR_DATA_0(expr_at);
		EXPR_DATA_0(expr_at) = EXPR_DATA_1(expr_at);
		EXPR_DATA_1(expr_at) = expr_swap;

	case PTN_GROUP:
	case PTN_NOT:
	case PTN_ONE_MORE:
	case PTN_ZERO_MORE:
	case PTN_OPTIONAL:

		pattern_reverse_expression(expr_data, EXPR_DATA_0(expr_at));
	}
}

static void
pattern_reverse_expression(widechar *expr_data, const int expr_start) {
	widechar expr_end, expr_crs, expr_prv;

	expr_end = EXPR_NXT(expr_start);

	/* empty expression */
	if (EXPR_TYPE(expr_end) == PTN_END) return;

	/* find end expression */
	while (EXPR_TYPE(expr_end) != PTN_END) expr_end = EXPR_NXT(expr_end);

	expr_crs = EXPR_PRV(expr_end);
	expr_prv = EXPR_PRV(expr_crs);

	/* relink expression before end expression */
	EXPR_NXT(expr_start) = expr_crs;
	EXPR_PRV(expr_crs) = expr_start;
	EXPR_NXT(expr_crs) = expr_prv;

	/* reverse any branching expressions */
	pattern_reverse_branch(expr_data, expr_crs);

	while (expr_prv != expr_start) {
		/* shift current expression */
		expr_crs = expr_prv;
		expr_prv = EXPR_PRV(expr_prv);

		/* reverse any branching expressions */
		pattern_reverse_branch(expr_data, expr_crs);

		/* relink current expression */
		EXPR_PRV(expr_crs) = EXPR_NXT(expr_crs);
		EXPR_NXT(expr_crs) = expr_prv;
	}

	/* relink expression after start expression */
	EXPR_PRV(expr_crs) = EXPR_NXT(expr_crs);
	EXPR_NXT(expr_crs) = expr_end;
	EXPR_PRV(expr_end) = expr_crs;
}

void EXPORT_CALL
_lou_pattern_reverse(widechar *expr_data) {
	pattern_reverse_expression(expr_data, 2);
}

////////////////////////////////////////////////////////////////////////////////

static int
pattern_check_chars(const widechar input_char, const widechar *expr_data) {
	int expr_cnt, i;

	expr_cnt = expr_data[0] + 1;

	for (i = 1; i < expr_cnt; i++)
		if (input_char == expr_data[i]) break;

	if (i == expr_cnt) return 0;
	return 1;
}

static int
pattern_check_attrs(const widechar input_char, const widechar *expr_data,
		const TranslationTableHeader *table) {
	int attrs;

	attrs = ((expr_data[0] << 16) | expr_data[1]) & ~(CTC_EndOfInput | CTC_EmpMatch);
	if (!checkAttr(input_char, attrs, table)) return 0;
	return 1;
}

static int
pattern_check_expression(const widechar *const input, int *input_crs,
		const int input_minmax, const int input_dir, const widechar *const expr_data,
		int (*hook)(const widechar input, const int data_len), widechar *hook_data,
		const int hook_max, int expr_crs, int notOperator, int loop_crs, int *loop_cnts,
		const TranslationTableHeader *table) {
	int input_crs_prv, input_start, attrs, ret, i;
	const widechar *data;

	data = NULL;

	/* save input_crs to know if loop consumed input */
	input_start = *input_crs;

	CHECK_OUTPUT(START, 0, __LINE__, "check start")

	while (!(EXPR_TYPE(expr_crs) == PTN_END && EXPR_TYPE(expr_crs) == PTN_END)) {
		/* end of input expression */
		if (EXPR_TYPE(expr_crs) == PTN_END_OF_INPUT) {
			if (*input_crs * input_dir >= input_minmax * input_dir) {
				if (notOperator)
					CHECK_OUTPUT(
							RETURN, 0, __LINE__, "end of input failed:  no input and not")
				else
					CHECK_OUTPUT(RETURN, 1, __LINE__, "end of input passed:  no input")
				return !notOperator;
			} else {
				if (notOperator)
					CHECK_OUTPUT(
							RETURN, 1, __LINE__, "end of input passed:  input and not")
				else
					CHECK_OUTPUT(RETURN, 0, __LINE__, "end of input failed:  input")
				return notOperator;
			}
		}

		/* no more input */
		if (*input_crs * input_dir >= input_minmax * input_dir) {
			switch (EXPR_TYPE(expr_crs)) {
			case PTN_ATTRIBUTES:

				attrs = (EXPR_DATA_0(expr_crs) << 16);
				if (attrs & CTC_EndOfInput) {
					if (notOperator) {
						CHECK_OUTPUT(RETURN, 0, __LINE__,
								"attributes failed:  end of input attribute:  not")
						return 0;
					}
					CHECK_OUTPUT(RETURN, 1, __LINE__,
							"attributes passed:  end of input attribute")
					return 1;
				}
				CHECK_OUTPUT(RETURN, 0, __LINE__,
						"attributes failed:  no end of input attribute")
				return 0;

			case PTN_ANY:
			case PTN_CHARS:

				CHECK_OUTPUT(RETURN, 0, __LINE__, "chars failed:  no input")
				return 0;
			}

			CHECK_OUTPUT(SHOW, 0, __LINE__, "no input")
		}

		switch (EXPR_TYPE(expr_crs)) {

		case PTN_START:

			expr_crs = EXPR_NXT(expr_crs);
			CHECK_OUTPUT(SHOW, 0, __LINE__, "start next")
			break;

		case PTN_GROUP:

			expr_crs = EXPR_DATA_0(expr_crs);
			CHECK_OUTPUT(SHOW, 0, __LINE__, "group next")
			break;

		case PTN_NOT:

			notOperator = !notOperator;
			expr_crs = EXPR_DATA_0(expr_crs);
			CHECK_OUTPUT(SHOW, 0, __LINE__, "not next")
			break;

		case PTN_ONE_MORE:

			CHECK_OUTPUT(SHOW, 0, __LINE__, "loop+ start")

		case PTN_ZERO_MORE:

			/* check if loop already started */
			if (expr_crs == loop_crs) {
				loop_cnts[EXPR_DATA_1(loop_crs)]++;
				CHECK_OUTPUT(SHOW, 0, __LINE__, "loop again")
			} else {
				/* check if loop nested, wasn't running but has a count */
				if (loop_cnts[EXPR_DATA_1(expr_crs)]) {
					CHECK_OUTPUT(SHOW, 0, __LINE__, "loop already running")
					goto loop_next;
				}

				/* start loop */
				loop_crs = expr_crs;
				loop_cnts[EXPR_DATA_1(loop_crs)] = 1;
				CHECK_OUTPUT(SHOW, 0, __LINE__, "loop start")
			}

			/* start loop expression */
			input_crs_prv = *input_crs;
			ret = pattern_check_expression(input, input_crs, input_minmax, input_dir,
					expr_data, hook, hook_data, hook_max, EXPR_DATA_0(expr_crs),
					notOperator, loop_crs, loop_cnts, table);
			if (ret) {
				CHECK_OUTPUT(RETURN, 1, __LINE__, "loop passed")
				return 1;
			}
			CHECK_OUTPUT(SHOW, 0, __LINE__, "loop failed")
			*input_crs = input_crs_prv;

			/* check loop count */
			loop_cnts[EXPR_DATA_1(loop_crs)]--;
			if (EXPR_TYPE(expr_crs) == PTN_ONE_MORE) {
				if (loop_cnts[EXPR_DATA_1(loop_crs)] < 1) {
					CHECK_OUTPUT(RETURN, 0, __LINE__, "loop+ failed")
					return 0;
				} else
					CHECK_OUTPUT(SHOW, 0, __LINE__, "loop+ passed")
			}

		/* continue after loop */
		loop_next:
			expr_crs = EXPR_NXT(expr_crs);
			CHECK_OUTPUT(SHOW, 0, __LINE__, "loop next")
			break;

		case PTN_OPTIONAL:

			/* save current state */
			input_crs_prv = *input_crs;

			/* start optional expression */
			CHECK_OUTPUT(CALL, 0, __LINE__, "option start")
			if (pattern_check_expression(input, input_crs, input_minmax, input_dir,
						expr_data, hook, hook_data, hook_max, EXPR_DATA_0(expr_crs),
						notOperator, loop_crs, loop_cnts, table)) {
				CHECK_OUTPUT(RETURN, 1, __LINE__, "option passed")
				return 1;
			}
			CHECK_OUTPUT(SHOW, 0, __LINE__, "option failed")

			/* continue after optional expression */
			*input_crs = input_crs_prv;
			CHECK_OUTPUT(SHOW, 0, __LINE__, "no option start")
			expr_crs = EXPR_NXT(expr_crs);
			break;

		case PTN_ALTERNATE:

			/* save current state */
			input_crs_prv = *input_crs;

			/* start first expression */
			CHECK_OUTPUT(CALL, 0, __LINE__, "or 1 start")
			if (pattern_check_expression(input, input_crs, input_minmax, input_dir,
						expr_data, hook, hook_data, hook_max, EXPR_DATA_0(expr_crs),
						notOperator, loop_crs, loop_cnts, table)) {
				CHECK_OUTPUT(RETURN, 1, __LINE__, "or 1 passed")
				return 1;
			}
			CHECK_OUTPUT(SHOW, 0, __LINE__, "or 1 failed")

			/* start second expression (no need to push) */
			*input_crs = input_crs_prv;
			CHECK_OUTPUT(SHOW, 0, __LINE__, "or 2 start")
			expr_crs = EXPR_DATA_1(expr_crs);
			break;

		case PTN_ANY:

			CHECK_OUTPUT(SHOW, 0, __LINE__, "any")
			*input_crs += input_dir;
			expr_crs = EXPR_NXT(expr_crs);
			break;

		case PTN_ATTRIBUTES:

			ret = pattern_check_attrs(
					input[*input_crs], EXPR_CONST_DATA(expr_crs), table);
			if (ret && notOperator) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "attributes failed:  not");
				return 0;
			}
			if (!ret && !notOperator) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "attributes failed");
				return 0;
			}
			CHECK_OUTPUT(SHOW, 0, __LINE__, "attributes passed")
			*input_crs += input_dir;
			expr_crs = EXPR_NXT(expr_crs);
			break;

		case PTN_CHARS:

			ret = pattern_check_chars(input[*input_crs], EXPR_CONST_DATA(expr_crs));
			if (ret && notOperator) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "chars failed:  not");
				return 0;
			}
			if (!ret && !notOperator) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "chars failed");
				return 0;
			}
			CHECK_OUTPUT(SHOW, 0, __LINE__, "chars passed")
			*input_crs += input_dir;
			expr_crs = EXPR_NXT(expr_crs);
			break;

		case PTN_HOOK:

			if (hook == NULL) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "hook failed:  NULL");
				return 0;
			}

			/* copy expression data */
			data = EXPR_CONST_DATA(expr_crs);
			for (i = 0; i < data[0]; i++) hook_data[i] = data[i + 1];

			/* call hook function */
			ret = hook(input[*input_crs], data[0]);
			if (ret && notOperator) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "hook failed:  not");
				return 0;
			}
			if (!ret && !notOperator) {
				CHECK_OUTPUT(RETURN, 0, __LINE__, "hook failed");
				return 0;
			}
			CHECK_OUTPUT(SHOW, 0, __LINE__, "hook passed")
			*input_crs += input_dir;
			expr_crs = EXPR_NXT(expr_crs);
			break;

		case PTN_END:
			break;

		default:

			CHECK_OUTPUT(RETURN, 0, __LINE__, "unknown opcode")
			return 0;
		}

		/* check end expression  */
		while (EXPR_TYPE(expr_crs) == PTN_END) {
			CHECK_OUTPUT(SHOW, 0, __LINE__, "end")

			/* check for end of expressions */
			if (EXPR_NXT(expr_crs) == PTN_END) break;

			expr_crs = EXPR_NXT(expr_crs);

			/* returning loop */
			if (EXPR_TYPE(expr_crs) == PTN_ZERO_MORE ||
					EXPR_TYPE(expr_crs) == PTN_ONE_MORE) {
				CHECK_OUTPUT(SHOW, 0, __LINE__, "end loop")

				/* check that loop consumed input */
				if (*input_crs == input_start) {
					CHECK_OUTPUT(RETURN, 0, __LINE__, "loop failed:  did not consume")
					return 0;
				}

				/* loops do not continue to the next expression */
				break;
			}

			/* returning not */
			if (EXPR_TYPE(expr_crs) == PTN_NOT) notOperator = !notOperator;

			expr_crs = EXPR_NXT(expr_crs);

			CHECK_OUTPUT(SHOW, 0, __LINE__, "end next")
		}

		CHECK_OUTPUT(SHOW, 0, __LINE__, "check next")
	}

	CHECK_OUTPUT(RETURN, 1, __LINE__, "check passed:  end of expression");
	return 1;
}

static int
pattern_check_hook(const widechar *input, const int input_start, const int input_minmax,
		const int input_dir, const widechar *expr_data,
		int (*hook)(const widechar input, const int data_len), widechar *hook_data,
		const int hook_max, const TranslationTableHeader *table) {
	int input_crs, ret, *loop_cnts;

	input_crs = input_start;
	loop_cnts = malloc(expr_data[1] * sizeof(int));
	memset(loop_cnts, 0, expr_data[1] * sizeof(int));
	ret = pattern_check_expression(input, &input_crs, input_minmax, input_dir, expr_data,
			hook, hook_data, hook_max, 2, 0, 0, loop_cnts, table);
	free(loop_cnts);
	return ret;
}

int EXPORT_CALL
_lou_pattern_check(const widechar *input, const int input_start, const int input_minmax,
		const int input_dir, const widechar *expr_data,
		const TranslationTableHeader *table) {
#ifdef CHECK_OUTPUT_DEFINED
	pattern_output(expr_data, table);
#endif
	return pattern_check_hook(
			input, input_start, input_minmax, input_dir, expr_data, NULL, NULL, 0, table);
}

////////////////////////////////////////////////////////////////////////////////
