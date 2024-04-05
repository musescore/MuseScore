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

#include "fluid_cmd.h"
#include "fluid_synth.h"
#include "fluid_settings.h"
#include "fluid_hash.h"
#include "fluid_midi_router.h"
#include "fluid_sfont.h"
#include "fluid_chan.h"

/* FIXME: LADSPA used to need a lot of parameters on a single line. This is not
 * necessary anymore, so the limits below could probably be reduced */
#define MAX_TOKENS 100
#define MAX_COMMAND_LEN 1024	/* max command length accepted by fluid_command() */
#define FLUID_WORKLINELENGTH 1024

#define FLUID_ENTRY_COMMAND(data) fluid_cmd_handler_t* handler=(fluid_cmd_handler_t*)(data)

/* the shell cmd handler struct */
struct _fluid_cmd_handler_t
{
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_midi_router_t *router;
    fluid_player_t *player;
    fluid_cmd_hash_t *commands;

    fluid_midi_router_rule_t *cmd_rule;        /* Rule currently being processed by shell command handler */
    int cmd_rule_type;                         /* Type of the rule (#fluid_midi_router_rule_type) */
};


struct _fluid_shell_t
{
    fluid_settings_t *settings;
    fluid_cmd_handler_t *handler;
    fluid_thread_t *thread;
    fluid_istream_t in;
    fluid_ostream_t out;
};


static fluid_thread_return_t fluid_shell_run(void *data);
static void fluid_shell_init(fluid_shell_t *shell,
                             fluid_settings_t *settings, fluid_cmd_handler_t *handler,
                             fluid_istream_t in, fluid_ostream_t out);
static int fluid_handle_voice_count(void *data, int ac, char **av,
                                    fluid_ostream_t out);

void fluid_shell_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "shell.prompt", "", 0);
    fluid_settings_register_int(settings, "shell.port", 9800, 1, 65535, 0);
}


/** the table of all handled commands */

static const fluid_cmd_t fluid_commands[] =
{
    /* general commands */
    {
        "help", "general", fluid_handle_help,
        "help                       Shows help topics ('help TOPIC' for more info)"
    },
    {
        "quit", "general", fluid_handle_quit,
        "quit                       Quit the synthesizer"
    },
    {
        "source", "general", fluid_handle_source,
        "source filename            Loads a file and parse every line as a command"
    },
    /* event commands */
    {
        "noteon", "event", fluid_handle_noteon,
        "noteon chan key vel        Sends noteon"
    },
    {
        "noteoff", "event", fluid_handle_noteoff,
        "noteoff chan key           Sends noteoff"
    },
    {
        "pitch_bend", "event", fluid_handle_pitch_bend,
        "pitch_bend chan offset     Bends pitch"
    },
    {
        "pitch_bend_range", "event", fluid_handle_pitch_bend_range,
        "pitch_bend_range chn range Sets pitch bend range for the given midi channel"
    },
    {
        "cc", "event", fluid_handle_cc,
        "cc chan ctrl value         Sends control-change message"
    },
    {
        "prog", "event", fluid_handle_prog,
        "prog chan num              Sends program-change message"
    },
    {
        "select", "event", fluid_handle_select,
        "select chan sfont bank prog  Combination of bank-select and program-change"
    },
    {
        "load", "general", fluid_handle_load,
        "load file [reset] [bankofs] Loads SoundFont (reset=0|1, def 1; bankofs=n, def 0)"
    },
    {
        "unload", "general", fluid_handle_unload,
        "unload id [reset]          Unloads SoundFont by ID (reset=0|1, default 1)"
    },
    {
        "reload", "general", fluid_handle_reload,
        "reload id                  Reload the SoundFont with the specified ID"
    },
    {
        "fonts", "general", fluid_handle_fonts,
        "fonts                      Display the list of loaded SoundFonts"
    },
    {
        "inst", "general", fluid_handle_inst,
        "inst font                  Print out the available instruments for the font"
    },
    {
        "channels", "general", fluid_handle_channels,
        "channels [-verbose]        Print out preset of all channels"
    },
    {
        "interp", "general", fluid_handle_interp,
        "interp num                 Choose interpolation method for all channels"
    },
    {
        "interpc", "general", fluid_handle_interpc,
        "interpc chan num           Choose interpolation method for one channel"
    },
    /* polymono commands */
    {
        "basicchannels", "polymono", fluid_handle_basicchannels,
        "basicchannels                         Prints the list of basic channels"
    },
    {
        "resetbasicchannels", "polymono", fluid_handle_resetbasicchannels,
        "resetbasicchannels [chan1 chan2..]    Resets all or some basic channels"
    },
    {
        "setbasicchannels", "polymono", fluid_handle_setbasicchannels,
        "setbasicchannels [chan mode val...]   Sets default, adds basic channels"
    },
    {
        "channelsmode", "polymono", fluid_handle_channelsmode,
        "channelsmode [chan1 chan2..]          Prints channels mode"
    },
    {
        "legatomode", "polymono", fluid_handle_legatomode,
        "legatomode [chan1 chan2..]            Prints channels legato mode"
    },
    {
        "setlegatomode", "polymono", fluid_handle_setlegatomode,
        "setlegatomode chan mode [chan mode..] Sets legato mode"
    },
    {
        "portamentomode", "polymono", fluid_handle_portamentomode,
        "portamentomode [chan1 chan2..]        Prints channels portamento mode"
    },
    {
        "setportamentomode", "polymono", fluid_handle_setportamentomode,
        "setportamentomode chan mode [chan mode..] Sets portamento mode"
    },
    {
        "breathmode", "polymono", fluid_handle_breathmode,
        "breathmode [chan1 chan2..]            Prints channels breath mode"
    },
    {
        "setbreathmode", "polymono", fluid_handle_setbreathmode,
        "setbreathmode chan poly(1/0) mono(1/0) breath_sync(1/0) [..] Sets breath mode"
    },
    /* reverb commands */
    {
        "rev_preset", "reverb", fluid_handle_reverbpreset,
        "rev_preset num              Load preset num into all reverb unit"
    },
    {
        "rev_setroomsize", "reverb", fluid_handle_reverbsetroomsize,
        "rev_setroomsize [group] num Set room size of all or one reverb group to num"
    },
    {
        "rev_setdamp", "reverb", fluid_handle_reverbsetdamp,
        "rev_setdamp [group] num     Set damping of all or one reverb group to num"
    },
    {
        "rev_setwidth", "reverb", fluid_handle_reverbsetwidth,
        "rev_setwidth [group] num    Set width of all or one reverb group to num"
    },
    {
        "rev_setlevel", "reverb", fluid_handle_reverbsetlevel,
        "rev_setlevel [group] num    Set output level of all or one reverb group to num"
    },
    {
        "reverb", "reverb", fluid_handle_reverb,
        "reverb [group] 0|1|on|off   Turn all or one reverb group on or off"
    },
    /* chorus commands */
    {
        "cho_set_nr", "chorus", fluid_handle_chorusnr,
        "cho_set_nr [group] n        Set n delay lines (default 3) in all or one chorus group"
    },
    {
        "cho_set_level", "chorus", fluid_handle_choruslevel,
        "cho_set_level [group] num   Set output level of all or one chorus group to num"
    },
    {
        "cho_set_speed", "chorus", fluid_handle_chorusspeed,
        "cho_set_speed [group] num   Set mod speed of all or one chorus group to num (Hz)"
    },
    {
        "cho_set_depth", "chorus", fluid_handle_chorusdepth,
        "cho_set_depth [group] num   Set modulation depth of all or one chorus group to num (ms)"
    },
    {
        "chorus", "chorus", fluid_handle_chorus,
        "chorus [group] 0|1|on|off   Turn all or one chorus group on or off"
    },
    {
        "gain", "general", fluid_handle_gain,
        "gain value                 Set the master gain (0 < gain < 5)"
    },
    {
        "voice_count", "general", fluid_handle_voice_count,
        "voice_count                Get number of active synthesis voices"
    },
    /* tuning commands */
    {
        "tuning", "tuning", fluid_handle_tuning,
        "tuning name bank prog      Create a tuning with name, bank number, \n"
        "                           and program number (0 <= bank,prog <= 127)"
    },
    {
        "tune", "tuning", fluid_handle_tune,
        "tune bank prog key pitch   Tune a key"
    },
    {
        "settuning", "tuning", fluid_handle_settuning,
        "settuning chan bank prog   Set the tuning for a MIDI channel"
    },
    {
        "resettuning", "tuning", fluid_handle_resettuning,
        "resettuning chan           Restore the default tuning of a MIDI channel"
    },
    {
        "tunings", "tuning", fluid_handle_tunings,
        "tunings                    Print the list of available tunings"
    },
    {
        "dumptuning", "tuning", fluid_handle_dumptuning,
        "dumptuning bank prog       Print the pitch details of the tuning"
    },
    {
        "reset", "general", fluid_handle_reset,
        "reset                      System reset (all notes off, reset controllers)"
    },
    /* settings commands */
    {
        "set", "settings", fluid_handle_set,
        "set name value             Set the value of a setting (must be a real-time setting to take effect immediately)"
    },
    {
        "get", "settings", fluid_handle_get,
        "get name                   Get the value of a setting"
    },
    {
        "info", "settings", fluid_handle_info,
        "info name                  Get information about a setting"
    },
    {
        "settings", "settings", fluid_handle_settings,
        "settings                   Print out all settings"
    },
    {
        "echo", "general", fluid_handle_echo,
        "echo arg                   Print arg"
    },
    /* Sleep command, useful to insert a delay between commands */
    {
        "sleep", "general", fluid_handle_sleep,
        "sleep  duration            sleep duration (in ms)"
    },
    /* LADSPA-related commands */
#ifdef LADSPA
    {
        "ladspa_effect", "ladspa", fluid_handle_ladspa_effect,
        "ladspa_effect              Create a new effect from a LADSPA plugin"
    },
    {
        "ladspa_link", "ladspa", fluid_handle_ladspa_link,
        "ladspa_link                Connect an effect port to a host port or buffer"
    },
    {
        "ladspa_buffer", "ladspa", fluid_handle_ladspa_buffer,
        "ladspa_buffer              Create a LADSPA buffer"
    },
    {
        "ladspa_set", "ladspa", fluid_handle_ladspa_set,
        "ladspa_set                 Set the value of an effect control port"
    },
    {
        "ladspa_check", "ladspa", fluid_handle_ladspa_check,
        "ladspa_check               Check LADSPA configuration"
    },
    {
        "ladspa_start", "ladspa", fluid_handle_ladspa_start,
        "ladspa_start               Start LADSPA effects"
    },
    {
        "ladspa_stop", "ladspa", fluid_handle_ladspa_stop,
        "ladspa_stop                Stop LADSPA effect unit"
    },
    {
        "ladspa_reset", "ladspa", fluid_handle_ladspa_reset,
        "ladspa_reset               Stop and reset LADSPA effects"
    },
#endif
    /* router commands */
    {
        "router_clear", "router", fluid_handle_router_clear,
        "router_clear               Clears all routing rules from the midi router"
    },
    {
        "router_default", "router", fluid_handle_router_default,
        "router_default             Resets the midi router to default state"
    },
    {
        "router_begin", "router", fluid_handle_router_begin,
        "router_begin [note|cc|prog|pbend|cpress|kpress]: Starts a new routing rule"
    },
    {
        "router_chan", "router", fluid_handle_router_chan,
        "router_chan min max mul add      filters and maps midi channels on current rule"
    },
    {
        "router_par1", "router", fluid_handle_router_par1,
        "router_par1 min max mul add      filters and maps parameter 1 (key/ctrl nr)"
    },
    {
        "router_par2", "router", fluid_handle_router_par2,
        "router_par2 min max mul add      filters and maps parameter 2 (vel/cc val)"
    },
    {
        "router_end", "router", fluid_handle_router_end,
        "router_end                 closes and commits the current routing rule"
    },
    /* Midi file player commands */
    {
        "player_start", "player", fluid_handle_player_start,
        "player_start               Start playing from the beginning of current song"
    },
    {
        "player_stop", "player", fluid_handle_player_stop,
        "player_stop                Stop playing (cannot be executed in a user command file)"
    },
    {
        "player_cont", "player", fluid_handle_player_continue,
        "player_cont                Continue playing (cannot be executed in a user command file)"
    },
    {
        "player_seek", "player", fluid_handle_player_seek,
        "player_seek num            Move forward/backward in current song to +/-num ticks"
    },
    {
        "player_next", "player", fluid_handle_player_next_song,
        "player_next                Move to next song (cannot be executed in a user command file)"
    },
    {
        "player_loop", "player", fluid_handle_player_loop,
        "player_loop num            Set loop number to num (-1 = loop forever)"
    },
    {
        "player_tempo_bpm", "player", fluid_handle_player_tempo_bpm,
        "player_tempo_bpm num       Set tempo to num beats per minute"
    },
    {
        "player_tempo_int", "player", fluid_handle_player_tempo_int,
        "player_tempo_int [mul]     Set internal tempo multiplied by mul (default mul=1.0)"
    },
#if WITH_PROFILING
    /* Profiling commands */
    {
        "profile", "profile", fluid_handle_profile,
        "profile                        Prints default parameters used by prof_start"
    },
    {
        "prof_set_notes", "profile",  fluid_handle_prof_set_notes,
        "prof_set_notes nbr [bank prog] Sets notes number generated by prof_start"
    },
    {
        "prof_set_print", "profile",  fluid_handle_prof_set_print,
        "prof_set_print mode            Sets print mode (0:simple, 1:full infos)"
    },
    {
        "prof_start", "profile",      fluid_handle_prof_start,
        "prof_start [n_prof [dur]]      Starts n_prof measures of duration(ms) each"
    }
#endif
};

/**
 * Process a string command.
 *
 * @param handler FluidSynth command handler
 * @param cmd Command string (NOTE: Gets modified by FluidSynth prior to 1.0.8)
 * @param out Output stream to display command response to
 * @return Integer value corresponding to: -1 on command error, 0 on success,
 *   1 if 'cmd' is a comment or is empty and -2 if quit was issued
 *
 * @note FluidSynth 1.0.8 and above no longer modifies the 'cmd' string.
 */
int
fluid_command(fluid_cmd_handler_t *handler, const char *cmd, fluid_ostream_t out)
{
    int result, num_tokens = 0;
    char **tokens = NULL;

    if(cmd[0] == '#' || cmd[0] == '\0')
    {
        return 1;
    }

    if(!fluid_shell_parse_argv(cmd, &num_tokens, &tokens))
    {
        fluid_ostream_printf(out, "Error parsing command\n");
        return FLUID_FAILED;
    }

    result = fluid_cmd_handler_handle(handler, num_tokens, &tokens[0], out);
    fluid_strfreev(tokens);

    return result;
}

/**
 * Create a new FluidSynth command shell.
 *
 * @param settings Setting parameters to use with the shell
 * @param handler Command handler
 * @param in Input stream
 * @param out Output stream
 * @param thread TRUE if shell should be run in a separate thread, FALSE to run
 *   it in the current thread (function blocks until "quit")
 * @return New shell instance or NULL on error
 */
fluid_shell_t *
new_fluid_shell(fluid_settings_t *settings, fluid_cmd_handler_t *handler,
                fluid_istream_t in, fluid_ostream_t out, int thread)
{
    fluid_shell_t *shell = FLUID_NEW(fluid_shell_t);

    if(shell == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return NULL;
    }


    fluid_shell_init(shell, settings, handler, in, out);

    if(thread)
    {
        shell->thread = new_fluid_thread("shell", fluid_shell_run, shell,
                                         0, TRUE);

        if(shell->thread == NULL)
        {
            delete_fluid_shell(shell);
            return NULL;
        }
    }
    else
    {
        shell->thread = NULL;
        fluid_shell_run(shell);
    }

    return shell;
}

static void
fluid_shell_init(fluid_shell_t *shell,
                 fluid_settings_t *settings, fluid_cmd_handler_t *handler,
                 fluid_istream_t in, fluid_ostream_t out)
{
    shell->settings = settings;
    shell->handler = handler;
    shell->in = in;
    shell->out = out;
}

/**
 * Delete a FluidSynth command shell.
 * @param shell Command shell instance
 */
void
delete_fluid_shell(fluid_shell_t *shell)
{
    fluid_return_if_fail(shell != NULL);

    if(shell->thread != NULL)
    {
        delete_fluid_thread(shell->thread);
    }

    FLUID_FREE(shell);
}

