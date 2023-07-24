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
#include "fluid_hash.h"
#include "fluid_synth.h"
#include "fluid_cmd.h"
#include "fluid_adriver.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"
#include "fluid_midi.h"

/* maximum allowed components of a settings variable (separated by '.') */
#define MAX_SETTINGS_TOKENS 8	/* currently only a max of 3 are used */
#define MAX_SETTINGS_LABEL 256	/* max length of a settings variable label */

static void fluid_settings_init(fluid_settings_t *settings);
static void fluid_settings_key_destroy_func(void *value);
static void fluid_settings_value_destroy_func(void *value);
static int fluid_settings_tokenize(const char *s, char *buf, char **ptr);

/* Common structure to all settings nodes */
typedef struct
{
    char *value;
    char *def;
    int hints;
    fluid_list_t *options;
    fluid_str_update_t update;
    void *data;
} fluid_str_setting_t;

typedef struct
{
    double value;
    double def;
    double min;
    double max;
    int hints;
    fluid_num_update_t update;
    void *data;
} fluid_num_setting_t;

typedef struct
{
    int value;
    int def;
    int min;
    int max;
    int hints;
    fluid_int_update_t update;
    void *data;
} fluid_int_setting_t;

typedef struct
{
    fluid_hashtable_t *hashtable;
} fluid_set_setting_t;

typedef struct
{
    int type;             /**< fluid_types_enum */

    union
    {
        fluid_str_setting_t str;
        fluid_num_setting_t num;
        fluid_int_setting_t i;
        fluid_set_setting_t set;
    };
} fluid_setting_node_t;

static fluid_setting_node_t *
new_fluid_str_setting(const char *value, const char *def, int hints)
{
    fluid_setting_node_t *node;
    fluid_str_setting_t *str;

    node = FLUID_NEW(fluid_setting_node_t);

    if(!node)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->type = FLUID_STR_TYPE;

    str = &node->str;
    str->value = value ? FLUID_STRDUP(value) : NULL;
    str->def = def ? FLUID_STRDUP(def) : NULL;
    str->hints = hints;
    str->options = NULL;
    str->update = NULL;
    str->data = NULL;
    return node;
}

static void
delete_fluid_str_setting(fluid_setting_node_t *node)
{
    fluid_return_if_fail(node != NULL);

    FLUID_ASSERT(node->type == FLUID_STR_TYPE);

    FLUID_FREE(node->str.value);
    FLUID_FREE(node->str.def);

    if(node->str.options)
    {
        fluid_list_t *list = node->str.options;

        while(list)
        {
            FLUID_FREE(list->data);
            list = fluid_list_next(list);
        }

        delete_fluid_list(node->str.options);
    }

    FLUID_FREE(node);
}


static fluid_setting_node_t *
new_fluid_num_setting(double min, double max, double def, int hints)
{
    fluid_setting_node_t *node;
    fluid_num_setting_t *num;

    node = FLUID_NEW(fluid_setting_node_t);

    if(!node)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->type = FLUID_NUM_TYPE;

    num = &node->num;
    num->value = def;
    num->def = def;
    num->min = min;
    num->max = max;
    num->hints = hints;
    num->update = NULL;
    num->data = NULL;

    return node;
}

static void
delete_fluid_num_setting(fluid_setting_node_t *node)
{
    fluid_return_if_fail(node != NULL);

    FLUID_ASSERT(node->type == FLUID_NUM_TYPE);
    FLUID_FREE(node);
}

static fluid_setting_node_t *
new_fluid_int_setting(int min, int max, int def, int hints)
{
    fluid_setting_node_t *node;
    fluid_int_setting_t *i;

    node = FLUID_NEW(fluid_setting_node_t);

    if(!node)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->type = FLUID_INT_TYPE;

    i = &node->i;
    i->value = def;
    i->def = def;
    i->min = min;
    i->max = max;
    i->hints = hints;
    i->update = NULL;
    i->data = NULL;
    return node;
}

static void
delete_fluid_int_setting(fluid_setting_node_t *node)
{
    fluid_return_if_fail(node != NULL);

    FLUID_ASSERT(node->type == FLUID_INT_TYPE);
    FLUID_FREE(node);
}

static fluid_setting_node_t *
new_fluid_set_setting(void)
{
    fluid_setting_node_t *node;
    fluid_set_setting_t *set;

    node = FLUID_NEW(fluid_setting_node_t);

    if(!node)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->type = FLUID_SET_TYPE;
    set = &node->set;

    set->hashtable = new_fluid_hashtable_full(fluid_str_hash, fluid_str_equal,
                     fluid_settings_key_destroy_func,
                     fluid_settings_value_destroy_func);

    if(!set->hashtable)
    {
        FLUID_FREE(node);
        return NULL;
    }

    return node;
}

static void
delete_fluid_set_setting(fluid_setting_node_t *node)
{
    fluid_return_if_fail(node != NULL);

    FLUID_ASSERT(node->type == FLUID_SET_TYPE);
    delete_fluid_hashtable(node->set.hashtable);
    FLUID_FREE(node);
}

/**
 * Create a new settings object
 *
 * @return the pointer to the settings object
 */
