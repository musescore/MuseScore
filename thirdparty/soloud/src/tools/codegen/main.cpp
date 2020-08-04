/*
SoLoud audio engine
Copyright (c) 2013-2020 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <vector>
#include <string>

#define VERSION "SoLoud C-Api Code Generator (c)2013-2020 Jari Komppa http://iki.fi/sol/"

//#define PRINT_FUNCTIONS

#define OUTDIR "../src/c_api/"
#define PYOUTDIR "../scripts/"

using namespace std;

char *gIncludeFile[] =
{
	"../include/soloud.h",
	"../include/soloud_audiosource.h",
	"../include/soloud_bassboostfilter.h",
	"../include/soloud_biquadresonantfilter.h",
	"../include/soloud_bus.h",
//	"../include/soloud_c.h",
	"../include/soloud_dcremovalfilter.h",
	"../include/soloud_echofilter.h",
//	"../include/soloud_error.h",
	"../include/soloud_fader.h",
	"../include/soloud_fft.h",
	"../include/soloud_fftfilter.h",
//	"../include/soloud_file.h",
//	"../include/soloud_file_hack_off.h",
//	"../include/soloud_file_hack_on.h",
	"../include/soloud_filter.h",
	"../include/soloud_flangerfilter.h",
	"../include/soloud_freeverbfilter.h",
//	"../include/soloud_internal.h",
	"../include/soloud_lofifilter.h",
//	"../include/soloud_misc.h",
	"../include/soloud_monotone.h",
	"../include/soloud_noise.h",
	"../include/soloud_openmpt.h",
	"../include/soloud_queue.h",
	"../include/soloud_robotizefilter.h",
	"../include/soloud_sfxr.h",
	"../include/soloud_speech.h",
	"../include/soloud_tedsid.h",
//	"../include/soloud_thread.h",
	"../include/soloud_vic.h",
	"../include/soloud_vizsn.h",
	"../include/soloud_wav.h",
	"../include/soloud_waveshaperfilter.h",
	"../include/soloud_wavstream.h"
};

int gIncludeFileCount = sizeof(gIncludeFile) / sizeof(char*);

struct Method
{
	string mRetType;
	string mFuncName;
	vector<string> mOrigParmCast;
	vector<string> mParmType;
	vector<string> mParmName;
	vector<string> mParmValue;
	vector<int> mRef;
};

struct Enum
{
	string mName;
	string mValue;
};

struct Class
{
	string mName;
	string mParent;
	vector<Method *> mMethod;
	vector<Enum *> mEnum;
};

vector<Class *>gClass;

string time_d("double");
string handle_d("unsigned int");
string bool_d("int");
string result_d("int");
int sourceline = 0;
string latestline = "";

string subs_str(string &aSrc)
{
	if (aSrc == "time") return string("double");
	if (aSrc == "handle") return string("unsigned int");
	if (aSrc == "bool") return string("int");
	if (aSrc == "result") return string("int");
	return aSrc;
}

int is_banned(string aName)
{	
	if (aName.find("Instance") != string::npos) return 1;
	if (aName == "AudioCollider") return 1;
	if (aName == "AudioAttenuator") return 1;
	if (aName == "Filter")  return 1;
	if (aName == "AudioSource") return 1;
	if (aName == "Fader") return 1;
	if (aName == "AlignedFloatBuffer")	return 1;
	if (aName == "TinyAlignedFloatBuffer")	return 1;
	return 0;
}

char * loadfile(const char *aFilename)
{
	FILE * f;
	f = fopen(aFilename, "rb");
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char * buf = new char[len+2];
	buf[len] = '\n';
	buf[len+1] = 0;
	fread(buf,1,len,f);
	fclose(f);
	return buf;
}

int is_whitespace(char c)
{
	if (c == ' ') return 1;
	if (c == '\t') return 1;
//	if (c == '\n') return 1;
	if (c == '\r') return 1;
	return 0;
}

int is_alphanumeric(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c == '_' || c == ':')
		return 1;
	return 0;
}

string token(char * buf, int &ofs)
{
	string s = "";
	
	while (is_whitespace(buf[ofs])) ofs++;

	if (buf[ofs] == '\n')
	{
		sourceline++;
	}

	if (is_alphanumeric(buf[ofs]))
	{
		while (is_alphanumeric(buf[ofs]))
		{
			s += buf[ofs];
			ofs++;
		}
	}
	else
	{
		s += buf[ofs];
		ofs++;
		if ((buf[ofs-1] == '/' && buf[ofs] == '/') ||
			(buf[ofs-1] == '/' && buf[ofs] == '*') ||
			(buf[ofs-1] == '*' && buf[ofs] == '/') ||
			(buf[ofs-1] == '=' && buf[ofs] == '=') ||
			(buf[ofs-1] == '!' && buf[ofs] == '=') ||
			(buf[ofs-1] == '<' && buf[ofs] == '=') ||
			(buf[ofs-1] == '>' && buf[ofs] == '=') ||
			(buf[ofs-1] == '-' && buf[ofs] == '=') ||
			(buf[ofs-1] == '+' && buf[ofs] == '=') ||
			(buf[ofs-1] == '+' && buf[ofs] == '+') ||
			(buf[ofs-1] == '-' && buf[ofs] == '-') ||
			(buf[ofs-1] == '/' && buf[ofs] == '=') ||
			(buf[ofs-1] == '*' && buf[ofs] == '=') ||
			(buf[ofs-1] == '%' && buf[ofs] == '='))
		{
			s += buf[ofs];
			ofs++;
		}		
	}
	latestline += s + " ";	
	return s;
}

#if 0
#define NEXTTOKEN { s = token(b, ofs); printf("%s ", s.c_str()); }
#define EXPECT(x) { string t = token(b, ofs); if (t != x) { printf("\n\nOops, expected \"%s\", not \"%s\", line %d\n", x, t.c_str(), __LINE__); PARSEERROR } }
#else
#define NEXTTOKEN { s = token(b, ofs);  }
#define EXPECT(x) if (token(b, ofs) != x) { PARSEERROR }
#endif
#define PARSEERROR { printf("Parse error near\n---8<---\n%s\n---8<---\nparser line %d, source line ~%d\n", (latestline.length() < 200)? latestline.c_str() : latestline.c_str() + latestline.length() - 100, __LINE__, sourceline); exit(0); }
#define IGNORE token(b, ofs);
#define ALLOW(x) { int tofs = ofs; if (token(b, tofs) == x) { NEXTTOKEN; } }

void parse_params(Method *m, char *b, int &ofs)
{
	string s;
	NEXTTOKEN;
	while (s != ")")
	{
		int ref = 0;
		string pt = "";
		if (s == "const")
		{
			pt += s;
			NEXTTOKEN;
		}
		if (s == "unsigned")
		{
			if (pt != "") pt += " ";
			pt += s;
			NEXTTOKEN;
		}
		
		if (pt != "") pt += " ";
		pt += s;
		NEXTTOKEN;
		if (s == ":")
		{
			EXPECT(":");
			NEXTTOKEN;
			pt = s;
			NEXTTOKEN;
		}

		if (s == "*")
		{
			pt += " *";
			NEXTTOKEN;
		}
		if (s == "&")
		{
			pt += " *";
			ref = 1;
			NEXTTOKEN;
		}
		string pn = s;
		NEXTTOKEN;
		string pv = "";
		if (s == "=")
		{
			NEXTTOKEN;
			while (s != "," && s != ")")
			{
				pv += s;
				NEXTTOKEN;
			}
		}
		if (s == ",") NEXTTOKEN;

		m->mParmName.push_back(pn);
		// Add !! as "cast" to bools to avoid "performance warning" in msvc
		if (pt == "bool")
			m->mOrigParmCast.push_back("!!");
		else
			m->mOrigParmCast.push_back("");
		m->mParmType.push_back(subs_str(pt));
		m->mParmValue.push_back(pv);
		m->mRef.push_back(ref);
	}
}

void parse(const char *aFilename, int aPrintProgress = 0)
{
	printf("Parsing %s..\n", aFilename);
	char *b = loadfile(aFilename);
	int ofs = 0;
	int newline = 0;
	Class *c = NULL;
	string s;
	int omit = 0;
	sourceline = 1;
	latestline = "";
	while (b[ofs])
	{
		NEXTTOKEN;
		if (s == "private" || s == "friend")
		{
			printf("'%s' not allowed - ", s.c_str());
			PARSEERROR;
		}
		if (s == "union")
		{
			// skip unions
			NEXTTOKEN;
			ALLOW("\n");
			// may be "union {", "union name {", "union name newline {"
			if (s != "{") NEXTTOKEN; 
			if (s != "{") NEXTTOKEN;
			if (s != "{") PARSEERROR;
			while (s != "}") NEXTTOKEN;
			while (s != ";") NEXTTOKEN; // may be union {}; or union {} name;
		}
		else
		if (s == "struct")
		{
			// skip helper structs
			NEXTTOKEN;
			ALLOW("\n");
			NEXTTOKEN;
			if (s == "{")
			{
				while (s != "}") NEXTTOKEN;
				EXPECT(";");
			}
			else
			if (s == ";")
			{
				// okay
			}
			else
			{
				PARSEERROR;
			}
		}
		else
		if (s == "}")
		{
			ALLOW(";");
			if (c)
			{
				if (c->mName == "Soloud")
					omit = !omit;
				else
					omit = 0;
			}
		}
		else
		if (s == "#" && newline)
		{
			while (token(b,ofs) != "\n") {}
			newline = 1;
		}
		else
		if (s == "//")
		{
			while (token(b,ofs) != "\n") {}
			newline = 1;
		}
		else
		if (s == "/*")
		{
			while (token(b,ofs) != "*/") {}
			newline = 0;
		}
		else
		if (s == "\"")
		{
			while (token(b,ofs) != "\"") {}
		}
		else
		if (s == "\n")
		{
			newline = 1;
		}
		else
		{
			if (s == "typedef")
			{
				// skip typedefs..
				while (s != ";") NEXTTOKEN;
			}
			else
			if (s == "namespace")
			{
				NEXTTOKEN;
				// Okay, kludge time: let's call thread functions a class, even though they're not, so we can ignore it
				if (s == "Thread" || s == "FFT" || s == "Misc" || s == "FreeverbImpl")
				{
					c = new Class;
					c->mName = "Instance";
					c->mParent = "";
				}
				ALLOW("\n");
				EXPECT("{");
			}
			else
			if (s == "class")
			{
				string classname;
				string parentname = "";
				NEXTTOKEN;
				classname = s;
				ALLOW("\n");
				NEXTTOKEN;
				if (s == ":")
				{
					EXPECT("public");
					NEXTTOKEN;
					parentname = s;
					ALLOW("\n");
					NEXTTOKEN;
				}

				if (s == "{") 
				{
					if (c) gClass.push_back(c);
					c = new Class;
					c->mName = classname;
					c->mParent = parentname;
					omit = 1;						
				}
			}
			else
			{
				if (s == "enum")
				{
					int enumvalue = 0;
					NEXTTOKEN; // skip enum name
					ALLOW("\n");
					EXPECT("{");
					ALLOW("\n");
					NEXTTOKEN;
					while (s != "}")
					{
						if (s == "//")
						{
							while (s != "\n") NEXTTOKEN;
							NEXTTOKEN;
						}
						else
						{
							Enum *e = new Enum;
							e->mName = s;
							NEXTTOKEN;
							if (s == "=")
							{
								NEXTTOKEN;
								e->mValue = s;
								enumvalue = atoi(s.c_str()) + 1;
							}
							else
							{
								if (s == "," || s == "\n")
								{
									char temp[256];
									sprintf(temp, "%d", enumvalue);
									enumvalue++;
									e->mValue = temp;
								}
							}
							ALLOW(",");
							ALLOW("\n");
							NEXTTOKEN;
							if (c)
							{
								c->mEnum.push_back(e);
							}
							else
							{
								PARSEERROR;
							}
						}
					}
					EXPECT(";");
				}
				else
				if (s == "~")
				{
					// non-virtual DTor
					if (!c)
					{
						PARSEERROR;
					}
					else
					{
						EXPECT(c->mName);
					}
					EXPECT("(");
					EXPECT(")");
					EXPECT(";");					
				}
				else
				if (c && s == c->mName)
				{
					// CTor
					Method *m = new Method;
					m->mFuncName = c->mName;
					m->mRetType = c->mName;
					EXPECT("(");
					parse_params(m, b, ofs);
					c->mMethod.push_back(m);
					EXPECT(";");
					
				}
				else
				if (s == "public:")
				{
					omit = !omit;
//					printf("\n%s going omit %d", c->mName.c_str(), omit);
				}
				else
				{
					// possibly function
					string vt1 = s;

					if (s == "volatile")
					{
						vt1 += s;
						NEXTTOKEN;
					}

					if (s == "const")
					{
						NEXTTOKEN;
						vt1 += " " + s;
					}

					if (s == "virtual")
					{
						NEXTTOKEN;
						vt1 = s;
					}

					if (s == "const")
					{
						NEXTTOKEN;
						vt1 += " " + s;
					}

					if (s == "~")
					{
						// virtual dtor
						if (!c)
						{
							PARSEERROR;
						}
						else
						{
							EXPECT(c->mName);
						}
						EXPECT("(");
						EXPECT(")");
						ALLOW("const");
						EXPECT(";");
						continue;
					}


					if (s == "unsigned")
					{
						NEXTTOKEN;
						vt1 += " " + s;
					}
					NEXTTOKEN;
					if (s == "*")
					{
						vt1 += " *";
						NEXTTOKEN;
					}
					if (s == "*")
					{
						vt1 += " *";
						NEXTTOKEN;
					}

					string vt2 = s;
					NEXTTOKEN;

					if (s == "(")
					{
						// function
						Method *m = new Method;
						m->mFuncName = vt2;
						m->mRetType = subs_str(vt1);
						parse_params(m, b, ofs);
						if (!omit)
						{
							if (!c)
							{
								PARSEERROR;
							}
							else
							{
								c->mMethod.push_back(m);
							}
						}
						else
						{
							// Should clean up m or we'll leak it
							// but what's a little memory leakage between friends?
						}
#ifdef PRINT_FUNCTIONS
						printf("%s %s(", m->mRetType.c_str(), m->mFuncName.c_str());
						int i;
						for (i = 0; i < (signed)m->mParmName.size(); i++)
						{
							if (i != 0)
								printf(", ");
							printf("%s %s", m->mParmType[i].c_str(), m->mParmName[i].c_str());
							if (m->mParmValue[i] != "")
								printf("= %s", m->mParmValue[i].c_str());
						}
						printf(")\n");
#endif
						ALLOW("const");
						ALLOW("=");
						ALLOW("0");
						EXPECT(";");
					}
					else
					{
						// alas, not a function.

						// May be an array or list so let's deal with that
						if (s == "[" || s == ",")
						{
							while (s != ";") NEXTTOKEN;
						}

						if (s != ";") PARSEERROR;
					}
				}
			}
			newline = 0;
		}
	}
	gClass.push_back(c);
}

