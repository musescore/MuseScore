#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "darray.h"
#include "tts.h"

static const char *ASCII[] =
{
	"null", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"", "", "", "",
	"space", "exclamation mark", "double quote", "hash",
	"dollar", "percent", "ampersand", "quote",
	"open parenthesis", "close parenthesis", "asterisk", "plus",
	"comma", "minus", "full stop", "slash",
	"zero", "one", "two", "three",
	"four", "five", "six", "seven",
	"eight", "nine", "colon", "semi colon",
	"less than", "equals", "greater than", "question mark",
#ifndef ALPHA_IN_DICT
	"at", "ay", "bee", "see",
	"dee", "e", "eff", "gee",
	"aych", "i", "jay", "kay",
	"ell", "em", "en", "ohe",
	"pee", "kju", "are", "es",
	"tee", "you", "vee", "double you",
	"eks", "why", "zed", "open bracket",
#else                             /* ALPHA_IN_DICT */
	"at", "A", "B", "C",
	"D", "E", "F", "G",
	"H", "I", "J", "K",
	"L", "M", "N", "O",
	"P", "Q", "R", "S",
	"T", "U", "V", "W",
	"X", "Y", "Z", "open bracket",
#endif                            /* ALPHA_IN_DICT */
	"back slash", "close bracket", "circumflex", "underscore",
#ifndef ALPHA_IN_DICT
	"back quote", "ay", "bee", "see",
	"dee", "e", "eff", "gee",
	"aych", "i", "jay", "kay",
	"ell", "em", "en", "ohe",
	"pee", "kju", "are", "es",
	"tee", "you", "vee", "double you",
	"eks", "why", "zed", "open brace",
#else                             /* ALPHA_IN_DICT */
	"back quote", "A", "B", "C",
	"D", "E", "F", "G",
	"H", "I", "J", "K",
	"L", "M", "N", "O",
	"P", "Q", "R", "S",
	"T", "U", "V", "W",
	"X", "Y", "Z", "open brace",
#endif                            /* ALPHA_IN_DICT */
	"vertical bar", "close brace", "tilde", "delete",
	NULL
};

/* Context definitions */
static const char Anything[] = "";
/* No context requirement */

static const char Nothing[] = " ";
/* Context is beginning or end of word */

static const char Silent[] = "";
/* No phonemes */


#define LEFT_PART       0
#define MATCH_PART      1
#define RIGHT_PART      2
#define OUT_PART        3

typedef const char *Rule[4];
/* Rule is an array of 4 character pointers */


/*0 = Punctuation */
/*
**      LEFT_PART       MATCH_PART      RIGHT_PART      OUT_PART
*/


static Rule punct_rules[] =
{
	{Anything, " ", Anything, " "},
	{Anything, "-", Anything, ""},
	{".", "'S", Anything, "z"},
	{"#:.E", "'S", Anything, "z"},
	{"#", "'S", Anything, "z"},
	{Anything, "'", Anything, ""},
	{Anything, ",", Anything, " "},
	{Anything, ".", Anything, " "},
	{Anything, "?", Anything, " "},
	{Anything, "!", Anything, " "},
	{Anything, 0, Anything, Silent},
};

static Rule A_rules[] =
{
	{Anything, "A", Nothing, "@"},
	{Nothing, "ARE", Nothing, "0r"},
	{Nothing, "AR", "O", "@r"},
	{Anything, "AR", "#", "er"},
	{"^", "AS", "#", "eIs"},
	{Anything, "A", "WA", "@"},
	{Anything, "AW", Anything, "O"},
	{" :", "ANY", Anything, "eni"},
	{Anything, "A", "^+#", "eI"},
	{"#:", "ALLY", Anything, "@li"},
	{Nothing, "AL", "#", "@l"},
	{Anything, "AGAIN", Anything, "@gen"},
	{"#:", "AG", "E", "IdZ"},
	{Anything, "A", "^+:#", "&"},
	{" :", "A", "^+ ", "eI"},
	{Anything, "A", "^%", "eI"},
	{Nothing, "ARR", Anything, "@r"},
	{Anything, "ARR", Anything, "&r"},
	{" :", "AR", Nothing, "0r"},
	{Anything, "AR", Nothing, "3"},
	{Anything, "AR", Anything, "0r"},
	{Anything, "AIR", Anything, "er"},
	{Anything, "AI", Anything, "eI"},
	{Anything, "AY", Anything, "eI"},
	{Anything, "AU", Anything, "O"},
	{"#:", "AL", Nothing, "@l"},
	{"#:", "ALS", Nothing, "@lz"},
	{Anything, "ALK", Anything, "Ok"},
	{Anything, "AL", "^", "Ol"},
	{" :", "ABLE", Anything, "eIb@l"},
	{Anything, "ABLE", Anything, "@b@l"},
	{Anything, "ANG", "+", "eIndZ"},
	{"^", "A", "^#", "eI"},
	{Anything, "A", Anything, "&"},
	{Anything, 0, Anything, Silent},
};

