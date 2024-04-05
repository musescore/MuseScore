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

/* This module: 3/2002
 * Author: Markus Nentwig, nentwig@users.sourceforge.net
 *
 * Complete rewrite: 10/2017
 * Author: Marcus Weseloh
 */

#include "fluid_ladspa.h"
#include "fluid_list.h"

#if defined(LADSPA) || defined(__DOXYGEN__)

#include <math.h>
#include <ladspa.h>


typedef enum _fluid_ladspa_state_t
{
    FLUID_LADSPA_INACTIVE = 0,
    FLUID_LADSPA_ACTIVE,
    FLUID_LADSPA_RUNNING

} fluid_ladspa_state_t;

typedef enum _fluid_ladspa_dir_t
{
    FLUID_LADSPA_INPUT,
    FLUID_LADSPA_OUTPUT,

} fluid_ladspa_dir_t;

typedef enum _fluid_ladspa_node_type_t
{
    FLUID_LADSPA_NODE_AUDIO = 1,
    FLUID_LADSPA_NODE_CONTROL = 2,
    FLUID_LADSPA_NODE_EFFECT = 4,
    FLUID_LADSPA_NODE_HOST = 8,
    FLUID_LADSPA_NODE_USER = 16,

} fluid_ladspa_node_type_t;

typedef struct _fluid_ladspa_node_t
{
    char *name;
    fluid_ladspa_node_type_t type;

    /* The buffer that LADSPA effects read and/or write to.
     * If FluidSynth has been compiled WITH_FLOAT, then this
     * points to the host buffer, otherwise it's a separate
     * buffer. */
    LADSPA_Data *effect_buffer;

    /* Only set for host nodes, points to the host buffer */
    fluid_real_t *host_buffer;

    int num_inputs;
    int num_outputs;

} fluid_ladspa_node_t;

typedef struct _fluid_ladspa_effect_t
{
    char *name;

    /* Pointer to the library that this effect was loaded from */
    fluid_module_t *lib;

    /* The descriptor defines the plugin implementation, the
     * handle points to an instance of that plugin */
    const LADSPA_Descriptor *desc;
    LADSPA_Handle *handle;

    int active;

    /* Decides if we should call the run (mix = 0) or run_adding (mix = 1)
     * plugin interface */
    int mix;

    /* Used to keep track of the port connection state */
    fluid_ladspa_node_t **port_nodes;

} fluid_ladspa_effect_t;

struct _fluid_ladspa_fx_t
{
    unsigned long sample_rate;

    /* The buffer size for all audio buffers (in samples) */
    int buffer_size;

    fluid_list_t *host_nodes;

    fluid_list_t *user_nodes;

    fluid_list_t *effects;

    fluid_rec_mutex_t api_mutex;

    fluid_atomic_int_t state;
    int pending_deactivation;

    fluid_cond_mutex_t *run_finished_mutex;
    fluid_cond_t *run_finished_cond;

};

#define LADSPA_API_ENTER(_fx) (fluid_rec_mutex_lock((_fx)->api_mutex))

#define LADSPA_API_RETURN(_fx, _ret)          \
    fluid_rec_mutex_unlock((_fx)->api_mutex); \
    return (_ret)

static void clear_ladspa(fluid_ladspa_fx_t *fx);

/* Node helpers */
static fluid_ladspa_node_t *new_fluid_ladspa_node(fluid_ladspa_fx_t *fx, const char *name,
        fluid_ladspa_node_type_t type, fluid_real_t *host_buffer);
static void delete_fluid_ladspa_node(fluid_ladspa_node_t *node);
static fluid_ladspa_node_t *get_node(fluid_ladspa_fx_t *fx, const char *name);

/* Effect helpers */
static fluid_ladspa_effect_t *
new_fluid_ladspa_effect(fluid_ladspa_fx_t *fx, const char *lib_name, const char *plugin_name);
static void delete_fluid_ladspa_effect(fluid_ladspa_effect_t *effect);
static void activate_effect(fluid_ladspa_effect_t *effect);
static void deactivate_effect(fluid_ladspa_effect_t *effect);
static fluid_ladspa_effect_t *get_effect(fluid_ladspa_fx_t *fx, const char *name);
static int get_effect_port_idx(const fluid_ladspa_effect_t *effect, const char *name);
static LADSPA_Data get_default_port_value(fluid_ladspa_effect_t *effect, unsigned int port_idx,
        int sample_rate);
static void connect_node_to_port(fluid_ladspa_node_t *node, fluid_ladspa_dir_t dir,
                                 fluid_ladspa_effect_t *effect, int port_idx);
static int create_control_port_nodes(fluid_ladspa_fx_t *fx, fluid_ladspa_effect_t *effect);

/* Plugin helpers */
static const LADSPA_Descriptor *get_plugin_descriptor(fluid_module_t *lib, const char *name);

/* Sanity checks */
static int check_all_ports_connected(fluid_ladspa_effect_t *effect, const char **name);
static int check_no_inplace_broken(fluid_ladspa_effect_t *effect, const char **name1, const char **name2);
static int check_host_output_used(fluid_ladspa_fx_t *fx);
static int check_all_audio_nodes_connected(fluid_ladspa_fx_t *fx, const char **name);

#ifndef WITH_FLOAT
static FLUID_INLINE void copy_host_to_effect_buffers(fluid_ladspa_fx_t *fx, int num_samples);
static FLUID_INLINE void copy_effect_to_host_buffers(fluid_ladspa_fx_t *fx, int num_samples);
#endif

/**
 * Creates a new LADSPA effects unit.
 *
 * @param sample_rate sample_rate for the LADSPA effects
 * @param buffer_size size of all audio buffers
 * @return pointer to the new LADSPA effects unit
 */