fluid_settings_t *
new_fluid_settings(void)
{
    fluid_settings_t *settings;

    settings = new_fluid_hashtable_full(fluid_str_hash, fluid_str_equal,
                                        fluid_settings_key_destroy_func,
                                        fluid_settings_value_destroy_func);

    if(settings == NULL)
    {
        return NULL;
    }

    fluid_rec_mutex_init(settings->mutex);
    fluid_settings_init(settings);
    return settings;
}

/**
 * Delete the provided settings object
 *
 * @param settings a settings object
 */
void
delete_fluid_settings(fluid_settings_t *settings)
{
    fluid_return_if_fail(settings != NULL);

    fluid_rec_mutex_destroy(settings->mutex);
    delete_fluid_hashtable(settings);
}

/* Settings hash key destroy function */
static void
fluid_settings_key_destroy_func(void *value)
{
    FLUID_FREE(value);    /* Free the string key value */
}

/* Settings hash value destroy function */
static void
fluid_settings_value_destroy_func(void *value)
{
    fluid_setting_node_t *node = value;

    switch(node->type)
    {
    case FLUID_NUM_TYPE:
        delete_fluid_num_setting(node);
        break;

    case FLUID_INT_TYPE:
        delete_fluid_int_setting(node);
        break;

    case FLUID_STR_TYPE:
        delete_fluid_str_setting(node);
        break;

    case FLUID_SET_TYPE:
        delete_fluid_set_setting(node);
        break;
    }
}

void
fluid_settings_init(fluid_settings_t *settings)
{
    fluid_return_if_fail(settings != NULL);

    fluid_synth_settings(settings);
#ifndef NO_GLIB
    fluid_shell_settings(settings);
#endif
    fluid_player_settings(settings);
    fluid_file_renderer_settings(settings);
    fluid_audio_driver_settings(settings);
    fluid_midi_driver_settings(settings);
}

static int
fluid_settings_tokenize(const char *s, char *buf, char **ptr)
{
    char *tokstr, *tok;
    int n = 0;

    if(FLUID_STRLEN(s) > MAX_SETTINGS_LABEL)
    {
        FLUID_LOG(FLUID_ERR, "Setting variable name exceeded max length of %d chars",
                  MAX_SETTINGS_LABEL);
        return 0;
    }

    FLUID_STRCPY(buf, s);	/* copy string to buffer, since it gets modified */
    tokstr = buf;

    while((tok = fluid_strtok(&tokstr, ".")))
    {
        if(n >= MAX_SETTINGS_TOKENS)
        {
            FLUID_LOG(FLUID_ERR, "Setting variable name exceeded max token count of %d",
                      MAX_SETTINGS_TOKENS);
            return 0;
        }
        else
        {
            ptr[n++] = tok;
        }
    }

    return n;
}

/**
 * Get a setting name, value and type
 *
 * @param settings a settings object
 * @param name Settings name
 * @param value Location to store setting node if found
 * @return #FLUID_OK if the node exists, #FLUID_FAILED otherwise
 */
static int
fluid_settings_get(fluid_settings_t *settings, const char *name,
                   fluid_setting_node_t **value)
{
    fluid_hashtable_t *table = settings;
    fluid_setting_node_t *node = NULL;
    char *tokens[MAX_SETTINGS_TOKENS];
    char buf[MAX_SETTINGS_LABEL + 1];
    int ntokens;
    int n;

    ntokens = fluid_settings_tokenize(name, buf, tokens);

    if(table == NULL || ntokens <= 0)
    {
        return FLUID_FAILED;
    }

    for(n = 0; n < ntokens; n++)
    {

        node = fluid_hashtable_lookup(table, tokens[n]);

        if(!node)
        {
            return FLUID_FAILED;
        }

        table = (node->type == FLUID_SET_TYPE) ? node->set.hashtable : NULL;
    }

    if(value)
    {
        *value = node;
    }

    return FLUID_OK;
}

/**
 * Set a setting name, value and type, replacing it if already exists
 *
 * @param settings a settings object
 * @param name Settings name
 * @param value Node instance to assign (used directly)
 * @return #FLUID_OK if the value has been set, #FLUID_FAILED otherwise
 */
static int
fluid_settings_set(fluid_settings_t *settings, const char *name, fluid_setting_node_t *value)
{
    fluid_hashtable_t *table = settings;
    fluid_setting_node_t *node;
    char *tokens[MAX_SETTINGS_TOKENS];
    char buf[MAX_SETTINGS_LABEL + 1];
    int n, num;
    char *dupname;

    num = fluid_settings_tokenize(name, buf, tokens);

    if(num == 0)
    {
        return FLUID_FAILED;
    }

    num--;

    for(n = 0; n < num; n++)
    {

        node = fluid_hashtable_lookup(table, tokens[n]);

        if(node)
        {

            if(node->type == FLUID_SET_TYPE)
            {
                table = node->set.hashtable;
            }
            else
            {
                /* path ends prematurely */
                FLUID_LOG(FLUID_ERR, "'%s' is not a node. Name of the setting was '%s'", tokens[n], name);
                return FLUID_FAILED;
            }

        }
        else
        {
            /* create a new node */
            fluid_setting_node_t *setnode;

            dupname = FLUID_STRDUP(tokens[n]);
            setnode = new_fluid_set_setting();

            if(!dupname || !setnode)
            {
                if(dupname)
                {
                    FLUID_FREE(dupname);
                }
                else
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                }

                if(setnode)
                {
                    delete_fluid_set_setting(setnode);
                }

                return FLUID_FAILED;
            }

            fluid_hashtable_insert(table, dupname, setnode);
            table = setnode->set.hashtable;
        }
    }

    dupname = FLUID_STRDUP(tokens[num]);

    if(!dupname)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return FLUID_FAILED;
    }

    fluid_hashtable_insert(table, dupname, value);

    return FLUID_OK;
}