static Rule B_rules[] =
{
	{Nothing, "BE", "^#", "bI"},
	{Anything, "BEING", Anything, "biIN"},
	{Nothing, "BOTH", Nothing, "b@UT"},
	{Nothing, "BUS", "#", "bIz"},
	{Anything, "BUIL", Anything, "bIl"},
	{Anything, "B", Anything, "b"},
	{Anything, 0, Anything, Silent},
};

static Rule C_rules[] =
{
	{Nothing, "CH", "^", "k"},
	{"^E", "CH", Anything, "k"},
	{Anything, "CH", Anything, "tS"},
	{" S", "CI", "#", "saI"},
	{Anything, "CI", "A", "S"},
	{Anything, "CI", "O", "S"},
	{Anything, "CI", "EN", "S"},
	{Anything, "C", "+", "s"},
	{Anything, "CK", Anything, "k"},
	{Anything, "COM", "%", "kVm"},
	{Anything, "C", Anything, "k"},
	{Anything, 0, Anything, Silent},
};

static Rule D_rules[] =
{
	{"#:", "DED", Nothing, "dId"},
	{".E", "D", Nothing, "d"},
	{"#:^E", "D", Nothing, "t"},
	{Nothing, "DE", "^#", "dI"},
	{Nothing, "DO", Nothing, "mDU"},
	{Nothing, "DOES", Anything, "dVz"},
	{Nothing, "DOING", Anything, "duIN"},
	{Nothing, "DOW", Anything, "daU"},
	{Anything, "DU", "A", "dZu"},
	{Anything, "D", Anything, "d"},
	{Anything, 0, Anything, Silent},
};

static Rule E_rules[] =
{
	{"#:", "E", Nothing, ""},
	{"':^", "E", Nothing, ""},
	{" :", "E", Nothing, "i"},
	{"#", "ED", Nothing, "d"},
	{"#:", "E", "D ", ""},
	{Anything, "EV", "ER", "ev"},
	{Anything, "E", "^%", "i"},
	{Anything, "ERI", "#", "iri"},
	{Anything, "ERI", Anything, "erI"},
	{"#:", "ER", "#", "3"},
	{Anything, "ER", "#", "er"},
	{Anything, "ER", Anything, "3"},
	{Nothing, "EVEN", Anything, "iven"},
	{"#:", "E", "W", ""},
	{"T", "EW", Anything, "u"},
	{"S", "EW", Anything, "u"},
	{"R", "EW", Anything, "u"},
	{"D", "EW", Anything, "u"},
	{"L", "EW", Anything, "u"},
	{"Z", "EW", Anything, "u"},
	{"N", "EW", Anything, "u"},
	{"J", "EW", Anything, "u"},
	{"TH", "EW", Anything, "u"},
	{"CH", "EW", Anything, "u"},
	{"SH", "EW", Anything, "u"},
	{Anything, "EW", Anything, "ju"},
	{Anything, "E", "O", "i"},
	{"#:S", "ES", Nothing, "Iz"},
	{"#:C", "ES", Nothing, "Iz"},
	{"#:G", "ES", Nothing, "Iz"},
	{"#:Z", "ES", Nothing, "Iz"},
	{"#:X", "ES", Nothing, "Iz"},
	{"#:J", "ES", Nothing, "Iz"},
	{"#:CH", "ES", Nothing, "Iz"},
	{"#:SH", "ES", Nothing, "Iz"},
	{"#:", "E", "S ", ""},
	{"#:", "ELY", Nothing, "li"},
	{"#:", "EMENT", Anything, "ment"},
	{Anything, "EFUL", Anything, "fUl"},
	{Anything, "EE", Anything, "i"},
	{Anything, "EARN", Anything, "3n"},
	{Nothing, "EAR", "^", "3"},
	{Anything, "EAD", Anything, "ed"},
	{"#:", "EA", Nothing, "i@"},
	{Anything, "EA", "SU", "e"},
	{Anything, "EA", Anything, "i"},
	{Anything, "EIGH", Anything, "eI"},
	{Anything, "EI", Anything, "i"},
	{Nothing, "EYE", Anything, "aI"},
	{Anything, "EY", Anything, "i"},
	{Anything, "EU", Anything, "ju"},
	{Anything, "E", Anything, "e"},
	{Anything, 0, Anything, Silent},
};

static Rule F_rules[] =
{
	{Anything, "FUL", Anything, "fUl"},
	{Anything, "F", Anything, "f"},
	{Anything, 0, Anything, Silent},
};

static Rule G_rules[] =
{
	{Anything, "GIV", Anything, "gIv"},
	{Nothing, "G", "I^", "g"},
	{Anything, "GE", "T", "ge"},
	{"SU", "GGES", Anything, "gdZes"},
	{Anything, "GG", Anything, "g"},
	{" B#", "G", Anything, "g"},
	{Anything, "G", "+", "dZ"},
	{Anything, "GREAT", Anything, "greIt"},
	{"#", "GH", Anything, ""},
	{Anything, "G", Anything, "g"},
	{Anything, 0, Anything, Silent},
};