fluid_ladspa_fx_t *new_fluid_ladspa_fx(fluid_real_t sample_rate, int buffer_size)
{
    fluid_ladspa_fx_t *fx;

    fx = FLUID_NEW(fluid_ladspa_fx_t);

    if(fx == NULL)
    {
        return NULL;
    }

    FLUID_MEMSET(fx, 0, sizeof(fluid_ladspa_fx_t));

    /* Setup recursive mutex to protect access to public API */
    fluid_rec_mutex_init(fx->api_mutex);

    fluid_atomic_int_set(&fx->state, FLUID_LADSPA_INACTIVE);
    fx->buffer_size = buffer_size;

    /* add 0.5 to minimize overall casting error */
    fx->sample_rate = (unsigned long)(sample_rate + 0.5);

    /* Setup mutex and cond used to signal that fluid_ladspa_run has finished */
    fx->run_finished_mutex = new_fluid_cond_mutex();

    if(fx->run_finished_mutex == NULL)
    {
        goto error_recovery;
    }

    fx->run_finished_cond = new_fluid_cond();

    if(fx->run_finished_cond == NULL)
    {
        goto error_recovery;
    }

    return fx;

error_recovery:
    delete_fluid_ladspa_fx(fx);
    return NULL;
}

/**
 * Destroy and free a LADSPA effects unit previously created
 * with new_fluid_ladspa_fx().
 *
 * @param fx LADSPA effects instance
 *
 * @note This function does not check the engine state for
 * possible users, so make sure that you only call this
 * if you are sure nobody is using the engine anymore (especially
 * that nobody calls fluid_ladspa_run)
 *
 */
void delete_fluid_ladspa_fx(fluid_ladspa_fx_t *fx)
{
    fluid_list_t *list;
    fluid_ladspa_node_t *node;

    fluid_return_if_fail(fx != NULL);

    clear_ladspa(fx);

    /* Delete the remaining host nodes */
    for(list = fx->host_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);
        delete_fluid_ladspa_node(node);
    }
    delete_fluid_list(fx->host_nodes);

    if(fx->run_finished_cond != NULL)
    {
        delete_fluid_cond(fx->run_finished_cond);
    }

    if(fx->run_finished_mutex != NULL)
    {
        delete_fluid_cond_mutex(fx->run_finished_mutex);
    }

    fluid_rec_mutex_destroy(fx->api_mutex);

    FLUID_FREE(fx);
}

/**
 * Add host buffers to the LADSPA engine.
 *
 * @param fx LADSPA fx instance
 * @param prefix common name prefix for the created nodes
 * @param num_buffers number of of buffers buffer array
 * @param buffers array of sample buffers
 * @param buf_stride number of samples contained in one buffer
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 *
 * @note The size of the buffers pointed to by the buffers array must be
 * at least as large as the buffer size given to new_fluid_ladspa_fx.
 *
 */
