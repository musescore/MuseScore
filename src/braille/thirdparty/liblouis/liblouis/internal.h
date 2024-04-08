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
 * @brief Internal API of liblouis
 */

#ifndef __LOUIS_H_
#define __LOUIS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include "liblouis.h"

/* Unlike Windows, Mingw can handle forward slashes as directory
   separator, see http://mingw.org/wiki/Posix_path_conversion */
#if defined(_WIN32) && !defined(__MINGW32__) && !defined(__MINGW64__)
#define PATH_SEP ';'
#define DIR_SEP '\\'
#else
#define PATH_SEP ':'
#define DIR_SEP '/'
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#define NUMVAR 50
#define EMPHMODECHARSSIZE 256
#define NOEMPHCHARSSIZE 256
#define LETSIGNSIZE 256
// noletsignbefore and noletsignafter is hardly ever used and usually
// only with very few chars, so it only needs a small array
#define LETSIGNBEFORESIZE 64
#define LETSIGNAFTERSIZE 64
#define SEQPATTERNSIZE 128
#define CHARSIZE sizeof(widechar)
#define DEFAULTRULESIZE 50

typedef struct intCharTupple {
	unsigned long long key;
	char value;
} intCharTupple;

/* HASHNUM must be prime */
#define HASHNUM 1123

#define MAXPASS 4
#define MAXSTRING 2048
#define MAX_MACRO_VAR 100  // maximal number of variable substitutions a macro can contain
#define MAX_EMPH_CLASSES 10	  // maximal number of emphasis classes
#define MAX_MODES 6			  // maximal number of modes that can be handled
#define MAX_SOURCE_FILES 100  // maximal number of files a table can consist of

typedef unsigned int TranslationTableOffset;

/* Basic type for translation table data, which carries all alignment
 * constraints that fields contained in translation table may have.
 * Notably TranslationTableCharacterAttributes is unsigned long long, so we need
 * at least this big basic type. */
typedef unsigned long long TranslationTableData;
#define OFFSETSIZE sizeof(TranslationTableData)

typedef enum {
	/* The first 8 are the predefined character classes. They need to be listed first and
	   in this order because of how allocateCharacterClasses works. */
	CTC_Space = 0x1,
	CTC_Letter = 0x2,
	CTC_Digit = 0x4,
	CTC_Punctuation = 0x8,
	CTC_UpperCase = 0x10,
	CTC_LowerCase = 0x20,
	CTC_Math = 0x40,
	CTC_Sign = 0x80,
	CTC_LitDigit = 0x100,
	CTC_CapsMode = 0x200,
	// bit 0x400 used to be taken by CTC_EmphMode
	CTC_NumericMode = 0x800,
	CTC_NumericNoContract = 0x1000,
	CTC_SeqDelimiter = 0x2000,
	CTC_SeqBefore = 0x4000,
	CTC_SeqAfter = 0x8000,
	/* The following 8 are reserved for %0 to %7 (in no particular order) */
	/* Be careful with changing these values (and also CTC_EndOfInput) because in
	   pattern_compile_expression they are stored in a unsigned int after cutting of the
	   16 least significant bits. */
	CTC_UserDefined1 = 0x10000,
	CTC_UserDefined2 = 0x20000,
	CTC_UserDefined3 = 0x40000,
	CTC_UserDefined4 = 0x80000,
	CTC_UserDefined5 = 0x100000,
	CTC_UserDefined6 = 0x200000,
	CTC_UserDefined7 = 0x400000,
	CTC_UserDefined8 = 0x800000,
	CTC_EndOfInput = 0x1000000,	 // only used by pattern matcher
	CTC_EmpMatch = 0x2000000,	 // only used in TranslationTableRule->before and
								 // TranslationTableRule->after
	CTC_MidEndNumericMode = 0x4000000,
	/* At least 37 more bits available in a unsigned long long (at least 64 bits). Used
	   for custom attributes 9 to 45. These need to be the last values of the enum. */
	CTC_UserDefined9 = 0x8000000,
	CTC_UserDefined10 = 0x10000000,
	CTC_UserDefined11 = 0x20000000,
	CTC_UserDefined12 = 0x40000000,
} TranslationTableCharacterAttribute;