static Rule H_rules[] =
{
	{Nothing, "HAV", Anything, "h&v"},
	{Nothing, "HERE", Anything, "hir"},
	{Nothing, "HOUR", Anything, "aU3"},
	{Anything, "HOW", Anything, "haU"},
	{Anything, "H", "#", "h"},
	{Anything, "H", Anything, ""},
	{Anything, 0, Anything, Silent},
};

static Rule I_rules[] =
{
	{Nothing, "IAIN", Nothing, "I@n"},
	{Nothing, "ING", Nothing, "IN"},
	{Nothing, "IN", Anything, "In"},
	{Nothing, "I", Nothing, "aI"},
	{Anything, "IN", "D", "aIn"},
	{Anything, "IER", Anything, "i3"},
	{"#:R", "IED", Anything, "id"},
	{Anything, "IED", Nothing, "aId"},
	{Anything, "IEN", Anything, "ien"},
	{Anything, "IE", "T", "aIe"},
	{" :", "I", "%", "aI"},
	{Anything, "I", "%", "i"},
	{Anything, "IE", Anything, "i"},
	{Anything, "I", "^+:#", "I"},
	{Anything, "IR", "#", "aIr"},
	{Anything, "IZ", "%", "aIz"},
	{Anything, "IS", "%", "aIz"},
	{Anything, "I", "D%", "aI"},
	{"+^", "I", "^+", "I"},
	{Anything, "I", "T%", "aI"},
	{"#:^", "I", "^+", "I"},
	{Anything, "I", "^+", "aI"},
	{Anything, "IR", Anything, "3"},
	{Anything, "IGH", Anything, "aI"},
	{Anything, "ILD", Anything, "aIld"},
	{Anything, "IGN", Nothing, "aIn"},
	{Anything, "IGN", "^", "aIn"},
	{Anything, "IGN", "%", "aIn"},
	{Anything, "IQUE", Anything, "ik"},
	{"^", "I", "^#", "aI"},
	{Anything, "I", Anything, "I"},
	{Anything, 0, Anything, Silent},
};

static Rule J_rules[] =
{
	{Anything, "J", Anything, "dZ"},
	{Anything, 0, Anything, Silent},
};

static Rule K_rules[] =
{
	{Nothing, "K", "N", ""},
	{Anything, "K", Anything, "k"},
	{Anything, 0, Anything, Silent},
};

static Rule L_rules[] =
{
	{Anything, "LO", "C#", "l@U"},
	{"L", "L", Anything, ""},
	{"#:^", "L", "%", "@l"},
	{Anything, "LEAD", Anything, "lid"},
	{Anything, "L", Anything, "l"},
	{Anything, 0, Anything, Silent},
};

static Rule M_rules[] =
{
	{Anything, "MOV", Anything, "muv"},
	{"#", "MM", "#", "m"},
	{Anything, "M", Anything, "m"},
	{Anything, 0, Anything, Silent},
};

static Rule N_rules[] =
{
	{"E", "NG", "+", "ndZ"},
	{Anything, "NG", "R", "Ng"},
	{Anything, "NG", "#", "Ng"},
	{Anything, "NGL", "%", "Ng@l"},
	{Anything, "NG", Anything, "N"},
	{Anything, "NK", Anything, "Nk"},
	{Nothing, "NOW", Nothing, "naU"},
	{"#", "NG", Nothing, "Ng"},
	{Anything, "N", Anything, "n"},
	{Anything, 0, Anything, Silent},
};

static Rule O_rules[] =
{
	{Anything, "OF", Nothing, "@v"},
	{Anything, "OROUGH", Anything, "3@U"},
	{"#:", "OR", Nothing, "3"},
	{"#:", "ORS", Nothing, "3z"},
	{Anything, "OR", Anything, "Or"},
	{Nothing, "ONE", Anything, "wVn"},
	{Anything, "OW", Anything, "@U"},
	{Nothing, "OVER", Anything, "@Uv3"},
	{Anything, "OV", Anything, "Vv"},
	{Anything, "O", "^%", "@U"},
	{Anything, "O", "^EN", "@U"},
	{Anything, "O", "^I#", "@U"},
	{Anything, "OL", "D", "@Ul"},
	{Anything, "OUGHT", Anything, "Ot"},
	{Anything, "OUGH", Anything, "Vf"},
	{Nothing, "OU", Anything, "aU"},
	{"H", "OU", "S#", "aU"},
	{Anything, "OUS", Anything, "@s"},
	{Anything, "OUR", Anything, "Or"},
	{Anything, "OULD", Anything, "Ud"},
	{"^", "OU", "^L", "V"},
	{Anything, "OUP", Anything, "up"},
	{Anything, "OU", Anything, "aU"},
	{Anything, "OY", Anything, "oI"},
	{Anything, "OING", Anything, "@UIN"},
	{Anything, "OI", Anything, "oI"},
	{Anything, "OOR", Anything, "Or"},
	{Anything, "OOK", Anything, "Uk"},
	{Anything, "OOD", Anything, "Ud"},
	{Anything, "OO", Anything, "u"},
	{Anything, "O", "E", "@U"},
	{Anything, "O", Nothing, "@U"},
	{Anything, "OA", Anything, "@U"},
	{Nothing, "ONLY", Anything, "@Unli"},
	{Nothing, "ONCE", Anything, "wVns"},
	{Anything, "ON'T", Anything, "@Unt"},
	{"C", "O", "N", "0"},
	{Anything, "O", "NG", "O"},
	{" :^", "O", "N", "V"},
	{"I", "ON", Anything, "@n"},
	{"#:", "ON", Nothing, "@n"},
	{"#^", "ON", Anything, "@n"},
	{Anything, "O", "ST ", "@U"},
	{Anything, "OF", "^", "Of"},
	{Anything, "OTHER", Anything, "VD3"},
	{Anything, "OSS", Nothing, "Os"},
	{"#:^", "OM", Anything, "Vm"},
	{Anything, "O", Anything, "0"},
	{Anything, 0, Anything, Silent},
};

