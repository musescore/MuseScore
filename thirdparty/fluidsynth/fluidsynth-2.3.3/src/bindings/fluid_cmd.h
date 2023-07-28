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

#ifndef _FLUID_CMD_H
#define _FLUID_CMD_H

#include "fluid_sys.h"


void fluid_shell_settings(fluid_settings_t *settings);


/** some help functions */
int fluid_is_number(char *a);
char *fluid_expand_path(char *path, char *new_path, int len);

/** the handlers for the command lines */
int fluid_handle_help(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_quit(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_noteon(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_noteoff(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_pitch_bend(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_pitch_bend_range(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_cc(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_prog(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_select(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_inst(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_channels(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_load(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_unload(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reload(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_fonts(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reverbpreset(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reverbsetroomsize(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reverbsetdamp(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reverbsetwidth(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reverbsetlevel(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_chorusnr(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_choruslevel(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_chorusspeed(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_chorusdepth(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_chorus(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reverb(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_gain(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_interp(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_interpc(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_tuning(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_tune(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_settuning(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_resettuning(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_tunings(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_dumptuning(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_reset(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_source(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_echo(void *data, int ac, char **av, fluid_ostream_t out);

int fluid_handle_set(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_get(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_info(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_settings(void *data, int ac, char **av, fluid_ostream_t out);

int fluid_handle_router_clear(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_router_default(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_router_begin(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_router_end(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_router_chan(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_router_par1(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_router_par2(void *data, int ac, char **av, fluid_ostream_t out);

int fluid_handle_player_start(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_stop(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_continue(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_next_song(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_seek(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_loop(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_tempo_bpm(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_player_tempo_int(void *data, int ac, char **av, fluid_ostream_t out);

#if WITH_PROFILING
int fluid_handle_profile(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_prof_set_notes(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_prof_set_print(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_prof_start(void *data, int ac, char **av, fluid_ostream_t out);
#endif

int fluid_handle_basicchannels(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_resetbasicchannels(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_setbasicchannels(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_channelsmode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_legatomode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_setlegatomode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_portamentomode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_setportamentomode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_breathmode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_setbreathmode(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_sleep(void *data, int ac, char **av, fluid_ostream_t out);

#ifdef LADSPA
int fluid_handle_ladspa_effect(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_link(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_buffer(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_set(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_check(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_start(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_stop(void *data, int ac, char **av, fluid_ostream_t out);
int fluid_handle_ladspa_reset(void *data, int ac, char **av, fluid_ostream_t out);
#endif


/**
 * Command handler function prototype.
 * @param data User defined data
 * @param ac Argument count
 * @param av Array of string arguments
 * @param out Output stream to send response to
 * @return Should return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
typedef int (*fluid_cmd_func_t)(void *data, int ac, char **av, fluid_ostream_t out);

/**
 * Shell command information structure.
 */
typedef struct
{
    char *name;                           /**< The name of the command, as typed in the shell */
    char *topic;                          /**< The help topic group of this command */
    fluid_cmd_func_t handler;             /**< Pointer to the handler for this command */
    char *help;                           /**< A help string */
} fluid_cmd_t;

fluid_cmd_t *fluid_cmd_copy(const fluid_cmd_t *cmd);
void delete_fluid_cmd(fluid_cmd_t *cmd);

int fluid_cmd_handler_handle(void *data,
                             int ac, char **av,
                             fluid_ostream_t out);

int fluid_cmd_handler_register(fluid_cmd_handler_t *handler, const fluid_cmd_t *cmd);
int fluid_cmd_handler_unregister(fluid_cmd_handler_t *handler, const char *cmd);


void fluid_server_remove_client(fluid_server_t *server, fluid_client_t *client);
void fluid_server_add_client(fluid_server_t *server, fluid_client_t *client);


fluid_client_t *new_fluid_client(fluid_server_t *server,
                                 fluid_settings_t *settings,
                                 fluid_socket_t sock);

void delete_fluid_client(fluid_client_t *client);
void fluid_client_quit(fluid_client_t *client);


#endif /* _FLUID_CMD_H */