typedef enum {
	pass_first = '`',
	pass_last = '~',
	pass_lookback = '_',
	pass_string = '\"',
	pass_dots = '@',
	pass_omit = '?',
	pass_startReplace = '[',
	pass_endReplace = ']',
	pass_startGroup = '{',
	pass_endGroup = '}',
	pass_variable = '#',
	pass_not = '!',
	pass_search = '/',
	pass_any = 'a',
	pass_digit = 'd',
	pass_litDigit = 'D',
	pass_letter = 'l',
	pass_math = 'm',
	pass_punctuation = 'p',
	pass_sign = 'S',
	pass_space = 's',
	pass_uppercase = 'U',
	pass_lowercase = 'u',
	pass_class1 = 'w',
	pass_class2 = 'x',
	pass_class3 = 'y',
	pass_class4 = 'z',
	pass_attributes = '$',
	pass_groupstart = '{',
	pass_groupend = '}',
	pass_groupreplace = ';',
	pass_swap = '%',
	pass_hyphen = '-',
	pass_until = '.',
	pass_eq = '=',
	pass_lt = '<',
	pass_gt = '>',
	pass_endTest = 32,
	pass_plus = '+',
	pass_copy = '*',
	pass_leftParen = '(',
	pass_rightParen = ')',
	pass_comma = ',',
	pass_lteq = 130,
	pass_gteq = 131,
	pass_invalidToken = 132,
	pass_noteq = 133,
	pass_and = 134,
	pass_or = 135,
	pass_nameFound = 136,
	pass_numberFound = 137,
	pass_boolean = 138,
	pass_class = 139,
	pass_define = 140,
	pass_emphasis = 141,
	pass_group = 142,
	pass_mark = 143,
	pass_repGroup = 143,
	pass_script = 144,
	pass_noMoreTokens = 145,
	pass_replace = 146,
	pass_if = 147,
	pass_then = 148,
	pass_all = 255
} pass_Codes;

typedef unsigned long long TranslationTableCharacterAttributes;

typedef struct {
	TranslationTableOffset next;
	widechar lookFor;
	widechar found;
} CharDotsMapping;

typedef struct {
	const char *sourceFile;
	int sourceLine;
	TranslationTableOffset next;
	TranslationTableOffset definitionRule;
	TranslationTableOffset otherRules;
	TranslationTableCharacterAttributes attributes;
	TranslationTableCharacterAttributes mode;
	TranslationTableOffset compRule;
	widechar value;
	TranslationTableOffset basechar;
	TranslationTableOffset linked;
} TranslationTableCharacter;