/**
 * Registers a new string value for the specified setting.
 *
 * @param settings a settings object
 * @param name the setting's name
 * @param def the default value for the setting
 * @param hints the hints for the setting
 * @return #FLUID_OK if the value has been register correctly, #FLUID_FAILED otherwise
 */
int
fluid_settings_register_str(fluid_settings_t *settings, const char *name, const char *def, int hints)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) != FLUID_OK)
    {
        node = new_fluid_str_setting(def, def, hints);
        retval = fluid_settings_set(settings, name, node);

        if(retval != FLUID_OK)
        {
            delete_fluid_str_setting(node);
        }
    }
    else
    {
        /* if variable already exists, don't change its value. */
        if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;
            FLUID_FREE(setting->def);
            setting->def = def ? FLUID_STRDUP(def) : NULL;
            setting->hints = hints;
            retval = FLUID_OK;
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "Failed to register string setting '%s' as it already exists with a different type", name);
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Registers a new float value for the specified setting.
 *
 * @param settings a settings object
 * @param name the setting's name
 * @param def the default value for the setting
 * @param min the smallest allowed value for the setting
 * @param max the largest allowed value for the setting
 * @param hints the hints for the setting
 * @return #FLUID_OK if the value has been register correctly, #FLUID_FAILED otherwise
 */
int
fluid_settings_register_num(fluid_settings_t *settings, const char *name, double def,
                            double min, double max, int hints)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);

    /* For now, all floating point settings are bounded below and above */
    hints |= FLUID_HINT_BOUNDED_BELOW | FLUID_HINT_BOUNDED_ABOVE;

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) != FLUID_OK)
    {
        /* insert a new setting */
        node = new_fluid_num_setting(min, max, def, hints);
        retval = fluid_settings_set(settings, name, node);

        if(retval != FLUID_OK)
        {
            delete_fluid_num_setting(node);
        }
    }
    else
    {
        if(node->type == FLUID_NUM_TYPE)
        {
            /* update the existing setting but don't change its value */
            fluid_num_setting_t *setting = &node->num;
            setting->min = min;
            setting->max = max;
            setting->def = def;
            setting->hints = hints;
            retval = FLUID_OK;
        }
        else
        {
            /* type mismatch */
            FLUID_LOG(FLUID_ERR, "Failed to register numeric setting '%s' as it already exists with a different type", name);
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Registers a new integer value for the specified setting.
 *
 * @param settings a settings object
 * @param name the setting's name
 * @param def the default value for the setting
 * @param min the smallest allowed value for the setting
 * @param max the largest allowed value for the setting
 * @param hints the hints for the setting
 * @return #FLUID_OK if the value has been register correctly, #FLUID_FAILED otherwise
 */
int
fluid_settings_register_int(fluid_settings_t *settings, const char *name, int def,
                            int min, int max, int hints)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);

    /* For now, all integer settings are bounded below and above */
    hints |= FLUID_HINT_BOUNDED_BELOW | FLUID_HINT_BOUNDED_ABOVE;

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) != FLUID_OK)
    {
        /* insert a new setting */
        node = new_fluid_int_setting(min, max, def, hints);
        retval = fluid_settings_set(settings, name, node);

        if(retval != FLUID_OK)
        {
            delete_fluid_int_setting(node);
        }
    }
    else
    {
        if(node->type == FLUID_INT_TYPE)
        {
            /* update the existing setting but don't change its value */
            fluid_int_setting_t *setting = &node->i;
            setting->min = min;
            setting->max = max;
            setting->def = def;
            setting->hints = hints;
            retval = FLUID_OK;
        }
        else
        {
            /* type mismatch */
            FLUID_LOG(FLUID_ERR, "Failed to register int setting '%s' as it already exists with a different type", name);
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Registers a callback for the specified string setting.
 *
 * @param settings a settings object
 * @param name the setting's name
 * @param callback an update function for the setting
 * @param data user supplied data passed to the update function
 * @return #FLUID_OK if the callback has been set, #FLUID_FAILED otherwise
 */
int fluid_settings_callback_str(fluid_settings_t *settings, const char *name,
                                fluid_str_update_t callback, void *data)
{
    fluid_setting_node_t *node;
    fluid_str_setting_t *setting;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if((fluid_settings_get(settings, name, &node) != FLUID_OK)
            || node->type != FLUID_STR_TYPE)
    {
        fluid_rec_mutex_unlock(settings->mutex);
        return FLUID_FAILED;
    }

    setting = &node->str;
    setting->update = callback;
    setting->data = data;

    fluid_rec_mutex_unlock(settings->mutex);
    return FLUID_OK;
}

/**
 * Registers a callback for the specified numeric setting.
 *
 * @param settings a settings object
 * @param name the setting's name
 * @param callback an update function for the setting
 * @param data user supplied data passed to the update function
 * @return #FLUID_OK if the callback has been set, #FLUID_FAILED otherwise
 */
int fluid_settings_callback_num(fluid_settings_t *settings, const char *name,
                                fluid_num_update_t callback, void *data)
{
    fluid_setting_node_t *node;
    fluid_num_setting_t *setting;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if((fluid_settings_get(settings, name, &node) != FLUID_OK)
            || node->type != FLUID_NUM_TYPE)
    {
        fluid_rec_mutex_unlock(settings->mutex);
        return FLUID_FAILED;
    }

    setting = &node->num;
    setting->update = callback;
    setting->data = data;

    fluid_rec_mutex_unlock(settings->mutex);
    return FLUID_OK;
}

/**
 * Registers a callback for the specified int setting.
 *
 * @param settings a settings object
 * @param name the setting's name
 * @param callback an update function for the setting
 * @param data user supplied data passed to the update function
 * @return #FLUID_OK if the callback has been set, #FLUID_FAILED otherwise
 */
int fluid_settings_callback_int(fluid_settings_t *settings, const char *name,
                                fluid_int_update_t callback, void *data)
{
    fluid_setting_node_t *node;
    fluid_int_setting_t *setting;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if((fluid_settings_get(settings, name, &node) != FLUID_OK)
            || node->type != FLUID_INT_TYPE)
    {
        fluid_rec_mutex_unlock(settings->mutex);
        return FLUID_FAILED;
    }

    setting = &node->i;
    setting->update = callback;
    setting->data = data;

    fluid_rec_mutex_unlock(settings->mutex);
    return FLUID_OK;
}

void* fluid_settings_get_user_data(fluid_settings_t * settings, const char *name)
{
    fluid_setting_node_t *node;
    void* retval = NULL;

    fluid_return_val_if_fail(settings != NULL, NULL);
    fluid_return_val_if_fail(name != NULL, NULL);
    fluid_return_val_if_fail(name[0] != '\0', NULL);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_NUM_TYPE)
        {
            fluid_num_setting_t *setting = &node->num;
            retval = setting->data;
        }
        else if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;
            retval = setting->data;
        }
        else if(node->type == FLUID_INT_TYPE)
        {
            fluid_int_setting_t *setting = &node->i;
            retval = setting->data;
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);
    return retval;
}