static Rule P_rules[] =
{
	{Anything, "PH", Anything, "f"},
	{Anything, "PEOP", Anything, "pip"},
	{Anything, "POW", Anything, "paU"},
	{Anything, "PUT", Nothing, "pUt"},
	{Anything, "P", Anything, "p"},
	{Anything, 0, Anything, Silent},
};

static Rule Q_rules[] =
{
	{Anything, "QUAR", Anything, "kwOr"},
	{Anything, "QU", Anything, "kw"},
	{Anything, "Q", Anything, "k"},
	{Anything, 0, Anything, Silent},
};

static Rule R_rules[] =
{
	{Nothing, "RE", "^#", "ri"},
	{Anything, "R", Anything, "r"},
	{Anything, 0, Anything, Silent},
};

static Rule S_rules[] =
{
	{Anything, "SH", Anything, "S"},
	{"#", "SION", Anything, "Z@n"},
	{Anything, "SOME", Anything, "sVm"},
	{"#", "SUR", "#", "Z3"},
	{Anything, "SUR", "#", "S3"},
	{"#", "SU", "#", "Zu"},
	{"#", "SSU", "#", "Su"},
	{"#", "SED", Nothing, "zd"},
	{"#", "S", "#", "z"},
	{Anything, "SAID", Anything, "sed"},
	{"^", "SION", Anything, "S@n"},
	{Anything, "S", "S", ""},
	{".", "S", Nothing, "z"},
	{"#:.E", "S", Nothing, "z"},
	{"#:^##", "S", Nothing, "z"},
	{"#:^#", "S", Nothing, "s"},
	{"U", "S", Nothing, "s"},
	{" :#", "S", Nothing, "z"},
	{Nothing, "SCH", Anything, "sk"},
	{Anything, "S", "C+", ""},
	{"#", "SM", Anything, "zm"},
	{"#", "SN", "'", "z@n"},
	{Anything, "S", Anything, "s"},
	{Anything, 0, Anything, Silent},
};

static Rule T_rules[] =
{
	{Nothing, "THE", Nothing, "D@"},
	{Anything, "TO", Nothing, "tu"},
	{Anything, "THAT", Nothing, "D&t"},
	{Nothing, "THIS", Nothing, "DIs"},
	{Nothing, "THEY", Anything, "DeI"},
	{Nothing, "THERE", Anything, "Der"},
	{Anything, "THER", Anything, "D3"},
	{Anything, "THEIR", Anything, "Der"},
	{Nothing, "THAN", Nothing, "D&n"},
	{Nothing, "THEM", Nothing, "Dem"},
	{Anything, "THESE", Nothing, "Diz"},
	{Nothing, "THEN", Anything, "Den"},
	{Anything, "THROUGH", Anything, "Tru"},
	{Anything, "THOSE", Anything, "D@Uz"},
	{Anything, "THOUGH", Nothing, "D@U"},
	{Nothing, "THUS", Anything, "DVs"},
	{Anything, "TH", Anything, "T"},
	{"#:", "TED", Nothing, "tId"},
	{"S", "TI", "#N", "tS"},
	{Anything, "TI", "O", "S"},
	{Anything, "TI", "A", "S"},
	{Anything, "TIEN", Anything, "S@n"},
	{Anything, "TUR", "#", "tS3"},
	{Anything, "TU", "A", "tSu"},
	{Nothing, "TWO", Anything, "tu"},
	{Anything, "T", Anything, "t"},
	{Anything, 0, Anything, Silent},
};