void fileheader(FILE * f)
{
	fprintf(f, 
		"/* **************************************************\n"
		" *  WARNING: this is a generated file. Do not edit. *\n"
		" *  Any edits will be overwritten by the generator. *\n"
		" ************************************************** */\n"
		"\n"
		"/*\n"
		"SoLoud audio engine\n"
		"Copyright (c) 2013-2020 Jari Komppa\n"
		"\n"
		"This software is provided 'as-is', without any express or implied\n"
		"warranty. In no event will the authors be held liable for any damages\n"
		"arising from the use of this software.\n"
		"\n"
		"Permission is granted to anyone to use this software for any purpose,\n"
		"including commercial applications, and to alter it and redistribute it\n"
		"freely, subject to the following restrictions:\n"
		"\n"
		"   1. The origin of this software must not be misrepresented; you must not\n"
		"   claim that you wrote the original software. If you use this software\n"
		"   in a product, an acknowledgment in the product documentation would be\n"
		"   appreciated but is not required.\n"
		"\n"
		"   2. Altered source versions must be plainly marked as such, and must not be\n"
		"   misrepresented as being the original software.\n"
		"\n"
		"   3. This notice may not be removed or altered from any source\n"
		"   distribution.\n"
		"*/\n"
		"\n"
		"/* " VERSION " */\n"
		"\n"
		);
}