/**
 * Get the type of the setting with the given name
 *
 * @param settings a settings object
 * @param name a setting's name
 * @return the type for the named setting (see #fluid_types_enum), or #FLUID_NO_TYPE when it does not exist
 */
int
fluid_settings_get_type(fluid_settings_t *settings, const char *name)
{
    fluid_setting_node_t *node;
    int type = FLUID_NO_TYPE;

    fluid_return_val_if_fail(settings != NULL, type);
    fluid_return_val_if_fail(name != NULL, type);
    fluid_return_val_if_fail(name[0] != '\0', type);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        type = node->type;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return type;
}

/**
 * Get the hints for the named setting as an integer bitmap
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param hints set to the hints associated to the setting if it exists
 * @return #FLUID_OK if hints associated to the named setting exist, #FLUID_FAILED otherwise
 */
int
fluid_settings_get_hints(fluid_settings_t *settings, const char *name, int *hints)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_NUM_TYPE)
        {
            fluid_num_setting_t *setting = &node->num;
            *hints = setting->hints;
            retval = FLUID_OK;
        }
        else if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;
            *hints = setting->hints;
            retval = FLUID_OK;
        }
        else if(node->type == FLUID_INT_TYPE)
        {
            fluid_int_setting_t *setting = &node->i;
            *hints = setting->hints;
            retval = FLUID_OK;
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Ask whether the setting is changeable in real-time.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @return TRUE if the setting is changeable in real-time, FALSE otherwise
 *
 * @note Before using this function, make sure the @p settings object has already been used to create
 * a synthesizer, a MIDI driver, an audio driver, a MIDI player, or a command handler (depending on
 * which settings you want to query).
 */
int
fluid_settings_is_realtime(fluid_settings_t *settings, const char *name)
{
    fluid_setting_node_t *node;
    int isrealtime = FALSE;

    fluid_return_val_if_fail(settings != NULL, 0);
    fluid_return_val_if_fail(name != NULL, 0);
    fluid_return_val_if_fail(name[0] != '\0', 0);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_NUM_TYPE)
        {
            fluid_num_setting_t *setting = &node->num;
            isrealtime = setting->update != NULL;
        }
        else if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;
            isrealtime = setting->update != NULL;
        }
        else if(node->type == FLUID_INT_TYPE)
        {
            fluid_int_setting_t *setting = &node->i;
            isrealtime = setting->update != NULL;
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return isrealtime;
}

/**
 * Set a string value for a named setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param str new string value
 * @return #FLUID_OK if the value has been set, #FLUID_FAILED otherwise
 */