int fluid_ladspa_add_host_ports(fluid_ladspa_fx_t *fx, const char *prefix,
                                int num_buffers, fluid_real_t buffers[], int buf_stride)
{
    int i;
    char name[99];
    fluid_ladspa_node_t *node;

    LADSPA_API_ENTER(fx);

    if(fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    /* Create nodes for all channels */
    for(i = 0; i < num_buffers; i++)
    {
        /* If there is more than one buffer, then append a 1-based index to each node name */
        if(num_buffers > 1)
        {
            FLUID_SNPRINTF(name, sizeof(name), "%s%d", prefix, (i + 1));
        }
        else
        {
            FLUID_STRNCPY(name, prefix, sizeof(name));
        }

        node = new_fluid_ladspa_node(fx, name,
                                     FLUID_LADSPA_NODE_AUDIO | FLUID_LADSPA_NODE_HOST,
                                     &buffers[i * buf_stride]);
        if (node == NULL)
        {
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }

        fx->host_nodes = fluid_list_append(fx->host_nodes, node);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}


/**
 * Set the sample rate of the LADSPA effects.
 *
 * @param fx LADSPA fx instance
 * @param sample_rate new sample rate
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 *
 * Resets the LADSPA effects if the sample rate is different from the
 * previous sample rate.
 *
 */
int fluid_ladspa_set_sample_rate(fluid_ladspa_fx_t *fx, fluid_real_t sample_rate)
{
    unsigned long new_sample_rate;

    LADSPA_API_ENTER(fx);

    /* Add 0.5 to minimize rounding errors */
    new_sample_rate = (unsigned long)(sample_rate + 0.5);

    if(fx->sample_rate == new_sample_rate)
    {
        LADSPA_API_RETURN(fx, FLUID_OK);
    }

    if(fluid_ladspa_is_active(fx))
    {
        if(fluid_ladspa_reset(fx) != FLUID_OK)
        {
            FLUID_LOG(FLUID_ERR, "Failed to reset LADSPA, unable to change sample rate");
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    fx->sample_rate = new_sample_rate;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Check if the LADSPA engine is currently used to render audio
 *
 * @param fx LADSPA fx instance
 * @return TRUE if LADSPA effects engine is active, otherwise FALSE
 *
 * If an engine is active, the only allowed user actions are deactivation or
 * setting values of  control ports on effects. Anything else, especially
 * adding or removing effects, buffers or ports, is only allowed in deactivated
 * state.
 */
int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx)
{
    int is_active;

    fluid_return_val_if_fail(fx != NULL, FALSE);

    LADSPA_API_ENTER(fx);

    is_active = (fluid_atomic_int_get(&fx->state) != FLUID_LADSPA_INACTIVE);

    LADSPA_API_RETURN(fx, is_active);
}

/**
 * Activate the LADSPA fx instance and each configured effect.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK if activation succeeded or already active, otherwise FLUID_FAILED
 */
int fluid_ladspa_activate(fluid_ladspa_fx_t *fx)
{
    fluid_list_t *list;
    fluid_ladspa_effect_t *effect;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    if(fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(fluid_ladspa_check(fx, NULL, 0) != FLUID_OK)
    {
        FLUID_LOG(FLUID_ERR, "LADSPA check failed, unable to activate effects");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    for(list = fx->effects; list; list = fluid_list_next(list))
    {
        effect = (fluid_ladspa_effect_t *) fluid_list_get(list);
        activate_effect(effect);
    }

    if(!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_INACTIVE, FLUID_LADSPA_ACTIVE))
    {
        for(list = fx->effects; list; list = fluid_list_next(list))
        {
            effect = (fluid_ladspa_effect_t *) fluid_list_get(list);
            deactivate_effect(effect);
        }

        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Deactivate a LADSPA fx instance and all configured effects.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK if deactivation succeeded, otherwise FLUID_FAILED
 *
 * @note This function may sleep.
 */
int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx)
{
    fluid_list_t *list;
    fluid_ladspa_effect_t *effect;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    /* If we are already inactive, then simply return success */
    if(fluid_atomic_int_get(&fx->state) == FLUID_LADSPA_INACTIVE)
    {
        LADSPA_API_RETURN(fx, FLUID_OK);
    }

    /* Notify fluid_ladspa_run that we would like to deactivate and that it should
     * send us a signal when its done if it is currently running */
    fx->pending_deactivation = 1;

    fluid_cond_mutex_lock(fx->run_finished_mutex);

    while(!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_ACTIVE, FLUID_LADSPA_INACTIVE))
    {
        fluid_cond_wait(fx->run_finished_cond, fx->run_finished_mutex);
    }

    fluid_cond_mutex_unlock(fx->run_finished_mutex);

    /* Now that we're inactive, deactivate all effects and return success */
    for(list = fx->effects; list; list = fluid_list_next(list))
    {
        effect = (fluid_ladspa_effect_t *) fluid_list_get(list);
        deactivate_effect(effect);
    }

    fx->pending_deactivation = 0;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Reset the LADSPA effects engine.
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 *
 * Deactivate LADSPA if currently active, remove all
 * effects, remove all user nodes and unload all libraries.
 */
int fluid_ladspa_reset(fluid_ladspa_fx_t *fx)
{
    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    if(fluid_ladspa_is_active(fx))
    {
        if(fluid_ladspa_deactivate(fx) != FLUID_OK)
        {
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    clear_ladspa(fx);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Processes audio data via the LADSPA effects unit.
 *
 * @param fx LADSPA effects instance
 * @param block_count number of blocks to render
 * @param block_size number of samples in a block
 *
 * FluidSynth calls this function during main output mixing,
 * just before processing the internal reverb and chorus effects.
 *
 * It copies audio data from the supplied buffers, runs all effects and copies the
 * resulting audio back into the same buffers.
 */
void fluid_ladspa_run(fluid_ladspa_fx_t *fx, int block_count, int block_size)
{
    int num_samples;
    fluid_list_t *list;
    fluid_ladspa_node_t *node;
    fluid_ladspa_effect_t *effect;

    /* Somebody wants to deactivate the engine, so let's give them a chance to do that.
     * And check that there is at least one effect, to avoid the overhead of the
     * atomic compare and exchange on an unconfigured LADSPA engine. */
    if(fx->pending_deactivation || fx->effects == NULL)
    {
        return;
    }

    /* Inform the engine that we are now running pluings, and bail out if it's not active */
    if(!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_ACTIVE, FLUID_LADSPA_RUNNING))
    {
        return;
    }

    num_samples = block_count * block_size;

#ifndef WITH_FLOAT
    copy_host_to_effect_buffers(fx, num_samples);
#endif

    for(list = fx->user_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        FLUID_MEMSET(node->effect_buffer, 0, fx->buffer_size * sizeof(LADSPA_Data));
    }

    /* Run each effect in the order that they were added */
    for(list = fx->effects; list; list = fluid_list_next(list))
    {
        effect = (fluid_ladspa_effect_t *) fluid_list_get(list);

        if(effect->mix)
        {
            effect->desc->run_adding(effect->handle, num_samples);
        }
        else
        {
            effect->desc->run(effect->handle, num_samples);
        }
    }

#ifndef WITH_FLOAT
    copy_effect_to_host_buffers(fx, num_samples);
#endif

    if(!fluid_atomic_int_compare_and_exchange(&fx->state, FLUID_LADSPA_RUNNING, FLUID_LADSPA_ACTIVE))
    {
        FLUID_LOG(FLUID_ERR, "Unable to reset LADSPA running state!");
    }

    /* If deactivation was requested while in running state, notify that we've finished now
     * and deactivation can proceed */
    if(fx->pending_deactivation)
    {
        fluid_cond_mutex_lock(fx->run_finished_mutex);
        fluid_cond_broadcast(fx->run_finished_cond);
        fluid_cond_mutex_unlock(fx->run_finished_mutex);
    }
}

/**
 * Check if the effect plugin supports the run_adding and set_run_adding_gain
 * interfaces necessary for output mixing
 *
 * @param fx LADSPA fx
 * @param name the name of the effect
 * @return TRUE if mix mode is supported, otherwise FALSE
 */
int fluid_ladspa_effect_can_mix(fluid_ladspa_fx_t *fx, const char *name)
{
    int can_mix;
    fluid_ladspa_effect_t *effect;

    fluid_return_val_if_fail(fx != NULL, FALSE);
    fluid_return_val_if_fail(name != NULL, FALSE);

    LADSPA_API_ENTER(fx);

    effect = get_effect(fx, name);

    if(effect == NULL)
    {
        LADSPA_API_RETURN(fx, FALSE);
    }

    can_mix = (effect->desc->run_adding != NULL
               && effect->desc->set_run_adding_gain != NULL);

    LADSPA_API_RETURN(fx, can_mix);
}

/**
 * Set if the effect should replace everything in the output buffers (mix = 0, default)
 * or add to the buffers with a fixed gain (mix = 1).
 *
 * @param fx LADSPA fx instance
 * @param name the name of the effect
 * @param mix (boolean) if to enable mix mode
 * @param gain the gain to apply to the effect output before adding to output.
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_effect_set_mix(fluid_ladspa_fx_t *fx, const char *name, int mix, float gain)
{
    fluid_ladspa_effect_t *effect;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    effect = get_effect(fx, name);

    if(effect == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(mix)
    {
        if(!fluid_ladspa_effect_can_mix(fx, name))
        {
            FLUID_LOG(FLUID_ERR, "Effect '%s' does not support mix mode", name);
            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }

        effect->desc->set_run_adding_gain(effect->handle, gain);
    }

    effect->mix = mix;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

static void clear_ladspa(fluid_ladspa_fx_t *fx)
{
    fluid_list_t *list;
    fluid_ladspa_node_t *node;
    fluid_ladspa_effect_t *effect;

    /* Deactivate and free all effects */
    for(list = fx->effects; list; list = fluid_list_next(list))
    {
        effect = (fluid_ladspa_effect_t *) fluid_list_get(list);

        deactivate_effect(effect);
        delete_fluid_ladspa_effect(effect);
    }
    delete_fluid_list(fx->effects);
    fx->effects = NULL;

    /* Delete all user nodes */
    for(list = fx->user_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        delete_fluid_ladspa_node(node);
    }
    delete_fluid_list(fx->user_nodes);
    fx->user_nodes = NULL;

    /* Mark all host nodes as unconnected */
    for(list = fx->host_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        node->num_inputs = 0;
        node->num_outputs = 0;
    }
}

/**
 * Check if a named host port exists
 *
 * @param fx LADSPA fx instance
 * @param name the port name
 * @return TRUE if the host port exists, otherwise FALSE
 */
int fluid_ladspa_host_port_exists(fluid_ladspa_fx_t *fx, const char *name)
{
    fluid_ladspa_node_t *node;

    fluid_return_val_if_fail(fx != NULL, FALSE);
    fluid_return_val_if_fail(name != NULL, FALSE);

    LADSPA_API_ENTER(fx);

    node = get_node(fx, name);

    if(node == NULL)
    {
        LADSPA_API_RETURN(fx, FALSE);
    }

    if(node->type & FLUID_LADSPA_NODE_HOST)
    {
        LADSPA_API_RETURN(fx, TRUE);
    }

    LADSPA_API_RETURN(fx, FALSE);
}

/**
 * Check if a named user buffer exists
 *
 * @param fx LADSPA fx instance
 * @param name the buffer name
 * @return TRUE if the buffer exists, otherwise FALSE
 */
int fluid_ladspa_buffer_exists(fluid_ladspa_fx_t *fx, const char *name)
{
    int exists;
    fluid_ladspa_node_t *node;

    fluid_return_val_if_fail(fx != NULL, FALSE);
    fluid_return_val_if_fail(name != NULL, FALSE);

    LADSPA_API_ENTER(fx);

    node = get_node(fx, name);

    if(node == NULL)
    {
        LADSPA_API_RETURN(fx, FALSE);
    }

    exists = ((node->type & FLUID_LADSPA_NODE_AUDIO) &&
              (node->type & FLUID_LADSPA_NODE_USER));

    LADSPA_API_RETURN(fx, exists);
}

/**
 * Check if the named port exists on an effect
 *
 * @param fx LADSPA fx instance
 * @param effect_name name of the effect
 * @param port_name the port name
 * @return TRUE if port was found, otherwise FALSE
 */
int fluid_ladspa_effect_port_exists(fluid_ladspa_fx_t *fx, const char *effect_name,
                                    const char *port_name)
{
    fluid_ladspa_effect_t *effect;
    int port_exists;

    fluid_return_val_if_fail(fx != NULL, FALSE);
    fluid_return_val_if_fail(effect_name != NULL, FALSE);
    fluid_return_val_if_fail(port_name != NULL, FALSE);

    LADSPA_API_ENTER(fx);

    effect = get_effect(fx, effect_name);

    if(effect == NULL)
    {
        LADSPA_API_RETURN(fx, FALSE);
    }

    port_exists = (get_effect_port_idx(effect, port_name) != -1);

    LADSPA_API_RETURN(fx, port_exists);
}

/**
 * Create and add a new audio buffer.
 *
 * @param fx LADSPA effects instance
 * @param name name of the new buffer
 * @return FLUID_OK on success, FLUID_FAILED on error
 */
int fluid_ladspa_add_buffer(fluid_ladspa_fx_t *fx, const char *name)
{
    fluid_ladspa_node_t *node;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    if(fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = new_fluid_ladspa_node(fx, name,
                                 FLUID_LADSPA_NODE_AUDIO | FLUID_LADSPA_NODE_USER, NULL);

    if(node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    fx->user_nodes = fluid_list_append(fx->user_nodes, node);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Set the value of an effect control port
 *
 * @param fx LADSPA fx instance
 * @param effect_name name of the effect
 * @param port_name name of the port
 * @param val floating point value
 * @return FLUID_OK on success, FLUID_FAILED on error
 */
int fluid_ladspa_effect_set_control(fluid_ladspa_fx_t *fx, const char *effect_name,
                                    const char *port_name, float val)
{
    fluid_ladspa_node_t *node;
    fluid_ladspa_effect_t *effect;
    int port_idx;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(effect_name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(port_name != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    effect = get_effect(fx, effect_name);

    if(effect == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_idx = get_effect_port_idx(effect, port_name);

    if(port_idx < 0)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(!LADSPA_IS_PORT_CONTROL(effect->desc->PortDescriptors[port_idx]))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = effect->port_nodes[port_idx];

    if(node == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node->effect_buffer[0] = val;

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Create an instance of a LADSPA plugin as an effect
 *
 * @param fx LADSPA effects instance
 * @param effect_name name of the effect
 * @param lib_name filename of ladspa plugin library
 * @param plugin_name optional, plugin name if there is more than one plugin in the library
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
int fluid_ladspa_add_effect(fluid_ladspa_fx_t *fx, const char *effect_name,
                            const char *lib_name, const char *plugin_name)
{
    fluid_ladspa_effect_t *effect;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(effect_name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(lib_name != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    if(fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    effect = new_fluid_ladspa_effect(fx, lib_name, plugin_name);

    if(effect == NULL)
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    effect->name = FLUID_STRDUP(effect_name);

    if(effect->name == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        delete_fluid_ladspa_effect(effect);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(create_control_port_nodes(fx, effect) != FLUID_OK)
    {
        delete_fluid_ladspa_effect(effect);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    fx->effects = fluid_list_append(fx->effects, effect);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Connect an effect audio port to a host port or buffer
 *
 * @param fx LADSPA effects instance
 * @param effect_name name of the effect
 * @param port_name the audio port name to connect to (case-insensitive prefix match)
 * @param name the host port or buffer to connect to (case-insensitive)
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 *
 * @note There is no corresponding disconnect function. If the connections need to be changed,
 * clear everything with fluid_ladspa_reset and start again from scratch.
 */
int fluid_ladspa_effect_link(fluid_ladspa_fx_t *fx, const char *effect_name,
                             const char *port_name, const char *name)
{
    fluid_ladspa_effect_t *effect;
    fluid_ladspa_node_t *node;
    int port_idx;
    int port_flags;
    int dir;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(effect_name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(port_name != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(name != NULL, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    if(fluid_ladspa_is_active(fx))
    {
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    effect = get_effect(fx, effect_name);

    if(effect == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Effect '%s' not found", effect_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_idx = get_effect_port_idx(effect, port_name);

    if(port_idx < 0)
    {
        FLUID_LOG(FLUID_ERR, "Port '%s' not found on effect '%s'", port_name, effect_name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    port_flags = effect->desc->PortDescriptors[port_idx];

    if(!LADSPA_IS_PORT_AUDIO(port_flags))
    {
        FLUID_LOG(FLUID_ERR, "Only audio effect ports can be linked to buffers or host ports");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    node = get_node(fx, name);

    if(node == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Link target '%s' not found", name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(!(node->type & FLUID_LADSPA_NODE_AUDIO))
    {
        FLUID_LOG(FLUID_ERR, "Link target '%s' needs to be an audio port or buffer", name);
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(LADSPA_IS_PORT_INPUT(port_flags))
    {
        dir = FLUID_LADSPA_INPUT;
    }
    else
    {
        dir = FLUID_LADSPA_OUTPUT;
    }

    connect_node_to_port(node, dir, effect, port_idx);

    LADSPA_API_RETURN(fx, FLUID_OK);
}

/**
 * Do a sanity check for problems in the LADSPA setup
 *
 * @param fx LADSPA fx instance
 * @param err externally provided buffer for the error message. Set to NULL if you don't need an error message.
 * @param err_size size of the err buffer
 * @return FLUID_OK if setup is valid, otherwise FLUID_FAILED (err will contain the error message)
 *
 * If the check detects problems and the err pointer is not NULL, a description of the first found
 * problem is written to that string (up to err_size - 1 characters).
 */
int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size)
{
    const char *str;
    const char *str2;
    fluid_list_t *list;
    fluid_ladspa_effect_t *effect;

    fluid_return_val_if_fail(fx != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(err == NULL || err_size >= 0, FLUID_FAILED);

    LADSPA_API_ENTER(fx);

    /* Check that there is at least one effect */
    if(fx->effects == NULL)
    {
        FLUID_SNPRINTF(err, err_size, "No effects configured\n");
        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    for(list = fx->effects; list; list = fluid_list_next(list))
    {
        effect = (fluid_ladspa_effect_t *) fluid_list_get(list);

        if(check_all_ports_connected(effect, &str) == FLUID_FAILED)
        {
            if(err != NULL)
            {
                FLUID_SNPRINTF(err, err_size, "Port '%s' on effect '%s' is not connected\n",
                               str, effect->name);
            }

            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }

        if(check_no_inplace_broken(effect, &str, &str2) == FLUID_FAILED)
        {
            if(err != NULL)
            {
                FLUID_SNPRINTF(err, err_size,
                               "effect '%s' is in-place broken, '%s' and '%s' are not allowed "
                               "to connect to the same node\n", effect->name, str, str2);
            }

            LADSPA_API_RETURN(fx, FLUID_FAILED);
        }
    }

    if(check_host_output_used(fx) == FLUID_FAILED)
    {
        if(err != NULL)
        {
            FLUID_SNPRINTF(err, err_size, "No effect outputs to one the host nodes\n");
        }

        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    if(check_all_audio_nodes_connected(fx, &str) == FLUID_FAILED)
    {
        if(err != NULL)
        {
            FLUID_SNPRINTF(err, err_size, "Audio node '%s' is not fully connected\n", str);
        }

        LADSPA_API_RETURN(fx, FLUID_FAILED);
    }

    LADSPA_API_RETURN(fx, FLUID_OK);
}


static void activate_effect(fluid_ladspa_effect_t *effect)
{
    if(!effect->active)
    {
        effect->active = TRUE;

        if(effect->desc->activate != NULL)
        {
            effect->desc->activate(effect->handle);
        }
    }
}

static void deactivate_effect(fluid_ladspa_effect_t *effect)
{
    if(effect->active)
    {
        effect->active = FALSE;

        if(effect->desc->deactivate != NULL)
        {
            effect->desc->deactivate(effect->handle);
        }
    }
}

/**
 * Return a LADSPA node by name. Nodes are searched case insensitive.
 *
 * @param fx LADSPA fx instance
 * @param name the node name string
 * @return a fluid_ladspa_node_t pointer or NULL if not found
 */
static fluid_ladspa_node_t *get_node(fluid_ladspa_fx_t *fx, const char *name)
{
    fluid_list_t *list;
    fluid_ladspa_node_t *node;

    for(list = fx->host_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        if(FLUID_STRCASECMP(node->name, name) == 0)
        {
            return node;
        }
    }

    for(list = fx->user_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        if(FLUID_STRCASECMP(node->name, name) == 0)
        {
            return node;
        }
    }

    return NULL;
}

/**
 * Return a LADSPA effect port index by name, using a 'fuzzy match'.
 *
 * @param effect pointer to fluid_ladspa_effect_t
 * @param name the port name
 * @return index of the port in the effect or -1 on error
 *
 * Returns the first effect port which matches the name. If no exact match is
 * found, returns the port that starts with the specified name, but only if there is
 * only one such match.
 */
static int get_effect_port_idx(const fluid_ladspa_effect_t *effect, const char *name)
{
    unsigned int i;
    int port = -1;

    for(i = 0; i < effect->desc->PortCount; i++)
    {
        if(FLUID_STRNCASECMP(effect->desc->PortNames[i], name, FLUID_STRLEN(name)) == 0)
        {
            /* exact match, return immediately */
            if(FLUID_STRLEN(effect->desc->PortNames[i]) == FLUID_STRLEN(name))
            {
                return i;
            }

            /* more than one prefix match should be treated as not found */
            if(port != -1)
            {
                return -1;
            }

            port = i;
        }
    }

    return port;
}

/**
 * Return a LADSPA descriptor structure for a plugin in a LADSPA library.
 *
 * @param lib pointer to the dynamically loaded library
 * @param name name (LADSPA Label) of the plugin
 * @return pointer to LADSPA_Descriptor, NULL on error or if not found
 *
 * The name is optional if the library contains only one plugin.
 */
static const LADSPA_Descriptor *get_plugin_descriptor(fluid_module_t *lib, const char *name)
{
    const LADSPA_Descriptor *desc;
    const LADSPA_Descriptor *last_desc = NULL;
    LADSPA_Descriptor_Function ladspa_descriptor;
    int i;

    if(!fluid_module_symbol(lib, "ladspa_descriptor", (void *)&ladspa_descriptor))
    {
        FLUID_LOG(FLUID_ERR, "Unable to find ladspa_descriptor in '%s'. "
                  "Is this really a LADSPA plugin?", fluid_module_name(lib));
        return NULL;
    }

    for(i = 0; /* endless */; i++)
    {
        desc = ladspa_descriptor(i);

        if(desc == NULL)
        {
            break;
        }

        if(name != NULL && FLUID_STRCMP(desc->Label, name) == 0)
        {
            return desc;
        }

        last_desc = desc;
    }

    if(name == NULL)
    {
        if(i == 1)
        {
            return last_desc;
        }

        FLUID_LOG(FLUID_ERR, "Library contains more than one plugin, please specify "
                  "the plugin label");
    }

    return NULL;
}

/**
 * Instantiate a LADSPA plugin from a library and set up the associated
 * control structures needed by the LADSPA fx engine.
 *
 * @param fx LADSPA fx instance
 * @param lib_name file path of the plugin library
 * @param plugin_name (optional) string name of the plugin (the LADSPA Label)
 * @return pointer to the new ladspa_plugin_t structure or NULL on error
 *
 * If the library contains only one plugin, then the name is optional.
 * Plugins are identified by their "Label" in the plugin descriptor structure.
 */
static fluid_ladspa_effect_t *
new_fluid_ladspa_effect(fluid_ladspa_fx_t *fx, const char *lib_name, const char *plugin_name)
{
    fluid_ladspa_effect_t *effect;

    effect = FLUID_NEW(fluid_ladspa_effect_t);

    if(effect == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(effect, 0, sizeof(fluid_ladspa_effect_t));

    effect->lib = fluid_module_open(lib_name);

    if(effect->lib == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Unable to load LADSPA library '%s': %s", lib_name,
                  fluid_module_error());
        delete_fluid_ladspa_effect(effect);
        return NULL;
    }

    effect->desc = get_plugin_descriptor(effect->lib, plugin_name);

    if(effect->desc == NULL)
    {
        delete_fluid_ladspa_effect(effect);
        return NULL;
    }

    effect->handle = effect->desc->instantiate(effect->desc, fx->sample_rate);

    if(effect->handle == NULL)
    {
        delete_fluid_ladspa_effect(effect);
        FLUID_LOG(FLUID_ERR, "Unable to instantiate plugin '%s' from '%s'", plugin_name, lib_name);
        return NULL;
    }

    effect->port_nodes = FLUID_ARRAY(fluid_ladspa_node_t *, effect->desc->PortCount);

    if(effect->port_nodes == NULL)
    {
        delete_fluid_ladspa_effect(effect);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(effect->port_nodes, 0, effect->desc->PortCount * sizeof(fluid_ladspa_node_t *));

    return effect;
}

static void delete_fluid_ladspa_effect(fluid_ladspa_effect_t *effect)
{
    unsigned int i;
    fluid_ladspa_node_t *node;

    fluid_return_if_fail(effect != NULL);

    /* Control nodes are created automatically when the effect is instantiated and
     * are private to this effect, so we can safely remove them here. Nodes connected
     * to audio ports might be connected to other effects as well, so we simply remove
     * any pointers to them from the effect. */
    if(effect->desc != NULL)
    {
        for(i = 0; i < effect->desc->PortCount; i++)
        {
            node = (fluid_ladspa_node_t *) effect->port_nodes[i];

            if(node && node->type & FLUID_LADSPA_NODE_CONTROL)
            {
                delete_fluid_ladspa_node(node);
            }
        }
    }

    FLUID_FREE(effect->port_nodes);

    if(effect->handle != NULL && effect->desc != NULL && effect->desc->cleanup != NULL)
    {
        effect->desc->cleanup(effect->handle);
    }

    if(effect->lib != NULL)
    {
        fluid_module_close(effect->lib);
    }

    FLUID_FREE(effect->name);
    FLUID_FREE(effect);
}

static fluid_ladspa_node_t *new_fluid_ladspa_node(fluid_ladspa_fx_t *fx, const char *name,
        fluid_ladspa_node_type_t type, fluid_real_t *host_buffer)
{
    int buffer_size;
    fluid_ladspa_node_t *node;

    /* For named nodes, make sure that the name is unique */
    if(FLUID_STRLEN(name) > 0 && get_node(fx, name) != NULL)
    {
        return NULL;
    }

    node = FLUID_NEW(fluid_ladspa_node_t);

    if(node == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(node, 0, sizeof(fluid_ladspa_node_t));

    node->name = FLUID_STRDUP(name);

    if(node->name == NULL)
    {
        delete_fluid_ladspa_node(node);
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    node->type = type;
    node->host_buffer = host_buffer;

    /* host audio nodes need a host buffer set */
    if((type & FLUID_LADSPA_NODE_AUDIO) && (type & FLUID_LADSPA_NODE_HOST))
    {
        if(node->host_buffer == NULL)
        {
            delete_fluid_ladspa_node(node);
            return NULL;
        }

#ifdef WITH_FLOAT
        /* If the host uses the same floating-point width as LADSPA, then effects
         * can work in-place on the host buffer. Otherwise well need a separate
         * buffer for type conversion. */
        node->effect_buffer = node->host_buffer;
#endif
    }

    if(node->effect_buffer == NULL)
    {
        /* Control nodes only store a single floating-point value */
        buffer_size = (type & FLUID_LADSPA_NODE_CONTROL) ? 1 : fx->buffer_size;

        node->effect_buffer = FLUID_ARRAY(LADSPA_Data, buffer_size);

        if(node->effect_buffer == NULL)
        {
            delete_fluid_ladspa_node(node);
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return NULL;
        }

        FLUID_MEMSET(node->effect_buffer, 0, buffer_size * sizeof(LADSPA_Data));
    }

    return node;
}

static void delete_fluid_ladspa_node(fluid_ladspa_node_t *node)
{
    fluid_return_if_fail(node != NULL);

    /* If effect_buffer the same as host_buffer, then the effect_buffer has been
     * provided externally, so don't free */
    if((node->effect_buffer != NULL) && ((void *)node->effect_buffer != (void *)node->host_buffer))
    {
        FLUID_FREE(node->effect_buffer);
    }

    FLUID_FREE(node->name);
    FLUID_FREE(node);
}

/**
 * Retrieve a ladspa_effect_t instance by it's name.
 *
 * @param fx LADSPA effects instance
 * @param name effect name
 * @return pointer to effect or NULL if not found
 */
static fluid_ladspa_effect_t *get_effect(fluid_ladspa_fx_t *fx, const char *name)
{
    fluid_list_t *list;
    fluid_ladspa_effect_t *effect;

    LADSPA_API_ENTER(fx);

    for(list = fx->effects; list; list = fluid_list_next(list))
    {
        effect = (fluid_ladspa_effect_t *) fluid_list_get(list);

        if(FLUID_STRNCASECMP(effect->name, name, FLUID_STRLEN(name)) == 0)
        {
            LADSPA_API_RETURN(fx, effect);
        }
    }

    LADSPA_API_RETURN(fx, NULL);
}

/**
 * Set the passed in float pointer to the default value of a effect port, as specified
 * by the LADSPA port hints. If no default hints are found or the port is not a control
 * node, it returns 0.0f;
 *
 * The sample rate is needed because some LADSPA port default hints are expressed as a
 * fraction of the current sample rate.
 *
 * @param effect pointer to effect instance
 * @param port_idx index of the port in the effect
 * @param sample_rate the current sample rate of the LADSPA fx
 * @return default port value or 0.0f
 */
static LADSPA_Data get_default_port_value(fluid_ladspa_effect_t *effect, unsigned int port_idx,
        int sample_rate)
{
    const LADSPA_PortRangeHint *hint;
    LADSPA_PortRangeHintDescriptor flags;
    LADSPA_Data value = 0.0;
    float low_factor = 0.0;
    float high_factor = 0.0;

    if(port_idx >= effect->desc->PortCount)
    {
        return value;
    }

    hint = &effect->desc->PortRangeHints[port_idx];
    flags = hint->HintDescriptor;

    if(!LADSPA_IS_HINT_HAS_DEFAULT(flags))
    {
        return value;
    }

    if(LADSPA_IS_HINT_DEFAULT_0(flags))
    {
        value = 0.0;
    }
    else if(LADSPA_IS_HINT_DEFAULT_1(flags))
    {
        value = 1.0;
    }
    else if(LADSPA_IS_HINT_DEFAULT_100(flags))
    {
        value = 100.0;
    }
    else if(LADSPA_IS_HINT_DEFAULT_440(flags))
    {
        value = 440.0;
    }
    /* defaults based on lower or upper bounds must consider HINT_SAMPLE_RATE */
    else
    {
        if(LADSPA_IS_HINT_DEFAULT_MINIMUM(flags))
        {
            low_factor = 1.0;
            high_factor = 0.0;
        }
        else if(LADSPA_IS_HINT_DEFAULT_LOW(flags))
        {
            low_factor = 0.75;
            high_factor = 0.25;
        }
        else if(LADSPA_IS_HINT_DEFAULT_MIDDLE(flags))
        {
            low_factor = 0.5;
            high_factor = 0.5;
        }
        else if(LADSPA_IS_HINT_DEFAULT_HIGH(flags))
        {
            low_factor = 0.25;
            high_factor = 0.75;
        }
        else if(LADSPA_IS_HINT_DEFAULT_MAXIMUM(flags))
        {
            low_factor = 0.0;
            high_factor = 1.0;
        }

        if(LADSPA_IS_HINT_LOGARITHMIC(flags) && low_factor > 0 && high_factor > 0)
        {
            value = exp(log(hint->LowerBound) * low_factor + log(hint->UpperBound) * high_factor);
        }
        else
        {
            value = (hint->LowerBound * low_factor + hint->UpperBound * high_factor);
        }

        if(LADSPA_IS_HINT_SAMPLE_RATE(flags))
        {
            value *= sample_rate;
        }
    }

    if(LADSPA_IS_HINT_INTEGER(flags))
    {
        /* LADSPA doesn't specify which rounding method to use, so lets keep it simple... */
        value = floor(value + 0.5);
    }

    return value;
}

/**
 * Create a control node for each control port on the passed in effect. The value of the
 * node is taken from port hint defaults, if available. This gets run automatically after
 * an effect has been added.
 *
 * @param fx LADSPA fx instance
 * @param effect effect instance
 * @return FLUID_OK on success, otherwise FLUID_FAILED
 */
static int create_control_port_nodes(fluid_ladspa_fx_t *fx, fluid_ladspa_effect_t *effect)
{
    unsigned int i;
    fluid_ladspa_node_t *node;
    fluid_ladspa_dir_t dir;
    int port_flags;

    for(i = 0; i < effect->desc->PortCount; i++)
    {
        port_flags = effect->desc->PortDescriptors[i];

        if(!LADSPA_IS_PORT_CONTROL(port_flags))
        {
            continue;
        }

        node = new_fluid_ladspa_node(fx, "",
                                     FLUID_LADSPA_NODE_EFFECT | FLUID_LADSPA_NODE_CONTROL, NULL);

        if(node == NULL)
        {
            return FLUID_FAILED;
        }

        node->effect_buffer[0] = get_default_port_value(effect, i, fx->sample_rate);

        dir = (LADSPA_IS_PORT_INPUT(port_flags)) ? FLUID_LADSPA_INPUT : FLUID_LADSPA_OUTPUT;

        connect_node_to_port(node, dir, effect, i);
    }

    return FLUID_OK;
}

static void connect_node_to_port(fluid_ladspa_node_t *node, fluid_ladspa_dir_t dir,
                                 fluid_ladspa_effect_t *effect, int port_idx)
{
    effect->desc->connect_port(effect->handle, port_idx, node->effect_buffer);
    effect->port_nodes[port_idx] = node;

    /* Mark node as connected in the respective direction */
    if(dir == FLUID_LADSPA_INPUT)
    {
        node->num_outputs++;
    }
    else
    {
        node->num_inputs++;
    }
}


/**
 * Check that all ports on the effect are connected to a node.
 *
 * @param effect LADSPA effect instance
 * @param name if check fails, points to the name of first failed port
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_all_ports_connected(fluid_ladspa_effect_t *effect, const char **name)
{
    unsigned int i;

    for(i = 0; i < effect->desc->PortCount; i++)
    {
        if(effect->port_nodes[i] == NULL)
        {
            *name = effect->desc->PortNames[i];
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

/**
 * In-place broken plugins can't cope with input and output audio ports connected
 * to the same buffer. Check for this condition in the effect.
 *
 * @param effect effect instance
 * @param name1 if check fails, points to the first port name
 * @param name2 if check fails, points to the second port name
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_no_inplace_broken(fluid_ladspa_effect_t *effect, const char **name1, const char **name2)
{
    unsigned int i, k;
    LADSPA_PortDescriptor flags1, flags2;

    if(!LADSPA_IS_INPLACE_BROKEN(effect->desc->Properties))
    {
        return FLUID_OK;
    }

    for(i = 0; i < effect->desc->PortCount; i++)
    {
        flags1 = effect->desc->PortDescriptors[i];

        for(k = 0; k < effect->desc->PortCount; k++)
        {
            flags2 = effect->desc->PortDescriptors[k];

            if(i != k
                    && effect->port_nodes[i]->effect_buffer == effect->port_nodes[k]->effect_buffer
                    && (flags1 & 0x3) != (flags2 & 0x3) /* first two bits encode direction */
                    && LADSPA_IS_PORT_AUDIO(flags1) && LADSPA_IS_PORT_AUDIO(flags2))
            {
                *name1 = effect->desc->PortNames[i];
                *name2 = effect->desc->PortNames[k];
                return FLUID_FAILED;
            }
        }
    }

    return FLUID_OK;
}

/**
 * Check that at least one host node is used by an effect
 *
 * @param fx LADSPA fx instance
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_host_output_used(fluid_ladspa_fx_t *fx)
{
    fluid_list_t *list;
    fluid_ladspa_node_t *node;

    for(list = fx->host_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        if(node->num_inputs)
        {
            return FLUID_OK;
        }
    }

    return FLUID_FAILED;
}

/**
 * Check that all user audio nodes have an input and an output
 *
 * @param fx LADSPA fx instance
 * @param name if check fails, points to the name of first failed node
 * @return FLUID_OK on successful check, otherwise FLUID_FAILED
 */
static int check_all_audio_nodes_connected(fluid_ladspa_fx_t *fx, const char **name)
{
    fluid_list_t *list;
    fluid_ladspa_node_t *node;

    for(list = fx->user_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        if(node->num_inputs == 0 || node->num_outputs == 0)
        {
            *name = node->name;
            return FLUID_FAILED;
        }
    }

    return FLUID_OK;
}

#ifndef WITH_FLOAT
/**
 * Copy and type convert host buffers to effect buffers. Used only if host and LADSPA
 * use different float types.
 */
static FLUID_INLINE void copy_host_to_effect_buffers(fluid_ladspa_fx_t *fx, int num_samples)
{
    int i;
    fluid_list_t *list;
    fluid_ladspa_node_t  *node;

    for(list = fx->host_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        /* Only copy host nodes that have at least one output or output, i.e.
         * that are connected to at least one effect port. */
        if(node->num_inputs > 0 || node->num_outputs > 0)
        {
            for(i = 0; i < num_samples; i++)
            {
                node->effect_buffer[i] = (LADSPA_Data)node->host_buffer[i];
            }
        }
    }
}

/**
 * Copy and type convert effect buffers to host buffers. Used only if host and LADSPA
 * use different float types.
 */
static FLUID_INLINE void copy_effect_to_host_buffers(fluid_ladspa_fx_t *fx, int num_samples)
{
    int i;
    fluid_list_t *list;
    fluid_ladspa_node_t  *node;

    for(list = fx->host_nodes; list; list = fluid_list_next(list))
    {
        node = (fluid_ladspa_node_t *) fluid_list_get(list);

        /* Only copy effect nodes that have at least one input, i.e. that are connected to
         * at least one effect output */
        if(node->num_inputs > 0)
        {
            for(i = 0; i < num_samples; i++)
            {
                node->host_buffer[i] = (fluid_real_t)node->effect_buffer[i];
            }
        }
    }
}
#endif /* WITH_FLOAT */

#else /* ifdef LADSPA */

/* Dummy functions to use if LADSPA is not compiled in, to keep the
 * FluidSynth library ABI stable */

int fluid_ladspa_is_active(fluid_ladspa_fx_t *fx)
{
    return FALSE;
}

int fluid_ladspa_activate(fluid_ladspa_fx_t *fx)
{
    return FLUID_FAILED;
}

int fluid_ladspa_deactivate(fluid_ladspa_fx_t *fx)
{
    return FLUID_FAILED;
}

int fluid_ladspa_reset(fluid_ladspa_fx_t *fx)
{
    return FLUID_FAILED;
}

int fluid_ladspa_check(fluid_ladspa_fx_t *fx, char *err, int err_size)
{
    return FLUID_FAILED;
}

int fluid_ladspa_host_port_exists(fluid_ladspa_fx_t *fx, const char *name)
{
    return FALSE;
}

int fluid_ladspa_add_buffer(fluid_ladspa_fx_t *fx, const char *name)
{
    return FLUID_FAILED;
}

int fluid_ladspa_buffer_exists(fluid_ladspa_fx_t *fx, const char *name)
{
    return FALSE;
}

int fluid_ladspa_add_effect(fluid_ladspa_fx_t *fx, const char *effect_name,
                            const char *lib_name, const char *plugin_name)
{
    return FLUID_FAILED;
}

int fluid_ladspa_effect_can_mix(fluid_ladspa_fx_t *fx, const char *name)
{
    return FALSE;
}

int fluid_ladspa_effect_set_mix(fluid_ladspa_fx_t *fx, const char *name, int mix, float gain)
{
    return FLUID_FAILED;
}

int fluid_ladspa_effect_port_exists(fluid_ladspa_fx_t *fx, const char *effect_name, const char *port_name)
{
    return FALSE;
}

int fluid_ladspa_effect_set_control(fluid_ladspa_fx_t *fx, const char *effect_name,
                                    const char *port_name, float val)
{
    return FLUID_FAILED;
}

int fluid_ladspa_effect_link(fluid_ladspa_fx_t *fx, const char *effect_name,
                             const char *port_name, const char *name)
{
    return FLUID_FAILED;
}

#endif /* ifdef LADSPA */