static Rule U_rules[] =
{
	{Nothing, "UN", "I", "jun"},
	{Nothing, "UN", Anything, "Vn"},
	{Nothing, "UPON", Anything, "@pOn"},
	{"T", "UR", "#", "Ur"},
	{"S", "UR", "#", "Ur"},
	{"R", "UR", "#", "Ur"},
	{"D", "UR", "#", "Ur"},
	{"L", "UR", "#", "Ur"},
	{"Z", "UR", "#", "Ur"},
	{"N", "UR", "#", "Ur"},
	{"J", "UR", "#", "Ur"},
	{"TH", "UR", "#", "Ur"},
	{"CH", "UR", "#", "Ur"},
	{"SH", "UR", "#", "Ur"},
	{Anything, "UR", "#", "jUr"},
	{Anything, "UR", Anything, "3"},
	{Anything, "U", "^ ", "V"},
	{Anything, "U", "^^", "V"},
	{Anything, "UY", Anything, "aI"},
	{" G", "U", "#", ""},
	{"G", "U", "%", ""},
	{"G", "U", "#", "w"},
	{"#N", "U", Anything, "ju"},
	{"T", "U", Anything, "u"},
	{"S", "U", Anything, "u"},
	{"R", "U", Anything, "u"},
	{"D", "U", Anything, "u"},
	{"L", "U", Anything, "u"},
	{"Z", "U", Anything, "u"},
	{"N", "U", Anything, "u"},
	{"J", "U", Anything, "u"},
	{"TH", "U", Anything, "u"},
	{"CH", "U", Anything, "u"},
	{"SH", "U", Anything, "u"},
	{Anything, "U", Anything, "ju"},
	{Anything, 0, Anything, Silent},
};

static Rule V_rules[] =
{
	{Anything, "VIEW", Anything, "vju"},
	{Anything, "V", Anything, "v"},
	{Anything, 0, Anything, Silent},
};

static Rule W_rules[] =
{
	{Nothing, "WERE", Anything, "w3"},
	{Anything, "WA", "S", "w0"},
	{Anything, "WA", "T", "w0"},
	{Anything, "WHERE", Anything, "hwer"},
	{Anything, "WHAT", Anything, "hw0t"},
	{Anything, "WHOL", Anything, "h@Ul"},
	{Anything, "WHO", Anything, "hu"},
	{Anything, "WH", Anything, "hw"},
	{Anything, "WAR", Anything, "wOr"},
	{Anything, "WOR", "^", "w3"},
	{Anything, "WR", Anything, "r"},
	{Anything, "W", Anything, "w"},
	{Anything, 0, Anything, Silent},
};

static Rule X_rules[] =
{
	{Anything, "X", Anything, "ks"},
	{Anything, 0, Anything, Silent},
};

static Rule Y_rules[] =
{
	{Anything, "YOUNG", Anything, "jVN"},
	{Nothing, "YOU", Anything, "ju"},
	{Nothing, "YES", Anything, "jes"},
	{Nothing, "Y", Anything, "j"},
	{"#:^", "Y", Nothing, "i"},
	{"#:^", "Y", "I", "i"},
	{" :", "Y", Nothing, "aI"},
	{" :", "Y", "#", "aI"},
	{" :", "Y", "^+:#", "I"},
	{" :", "Y", "^#", "aI"},
	{Anything, "Y", Anything, "I"},
	{Anything, 0, Anything, Silent},
};

static Rule Z_rules[] =
{
	{Anything, "Z", Anything, "z"},
	{Anything, 0, Anything, Silent},
};

static Rule *Rules[] =
{
	punct_rules,
	A_rules, B_rules, C_rules, D_rules, E_rules, F_rules, G_rules,
	H_rules, I_rules, J_rules, K_rules, L_rules, M_rules, N_rules,
	O_rules, P_rules, Q_rules, R_rules, S_rules, T_rules, U_rules,
	V_rules, W_rules, X_rules, Y_rules, Z_rules
};


static const char *Cardinals[] =
{
	"zero", "one", "two", "three", "four", 
	"five", "six", "seven", "eight", "nine", 
	"ten", "eleven", "twelve", "thirteen", "fourteen", 
	"fifteen", "sixteen", "seventeen", "eighteen", "nineteen"
};


static const char *Twenties[] =
{
	"twenty", "thirty", "forty", "fifty",
	"sixty", "seventy", "eighty", "ninety"
};


static const char *Ordinals[] =
{
	"zeroth", "first", "second", "third", "fourth", 
	"fifth", "sixth", "seventh","eighth", "ninth",
	"tenth", "eleventh", "twelfth", "thirteenth", "fourteenth", 
	"fifteenth", "sixteenth", "seventeenth", "eighteenth", "nineteenth"
};


static const char *Ord_twenties[] =
{
	"twentieth", "thirtieth", "fortieth", "fiftieth",
	"sixtieth", "seventieth", "eightieth", "ninetieth"
};