int
fluid_settings_setstr(fluid_settings_t *settings, const char *name, const char *str)
{
    fluid_setting_node_t *node;
    fluid_str_setting_t *setting;
    char *new_value = NULL;
    fluid_str_update_t callback = NULL;
    void *data = NULL;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if((fluid_settings_get(settings, name, &node) != FLUID_OK)
            || (node->type != FLUID_STR_TYPE))
    {
        FLUID_LOG(FLUID_ERR, "Unknown string setting '%s'", name);
        goto error_recovery;
    }

    setting = &node->str;

    if(setting->value)
    {
        FLUID_FREE(setting->value);
    }

    if(str)
    {
        new_value = FLUID_STRDUP(str);

        if(new_value == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }
    }

    setting->value = new_value;

    callback = setting->update;
    data = setting->data;

    /* Release the mutex before calling the update callback, to avoid
     * possible deadlocks with FluidSynths API lock */
    fluid_rec_mutex_unlock(settings->mutex);

    if(callback)
    {
        (*callback)(data, name, new_value);
    }

    return FLUID_OK;

error_recovery:
    fluid_rec_mutex_unlock(settings->mutex);
    return FLUID_FAILED;
}

/**
 * Copy the value of a string setting into the provided buffer (thread safe)
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param str Caller supplied buffer to copy string value to
 * @param len Size of 'str' buffer (no more than len bytes will be written, which
 *   will always include a zero terminator)
 * @return #FLUID_OK if the value exists, #FLUID_FAILED otherwise
 *
 * @note A size of 256 should be more than sufficient for the string buffer.
 *
 * @since 1.1.0
 */
int
fluid_settings_copystr(fluid_settings_t *settings, const char *name,
                       char *str, int len)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(str != NULL, retval);
    fluid_return_val_if_fail(len > 0, retval);

    str[0] = 0;

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;

            if(setting->value)
            {
                FLUID_STRNCPY(str, setting->value, len);
            }

            retval = FLUID_OK;
        }
        else if(node->type == FLUID_INT_TYPE)       /* Handle boolean integers for backwards compatibility */
        {
            fluid_int_setting_t *setting = &node->i;

            if(setting->hints & FLUID_HINT_TOGGLED)
            {
                FLUID_STRNCPY(str, setting->value ? "yes" : "no", len);

                retval = FLUID_OK;
            }
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Duplicate the value of a string setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param str Location to store pointer to allocated duplicate string
 * @return #FLUID_OK if the value exists and was successfully duplicated, #FLUID_FAILED otherwise
 *
 * Like fluid_settings_copystr() but allocates a new copy of the string.  Caller
 * owns the string and should free it with fluid_free() when done using it.
 *
 * @since 1.1.0
 */
int
fluid_settings_dupstr(fluid_settings_t *settings, const char *name, char **str)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(str != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;

            if(setting->value)
            {
                *str = FLUID_STRDUP(setting->value);

                if(!*str)
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                }
            }

            if(!setting->value || *str)
            {
                retval = FLUID_OK;    /* Don't set to FLUID_OK if out of memory */
            }
        }
        else if(node->type == FLUID_INT_TYPE)       /* Handle boolean integers for backwards compatibility */
        {
            fluid_int_setting_t *setting = &node->i;

            if(setting->hints & FLUID_HINT_TOGGLED)
            {
                *str = FLUID_STRDUP(setting->value ? "yes" : "no");

                if(!*str)
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                }

                if(!setting->value || *str)
                {
                    retval = FLUID_OK;    /* Don't set to FLUID_OK if out of memory */
                }
            }
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}


/**
 * Test a string setting for some value.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param s a string to be tested
 * @return TRUE if the value exists and is equal to \c s, FALSE otherwise
 */