void emit_cppstart(FILE * f)
{
	int i;
	for (i = 0; i < gIncludeFileCount; i++)
		fprintf(f, "#include \"%s\"\n", gIncludeFile[i]);

	fprintf(f,		
		"\n"
		"using namespace SoLoud;\n"
		"\n"
		"extern \"C\"\n"
		"{\n\n"
	);
}

void emit_ctor(FILE * f, const char * cl)
{
	fprintf(f,
		"void * %s_create()\n"
		"{\n"
		"  return (void *)new %s;\n"
		"}\n"
		"\n", cl, cl);
}

void emit_dtor(FILE * f, const char * cl)
{
	fprintf(f,
		"void %s_destroy(void * aClassPtr)\n"
		"{\n"
		"  delete (%s *)aClassPtr;\n"
		"}\n"
		"\n", cl, cl);
}

void emit_cppend(FILE * f)
{
	fprintf(f,
		"} // extern \"C\"\n"
		"\n");
}

void emit_func(FILE * f, int aClass, int aMethod)
{
	int i;
	int initfunc = 0;
	Class *c = gClass[aClass];
	Method *m = c->mMethod[aMethod];

	if (c->mName == "Soloud" && m->mFuncName.find("_init") != string::npos)
	{
		// Init function, needs "a bit" of special handling.
		initfunc = 1;
		string fn = OUTDIR "soloud_c_" + m->mFuncName.substr(0, m->mFuncName.find_first_of('_')) + ".cpp";
		f = fopen(fn.c_str(), "w");
		fileheader(f);
		emit_cppstart(f);
	}

	if (initfunc)
	{
		fprintf(f, 
			"%s %s_%s(", 
			m->mRetType.c_str(), 
			c->mName.c_str(), 
			m->mFuncName.c_str());
	}
	else
	{
		fprintf(f, 
			"%s %s_%s(void * aClassPtr", 
			m->mRetType.c_str(), 
			c->mName.c_str(), 
			m->mFuncName.c_str());
	}

	int had_defaults = 0;
	for (i = 0; i < (signed)m->mParmName.size(); i++)
	{
		if (m->mParmValue[i] == "")
		{
			if (!(i == 0 && initfunc))
				fprintf(f, ", ");
			fprintf(f, 
				"%s %s",
				m->mParmType[i].c_str(),
				m->mParmName[i].c_str());
		}
		else
		{
			had_defaults = 1;
		}
	}
	
	if (initfunc)
	{
		fprintf(f, 
			")\n"
			"{\n"
			"\t%s%s(",
			(m->mRetType == "void")?"":"return ",
			m->mFuncName.c_str());
	}
	else
	{
		fprintf(f, 
			")\n"
			"{\n"
			"\t%s * cl = (%s *)aClassPtr;\n"
			"\t%scl->%s(",
			c->mName.c_str(),
			c->mName.c_str(),
			(m->mRetType == "void")?"":"return ",
			m->mFuncName.c_str());
	}

	for (i = 0; i < (signed)m->mParmName.size(); i++)
	{
		if (m->mParmValue[i] == "")
		{
			if (i != 0)
				fprintf(f, ", ");
			if (m->mRef[i])
				fprintf(f, "*");
			fprintf(f, 
				"%s%s",				
				m->mOrigParmCast[i].c_str(),
				m->mParmName[i].c_str());
		}
	}
	fprintf(f,
		");\n"
		"}\n"
		"\n");

	if (had_defaults)
	{
		if (initfunc)
		{
			fprintf(f, 
				"%s %s_%sEx(", 
				m->mRetType.c_str(), 
				c->mName.c_str(), 
				m->mFuncName.c_str());
		}
		else
		{
			fprintf(f, 
				"%s %s_%sEx(void * aClassPtr", 
				m->mRetType.c_str(), 
				c->mName.c_str(), 
				m->mFuncName.c_str());
		}
		for (i =0; i < (signed)m->mParmName.size(); i++)
		{
			if (!(i == 0 && initfunc))
				fprintf(f, ", ");
			fprintf(f, 
				"%s %s",
				m->mParmType[i].c_str(),
				m->mParmName[i].c_str());
		}
		
		if (initfunc)
		{
			fprintf(f, 
				")\n"
				"{\n"
				"\t%s%s(",
				(m->mRetType == "void")?"":"return ",
				m->mFuncName.c_str());
		}
		else
		{
			fprintf(f, 
				")\n"
				"{\n"
				"\t%s * cl = (%s *)aClassPtr;\n"
				"\t%scl->%s(",
				c->mName.c_str(),
				c->mName.c_str(),
				(m->mRetType == "void")?"":"return ",
				m->mFuncName.c_str());
		}

		for (i = 0; i < (signed)m->mParmName.size(); i++)
		{
			if (i != 0)
				fprintf(f, ", ");
			if (m->mRef[i])
				fprintf(f, "*");
			fprintf(f, 
				"%s%s",				
				m->mOrigParmCast[i].c_str(),
				m->mParmName[i].c_str());
		}
		fprintf(f,
			");\n"
			"}\n"
			"\n");
	}

	if (initfunc)
	{
		emit_cppend(f);
		fclose(f);
	}
}