/*
** Translate a number to phonemes.  This version is for CARDINAL numbers.
**       Note: this is recursive.
*/
static int xlate_cardinal(int value, darray *phone)
{
	int nph = 0;

	if (value < 0)
	{
		nph += xlate_string("minus", phone);
		value = (-value);

		if (value < 0)                 /* Overflow!  -32768 */
		{
			nph += xlate_string("a lot", phone);
			return nph;
		}
	}

	if (value >= 1000000000L)
		/* Billions */
	{
		nph += xlate_cardinal(value / 1000000000L, phone);
		nph += xlate_string("billion", phone);
		value = value % 1000000000;

		if (value == 0)
			return nph;                   /* Even billion */

		if (value < 100)
			nph += xlate_string("and", phone);

		/* as in THREE BILLION AND FIVE */
	}

	if (value >= 1000000L)
		/* Millions */
	{
		nph += xlate_cardinal(value / 1000000L, phone);
		nph += xlate_string("million", phone);
		value = value % 1000000L;

		if (value == 0)
			return nph;                   /* Even million */

		if (value < 100)
			nph += xlate_string("and", phone);

		/* as in THREE MILLION AND FIVE */
	}

	/* Thousands 1000..1099 2000..99999 */
	/* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */

	if ((value >= 1000L && value <= 1099L) || value >= 2000L)
	{
		nph += xlate_cardinal(value / 1000L, phone);
		nph += xlate_string("thousand", phone);
		value = value % 1000L;

		if (value == 0)
			return nph;                   /* Even thousand */

		if (value < 100)
			nph += xlate_string("and", phone);

		/* as in THREE THOUSAND AND FIVE */
	}

	if (value >= 100L)
	{
		nph += xlate_string(Cardinals[value / 100], phone);
		nph += xlate_string("hundred", phone);
		value = value % 100;

		if (value == 0)
			return nph;                   /* Even hundred */
	}

	if (value >= 20)
	{
		nph += xlate_string(Twenties[(value - 20) / 10], phone);
		value = value % 10;

		if (value == 0)
			return nph;                   /* Even ten */
	}

	nph += xlate_string(Cardinals[value], phone);

	return nph;
}

#if 0
/*
** Translate a number to phonemes.  This version is for ORDINAL numbers.
**       Note: this is recursive.
*/
static int xlate_ordinal(int value, darray *phone)
{
	int nph = 0;

	if (value < 0)
	{
		nph += xlate_string("minus", phone);
		value = (-value);

		if (value < 0)                 /* Overflow!  -32768 */
		{
			nph += xlate_string("a lot", phone);
			return nph;
		}
	}

	if (value >= 1000000000L)
		/* Billions */
	{
		nph += xlate_cardinal(value / 1000000000L, phone);
		value = value % 1000000000;

		if (value == 0)
		{
			nph += xlate_string("billionth", phone);
			return nph;                  /* Even billion */
		}

		nph += xlate_string("billion", phone);

		if (value < 100)
			nph += xlate_string("and", phone);

		/* as in THREE BILLION AND FIVE */
	}

	if (value >= 1000000L)
		/* Millions */
	{
		nph += xlate_cardinal(value / 1000000L, phone);
		value = value % 1000000L;

		if (value == 0)
		{
			nph += xlate_string("millionth", phone);
			return nph;                  /* Even million */
		}

		nph += xlate_string("million", phone);

		if (value < 100)
			nph += xlate_string("and", phone);

		/* as in THREE MILLION AND FIVE */
	}

	/* Thousands 1000..1099 2000..99999 */
	/* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */

	if ((value >= 1000L && value <= 1099L) || value >= 2000L)
	{
		nph += xlate_cardinal(value / 1000L, phone);
		value = value % 1000L;

		if (value == 0)
		{
			nph += xlate_string("thousandth", phone);
			return nph;                  /* Even thousand */
		}

		nph += xlate_string("thousand", phone);

		if (value < 100)
			nph += xlate_string("and", phone);

		/* as in THREE THOUSAND AND FIVE */
	}

	if (value >= 100L)
	{
		nph += xlate_string(Cardinals[value / 100], phone);
		value = value % 100;

		if (value == 0)
		{
			nph += xlate_string("hundredth", phone);
			return nph;                  /* Even hundred */
		}

		nph += xlate_string("hundred", phone);
	}

	if (value >= 20)
	{
		if ((value % 10) == 0)
		{
			nph += xlate_string(Ord_twenties[(value - 20) / 10], phone);
			return nph;                  /* Even ten */
		}

		nph += xlate_string(Twenties[(value - 20) / 10], phone);

		value = value % 10;
	}

	nph += xlate_string(Ordinals[value], phone);

	return nph;
}
#endif

static int isvowel(int chr)
{
	return (chr == 'A' || chr == 'E' || chr == 'I' ||
		chr == 'O' || chr == 'U');
}

static int isconsonant(int chr)
{
	return (isupper(chr) && !isvowel(chr));
}

static int leftmatch(
	const char *pattern,                    /* first char of pattern to match in text */
	const char *context)                     /* last char of text to be matched */