static fluid_thread_return_t
fluid_shell_run(void *data)
{
    fluid_shell_t *shell = (fluid_shell_t *)data;
    char workline[FLUID_WORKLINELENGTH];
    char *prompt = NULL;
    int cont = 1;
    int errors = FALSE;
    int n;

    if(shell->settings)
    {
        fluid_settings_dupstr(shell->settings, "shell.prompt", &prompt);    /* ++ alloc prompt */
    }

    /* handle user input */
    while(cont)
    {

        n = fluid_istream_readline(shell->in, shell->out, prompt ? prompt : "", workline, FLUID_WORKLINELENGTH);

        if(n < 0)
        {
            FLUID_LOG(FLUID_PANIC, "An error occurred while reading from stdin.");
            break;
        }

        /* handle the command */
        switch(fluid_command(shell->handler, workline, shell->out))
        {

        case 1: /* empty line or comment */
            break;

        case FLUID_FAILED: /* erroneous command */
            errors = TRUE;

        case FLUID_OK: /* valid command */
            break;

        case -2: /* quit */
            cont = 0;
            break;
        }

        if(n == 0)
        {
            if(shell->settings)
            {
                FLUID_LOG(FLUID_INFO, "Received EOF while reading commands, exiting the shell.");
            }
            break;
        }
    }

    FLUID_FREE(prompt);    /* -- free prompt */

    /* return FLUID_THREAD_RETURN_VALUE on success, something else on failure */
    return errors ? (fluid_thread_return_t)(-1) : FLUID_THREAD_RETURN_VALUE;
}

/**
 * A convenience function to create a shell interfacing to standard input/output
 * console streams.
 *
 * @param settings Settings instance for the shell
 * @param handler Command handler callback
 *
 * The shell is run in the current thread, this function will only
 * return after the \c quit command has been issued.
 */
void
fluid_usershell(fluid_settings_t *settings, fluid_cmd_handler_t *handler)
{
    fluid_shell_t shell;
    fluid_shell_init(&shell, settings, handler, fluid_get_stdin(), fluid_get_stdout());
    fluid_shell_run(&shell);
}

/**
 * Execute shell commands in a file.
 *
 * @param handler Command handler callback
 * @param filename File name
 * @return 0 on success, a negative value on error
 */
int
fluid_source(fluid_cmd_handler_t *handler, const char *filename)
{
    int file;
    fluid_shell_t shell;
    int result;

#ifdef _WIN32
    file = _open(filename, _O_RDONLY);
#else
    file = open(filename, O_RDONLY);
#endif

    if(file < 0)
    {
        return file;
    }

    fluid_shell_init(&shell, NULL, handler, file, fluid_get_stdout());
    result = (fluid_shell_run(&shell) == FLUID_THREAD_RETURN_VALUE) ? 0 : -1;

#ifdef _WIN32
    _close(file);
#else
    close(file);
#endif

    return result;
}

/**
 * Get the user specific FluidSynth command file name.
 *
 * @param buf Caller supplied string buffer to store file name to.
 * @param len Length of \a buf
 * @return Returns \a buf pointer or NULL if no user command file for this system type.
 *
 * On Windows this is currently @c "%USERPROFILE%\fluidsynth.cfg".
 * For anything else (except MACOS9) @c "$HOME/.fluidsynth".
 */
char *
fluid_get_userconf(char *buf, int len)
{
    const char *home = NULL;
    const char *config_file;
#if defined(_WIN32)
    home = getenv("USERPROFILE");
    config_file = "\\fluidsynth.cfg";

#elif !defined(MACOS9)
    home = getenv("HOME");
    config_file = "/.fluidsynth";

#endif

    if(home == NULL)
    {
        return NULL;
    }
    else
    {
        FLUID_SNPRINTF(buf, len, "%s%s", home, config_file);
        return buf;
    }
}

/**
 * Get the system FluidSynth command file name.
 *
 * @param buf Caller supplied string buffer to store file name to.
 * @param len Length of \a buf
 * @return Returns \a buf pointer or NULL if no system command file for this system type.
 *
 * MACOS does not have a system-wide config file currently. Since fluidsynth 2.2.9, the config
 * on Windows is @c "%PROGRAMDATA%\fluidsynth\fluidsynth.cfg". For anything else it
 * returns @c "/etc/fluidsynth.conf".
 */
char *
fluid_get_sysconf(char *buf, int len)
{
#if defined(_WIN32)
    const char* program_data = getenv("ProgramData");
    if(program_data == NULL || program_data[0] == '\0')
    {
        return NULL;
    }

    FLUID_SNPRINTF(buf, len, "%s\\fluidsynth\\fluidsynth.cfg", program_data);
    return buf;
#elif defined(MACOS9)
    return NULL;
#else
    FLUID_SNPRINTF(buf, len, "/etc/fluidsynth.conf");
    return buf;
#endif
}


/*
 *  handlers
 */
