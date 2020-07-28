/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "fluid_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fluidsynth_priv.h"

#if !defined(WIN32) && !defined(MACINTOSH)
#define _GNU_SOURCE
#include <getopt.h>
#endif

#if defined(WIN32)
#include <windows.h>
#endif

#include "fluidsynth.h"

#if defined(WIN32) && !defined(MINGW32)
#include "config_win32.h"
#endif

#ifdef HAVE_SIGNAL_H
#include "signal.h"
#endif

#include "fluid_lash.h"

#ifndef WITH_MIDI
#define WITH_MIDI 1
#endif

/* default audio fragment count (if none specified) */
#ifdef WIN32
#define DEFAULT_FRAG_COUNT 32
#else
#define DEFAULT_FRAG_COUNT 16
#endif

void print_usage(void);
void print_help(void);
void print_welcome(void);

static fluid_cmd_handler_t* newclient(void* data, char* addr);

/*
 * the globals
 */
fluid_cmd_handler_t* cmd_handler = NULL;
int option_help = 0;		/* set to 1 if "-o help" is specified */

/*
 * support for the getopt function
 */
#if !defined(WIN32) && !defined(MACINTOSH)
#define GETOPT_SUPPORT 1
int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;
#endif


/* process_o_cmd_line_option
 *
 * Purpose:
 * Process a command line option -o setting=value,
 * for example: -o synth.polyhony=16
 */
void process_o_cmd_line_option(fluid_settings_t* settings, char* optarg){
  char* val;
  for (val = optarg; *val != '\0'; val++) {
    if (*val == '=') {
      *val++ = 0;
      break;
    }
  }

  /* did user request list of settings */
  if (strcmp (optarg, "help") == 0)
  {
    option_help = 1;
    return;
  }

  /* At this point:
   * optarg => "synth.polyphony"
   * val => "16"
   */
  switch(fluid_settings_get_type(settings, optarg)){
  case FLUID_NUM_TYPE:
    if (fluid_settings_setnum(settings, optarg, atof(val))){
      break;
    };
  case FLUID_INT_TYPE:
    if (fluid_settings_setint(settings, optarg, atoi(val))){
      break;
    };
  case FLUID_STR_TYPE:
    if (fluid_settings_setstr(settings, optarg, val)){
      break;
    };
  default:
    fprintf (stderr, "Settings argument on command line: Failed to set \"%s\" to \"%s\".\n"
	      "Most likely the parameter \"%s\" does not exist.\n", optarg, val, optarg);
  }
}

static void
print_pretty_int (int i)
{
  if (i == INT_MAX) printf ("MAXINT");
  else if (i == INT_MIN) printf ("MININT");
  else printf ("%d", i);
}

/* fluid_settings_foreach function for displaying option help  "-o help" */
static void
settings_foreach_func (void *data, char *name, int type)
{
  fluid_settings_t *settings = (fluid_settings_t *)data;
  double dmin, dmax, ddef;
  int imin, imax, idef;
  char *defstr;

  switch (type)
  {
  case FLUID_NUM_TYPE:
    fluid_settings_getnum_range (settings, name, &dmin, &dmax);
    ddef = fluid_settings_getnum_default (settings, name);
    printf ("%-24s FLOAT [min=%0.3f, max=%0.3f, def=%0.3f]\n",
	    name, dmin, dmax, ddef);
    break;
  case FLUID_INT_TYPE:
    fluid_settings_getint_range (settings, name, &imin, &imax);
    idef = fluid_settings_getint_default (settings, name);
    printf ("%-24s INT   [min=", name);
    print_pretty_int (imin);
    printf (", max=");
    print_pretty_int (imax);
    printf (", def=");
    print_pretty_int (idef);
    printf ("]\n");
    break;
  case FLUID_STR_TYPE:
    defstr = fluid_settings_getstr_default (settings, name);
    printf ("%-24s STR", name);
    if (defstr) printf ("   [def='%s']\n", defstr);
    else printf ("\n");
    break;
  case FLUID_SET_TYPE:
    printf ("%-24s SET\n", name);
    break;
  }
}


#ifdef HAVE_SIGNAL_H
/*
 * handle_signal
 */