{
	const char *pat;
	const char *text;
	int count;

	if (*pattern == '\0')
		/* null string matches any context */
	{
		return 1;
	}

	/* point to last character in pattern string */
	count = (int)strlen(pattern);

	pat = pattern + (count - 1);

	text = context;

	for (; count > 0; pat--, count--)
	{
		/* First check for simple text or space */
		if (isalpha(*pat) || *pat == '\'' || *pat == ' ')
		{
			if (*pat != *text)
			{
				return 0;
			}
			else
			{
				text--;
				continue;
			}
		}

		switch (*pat)
		{

		case '#':                   /* One or more vowels */

			if (!isvowel(*text))
				return 0;

			text--;

			while (isvowel(*text))
				text--;

			break;

		case ':':                   /* Zero or more consonants */
			while (isconsonant(*text))
				text--;

			break;

		case '^':                   /* One consonant */
			if (!isconsonant(*text))
				return 0;

			text--;

			break;

		case '.':                   /* B, D, V, G, J, L, M, N, R, W, Z */
			if (*text != 'B' && *text != 'D' && *text != 'V'
				&& *text != 'G' && *text != 'J' && *text != 'L'
				&& *text != 'M' && *text != 'N' && *text != 'R'
				&& *text != 'W' && *text != 'Z')
				return 0;

			text--;

			break;

		case '+':                   /* E, I or Y (front vowel) */
			if (*text != 'E' && *text != 'I' && *text != 'Y')
				return 0;

			text--;

			break;

		case '%':

		default:
			fprintf(stderr, "Bad char in left rule: '%c'\n", *pat);

			return 0;
		}
	}

	return 1;
}

static int rightmatch(
	const char *pattern,                    /* first char of pattern to match in text */
	const char *context)                     /* last char of text to be matched */
{
	const char *pat;
	const char *text;

	if (*pattern == '\0')
		/* null string matches any context */
		return 1;

	pat = pattern;

	text = context;

	for (pat = pattern; *pat != '\0'; pat++)
	{
		/* First check for simple text or space */
		if (isalpha(*pat) || *pat == '\'' || *pat == ' ')
		{
			if (*pat != *text)
			{
				return 0;
			}
			else
			{
				text++;
				continue;
			}
		}

		switch (*pat)
		{

		case '#':                   /* One or more vowels */

			if (!isvowel(*text))
				return 0;

			text++;

			while (isvowel(*text))
				text++;

			break;

		case ':':                   /* Zero or more consonants */
			while (isconsonant(*text))
				text++;

			break;

		case '^':                   /* One consonant */
			if (!isconsonant(*text))
				return 0;

			text++;

			break;

		case '.':                   /* B, D, V, G, J, L, M, N, R, W, Z */
			if (*text != 'B' && *text != 'D' && *text != 'V'
				&& *text != 'G' && *text != 'J' && *text != 'L'
				&& *text != 'M' && *text != 'N' && *text != 'R'
				&& *text != 'W' && *text != 'Z')
				return 0;

			text++;

			break;

		case '+':                   /* E, I or Y (front vowel) */
			if (*text != 'E' && *text != 'I' && *text != 'Y')
				return 0;

			text++;

			break;

		case '%':                   /* ER, E, ES, ED, ING, ELY (a suffix) */
			if (*text == 'E')
			{
				text++;

				if (*text == 'L')
				{
					text++;

					if (*text == 'Y')
					{
						text++;
						break;
					}

					else
					{
						text--;               /* Don't gobble L */
						break;
					}
				}

				else
					if (*text == 'R' || *text == 'S' || *text == 'D')
						text++;

				break;
			}

			else
				if (*text == 'I')
				{
					text++;

					if (*text == 'N')
					{
						text++;

						if (*text == 'G')
						{
							text++;
							break;
						}
					}

					return 0;
				}

				else
					return 0;

		default:
			fprintf(stderr, "Bad char in right rule:'%c'\n", *pat);

			return 0;
		}
	}

	return 1;
}

static void phone_cat(darray *arg, const char *s)
{
	char ch;

	while ((ch = *s++))
		arg->put(ch);
}


static int find_rule(darray *arg, char *word, int index, Rule *rules)
{
	for (;;)                         /* Search for the rule */
	{
		Rule *rule;
		const char *left,
			*match,
			*right,
			*output;
		int remainder;
		rule = rules++;
		match = (*rule)[1];

		if (match == 0)
			/* bad symbol! */
		{
			fprintf(stderr, "Error: Can't find rule for: '%c' in \"%s\"\n",
				word[index], word);
			return index + 1;            /* Skip it! */
		}

		for (remainder = index; *match != '\0'; match++, remainder++)
		{
			if (*match != word[remainder])
				break;
		}

		if (*match != '\0')
			continue;                     /* found missmatch */

		left = (*rule)[0];

		right = (*rule)[2];

		if (!leftmatch(left, &word[index - 1]))
			continue;

		if (!rightmatch(right, &word[remainder]))
			continue;

		output = (*rule)[3];

		phone_cat(arg, output);

		return remainder;
	}
}

