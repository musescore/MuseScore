/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
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

#if !defined(_WIN32) && !defined(MACINTOSH)
#define _GNU_SOURCE
#endif

#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#define GETOPT_SUPPORT 1
#endif

#ifdef LIBINSTPATCH_SUPPORT
#include <libinstpatch/libinstpatch.h>
#endif

#ifdef HAVE_LASH
#include "fluid_lash.h"
#endif

#ifdef SYSTEMD_SUPPORT
#include <systemd/sd-daemon.h>
#endif

#if SDL2_SUPPORT
#include <SDL.h>
#endif

#if PIPEWIRE_SUPPORT
#include <pipewire/pipewire.h>
#endif

void print_usage(void);
void print_help(fluid_settings_t *settings);
void print_welcome(void);
void print_configure(void);
void fluid_wasapi_device_enumerate(void);

/*
 * the globals
 */
int option_help = 0;		/* set to 1 if "-o help" is specified */


/* Process a command line option -o setting=value, for example: -o synth.polyhony=16 */
int process_o_cmd_line_option(fluid_settings_t *settings, char *optarg)
{
    char *val;
    int hints;
    int ival;

    for(val = optarg; *val != '\0'; val++)
    {
        if(*val == '=')
        {
            *val++ = 0;
            break;
        }
    }

    /* did user request list of settings */
    if(FLUID_STRCMP(optarg, "help") == 0)
    {
        option_help = 1;
        return FLUID_OK;
    }

    if(FLUID_STRCMP(optarg, "") == 0)
    {
        fprintf(stderr, "Invalid -o option (name part is empty)\n");
        return FLUID_FAILED;
    }

    switch(fluid_settings_get_type(settings, optarg))
    {
    case FLUID_NUM_TYPE:
        if(fluid_settings_setnum(settings, optarg, atof(val)) != FLUID_OK)
        {
            fprintf(stderr, "Failed to set floating point parameter '%s'\n", optarg);
            return FLUID_FAILED;
        }

        break;

    case FLUID_INT_TYPE:
        if(fluid_settings_get_hints(settings, optarg, &hints) == FLUID_OK
                && hints & FLUID_HINT_TOGGLED)
        {
            if(FLUID_STRCASECMP(val, "yes") == 0
                    || FLUID_STRCASECMP(val, "true") == 0
                    || FLUID_STRCASECMP(val, "t") == 0)
            {
                ival = 1;
            }
            else
            {
                ival = atoi(val);
            }
        }
        else
        {
            ival = atoi(val);
        }

        if(fluid_settings_setint(settings, optarg, ival) != FLUID_OK)
        {
            fprintf(stderr, "Failed to set integer parameter '%s'\n", optarg);
            return FLUID_FAILED;
        }

        break;

    case FLUID_STR_TYPE:
        if(fluid_settings_setstr(settings, optarg, val) != FLUID_OK)
        {
            fprintf(stderr, "Failed to set string parameter '%s'\n", optarg);
            return FLUID_FAILED;
        }

        break;

    default:
        fprintf(stderr, "Setting parameter '%s' not found\n", optarg);
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

static void
print_pretty_int(int i)
{
    if(i == INT_MAX)
    {
        printf("MAXINT");
    }
    else if(i == INT_MIN)
    {
        printf("MININT");
    }
    else
    {
        printf("%d", i);
    }
}

#ifdef _WIN32
/* Function using win32 api to convert ANSI encoding string to UTF8 encoding string */
static char*
win32_ansi_to_utf8(const char* ansi_null_terminated_string)
{
    LPWSTR u16_buf = NULL;
    char *u8_buf = NULL;
    fluid_return_val_if_fail(ansi_null_terminated_string != NULL, NULL);
    do
    {
        int u16_count, u8_byte_count;
        u16_count = MultiByteToWideChar(CP_ACP, 0, ansi_null_terminated_string, -1, NULL, 0);
        if (u16_count == 0)
        {
            fprintf(stderr, "Failed to convert ANSI string to wide char string\n");
            break;
        }
        u16_buf = malloc(u16_count * sizeof(WCHAR));
        if (u16_buf == NULL)
        {
            fprintf(stderr, "Out of memory\n");
            break;
        }
        u16_count = MultiByteToWideChar(CP_ACP, 0, ansi_null_terminated_string, -1, u16_buf, u16_count);
        u8_byte_count = WideCharToMultiByte(CP_UTF8, 0, u16_buf, u16_count, NULL, 0, NULL, NULL);

        u8_buf = malloc(u8_byte_count);
        if (u8_buf == NULL)
        {
            fprintf(stderr, "Out of memory\n");
            break;
        }
        WideCharToMultiByte(CP_UTF8, 0, u16_buf, u16_count, u8_buf, u8_byte_count, NULL, NULL);
    } while (0);
    free(u16_buf);
    return u8_buf;
}
#endif

typedef struct
{
    int count;            /* Total count of options */
    int curindex;         /* Current index in options */
} OptionBag;

/* Function to display each string option value */
static void
settings_option_foreach_func(void *data, const char *name, const char *option)
{
    OptionBag *bag = data;

    bag->curindex++;

    if(bag->curindex < bag->count)
    {
        printf("'%s',", option);
    }
    else
    {
        printf("'%s'", option);
    }
}

/* fluid_settings_foreach function for displaying option help  "-o help" */
static void
settings_foreach_func(void *data, const char *name, int type)
{
    fluid_settings_t *settings = (fluid_settings_t *)data;
    double dmin, dmax, ddef;
    int imin, imax, idef, hints;
    char *defstr;
    int count;
    OptionBag bag;

    switch(type)
    {
    case FLUID_NUM_TYPE:
        fluid_settings_getnum_range(settings, name, &dmin, &dmax);
        fluid_settings_getnum_default(settings, name, &ddef);
        printf("%-24s FLOAT [min=%0.3f, max=%0.3f, def=%0.3f]\n",
               name, dmin, dmax, ddef);
        break;

    case FLUID_INT_TYPE:
        fluid_settings_getint_range(settings, name, &imin, &imax);
        fluid_settings_getint_default(settings, name, &idef);
        fluid_settings_get_hints(settings, name, &hints);

        if(!(hints & FLUID_HINT_TOGGLED))
        {
            printf("%-24s INT   [min=", name);
            print_pretty_int(imin);
            printf(", max=");
            print_pretty_int(imax);
            printf(", def=");
            print_pretty_int(idef);
            printf("]\n");
        }
        else
        {
            printf("%-24s BOOL  [def=%s]\n", name, idef ? "True" : "False");
        }

        break;

    case FLUID_STR_TYPE:
        printf("%-24s STR", name);

        fluid_settings_getstr_default(settings, name, &defstr);
        count = fluid_settings_option_count(settings, name);

        if(defstr || count > 0)
        {
            if(defstr && count > 0)
            {
                printf("   [def='%s' vals:", defstr);
            }
            else if(defstr)
            {
                printf("   [def='%s'", defstr);
            }
            else
            {
                printf("   [vals:");
            }

            if(count > 0)
            {
                bag.count = count;
                bag.curindex = 0;
                fluid_settings_foreach_option(settings, name, &bag,
                                              settings_option_foreach_func);
            }

            printf("]\n");
        }
        else
        {
            printf("\n");
        }

        break;

    case FLUID_SET_TYPE:
        printf("%-24s SET\n", name);
        break;
    }
}

/* Output options for a setting string to stdout */
static void
show_settings_str_options(fluid_settings_t *settings, char *name)
{
    OptionBag bag;

    bag.count = fluid_settings_option_count(settings, name);
    bag.curindex = 0;
    fluid_settings_foreach_option(settings, name, &bag,
                                  settings_option_foreach_func);
    printf("\n");
}

static void
fast_render_loop(fluid_settings_t *settings, fluid_synth_t *synth, fluid_player_t *player)
{
    fluid_file_renderer_t *renderer;

    renderer = new_fluid_file_renderer(synth);

    if(!renderer)
    {
        return;
    }

    while(fluid_player_get_status(player) == FLUID_PLAYER_PLAYING)
    {
        if(fluid_file_renderer_process_block(renderer) != FLUID_OK)
        {
            break;
        }
    }

    delete_fluid_file_renderer(renderer);
}

/*
 * main
 * Process initialization steps in the following order:

    1)creating the settings.
    2)reading/setting all options in command line.
    3)read configuration file the first time and execute all "set" commands
    4)creating the synth.
    5)loading the soundfonts specified in command line
	  (multiple soundfonts loading is possible).
    6)loading a default soundfont if no soundfont are supplied.
    7)create the router.
    8)create the midi driver connected to the router.
    9)create a player and add it any midifile specified in command line.
	  (multiple midifiles loading is possible).
    10)create a command handler.
    11)reading the entire configuration file for the second time and submit it
       to the command handler before starting the player.
    12)Start the player.
    13)create a tcp shell if any requested.
    14)entering fast rendering loop if requested, otherwise
    15)create the audio driver (i.e synthesis thread) and a synchronous user
       shell if interactive.
 */
int main(int argc, char **argv)
{
    fluid_settings_t *settings;
    int result = -1;
    int arg1 = 1;
    char buf[512];
    int c, i;
    int interactive = 1;
    int quiet = 0;
    int midi_in = 1;
    fluid_player_t *player = NULL;
    fluid_midi_router_t *router = NULL;
    fluid_midi_driver_t *mdriver = NULL;
    fluid_audio_driver_t *adriver = NULL;
    fluid_synth_t *synth = NULL;
    fluid_cmd_handler_t *cmd_handler = NULL;
#ifdef NETWORK_SUPPORT
    fluid_server_t *server = NULL;
    int with_server = 0;
#endif
    char *config_file = NULL;
    int audio_groups = 0;
    int audio_channels = 0;
    int dump = 0;
    int fast_render = 0;
    static const char optchars[] = "a:C:c:dE:f:F:G:g:hijK:L:lm:nO:o:p:QqR:r:sT:Vvz:";
#ifdef HAVE_LASH
    int connect_lash = 1;
    int enabled_lash = 0;		/* set to TRUE if lash gets enabled */
    fluid_lash_args_t *lash_args;

    lash_args = fluid_lash_extract_args(&argc, &argv);
#endif

#if SDL2_SUPPORT
    // Tell SDL that it shouldn't intercept signals, otherwise SIGINT and SIGTERM won't quit fluidsynth
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    if(SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        fprintf(stderr, "Warning: Unable to initialize SDL2 Audio: %s", SDL_GetError());
    }
    else
    {
        atexit(SDL_Quit);
    }
#endif

#if PIPEWIRE_SUPPORT
    pw_init(&argc, &argv);
    atexit(pw_deinit);
#endif

    /* create the settings */
    settings = new_fluid_settings();

    /* reading / setting options from the command line */
#ifdef GETOPT_SUPPORT	/* pre section of GETOPT supported argument handling */
    opterr = 0;

    while(1)
    {
        int option_index = 0;

        static struct option long_options[] =
        {
            {"audio-bufcount", 1, 0, 'c'},
            {"audio-bufsize", 1, 0, 'z'},
            {"audio-channels", 1, 0, 'L'},
            {"audio-driver", 1, 0, 'a'},
            {"audio-file-endian", 1, 0, 'E'},
            {"audio-file-format", 1, 0, 'O'},
            {"audio-file-type", 1, 0, 'T'},
            {"audio-groups", 1, 0, 'G'},
            {"chorus", 1, 0, 'C'},
            {"connect-jack-outputs", 0, 0, 'j'},
            {"disable-lash", 0, 0, 'l'},
            {"dump", 0, 0, 'd'},
            {"fast-render", 1, 0, 'F'},
            {"gain", 1, 0, 'g'},
            {"help", 0, 0, 'h'},
            {"load-config", 1, 0, 'f'},
            {"midi-channels", 1, 0, 'K'},
            {"midi-driver", 1, 0, 'm'},
            {"no-midi-in", 0, 0, 'n'},
            {"no-shell", 0, 0, 'i'},
            {"option", 1, 0, 'o'},
            {"portname", 1, 0, 'p'},
            {"query-audio-devices", 0, 0, 'Q'},
            {"quiet", 0, 0, 'q'},
            {"reverb", 1, 0, 'R'},
            {"sample-rate", 1, 0, 'r'},
            {"server", 0, 0, 's'},
            {"verbose", 0, 0, 'v'},
            {"version", 0, 0, 'V'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, optchars, long_options, &option_index);

        if(c == -1)
        {
            break;
        }

#else	/* "pre" section to non getopt argument handling */

    for(i = 1; i < argc; i++)
    {
        char *optarg;

        /* Skip non switch arguments (assume they are file names) */
        if((argv[i][0] != '-') || (argv[i][1] == '\0'))
        {
            break;
        }

        c = argv[i][1];

        optarg = strchr(optchars, c);	/* find the option character in optchars */

        if(optarg && optarg[1] == ':')	/* colon follows if switch argument expected */
        {
            if(++i >= argc)
            {
                printf("Option -%c requires an argument\n", c);
                print_usage();
                goto cleanup;
            }
            else
            {
                optarg = argv[i];

                if(optarg[0] == '-')
                {
                    printf("Expected argument to option -%c found switch instead\n", c);
                    print_usage();
                    goto cleanup;
                }
            }
        }
        else
        {
            optarg = "";
        }

#endif

        switch(c)
        {
#ifdef GETOPT_SUPPORT

        case 0:	/* shouldn't normally happen, a long option's flag is set to NULL */
            printf("option %s", long_options[option_index].name);

            if(optarg)
            {
                printf(" with arg %s", optarg);
            }

            printf("\n");
            break;
#endif

        case 'a':
            if(FLUID_STRCMP(optarg, "help") == 0)
            {
                printf("-a options (audio driver):\n   ");
                show_settings_str_options(settings, "audio.driver");
                result = 0;
                goto cleanup;
            }
            else
            {
                if(fluid_settings_setstr(settings, "audio.driver", optarg) != FLUID_OK)
                {
                    goto cleanup;
                }
            }

            break;

        case 'C':
            if((optarg != NULL) && ((FLUID_STRCMP(optarg, "0") == 0) || (FLUID_STRCMP(optarg, "no") == 0)))
            {
                fluid_settings_setint(settings, "synth.chorus.active", FALSE);
            }
            else
            {
                fluid_settings_setint(settings, "synth.chorus.active", TRUE);
            }

            break;

        case 'c':
            if(fluid_settings_setint(settings, "audio.periods", atoi(optarg)) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 'd':
            dump = 1;
            break;

        case 'E':
            if(FLUID_STRCMP(optarg, "help") == 0)
            {
                printf("-E options (audio file byte order):\n   ");
                show_settings_str_options(settings, "audio.file.endian");

#if LIBSNDFILE_SUPPORT
                printf("\nauto: Use audio file format's default endian byte order\n"
                       "cpu: Use CPU native byte order\n");
#else
                printf("\nNOTE: No libsndfile support!\n"
                       "cpu: Use CPU native byte order\n");
#endif
                result = 0;
                goto cleanup;
            }
            else
            {
                if(fluid_settings_setstr(settings, "audio.file.endian", optarg) != FLUID_OK)
                {
                    goto cleanup;
                }
            }

            break;

        case 'f':
            config_file = optarg;
            break;

        case 'F':
            if(fluid_settings_setstr(settings, "audio.file.name", optarg) != FLUID_OK)
            {
                goto cleanup;
            }
            fast_render = 1;
            break;

        case 'G':
            audio_groups = atoi(optarg);
            break;

        case 'g':
            if(fluid_settings_setnum(settings, "synth.gain", atof(optarg)) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 'h':
            print_welcome();
            print_help(settings);
            result = 0;
            goto cleanup;
            break;

        case 'i':
            interactive = 0;
            break;

        case 'j':
#if JACK_SUPPORT
            fluid_settings_setint(settings, "audio.jack.autoconnect", 1);
#endif
            fluid_settings_setint(settings, "midi.autoconnect", 1);
            break;

        case 'K':
            if(fluid_settings_setint(settings, "synth.midi-channels", atoi(optarg)) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 'L':
            audio_channels = atoi(optarg);
            if(fluid_settings_setint(settings, "synth.audio-channels", audio_channels) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 'l':			/* disable LASH */
#ifdef HAVE_LASH
            connect_lash = 0;
#endif
            break;

        case 'm':
            if(FLUID_STRCMP(optarg, "help") == 0)
            {
                printf("-m options (MIDI driver):\n   ");
                show_settings_str_options(settings, "midi.driver");
                result = 0;
                goto cleanup;
            }
            else
            {
                if(fluid_settings_setstr(settings, "midi.driver", optarg) != FLUID_OK)
                {
                    goto cleanup;
                }
            }

            break;

        case 'n':
            midi_in = 0;
            break;

        case 'O':
            if(FLUID_STRCMP(optarg, "help") == 0)
            {
                printf("-O options (audio file format):\n   ");
                show_settings_str_options(settings, "audio.file.format");

#if LIBSNDFILE_SUPPORT
                printf("\ns8, s16, s24, s32: Signed PCM audio of the given number of bits\n");
                printf("float, double: 32 bit and 64 bit floating point audio\n");
                printf("u8: Unsigned 8 bit audio\n");
#else
                printf("\nNOTE: No libsndfile support!\n");
#endif
                result = 0;
                goto cleanup;
            }
            else
            {
                fluid_settings_setstr(settings, "audio.file.format", optarg);
            }

            break;

        case 'o':
            if(process_o_cmd_line_option(settings, optarg) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 'p' :
            if(fluid_settings_setstr(settings, "midi.portname", optarg) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 'Q':
            print_welcome();
#ifdef WASAPI_SUPPORT
            fluid_wasapi_device_enumerate();
            result = 0;
#else
            fprintf(stderr, "Error: This version of fluidsynth was compiled without WASAPI support. Audio device enumeration is not available.");
            result = 1;
#endif
            goto cleanup;

        case 'q':
            quiet = 1;

#if defined(_WIN32)
            /* Windows logs to stdout by default, so make sure anything
             * lower than PANIC is not printed either */
            fluid_set_log_function(FLUID_ERR, NULL, NULL);
            fluid_set_log_function(FLUID_WARN, NULL, NULL);
            fluid_set_log_function(FLUID_INFO, NULL, NULL);
            fluid_set_log_function(FLUID_DBG, NULL, NULL);
#endif
            break;

        case 'R':
            if((optarg != NULL) && ((FLUID_STRCMP(optarg, "0") == 0) || (FLUID_STRCMP(optarg, "no") == 0)))
            {
                fluid_settings_setint(settings, "synth.reverb.active", FALSE);
            }
            else
            {
                fluid_settings_setint(settings, "synth.reverb.active", TRUE);
            }

            break;

        case 'r':
            if(fluid_settings_setnum(settings, "synth.sample-rate", atof(optarg)) != FLUID_OK)
            {
                goto cleanup;
            }
            break;

        case 's':
#ifdef NETWORK_SUPPORT
            with_server = 1;
#else
            printf("\nNOTE: FluidSynth compiled without network support, unable to start server!\n");
#endif
            break;

        case 'T':
            if(FLUID_STRCMP(optarg, "help") == 0)
            {
                printf("-T options (audio file type):\n   ");
                show_settings_str_options(settings, "audio.file.type");

#if LIBSNDFILE_SUPPORT
                printf("\nauto: Determine type from file name extension, defaults to \"wav\"\n");
#else
                printf("\nNOTE: No libsndfile support!\n");
#endif
                result = 0;
                goto cleanup;
            }
            else
            {
                if(fluid_settings_setstr(settings, "audio.file.type", optarg) != FLUID_OK)
                {
                    goto cleanup;
                }
            }

            break;

        case 'V':
            print_welcome();
            print_configure();
            result = 0;
            goto cleanup;

        case 'v':
            fluid_settings_setint(settings, "synth.verbose", TRUE);
            fluid_set_log_function(FLUID_DBG, fluid_default_log_function, NULL);
            break;

        case 'z':
            if(fluid_settings_setint(settings, "audio.period-size", atoi(optarg)) != FLUID_OK)
            {
                goto cleanup;
            }
            break;
#ifdef GETOPT_SUPPORT

        case '?':
            printf("Unknown option %c\n", optopt);
            print_usage();
            goto cleanup;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            break;
#else			/* Non getopt default case */

        default:
            printf("Unknown switch '%c'\n", c);
            print_usage();
            goto cleanup;
#endif
        }	/* end of switch statement */
    }	/* end of loop */

#ifdef GETOPT_SUPPORT
    arg1 = optind;
#else
    arg1 = i;
#endif

    if (!quiet)
    {
        print_welcome();
    }

    /* option help requested?  "-o help" */
    if(option_help)
    {
        printf("FluidSynth settings:\n");
        fluid_settings_foreach(settings, settings, settings_foreach_func);
        result = 0;
        goto cleanup;
    }

#ifdef _WIN32
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

#ifdef HAVE_LASH

    /* connect to the lash server */
    if(connect_lash)
    {
        enabled_lash = fluid_lash_connect(lash_args);
        fluid_settings_setint(settings, "lash.enable", enabled_lash ? 1 : 0);
    }

#endif

    /* The 'groups' setting is relevant for LADSPA operation and channel mapping
     * in rvoice_mixer.
     * If not given, set number groups to number of audio channels, because
     * they are the same (there is nothing between synth output and 'sound card')
     */
    if((audio_groups == 0) && (audio_channels != 0))
    {
        audio_groups = audio_channels;
    }

    if(audio_groups != 0)
    {
        if(fluid_settings_setint(settings, "synth.audio-groups", audio_groups) != FLUID_OK)
        {
            goto cleanup;
        }
    }

    if(fast_render)
    {
        midi_in = 0;		/* disable MIDI driver creation */
        interactive = 0;	/* disable user shell creation */
#ifdef NETWORK_SUPPORT
        with_server = 0;	/* disable tcp server shell creation */
#endif
        fluid_settings_setstr(settings, "player.timing-source", "sample");
        fluid_settings_setint(settings, "synth.lock-memory", 0);
    }

    if(config_file == NULL)
    {
        config_file = fluid_get_userconf(buf, sizeof(buf));
        if(config_file == NULL || !g_file_test(config_file, G_FILE_TEST_EXISTS))
        {
            config_file = fluid_get_sysconf(buf, sizeof(buf));
        }

        /* if the automatically selected command file does not exist, do not even attempt to open it */
        if(config_file != NULL && !fluid_file_test(config_file, FLUID_FILE_TEST_EXISTS))
        {
            config_file = NULL;
        }
    }

    /* Handle set commands before creating the synth */
    if(config_file != NULL)
    {
        cmd_handler = new_fluid_cmd_handler2(settings, NULL, NULL, NULL);
        if(cmd_handler == NULL)
        {
            fprintf(stderr, "Failed to create the early command handler\n");
            goto cleanup;
        }

        if(fluid_source(cmd_handler, config_file) < 0)
        {
            fprintf(stderr, "Failed to early-execute command configuration file '%s'\n", config_file);
            /* the command file seems broken, don't read it again */
            config_file = NULL;
        }

        delete_fluid_cmd_handler(cmd_handler);
        cmd_handler = NULL;
    }

    /* create the synthesizer */
    synth = new_fluid_synth(settings);

    if(synth == NULL)
    {
        fprintf(stderr, "Failed to create the synthesizer\n");
        goto cleanup;
    }

    /* load the soundfonts (check that all non options are SoundFont or MIDI files) */
    for(i = arg1; i < argc; i++)
    {
        const char *u8_path = argv[i];
#if defined(_WIN32)
        /* try to convert ANSI encoding path to UTF8 encoding path */
        char *u8_buf = win32_ansi_to_utf8(argv[i]);
        if (u8_buf == NULL)
        {
            // error msg. already printed
            goto cleanup;
        }
        u8_path = u8_buf;
#endif
        if(fluid_is_midifile(u8_path))
        {
            continue;
        }

        if(fluid_is_soundfont(u8_path))
        {
            if(fluid_synth_sfload(synth, u8_path, 1) == -1)
            {
                fprintf(stderr, "Failed to load the SoundFont %s\n", argv[i]);
            }
        }
        else
        {
            fprintf(stderr, "Parameter '%s' not a SoundFont or MIDI file or error occurred identifying it.\n", argv[i]);
        }
#if defined(_WIN32)
        free(u8_buf);
#endif
    }

    /* Try to load the default soundfont, if no soundfont specified */
    if(fluid_synth_get_sfont(synth, 0) == NULL)
    {
        char *s;

        if(fluid_settings_dupstr(settings, "synth.default-soundfont", &s) != FLUID_OK)
        {
            s = NULL;
        }

        if((s != NULL) && (s[0] != '\0'))
        {
            fluid_synth_sfload(synth, s, 1);
        }

        FLUID_FREE(s);
    }

    router = new_fluid_midi_router(
                 settings,
                 dump ? fluid_midi_dump_postrouter : fluid_synth_handle_midi_event,
                 (void *)synth);

    if(router == NULL)
    {
        fprintf(stderr, "Failed to create the MIDI input router; no MIDI input\n"
                "will be available. You can access the synthesizer \n"
                "through the console.\n");
    }

    /* start the midi router and link it to the synth */
    if(midi_in && router != NULL)
    {
        /* In dump mode, text output is generated for events going into and out of the router.
         * The example dump functions are put into the chain before and after the router..
         */
        mdriver = new_fluid_midi_driver(
                      settings,
                      dump ? fluid_midi_dump_prerouter : fluid_midi_router_handle_midi_event,
                      (void *) router);

        if(mdriver == NULL)
        {
            fprintf(stderr, "Failed to create the MIDI thread; no MIDI input\n"
                    "will be available. You can access the synthesizer \n"
                    "through the console.\n");
        }
    }

    /* create the player and add any midi files, if requested */
    for(i = arg1; i < argc; i++)
    {
        if((argv[i][0] != '-') && fluid_is_midifile(argv[i]))
        {
            if(player == NULL)
            {
                player = new_fluid_player(synth);

                if(player == NULL)
                {
                    fprintf(stderr, "Failed to create the midifile player.\n"
                            "Continuing without a player.\n");
                    break;
                }

                if(router != NULL)
                {
                    fluid_player_set_playback_callback(player, fluid_midi_router_handle_midi_event, router);
                }
            }

            fluid_player_add(player, argv[i]);
        }
    }

    /* try to load and execute the user or system configuration file */
    cmd_handler = new_fluid_cmd_handler2(settings, synth, router, player);

    if(cmd_handler == NULL)
    {
        fprintf(stderr, "Failed to create the command handler\n");
        goto cleanup;
    }

    if(config_file != NULL && fluid_source(cmd_handler, config_file) < 0)
    {
        fprintf(stderr, "Failed to execute command configuration file '%s'\n", config_file);
    }

    /* start the player. Must be done after executing commands configuration file.
       This allows any existing player commands to be run prior the player is started.
       Example:
       player_tempo_bpm 60 # set a low tempo
       player_loop -1      # loop song forever
    */
    if(player != NULL)
    {
        fluid_player_play(player);
    }

    /* run the server, if requested */
#ifdef NETWORK_SUPPORT

    if(with_server)
    {
        server = new_fluid_server2(settings, synth, router, player);

        if(server == NULL)
        {
            fprintf(stderr, "Failed to create the server.\n"
                    "Continuing without it.\n");
        }

#ifdef SYSTEMD_SUPPORT
        else
        {
            sd_notify(0, "READY=1");
        }

#endif
    }

#endif

#ifdef HAVE_LASH

    if(enabled_lash)
    {
        fluid_lash_create_thread(synth);
    }

#endif

    /* fast rendering audio file, if requested */
    if(fast_render)
    {
        char *filename;

        if(player == NULL)
        {
            fprintf(stderr, "No midi file specified!\n");
            goto cleanup;
        }

        fluid_settings_dupstr(settings, "audio.file.name", &filename);
        if(filename)
        {
            if (!quiet)
            {
                printf("Rendering audio to file '%s'..\n", filename);
            }

            FLUID_FREE(filename);
        }

        fast_render_loop(settings, synth, player);
    }
    else /* start the synthesis thread */
    {
        adriver = new_fluid_audio_driver(settings, synth);

        if(adriver == NULL)
        {
            fprintf(stderr, "Failed to create the audio driver. Giving up.\n");
            goto cleanup;
        }

        /* run the shell */
        if(interactive)
        {
            printf("Type 'help' for help topics.\n\n");

            /* In dump mode we set the prompt to "". The UI cannot easily
            * handle lines, which don't end with CR.  Changing the prompt
            * cannot be done through a command, because the current shell
            * does not handle empty arguments.  The ordinary case is dump ==
            * 0.
            */
            fluid_settings_setstr(settings, "shell.prompt", dump ? "" : "> ");
            fluid_usershell(settings, cmd_handler); /* this is a synchronous shell */
        }
    }

    result = 0;

cleanup:

#ifdef NETWORK_SUPPORT

    if(server != NULL)
    {
        /* if the user typed 'quit' in the shell, kill the server */
        if(!interactive)
        {
            fluid_server_join(server);
        }

#ifdef SYSTEMD_SUPPORT
        sd_notify(0, "STOPPING=1");
#endif
        delete_fluid_server(server);
    }

#endif	/* NETWORK_SUPPORT */

    delete_fluid_cmd_handler(cmd_handler);

    if(player != NULL)
    {
        /* if the user typed 'quit' in the shell, stop the player */
        if(interactive)
        {
            fluid_player_stop(player);
        }

        if(adriver != NULL || !fluid_settings_str_equal(settings, "player.timing-source", "sample"))
        {
            /* if no audio driver and sample timers are used, nothing makes the player advance */
            fluid_player_join(player);
        }
    }

    delete_fluid_audio_driver(adriver);
    delete_fluid_player(player);
    delete_fluid_midi_driver(mdriver);
    delete_fluid_midi_router(router);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return result;
}

/*
 * print_usage
 */
void
print_usage()
{
    fprintf(stderr, "Usage: fluidsynth [options] [soundfonts]\n");
    fprintf(stderr, "Try -h for help.\n");
}

void
print_welcome()
{
    printf("FluidSynth runtime version %s\n"
           "Copyright (C) 2000-2023 Peter Hanappe and others.\n"
           "Distributed under the LGPL license.\n"
           "SoundFont(R) is a registered trademark of Creative Technology Ltd.\n\n",
           fluid_version_str());
}

void print_configure()
{
    puts("FluidSynth executable version " FLUIDSYNTH_VERSION);
    puts("Sample type="
#ifdef WITH_FLOAT
         "float"
#else
         "double"
#endif
        );
}

/*
 * print_help
 */
void
print_help(fluid_settings_t *settings)
{
    char *audio_options;
    char *midi_options;

    audio_options = fluid_settings_option_concat(settings, "audio.driver", NULL);
    midi_options = fluid_settings_option_concat(settings, "midi.driver", NULL);

    printf("Usage: \n");
    printf("  fluidsynth [options] [soundfonts] [midifiles]\n");
#ifndef GETOPT_SUPPORT
    printf("\nNote:"
           "\n  This version of fluidsynth was compiled without getopt support."
           "\n  Thus, the long options are not supported.\n\n");
#endif
    printf("Possible options:\n");
    printf(" -a, --audio-driver=[label]\n"
           "    The name of the audio driver to use.\n"
           "    Valid values: %s\n", audio_options ? audio_options : "ERROR");
    printf(" -c, --audio-bufcount=[count]\n"
           "    Number of audio buffers\n");
    printf(" -C, --chorus\n"
           "    Turn the chorus on or off [0|1|yes|no, default = on]\n");
    printf(" -d, --dump\n"
           "    Dump incoming and outgoing MIDI events to stdout\n");
    printf(" -E, --audio-file-endian\n"
           "    Audio file endian for fast rendering or aufile driver (\"help\" for list)\n");
    printf(" -f, --load-config\n"
           "    Load command configuration file (shell commands)\n");
    printf(" -F, --fast-render=[file]\n"
           "    Render MIDI file to raw audio data and store in [file]\n");
    printf(" -g, --gain\n"
           "    Set the master gain [0 < gain < 10, default = 0.2]\n");
    printf(" -G, --audio-groups\n"
           "    Defines the number of LADSPA audio nodes\n");
    printf(" -h, --help\n"
           "    Print out this help summary\n");
    printf(" -i, --no-shell\n"
           "    Don't read commands from the shell [default = yes]\n");
    printf(" -j, --connect-jack-outputs\n"
           "    Attempt to connect the jack outputs to the physical ports\n");
    printf(" -K, --midi-channels=[num]\n"
           "    The number of midi channels [default = 16]\n");
#ifdef HAVE_LASH
    printf(" -l, --disable-lash\n"
           "    Don't connect to LASH server\n");
#endif
    printf(" -L, --audio-channels=[num]\n"
           "    The number of stereo audio channels [default = 1]\n");
    printf(" -m, --midi-driver=[label]\n"
           "    The name of the midi driver to use.\n"
           "    Valid values: %s\n", midi_options ? midi_options : "ERROR");
    printf(" -n, --no-midi-in\n"
           "    Don't create a midi driver to read MIDI input events [default = yes]\n");
    printf(" -o\n"
           "    Define a setting, -o name=value (\"-o help\" to dump current values)\n");
    printf(" -O, --audio-file-format\n"
           "    Audio file format for fast rendering or aufile driver (\"help\" for list)\n");
    printf(" -p, --portname=[label]\n"
           "    Set MIDI port name (alsa_seq, coremidi drivers)\n");
#ifdef WASAPI_SUPPORT
    printf(" -Q, --query-audio-devices\n"
           "    Probe all available soundcards for supported modes, sample-rates and sample-formats.\n");
#endif
    printf(" -q, --quiet\n"
           "    Do not print welcome message or other informational output\n"
           "    (Windows only: also suppress all log messages lower than PANIC\n");
    printf(" -r, --sample-rate\n"
           "    Set the sample rate\n");
    printf(" -R, --reverb\n"
           "    Turn the reverb on or off [0|1|yes|no, default = on]\n");
    printf(" -s, --server\n"
           "    Start FluidSynth as a server process\n");
    printf(" -T, --audio-file-type\n"
           "    Audio file type for fast rendering or aufile driver (\"help\" for list)\n");
    printf(" -v, --verbose\n"
           "    Print out verbose messages about midi events (synth.verbose=1) as well as other debug messages\n");
    printf(" -V, --version\n"
           "    Show version of program\n");
    printf(" -z, --audio-bufsize=[size]\n"
           "    Size of each audio buffer\n");

    if(audio_options)
    {
        FLUID_FREE(audio_options);
    }

    if(midi_options)
    {
        FLUID_FREE(midi_options);
    }
}
