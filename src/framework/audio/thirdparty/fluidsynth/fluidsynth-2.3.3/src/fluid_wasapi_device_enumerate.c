/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2021  Chris Xiong and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_sys.h"
#include <assert.h>

static char **devs;
static int flag;

static void devenumcb(void *p, const char *s, const char *opt)
{
	int *c = (int*) p;
	printf(" %s\n", opt);
	devs[*c] = malloc(strlen(opt) + 1);
	strcpy(devs[*c], opt);
	++ *c;
}

static void eatlog(int lvl,const char *m, void* d)
{
	flag = lvl;
}

void fluid_wasapi_device_enumerate(void)
{
    static const int sample_rates[] = {8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000, 88200, 96000, 0};
    static const char *sample_formats[3] = {"16bits", "float", "\0"};

    int e, d, s, f, i, devcnt;
    fluid_synth_t *synth;
    fluid_audio_driver_t *adriver;
    fluid_settings_t *settings = new_fluid_settings();
	fluid_settings_setstr(settings, "audio.driver", "wasapi");
	devcnt = fluid_settings_option_count(settings, "audio.wasapi.device");
	devs = calloc(devcnt, sizeof(char*));

	puts("Available audio devices:");
	devcnt = 0;
	fluid_settings_foreach_option(settings, "audio.wasapi.device", &devcnt, devenumcb);

	fluid_set_log_function(FLUID_INFO, eatlog, NULL);
	fluid_set_log_function(FLUID_WARN, eatlog, NULL);
	fluid_set_log_function(FLUID_ERR, eatlog, NULL);

	assert(devcnt == fluid_settings_option_count(settings, "audio.wasapi.device"));
	puts("");
	for (e = 0; e < 2; ++e)
	{
		puts(e ? "Exclusive mode:" : "Shared mode:");
		fluid_settings_setint(settings, "audio.wasapi.exclusive-mode", e);
		for (d = 0; d < devcnt; ++d)
		{
			printf("\t%s\n", devs[d]);
			fluid_settings_setstr(settings, "audio.wasapi.device", devs[d]);
			for (s = 0; sample_rates[s]; ++s)
			{
				fluid_settings_setnum(settings, "synth.sample-rate", sample_rates[s]);
				for (f = 0; sample_formats[f][0]; ++f)
				{
					int n, supported, c;
					fluid_settings_setstr(settings, "audio.sample-format", sample_formats[f]);
					flag = 0;
					synth = new_fluid_synth(settings);
					adriver = new_fluid_audio_driver(settings,synth);
                    supported = adriver != NULL;
                    delete_fluid_audio_driver(adriver);
					delete_fluid_synth(synth);
					n = printf("\t  %dHz, %s ", sample_rates[s], sample_formats[f]);
					for (c = 50 - n; c; --c)
					{
						putchar('.');
					}
					printf(" %s%s\n", supported ? "OK" : "FAILED", flag == FLUID_WARN ? "(W)" : flag == FLUID_INFO ? "(I)" : "\0");
				}
			}
			puts("");
		}
	}
    
    puts("OK    : Mode is natively supported by the audio device.");
    puts("OK(I) : Mode is supported, but resampling may occur deep within WASAPI to satisfy device's needs.");
    puts("FAILED: Mode is not supported.");

	delete_fluid_settings(settings);
	for (i = 0; i < devcnt; ++i)
	{
		free(devs[i]);
	}
	free(devs);
}
