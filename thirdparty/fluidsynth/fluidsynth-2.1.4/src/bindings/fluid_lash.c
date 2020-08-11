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
#include "fluid_lash.h"

#ifdef HAVE_LASH

static void fluid_lash_save(fluid_synth_t *synth);
static void fluid_lash_load(fluid_synth_t *synth, const char *filename);
static void *fluid_lash_run(void *data);

/*
 * lash client - this symbol needs to be in the library else
 * all clients would need a fluid_lash_client symbol.
 */
lash_client_t *fluid_lash_client;

static pthread_t fluid_lash_thread;


fluid_lash_args_t *
fluid_lash_extract_args(int *pargc, char  ***pargv)
{
    return lash_extract_args(pargc, pargv);
}

int
fluid_lash_connect(fluid_lash_args_t *args)
{
    fluid_lash_client = lash_init(args, PACKAGE, LASH_Config_Data_Set | LASH_Terminal, LASH_PROTOCOL(2, 0));
    return fluid_lash_client && lash_enabled(fluid_lash_client);
}

void
fluid_lash_create_thread(fluid_synth_t *synth)
{
    pthread_create(&fluid_lash_thread, NULL, fluid_lash_run, synth);
}

static void
fluid_lash_save(fluid_synth_t *synth)
{
    int i;
    int sfcount;
    fluid_sfont_t *sfont;
    lash_config_t *config;
    char num[32];

    sfcount = fluid_synth_sfcount(synth);

    config = lash_config_new();
    lash_config_set_key(config, "soundfont count");
    lash_config_set_value_int(config, sfcount);
    lash_send_config(fluid_lash_client, config);

    for(i = sfcount - 1; i >= 0; i--)
    {
        sfont = fluid_synth_get_sfont(synth, i);
        config = lash_config_new();

        sprintf(num, "%d", i);

        lash_config_set_key(config, num);
        lash_config_set_value_string(config, sfont->get_name(sfont));

        lash_send_config(fluid_lash_client, config);
    }
}

static void
fluid_lash_load(fluid_synth_t *synth, const char *filename)
{
    fluid_synth_sfload(synth, filename, 1);
}

static void *
fluid_lash_run(void *data)
{
    lash_event_t *event;
    lash_config_t *config;
    fluid_synth_t *synth;
    int done = 0;
    int err;
    int pending_restores = 0;

    synth = (fluid_synth_t *) data;

    while(!done)
    {
        while((event = lash_get_event(fluid_lash_client)))
        {
            switch(lash_event_get_type(event))
            {
            case LASH_Save_Data_Set:
                fluid_lash_save(synth);
                lash_send_event(fluid_lash_client, event);
                break;

            case LASH_Restore_Data_Set:
                lash_event_destroy(event);
                break;

            case LASH_Quit:
                err = kill(getpid(), SIGQUIT);

                if(err)
                {
                    fprintf(stderr, "%s: error sending signal: %s", __FUNCTION__, strerror(errno));
                }

                lash_event_destroy(event);
                done = 1;
                break;

            case LASH_Server_Lost:
                lash_event_destroy(event);
                done = 1;
                break;

            default:
                fprintf(stderr, "Received unknown LASH event of type %d\n", lash_event_get_type(event));
                lash_event_destroy(event);
                break;
            }
        }

        while((config = lash_get_config(fluid_lash_client)))
        {
            if(FLUID_STRCMP(lash_config_get_key(config), "soundfont count") == 0)
            {
                pending_restores = lash_config_get_value_int(config);
            }
            else
            {
                fluid_lash_load(synth, lash_config_get_value_string(config));
                pending_restores--;
            }

            lash_config_destroy(config);

            if(!pending_restores)
            {
                event = lash_event_new_with_type(LASH_Restore_Data_Set);
                lash_send_event(fluid_lash_client, event);
            }
        }

        usleep(10000);
    }

    return NULL;
}

#endif		/* #if HAVE_LASH   #else */