typedef enum { /* Op codes */
	CTO_IncludeFile,
	CTO_Locale, /* Deprecated, do not use */
	CTO_Undefined,
	/* Do not change the order of the following opcodes! */
	CTO_CapsLetter,
	CTO_BegCapsWord,
	CTO_EndCapsWord,
	CTO_BegCaps,
	CTO_EndCaps,
	CTO_BegCapsPhrase,
	CTO_EndCapsPhrase,
	CTO_LenCapsPhrase,
	CTO_ModeLetter,
	CTO_BegModeWord,
	CTO_EndModeWord,
	CTO_BegMode,
	CTO_EndMode,
	CTO_BegModePhrase,
	CTO_EndModePhrase,
	CTO_LenModePhrase,
	/* End of ordered opcodes */
	CTO_LetterSign,
	CTO_NoLetsignBefore,
	CTO_NoLetsign,
	CTO_NoLetsignAfter,
	CTO_NumberSign,
	CTO_NoNumberSign,
	CTO_NumericModeChars,
	CTO_MidEndNumericModeChars,
	CTO_NumericNoContractChars,
	CTO_SeqDelimiter,
	CTO_SeqBeforeChars,
	CTO_SeqAfterChars,
	CTO_SeqAfterPattern,
	CTO_SeqAfterExpression,
	CTO_EmphClass,

	/* Do not change the order of the following opcodes! */
	CTO_EmphLetter,
	CTO_BegEmphWord,
	CTO_EndEmphWord,
	CTO_BegEmph,
	CTO_EndEmph,
	CTO_BegEmphPhrase,
	CTO_EndEmphPhrase,
	CTO_LenEmphPhrase,
	/* End of ordered opcodes */

	CTO_CapsModeChars,
	CTO_EmphModeChars,
	CTO_NoEmphChars,
	CTO_BegComp,
	CTO_EndComp,
	CTO_NoContractSign,
	CTO_MultInd,
	CTO_CompDots,
	CTO_Comp6,
	CTO_Class,	/* define a character class */
	CTO_After,	/* only match if after character in class */
	CTO_Before, /* only match if before character in class 30 */
	CTO_NoBack,
	CTO_NoFor,
	CTO_EmpMatchBefore,
	CTO_EmpMatchAfter,
	CTO_SwapCc,
	CTO_SwapCd,
	CTO_SwapDd,
	CTO_Space,
	CTO_Digit,
	CTO_Punctuation,
	CTO_Math,
	CTO_Sign,
	CTO_Letter,
	CTO_UpperCase,
	CTO_LowerCase,
	CTO_Grouping,
	CTO_UpLow,
	CTO_LitDigit,
	CTO_Display,
	CTO_Replace,
	CTO_Context,
	CTO_Correct,
	CTO_Pass2,
	CTO_Pass3,
	CTO_Pass4,
	CTO_Repeated,
	CTO_RepWord,
	CTO_RepEndWord,
	CTO_CapsNoCont,
	CTO_Always,
	CTO_ExactDots,
	CTO_NoCross,
	CTO_Syllable,
	CTO_NoCont,
	CTO_CompBrl,
	CTO_Literal,
	CTO_LargeSign,
	CTO_WholeWord,
	CTO_PartWord,
	CTO_JoinNum,
	CTO_JoinableWord,
	CTO_LowWord,
	CTO_Contraction,
	CTO_SuffixableWord, /** whole word or beginning of word */
	CTO_PrefixableWord, /** whole word or end of word */
	CTO_BegWord,		/** beginning of word only */
	CTO_BegMidWord,		/** beginning or middle of word */
	CTO_MidWord,		/** middle of word only 20 */
	CTO_MidEndWord,		/** middle or end of word */
	CTO_EndWord,		/** end of word only */
	CTO_PrePunc,		/** punctuation in string at beginning of word */
	CTO_PostPunc,		/** punctuation in string at end of word */
	CTO_BegNum,			/** beginning of number */
	CTO_MidNum,			/** middle of number, e.g., decimal point */
	CTO_EndNum,			/** end of number */
	CTO_DecPoint,
	CTO_Hyphen,
	// CTO_Apostrophe,
	// CTO_Initial,
	CTO_NoBreak,
	CTO_Match,
	CTO_BackMatch,
	CTO_Attribute,
	CTO_Base,
	CTO_Macro,
	CTO_None,

	/** "internal" opcodes */
	CTO_EndCapsPhraseBefore,
	CTO_EndCapsPhraseAfter,

	CTO_All
} TranslationTableOpcode;

typedef struct {
	const char *sourceFile;
	int sourceLine;
	TranslationTableOffset charsnext;			/** next chars entry */
	TranslationTableOffset dotsnext;			/** next dots entry */
	TranslationTableCharacterAttributes after;	/** character types which must follow */
	TranslationTableCharacterAttributes before; /** character types which must precede */
	TranslationTableOffset patterns;			/** before and after patterns */
	TranslationTableOpcode opcode; /** rule for testing validity of replacement */
	char nocross;
	short charslen;						 /** length of string to be replaced */
	short dotslen;						 /** length of replacement string */
	widechar charsdots[DEFAULTRULESIZE]; /** find and replacement strings */
} TranslationTableRule;

typedef struct /* state transition */
{
	widechar ch;
	widechar newState;
} HyphenationTrans;

typedef union {
	HyphenationTrans *pointer;
	TranslationTableOffset offset;
} PointOff;

typedef struct /* one state */
{
	PointOff trans;
	TranslationTableOffset hyphenPattern;
	widechar fallbackState;
	widechar numTrans;
} HyphenationState;