int
fluid_settings_str_equal(fluid_settings_t *settings, const char *name, const char *s)
{
    fluid_setting_node_t *node;
    int retval = FALSE;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(s != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;

            if(setting->value)
            {
                retval = FLUID_STRCMP(setting->value, s) == 0;
            }
        }
        else if(node->type == FLUID_INT_TYPE)       /* Handle boolean integers for backwards compatibility */
        {
            fluid_int_setting_t *setting = &node->i;

            if(setting->hints & FLUID_HINT_TOGGLED)
            {
                retval = FLUID_STRCMP(setting->value ? "yes" : "no", s) == 0;
            }
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Get the default value of a string setting.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param def the default string value of the setting if it exists
 * @return FLUID_OK if a default value exists, FLUID_FAILED otherwise
 *
 * @note The returned string is not owned by the caller and should not be modified or freed.
 */
int
fluid_settings_getstr_default(fluid_settings_t *settings, const char *name, char **def)
{
    fluid_setting_node_t *node;
    char *retval = NULL;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK)
    {
        if(node->type == FLUID_STR_TYPE)
        {
            fluid_str_setting_t *setting = &node->str;
            retval = setting->def;
        }
        else if(node->type == FLUID_INT_TYPE)       /* Handle boolean integers for backwards compatibility */
        {
            fluid_int_setting_t *setting = &node->i;

            if(setting->hints & FLUID_HINT_TOGGLED)
            {
                retval = setting->def ? "yes" : "no";
            }
        }
    }

    *def = retval;
    fluid_rec_mutex_unlock(settings->mutex);

    return retval != NULL ? FLUID_OK : FLUID_FAILED;
}

/**
 * Add an option to a string setting (like an enumeration value).
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param s option string to add
 * @return #FLUID_OK if the setting exists and option was added, #FLUID_FAILED otherwise
 *
 * Causes the setting's #FLUID_HINT_OPTIONLIST hint to be set.
 */
int
fluid_settings_add_option(fluid_settings_t *settings, const char *name, const char *s)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(s != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_STR_TYPE))
    {
        fluid_str_setting_t *setting = &node->str;
        char *copy = FLUID_STRDUP(s);
        setting->options = fluid_list_append(setting->options, copy);
        setting->hints |= FLUID_HINT_OPTIONLIST;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Remove an option previously assigned by fluid_settings_add_option().
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param s option string to remove
 * @return #FLUID_OK if the setting exists and option was removed, #FLUID_FAILED otherwise
 */
int
fluid_settings_remove_option(fluid_settings_t *settings, const char *name, const char *s)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(s != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_STR_TYPE))
    {

        fluid_str_setting_t *setting = &node->str;
        fluid_list_t *list = setting->options;

        while(list)
        {
            char *option = (char *) fluid_list_get(list);

            if(FLUID_STRCMP(s, option) == 0)
            {
                FLUID_FREE(option);
                setting->options = fluid_list_remove_link(setting->options, list);
                retval = FLUID_OK;
                break;
            }

            list = fluid_list_next(list);
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Set a numeric value for a named setting.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val new setting's value
 * @return #FLUID_OK if the value has been set, #FLUID_FAILED otherwise
 */
int
fluid_settings_setnum(fluid_settings_t *settings, const char *name, double val)
{
    fluid_setting_node_t *node;
    fluid_num_setting_t *setting;
    fluid_num_update_t callback = NULL;
    void *data = NULL;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if((fluid_settings_get(settings, name, &node) != FLUID_OK)
            || (node->type != FLUID_NUM_TYPE))
    {
        FLUID_LOG(FLUID_ERR, "Unknown numeric setting '%s'", name);
        goto error_recovery;
    }

    setting = &node->num;

    if(val < setting->min || val > setting->max)
    {
        FLUID_LOG(FLUID_ERR, "requested set value for '%s' out of range", name);
        goto error_recovery;
    }

    setting->value = val;

    callback = setting->update;
    data = setting->data;

    /* Release the mutex before calling the update callback, to avoid
     * possible deadlocks with FluidSynths API lock */
    fluid_rec_mutex_unlock(settings->mutex);

    if(callback)
    {
        (*callback)(data, name, val);
    }

    return FLUID_OK;

error_recovery:
    fluid_rec_mutex_unlock(settings->mutex);
    return FLUID_FAILED;
}

/**
 * Get the numeric value of a named setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val variable pointer to receive the setting's numeric value
 * @return #FLUID_OK if the value exists, #FLUID_FAILED otherwise
 */
int
fluid_settings_getnum(fluid_settings_t *settings, const char *name, double *val)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(val != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_NUM_TYPE))
    {
        fluid_num_setting_t *setting = &node->num;
        *val = setting->value;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * float-typed wrapper for fluid_settings_getnum
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val variable pointer to receive the setting's float value
 * @return #FLUID_OK if the value exists, #FLUID_FAILED otherwise
 */
int fluid_settings_getnum_float(fluid_settings_t *settings, const char *name, float *val)
{
    double tmp;

    if(fluid_settings_getnum(settings, name, &tmp) == FLUID_OK)
    {
        *val = tmp;
        return FLUID_OK;
    }

    return FLUID_FAILED;
}

/**
 * Get the range of values of a numeric setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param min setting's range lower limit
 * @param max setting's range upper limit
 * @return #FLUID_OK if the setting's range exists, #FLUID_FAILED otherwise
 */
int
fluid_settings_getnum_range(fluid_settings_t *settings, const char *name,
                            double *min, double *max)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(min != NULL, retval);
    fluid_return_val_if_fail(max != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_NUM_TYPE))
    {
        fluid_num_setting_t *setting = &node->num;
        *min = setting->min;
        *max = setting->max;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Get the default value of a named numeric (double) setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val set to the default value if the named setting exists
 * @return #FLUID_OK if the default value of the named setting exists, #FLUID_FAILED otherwise
 */
int
fluid_settings_getnum_default(fluid_settings_t *settings, const char *name, double *val)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(val != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_NUM_TYPE))
    {
        fluid_num_setting_t *setting = &node->num;
        *val = setting->def;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Set an integer value for a setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val new setting's integer value
 * @return #FLUID_OK if the value has been set, #FLUID_FAILED otherwise
 */
int
fluid_settings_setint(fluid_settings_t *settings, const char *name, int val)
{
    fluid_setting_node_t *node;
    fluid_int_setting_t *setting;
    fluid_int_update_t callback = NULL;
    void *data = NULL;

    fluid_return_val_if_fail(settings != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name[0] != '\0', FLUID_FAILED);

    fluid_rec_mutex_lock(settings->mutex);

    if((fluid_settings_get(settings, name, &node) != FLUID_OK)
            || (node->type != FLUID_INT_TYPE))
    {
        FLUID_LOG(FLUID_ERR, "Unknown integer parameter '%s'", name);
        goto error_recovery;
    }

    setting = &node->i;

    if(val < setting->min || val > setting->max)
    {
        FLUID_LOG(FLUID_ERR, "requested set value for setting '%s' out of range", name);
        goto error_recovery;
    }

    setting->value = val;

    callback = setting->update;
    data = setting->data;

    /* Release the mutex before calling the update callback, to avoid
     * possible deadlocks with FluidSynths API lock */
    fluid_rec_mutex_unlock(settings->mutex);

    if(callback)
    {
        (*callback)(data, name, val);
    }

    return FLUID_OK;

error_recovery:
    fluid_rec_mutex_unlock(settings->mutex);
    return FLUID_FAILED;
}

/**
 * Get an integer value setting.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val pointer to a variable to receive the setting's integer value
 * @return #FLUID_OK if the value exists, #FLUID_FAILED otherwise
 */
int
fluid_settings_getint(fluid_settings_t *settings, const char *name, int *val)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(val != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_INT_TYPE))
    {
        fluid_int_setting_t *setting = &node->i;
        *val = setting->value;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Get the range of values of an integer setting
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param min setting's range lower limit
 * @param max setting's range upper limit
 * @return #FLUID_OK if the setting's range exists, #FLUID_FAILED otherwise
 */
int
fluid_settings_getint_range(fluid_settings_t *settings, const char *name,
                            int *min, int *max)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(min != NULL, retval);
    fluid_return_val_if_fail(max != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_INT_TYPE))
    {
        fluid_int_setting_t *setting = &node->i;
        *min = setting->min;
        *max = setting->max;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Get the default value of an integer setting.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param val set to the setting's default integer value if it exists
 * @return #FLUID_OK if the setting's default integer value exists, #FLUID_FAILED otherwise
 */
int fluid_settings_getint_default(fluid_settings_t *settings, const char *name, int *val)
{
    fluid_setting_node_t *node;
    int retval = FLUID_FAILED;

    fluid_return_val_if_fail(settings != NULL, retval);
    fluid_return_val_if_fail(name != NULL, retval);
    fluid_return_val_if_fail(name[0] != '\0', retval);
    fluid_return_val_if_fail(val != NULL, retval);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && (node->type == FLUID_INT_TYPE))
    {
        fluid_int_setting_t *setting = &node->i;
        *val = setting->def;
        retval = FLUID_OK;
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return retval;
}

/**
 * Iterate the available options for a named string setting, calling the provided
 * callback function for each existing option.
 *
 * @param settings a settings object
 * @param name a setting's name
 * @param data any user provided pointer
 * @param func callback function to be called on each iteration
 *
 * @note Starting with FluidSynth 1.1.0 the \p func callback is called for each
 * option in alphabetical order.  Sort order was undefined in previous versions.
 */
void
fluid_settings_foreach_option(fluid_settings_t *settings, const char *name,
                              void *data, fluid_settings_foreach_option_t func)
{
    fluid_setting_node_t *node;
    fluid_str_setting_t *setting;
    fluid_list_t *p, *newlist = NULL;

    fluid_return_if_fail(settings != NULL);
    fluid_return_if_fail(name != NULL);
    fluid_return_if_fail(name[0] != '\0');
    fluid_return_if_fail(func != NULL);

    fluid_rec_mutex_lock(settings->mutex);        /* ++ lock */

    if(fluid_settings_get(settings, name, &node) != FLUID_OK
            || node->type != FLUID_STR_TYPE)
    {
        fluid_rec_mutex_unlock(settings->mutex);    /* -- unlock */
        return;
    }

    setting = &node->str;

    /* Duplicate option list */
    for(p = setting->options; p; p = p->next)
    {
        newlist = fluid_list_append(newlist, fluid_list_get(p));
    }

    /* Sort by name */
    newlist = fluid_list_sort(newlist, fluid_list_str_compare_func);

    for(p = newlist; p; p = p->next)
    {
        (*func)(data, name, (const char *)fluid_list_get(p));
    }

    fluid_rec_mutex_unlock(settings->mutex);    /* -- unlock */

    delete_fluid_list(newlist);
}

/**
 * Count option string values for a string setting.
 *
 * @param settings a settings object
 * @param name Name of setting
 * @return Count of options for this string setting (0 if none, -1 if not found
 *   or not a string setting)
 *
 * @since 1.1.0
 */
int
fluid_settings_option_count(fluid_settings_t *settings, const char *name)
{
    fluid_setting_node_t *node;
    int count = -1;

    fluid_return_val_if_fail(settings != NULL, -1);
    fluid_return_val_if_fail(name != NULL, -1);
    fluid_return_val_if_fail(name[0] != '\0', -1);

    fluid_rec_mutex_lock(settings->mutex);

    if(fluid_settings_get(settings, name, &node) == FLUID_OK
            && node->type == FLUID_STR_TYPE)
    {
        count = fluid_list_size(node->str.options);
    }

    fluid_rec_mutex_unlock(settings->mutex);

    return (count);
}

/**
 * Concatenate options for a string setting together with a separator between.
 *
 * @param settings Settings object
 * @param name Settings name
 * @param separator String to use between options (NULL to use ", ")
 * @return Newly allocated string or NULL on error (out of memory, not a valid
 *   setting \p name or not a string setting). Free the string when finished with it by using fluid_free().
 *
 * @since 1.1.0
 */
char *
fluid_settings_option_concat(fluid_settings_t *settings, const char *name,
                             const char *separator)
{
    fluid_setting_node_t *node;
    fluid_list_t *p, *newlist = NULL;
    size_t count, len;
    char *str, *option;

    fluid_return_val_if_fail(settings != NULL, NULL);
    fluid_return_val_if_fail(name != NULL, NULL);
    fluid_return_val_if_fail(name[0] != '\0', NULL);

    if(!separator)
    {
        separator = ", ";
    }

    fluid_rec_mutex_lock(settings->mutex);        /* ++ lock */

    if(fluid_settings_get(settings, name, &node) != FLUID_OK
            || node->type != FLUID_STR_TYPE)
    {
        fluid_rec_mutex_unlock(settings->mutex);    /* -- unlock */
        return (NULL);
    }

    /* Duplicate option list, count options and get total string length */
    for(p = node->str.options, count = 0, len = 0; p; p = p->next)
    {
        option = fluid_list_get(p);

        if(option)
        {
            newlist = fluid_list_append(newlist, option);
            len += FLUID_STRLEN(option);
            count++;
        }
    }

    if(count > 1)
    {
        len += (count - 1) * FLUID_STRLEN(separator);
    }

    len++;        /* For terminator */

    /* Sort by name */
    newlist = fluid_list_sort(newlist, fluid_list_str_compare_func);

    str = FLUID_MALLOC(len);

    if(str)
    {
        str[0] = 0;

        for(p = newlist; p; p = p->next)
        {
            option = fluid_list_get(p);
            strcat(str, option);

            if(p->next)
            {
                strcat(str, separator);
            }
        }
    }

    fluid_rec_mutex_unlock(settings->mutex);    /* -- unlock */

    delete_fluid_list(newlist);

    if(!str)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
    }

    return (str);
}

/* Structure passed to fluid_settings_foreach_iter recursive function */
typedef struct
{
    char path[MAX_SETTINGS_LABEL + 1];    /* Maximum settings label length */
    fluid_list_t *names;                  /* For fluid_settings_foreach() */
} fluid_settings_foreach_bag_t;

static int
fluid_settings_foreach_iter(void *key, void *value, void *data)
{
    fluid_settings_foreach_bag_t *bag = data;
    char *keystr = key;
    fluid_setting_node_t *node = value;
    size_t pathlen;
    char *s;

    pathlen = FLUID_STRLEN(bag->path);

    if(pathlen > 0)
    {
        bag->path[pathlen] = '.';
        bag->path[pathlen + 1] = 0;
    }

    strcat(bag->path, keystr);

    switch(node->type)
    {
    case FLUID_NUM_TYPE:
    case FLUID_INT_TYPE:
    case FLUID_STR_TYPE:
        s = FLUID_STRDUP(bag->path);

        if(s)
        {
            bag->names = fluid_list_append(bag->names, s);
        }

        break;

    case FLUID_SET_TYPE:
        fluid_hashtable_foreach(node->set.hashtable,
                                fluid_settings_foreach_iter, bag);
        break;
    }

    bag->path[pathlen] = 0;

    return 0;
}

/**
 * Iterate the existing settings defined in a settings object, calling the
 * provided callback function for each setting.
 *
 * @param settings a settings object
 * @param data any user provided pointer
 * @param func callback function to be called on each iteration
 *
 * @note Starting with FluidSynth 1.1.0 the \p func callback is called for each
 * setting in alphabetical order.  Sort order was undefined in previous versions.
 */
void
fluid_settings_foreach(fluid_settings_t *settings, void *data,
                       fluid_settings_foreach_t func)
{
    fluid_settings_foreach_bag_t bag;
    fluid_setting_node_t *node;
    fluid_list_t *p;

    fluid_return_if_fail(settings != NULL);
    fluid_return_if_fail(func != NULL);

    bag.path[0] = 0;
    bag.names = NULL;

    fluid_rec_mutex_lock(settings->mutex);

    /* Add all node names to the bag.names list */
    fluid_hashtable_foreach(settings, fluid_settings_foreach_iter, &bag);

    /* Sort names */
    bag.names = fluid_list_sort(bag.names, fluid_list_str_compare_func);

    /* Loop over names and call the callback */
    for(p = bag.names; p; p = p->next)
    {
        if(fluid_settings_get(settings, (const char *)(p->data), &node) == FLUID_OK
                && node)
        {
            (*func)(data, (const char *)(p->data), node->type);
        }

        FLUID_FREE(p->data);        /* -- Free name */
    }

    fluid_rec_mutex_unlock(settings->mutex);

    delete_fluid_list(bag.names);         /* -- Free names list */
}

/**
 * Split a comma-separated list of integers and fill the passed
 * in buffer with the parsed values.
 *
 * @param str the comma-separated string to split
 * @param buf user-supplied buffer to hold the parsed numbers
 * @param buf_len length of user-supplied buffer
 * @return number of parsed values or -1 on failure
 */
int fluid_settings_split_csv(const char *str, int *buf, int buf_len)
{
    char *s;
    char *tok;
    char *tokstr;
    int n = 0;

    s = tokstr = FLUID_STRDUP(str);

    if(s == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return -1;
    }

    while((tok = fluid_strtok(&tokstr, ",")) && n < buf_len)
    {
        buf[n++] = atoi(tok);
    }

    FLUID_FREE(s);

    return n;
}