void handle_signal(int sig_num)
{
}
#endif


/*
 * main
 */
int main(int argc, char** argv)
{
  fluid_settings_t* settings;
  int arg1 = 1;
  char buf[512];
  int c, i, fragcount = DEFAULT_FRAG_COUNT;
  int interactive = 1;
  int midi_in = 1;
  fluid_player_t* player = NULL;
  fluid_midi_router_t* router = NULL;
  fluid_midi_driver_t* mdriver = NULL;
  fluid_audio_driver_t* adriver = NULL;
  fluid_synth_t* synth = NULL;
  fluid_server_t* server = NULL;
  char* midi_id = NULL;
  char* midi_driver = NULL;
  char* midi_device = NULL;
  char* config_file = NULL;
  int audio_groups = 0;
  int audio_channels = 0;
  int with_server = 0;
  int dump = 0;
  int connect_lash = 1;
  char *optchars = "a:C:c:df:G:g:hijK:L:lm:no:p:R:r:sVvz:";
#ifdef LASH_ENABLED
  int enabled_lash = 0;		/* set to TRUE if lash gets enabled */
  fluid_lash_args_t *lash_args;

  lash_args = fluid_lash_extract_args (&argc, &argv);
#endif

  settings = new_fluid_settings();

#ifdef GETOPT_SUPPORT	/* pre section of GETOPT supported argument handling */
  opterr = 0;

  while (1) {
    int option_index = 0;

    static struct option long_options[] = {
      {"audio-bufcount", 1, 0, 'c'},
      {"audio-bufsize", 1, 0, 'z'},
      {"audio-channels", 1, 0, 'L'},
      {"audio-driver", 1, 0, 'a'},
      {"audio-groups", 1, 0, 'G'},
      {"chorus", 1, 0, 'C'},
      {"connect-jack-outputs", 0, 0, 'j'},
      {"disable-lash", 0, 0, 'l'},
      {"dump", 0, 0, 'd'},
      {"gain", 1, 0, 'g'},
      {"help", 0, 0, 'h'},
      {"load-config", 1, 0, 'f'},
      {"midi-channels", 1, 0, 'K'},
      {"midi-driver", 1, 0, 'm'},
      {"no-midi-in", 0, 0, 'n'},
      {"no-shell", 0, 0, 'i'},
      {"option", 1, 0, 'o'},
      {"portname", 1, 0, 'p'},
      {"reverb", 1, 0, 'R'},
      {"sample-rate", 1, 0, 'r'},
      {"server", 0, 0, 's'},
      {"verbose", 0, 0, 'v'},
      {"version", 0, 0, 'V'},
      {0, 0, 0, 0}
    };

    c = getopt_long(argc, argv, optchars, long_options, &option_index);
    if (c == -1) {
      break;
    }
#else	/* "pre" section to non getopt argument handling */
  for (i = 1; i < argc; i++) {
    char *optarg;

    /* Skip non switch arguments (assume they are file names) */
    if ((argv[i][0] != '-') || (argv[i][1] == '\0')) break;

    c = argv[i][1];

    optarg = strchr (optchars, c);	/* find the option character in optchars */
    if (optarg && optarg[1] == ':')	/* colon follows if switch argument expected */
    {
      if (++i >= argc)
      {
	printf ("Option -%c requires an argument\n", c);
	print_usage();
	exit(0);
      }
      else
      {
	optarg = argv[i];
	if (optarg[0] == '-')
	{
	  printf ("Expected argument to option -%c found switch instead\n", c);
	  print_usage();
	  exit(0);
	}
      }
    }
    else optarg = "";
#endif

    switch (c) {
#ifdef GETOPT_SUPPORT
    case 0:	/* shouldn't normally happen, a long option's flag is set to NULL */
      printf ("option %s", long_options[option_index].name);
      if (optarg) {
	printf (" with arg %s", optarg);
      }
      printf ("\n");
      break;
#endif
    case 'a':
      fluid_settings_setstr(settings, "audio.driver", optarg);
      break;
    case 'C':
      if ((optarg != NULL) && ((strcmp(optarg, "0") == 0) || (strcmp(optarg, "no") == 0))) {
	fluid_settings_setstr(settings, "synth.chorus.active", "no");
      } else {
	fluid_settings_setstr(settings, "synth.chorus.active", "yes");
      }
      break;
    case 'c':
      fluid_settings_setint(settings, "audio.periods", atoi(optarg));
      break;
    case 'd':
      fluid_settings_setstr(settings, "synth.dump", "yes");
      dump = 1;
      break;
    case 'f':
      config_file = optarg;
      break;
    case 'G':
      audio_groups = atoi(optarg);
      break;
    case 'g':
      fluid_settings_setnum(settings, "synth.gain", atof(optarg));
      break;
    case 'h':
      print_help();
      break;
    case 'i':
      interactive = 0;
      break;
    case 'j':
      fluid_settings_setint(settings, "audio.jack.autoconnect", 1);
      break;
    case 'K':
      fluid_settings_setint(settings, "synth.midi-channels", atoi(optarg));
      break;
    case 'L':
      audio_channels = atoi(optarg);
      fluid_settings_setint(settings, "synth.audio-channels", audio_channels);
      break;
    case 'l':			/* disable LASH */
      connect_lash = 0;
      break;
    case 'm':
      fluid_settings_setstr(settings, "midi.driver", optarg);
      break;
    case 'n':
      midi_in = 0;
      break;
    case 'o':
      process_o_cmd_line_option(settings, optarg);
      break;
    case 'p' :
      fluid_settings_setstr(settings, "midi.portname", optarg);
      break;
    case 'R':
      if ((optarg != NULL) && ((strcmp(optarg, "0") == 0) || (strcmp(optarg, "no") == 0))) {
	fluid_settings_setstr(settings, "synth.reverb.active", "no");
      } else {
	fluid_settings_setstr(settings, "synth.reverb.active", "yes");
      }
      break;
    case 'r':
      fluid_settings_setnum(settings, "synth.sample-rate", atof(optarg));
      break;
    case 's':
      with_server = 1;
      break;
    case 'V':
      printf("FluidSynth %s\n", VERSION);
      exit (0);
      break;
    case 'v':
      fluid_settings_setstr(settings, "synth.verbose", "yes");
      break;
    case 'z':
      fluid_settings_setint(settings, "audio.period-size", atoi(optarg));
      break;
#ifdef GETOPT_SUPPORT
    case '?':
      printf ("Unknown option %c\n", optopt);
      print_usage();
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
      break;
#else			/* Non getopt default case */
    default:
      printf ("Unknown switch '%c'\n", c);
      print_usage();
      exit(0);
      break;
#endif
    }	/* end of switch statement */
  }	/* end of loop */

#ifdef GETOPT_SUPPORT
  arg1 = optind;
#else
  arg1 = i;
#endif

  /* option help requested?  "-o help" */
  if (option_help)
  {
    print_welcome ();
    printf ("FluidSynth settings:\n");
    fluid_settings_foreach (settings, settings, settings_foreach_func);
    exit (0);
  }

#ifdef WIN32
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

#ifdef LASH_ENABLED
  /* connect to the lash server */
  if (connect_lash)
    {
      enabled_lash = fluid_lash_connect (lash_args);
      fluid_settings_setint (settings, "lash.enable", enabled_lash ? 1 : 0);
    }
#endif

  /* The 'groups' setting is only relevant for LADSPA operation
   * If not given, set number groups to number of audio channels, because
   * they are the same (there is nothing between synth output and 'sound card')
   */
  if ((audio_groups == 0) && (audio_channels != 0)) {
      audio_groups = audio_channels;
  }
  fluid_settings_setint(settings, "synth.audio-groups", audio_groups);

  /* create the synthesizer */
  synth = new_fluid_synth(settings);
  if (synth == NULL) {
    fprintf(stderr, "Failed to create the synthesizer\n");
    exit(-1);
  }

  cmd_handler = new_fluid_cmd_handler(synth);
  if (cmd_handler == NULL) {
    fprintf(stderr, "Failed to create the command handler\n");
    goto cleanup;
  }

  /* try to load the user or system configuration */
  if (config_file != NULL) {
    fluid_source(cmd_handler, config_file);
  } else if (fluid_get_userconf(buf, 512) != NULL) {
    fluid_source(cmd_handler, buf);
  } else if (fluid_get_sysconf(buf, 512) != NULL) {
    fluid_source(cmd_handler, buf);
  }

  /* load the soundfonts (check that all non options are SoundFont or MIDI files) */
  for (i = arg1; i < argc; i++) {
    if (fluid_is_soundfont(argv[i]))
    {
      if (fluid_synth_sfload(synth, argv[i], 1) == -1)
	fprintf(stderr, "Failed to load the SoundFont %s\n", argv[i]);
    }
    else if (!fluid_is_midifile(argv[i]))
      fprintf (stderr, "Parameter '%s' not a SoundFont or MIDI file or error occurred identifying it.\n",
	       argv[i]);
  }

#ifdef HAVE_SIGNAL_H
/*   signal(SIGINT, handle_signal); */
#endif

  /* start the synthesis thread */
  adriver = new_fluid_audio_driver(settings, synth);
  if (adriver == NULL) {
    fprintf(stderr, "Failed to create the audio driver\n");
    goto cleanup;
  }


  /* start the midi router and link it to the synth */
#if WITH_MIDI
  if (midi_in) {
    /* In dump mode, text output is generated for events going into and out of the router.
     * The example dump functions are put into the chain before and after the router..
     */

    router = new_fluid_midi_router(
      settings,
      dump ? fluid_midi_dump_postrouter : fluid_synth_handle_midi_event,
      (void*)synth);

    if (router == NULL) {
      fprintf(stderr, "Failed to create the MIDI input router; no MIDI input\n"
	      "will be available. You can access the synthesizer \n"
	      "through the console.\n");
    } else {
      fluid_synth_set_midi_router(synth, router); /* Fixme, needed for command handler */
      mdriver = new_fluid_midi_driver(
	settings,
	dump ? fluid_midi_dump_prerouter : fluid_midi_router_handle_midi_event,
	(void*) router);
      if (mdriver == NULL) {
	fprintf(stderr, "Failed to create the MIDI thread; no MIDI input\n"
		"will be available. You can access the synthesizer \n"
		"through the console.\n");
      }
    }
  }
#endif

  /* play the midi files, if any */
  for (i = arg1; i < argc; i++) {
    if ((argv[i][0] != '-') && fluid_is_midifile(argv[i])) {

      if (player == NULL) {
	player = new_fluid_player(synth);
	if (player == NULL) {
	  fprintf(stderr, "Failed to create the midifile player.\n"
		  "Continuing without a player.\n");
	  break;
	}
      }

      fluid_player_add(player, argv[i]);
    }
  }

  if (player != NULL) {
    fluid_player_play(player);
  }

  /* run the server, if requested */
#if !defined(MACINTOSH) && !defined(WIN32)
  if (with_server) {
    server = new_fluid_server(settings, newclient, synth);
    if (server == NULL) {
      fprintf(stderr, "Failed to create the server.\n"
	     "Continuing without it.\n");
    }
  }
#endif


#ifdef LASH_ENABLED
  if (enabled_lash)
    fluid_lash_create_thread (synth);
#endif

  /* run the shell */
  if (interactive) {
    print_welcome();

    printf ("Type 'help' for information on commands and 'help help' for help topics.\n\n");

    /* In dump mode we set the prompt to "". The UI cannot easily
     * handle lines, which don't end with CR.  Changing the prompt
     * cannot be done through a command, because the current shell
     * does not handle empty arguments.  The ordinary case is dump ==
     * 0.
     */
    fluid_settings_setstr(settings, "shell.prompt", dump ? "" : "> ");
    fluid_usershell(settings, cmd_handler);
  }

 cleanup:

#if !defined(MACINTOSH) && !defined(WIN32)
  if (server != NULL) {
    /* if the user typed 'quit' in the shell, kill the server */
    if (!interactive) {
      fluid_server_join(server);
    }
    delete_fluid_server(server);
  }
#endif

  if (cmd_handler != NULL) {
    delete_fluid_cmd_handler(cmd_handler);
  }

  if (player != NULL) {
    /* if the user typed 'quit' in the shell, stop the player */
    if (interactive) {
      fluid_player_stop(player);
    }
    fluid_player_join(player);
    delete_fluid_player(player);
  }

  if (router) {
#if WITH_MIDI
    if (mdriver) {
      delete_fluid_midi_driver(mdriver);
    }
    delete_fluid_midi_router(router);
#endif
  }

  if (adriver) {
    delete_fluid_audio_driver(adriver);
  }

  if (synth) {
    delete_fluid_synth(synth);
  }

  if (settings) {
    delete_fluid_settings(settings);
  }

  return 0;
}

static fluid_cmd_handler_t* newclient(void* data, char* addr)
{
  fluid_synth_t* synth = (fluid_synth_t*) data;
  return new_fluid_cmd_handler(synth);
}


/*
 * print_usage
 */
void
print_usage()
{
  print_welcome ();
  fprintf(stderr, "Usage: fluidsynth [options] [soundfonts]\n");
  fprintf(stderr, "Try -h for help.\n");
  exit(0);
}

/*
 * print_welcome
 */
void
print_welcome()
{
  printf("FluidSynth version %s\n"
	 "Copyright (C) 2000-2006 Peter Hanappe and others.\n"
	 "Distributed under the LGPL license.\n"
	 "SoundFont(R) is a registered trademark of E-mu Systems, Inc.\n\n",
	 FLUIDSYNTH_VERSION);
}

/*
 * print_help
 */
void
print_help()
{
  print_welcome ();
  printf("Usage: \n");
  printf("  fluidsynth [options] [soundfonts] [midifiles]\n");
  printf("Possible options:\n");
  printf(" -a, --audio-driver=[label]\n"
	 "    The audio driver [alsa,jack,oss,dsound,...]\n");
  printf(" -C, --chorus\n"
	 "    Turn the chorus on or off [0|1|yes|no, default = on]\n");
  printf(" -c, --audio-bufcount=[count]\n"
	 "    Number of audio buffers\n");
  printf(" -d, --dump\n"
	 "    Dump incoming and outgoing MIDI events to stdout\n");
  printf(" -f, --load-config\n"
	 "    Load command configuration file (shell commands)\n");
  printf(" -G, --audio-groups\n"
	 "    Defines the number of LADSPA audio nodes\n");
  printf(" -g, --gain\n"
	 "    Set the master gain [0 < gain < 10, default = 0.2]\n");
  printf(" -h, --help\n"
	 "    Print out this help summary\n");
  printf(" -i, --no-shell\n"
	 "    Don't read commands from the shell [default = yes]\n");
  printf(" -j, --connect-jack-outputs\n"
	 "    Attempt to connect the jack outputs to the physical ports\n");
  printf(" -K, --midi-channels=[num]\n"
	 "    The number of midi channels [default = 16]\n");
  printf(" -L, --audio-channels=[num]\n"
	 "    The number of stereo audio channels [default = 1]\n");
#ifdef LASH_ENABLED
  printf(" -l, --disable-lash\n"
	 "    Don't connect to LASH server\n");
#endif
  printf(" -m, --midi-driver=[label]\n"
	 "    The name of the midi driver to use [oss,alsa,alsa_seq,...]\n");
  printf(" -n, --no-midi-in\n"
	 "    Don't create a midi driver to read MIDI input events [default = yes]\n");
  printf(" -p, --portname=[label]\n"
	 "    Set MIDI port name (alsa_seq, coremidi drivers)\n");
  printf(" -o\n"
	 "    Define a setting, -o name=value (\"-o help\" to dump current values)\n");
  printf(" -R, --reverb\n"
	 "    Turn the reverb on or off [0|1|yes|no, default = on]\n");
  printf(" -r, --sample-rate\n"
	 "    Set the sample rate\n");
  printf(" -s, --server\n"
	 "    Start FluidSynth as a server process\n");
  printf(" -V, --version\n"
	 "    Show version of program\n");
  printf(" -v, --verbose\n"
	 "    Print out verbose messages about midi events\n");
  printf(" -z, --audio-bufsize=[size]\n"
	 "    Size of each audio buffer\n");
  exit(0);
}