typedef struct CharacterClass {
	struct CharacterClass *next;
	TranslationTableCharacterAttributes attribute;
	widechar length;
	widechar name[1];
} CharacterClass;

typedef struct RuleName {
	struct RuleName *next;
	TranslationTableOffset ruleOffset;
	widechar length;
	widechar name[1];
} RuleName;

typedef struct {
	/* either typeform or mode should be set, not both */
	formtype typeform; /* corresponding value in "typeforms" enum */
	TranslationTableCharacterAttributes mode; /* corresponding character attribute */
	unsigned int value;						  /* bit field that contains a single "1" */
	unsigned short
			rule; /* emphasis rules (index in emphRules, emphModeChars and noEmphChars) */
} EmphasisClass;

typedef struct {
	TranslationTableOffset tableSize;
	TranslationTableOffset bytesUsed;
	TranslationTableOffset charToDots[HASHNUM];
	TranslationTableOffset dotsToChar[HASHNUM];
	TranslationTableData ruleArea[1]; /** Space for storing all rules and values */
} DisplayTableHeader;

/**
 * Translation table header
 */
typedef struct { /* translation table */

	/* state needed during compilation */
	TranslationTableOffset tableSize;
	TranslationTableOffset bytesUsed;
	CharacterClass *characterClasses;
	TranslationTableCharacterAttributes nextCharacterClassAttribute;
	TranslationTableCharacterAttributes nextNumberedCharacterClassAttribute;
	RuleName *ruleNames;
	TranslationTableCharacterAttributes
			numberedAttributes[8]; /* attributes 0-7 used in match rules (could also be
								   stored in `characterClasses', but this is slightly
								   faster) */
	int usesAttributeOrClass;	   /* 1 = attribute, 2 = class */
	char *sourceFiles[MAX_SOURCE_FILES + 1];

	/* needed for translation or other api functions */
	int finalized;
	int capsNoCont;
	int numPasses;
	int corrections;
	int syllables;
	int usesSequences;
	int usesNumericMode;
	int hasCapsModeChars;
	TranslationTableOffset undefined;
	TranslationTableOffset letterSign;
	TranslationTableOffset numberSign;
	TranslationTableOffset noContractSign;
	TranslationTableOffset noNumberSign;
	widechar seqPatterns[SEQPATTERNSIZE];
	char *emphClassNames[MAX_EMPH_CLASSES];
	EmphasisClass emphClasses[MAX_EMPH_CLASSES];
	EmphasisClass modes[MAX_MODES];
	int seqPatternsCount;
	widechar seqAfterExpression[SEQPATTERNSIZE];
	int seqAfterExpressionLength;
	TranslationTableOffset emphRules[MAX_EMPH_CLASSES + MAX_MODES]
									[9]; /* 9 is the size of the EmphCodeOffset enum */
	TranslationTableOffset begComp;
	TranslationTableOffset endComp;
	TranslationTableOffset hyphenStatesArray;
	widechar noLetsignBefore[LETSIGNBEFORESIZE];
	int noLetsignBeforeCount;
	widechar noLetsign[LETSIGNSIZE];
	int noLetsignCount;
	widechar noLetsignAfter[LETSIGNAFTERSIZE];
	int noLetsignAfterCount;
	widechar emphModeChars[MAX_EMPH_CLASSES] /* does not include caps: capsmodechars are
											  * currently stored as character attributes
											  */
						  [EMPHMODECHARSSIZE + 1];
	widechar noEmphChars[MAX_EMPH_CLASSES] /* does not include caps */
						[NOEMPHCHARSSIZE + 1];
	TranslationTableOffset characters[HASHNUM]; /** Character definitions */
	TranslationTableOffset dots[HASHNUM];		/** Dot definitions */
	TranslationTableOffset forPassRules[MAXPASS + 1];
	TranslationTableOffset backPassRules[MAXPASS + 1];
	TranslationTableOffset forRules[HASHNUM];  /** chains of forward rules */
	TranslationTableOffset backRules[HASHNUM]; /** Chains of backward rules */
	TranslationTableData ruleArea[1]; /** Space for storing all rules and values */
} TranslationTableHeader;

