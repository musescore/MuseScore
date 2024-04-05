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

#include "fluid_mdriver.h"
#include "fluid_settings.h"


/*
 * fluid_mdriver_definition
 */
struct _fluid_mdriver_definition_t
{
    const char *name;
    fluid_midi_driver_t *(*new)(fluid_settings_t *settings,
                                handle_midi_event_func_t event_handler,
                                void *event_handler_data);
    void (*free)(fluid_midi_driver_t *p);
    void (*settings)(fluid_settings_t *settings);
};


static const fluid_mdriver_definition_t fluid_midi_drivers[] =
{
#if ALSA_SUPPORT
    {
        "alsa_seq",
        new_fluid_alsa_seq_driver,
        delete_fluid_alsa_seq_driver,
        fluid_alsa_seq_driver_settings
    },
    {
        "alsa_raw",
        new_fluid_alsa_rawmidi_driver,
        delete_fluid_alsa_rawmidi_driver,
        fluid_alsa_rawmidi_driver_settings
    },
#endif
#if JACK_SUPPORT
    {
        "jack",
        new_fluid_jack_midi_driver,
        delete_fluid_jack_midi_driver,
        fluid_jack_midi_driver_settings
    },
#endif
#if OSS_SUPPORT
    {
        "oss",
        new_fluid_oss_midi_driver,
        delete_fluid_oss_midi_driver,
        fluid_oss_midi_driver_settings
    },
#endif
#if WINMIDI_SUPPORT
    {
        "winmidi",
        new_fluid_winmidi_driver,
        delete_fluid_winmidi_driver,
        fluid_winmidi_midi_driver_settings
    },
#endif
#if MIDISHARE_SUPPORT
    {
        "midishare",
        new_fluid_midishare_midi_driver,
        delete_fluid_midishare_midi_driver,
        NULL
    },
#endif
#if COREMIDI_SUPPORT
    {
        "coremidi",
        new_fluid_coremidi_driver,
        delete_fluid_coremidi_driver,
        fluid_coremidi_driver_settings
    },
#endif
    /* NULL terminator to avoid zero size array if no driver available */
    { NULL, NULL, NULL, NULL }
};


void fluid_midi_driver_settings(fluid_settings_t *settings)
{
    unsigned int i;
    const char *def_name = NULL;

    fluid_settings_register_int(settings, "midi.autoconnect", 0, 0, 1, FLUID_HINT_TOGGLED);

    fluid_settings_register_int(settings, "midi.realtime-prio",
                                FLUID_DEFAULT_MIDI_RT_PRIO, 0, 99, 0);
    
    fluid_settings_register_str(settings, "midi.driver", "", 0);

    for(i = 0; i < FLUID_N_ELEMENTS(fluid_midi_drivers) - 1; i++)
    {
        /* Select the default driver */
        if (def_name == NULL)
        {
            def_name = fluid_midi_drivers[i].name;
        }
    
        /* Add the driver to the list of options */
        fluid_settings_add_option(settings, "midi.driver", fluid_midi_drivers[i].name);

        if(fluid_midi_drivers[i].settings != NULL)
        {
            fluid_midi_drivers[i].settings(settings);
        }
    }

    /* Set the default driver, if any */
    if(def_name != NULL)
    {
        fluid_settings_setstr(settings, "midi.driver", def_name);
    }
}

/**
 * Create a new MIDI driver instance.
 *
 * @param settings Settings used to configure new MIDI driver. See \ref settings_midi for available options.
 * @param handler MIDI handler callback (for example: fluid_midi_router_handle_midi_event()
 *   for MIDI router)
 * @param event_handler_data Caller defined data to pass to 'handler'
 * @return New MIDI driver instance or NULL on error
 *
 * Which MIDI driver is actually created depends on the \ref settings_midi_driver option.
 */
fluid_midi_driver_t *new_fluid_midi_driver(fluid_settings_t *settings, handle_midi_event_func_t handler, void *event_handler_data)
{
    fluid_midi_driver_t *driver = NULL;
    char *allnames;
    const fluid_mdriver_definition_t *def;

    for(def = fluid_midi_drivers; def->name != NULL; def++)
    {
        if(fluid_settings_str_equal(settings, "midi.driver", def->name))
        {
            FLUID_LOG(FLUID_DBG, "Using '%s' midi driver", def->name);
            driver = def->new(settings, handler, event_handler_data);

            if(driver)
            {
                driver->define = def;
            }

            return driver;
        }
    }

    FLUID_LOG(FLUID_ERR, "Couldn't find the requested midi driver.");
    allnames = fluid_settings_option_concat(settings, "midi.driver", NULL);
    if(allnames != NULL)
    {
        if(allnames[0] != '\0')
        {
            FLUID_LOG(FLUID_INFO, "This build of fluidsynth supports the following MIDI drivers: %s", allnames);
        }
        else
        {
            FLUID_LOG(FLUID_INFO, "This build of fluidsynth doesn't support any MIDI drivers.");
        }

        FLUID_FREE(allnames);
    }

    return NULL;
}

/**
 * Delete a MIDI driver instance.
 * @param driver MIDI driver to delete
 */
void delete_fluid_midi_driver(fluid_midi_driver_t *driver)
{
    fluid_return_if_fail(driver != NULL);
    driver->define->free(driver);
}