void generate()
{
	FILE * f, *cppf, *deff, *pyff;
	f = fopen("../include/soloud_c.h", "w");
	cppf = fopen(OUTDIR "soloud_c.cpp", "w");
	deff = fopen(OUTDIR "soloud.def", "w");
	pyff = fopen(PYOUTDIR "soloud_codegen.py", "w");
	fileheader(f);
	fileheader(cppf);
	fprintf(pyff, "# " VERSION "\n# Warning: This file is generated. Any manual changes will be overwritten.\n# Data for SoLoud glue code generation\n\n");

	fprintf(deff,
//		"LIBRARY soloud\n"
		"EXPORTS\n");

	emit_cppstart(cppf);

	fprintf(f,
		"#ifndef SOLOUD_C_H_INCLUDED\n"
		"#define SOLOUD_C_H_INCLUDED\n"
		"\n"
		"#ifdef  __cplusplus\n"
		"extern \"C\" {\n"
		"#endif\n");


	int i, j, k;
	int first = 1;
	fprintf(f, 
		"// Collected enumerations\n"
		"enum SOLOUD_ENUMS\n"
		"{\n");
	fprintf(pyff, "# Enumerations\nsoloud_enum = {\n");
	for (i = 0; i < (signed)gClass.size(); i++)
	{
		if (gClass[i]->mName.find("Instance") == string::npos &&
			gClass[i]->mName != "Filter" &&
			gClass[i]->mName != "AudioSource")
		{
			string cn = gClass[i]->mName;
		
			int j;

			for (j = 0; j < (signed)cn.length(); j++)
				cn[j] = toupper(cn[j]);

			for (j = 0; j < (signed)gClass[i]->mEnum.size(); j++)
			{
				if (!first)
				{
					fprintf(f, ",\n");
					fprintf(pyff, ",\n");
				}
				else
				{
					first = 0;
				}
				fprintf(f, "\t%s_%s = %s", cn.c_str(), gClass[i]->mEnum[j]->mName.c_str(), gClass[i]->mEnum[j]->mValue.c_str());
				fprintf(pyff, "'%s_%s': %s", cn.c_str(), gClass[i]->mEnum[j]->mName.c_str(), gClass[i]->mEnum[j]->mValue.c_str());
			}
		}
	}
	fprintf(f,
		"\n"
		"};\n"
		"\n");
	fprintf(pyff, "\n}\n\n");

	fprintf(pyff, "# Handle types\nsoloud_type = [\n");

	fprintf(f, "// Object handle typedefs\n");
	for (i = 0; i < (signed)gClass.size(); i++)
	{
		if (gClass[i]->mName.find("Instance") == string::npos)// &&
			//gClass[i]->mName != "Filter" &&
			//gClass[i]->mName != "AudioSource")
		{
			fprintf(f, "typedef void * %s;\n", gClass[i]->mName.c_str());
			fprintf(pyff, "%s'%s'", (i!=0)?",\n":"", gClass[i]->mName.c_str());
		}
	}
	fprintf(f, "typedef void * File;\n");

	fprintf(pyff, "\n]\n\n");

	fprintf(pyff, "# Functions\n"
		          "# [return type, function name, [[param type, param name], ...], ...]\n"
				  "soloud_func = [\n");

	first = 1;

	for (i = 0; i < (signed)gClass.size(); i++)
	{
		if (!is_banned(gClass[i]->mName))
		{
			fprintf(f,
				"\n"
				"/*\n"
				" * %s\n"
				" */\n",
				gClass[i]->mName.c_str());
			fprintf(f, "void %s_destroy(%s * a%s);\n", gClass[i]->mName.c_str(), gClass[i]->mName.c_str(), gClass[i]->mName.c_str());
			fprintf(deff, "\t%s_destroy\n", gClass[i]->mName.c_str());
			fprintf(pyff, "%s['void', '%s_destroy', [['%s *', 'a%s']]]", first?"":",\n", gClass[i]->mName.c_str(), gClass[i]->mName.c_str(), gClass[i]->mName.c_str());
			first = 0;
			emit_dtor(cppf, gClass[i]->mName.c_str());
			
			for (j = 0; j < (signed)gClass[i]->mMethod.size(); j++)
			{
				if (gClass[i]->mMethod[j]->mFuncName.find("Instance") == string::npos &&
					gClass[i]->mMethod[j]->mFuncName.find("interlace") == string::npos) 
				{
					if (gClass[i]->mName == gClass[i]->mMethod[j]->mRetType)
					{
						// CTor
						fprintf(f, "%s * %s_create();\n", gClass[i]->mName.c_str(), gClass[i]->mName.c_str());
						fprintf(deff, "\t%s_create\n", gClass[i]->mName.c_str());
						fprintf(pyff, ",\n['%s *', '%s_create', [[]]]", gClass[i]->mName.c_str(), gClass[i]->mName.c_str());
						// TODO: ctors with params? none in soloud so far..
						emit_ctor(cppf, gClass[i]->mName.c_str());
					}
					else
					{
						emit_func(cppf, i, j);
						int has_defaults;
						has_defaults = 0;
						fprintf(f, 
							"%s %s_%s(%s * a%s",
							gClass[i]->mMethod[j]->mRetType.c_str(), 
							gClass[i]->mName.c_str(),
							gClass[i]->mMethod[j]->mFuncName.c_str(),
							gClass[i]->mName.c_str(),
							gClass[i]->mName.c_str());
						fprintf(deff, "\t%s_%s\n", gClass[i]->mName.c_str(), gClass[i]->mMethod[j]->mFuncName.c_str());

						fprintf(pyff, ",\n['%s', '%s_%s', [['%s *', 'a%s']",
							gClass[i]->mMethod[j]->mRetType.c_str(), 
							gClass[i]->mName.c_str(),
							gClass[i]->mMethod[j]->mFuncName.c_str(),
							gClass[i]->mName.c_str(),
							gClass[i]->mName.c_str());

						for (k = 0; k < (signed)gClass[i]->mMethod[j]->mParmName.size(); k++)
						{
							if (gClass[i]->mMethod[j]->mParmValue[k] == "")
							{
								if (gClass[i]->mMethod[j]->mParmType[k] != "Soloud *")
								{
									fprintf(f, ", %s %s", 
										gClass[i]->mMethod[j]->mParmType[k].c_str(),
										gClass[i]->mMethod[j]->mParmName[k].c_str());
									fprintf(pyff, ", ['%s', '%s']", 
										gClass[i]->mMethod[j]->mParmType[k].c_str(),
										gClass[i]->mMethod[j]->mParmName[k].c_str());
								}
							}
							else
							{
								has_defaults = 1;
							}
						}
						fprintf(f, ");\n");
						fprintf(pyff, "]]");
						if (has_defaults)
						{
							fprintf(f, 
								"%s %s_%sEx(%s * a%s",
								gClass[i]->mMethod[j]->mRetType.c_str(), 
								gClass[i]->mName.c_str(),
								gClass[i]->mMethod[j]->mFuncName.c_str(),
								gClass[i]->mName.c_str(),
								gClass[i]->mName.c_str());
							fprintf(pyff, 
								",\n['%s', '%s_%sEx', [['%s *', 'a%s']",
								gClass[i]->mMethod[j]->mRetType.c_str(), 
								gClass[i]->mName.c_str(),
								gClass[i]->mMethod[j]->mFuncName.c_str(),
								gClass[i]->mName.c_str(),
								gClass[i]->mName.c_str());
							fprintf(deff, "\t%s_%sEx\n", gClass[i]->mName.c_str(), gClass[i]->mMethod[j]->mFuncName.c_str());
							int had_defaults = 0;
							for (k = 0; k < (signed)gClass[i]->mMethod[j]->mParmName.size(); k++)
							{
								if (gClass[i]->mMethod[j]->mParmType[k] != "Soloud *")
								{
									fprintf(f, ", %s %s", 
										gClass[i]->mMethod[j]->mParmType[k].c_str(),
										gClass[i]->mMethod[j]->mParmName[k].c_str());
									fprintf(pyff, ", ['%s', '%s'", 
										gClass[i]->mMethod[j]->mParmType[k].c_str(),
										gClass[i]->mMethod[j]->mParmName[k].c_str());
									if (gClass[i]->mMethod[j]->mParmValue[k] != "")
									{
										fprintf(f, " /* = %s */", gClass[i]->mMethod[j]->mParmValue[k].c_str());
										fprintf(pyff, ", '%s'", gClass[i]->mMethod[j]->mParmValue[k].c_str());
										had_defaults = 1;
									}
									fprintf(pyff, "]");
								}
							}
							fprintf(f, ");\n");
							fprintf(pyff, "]]");
						}
					}
				}
			}
		}
	}

	fprintf(f,
		"#ifdef  __cplusplus\n"
		"} // extern \"C\"\n"
		"#endif\n"
		"\n"
		"#endif // SOLOUD_C_H_INCLUDED\n"
		"\n");

	fprintf(pyff, "\n]\n\n");

	emit_cppend(cppf);

	fclose(f);
	fclose(cppf);
	fclose(deff);
	fclose(pyff);
}

