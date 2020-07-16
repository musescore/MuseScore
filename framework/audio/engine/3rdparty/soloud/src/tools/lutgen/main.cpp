/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

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
#include <math.h>

#define VERSION "SoLoud Lookup Table Generator (c)2015 Jari Komppa http://iki.fi/sol/"

#define OUTDIR "../src/core/"

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
		"Copyright (c) 2013-2015 Jari Komppa\n"
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


void gen_bitrev(FILE *f, int bits)
{
	int				length;
	int				cnt;
	int				br_index;
	int				bit;

	length = 1L << bits;

	fprintf(f,"int Soloud_fft_bitrev_%d[%d] = {\n", bits, length);

	int linelen = 0;
	linelen += fprintf(f, "0, ");
	br_index = 0;
	for (cnt = 1; cnt < length; ++cnt)
	{
		/* ++br_index (bit reversed) */
		bit = length >> 1;
		while (((br_index ^= bit) & bit) == 0)
		{
			bit >>= 1;
		}

		linelen += fprintf(f,"%d%s", br_index, cnt < length-1 ? ", ":"");
		if (linelen > 70)
		{
			fprintf(f,"\n");
			linelen = 0;
		}
	}
	fprintf(f,"};\n\n");
}

void gen_triglut(FILE * f, int bits)
{
	int		total_len;

	float *_ptr = 0;
	total_len = (1L << (bits - 1)) - 4;

	fprintf(f, "float Soloud_fft_trig_%d[%d] = {\n", bits, total_len);

	_ptr = new float[total_len];
	memset(_ptr, 0, sizeof(float) * total_len);

	const double	PI = atan(1.0f) * 4;
	for (int level = 3; level < bits; ++level)
	{
		const int		level_len = 1L << (level - 1);
		float * const	level_ptr = const_cast<float *> (_ptr + (int)(1 << (level - 1)) - 4);
		
		const double	mul = PI / (level_len << 1);

		for (int i = 0; i < level_len; ++i)
		{
			level_ptr[i] = (float)cos(i * mul);
		}
	}

	int i;
	int linelen = 0;
	for (i = 0; i < total_len; i++)
	{
		linelen += fprintf(f, "%.18ff%s", _ptr[i], i < total_len-1 ? ", ":"\n");
		if (linelen > 60)
		{
			fprintf(f, "\n");
			linelen = 0;
		}
	} 
	fprintf(f, "};\n\n");
	delete[] _ptr;
}

int main(int parc, char ** pars)
{
	printf(VERSION "\n");

	if (parc < 2 || _stricmp(pars[1], "go") != 0)
	{
		printf("\nThis program will generate lookup table include file.\n"
			"You probably ran this by mistake.\n"
			"Use parameter 'go' to actually do something.\n"
			"\n"
			"Note that output will be " OUTDIR "soloud_fft_lut.cpp.\n"
			"\n");
		return 0;
	}
	FILE * f = fopen(OUTDIR "soloud_fft_lut.cpp", "w");
	fileheader(f);
	gen_bitrev(f, 10);
	gen_bitrev(f, 8);
	gen_triglut(f, 10);
	gen_triglut(f, 8);
	fclose(f);
	return 0;
}