int
fluid_handle_noteon(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 3)
    {
        fluid_ostream_printf(out, "noteon: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1]) || !fluid_is_number(av[2]))
    {
        fluid_ostream_printf(out, "noteon: invalid argument\n");
        return FLUID_FAILED;
    }

    return fluid_synth_noteon(handler->synth, atoi(av[0]), atoi(av[1]), atoi(av[2]));
}

int
fluid_handle_noteoff(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 2)
    {
        fluid_ostream_printf(out, "noteoff: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "noteon: invalid argument\n");
        return FLUID_FAILED;
    }

    return fluid_synth_noteoff(handler->synth, atoi(av[0]), atoi(av[1]));
}

int
fluid_handle_pitch_bend(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 2)
    {
        fluid_ostream_printf(out, "pitch_bend: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "pitch_bend: invalid argument\n");
        return FLUID_FAILED;
    }

    return fluid_synth_pitch_bend(handler->synth, atoi(av[0]), atoi(av[1]));
}

int
fluid_handle_pitch_bend_range(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int channum;
    int value;

    if(ac < 2)
    {
        fluid_ostream_printf(out, "pitch_bend_range: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "pitch_bend_range: invalid argument\n");
        return FLUID_FAILED;
    }

    channum = atoi(av[0]);
    value = atoi(av[1]);
    fluid_channel_set_pitch_wheel_sensitivity(handler->synth->channel[channum], value);
    return FLUID_OK;
}

int
fluid_handle_cc(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 3)
    {
        fluid_ostream_printf(out, "cc: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1]) || !fluid_is_number(av[2]))
    {
        fluid_ostream_printf(out, "cc: invalid argument\n");
        return FLUID_FAILED;
    }

    return fluid_synth_cc(handler->synth, atoi(av[0]), atoi(av[1]), atoi(av[2]));
}

int
fluid_handle_prog(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 2)
    {
        fluid_ostream_printf(out, "prog: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "prog: invalid argument\n");
        return FLUID_FAILED;
    }

    return fluid_synth_program_change(handler->synth, atoi(av[0]), atoi(av[1]));
}

int
fluid_handle_select(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int sfont_id;
    int chan;
    int bank;
    int prog;

    if(ac < 4)
    {
        fluid_ostream_printf(out, "preset: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]) || !fluid_is_number(av[1])
            || !fluid_is_number(av[2]) || !fluid_is_number(av[3]))
    {
        fluid_ostream_printf(out, "preset: invalid argument\n");
        return FLUID_FAILED;
    }

    chan = atoi(av[0]);
    sfont_id = atoi(av[1]);
    bank = atoi(av[2]);
    prog = atoi(av[3]);

    if(sfont_id != 0)
    {
        return fluid_synth_program_select(handler->synth, chan, sfont_id, bank, prog);
    }
    else
    {
        if(fluid_synth_bank_select(handler->synth, chan, bank) == FLUID_OK)
        {
            return fluid_synth_program_change(handler->synth, chan, prog);
        }

        return FLUID_FAILED;
    }
}

int
fluid_handle_inst(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int font;
    fluid_sfont_t *sfont;
    fluid_preset_t *preset;
    int offset;

    if(ac < 1)
    {
        fluid_ostream_printf(out, "inst: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "inst: invalid argument\n");
        return FLUID_FAILED;
    }

    font = atoi(av[0]);

    sfont = fluid_synth_get_sfont_by_id(handler->synth, font);
    offset = fluid_synth_get_bank_offset(handler->synth, font);

    if(sfont == NULL)
    {
        fluid_ostream_printf(out, "inst: invalid font number\n");
        return FLUID_FAILED;
    }

    fluid_sfont_iteration_start(sfont);

    while((preset = fluid_sfont_iteration_next(sfont)) != NULL)
    {
        fluid_ostream_printf(out, "%03d-%03d %s\n",
                             fluid_preset_get_banknum(preset) + offset,
                             fluid_preset_get_num(preset),
                             fluid_preset_get_name(preset));
    }

    return FLUID_OK;
}


int
fluid_handle_channels(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_preset_t *preset;
    int verbose = 0;
    int i;

    if(ac > 0 && FLUID_STRCMP(av[0], "-verbose") == 0)
    {
        verbose = 1;
    }

    for(i = 0; i < fluid_synth_count_midi_channels(handler->synth); i++)
    {
        preset = fluid_synth_get_channel_preset(handler->synth, i);

        if(preset == NULL)
        {
            fluid_ostream_printf(out, "chan %d, no preset\n", i);
        }
        else if(!verbose)
        {
            fluid_ostream_printf(out, "chan %d, %s\n", i, fluid_preset_get_name(preset));
        }
        else
        {
            fluid_ostream_printf(out,
                                 "chan %d, sfont %d, bank %d, preset %d, %s\n",
                                 i,
                                 fluid_sfont_get_id(preset->sfont),
                                 fluid_preset_get_banknum(preset),
                                 fluid_preset_get_num(preset),
                                 fluid_preset_get_name(preset));
        }
    }

    return FLUID_OK;
}

int
fluid_handle_load(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    char buf[1024];
    int id;
    int reset = 1;
    int offset = 0;

    if(ac < 1)
    {
        fluid_ostream_printf(out, "load: too few arguments\n");
        return FLUID_FAILED;
    }

    if(ac == 2)
    {
        reset = atoi(av[1]);
    }

    if(ac == 3)
    {
        offset = atoi(av[2]);
    }

    /* Load the SoundFont without resetting the programs. The reset will
     * be done later (if requested). */
    id = fluid_synth_sfload(handler->synth, fluid_expand_path(av[0], buf, 1024), 0);

    if(id == -1)
    {
        fluid_ostream_printf(out, "failed to load the SoundFont\n");
        return FLUID_FAILED;
    }
    else
    {
        fluid_ostream_printf(out, "loaded SoundFont has ID %d\n", id);
    }

    if(offset)
    {
        fluid_synth_set_bank_offset(handler->synth, id, offset);
    }

    /* The reset should be done after the offset is set. */
    if(reset)
    {
        fluid_synth_program_reset(handler->synth);
    }

    return FLUID_OK;
}

int
fluid_handle_unload(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int reset = 1;

    if(ac < 1)
    {
        fluid_ostream_printf(out, "unload: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "unload: expected a number as argument\n");
        return FLUID_FAILED;
    }

    if(ac == 2)
    {
        reset = atoi(av[1]);
    }

    if(fluid_synth_sfunload(handler->synth, atoi(av[0]), reset) != 0)
    {
        fluid_ostream_printf(out, "failed to unload the SoundFont\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

int
fluid_handle_reload(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 1)
    {
        fluid_ostream_printf(out, "reload: too few arguments\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "reload: expected a number as argument\n");
        return FLUID_FAILED;
    }

    if(fluid_synth_sfreload(handler->synth, atoi(av[0])) == -1)
    {
        fluid_ostream_printf(out, "failed to reload the SoundFont\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}


int
fluid_handle_fonts(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int i;
    fluid_sfont_t *sfont;
    int num;

    num = fluid_synth_sfcount(handler->synth);

    if(num == 0)
    {
        fluid_ostream_printf(out, "no SoundFont loaded (try load)\n");
        return FLUID_OK;
    }

    fluid_ostream_printf(out, "ID  Name\n");

    for(i = 0; i < num; i++)
    {
        sfont = fluid_synth_get_sfont(handler->synth, i);

        if(sfont)
        {
            fluid_ostream_printf(out, "%2d  %s\n",
                                 fluid_sfont_get_id(sfont),
                                 fluid_sfont_get_name(sfont));
        }
        else
        {
            fluid_ostream_printf(out, "sfont is \"NULL\" for index %d\n", i);
        }
    }

    return FLUID_OK;
}

/* Purpose:
 * Response to 'rev_preset' command.
 * Load the values from a reverb preset into the reverb unit. */
int
fluid_handle_reverbpreset(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int reverb_preset_number;

    fluid_ostream_printf(out, "rev_preset is deprecated and will be removed in a future release!\n");

    if(ac < 1)
    {
        fluid_ostream_printf(out, "rev_preset: too few arguments\n");
        return FLUID_FAILED;
    }

    reverb_preset_number = atoi(av[0]);

    if(fluid_synth_set_reverb_preset(handler->synth, reverb_preset_number) != FLUID_OK)
    {
        fluid_ostream_printf(out, "rev_preset: Failed. Parameter out of range?\n");
        return FLUID_FAILED;
    };

    return FLUID_OK;
}

/*
  The function is useful for reverb and chorus commands which have
  1 or 2 parameters.
  The function checks that there is 1 or 2 arguments.
  When there is 2 parameters it checks the first argument that must be
  an fx group index in the range[0..synth->effects_groups-1].

  return the group index:
  -1, when the command is for all fx groups.
  0 to synth->effects_groups-1, when the command is for this group index.
  -2 if error.
*/
static int check_fx_group_idx(int ac, char **av, fluid_ostream_t out,
                              fluid_synth_t *synth, const char *name_cde)
{
    int fx_group; /* fx unit index */
    int ngroups;  /* count of fx groups */

    /* One or 2 arguments allowed */
    if(ac < 1 || ac > 2)
    {
        fluid_ostream_printf(out, "%s: needs 1 or 2 arguments\n", name_cde);
        return -2;
    }

    /* check optional first argument which is a fx group index */
    fx_group = -1;

    if(ac > 1)
    {
        fx_group = atoi(av[0]); /* get fx group index */
        ngroups = fluid_synth_count_effects_groups(synth);

        if(!fluid_is_number(av[0]) || fx_group < 0 || fx_group >= ngroups)
        {
            fluid_ostream_printf(out, "%s: group index \"%s\" must be in range [%d..%d]\n",
                                 name_cde, av[0], 0, ngroups - 1);
            return -2;
        }
    }

    return fx_group;
}

/* parameter value */
struct value
{
    const char *name;
    double min;
    double max;
};

/*
  check 2 arguments for reverb commands : fx group index  , value
  - group index must be an integer in the range [-1..synth->effects_groups].
  - value must be a double in the range [min..max]
  @param param a pointer on a value to return the second value argument.
  return the fx group index:
  -1 when the command is for all fx group.
  0 to synth->effects_groups-1 when the command is for this group index.
  -2 if error.
*/
static int check_fx_reverb_param(int ac, char **av, fluid_ostream_t out,
                                 fluid_synth_t *synth, const char *name_cde,
                                 const struct value *value,
                                 fluid_real_t *param)
{
    /* get and check fx group index argument */
    int fx_group = check_fx_group_idx(ac, av, out, synth, name_cde);

    if(fx_group >= -1)
    {
        fluid_real_t val;

        /* get and check value argument */
        ac--;
        val = atof(av[ac]);

        if(!fluid_is_number(av[ac]) || val < value->min || val > value->max)
        {
            fluid_ostream_printf(out, "%s: %s \"%s\" must be in range [%f..%f]\n",
                                 name_cde, value->name, av[ac], value->min, value->max);
            return -2;
        }

        *param = val;
    }

    return fx_group;
}

/* Purpose:
 * Response to fluid_handle_reverbsetxxxx commands
 */
static int
fluid_handle_reverb_command(void *data, int ac, char **av, fluid_ostream_t out,
                            int param)
{
    int fx_group;

    /* reverb commands name table */
    static const char *const name_cde[FLUID_REVERB_PARAM_LAST] =
    {"rev_setroomsize", "rev_setdamp", "rev_setwidth", "rev_setlevel"};

    /* name and min/max values table */
    static struct value values[FLUID_REVERB_PARAM_LAST] =
    {
        {"room size", 0, 0},
        {"damp", 0, 0},
        {"width", 0, 0},
        {"level", 0, 0}
    };

    FLUID_ENTRY_COMMAND(data);
    fluid_real_t value;

    fluid_settings_getnum_range(handler->settings, "synth.reverb.room-size",
                                &values[FLUID_REVERB_ROOMSIZE].min,
                                &values[FLUID_REVERB_ROOMSIZE].max);

    fluid_settings_getnum_range(handler->settings, "synth.reverb.damp",
                                &values[FLUID_REVERB_DAMP].min,
                                &values[FLUID_REVERB_DAMP].max);


    fluid_settings_getnum_range(handler->settings, "synth.reverb.width",
                                &values[FLUID_REVERB_WIDTH].min,
                                &values[FLUID_REVERB_WIDTH].max);

    fluid_settings_getnum_range(handler->settings, "synth.reverb.level",
                                &values[FLUID_REVERB_LEVEL].min,
                                &values[FLUID_REVERB_LEVEL].max);

    /* get and check command arguments */
    fx_group = check_fx_reverb_param(ac, av, out, handler->synth,
                                     name_cde[param], &values[param], &value);

    if(fx_group >= -1)
    {
        /* run reverb function */
        fluid_synth_reverb_set_param(handler->synth, fx_group, param, value);
        return FLUID_OK;
    }

    return FLUID_FAILED;
}

/* Purpose:
 * Response to 'rev_setroomsize' command.
 * Load the new room size into the reverb fx group.
 * Example: rev_setroomzize 0 0.5
 * load roomsize 0.5 in the reverb fx group at index 0
 */
int
fluid_handle_reverbsetroomsize(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_reverb_command(data, ac, av, out, FLUID_REVERB_ROOMSIZE);
}

/* Purpose:
 * Response to 'rev_setdamp' command.
 * Load the new damp factor into the reverb fx group.
 * Example: rev_setdamp 1 0.5
 * load damp 0.5 in the reverb fx group at index 1
 */
int
fluid_handle_reverbsetdamp(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_reverb_command(data, ac, av, out, FLUID_REVERB_DAMP);
}

/* Purpose:
 * Response to 'rev_setwidth' command.
 * Load the new width into the reverb fx group.
 * Example: rev_setwidth 1 0.5
 * load width 0.5 in the reverb fx group at index 1.
 */
int
fluid_handle_reverbsetwidth(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_reverb_command(data, ac, av, out, FLUID_REVERB_WIDTH);
}

/* Purpose:
 * Response to 'rev_setlevel' command.
 * Load the new level into the reverb fx group.
 * Example: rev_setlevel 1 0.5
 * load level 0.5 in the reverb fx group at index 1.
 */
int
fluid_handle_reverbsetlevel(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_reverb_command(data, ac, av, out, FLUID_REVERB_LEVEL);
}

/* reverb/chorus on/off commands enum */
enum rev_chor_on_cde
{
    REVERB_ON_CDE,
    CHORUS_ON_CDE,
    NBR_REV_CHOR_ON_CDE
};

/* Purpose:
 * Set one or all reverb/chorus units on or off
 */
static int
fluid_handle_reverb_chorus_on_command(void *data, int ac, char **av, fluid_ostream_t out,
                                      enum rev_chor_on_cde cde)
{
    /* commands name table */
    static const char *const name_cde[NBR_REV_CHOR_ON_CDE] = {"reverb", "chorus"};
    /* functions table */
    static int (*onoff_func[NBR_REV_CHOR_ON_CDE])(fluid_synth_t *, int, int) =
    {
        fluid_synth_reverb_on, fluid_synth_chorus_on
    };

    FLUID_ENTRY_COMMAND(data);
    int onoff;

    /* get and check fx group index argument */
    int fx_group = check_fx_group_idx(ac, av, out, handler->synth, name_cde[cde]);

    if(fx_group >= -1)
    {
        ac--;

        /* check argument value */
        if((FLUID_STRCMP(av[ac], "0") == 0) || (FLUID_STRCMP(av[ac], "off") == 0))
        {
            onoff = 0;
        }
        else if((FLUID_STRCMP(av[ac], "1") == 0) || (FLUID_STRCMP(av[ac], "on") == 0))
        {
            onoff = 1;
        }
        else
        {
            fluid_ostream_printf(out, "%s: invalid arguments %s [0|1|on|off]\n",
                                 name_cde[cde], av[ac]);
            return FLUID_FAILED;
        }

        /* run on/off function */
        return onoff_func[cde](handler->synth, fx_group, onoff);
    }

    return FLUID_FAILED;
}

/* Purpose:
 * Response to: reverb [fx group] on command.
 * Examples:
 * reverb off ,disable all reverb groups.
 * reverb 1 on ,enable reverb group at index 1.
 */
int
fluid_handle_reverb(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_reverb_chorus_on_command(data, ac, av, out, REVERB_ON_CDE);
}

/* Purpose:
 * Response to fluid_handle_chorus_xxx commands
 */
static int
fluid_handle_chorus_command(void *data, int ac, char **av, fluid_ostream_t out,
                            int param)
{
    /* chorus commands name table */
    static const char *const name_cde[FLUID_CHORUS_PARAM_LAST - 1] =
    {"cho_set_nr", "cho_set_level", "cho_set_speed", "cho_set_depth"};

    /* value name table */
    static const char *const name_value[FLUID_CHORUS_PARAM_LAST - 1] =
    {"nr", "level", "speed", "depth"};

    /* setting name (except lfo waveform type) */
    static const char *name[FLUID_CHORUS_PARAM_LAST-1] =
    {
        "synth.chorus.nr", "synth.chorus.level",
        "synth.chorus.speed", "synth.chorus.depth"
    };

    FLUID_ENTRY_COMMAND(data);

    /* get and check index fx group index argument */
    int fx_group = check_fx_group_idx(ac, av, out, handler->synth, name_cde[param]);

    if(fx_group >= -1)
    {
        double value;
        /* get and check value argument */
        ac--;

        if(!fluid_is_number(av[ac]))
        {
            fluid_ostream_printf(out, "%s: %s \"%s\" must be a number\n",
                                 name_cde[param], name_value[param], av[ac]);
            return FLUID_FAILED;
        }

        if(param == FLUID_CHORUS_NR) /* commands with integer parameter */
        {
            int min, max;
            int int_value = atoi(av[ac]);

            fluid_settings_getint_range(handler->settings, name[param], &min, &max);
            if(int_value < min || int_value > max)
            {
                fluid_ostream_printf(out, "%s: %s \"%s\" must be in range [%d..%d]\n",
                                     name_cde[param], name_value[param], av[ac], min, max);
                return FLUID_FAILED;
            }
            value = (double)int_value;
        }
        else /* commands with float parameter */
        {
            double min, max;
            value = atof(av[ac]);

            fluid_settings_getnum_range(handler->settings, name[param], &min, &max);
            if(value < min || value > max)
            {
                fluid_ostream_printf(out, "%s: %s \"%s\" must be in range [%f..%f]\n",
                                     name_cde[param], name_value[param], av[ac], min, max);
                return FLUID_FAILED;
            }
        }

        /* run chorus function */
        fluid_synth_chorus_set_param(handler->synth, fx_group, param, value);
        return FLUID_OK;
    }

    return FLUID_FAILED;
}

/* Purpose:
 * Response to 'cho_set_nr' command
 * Load the new voice count into the chorus fx group.
 * Example: cho_set_nr 1 3
 * load 3 voices in the chorus fx group at index 1.
 */
int
fluid_handle_chorusnr(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_chorus_command(data, ac, av, out, FLUID_CHORUS_NR);
}

/* Purpose:
 * Response to 'cho_setlevel' command
 * Example: cho_set_level 1 3
 * load level 3 in the chorus fx group at index 1.
 */
int
fluid_handle_choruslevel(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_chorus_command(data, ac, av, out, FLUID_CHORUS_LEVEL);
}

/* Purpose:
 * Response to 'cho_setspeed' command
 * Example: cho_set_speed 1 0.1
 * load speed 0.1 in the chorus fx group at index 1.
 */
int
fluid_handle_chorusspeed(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_chorus_command(data, ac, av, out, FLUID_CHORUS_SPEED);
}

/* Purpose:
 * Response to 'cho_setdepth' command
 * Example: cho_set_depth 1 0.3
 * load depth 0.3 in the chorus fx group at index 1.
 */
int
fluid_handle_chorusdepth(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_chorus_command(data, ac, av, out, FLUID_CHORUS_DEPTH);
}

/* Purpose:
 * Response to: chorus [fx group] on command.
 * Examples:
 * chorus off ,disable all chorus groups.
 * chorus 1 on ,enable chorus group at index 1.
 */
int
fluid_handle_chorus(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_reverb_chorus_on_command(data, ac, av, out, CHORUS_ON_CDE);
}

/* Purpose:
 * Response to the 'echo' command.
 * The command itself is useful, when the synth is used via TCP/IP.
 * It can signal for example, that a list of commands has been processed.
 */
int
fluid_handle_echo(void *data, int ac, char **av, fluid_ostream_t out)
{
    if(ac < 1)
    {
        fluid_ostream_printf(out, "echo: too few arguments.\n");
        return FLUID_FAILED;
    }

    fluid_ostream_printf(out, "%s\n", av[0]);

    return FLUID_OK;
}

/* Purpose:
 * Sleep during a time in ms
 * The command itself is useful to insert a delay between commands.
 * It can help for example to build a small song using noteon/noteoff commands
 * in a command file.
 */
int
fluid_handle_sleep(void *data, int ac, char **av, fluid_ostream_t out)
{
    if(ac < 1)
    {
        fluid_ostream_printf(out, "sleep: too few arguments.\n");
        return -1;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "sleep: argument should be a number in ms.\n");
        return -1;
    }

    fluid_msleep(atoi(av[0]));	/* delay in milliseconds */

    return 0;
}

int
fluid_handle_source(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 1)
    {
        fluid_ostream_printf(out, "source: too few arguments.\n");
        return FLUID_FAILED;
    }

    fluid_source(handler, av[0]);

    return FLUID_OK;
}

/* Purpose:
 * Response to 'gain' command. */
int
fluid_handle_gain(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    float gain;

    if(ac < 1)
    {
        fluid_ostream_printf(out, "gain: too few arguments.\n");
        return FLUID_FAILED;
    }

    gain = atof(av[0]);

    if((gain < 0.0f) || (gain > 5.0f))
    {
        fluid_ostream_printf(out, "gain: value should be between '0' and '5'.\n");
        return FLUID_FAILED;
    };

    fluid_synth_set_gain(handler->synth, gain);

    return FLUID_OK;
}

/* Response to voice_count command */
static int
fluid_handle_voice_count(void *data, int ac, char **av,
                         fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ostream_printf(out, "voice_count: %d\n",
                         fluid_synth_get_active_voice_count(handler->synth));
    return FLUID_OK;
}

/* Purpose:
 * Response to 'interp' command. */
int
fluid_handle_interp(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int interp;
    int chan = -1; /* -1: Set all channels */

    if(ac < 1)
    {
        fluid_ostream_printf(out, "interp: too few arguments.\n");
        return FLUID_FAILED;
    }

    interp = atoi(av[0]);

    if((interp < 0) || (interp > FLUID_INTERP_HIGHEST))
    {
        fluid_ostream_printf(out, "interp: Bad value\n");
        return FLUID_FAILED;
    };

    fluid_synth_set_interp_method(handler->synth, chan, interp);

    return FLUID_OK;
}

/* Purpose:
 * Response to 'interp' command. */
int
fluid_handle_interpc(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int interp;
    int chan;

    if(ac < 2)
    {
        fluid_ostream_printf(out, "interpc: too few arguments.\n");
        return FLUID_FAILED;
    }

    chan = atoi(av[0]);
    interp = atoi(av[1]);

    if((chan < 0) || (chan >= fluid_synth_count_midi_channels(handler->synth)))
    {
        fluid_ostream_printf(out, "interp: Bad value for channel number.\n");
        return FLUID_FAILED;
    };

    if((interp < 0) || (interp > FLUID_INTERP_HIGHEST))
    {
        fluid_ostream_printf(out, "interp: Bad value for interpolation method.\n");
        return FLUID_FAILED;
    };

    fluid_synth_set_interp_method(handler->synth, chan, interp);

    return FLUID_OK;
}

int
fluid_handle_tuning(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    char *name;
    int bank, prog;

    if(ac < 3)
    {
        fluid_ostream_printf(out, "tuning: too few arguments.\n");
        return FLUID_FAILED;
    }

    name = av[0];

    if(!fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "tuning: 2nd argument should be a number.\n");
        return FLUID_FAILED;
    }

    bank = atoi(av[1]);

    if((bank < 0) || (bank >= 128))
    {
        fluid_ostream_printf(out, "tuning: invalid bank number.\n");
        return FLUID_FAILED;
    };

    if(!fluid_is_number(av[2]))
    {
        fluid_ostream_printf(out, "tuning: 3rd argument should be a number.\n");
        return FLUID_FAILED;
    }

    prog = atoi(av[2]);

    if((prog < 0) || (prog >= 128))
    {
        fluid_ostream_printf(out, "tuning: invalid program number.\n");
        return FLUID_FAILED;
    };

    fluid_synth_activate_key_tuning(handler->synth, bank, prog, name, NULL, FALSE);

    return FLUID_OK;
}

int
fluid_handle_tune(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int bank, prog, key;
    double pitch;

    if(ac < 4)
    {
        fluid_ostream_printf(out, "tune: too few arguments.\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "tune: 1st argument should be a number.\n");
        return FLUID_FAILED;
    }

    bank = atoi(av[0]);

    if((bank < 0) || (bank >= 128))
    {
        fluid_ostream_printf(out, "tune: invalid bank number.\n");
        return FLUID_FAILED;
    };

    if(!fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "tune: 2nd argument should be a number.\n");
        return FLUID_FAILED;
    }

    prog = atoi(av[1]);

    if((prog < 0) || (prog >= 128))
    {
        fluid_ostream_printf(out, "tune: invalid program number.\n");
        return FLUID_FAILED;
    };

    if(!fluid_is_number(av[2]))
    {
        fluid_ostream_printf(out, "tune: 3rd argument should be a number.\n");
        return FLUID_FAILED;
    }

    key = atoi(av[2]);

    if((key < 0) || (key >= 128))
    {
        fluid_ostream_printf(out, "tune: invalid key number.\n");
        return FLUID_FAILED;
    };

    pitch = atof(av[3]);

    if(pitch < 0.0f)
    {
        fluid_ostream_printf(out, "tune: invalid pitch.\n");
        return FLUID_FAILED;
    };

    fluid_synth_tune_notes(handler->synth, bank, prog, 1, &key, &pitch, 0);

    return FLUID_OK;
}

int
fluid_handle_settuning(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int chan, bank, prog;

    if(ac < 3)
    {
        fluid_ostream_printf(out, "settuning: too few arguments.\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "tune: 1st argument should be a number.\n");
        return FLUID_FAILED;
    }

    chan = atoi(av[0]);

    if((chan < 0) || (chan >= fluid_synth_count_midi_channels(handler->synth)))
    {
        fluid_ostream_printf(out, "tune: invalid channel number.\n");
        return FLUID_FAILED;
    };

    if(!fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "tuning: 2nd argument should be a number.\n");
        return FLUID_FAILED;
    }

    bank = atoi(av[1]);

    if((bank < 0) || (bank >= 128))
    {
        fluid_ostream_printf(out, "tuning: invalid bank number.\n");
        return FLUID_FAILED;
    };

    if(!fluid_is_number(av[2]))
    {
        fluid_ostream_printf(out, "tuning: 3rd argument should be a number.\n");
        return FLUID_FAILED;
    }

    prog = atoi(av[2]);

    if((prog < 0) || (prog >= 128))
    {
        fluid_ostream_printf(out, "tuning: invalid program number.\n");
        return FLUID_FAILED;
    };

    fluid_synth_activate_tuning(handler->synth, chan, bank, prog, FALSE);

    return FLUID_OK;
}

int
fluid_handle_resettuning(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int chan;

    if(ac < 1)
    {
        fluid_ostream_printf(out, "resettuning: too few arguments.\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "tune: 1st argument should be a number.\n");
        return FLUID_FAILED;
    }

    chan = atoi(av[0]);

    if((chan < 0) || (chan >= fluid_synth_count_midi_channels(handler->synth)))
    {
        fluid_ostream_printf(out, "tune: invalid channel number.\n");
        return FLUID_FAILED;
    };

    fluid_synth_deactivate_tuning(handler->synth, chan, FALSE);

    return FLUID_OK;
}

int
fluid_handle_tunings(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int bank, prog;
    char name[256];
    int count = 0;

    fluid_synth_tuning_iteration_start(handler->synth);

    while(fluid_synth_tuning_iteration_next(handler->synth, &bank, &prog))
    {
        fluid_synth_tuning_dump(handler->synth, bank, prog, name, 256, NULL);
        fluid_ostream_printf(out, "%03d-%03d %s\n", bank, prog, name);
        count++;
    }

    if(count == 0)
    {
        fluid_ostream_printf(out, "No tunings available\n");
    }

    return FLUID_OK;
}

int
fluid_handle_dumptuning(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int bank, prog, i, res;
    double pitch[128];
    char name[256];

    if(ac < 2)
    {
        fluid_ostream_printf(out, "dumptuning: too few arguments.\n");
        return FLUID_FAILED;
    }

    if(!fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "dumptuning: 1st argument should be a number.\n");
        return FLUID_FAILED;
    }

    bank = atoi(av[0]);

    if((bank < 0) || (bank >= 128))
    {
        fluid_ostream_printf(out, "dumptuning: invalid bank number.\n");
        return FLUID_FAILED;
    };

    if(!fluid_is_number(av[1]))
    {
        fluid_ostream_printf(out, "dumptuning: 2nd argument should be a number.\n");
        return FLUID_FAILED;
    }

    prog = atoi(av[1]);

    if((prog < 0) || (prog >= 128))
    {
        fluid_ostream_printf(out, "dumptuning: invalid program number.\n");
        return FLUID_FAILED;
    };

    res = fluid_synth_tuning_dump(handler->synth, bank, prog, name, 256, pitch);

    if(FLUID_OK != res)
    {
        fluid_ostream_printf(out, "Tuning %03d-%03d does not exist.\n", bank, prog);
        return FLUID_FAILED;
    }

    fluid_ostream_printf(out, "%03d-%03d %s:\n", bank, prog, name);

    for(i = 0; i < 128; i++)
    {
        fluid_ostream_printf(out, "key %03d, pitch %5.2f\n", i, pitch[i]);
    }

    return FLUID_OK;
}

int
fluid_handle_set(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    int hints;
    int ival, icur;
    double fval, fcur;
    char *scur;
    int ret = FLUID_FAILED;

    if(ac < 2)
    {
        fluid_ostream_printf(out, "set: Too few arguments.\n");
        return ret;
    }

    switch(fluid_settings_get_type(handler->settings, av[0]))
    {
    case FLUID_NO_TYPE:
        fluid_ostream_printf(out, "set: Parameter '%s' not found.\n", av[0]);
        return ret;

    case FLUID_INT_TYPE:
        if(fluid_settings_get_hints(handler->settings, av[0], &hints) == FLUID_OK
                && hints & FLUID_HINT_TOGGLED)
        {
            if(FLUID_STRCASECMP(av[1], "yes") == 0
                    || FLUID_STRCASECMP(av[1], "true") == 0
                    || FLUID_STRCASECMP(av[1], "t") == 0)
            {
                ival = 1;
            }
            else
            {
                ival = atoi(av[1]);
            }
        }
        else
        {
            ival = atoi(av[1]);
        }

        fluid_settings_getint(handler->settings, av[0], &icur);
        if (icur == ival)
        {
            return FLUID_OK;
        }

        ret = fluid_settings_setint(handler->settings, av[0], ival);
        break;

    case FLUID_NUM_TYPE:
        fval = atof(av[1]);
        fluid_settings_getnum(handler->settings, av[0], &fcur);
        if (fcur == fval)
        {
            return FLUID_OK;
        }

        ret = fluid_settings_setnum(handler->settings, av[0], fval);
        break;

    case FLUID_STR_TYPE:
        fluid_settings_dupstr(handler->settings, av[0], &scur);

        if(scur && !FLUID_STRCMP(scur, av[1]))
        {
            FLUID_FREE(scur);
            return FLUID_OK;
        }
        ret = fluid_settings_setstr(handler->settings, av[0], av[1]);
        FLUID_FREE(scur);
        break;

    case FLUID_SET_TYPE:
        fluid_ostream_printf(out, "set: Parameter '%s' is a node.\n", av[0]);
        return FLUID_FAILED;

    default:
        fluid_ostream_printf(out, "Unhandled settings type.");
        return FLUID_FAILED;
    }

    if(ret == FLUID_FAILED)
    {
        fluid_ostream_printf(out, "set: Value out of range. Try 'info %s' for valid ranges\n", av[0]);
    }

    if((handler->synth != NULL || handler->router != NULL) && !fluid_settings_is_realtime(handler->settings, av[0]))
    {
        fluid_ostream_printf(out, "Warning: '%s' is not a realtime setting, changes won't take effect.\n", av[0]);
    }

    return ret;
}

int
fluid_handle_get(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);

    if(ac < 1)
    {
        fluid_ostream_printf(out, "get: too few arguments.\n");
        return FLUID_FAILED;
    }

    switch(fluid_settings_get_type(handler->settings, av[0]))
    {
    case FLUID_NO_TYPE:
        fluid_ostream_printf(out, "get: no such setting '%s'.\n", av[0]);
        return FLUID_FAILED;

    case FLUID_NUM_TYPE:
    {
        double value;
        fluid_settings_getnum(handler->settings, av[0], &value);
        fluid_ostream_printf(out, "%.3f\n", value);
        break;
    }

    case FLUID_INT_TYPE:
    {
        int value;
        fluid_settings_getint(handler->settings, av[0], &value);
        fluid_ostream_printf(out, "%d\n", value);
        break;
    }

    case FLUID_STR_TYPE:
    {
        char *s = NULL;
        fluid_settings_dupstr(handler->settings, av[0], &s);       /* ++ alloc string */
        fluid_ostream_printf(out, "%s\n", s ? s : "NULL");
        FLUID_FREE(s);    /* -- free string */

        break;
    }

    case FLUID_SET_TYPE:
        fluid_ostream_printf(out, "%s is a node\n", av[0]);
        break;
    }

    return FLUID_OK;
}

struct _fluid_handle_settings_data_t
{
    size_t len;
    fluid_settings_t *settings;
    fluid_ostream_t out;
};

static void fluid_handle_settings_iter1(void *data, const char *name, int type)
{
    struct _fluid_handle_settings_data_t *d = (struct _fluid_handle_settings_data_t *) data;

    size_t len = FLUID_STRLEN(name);

    if(len > d->len)
    {
        d->len = len;
    }
}

static void fluid_handle_settings_iter2(void *data, const char *name, int type)
{
    struct _fluid_handle_settings_data_t *d = (struct _fluid_handle_settings_data_t *) data;

    size_t len = FLUID_STRLEN(name);
    fluid_ostream_printf(d->out, "%s", name);

    while(len++ < d->len)
    {
        fluid_ostream_printf(d->out, " ");
    }

    fluid_ostream_printf(d->out, "   ");

    switch(fluid_settings_get_type(d->settings, name))
    {
    case FLUID_NUM_TYPE:
    {
        double value;
        fluid_settings_getnum(d->settings, name, &value);
        fluid_ostream_printf(d->out, "%.3f\n", value);
        break;
    }

    case FLUID_INT_TYPE:
    {
        int value, hints;
        fluid_settings_getint(d->settings, name, &value);

        if(fluid_settings_get_hints(d->settings, name, &hints) == FLUID_OK)
        {
            if(!(hints & FLUID_HINT_TOGGLED))
            {
                fluid_ostream_printf(d->out, "%d\n", value);
            }
            else
            {
                fluid_ostream_printf(d->out, "%s\n", value ? "True" : "False");
            }
        }

        break;
    }

    case FLUID_STR_TYPE:
    {
        char *s = NULL;
        fluid_settings_dupstr(d->settings, name, &s);     /* ++ alloc string */
        fluid_ostream_printf(d->out, "%s\n", s ? s : "NULL");
        FLUID_FREE(s);    /* -- free string */

        break;
    }
    }
}

int
fluid_handle_settings(void *d, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(d);
    struct _fluid_handle_settings_data_t data;

    data.len = 0;
    data.settings = handler->settings;
    data.out = out;

    fluid_settings_foreach(handler->settings, &data, fluid_handle_settings_iter1);
    fluid_settings_foreach(handler->settings, &data, fluid_handle_settings_iter2);
    return FLUID_OK;
}


struct _fluid_handle_option_data_t
{
    int first;
    fluid_ostream_t out;
};

void fluid_handle_print_option(void *data, const char *name, const char *option)
{
    struct _fluid_handle_option_data_t *d = (struct _fluid_handle_option_data_t *) data;

    if(d->first)
    {
        fluid_ostream_printf(d->out, "%s", option);
        d->first = 0;
    }
    else
    {
        fluid_ostream_printf(d->out, ", %s", option);
    }
}

int
fluid_handle_info(void *d, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(d);
    fluid_settings_t *settings = handler->settings;
    struct _fluid_handle_option_data_t data;

    if(ac < 1)
    {
        fluid_ostream_printf(out, "info: too few arguments.\n");
        return FLUID_FAILED;
    }

    switch(fluid_settings_get_type(settings, av[0]))
    {
    case FLUID_NO_TYPE:
        fluid_ostream_printf(out, "info: no such setting '%s'.\n", av[0]);
        return FLUID_FAILED;

    case FLUID_NUM_TYPE:
    {
        double value, min, max, def;

        if(fluid_settings_getnum_range(settings, av[0], &min, &max) == FLUID_OK
                && fluid_settings_getnum(settings, av[0], &value) == FLUID_OK
                && fluid_settings_getnum_default(settings, av[0], &def) == FLUID_OK)
        {
            fluid_ostream_printf(out, "%s:\n", av[0]);
            fluid_ostream_printf(out, "Type:          number\n");
            fluid_ostream_printf(out, "Value:         %.3f\n", value);
            fluid_ostream_printf(out, "Minimum value: %.3f\n", min);
            fluid_ostream_printf(out, "Maximum value: %.3f\n", max);
            fluid_ostream_printf(out, "Default value: %.3f\n", def);
            fluid_ostream_printf(out, "Real-time:     %s\n",
                                 fluid_settings_is_realtime(settings, av[0]) ? "yes" : "no");
        }
        else
        {
            fluid_ostream_printf(out, "An error occurred when processing %s\n", av[0]);
        }

        break;
    }

    case FLUID_INT_TYPE:
    {
        int value, min, max, def, hints;

        if(fluid_settings_getint_range(settings, av[0], &min, &max) == FLUID_OK
                && fluid_settings_getint(settings, av[0], &value) == FLUID_OK
                && fluid_settings_get_hints(settings, av[0], &hints) == FLUID_OK
                && fluid_settings_getint_default(settings, av[0], &def) == FLUID_OK)
        {
            fluid_ostream_printf(out, "%s:\n", av[0]);

            if(!(hints & FLUID_HINT_TOGGLED))
            {
                fluid_ostream_printf(out, "Type:          integer\n");
                fluid_ostream_printf(out, "Value:         %d\n", value);
                fluid_ostream_printf(out, "Minimum value: %d\n", min);
                fluid_ostream_printf(out, "Maximum value: %d\n", max);
                fluid_ostream_printf(out, "Default value: %d\n", def);
            }
            else
            {
                fluid_ostream_printf(out, "Type:          boolean\n");
                fluid_ostream_printf(out, "Value:         %s\n", value ? "True" : "False");
                fluid_ostream_printf(out, "Default value: %s\n", def ? "True" : "False");
            }

            fluid_ostream_printf(out, "Real-time:     %s\n",
                                 fluid_settings_is_realtime(settings, av[0]) ? "yes" : "no");
        }
        else
        {
            fluid_ostream_printf(out, "An error occurred when processing %s\n", av[0]);
        }

        break;
    }

    case FLUID_STR_TYPE:
    {
        char *s = NULL;
        fluid_settings_dupstr(settings, av[0], &s);         /* ++ alloc string */
        fluid_ostream_printf(out, "%s:\n", av[0]);
        fluid_ostream_printf(out, "Type:          string\n");
        fluid_ostream_printf(out, "Value:         %s\n", s ? s : "NULL");
        FLUID_FREE(s);    /* -- free string */

        fluid_settings_getstr_default(settings, av[0], &s);
        fluid_ostream_printf(out, "Default value: %s\n", s);

        data.out = out;
        data.first = 1;
        fluid_ostream_printf(out, "Options:       ");
        fluid_settings_foreach_option(settings, av[0], &data, fluid_handle_print_option);
        fluid_ostream_printf(out, "\n");

        fluid_ostream_printf(out, "Real-time:     %s\n",
                             fluid_settings_is_realtime(settings, av[0]) ? "yes" : "no");
        break;
    }

    case FLUID_SET_TYPE:
        fluid_ostream_printf(out, "%s:\n", av[0]);
        fluid_ostream_printf(out, "Type:          node\n");
        break;
    }

    return FLUID_OK;
}

int
fluid_handle_reset(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_system_reset(handler->synth);
    return FLUID_OK;
}

int
fluid_handle_quit(void *data, int ac, char **av, fluid_ostream_t out)
{
    fluid_ostream_printf(out, "cheers!\n");
    return -2;
}

int
fluid_handle_help(void *data, int ac, char **av, fluid_ostream_t out)
{
    /* Purpose:
     * Prints the help text for the command line commands.
     * Can be used as follows:
     * - help
     * - help (topic), where (topic) is 'general', 'chorus', etc.
     * - help all
     */
    char *topic = "help"; /* default, if no topic is given */
    int count = 0;
    unsigned int i;

    fluid_ostream_printf(out, "\n");

    /* 1st argument (optional): help topic */
    if(ac >= 1)
    {
        topic = av[0];
    }

    if(FLUID_STRCMP(topic, "help") == 0)
    {
        /* "help help": Print a list of all topics */
        fluid_ostream_printf(out,
                             "*** Help topics:***\n"
                             "help all (prints all topics)\n");

        for(i = 0; i < FLUID_N_ELEMENTS(fluid_commands); i++)
        {
            int listed_first_time = 1;
            unsigned int ii;

            for(ii = 0; ii < i; ii++)
            {
                if(FLUID_STRCMP(fluid_commands[i].topic, fluid_commands[ii].topic) == 0)
                {
                    listed_first_time = 0;
                }; /* if topic has already been listed */
            }; /* for all topics (inner loop) */

            if(listed_first_time)
            {
                fluid_ostream_printf(out, "help %s\n", fluid_commands[i].topic);
            };
        }; /* for all topics (outer loop) */
    }
    else
    {
        /* help (arbitrary topic or "all") */
        for(i = 0; i < FLUID_N_ELEMENTS(fluid_commands); i++)
        {
            if(fluid_commands[i].help != NULL)
            {
                if(FLUID_STRCMP(topic, "all") == 0 || FLUID_STRCMP(topic, fluid_commands[i].topic) == 0)
                {
                    fluid_ostream_printf(out, "%s\n", fluid_commands[i].help);
                    count++;
                }; /* if it matches the topic */
            }; /* if help text exists */
        }; /* foreach command */

        if(count == 0)
        {
            fluid_ostream_printf(out, "Unknown help topic. Try 'help help'.\n");
        };
    };

    return FLUID_OK;
}

#define CHECK_VALID_ROUTER(_router, _out)                                                \
  if (router == NULL) {                                                                  \
    fluid_ostream_printf(out, "cannot execute router command without a midi router.\n"); \
    return FLUID_FAILED;                                                                 \
  }

/* Command handler for "router_clear" command */
int fluid_handle_router_clear(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 0)
    {
        fluid_ostream_printf(out, "router_clear needs no arguments.\n");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    fluid_midi_router_clear_rules(router);

    return FLUID_OK;
}

/* Command handler for "router_default" command */
int fluid_handle_router_default(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 0)
    {
        fluid_ostream_printf(out, "router_default needs no arguments.\n");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    fluid_midi_router_set_default_rules(router);

    return FLUID_OK;
}

/* Command handler for "router_begin" command */
int fluid_handle_router_begin(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 1)
    {
        fluid_ostream_printf(out, "router_begin requires [note|cc|prog|pbend|cpress|kpress]\n");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    if(FLUID_STRCMP(av[0], "note") == 0)
    {
        handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_NOTE;
    }
    else if(FLUID_STRCMP(av[0], "cc") == 0)
    {
        handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_CC;
    }
    else if(FLUID_STRCMP(av[0], "prog") == 0)
    {
        handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_PROG_CHANGE;
    }
    else if(FLUID_STRCMP(av[0], "pbend") == 0)
    {
        handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_PITCH_BEND;
    }
    else if(FLUID_STRCMP(av[0], "cpress") == 0)
    {
        handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE;
    }
    else if(FLUID_STRCMP(av[0], "kpress") == 0)
    {
        handler->cmd_rule_type = FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE;
    }
    else
    {
        fluid_ostream_printf(out, "router_begin requires [note|cc|prog|pbend|cpress|kpress]\n");
        return FLUID_FAILED;
    }

    if(handler->cmd_rule)
    {
        delete_fluid_midi_router_rule(handler->cmd_rule);
    }

    handler->cmd_rule = new_fluid_midi_router_rule();

    if(!handler->cmd_rule)
    {
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/* Command handler for "router_end" command */
int fluid_handle_router_end(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 0)
    {
        fluid_ostream_printf(out, "router_end needs no arguments.\n");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    if(!handler->cmd_rule)
    {
        fluid_ostream_printf(out, "No active router_begin command.\n");
        return FLUID_FAILED;
    }

    /* Add the rule */
    if(fluid_midi_router_add_rule(router, handler->cmd_rule, handler->cmd_rule_type) != FLUID_OK)
    {
        delete_fluid_midi_router_rule(handler->cmd_rule);    /* Free on failure */
    }

    handler->cmd_rule = NULL;

    return FLUID_OK;
}

/* Command handler for "router_chan" command */
int fluid_handle_router_chan(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 4)
    {
        fluid_ostream_printf(out, "router_chan needs four args: min, max, mul, add.");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    if(!handler->cmd_rule)
    {
        fluid_ostream_printf(out, "No active router_begin command.\n");
        return FLUID_FAILED;
    }

    fluid_midi_router_rule_set_chan(handler->cmd_rule, atoi(av[0]), atoi(av[1]),
                                    atof(av[2]), atoi(av[3]));
    return FLUID_OK;
}

/* Command handler for "router_par1" command */
int fluid_handle_router_par1(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 4)
    {
        fluid_ostream_printf(out, "router_par1 needs four args: min, max, mul, add.");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    if(!handler->cmd_rule)
    {
        fluid_ostream_printf(out, "No active router_begin command.\n");
        return FLUID_FAILED;
    }

    fluid_midi_router_rule_set_param1(handler->cmd_rule, atoi(av[0]), atoi(av[1]),
                                      atof(av[2]), atoi(av[3]));
    return FLUID_OK;
}

/* Command handler for "router_par2" command */
int fluid_handle_router_par2(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_midi_router_t *router = handler->router;

    if(ac != 4)
    {
        fluid_ostream_printf(out, "router_par2 needs four args: min, max, mul, add.");
        return FLUID_FAILED;
    }

    CHECK_VALID_ROUTER(router, out);

    if(!handler->cmd_rule)
    {
        fluid_ostream_printf(out, "No active router_begin command.\n");
        return FLUID_FAILED;
    }

    fluid_midi_router_rule_set_param2(handler->cmd_rule, atoi(av[0]), atoi(av[1]),
                                      atof(av[2]), atoi(av[3]));
    return FLUID_OK;
}

/**  commands Poly/mono mode *************************************************/

static const char *const mode_name[] =
{
    "poly omni on (0)", "mono omni on (1)",
    "poly omni off(2)", "mono omni off(3)"
};

/*
  Prints result message for commands: basicchannels, resetbasicchannels.
  Prints all basic channels and print a warning if there is no basic channel.

  @param synth the synth instance.
  @param out output stream.
*/
static int print_basic_channels(fluid_synth_t *synth, fluid_ostream_t out)
{
    static const char warning_msg[] = "Warning: no basic channels. All MIDI channels are disabled.\n"
                                      "Make use of setbasicchannels to set at least a default basic channel.\n";

    int n_chan = synth->midi_channels;
    int i, n = 0;

    /* prints all basic channels */
    for(i = 0; i < n_chan; i++)
    {
        int basic_chan, mode_chan, val;

        if(fluid_synth_get_basic_channel(synth, i, &basic_chan, &mode_chan, &val) == FLUID_OK)
        {
            if(basic_chan == i)
            {
                n++;
                fluid_ostream_printf(out, "Basic channel:%3d, %s, nbr:%3d\n", i,
                                     mode_name[mode_chan &  FLUID_CHANNEL_MODE_MASK ],
                                     val);
            }
        }
        else
        {
            return FLUID_FAILED; /* error */
        }
    }

    /* prints a warning if there is no basic channel */
    if(n == 0)
    {
        fluid_ostream_printf(out, warning_msg);
    }

    return FLUID_OK;
}

/*-----------------------------------------------------------------------------
  basicchannels
   Prints the list of all MIDI basic channels information
   example:

	Basic channel:  0, poly omni on (0), nbr:  3
	Basic channel:  3, poly omni off(2), nbr:  1
	Basic channel:  8, mono omni off(3), nbr:  2
	Basic channel: 13, mono omni on (1), nbr:  3
*/
int fluid_handle_basicchannels(void *data, int ac, char **av,
                               fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    return print_basic_channels(synth, out);
}

/*
  Searches a mode name and returns the channel mode number.
  name must be: poly_omnion,  mono_omnion, poly_omnioff, mono_omnioff.
  @param name to search.
  @return channel mode number (0 to 3) if name is valid, -1 otherwise.
*/
static int get_channel_mode_num(char *name)
{
    /* argument names for channel mode parameter (see resetbasicchannels and
       setbasicchannels commands*/
    static const char *const name_channel_mode [FLUID_CHANNEL_MODE_LAST] =
    {"poly_omnion", "mono_omnion", "poly_omnioff", "mono_omnioff"};
    int i;

    for(i = 0 ; i <  FLUID_CHANNEL_MODE_LAST; i++)
    {
        if(! FLUID_STRCMP(name, name_channel_mode[i]))
        {
            return i;
        }
    }

    return -1;
}

static const char invalid_arg_msg[] = "invalid argument\n";
/*
 checks basic channels arguments: chan1 mode1 val  chan2 mode2 val2  ...
 All arguments can be numeric. mode parameter can be a name.
 Each group entry must have 3 parameters (chan,mode,val).

 @param ac argument count.
 @param av argument table.
 @param out output stream.
 @param name_cde command name prefix.
 @return 0 if arguments are valid, -1 otherwise.
*/
static int check_basicchannels_arguments(int ac, char **av,
        fluid_ostream_t out, char const *name_cde)
{
    static const char too_few_arg_msg[] = "too few argument, chan mode val [chan mode val]...\n";
    int i;

    for(i = 0; i < ac; i++)
    {
        /* checks parameters for list entries: 	chan1 mode1 val  chan2 mode2 val2  ...*/
        /* all parameters can be numeric. mode parameter can be a name. */
        if(!fluid_is_number(av[i]) &&
                ((i % 3 != 1) || get_channel_mode_num(av[i]) < 0))
        {
            fluid_ostream_printf(out, "%s: %s", name_cde, invalid_arg_msg);
            return -1;
        }
    }

    if(ac % 3)
    {
        /* each group entry needs 3 parameters: basicchan,mode,val */
        fluid_ostream_printf(out, "%s: channel %d, %s\n", name_cde,
                             atoi(av[((ac / 3) * 3)]), too_few_arg_msg);
        return -1;
    }

    return 0;
}

/*
 checks channels arguments: chan1  chan2   ...
 all arguments must be numeric.

 @param ac argument count.
 @param av argument table.
 @param out output stream.
 @param name_cde command name prefix.
 @return 0 if arguments are valid, -1 otherwise.
*/
static int check_channels_arguments(int ac, char **av,
                                    fluid_ostream_t out, char const *name_cde)
{
    int i;

    for(i = 0; i < ac; i++)
    {
        if(!fluid_is_number(av[i]))
        {
            fluid_ostream_printf(out, "%s: %s", name_cde, invalid_arg_msg);
            return -1;
        }
    }

    return 0;
}

/*-----------------------------------------------------------------------------
  resetbasicchannels

  With no parameters the command resets all basic channels.
  Note: Be aware than when a synth instance has no basic channels, all channels
  are disabled.
  In the intend to get some MIDI channels enabled, use the command setbasicchannels.

  resetbasicchannels chan1  [chan2  .  .  .]
  Resets basic channel group chan1, basic channel group chan2 . . .
*/
int fluid_handle_resetbasicchannels(void *data, int ac, char **av,
                                    fluid_ostream_t out)
{
    static const char name_cde[] = "resetbasicchannels";
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;

    /* checks channels arguments: chan1 chan2 .... */
    if(check_channels_arguments(ac, av, out, name_cde) < 0)
    {
        return -1;
    }

    if(ac)
    {
        /* resetbasicchannels chan1  [chan2  .  .  .] */
        int i;

        for(i = 0; i < ac; i++)
        {
            int chan = atoi(av[i]);
            int result = fluid_synth_reset_basic_channel(synth, chan);

            if(result == FLUID_FAILED)
            {
                fluid_ostream_printf(out, "%s: channel %3d, %s", name_cde, chan, invalid_arg_msg);
            }
        }
    }
    else
    {
        /* resets all basic channels */
        fluid_synth_reset_basic_channel(synth, -1);
    }

    /* prints result */
    return print_basic_channels(synth, out);
}

/*-----------------------------------------------------------------------------
  setbasicchannels

  With no parameters the command sets one channel basic at basic channel 0 in
  Omni On Poly (i.e all the MIDI channels are polyphonic).

  setbasicchannels chan1 mode1 nbr1    [chan2 mode2 nbr2]  ...  ...

  Adds basic channel 1 and 2

  The command fails if any channels overlaps any existing basic channel groups.
  To make room if necessary, existing basic channel groups can be cleared using
  resetbasicchannels command.
  Mode can be a numeric value or a name:
      numeric: 0 to 3 or
      name: poly_omnion , mono_omnion, poly_omnioff, mono_omnioff.
*/
int fluid_handle_setbasicchannels(void *data, int ac, char **av,
                                  fluid_ostream_t out)
{
    static const char name_cde[] = "setbasicchannels";
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int result;
    int i, n ;

    if(!ac)
    {
        /* sets one default basic channel */
        fluid_synth_reset_basic_channel(synth, -1); /* reset all basic channels */
        /* sets one basic channel Omni On Poly (i.e all the MIDI channels are polyphonic) */
        fluid_synth_set_basic_channel(synth, 0, FLUID_CHANNEL_MODE_OMNION_POLY, 0);
        return 0;
    }

    /* checks parameters: 	chan1 mode1 val1  chan2 mode2 val2 */
    if(check_basicchannels_arguments(ac, av, out, name_cde) < 0)
    {
        return -1;
    }

    n = ac / 3; /* number of basic channel information */

    for(i = 0; i < n; i++)
    {
        int basicchan, mode, val;

        basicchan = atoi(av[(i * 3)]);  /* chan is numeric */

        if(fluid_is_number(av[(i * 3) + 1]))
        {
            /* chan is numeric */
            mode = atoi(av[(i * 3) + 1]);
        }
        else
        {
            /* mode is a name */
            mode = get_channel_mode_num(av[(i * 3) + 1]);
        }

        val = atoi(av[(i * 3) + 2]);    /* val is numeric */

        /* changes or sets basic channels */
        result = fluid_synth_set_basic_channel(synth, basicchan, mode, val);

        if(result == FLUID_FAILED)
        {
            fluid_ostream_printf(out, "%s: channel %3d, mode %3d, nbr %3d, %s",
                                 name_cde, basicchan, mode, val, invalid_arg_msg);
        }
    }

    return 0;
}

/*
 Print result message : "channel:x is outside MIDI channel count(y)"
 for commands: channelsmode, portamentomode, legatomode, breathmode,setbreathmode.
 @param out output stream.
 @param name_cde command name prefix.
 @param chan, MIDI channel number x.
 @param n_chan, number of MIDI channels y.
*/
static void print_channel_is_outside_count(fluid_ostream_t out, char const *name_cde,
        int chan, int n_chan)
{
    fluid_ostream_printf(out, "%s: channel %3d is outside MIDI channel count(%d)\n",
                         name_cde, chan, n_chan);
}


/*-----------------------------------------------------------------------------
  channelsmode
     Prints channel mode of all MIDI channels (Poly/mono, Enabled, Basic Channel)
     example

     Channel    , Status , Type         , Mode            , Nbr of channels
     channel:  0, disabled
     channel:  1, disabled
     channel:  2, disabled
     channel:  3, disabled
     channel:  4, disabled
     channel:  5, enabled, basic channel, mono omni off(3), nbr:  2
     channel:  6, enabled, --           , mono            , --
     channel:  7, disabled
     channel:  8, disabled
     channel:  9, disabled
     channel: 10, enabled, basic channel, mono omni off(3), nbr:  4
     channel: 11, enabled, --           , mono            , --
     channel: 12, enabled, --           , mono            , --
     channel: 13, enabled, --           , mono            , --
     channel: 14, disabled
     channel: 15, disabled

  channelsmode chan1 chan2
     Prints only channel mode of MIDI channels chan1, chan2
*/
int fluid_handle_channelsmode(void *data, int ac, char **av,
                              fluid_ostream_t out)
{
    static const char header[] =
        "Channel    , Status , Type         , Mode            , Nbr of channels\n";
    static const char name_cde[] = "channelsmode";
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;

    int i, n, n_chan = synth->midi_channels;

    /* checks parameters: 	chan1 chan2 .... */
    if(check_channels_arguments(ac, av, out, name_cde) < 0)
    {
        return -1;
    }

    if(ac)
    {
        n = ac; /* prints ac MIDI channels number */
    }
    else
    {
        n = n_chan; /* prints all MIDI channels number */
    }

    /* prints header */
    fluid_ostream_printf(out, header);

    for(i = 0; i < n; i++)
    {
        int basic_chan, mode, val;
        int chan = ac ? atoi(av[i]) : i;
        int result = fluid_synth_get_basic_channel(synth, chan, &basic_chan, &mode, &val);

        if(result == FLUID_OK)
        {
            if(basic_chan != FLUID_FAILED)
            {
                /* This channel is enabled */
                const char *p_basicchan;  /* field basic channel */
                const char *p_mode;  /* field mode */
                const char *p_nbr; /* field Nbr */
                static const char blank[] = "--"; /* field empty */

                if(chan == basic_chan)
                {
                    /* This channel is a basic channel */
                    char nbr[10]; /* field Nbr */
                    FLUID_SNPRINTF(nbr, sizeof(nbr), "nbr:%3d", val);
                    p_nbr = nbr;
                    p_mode = mode_name[mode];
                    p_basicchan = "basic channel";
                }
                else
                {
                    /* This channel is member of a basic channel group */
                    p_basicchan = blank;

                    if(mode & FLUID_CHANNEL_POLY_OFF)
                    {
                        p_mode = "mono";
                    }
                    else
                    {
                        p_mode = "poly";
                    }

                    p_nbr = blank;
                }

                fluid_ostream_printf(out,
                                     "channel:%3d, enabled, %-13s, %-16s, %s\n",
                                     chan,
                                     p_basicchan,
                                     p_mode,
                                     p_nbr);
            }
            else
            {
                fluid_ostream_printf(out, "channel:%3d, disabled\n", chan);
            }
        }
        else
        {
            print_channel_is_outside_count(out, name_cde, chan, n_chan);

            if(i < n - 1)
            {
                fluid_ostream_printf(out, header);
            }
        }
    }

    return 0;
}

/**  commands mono legato mode ***********************************************/
/*
 Prints result message for commands: legatomode, portamentomode.
 @param result result from the command (FLUID_OK,FLUID_FAILED).
 @param out output stream.
 @param name_cde command name prefix.
 @param chan MIDI channel number to display.
 @param name_mode name of the mode to display.
 @param n_chan, number of MIDI channels.
*/
static void print_result_get_channel_mode(int result, fluid_ostream_t out,
        char const *name_cde, int chan,
        char const *name_mode,	int n_chan)
{
    if(result == FLUID_OK)
    {
        fluid_ostream_printf(out, "%s: channel %3d, %s\n", name_cde, chan, name_mode);
    }
    else
    {
        print_channel_is_outside_count(out, name_cde, chan, n_chan);
    }
}

/*-----------------------------------------------------------------------------
 legatomode
     Prints legato mode of all MIDI channels
     example

     channel:  0, (1)multi-retrigger
     channel:  1, (0)retrigger
     channel:  2, (1)multi-retrigger
     .....

 legatomode chan1 chan2
     Prints only legato mode of MIDI channels chan1, chan2
*/
int fluid_handle_legatomode(void *data, int ac, char **av,
                            fluid_ostream_t out)
{
    static const char name_cde[] = "legatomode";
    static const char *const name_legato_mode[FLUID_CHANNEL_LEGATO_MODE_LAST] =
    {	"(0)retrigger", "(1)multi-retrigger"	};

    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int mode = 0;
    int i, n, n_chan = synth->midi_channels;

    /* checks channels arguments: chan1 chan2 .... */
    if(check_channels_arguments(ac, av, out, name_cde) < 0)
    {
        return -1;
    }

    if(ac)
    {
        n = ac; /* prints ac MIDI channels number */
    }
    else
    {
        n = n_chan; /* prints all MIDI channels number */
    }

    /* prints header */
    fluid_ostream_printf(out, "Channel    , legato mode\n");

    for(i = 0; i < n; i++)
    {
        int chan = ac ? atoi(av[i]) : i;
        int result = fluid_synth_get_legato_mode(synth, chan, &mode);
        print_result_get_channel_mode(result, out, name_cde, chan, name_legato_mode[mode], n_chan);
    }

    return 0;
}

/*
 checks channels arguments by group:
 -example by group of 2 arguments: chan1 val1  chan2 val2  .. ..
 -example by group of 4 arguments: chan1 val1 val2 val3   chan2 val1 val2 val3  ....

 all arguments must be numeric.

 @param ac argument count.
 @param av argument table.
 @param nbr_arg_group number of arguments by group expected.
 @param out output stream.
 @param name_cde command name prefix.
 @param nbr_arg_group_msg message when the number of argument by group is invalid.
 @return 0 if arguments are valid, -1 otherwise.
*/
static int check_channels_group_arguments(int ac, char **av, int nbr_arg_group,
        fluid_ostream_t out,
        char const *name_cde,
        char const *nbr_arg_group_msg
                                         )
{
    if(ac)
    {
        /* checks channels numeric arguments */
        if(check_channels_arguments(ac, av, out, name_cde) < 0)
        {
            return -1;
        }

        if(ac % nbr_arg_group)
        {
            /* each group entry needs nbr_arg_group parameters */
            fluid_ostream_printf(out, "%s: channel %d, %s\n", name_cde,
                                 atoi(av[((ac / nbr_arg_group) * nbr_arg_group)]),
                                 nbr_arg_group_msg);
            return -1;
        }
    }
    else
    {
        fluid_ostream_printf(out, "%s: %s", name_cde, nbr_arg_group_msg);
        return -1;
    }

    return 0;
}

/*
 Prints result message for commands: setlegatomode, setportamentomode.
 @param result result from the command (FLUID_FAILED).
 @param out output stream.
 @param name_cde command name prefix.
 @param chan, MIDI channel number to display.
 @param mode, mode value to display.
*/
static void print_result_set_channel_mode(int result, fluid_ostream_t out,
        char const *name_cde,
        int chan, int mode)
{
    if(result == FLUID_FAILED)
    {
        fluid_ostream_printf(out, "%s: channel %3d, mode %3d, %s", name_cde, chan, mode, invalid_arg_msg);
    }
}

static const char too_few_arg_chan_mode_msg[] = "too few argument, chan mode [chan mode]...\n";
/*-----------------------------------------------------------------------------
  setlegatomode chan0 mode1 [chan1 mode0] ..  ..

  Changes legato mode for channels chan0 and [chan1]
*/
int fluid_handle_setlegatomode(void *data, int ac, char **av,
                               fluid_ostream_t out)
{
    static const char name_cde[] = "setlegatomode";
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int i, n ;

    /* checks channels arguments by group of 2: chan1 val1 chan2 val1 ..  ..*/
    if(check_channels_group_arguments(ac, av, 2, out, name_cde, too_few_arg_chan_mode_msg) < 0)
    {
        return -1;
    }

    n = ac / 2; /* number of legato groups information */

    for(i = 0; i < n; i++)
    {
        int chan = atoi(av[(i * 2)]);
        int mode = atoi(av[(i * 2) + 1]);
        /* changes legato mode */

        int result = fluid_synth_set_legato_mode(synth, chan, mode);
        print_result_set_channel_mode(result, out, name_cde, chan, mode);
    }

    return 0;
}

/**  commands mono/poly portamento mode **************************************/

/*-----------------------------------------------------------------------------
 portamentomode
     Prints portamento mode of all MIDI channels
     example

     channel:  0, (2)staccato only
     channel:  1, (1)legato only
     channel:  2, (0)each note
     channel:  3, (1)legato only
     .....

 portamentomode chan1 chan2
     Prints only portamento mode of MIDI channels chan1, chan2
*/
int fluid_handle_portamentomode(void *data, int ac, char **av,
                                fluid_ostream_t out)
{
    static const char name_cde[] = "portamentomode";
    static const char *const name_portamento_mode[FLUID_CHANNEL_PORTAMENTO_MODE_LAST] =
    { "(0)each note", "(1)legato only", "(2)staccato only"	};

    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int mode = 0;
    int i, n, n_chan = synth->midi_channels;

    /* checks channels arguments: chan1 chan2 . . . */
    if(check_channels_arguments(ac, av, out, name_cde) < 0)
    {
        return -1;
    }

    if(ac)
    {
        n = ac; /* prints ac MIDI channels number */
    }
    else
    {
        n = n_chan; /* prints all MIDI channels number */
    }

    /* prints header */
    fluid_ostream_printf(out, "Channel    , portamento mode\n");

    for(i = 0; i < n; i++)
    {
        int chan = ac ? atoi(av[i]) : i;
        int result = fluid_synth_get_portamento_mode(synth, chan, &mode);
        print_result_get_channel_mode(result, out, name_cde, chan, name_portamento_mode[mode], n_chan);
    }

    return 0;
}

/*-----------------------------------------------------------------------------
  setportamentomode chan1 mode1 [chan2 mode2] ..  ..

  Changes portamento mode for channels chan1 and [chan2]
*/
int fluid_handle_setportamentomode(void *data, int ac, char **av,
                                   fluid_ostream_t out)
{
    static const char name_cde[] = "setportamentomode";
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int i, n ;

    /* checks channels arguments by group of 2: chan1 val1 chan2 val1 ..  .. */
    if(check_channels_group_arguments(ac, av, 2, out, name_cde, too_few_arg_chan_mode_msg) < 0)
    {
        return -1;
    }

    n = ac / 2; /* number of portamento groups information */

    for(i = 0; i < n; i++)
    {
        int chan = atoi(av[(i * 2)]);
        int mode = atoi(av[(i * 2) + 1]);
        /* changes portamento mode */

        int result = fluid_synth_set_portamento_mode(synth, chan, mode);
        print_result_set_channel_mode(result, out, name_cde, chan, mode);
    }

    return 0;
}

/**  commands mono/poly breath mode *******************************************/
/*-----------------------------------------------------------------------------
 breathmode
    Prints breath options of all MIDI channels.
	poly breath on/off, mono breath on/off, breath sync on/off

    example

	Channel    , poly breath , mono breath , breath sync
	channel:  0, off         , off         , off
	channel:  1, off         , off         , off
	channel:  2, off         , off         , off
	.....

 breathmode chan1 chan2
     Prints only breath mode of MIDI channels chan1, chan2
*/
int fluid_handle_breathmode(void *data, int ac, char **av,
                            fluid_ostream_t out)
{
    static const char name_cde[] = "breathmode";
    static const char *const header = "Channel    , poly breath , mono breath , breath sync\n";
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int breathmode;
    int i, n, n_chan = synth->midi_channels;

    /* checks channels arguments: chan1 chan2 . . . */
    if(check_channels_arguments(ac, av, out, name_cde) < 0)
    {
        return -1;
    }

    if(ac)
    {
        n = ac; /* prints ac MIDI channels number */
    }
    else
    {
        n = n_chan; /* prints all MIDI channels number */
    }

    /* prints header */
    fluid_ostream_printf(out, header);

    for(i = 0; i < n; i++)
    {
        int chan = ac ? atoi(av[i]) : i;
        int result = fluid_synth_get_breath_mode(synth, chan, &breathmode);

        if(result == FLUID_OK)
        {
            static const char on_msg[] = "on";
            static const char off_msg[] = "off";
            const char *msg_poly_breath, * msg_mono_breath, * msg_breath_sync;

            if(breathmode &  FLUID_CHANNEL_BREATH_POLY)
            {
                msg_poly_breath = on_msg;
            }
            else
            {
                msg_poly_breath = off_msg;
            }

            if(breathmode &  FLUID_CHANNEL_BREATH_MONO)
            {
                msg_mono_breath = on_msg;
            }
            else
            {
                msg_mono_breath = off_msg;
            }

            if(breathmode &  FLUID_CHANNEL_BREATH_SYNC)
            {
                msg_breath_sync = on_msg;
            }
            else
            {
                msg_breath_sync = off_msg;
            }

            fluid_ostream_printf(out, "channel:%3d, %-12s, %-12s, %-11s\n", chan,
                                 msg_poly_breath, msg_mono_breath, msg_breath_sync);
        }
        else
        {
            print_channel_is_outside_count(out, name_cde, chan, n_chan);

            if(i < n - 1)
            {
                fluid_ostream_printf(out, header);
            }
        }
    }

    return 0;
}

/*-----------------------------------------------------------------------------
  setbreathmode chan1 poly_breath_mode(1/0) mono_breath_mode(1/0) mono_breath_sync(1/0)

  Changes breath options for channels chan1 [chan2] ..  ..

  Example:  setbreathmode  4 0 1 1

  Parameter 1 is the channel number (i.e 4).
  Parameter 2 is the " Breath modulator " enable/disable  for poly mode (i.e disabled).
  Parameter 3 is the " Breath modulator " enable/disable for mono mode (i.e enabled).
  Parameter 4 is "breath sync noteOn/Off" enable/disable for mono mode only (i.e enabled).

*/
int fluid_handle_setbreathmode(void *data, int ac, char **av,
                               fluid_ostream_t out)
{
    static const char name_cde[] = "setbreathmode";
    static const char too_few_arg_breath_msg[] =
        "too few argument:\nchan 1/0(breath poly) 1/0(breath mono) 1/0(breath sync mono)[..]\n";

    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;
    int i, n, n_chan = synth->midi_channels;

    /* checks channels arguments by group of 4:
    chan1 val1 val2 val3   chan2 val1 val2 val3 ....  ....*/
    if(check_channels_group_arguments(ac, av, 4, out, name_cde, too_few_arg_breath_msg) < 0)
    {
        return -1;
    }

    n = ac / 4; /* number of breath groups information */

    for(i = 0; i < n; i++)
    {
        int result;
        int chan = atoi(av[(i * 4)]);
        int poly_breath = atoi(av[(i * 4) + 1]);
        int mono_breath = atoi(av[(i * 4) + 2]);
        int breath_sync = atoi(av[(i * 4) + 3]);
        int breath_infos = 0;

        /* changes  breath infos */
        if(poly_breath)
        {
            breath_infos |= FLUID_CHANNEL_BREATH_POLY;
        }

        if(mono_breath)
        {
            breath_infos |= FLUID_CHANNEL_BREATH_MONO;
        }

        if(breath_sync)
        {
            breath_infos |= FLUID_CHANNEL_BREATH_SYNC;
        }

        result = fluid_synth_set_breath_mode(synth, chan, breath_infos);

        if(result == FLUID_FAILED)
        {
            print_channel_is_outside_count(out, name_cde, chan, n_chan);
        }
    }

    return 0;
}

/**  commands  for Midi file player ******************************************/

/* check player argument */
int player_check_arg(const char *name_cde, int ac, char **av, fluid_ostream_t out)
{
    /* check if there is one argument that is a number */
    if(ac != 1 || !fluid_is_number(av[0]))
    {
        fluid_ostream_printf(out, "%s: %s", name_cde, invalid_arg_msg);
        return FLUID_FAILED;
    }
    return FLUID_OK;
}

/* print current position, total ticks, and tempo
 * @current_tick position to display. if -1 the function displays the value
 *  returned by fluid_player_get_current_tick().
*/
void player_print_position(fluid_player_t *player, int current_tick, fluid_ostream_t out)
{
    int total_ticks = fluid_player_get_total_ticks(player);
    int tempo_bpm = fluid_player_get_bpm(player);
    if(current_tick == -1)
    {
        current_tick = fluid_player_get_current_tick(player);
    }
    fluid_ostream_printf(out, "player current pos:%d, end:%d, bpm:%d\n\n",
                         current_tick, total_ticks, tempo_bpm);
}

/* player commands enum */
enum
{
    PLAYER_LOOP_CDE, /* player_loop num,(Set loop number to num) */
    PLAYER_SEEK_CDE, /* player_seek num (Move forward/backward to +/-num ticks) */
    PLAYER_STOP_CDE,      /* player_stop    (Stop playing) */
    PLAYER_CONT_CDE,      /* player_cont    (Continue playing) */
    PLAYER_NEXT_CDE,      /* player_next    (Move to next song) */
    PLAYER_START_CDE      /* player_start   (Move to start of song) */
};

/* Command handler: see player commands enum (above)
 * @cmd player commands enumeration value.
*/
int fluid_handle_player_cde(void *data, int ac, char **av, fluid_ostream_t out, int cmd)
{
    FLUID_ENTRY_COMMAND(data);
    int arg = 0, was_running;
    int seek = -1;  /* current seek position in tick */

    /* commands name table */
    static const char *name_cde[] =
    {"player_loop", "player_seek"};

    /* get argument for PLAYER_LOOP_CDE, PLAYER_SEEK_CDE */
    if(cmd <= PLAYER_SEEK_CDE)
    {
        /* check argument */
        if(player_check_arg(name_cde[cmd], ac, av, out) == FLUID_FAILED)
        {
            return FLUID_FAILED;
        }

        arg = atoi(av[0]);
    }

    if(cmd == PLAYER_LOOP_CDE)  /* player_loop */
    {
        fluid_player_set_loop(handler->player, arg);
        return FLUID_OK;
    }

    if(cmd == PLAYER_CONT_CDE)  /* player_cont */
    {
        fluid_player_play(handler->player);
        return FLUID_OK;
    }

    was_running = fluid_player_get_status(handler->player) == FLUID_PLAYER_PLAYING;
    if(was_running)
    {
        fluid_player_stop(handler->player);  /* player_stop */
    }

    if(cmd != PLAYER_STOP_CDE)
    {
        /* seek for player_next, player_seek, player_start */
        /* set seek to maximum position */
        seek = fluid_player_get_total_ticks(handler->player);

        if(cmd == PLAYER_SEEK_CDE)
        {
            /* Move position forward/backward +/- num ticks*/
            arg  += fluid_player_get_current_tick(handler->player);

            /* keep seek between minimum and maximum in current song */
            if(arg < 0)
            {
                seek = 0; /* minimum position */
            }
            else if(!was_running || arg < seek)
            {
                seek = arg; /* seek < maximum position */
            }
        }

        if(cmd == PLAYER_START_CDE)  /* player_start */
        {
            seek = 0; /* beginning of the current song */
        }

        fluid_player_seek(handler->player, seek);
        if(was_running)
        {
            fluid_player_play(handler->player);
        }
    }
    /* display position */
    player_print_position(handler->player, seek, out);

    return FLUID_OK;
}

/* Command handler for "player_start" command */
int fluid_handle_player_start(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_cde(data, ac, av, out, PLAYER_START_CDE);
}

/* Command handler for "player_stop" command */
int fluid_handle_player_stop(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_cde(data, ac, av, out, PLAYER_STOP_CDE);
}

/* Command handler for "player_continue" command */
int fluid_handle_player_continue(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_cde(data, ac, av, out, PLAYER_CONT_CDE);
}
/* Command handler for "player_seek" command */
int fluid_handle_player_seek(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_cde(data, ac, av, out, PLAYER_SEEK_CDE);
}

/* Command handler for "player_next" command */
int fluid_handle_player_next_song(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_cde(data, ac, av, out, PLAYER_NEXT_CDE);
}

/* Command handler for "player_loop" command */
int fluid_handle_player_loop(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_cde(data, ac, av, out, PLAYER_LOOP_CDE);
}

/* Command handler for player tempo commands:
   player_tempo_int [mul], set the player to internal tempo multiplied by mul
   player_tempo_bpm bpm, set the player to external tempo in beat per minute.
   examples:
    player_tempo_int      set the player to internal tempo with a default
                          multiplier set to 1.0.

    player_tempo_int 0.5  set the player to internal tempo divided by 2.

    player_tempo_bpm 75, set the player to external tempo of 75 beats per minute.
*/
int fluid_handle_player_tempo_cde(void *data, int ac, char **av, fluid_ostream_t out, int cmd)
{
    FLUID_ENTRY_COMMAND(data);
    /* default multiplier for player_tempo_int command without argument*/
    double arg = 1.0F;

    /* commands name table */
    static const char *name_cde[] =
    {"player_tempo_int", "player_tempo_bpm"};

    static const struct /* argument infos */
    {
        double min;
        double max;
        char *name;
    }argument[2] = {{0.1F, 10.F, "multiplier"}, {1.0F, 600.0F, "bpm"}};

    /* get argument for: player_tempo_int [mul],  player_tempo_bpm bpm */
    if((cmd == FLUID_PLAYER_TEMPO_EXTERNAL_BPM) || ac)
    {
        /* check argument presence */
        if(player_check_arg(name_cde[cmd], ac, av, out) == FLUID_FAILED)
        {
            return FLUID_FAILED;
        }

        arg = atof(av[0]);

        /* check if argument is in valid range */
        if(arg < argument[cmd].min || arg > argument[cmd].max)
        {
            fluid_ostream_printf(out, "%s: %s %f must be in range [%f..%f]\n",
                                 name_cde[cmd], argument[cmd].name, arg,
                                 argument[cmd].min, argument[cmd].max);
            return FLUID_FAILED;
        }
    }

    fluid_player_set_tempo(handler->player, cmd, arg);

    return FLUID_OK;
}

/* Command handler for "player_tempo_int [mul]" command */
int fluid_handle_player_tempo_int(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_tempo_cde(data, ac, av, out, FLUID_PLAYER_TEMPO_INTERNAL);
}

/* Command handler for "player_tempo_bpm bmp" command */
int fluid_handle_player_tempo_bpm(void *data, int ac, char **av, fluid_ostream_t out)
{
    return fluid_handle_player_tempo_cde(data, ac, av, out, FLUID_PLAYER_TEMPO_EXTERNAL_BPM);
}

#ifdef LADSPA

#define CHECK_LADSPA_ENABLED(_fx, _out) \
  if (_fx == NULL) \
  { \
    fluid_ostream_printf(_out, "LADSPA is not enabled.\n"); \
    return FLUID_FAILED; \
  }

#define CHECK_LADSPA_INACTIVE(_fx, _out) \
  if (fluid_ladspa_is_active(_fx)) \
  { \
    fluid_ostream_printf(_out, "LADSPA already started.\n"); \
    return FLUID_FAILED; \
  }

#define LADSPA_ERR_LEN (1024)

/**
 * ladspa_start
 */
int fluid_handle_ladspa_start(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;
    char error[LADSPA_ERR_LEN];

    if(ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_start does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if(fluid_ladspa_check(fx, error, LADSPA_ERR_LEN) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Unable to start LADSPA: %s", error);
        return FLUID_FAILED;
    }

    if(fluid_ladspa_activate(fx) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Unable to start LADSPA.\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/**
 * ladspa_stop
 */
int fluid_handle_ladspa_stop(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if(ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_stop does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);

    if(!fluid_ladspa_is_active(fx))
    {
        fluid_ostream_printf(out, "LADSPA has not been started.\n");
    }

    if(fluid_ladspa_deactivate(fx) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Unable to stop LADSPA.\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/**
 * ladspa_reset
 */
int fluid_handle_ladspa_reset(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if(ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_reset does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);

    fluid_ladspa_reset(fx);

    return FLUID_OK;
}

/**
 * ladspa_check
 */
int fluid_handle_ladspa_check(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;
    char error[LADSPA_ERR_LEN];

    if(ac != 0)
    {
        fluid_ostream_printf(out, "ladspa_reset does not accept any arguments\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);

    if(fluid_ladspa_check(fx, error, LADSPA_ERR_LEN) != FLUID_OK)
    {
        fluid_ostream_printf(out, "LADSPA check failed: %s", error);
        return FLUID_FAILED;
    }

    fluid_ostream_printf(out, "LADSPA check ok\n");

    return FLUID_OK;
}

/**
 * ladspa_set <effect> <port> <value>
 */
int fluid_handle_ladspa_set(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if(ac != 3)
    {
        fluid_ostream_printf(out, "ladspa_set needs three arguments: <effect> <port> <value>\n");
        return FLUID_FAILED;
    };

    CHECK_LADSPA_ENABLED(fx, out);

    /* Redundant check, just here to give a more detailed error message */
    if(!fluid_ladspa_effect_port_exists(fx, av[0], av[1]))
    {
        fluid_ostream_printf(out, "Port '%s' not found on effect '%s'\n", av[1], av[0]);
        return FLUID_FAILED;
    }

    if(fluid_ladspa_effect_set_control(fx, av[0], av[1], atof(av[2])) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to set port '%s' on effect '%s', "
                             "maybe it is not a control port?\n", av[1], av[0]);
        return FLUID_FAILED;
    }

    return FLUID_OK;
};

/**
 * ladspa_buffer <name>
 */
int fluid_handle_ladspa_buffer(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if(ac != 1)
    {
        fluid_ostream_printf(out, "ladspa_buffer needs one argument: <name>\n");
        return FLUID_FAILED;
    };

    CHECK_LADSPA_ENABLED(fx, out);

    CHECK_LADSPA_INACTIVE(fx, out);

    if(fluid_ladspa_add_buffer(fx, av[0]) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to add buffer\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
};

/**
 * ladspa_effect <name> <library> [plugin] [--mix [gain]]
 */
int fluid_handle_ladspa_effect(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;
    char *plugin_name = NULL;
    int pos;
    int mix = FALSE;
    float gain = 1.0f;

    if(ac < 2 || ac > 5)
    {
        fluid_ostream_printf(out, "ladspa_effect invalid arguments: "
                             "<name> <library> [plugin] [--mix [gain]]\n");
        return FLUID_FAILED;
    }

    pos = 2;

    /* If the first optional arg is not --mix, then it must be the plugin label */
    if((pos < ac) && (FLUID_STRCMP(av[pos], "--mix") != 0))
    {
        plugin_name = av[pos];
        pos++;
    }

    /* If this optional arg is --mix and there's an argument after it, that that
     * must be the gain */
    if((pos < ac) && (FLUID_STRCMP(av[pos], "--mix") == 0))
    {
        mix = TRUE;

        if(pos + 1 < ac)
        {
            gain = atof(av[pos + 1]);
        }
    }

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if(fluid_ladspa_add_effect(fx, av[0], av[1], plugin_name) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to create effect\n");
        return FLUID_FAILED;
    }

    if(mix)
    {
        if(!fluid_ladspa_effect_can_mix(fx, av[0]))
        {
            fluid_ostream_printf(out, "Effect '%s' does not support --mix mode\n", av[0]);
            return FLUID_FAILED;
        }

        if(fluid_ladspa_effect_set_mix(fx, av[0], mix, gain) != FLUID_OK)
        {
            fluid_ostream_printf(out, "Failed to set --mix mode\n");
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

/*
 * ladspa_link <effect> <port> <buffer or host port>
 */
int fluid_handle_ladspa_link(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_ladspa_fx_t *fx = handler->synth->ladspa_fx;

    if(ac != 3)
    {
        fluid_ostream_printf(out, "ladspa_link needs 3 arguments: "
                             "<effect> <port> <buffer or host name>\n");
        return FLUID_FAILED;
    }

    CHECK_LADSPA_ENABLED(fx, out);
    CHECK_LADSPA_INACTIVE(fx, out);

    if(!fluid_ladspa_effect_port_exists(fx, av[0], av[1]))
    {
        fluid_ostream_printf(out, "Port '%s' not found on effect '%s'\n", av[1], av[0]);
        return FLUID_FAILED;
    }

    if(!fluid_ladspa_host_port_exists(fx, av[2]) && !fluid_ladspa_buffer_exists(fx, av[2]))
    {
        fluid_ostream_printf(out, "Host port or buffer '%s' not found.\n", av[2]);
        return FLUID_FAILED;
    }

    if(fluid_ladspa_effect_link(fx, av[0], av[1], av[2]) != FLUID_OK)
    {
        fluid_ostream_printf(out, "Failed to link port\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

#endif /* LADSPA */

#if WITH_PROFILING
/*
* Locks profile command to prevent simultaneous changes by an other shell
* (may be a server shell (tcp)).
*
* @return FLUID_OK if success , otherwise FLUID_FAILED.
*/
static int fluid_profile_lock_command(fluid_ostream_t out)
{
    if(! fluid_atomic_int_compare_and_exchange(&fluid_profile_lock, 0, 1))
    {
        fluid_ostream_printf(out,
                             "profile command in use in another shell. Try later!\n");
        return FLUID_FAILED;
    }

    return FLUID_OK;
}

/*
* Unlocks profile command.
*/
static void fluid_profile_unlock_command(void)
{
    fluid_atomic_int_set(&fluid_profile_lock, 0);
}

/*
* command: profile
*
* Prints default parameters used by prof_start command.
*
* Notes:0, bank:0, prog:16, print:0, n_prof:1, dur:500 ms.
*
* @return FLUID_OK if success , otherwise FLUID_FAILED.
*/
int
fluid_handle_profile(void *data, int ac, char **av, fluid_ostream_t out)
{
    /* locks to prevent simultaneous changes by an other shell */
    /* (may be a server shell (tcp)) */
    if(fluid_profile_lock_command(out) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    /* prints default parameters */
    fluid_ostream_printf(out,
                         " Notes:%d, bank:%d, prog:%d, print:%d, n_prof:%d, dur:%d ms\n",
                         fluid_profile_notes, fluid_profile_bank, fluid_profile_prog,
                         fluid_profile_print,
                         fluid_profile_n_prof, fluid_profile_dur);

    /* unlocks */
    fluid_profile_unlock_command();
    return FLUID_OK;
}

/*
* command: prof_set_notes  nbr  [bank prog]
*
* Sets notes number generated by prof_start command.
*
* nbr: notes numbers (generated during command "prof_start").
* bank, prog: preset bank and program number (default value if not specified).
*
* @return FLUID_OK if success , otherwise FLUID_FAILED.
*/
int
fluid_handle_prof_set_notes(void *data, int ac, char **av, fluid_ostream_t out)
{
    unsigned short nbr;  /* previous parameters */
    unsigned char bank, prog;  /* previous parameters */
    int r; /* return */

    /* locks to prevent simultaneous changes by an other shell  */
    if(fluid_profile_lock_command(out) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    /* checks parameters */
    if(ac < 1)
    {
        fluid_ostream_printf(out, "profile_notes: too few arguments\n");
        fluid_profile_unlock_command();
        return FLUID_FAILED;
    }

    /* gets default parameters */
    nbr = fluid_profile_notes, bank = fluid_profile_bank;
    prog = fluid_profile_prog;

    r = fluid_is_number(av[0]);

    if(r)
    {
        /* checks nbr */
        nbr = atoi(av[0]);	/* get nbr parameter */

        if(ac >= 2)
        {
            /* [bank prog] are optional */
            if(ac >= 3)
            {
                r = fluid_is_number(av[1]) && fluid_is_number(av[2]);

                if(r)
                {
                    bank = atoi(av[1]);	/* gets bank parameter */
                    prog = atoi(av[2]);	/* gets prog parameter */
                }
            }
            else
            {
                /* prog is needed */
                fluid_ostream_printf(out, "profile_set_notes: too few arguments\n");
                fluid_profile_unlock_command();
                return FLUID_FAILED;
            }
        }
    }

    if(!r)
    {
        fluid_ostream_printf(out, "profile_set_notes: invalid argument\n");
        fluid_profile_unlock_command();
        return FLUID_FAILED;
    }

    /* Saves new parameters */
    fluid_profile_notes = nbr;
    fluid_profile_bank = bank;
    fluid_profile_prog = prog;

    /* unlocks */
    fluid_profile_unlock_command();
    return FLUID_OK;
}

/*
* command: prof_set_print  mode
*
* The command sets the print mode.
*
* mode: result print mode(used by prof_start").
*       0: simple printing, >0: full printing
*
* @return FLUID_OK if success , otherwise FLUID_FAILED.
*/
int
fluid_handle_prof_set_print(void *data, int ac, char **av, fluid_ostream_t out)
{
    int r;

    /* locks to prevent simultaneous changes by an other shell  */
    if(fluid_profile_lock_command(out) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    /* checks parameters */
    if(ac < 1)
    {
        fluid_ostream_printf(out, "profile_set_print: too few arguments\n");
        fluid_profile_unlock_command();
        return FLUID_FAILED;
    }

    /* gets parameters */
    if(fluid_is_number(av[0]))
    {
        /* checks and gets mode */
        fluid_profile_print =  atoi(av[0]);	/* gets and saves mode parameter */
        r = FLUID_OK;
    }
    else
    {
        fluid_ostream_printf(out, "profile_set_print: invalid argument\n");
        r = FLUID_FAILED;
    }

    /* unlocks */
    fluid_profile_unlock_command();
    return r;
}

/*
* Generates simultaneous notes for precise profiling.
*
* @param synth, synthesizer instance.
* @param notes, the number of notes to generate.
* @param bank, prog, soundfont bank preset number used.
* @param out, stream output device.
* @return the number of voices generated. It can be lower than notes number
*  when the preset have instrument only on few key range.
*/
static unsigned short fluid_profile_send_notes(fluid_synth_t *synth, int notes,
        int bank, int prog,
        fluid_ostream_t out)
{
    int n;         /* number of notes generated */
    int n_voices, n_actives = 0; /* Maximum voices, voices generated */
    int n_chan, chan, key ;
    /* MIDI channels count and maximum polyphony */
    n_chan = fluid_synth_count_midi_channels(synth); /* channels count */
    n_voices = fluid_synth_get_polyphony(synth); /* maximum voices */
    /* */
    fluid_ostream_printf(out, "Generating %d notes, ", notes);

    for(n = 0, key = FLUID_PROFILE_LAST_KEY + 1, chan = -1; n < notes; n++, key++)
    {
        if(key > FLUID_PROFILE_LAST_KEY)
        {
            /* next channel */
            chan++;

            if(chan >= n_chan)
            {
                break;    /* stops generation */
            }

            /* select preset */
            fluid_synth_bank_select(synth, chan, bank);
            fluid_synth_program_change(synth, chan, prog);
            key = FLUID_PROFILE_FIRST_KEY;
        }

        fluid_synth_noteon(synth, chan, key, FLUID_PROFILE_DEFAULT_VEL);
        n_actives = fluid_synth_get_active_voice_count(synth); /* running voices */

        if(n_actives >= n_voices)
        {
            fluid_ostream_printf(out, "max polyphony reached:%d, ", n_voices);
            break;  /* stops notes generation */
        }
    }

    fluid_ostream_printf(out, "generated voices:%d\n", n_actives);
    return n_actives;
}

/*
* command: prof_start  [n_prof   [dur] ]
*
* Starts n_prof measures of dur duration(ms) each.
*
* n_prof  number of measures (default value if not specified).
* dur: measure duration (ms) (default value if not specified).
*
* The result of each measure is displayed.
*
* Note: The command ends when the last measure ends or when the user
* cancels the command using <ENTER> key (cancellation using <ENTER>
* is implemented using FLUID_PROFILE_CANCEL macro in fluid_sys.h).
*
* @return FLUID_OK if success , otherwise FLUID_FAILED.
*/
int
fluid_handle_prof_start(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_synth_t *synth = handler->synth;

    unsigned short n_prof, n, dur; /* previous parameters */
    unsigned int total_dur, rem_dur; /* total and remainder duration (ms) */
    unsigned short notes; /* notes number to generate */
    int n_actives = 0;  /* actives voices */
    float gain; /* current gain */
    int r = 1;	/* checking parameter result */

    /* Locks to prevent simultaneous command by an other shell */
    if(fluid_profile_lock_command(out) == FLUID_FAILED)
    {
        return FLUID_FAILED;
    }

    /* Gets previous parameters values */
    n_prof = fluid_profile_n_prof;  /* number of measuses */
    dur = fluid_profile_dur;        /* duration of one measure (ms) */

    /* check parameters */
    if(ac >= 1)
    {
        r = fluid_is_number(av[0]);

        if(r)
        {
            n_prof = atoi(av[0]);	/* gets n_prof parameter */

            if(ac >= 2)
            {
                r = fluid_is_number(av[1]);

                if(r)
                {
                    dur = atoi(av[1]);/* gets dur parameter */
                }
            }
        }
    }

    if(!r || n_prof < 1 || dur < 1)
    {
        fluid_ostream_printf(out, "profile_start: invalid argument\n");
        fluid_profile_unlock_command();
        return FLUID_FAILED;
    }

    /* Saves new parameters */
    fluid_profile_n_prof = n_prof;
    fluid_profile_dur = dur;

    /* Saves current gain */
    gain = fluid_synth_get_gain(synth);

    /* Generates notes if any */
    notes =  fluid_profile_notes;

    if(notes)
    {
        /* checks if the synth is playing */
        /* Warn the user */
        if(fluid_synth_get_active_voice_count(synth))
        {
            fluid_ostream_printf(out,
                                 "Warning: can't generate notes, please stop any playing\n");
        }
        else
        {
            float send_gain;
            /* sets low gain before sending notes */
            fluid_synth_set_gain(synth, 0.01);
            /* sends notes */
            n_actives = fluid_profile_send_notes(synth, notes, fluid_profile_bank,
                                                 fluid_profile_prog, out);
            /* compensates gain to avoid a loud sound */
            send_gain =  1.0 * pow(10, (n_actives * FLUID_PROFILE_VOICE_ATTEN) / 20);
            fluid_synth_set_gain(synth, send_gain);

            /* Before starting profiling immediately we wait to ensures that voices are
               currently synthesized by audio rendering API. This ensure that macro
               probes will register the expected number of actives voices.
            */
            fluid_msleep(200); /* wait 200 ms */
        }
    }

    /* Starts - waits - prints n_prof measures */
    fluid_ostream_printf(out, "Number of measures(n_prof):%d, duration of one measure(dur):%dms\n",
                         n_prof, dur);

    /* Clears any previous <ENTER> pending key */
    fluid_profile_is_cancel_req();

    total_dur = rem_dur = n_prof * dur;

    for(n = 0 ; n < n_prof; rem_dur -= dur, n++)
    {
        unsigned int end_ticks;/* ending position (in ticks) */
        unsigned int tm, ts, rm, rs;
        int status;
        ts = total_dur / 1000;
        tm = ts / 60;
        ts = ts % 60;  /* total minutes and seconds */

        rs = rem_dur / 1000;
        rm = rs / 60;
        rs = rs % 60;  /* remainder minutes and seconds */

        /* Prints total and remainder duration */
#ifdef FLUID_PROFILE_CANCEL
        fluid_ostream_printf(out,
                             "\nProfiling time(mm:ss): Total=%d:%d  Remainder=%d:%d, press <ENTER> to cancel\n",
                             tm, ts, rm, rs);
#else
        fluid_ostream_printf(out,
                             "\nProfiling time(mm:ss): Total=%d:%d  Remainder=%d:%d\n",
                             tm, ts, rm, rs);
#endif

        /* converts duration(ms) in end position in ticks. */
        end_ticks = fluid_atomic_int_get(&synth->ticks_since_start) +
                    dur * synth->sample_rate / 1000;
        /* requests to start the measurement in audio rendering API */
        fluid_profile_start_stop(end_ticks, n);

        /* waits while running */
        do
        {
            /* passive waiting */
            fluid_msleep(500); /* wait 500 ms */
            status = fluid_profile_get_status();
        }
        while(status  == PROFILE_RUNNING);

        /* checks if data are ready */
        if(status == PROFILE_READY)
        {
            /* profiling data are ready, prints profile data */
            fluid_profiling_print_data(synth->sample_rate, out);
        }
        /* checks if the measurement has been cancelled */
        else if(status == PROFILE_CANCELED || status == PROFILE_STOP)
        {
            fluid_ostream_printf(out, "Profiling cancelled.\n");
            break; /* cancel the command */
        }
    }

    /* Stops voices if any had been generated */
    if(n_actives)
    {
        fluid_ostream_printf(out, "Stopping %d voices...", n_actives);
        fluid_synth_system_reset(synth);

        /* waits until all voices become inactives */
        do
        {
            fluid_msleep(10);   /* wait 10 ms */
            n_actives = fluid_synth_get_active_voice_count(synth);
        }
        while(n_actives);

        fluid_ostream_printf(out, "voices stopped.\n");
    }

    /* Restores initial gain */
    fluid_synth_set_gain(synth, gain);

    /* Unlocks */
    fluid_profile_unlock_command();
    return FLUID_OK;
}
#endif /* WITH_PROFILING */

int
fluid_is_number(char *a)
{
    while(*a != 0)
    {
        if(((*a < '0') || (*a > '9')) && (*a != '-') && (*a != '+') && (*a != '.'))
        {
            return FALSE;
        }

        a++;
    }

    return TRUE;
}

char *
fluid_expand_path(char *path, char *new_path, int len)
{
#if defined(_WIN32) || defined(MACOS9)
    FLUID_SNPRINTF(new_path, len - 1, "%s", path);
#else

    if((path[0] == '~') && (path[1] == '/'))
    {
        char *home = getenv("HOME");

        if(home == NULL)
        {
            FLUID_SNPRINTF(new_path, len - 1, "%s", path);
        }
        else
        {
            FLUID_SNPRINTF(new_path, len - 1, "%s%s", home, &path[1]);
        }
    }
    else
    {
        FLUID_SNPRINTF(new_path, len - 1, "%s", path);
    }

#endif

    new_path[len - 1] = 0;
    return new_path;
}



/*
 * Command
 */

fluid_cmd_t *fluid_cmd_copy(const fluid_cmd_t *cmd)
{
    fluid_cmd_t *copy = FLUID_NEW(fluid_cmd_t);

    if(copy == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "Out of memory");
        return NULL;
    }

    copy->name = FLUID_STRDUP(cmd->name);
    copy->topic = FLUID_STRDUP(cmd->topic);
    copy->help = FLUID_STRDUP(cmd->help);
    copy->handler = cmd->handler;
    return copy;
}

void delete_fluid_cmd(fluid_cmd_t *cmd)
{
    fluid_return_if_fail(cmd != NULL);
    FLUID_FREE(cmd->name);
    FLUID_FREE(cmd->topic);
    FLUID_FREE(cmd->help);
    FLUID_FREE(cmd);
}

/*
 * Command handler
 */

static void
fluid_cmd_handler_destroy_hash_value(void *value)
{
    delete_fluid_cmd((fluid_cmd_t *)value);
}

/**
 * Create a new command handler.
 *
 * See new_fluid_cmd_handler2() for more information.
 */
fluid_cmd_handler_t *new_fluid_cmd_handler(fluid_synth_t *synth, fluid_midi_router_t *router)
{
    return new_fluid_cmd_handler2(fluid_synth_get_settings(synth), synth, router, NULL);
}

/**
 * Create a new command handler.
 *
 * @param settings If not NULL, all the settings related commands will be added to the new handler. The @p settings
 * object must be the same as the one you used for creating the @p synth and @p router. Otherwise the
 * behaviour is undefined.
 * @param synth If not NULL, all the default synthesizer commands will be added to the new handler.
 * @param router If not NULL, all the default midi_router commands will be added to the new handler.
 * @param player If not NULL, all the default midi file player commands will be added to the new handler.
 * @return New command handler, or NULL if alloc failed
 */
fluid_cmd_handler_t *new_fluid_cmd_handler2(fluid_settings_t *settings,
                                            fluid_synth_t *synth,
                                            fluid_midi_router_t *router,
                                            fluid_player_t *player)
{
    unsigned int i;
    fluid_cmd_handler_t *handler;

    handler = FLUID_NEW(fluid_cmd_handler_t);

    if(handler == NULL)
    {
        return NULL;
    }

    FLUID_MEMSET(handler, 0, sizeof(*handler));

    handler->commands = new_fluid_hashtable_full(fluid_str_hash, fluid_str_equal,
                        NULL, fluid_cmd_handler_destroy_hash_value);

    if(handler->commands == NULL)
    {
        FLUID_FREE(handler);
        return NULL;
    }

    handler->settings = settings;
    handler->synth = synth;
    handler->router = router;
    handler->player = player;

    for(i = 0; i < FLUID_N_ELEMENTS(fluid_commands); i++)
    {
        const fluid_cmd_t *cmd = &fluid_commands[i];
        int is_settings_cmd = FLUID_STRCMP(cmd->topic, "settings") == 0;
        int is_router_cmd = FLUID_STRCMP(cmd->topic, "router") == 0;
        int is_player_cmd = FLUID_STRCMP(cmd->topic, "player") == 0;
        int is_synth_cmd = !(is_settings_cmd || is_router_cmd || is_player_cmd);

        int no_cmd = is_settings_cmd && settings == NULL;   /* no settings command */
        no_cmd = no_cmd || (is_router_cmd && router == NULL); /* no router command */
        no_cmd = no_cmd || (is_player_cmd && player == NULL); /* no player command */
        no_cmd = no_cmd || (is_synth_cmd && synth == NULL);   /* no synth command */

        if(no_cmd)
        {
            /* register a no-op command, this avoids an unknown command error later on */
            fluid_cmd_t noop = *cmd;
            noop.handler = NULL;
            fluid_cmd_handler_register(handler, &noop);
        }
        else
        {
            fluid_cmd_handler_register(handler, cmd);
        }
    }

    return handler;
}

/**
 * Delete a command handler.
 *
 * @param handler Command handler to delete
 */
void
delete_fluid_cmd_handler(fluid_cmd_handler_t *handler)
{
    fluid_return_if_fail(handler != NULL);

    delete_fluid_hashtable(handler->commands);
    FLUID_FREE(handler);
}

/**
 * Register a new command to the handler.
 *
 * @param handler Command handler instance
 * @param cmd Command info (gets copied)
 * @return #FLUID_OK if command was inserted, #FLUID_FAILED otherwise
 */
int
fluid_cmd_handler_register(fluid_cmd_handler_t *handler, const fluid_cmd_t *cmd)
{
    fluid_cmd_t *copy = fluid_cmd_copy(cmd);
    fluid_hashtable_insert(handler->commands, copy->name, copy);
    return FLUID_OK;
}

/**
 * Unregister a command from a command handler.
 *
 * @param handler Command handler instance
 * @param cmd Name of the command
 * @return TRUE if command was found and unregistered, FALSE otherwise
 */
int
fluid_cmd_handler_unregister(fluid_cmd_handler_t *handler, const char *cmd)
{
    return fluid_hashtable_remove(handler->commands, cmd);
}

int
fluid_cmd_handler_handle(void *data, int ac, char **av, fluid_ostream_t out)
{
    FLUID_ENTRY_COMMAND(data);
    fluid_cmd_t *cmd;

    cmd = fluid_hashtable_lookup(handler->commands, av[0]);

    if(cmd)
    {
        if(cmd->handler)
        {
            return (*cmd->handler)(handler, ac - 1, av + 1, out);
        }
        else
        {
            /* no-op command */
            return 1;
        }
    }
    else
    {
        fluid_ostream_printf(out, "unknown command: %s (try help)\n", av[0]);
        return FLUID_FAILED;
    }
}


#ifdef NETWORK_SUPPORT

struct _fluid_server_t
{
    fluid_server_socket_t *socket;
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_midi_router_t *router;
    fluid_player_t *player;
    fluid_list_t *clients;
    fluid_mutex_t mutex;
};

static void fluid_server_close(fluid_server_t *server)
{
    fluid_list_t *list;
    fluid_list_t *clients;
    fluid_client_t *client;

    fluid_return_if_fail(server != NULL);

    fluid_mutex_lock(server->mutex);
    clients = server->clients;
    server->clients = NULL;
    fluid_mutex_unlock(server->mutex);

    list = clients;

    while(list)
    {
        client = fluid_list_get(list);
        fluid_client_quit(client);
        list = fluid_list_next(list);
    }

    delete_fluid_list(clients);

    if(server->socket)
    {
        delete_fluid_server_socket(server->socket);
        server->socket = NULL;
    }
}

static int
fluid_server_handle_connection(fluid_server_t *server, fluid_socket_t client_socket, char *addr)
{
    fluid_client_t *client;

    client = new_fluid_client(server, server->settings, client_socket);

    if(client == NULL)
    {
        return -1;
    }

    fluid_server_add_client(server, client);

    return 0;
}

void fluid_server_add_client(fluid_server_t *server, fluid_client_t *client)
{
    fluid_mutex_lock(server->mutex);
    server->clients = fluid_list_append(server->clients, client);
    fluid_mutex_unlock(server->mutex);
}

void fluid_server_remove_client(fluid_server_t *server, fluid_client_t *client)
{
    fluid_mutex_lock(server->mutex);
    server->clients = fluid_list_remove(server->clients, client);
    fluid_mutex_unlock(server->mutex);
}

struct _fluid_client_t
{
    fluid_server_t *server;
    fluid_settings_t *settings;
    fluid_cmd_handler_t *handler;
    fluid_socket_t socket;
    fluid_thread_t *thread;
};


static fluid_thread_return_t fluid_client_run(void *data)
{
    fluid_shell_t shell;
    fluid_client_t *client = (fluid_client_t *)data;

    fluid_shell_init(&shell,
                     client->settings,
                     client->handler,
                     fluid_socket_get_istream(client->socket),
                     fluid_socket_get_ostream(client->socket));
    fluid_shell_run(&shell);
    fluid_server_remove_client(client->server, client);
    delete_fluid_client(client);

    return FLUID_THREAD_RETURN_VALUE;
}


fluid_client_t *
new_fluid_client(fluid_server_t *server, fluid_settings_t *settings, fluid_socket_t sock)
{
    fluid_client_t *client;

    client = FLUID_NEW(fluid_client_t);

    if(client == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    client->server = server;
    client->socket = sock;
    client->settings = settings;
    client->handler = new_fluid_cmd_handler2(fluid_synth_get_settings(server->synth),
                                             server->synth, server->router,
                                             server->player);
    client->thread = new_fluid_thread("client", fluid_client_run, client,
                                      0, FALSE);

    if(client->handler == NULL || client->thread == NULL)
    {
        goto error_recovery;
    }

    return client;

error_recovery:
    FLUID_LOG(FLUID_ERR, "Out of memory");
    delete_fluid_client(client);
    return NULL;

}

void fluid_client_quit(fluid_client_t *client)
{
    fluid_socket_close(client->socket);

    FLUID_LOG(FLUID_DBG, "fluid_client_quit: joining");
    fluid_thread_join(client->thread);
    FLUID_LOG(FLUID_DBG, "fluid_client_quit: done");
}

void delete_fluid_client(fluid_client_t *client)
{
    fluid_return_if_fail(client != NULL);

    delete_fluid_cmd_handler(client->handler);
    fluid_socket_close(client->socket);
    delete_fluid_thread(client->thread);

    FLUID_FREE(client);
}

#endif /* NETWORK_SUPPORT */

/**
 * Create a new TCP/IP command shell server.
 *
 * See new_fluid_server2() for more information.
 */
fluid_server_t *
new_fluid_server(fluid_settings_t *settings,
                 fluid_synth_t *synth, fluid_midi_router_t *router)
{
    return new_fluid_server2(settings, synth, router, NULL);
}

/**
 * Create a new TCP/IP command shell server.
 *
 * @param settings Settings instance to use for the shell
 * @param synth If not NULL, the synth instance for the command handler to be used by the client
 * @param router If not NULL, the midi_router instance for the command handler to be used by the client
 * @param player If not NULL, the player instance for the command handler to be used by the client
 * @return New shell server instance or NULL on error
 */
fluid_server_t *
new_fluid_server2(fluid_settings_t *settings,
                 fluid_synth_t *synth, fluid_midi_router_t *router,
                 fluid_player_t *player)
{
#ifdef NETWORK_SUPPORT
    fluid_server_t *server;
    int port;

    server = FLUID_NEW(fluid_server_t);

    if(server == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    server->settings = settings;
    server->clients = NULL;
    server->synth = synth;
    server->router = router;
    server->player = player;

    fluid_mutex_init(server->mutex);

    fluid_settings_getint(settings, "shell.port", &port);

    server->socket = new_fluid_server_socket(port,
                     (fluid_server_func_t) fluid_server_handle_connection,
                     server);

    if(server->socket == NULL)
    {
        FLUID_FREE(server);
        return NULL;
    }

    return server;
#else
    FLUID_LOG(FLUID_WARN, "Network support disabled on this platform.");
    return NULL;
#endif
}

/**
 * Delete a TCP/IP shell server.
 *
 * @param server Shell server instance
 */
void
delete_fluid_server(fluid_server_t *server)
{
#ifdef NETWORK_SUPPORT
    fluid_return_if_fail(server != NULL);

    fluid_server_close(server);

    FLUID_FREE(server);
#endif
}

/**
 * Join a shell server thread (wait until it quits).
 *
 * @param server Shell server instance
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int fluid_server_join(fluid_server_t *server)
{
#ifdef NETWORK_SUPPORT
    return fluid_server_socket_join(server->socket);
#else
    return FLUID_FAILED;
#endif
}