typedef enum {
	alloc_typebuf,
	alloc_wordBuffer,
	alloc_emphasisBuffer,
	alloc_destSpacing,
	alloc_passbuf,
	alloc_posMapping1,
	alloc_posMapping2,
	alloc_posMapping3
} AllocBuf;

#define MAXPASSBUF 3

typedef enum {
	begPhraseOffset = 0,
	endPhraseBeforeOffset = 1,
	endPhraseAfterOffset = 2,
	begOffset = 3,
	endOffset = 4,
	letterOffset = 5,
	begWordOffset = 6,
	endWordOffset = 7,
	lenPhraseOffset = 8
} EmphCodeOffset;

/* Grouping the begin, end, word and symbol bits and using the type of
 * a single bit group for representing the emphasis classes allows us
 * to do simple bit operations. */

/* fields contain sums of EmphasisClass.value */
/* MAX_EMPH_CLASSES + MAX_MODES may not exceed 16 */
typedef struct {
	unsigned int begin : 16;
	unsigned int end : 16;
	unsigned int word : 16;
	unsigned int symbol : 16;
} EmphasisInfo;

typedef enum { noEncoding, bigEndian, littleEndian, ascii8 } EncodingType;

typedef struct {
	const char *fileName;
	const char *sourceFile;
	FILE *in;
	int lineNumber;
	EncodingType encoding;
	int status;
	int linelen;
	int linepos;
	int checkencoding[2];
	widechar line[MAXSTRING];
} FileInfo;

/* The following function definitions are hooks into
 * compileTranslationTable.c. Some are used by other library modules.
 * Others are used by tools like lou_allround.c and lou_debug.c. */

/**
 * Comma separated list of directories to search for tables.
 */
char *EXPORT_CALL
_lou_getTablePath(void);

/**
 * Resolve tableList against base.
 */
char **EXPORT_CALL
_lou_resolveTable(const char *tableList, const char *base);

/**
 * The default table resolver
 */
char **EXPORT_CALL
_lou_defaultTableResolver(const char *tableList, const char *base);

/**
 * muse::Return single-cell dot pattern corresponding to a character.
 * TODO: move to commonTranslationFunctions.c
 */
widechar EXPORT_CALL
_lou_getDotsForChar(widechar c, const DisplayTableHeader *table);

/**
 * muse::Return character corresponding to a single-cell dot pattern.
 * TODO: move to commonTranslationFunctions.c
 */
widechar EXPORT_CALL
_lou_getCharForDots(widechar d, const DisplayTableHeader *table);

void EXPORT_CALL
_lou_getTable(const char *tableList, const char *displayTableList,
		const TranslationTableHeader **translationTable,
		const DisplayTableHeader **displayTable);

const TranslationTableHeader *EXPORT_CALL
_lou_getTranslationTable(const char *tableList);

const DisplayTableHeader *EXPORT_CALL
_lou_getDisplayTable(const char *tableList);

int EXPORT_CALL
_lou_compileTranslationRule(const char *tableList, const char *inString);

int EXPORT_CALL
_lou_compileDisplayRule(const char *tableList, const char *inString);

/**
 * Allocate memory for internal buffers
 *
 * Used by lou_translateString.c and lou_backTranslateString.c ONLY
 * to allocate memory for internal buffers.
 * TODO: move to utils.c
 */
void *EXPORT_CALL
_lou_allocMem(AllocBuf buffer, int index, int srcmax, int destmax);

/**
 * Hash function for character strings
 *
 * @param lowercase Whether to convert the string to lowercase because
 *                  making the hash of it.
 */
unsigned long int EXPORT_CALL
_lou_stringHash(const widechar *c, int lowercase, const TranslationTableHeader *table);

/**
 * Hash function for single characters
 */
unsigned long int EXPORT_CALL
_lou_charHash(widechar c);

/**
 * muse::Return a string in the same format as the characters operand in opcodes
 */
const char *EXPORT_CALL
_lou_showString(widechar const *chars, int length, int forceHex);

/**
 * Print out dot numbers
 *
 * @return a string containing the dot numbers. The longest possible
 * output is "\123456789ABCDEF0/"
 */
