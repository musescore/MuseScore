#include <iostream>
#include "psid.h"
#include "tedmem.h"
#include "tedplay.h"

using namespace std;


static void printHeader()
{
	printf("TedSid2Dump - TED / SID register write dumper for SoLoud\nCopyright 2015 Jari Komppa\n"
		"Based on tedplay - a (mostly) Commodore 264 family media player\nCopyright 2012 Attila Grosz\n"
		"\n");
}

static void printUsage()
{
	printf("Usage:\n"
		"tedsid2dump filename msecs [-s speed] [-m sid model] [-t tune number] [-i]\n\n"
		"Where:\n"
		"-s 1-6, song speed. 3 = single, 5 = double. Default 3\n"
		"-m model 0:6581 1:8580 2:8580DB 3:6581R1. Default 1\n"
		"-t the number of sub-tune to play. Default 1\n"
		"-i Show information and quit\n"
		"-q Quantize timestamps by 1000 ticks\n\n"
		"Example:\n"
		"tedsid2dump foobar.sid 60000 -s 5 -m 0 -t 1\n\n");		
// s = setplaybackSpeed(3);// 5);// 3);
// m = setModel
// t = ~psidChangeTrack
}

extern void process(int ticks);
FILE * outfile = NULL;
int oldregs[1024] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
int lasttime = 0;
int currtime = 0;
int firstwrite = 1;
int quantize = 0;
int regwrites = 0;

void storeregwrite(int reg, int value)
{
	if (!outfile)
		return;
	if (oldregs[reg] != value)
	{
		oldregs[reg] = value;
		int timedelta = currtime - lasttime;
		// skip start silence
		if (firstwrite)
		{
			timedelta = 0;
		}
		while (timedelta > 0x7fff)
		{
			// If we have a long delay, write NOPs
			unsigned short stimedelta = 0xffff;
			fwrite(&stimedelta, 1, 2, outfile);
			unsigned char a = 100; // nop
			fwrite(&a, 1, 1, outfile);
			a = 0;
			fwrite(&a, 1, 1, outfile);
			timedelta -= 0x7fff;
		}
		if (timedelta > 1000*quantize || firstwrite)
		{
			unsigned short stimedelta = timedelta | 0x8000;
			fwrite(&stimedelta, 1, 2, outfile);
			lasttime = currtime;
		}
		fwrite(&value, 1, 1, outfile);
		unsigned char a = reg;
		fwrite(&a, 1, 1, outfile);
		regwrites++;
		firstwrite = 0;
	}
}

extern int selected_model;

int main(int argc, char *argv[])
{
	printHeader();
    if (argc < 3) {
        printUsage();
        return -1;
    }
	
	char * fn = NULL; // first non- '-' param
	int msec = 0; // second non- '-' param
	int speed = 3;
	int model = -1;
	int tune = 1;
	int info = 0;

	int i;
	for (i = 1; i < argc; i++)
	{
		char *s = argv[i];
		if (s[0] == '-')
		{
			switch (s[1])
			{
			case 's':
			case 'S':
				i++;
				speed = atoi(argv[i]);
				if (speed <= 0)
				{
					printf("Error: invalid speed number.\n");
					return -1;
				}
				break;
			case 'm':
			case 'M':
				i++;
				model = atoi(argv[i]);
				if (model <= 0)
				{
					printf("Error: invalid model number.\n");
					return -1;
				}
				break;
			case 't':
			case 'T':
				i++;
				tune = atoi(argv[i]);
				if (tune <= 0)
				{
					printf("Error: invalid tune number.\n");
					return -1;
				}
				break;
			case 'i':
			case 'I':
				info = 1;			
				break;
			case 'q':
			case 'Q':
				quantize = 1;
				break;
			}
		}
		else
		{
			if (fn == NULL)
			{
				fn = s;
				FILE * f = fopen(s, "rb");
				if (!f)
				{
					printf("Error: %s not found. Run without parameters for help.\n", s);
					return -1;
				}
				fclose(f);
			}
			else
			{
				msec = atoi(s);
				if (msec <= 0)
				{
					printf("Error: Milliseconds <= 0. Run without parameters for help.\n");
					return -1;
				}
			}
		}
	}

	if (fn == NULL)
	{
		printf("Error: No filename given. Run without parameters for help.\n");
		return -1;
	}

	char outfilename[2048];
	sprintf(outfilename, "%s.dump", fn);

	int outputMilliseconds = msec;

	machineInit();
	int retval = tedplayMain(fn, model);
	if (0 == retval) 
	{
		printPsidInfo(getPsidHeader());
		if (info)
		{
			return 0;
		}
		tedPlaySetSpeed(speed);
		int i;
		for (i = 1; i < tune; i++)
			psidChangeTrack(1);
		
		outfile = fopen(outfilename, "wb");
		fputc('D', outfile);
		fputc('u', outfile);
		fputc('m', outfile);
		fputc('p', outfile);
		fputc(0, outfile);
		fputc(selected_model, outfile);
		fputc(0, outfile);
		fputc(0, outfile);

		for (i = 0; i < outputMilliseconds; i++)
		{
			if (i % 1000 == 0 || i == outputMilliseconds-1)
			printf("\rRendering %02d:%02d (%3.1f%%)", (i+1) / (60 * 1000), ((i+1) / 1000) % 60, ((i+1)*100.0f)/outputMilliseconds);
			process(TED_SOUND_CLOCK / 1000); // 1 ms
		}
		fclose(outfile);
		printf("\n%d regwrites written to %s\n", regwrites, outfilename);
		printf("\nAll done.\n");
		tedplayClose();
	}

    return retval;
}