static void guess_word(darray *arg, char *word)
{
	int index;                       /* Current position in word */
	int type;                        /* First letter of match part */
	index = 1;                       /* Skip the initial blank */

	do
	{
		if (isupper(word[index]))
			type = word[index] - 'A' + 1;
		else
			type = 0;

		index = find_rule(arg, word, index, Rules[type]);
	}

	while (word[index] != '\0');
}


static int NRL(const char *s, int n, darray *phone)
{
	int old = phone->getSize();
	char *word = (char *) malloc(n + 3); // TODO: may return null
	char *d = word;
	*d++ = ' ';

	while (n-- > 0)
	{
		char ch = *s++;

		if (islower(ch))
			ch = (char)toupper(ch);

		*d++ = ch;
	}

	*d++ = ' '; // kinda unnecessary

	*d = '\0';
	guess_word(phone, word);
	free(word);
	return phone->getSize() - old;
}


static int spell_out(const char *word, int n, darray *phone)
{
	int nph = 0;

	while (n-- > 0)
	{
		nph += xlate_string(ASCII[*word++ & 0x7F], phone);
	}

	return nph;
}

static int suspect_word(const char *s, int n)
{
	int i = 0;
	int seen_lower = 0;
	int seen_upper = 0;
	int seen_vowel = 0;
	int last = 0;

	for (i = 0; i < n; i++)
	{
		char ch = *s++;

		if (i && last != '-' && isupper(ch))
			seen_upper = 1;

		if (islower(ch))
		{
			seen_lower = 1;
			ch = (char)toupper(ch);
		}

		if (ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U' || ch == 'Y')
			seen_vowel = 1;

		last = ch;
	}

	return !seen_vowel || (seen_upper && seen_lower) || !seen_lower;
}

static int xlate_word(const char *word, int n, darray *phone)
{
	int nph = 0;

	if (*word != '[')
	{
		if (suspect_word(word, n))
			return spell_out(word, n, phone);
		else
		{
			nph += NRL(word, n, phone);
		}
	}

	else
	{
		if ((++word)[(--n) - 1] == ']')
			n--;

		while (n-- > 0)
		{
			phone->put(*word++);
			nph++;
		}
	}

	phone->put(' ');

	return nph + 1;
}


int xlate_string(const char *string, darray *phone)
{
	int nph = 0;
	const char *s = string;
	char ch;

	while (isspace(ch = *s))
		s++;

	while (*s)
	{
		ch = *s;
		const char *word = s;

		if (isalpha(ch))
		{
			while (isalpha(ch = *s) || ((ch == '\'' || ch == '-' || ch == '.') && isalpha(s[1])))
			{
				s++;
			}

			if (!ch || isspace(ch) || ispunct(ch) || (isdigit(ch) && !suspect_word(word, (int)(s - word))))
			{
				nph += xlate_word(word, (int)(s - word), phone);
			}
			else
			{
				while (*s && !isspace(*s) && !ispunct(*s))
				{
					ch = *s;
					s++;
				}

				nph += spell_out(word, (int)(s - word), phone);
			}
		}
		else
		{
			if (isdigit(ch) || (ch == '-' && isdigit(s[1])))
			{
				int sign = (ch == '-') ? -1 : 1;
				int value = 0;

				if (sign < 0)
				{
					ch = *++s;
				}

				while (isdigit(ch = *s))
				{
					value = value * 10 + ch - '0';
					s++;
				}

				if (ch == '.' && isdigit(s[1]))
				{
					word = ++s;
					nph += xlate_cardinal(value * sign, phone);
					nph += xlate_string("point", phone);

					while (isdigit(ch = *s))
					{
						s++;
					}

					nph += spell_out(word, (int)(s - word), phone);
				}
				else
				{
					/* check for ordinals, date, time etc. can go in here */
					nph += xlate_cardinal(value * sign, phone);
				}
			}
			else
			{
				if (ch == '[' && strchr(s, ']'))
				{
					const char *thisword = s;

					while (*s && *s++ != ']')
						/* nothing */
						;

					nph += xlate_word(thisword, (int)(s - thisword), phone);
				}
				else
				{
					if (ispunct(ch))
					{
						switch (ch)
						{

						case '!':

						case '?':

						case '.':
							s++;
							phone->put('.');// (' ');
							break;

						case '"':                 /* change pitch ? */

						case ':':

						case '-':

						case ';':

						case ',':

						case '(':

						case ')':
							s++;
							phone->put(' ');
							break;

						case '[':
						{
							const char *e = strchr(s, ']');

							if (e)
							{
								s++;

								while (s < e)
									phone->put(*s++);

								s = e + 1;

								break;
							}
						}
						// fallthrough
						default:
							nph += spell_out(word, 1, phone);
							s++;
							break;
						}
					}
					else
					{
						while (*s && !isspace(*s))
						{
							ch = *s;
							s++;
						}

						nph += spell_out(word, (int)(s - word), phone);
					}
				}
			}

			while (isspace(ch = *s))
				s++;
		}
	}

	return nph;
}