const char *EXPORT_CALL
_lou_unknownDots(widechar dots);

/**
 * muse::Return a character string in the format of the dots operand
 */
const char *EXPORT_CALL
_lou_showDots(widechar const *dots, int length);

/**
 * muse::Return a character string where the attributes are indicated
 * by the attribute letters used in multipass opcodes
 */
char *EXPORT_CALL
_lou_showAttributes(TranslationTableCharacterAttributes a);

/**
 * muse::Return number of the opcode
 *
 * @param toFind the opcodes
 */
TranslationTableOpcode EXPORT_CALL
_lou_findOpcodeNumber(const char *tofind);

/**
 * muse::Return the name of the opcode associated with an opcode number
 *
 * @param opcode an opcode
 */
const char *EXPORT_CALL
_lou_findOpcodeName(TranslationTableOpcode opcode);

/**
 * Convert string to wide characters
 *
 * Takes a character string and produces a sequence of wide characters.
 * Opposite of _lou_showString.
 *
 * @param inString the input string
 * @param outString the output wide char sequence
 * @return length of the widechar sequence.
 */
int EXPORT_CALL
_lou_extParseChars(const char *inString, widechar *outString);

/**
 * Convert string to wide characters containing dot patterns
 *
 * Takes a character string and produces a sequence of wide characters
 * containing dot patterns. Opposite of _lou_showDots.
 * @param inString the input string
 * @param outString the output wide char sequence
 * @return length of the widechar sequence.
 */
int EXPORT_CALL
_lou_extParseDots(const char *inString, widechar *outString);

int EXPORT_CALL
_lou_translate(const char *tableList, const char *displayTableList, const widechar *inbuf,
		int *inlen, widechar *outbuf, int *outlen, formtype *typeform, char *spacing,
		int *outputPos, int *inputPos, int *cursorPos, int mode,
		const TranslationTableRule **rules, int *rulesLen);

int EXPORT_CALL
_lou_backTranslate(const char *tableList, const char *displayTableList,
		const widechar *inbuf, int *inlen, widechar *outbuf, int *outlen,
		formtype *typeform, char *spacing, int *outputPos, int *inputPos, int *cursorPos,
		int mode, const TranslationTableRule **rules, int *rulesLen);

void EXPORT_CALL
_lou_resetPassVariables(void);

int EXPORT_CALL
_lou_handlePassVariableTest(const widechar *instructions, int *IC, int *itsTrue);

int EXPORT_CALL
_lou_handlePassVariableAction(const widechar *instructions, int *IC);

int EXPORT_CALL
_lou_pattern_compile(const widechar *input, const int input_max, widechar *expr_data,
		const int expr_max, TranslationTableHeader *table, const FileInfo *nested);

void EXPORT_CALL
_lou_pattern_reverse(widechar *expr_data);

int EXPORT_CALL
_lou_pattern_check(const widechar *input, const int input_start, const int input_minmax,
		const int input_dir, const widechar *expr_data,
		const TranslationTableHeader *table);

/**
 * Read a line of widechar's from an input file
 */
int EXPORT_CALL
_lou_getALine(FileInfo *info);

#ifdef DEBUG
/* Can be inserted in code to be used as a breakpoint in gdb */
void EXPORT_CALL
_lou_debugHook(void);
#endif

/**
 * Print an out-of-memory message and exit
 */
void EXPORT_CALL
_lou_outOfMemory(void);

/**
 * Helper for logging a widechar buffer
 */
void EXPORT_CALL
_lou_logWidecharBuf(logLevels level, const char *msg, const widechar *wbuf, int wlen);

void EXPORT_CALL
_lou_logMessage(logLevels level, const char *format, ...);

extern int translation_direction;

/**
 * muse::Return 1 if given translation mode is valid. muse::Return 0 otherwise.
 */
int EXPORT_CALL
_lou_isValidMode(int mode);

/**
 * muse::Return the default braille representation for a character.
 */
widechar EXPORT_CALL
_lou_charToFallbackDots(widechar c);

static inline int
isASCII(widechar c) {
	return (c >= 0X20) && (c < 0X7F);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LOUIS_H_ */