void generate_cpp()
{
	FILE * f;
	f = fopen("soloud_c.cpp", "w");
	fileheader(f);

	fclose(f);
}

void inherit_stuff()
{
	int i, j, k, l;

	for (i = 0; i < (signed)gClass.size(); i++)
	{
		if (gClass[i]->mParent != "")
		{
			for (j = 0; j < (signed)gClass.size(); j++)
			{
				if (gClass[j]->mName == gClass[i]->mParent)
				{
					// Only handle single level of inheritance for now. If more are needed,
					// this needs to be recursive.
					for (k = 0; k < (signed)gClass[j]->mMethod.size(); k++)
					{
						if (gClass[j]->mMethod[k]->mFuncName != gClass[j]->mName)
						{
							int found = 0;
							for (l = 0; !found && l < (signed)gClass[i]->mMethod.size(); l++)
							{
								if (gClass[i]->mMethod[l]->mFuncName == gClass[j]->mMethod[k]->mFuncName)
								{
									found = 1;
								}
							}

							if (!found)
							{
								gClass[i]->mMethod.push_back(gClass[j]->mMethod[k]);
							}
						}
					}
				}
			}
		}
	}
}

#ifdef _MSC_VER
  #define strcasecmp _stricmp
#endif

int main(int parc, char ** pars)
{
	printf(VERSION "\n");
	
	if (parc < 2 || strcasecmp(pars[1], "go") != 0)
	{
		printf("\nThis program will generate the 'C' api wrapper code.\n"
			   "You probably ran this by mistake.\n"
			   "Use parameter 'go' to actually do something.\n"
			   "\n"			   
			   "Note that output will be ../include/soloud_c.h and " OUTDIR "soloud_c*.cpp.\n"
			   "\n");
		return 0;
	}

	int i;
	for (i = 0; i < gIncludeFileCount; i++)
		parse(gIncludeFile[i]);

	printf("Handling inheritance..\n");
	inherit_stuff();

	printf("Generating header and cpp..\n");
	generate();

	printf("All done.\n");
}


