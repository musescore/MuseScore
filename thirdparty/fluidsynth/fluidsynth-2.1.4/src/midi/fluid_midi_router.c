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

/* Original author: Markus Nentwig, nentwig@users.sourceforge.net
 *
 * Josh Green made it general purpose with a complete usable public API and
 * cleaned it up a bit.
 */

#include "fluid_midi_router.h"
#include "fluid_midi.h"
#include "fluid_synth.h"

/*
 * fluid_midi_router
 */
struct _fluid_midi_router_t
{
    fluid_mutex_t rules_mutex;
    fluid_midi_router_rule_t *rules[FLUID_MIDI_ROUTER_RULE_COUNT];        /* List of rules for each rule type */
    fluid_midi_router_rule_t *free_rules;      /* List of rules to free (was waiting for final events which were received) */

    handle_midi_event_func_t event_handler;    /* Callback function for generated events */
    void *event_handler_data;                  /* One arg for the callback */

    int nr_midi_channels;                      /* For clipping the midi channel */
};

struct _fluid_midi_router_rule_t
{
    int chan_min;                            /* Channel window, for which this rule is valid */
    int chan_max;
    fluid_real_t chan_mul;                   /* Channel multiplier (usually 0 or 1) */
    int chan_add;                            /* Channel offset */

    int par1_min;                            /* Parameter 1 window, for which this rule is valid */
    int par1_max;
    fluid_real_t par1_mul;
    int par1_add;

    int par2_min;                            /* Parameter 2 window, for which this rule is valid */
    int par2_max;
    fluid_real_t par2_mul;
    int par2_add;

    int pending_events;                      /* In case of noteon: How many keys are still down? */
    char keys_cc[128];                       /* Flags, whether a key is down / controller is set (sustain) */
    fluid_midi_router_rule_t *next;          /* next entry */
    int waiting;                             /* Set to TRUE when rule has been deactivated but there are still pending_events */
};


/**
 * Create a new midi router.  The default rules will pass all events unmodified.
 * @param settings Settings used to configure MIDI router
 * @param handler MIDI event callback.
 * @param event_handler_data Caller defined data pointer which gets passed to 'handler'
 * @return New MIDI router instance or NULL on error
 *
 * The MIDI handler callback should process the possibly filtered/modified MIDI
 * events from the MIDI router and forward them on to a synthesizer for example.
 * The function fluid_synth_handle_midi_event() can be used for \a handle and
 * a #fluid_synth_t passed as the \a event_handler_data parameter for this purpose.
 */
fluid_midi_router_t *
new_fluid_midi_router(fluid_settings_t *settings, handle_midi_event_func_t handler,
                      void *event_handler_data)
{
    fluid_midi_router_t *router = NULL;
    int i;

    router = FLUID_NEW(fluid_midi_router_t);

    if(router == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(router, 0, sizeof(fluid_midi_router_t));

    /* Retrieve the number of MIDI channels for range limiting */
    fluid_settings_getint(settings, "synth.midi-channels", &router->nr_midi_channels);

    fluid_mutex_init(router->rules_mutex);

    router->event_handler = handler;
    router->event_handler_data = event_handler_data;

    /* Create default routing rules which pass all events unmodified */
    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        router->rules[i] = new_fluid_midi_router_rule();

        if(!router->rules[i])
        {
            goto error_recovery;
        }
    }

    return router;

error_recovery:
    delete_fluid_midi_router(router);
    return NULL;
}

/**
 * Delete a MIDI router instance.
 * @param router MIDI router to delete
 * @return Returns #FLUID_OK on success, #FLUID_FAILED otherwise (only if NULL
 *   \a router passed really)
 */
void
delete_fluid_midi_router(fluid_midi_router_t *router)
{
    fluid_midi_router_rule_t *rule;
    fluid_midi_router_rule_t *next_rule;
    int i;

    fluid_return_if_fail(router != NULL);

    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        for(rule = router->rules[i]; rule; rule = next_rule)
        {
            next_rule = rule->next;
            FLUID_FREE(rule);
        }
    }

    fluid_mutex_destroy(router->rules_mutex);
    FLUID_FREE(router);
}

/**
 * Set a MIDI router to use default "unity" rules. Such a router will pass all
 * events unmodified.
 * @param router Router to set to default rules.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 */
int
fluid_midi_router_set_default_rules(fluid_midi_router_t *router)
{
    fluid_midi_router_rule_t *new_rules[FLUID_MIDI_ROUTER_RULE_COUNT];
    fluid_midi_router_rule_t *del_rules[FLUID_MIDI_ROUTER_RULE_COUNT];
    fluid_midi_router_rule_t *rule, *next_rule, *prev_rule;
    int i, i2;

    fluid_return_val_if_fail(router != NULL, FLUID_FAILED);

    /* Allocate new default rules outside of lock */

    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        new_rules[i] = new_fluid_midi_router_rule();

        if(!new_rules[i])
        {
            /* Free already allocated rules */
            for(i2 = 0; i2 < i; i2++)
            {
                delete_fluid_midi_router_rule(new_rules[i2]);
            }

            return FLUID_FAILED;
        }
    }


    fluid_mutex_lock(router->rules_mutex);        /* ++ lock */

    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        del_rules[i] = NULL;
        prev_rule = NULL;

        /* Process existing rules */
        for(rule = router->rules[i]; rule; rule = next_rule)
        {
            next_rule = rule->next;

            if(rule->pending_events == 0)     /* Rule has no pending events? */
            {
                /* Remove rule from rule list */
                if(prev_rule)
                {
                    prev_rule->next = next_rule;
                }
                else if(rule == router->rules[i])
                {
                    router->rules[i] = next_rule;
                }

                /* Prepend to delete list */
                rule->next = del_rules[i];
                del_rules[i] = rule;
            }
            else
            {
                rule->waiting = TRUE;          /* Pending events, mark as waiting */
                prev_rule = rule;
            }
        }

        /* Prepend new default rule */
        new_rules[i]->next = router->rules[i];
        router->rules[i] = new_rules[i];
    }

    fluid_mutex_unlock(router->rules_mutex);      /* -- unlock */


    /* Free old rules outside of lock */

    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        for(rule = del_rules[i]; rule; rule = next_rule)
        {
            next_rule = rule->next;
            FLUID_FREE(rule);
        }
    }

    return FLUID_OK;
}

/**
 * Clear all rules in a MIDI router. Such a router will drop all events until
 * rules are added.
 * @param router Router to clear all rules from
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 * @since 1.1.0
 */
int
fluid_midi_router_clear_rules(fluid_midi_router_t *router)
{
    fluid_midi_router_rule_t *del_rules[FLUID_MIDI_ROUTER_RULE_COUNT];
    fluid_midi_router_rule_t *rule, *next_rule, *prev_rule;
    int i;

    fluid_return_val_if_fail(router != NULL, FLUID_FAILED);

    fluid_mutex_lock(router->rules_mutex);        /* ++ lock */

    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        del_rules[i] = NULL;
        prev_rule = NULL;

        /* Process existing rules */
        for(rule = router->rules[i]; rule; rule = next_rule)
        {
            next_rule = rule->next;

            if(rule->pending_events == 0)     /* Rule has no pending events? */
            {
                /* Remove rule from rule list */
                if(prev_rule)
                {
                    prev_rule->next = next_rule;
                }
                else if(rule == router->rules[i])
                {
                    router->rules[i] = next_rule;
                }

                /* Prepend to delete list */
                rule->next = del_rules[i];
                del_rules[i] = rule;
            }
            else
            {
                rule->waiting = TRUE;           /* Pending events, mark as waiting */
                prev_rule = rule;
            }
        }
    }

    fluid_mutex_unlock(router->rules_mutex);      /* -- unlock */


    /* Free old rules outside of lock */

    for(i = 0; i < FLUID_MIDI_ROUTER_RULE_COUNT; i++)
    {
        for(rule = del_rules[i]; rule; rule = next_rule)
        {
            next_rule = rule->next;
            FLUID_FREE(rule);
        }
    }

    return FLUID_OK;
}

/**
 * Add a rule to a MIDI router.
 * @param router MIDI router
 * @param rule Rule to add (used directly and should not be accessed again following a
 *   successful call to this function).
 * @param type The type of rule to add (#fluid_midi_router_rule_type)
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise (invalid rule for example)
 * @since 1.1.0
 */
int
fluid_midi_router_add_rule(fluid_midi_router_t *router, fluid_midi_router_rule_t *rule,
                           int type)
{
    fluid_midi_router_rule_t *free_rules, *next_rule;

    fluid_return_val_if_fail(router != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(rule != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(type >= 0 && type < FLUID_MIDI_ROUTER_RULE_COUNT, FLUID_FAILED);


    fluid_mutex_lock(router->rules_mutex);        /* ++ lock */

    /* Take over free rules list, if any (to free outside of lock) */
    free_rules = router->free_rules;
    router->free_rules = NULL;

    rule->next = router->rules[type];
    router->rules[type] = rule;

    fluid_mutex_unlock(router->rules_mutex);      /* -- unlock */


    /* Free any deactivated rules which were waiting for events and are now done */

    for(; free_rules; free_rules = next_rule)
    {
        next_rule = free_rules->next;
        FLUID_FREE(free_rules);
    }

    return FLUID_OK;
}

/**
 * Create a new MIDI router rule.
 * @return Newly allocated router rule or NULL if out of memory.
 * @since 1.1.0
 *
 * The new rule is a "unity" rule which will accept any values and wont modify
 * them.
 */
fluid_midi_router_rule_t *
new_fluid_midi_router_rule(void)
{
    fluid_midi_router_rule_t *rule;

    rule = FLUID_NEW(fluid_midi_router_rule_t);

    if(rule == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(rule, 0, sizeof(fluid_midi_router_rule_t));

    rule->chan_min = 0;
    rule->chan_max = 999999;
    rule->chan_mul = 1.0;
    rule->chan_add = 0;
    rule->par1_min = 0;
    rule->par1_max = 999999;
    rule->par1_mul = 1.0;
    rule->par1_add = 0;
    rule->par2_min = 0;
    rule->par2_max = 999999;
    rule->par2_mul = 1.0;
    rule->par2_add = 0;

    return rule;
};

/**
 * Free a MIDI router rule.
 * @param rule Router rule to free
 * @since 1.1.0
 *
 * Note that rules which have been added to a router are managed by the router,
 * so this function should seldom be needed.
 */
void
delete_fluid_midi_router_rule(fluid_midi_router_rule_t *rule)
{
    fluid_return_if_fail(rule != NULL);
    FLUID_FREE(rule);
}

/**
 * Set the channel portion of a rule.
 * @param rule MIDI router rule
 * @param min Minimum value for rule match
 * @param max Maximum value for rule match
 * @param mul Value which is multiplied by matching event's channel value (1.0 to not modify)
 * @param add Value which is added to matching event's channel value (0 to not modify)
 * @since 1.1.0
 *
 * The \a min and \a max parameters define a channel range window to match
 * incoming events to.  If \a min is less than or equal to \a max then an event
 * is matched if its channel is within the defined range (including \a min
 * and \a max). If \a min is greater than \a max then rule is inverted and matches
 * everything except in *between* the defined range (so \a min and \a max would match).
 *
 * The \a mul and \a add values are used to modify event channel values prior to
 * sending the event, if the rule matches.
 */
void
fluid_midi_router_rule_set_chan(fluid_midi_router_rule_t *rule, int min, int max,
                                float mul, int add)
{
    fluid_return_if_fail(rule != NULL);
    rule->chan_min = min;
    rule->chan_max = max;
    rule->chan_mul = mul;
    rule->chan_add = add;
}

/**
 * Set the first parameter portion of a rule.
 * @param rule MIDI router rule
 * @param min Minimum value for rule match
 * @param max Maximum value for rule match
 * @param mul Value which is multiplied by matching event's 1st parameter value (1.0 to not modify)
 * @param add Value which is added to matching event's 1st parameter value (0 to not modify)
 * @since 1.1.0
 *
 * The 1st parameter of an event depends on the type of event.  For note events
 * its the MIDI note #, for CC events its the MIDI control number, for program
 * change events its the MIDI program #, for pitch bend events its the bend value,
 * for channel pressure its the channel pressure value and for key pressure
 * its the MIDI note number.
 *
 * Pitch bend values have a maximum value of 16383 (8192 is pitch bend center) and all
 * other events have a max of 127.  All events have a minimum value of 0.
 *
 * The \a min and \a max parameters define a parameter range window to match
 * incoming events to.  If \a min is less than or equal to \a max then an event
 * is matched if its 1st parameter is within the defined range (including \a min
 * and \a max). If \a min is greater than \a max then rule is inverted and matches
 * everything except in *between* the defined range (so \a min and \a max would match).
 *
 * The \a mul and \a add values are used to modify event 1st parameter values prior to
 * sending the event, if the rule matches.
 */
void
fluid_midi_router_rule_set_param1(fluid_midi_router_rule_t *rule, int min, int max,
                                  float mul, int add)
{
    fluid_return_if_fail(rule != NULL);
    rule->par1_min = min;
    rule->par1_max = max;
    rule->par1_mul = mul;
    rule->par1_add = add;
}

/**
 * Set the second parameter portion of a rule.
 * @param rule MIDI router rule
 * @param min Minimum value for rule match
 * @param max Maximum value for rule match
 * @param mul Value which is multiplied by matching event's 2nd parameter value (1.0 to not modify)
 * @param add Value which is added to matching event's 2nd parameter value (0 to not modify)
 * @since 1.1.0
 *
 * The 2nd parameter of an event depends on the type of event.  For note events
 * its the MIDI velocity, for CC events its the control value and for key pressure
 * events its the key pressure value.  All other types lack a 2nd parameter.
 *
 * All applicable 2nd parameters have the range 0-127.
 *
 * The \a min and \a max parameters define a parameter range window to match
 * incoming events to.  If \a min is less than or equal to \a max then an event
 * is matched if its 2nd parameter is within the defined range (including \a min
 * and \a max). If \a min is greater than \a max then rule is inverted and matches
 * everything except in *between* the defined range (so \a min and \a max would match).
 *
 * The \a mul and \a add values are used to modify event 2nd parameter values prior to
 * sending the event, if the rule matches.
 */
void
fluid_midi_router_rule_set_param2(fluid_midi_router_rule_t *rule, int min, int max,
                                  float mul, int add)
{
    fluid_return_if_fail(rule != NULL);
    rule->par2_min = min;
    rule->par2_max = max;
    rule->par2_mul = mul;
    rule->par2_add = add;
}

/**
 * Handle a MIDI event through a MIDI router instance.
 * @param data MIDI router instance #fluid_midi_router_t, its a void * so that
 *   this function can be used as a callback for other subsystems
 *   (new_fluid_midi_driver() for example).
 * @param event MIDI event to handle
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * Purpose: The midi router is called for each event, that is received
 * via the 'physical' midi input. Each event can trigger an arbitrary number
 * of generated events (one for each rule that matches).
 *
 * In default mode, a noteon event is just forwarded to the synth's 'noteon' function,
 * a 'CC' event to the synth's 'CC' function and so on.
 *
 * The router can be used to:
 * - filter messages (for example: Pass sustain pedal CCs only to selected channels)
 * - split the keyboard (noteon with notenr < x: to ch 1, >x to ch 2)
 * - layer sounds (for each noteon received on ch 1, create a noteon on ch1, ch2, ch3,...)
 * - velocity scaling (for each noteon event, scale the velocity by 1.27 to give DX7 users
 *   a chance)
 * - velocity switching ("v <=100: Angel Choir; V > 100: Hell's Bells")
 * - get rid of aftertouch
 * - ...
 */
int
fluid_midi_router_handle_midi_event(void *data, fluid_midi_event_t *event)
{
    fluid_midi_router_t *router = (fluid_midi_router_t *)data;
    fluid_midi_router_rule_t **rulep, *rule, *next_rule, *prev_rule = NULL;
    int event_has_par2 = 0; /* Flag, indicates that current event needs two parameters */
    int par1_max = 127;     /* Range limit for par1 */
    int par2_max = 127;     /* Range limit for par2 */
    int ret_val = FLUID_OK;

    int chan; /* Channel of the generated event */
    int par1; /* par1 of the generated event */
    int par2;
    int event_par1;
    int event_par2;
    fluid_midi_event_t new_event;

    /* Some keyboards report noteoff through a noteon event with vel=0.
     * Convert those to noteoff to ease processing. */
    if(event->type == NOTE_ON && event->param2 == 0)
    {
        event->type = NOTE_OFF;
        event->param2 = 127;        /* Release velocity */
    }

    fluid_mutex_lock(router->rules_mutex);    /* ++ lock rules */

    /* Depending on the event type, choose the correct list of rules. */
    switch(event->type)
    {
    case NOTE_ON:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_NOTE];
        event_has_par2 = 1;
        break;

    case NOTE_OFF:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_NOTE];
        event_has_par2 = 1;
        break;

    case CONTROL_CHANGE:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_CC];
        event_has_par2 = 1;
        break;

    case PROGRAM_CHANGE:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_PROG_CHANGE];
        break;

    case PITCH_BEND:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_PITCH_BEND];
        par1_max = 16383;
        break;

    case CHANNEL_PRESSURE:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE];
        break;

    case KEY_PRESSURE:
        rulep = &router->rules[FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE];
        event_has_par2 = 1;
        break;

    case MIDI_SYSTEM_RESET:
    case MIDI_SYSEX:
        ret_val = router->event_handler(router->event_handler_data, event);
        fluid_mutex_unlock(router->rules_mutex);   /* -- unlock rules */
        return ret_val;

    default:
        rulep = NULL;    /* Event will not be passed on */
        break;
    }

    /* Loop over rules in the list, looking for matches for this event. */
    for(rule = rulep ? *rulep : NULL; rule; prev_rule = rule, rule = next_rule)
    {
        event_par1 = (int)event->param1;
        event_par2 = (int)event->param2;
        next_rule = rule->next;     /* Rule may get removed from list, so get next here */

        /* Channel window */
        if(rule->chan_min > rule->chan_max)
        {
            /* Inverted rule: Exclude everything between max and min (but not min/max) */
            if(event->channel > rule->chan_max && event->channel < rule->chan_min)
            {
                continue;
            }
        }
        else        /* Normal rule: Exclude everything < max or > min (but not min/max) */
        {
            if(event->channel > rule->chan_max || event->channel < rule->chan_min)
            {
                continue;
            }
        }

        /* Par 1 window */
        if(rule->par1_min > rule->par1_max)
        {
            /* Inverted rule: Exclude everything between max and min (but not min/max) */
            if(event_par1 > rule->par1_max && event_par1 < rule->par1_min)
            {
                continue;
            }
        }
        else        /* Normal rule: Exclude everything < max or > min (but not min/max)*/
        {
            if(event_par1 > rule->par1_max || event_par1 < rule->par1_min)
            {
                continue;
            }
        }

        /* Par 2 window (only applies to event types, which have 2 pars)
         * For noteoff events, velocity switching doesn't make any sense.
         * Velocity scaling might be useful, though.
         */
        if(event_has_par2 && event->type != NOTE_OFF)
        {
            if(rule->par2_min > rule->par2_max)
            {
                /* Inverted rule: Exclude everything between max and min (but not min/max) */
                if(event_par2 > rule->par2_max && event_par2 < rule->par2_min)
                {
                    continue;
                }
            }
            else      /* Normal rule: Exclude everything < max or > min (but not min/max)*/
            {
                if(event_par2 > rule->par2_max || event_par2 < rule->par2_min)
                {
                    continue;
                }
            }
        }

        /* Channel scaling / offset
         * Note: rule->chan_mul will probably be 0 or 1. If it's 0, input from all
         * input channels is mapped to the same synth channel.
         */
        chan = rule->chan_add + (int)((fluid_real_t)event->channel * rule->chan_mul
                     + (fluid_real_t)0.5);

        /* Par 1 scaling / offset */
        par1 = rule->par1_add + (int)((fluid_real_t)event_par1 * rule->par1_mul
                     + (fluid_real_t)0.5);

        /* Par 2 scaling / offset, if applicable */
        if(event_has_par2)
        {
            par2 = rule->par2_add + (int)((fluid_real_t)event_par2 * rule->par2_mul
                         + (fluid_real_t)0.5);
        }
        else
        {
            par2 = 0;
        }

        /* Channel range limiting */
        if(chan < 0)
        {
            chan = 0;
        }
        else if(chan >= router->nr_midi_channels)
        {
            chan = router->nr_midi_channels - 1;
        }

        /* Par1 range limiting */
        if(par1 < 0)
        {
            par1 = 0;
        }
        else if(par1 > par1_max)
        {
            par1 = par1_max;
        }

        /* Par2 range limiting */
        if(event_has_par2)
        {
            if(par2 < 0)
            {
                par2 = 0;
            }
            else if(par2 > par2_max)
            {
                par2 = par2_max;
            }
        }

        /* At this point we have to create an event of event->type on 'chan' with par1 (maybe par2).
         * We keep track on the state of noteon and sustain pedal events. If the application tries
         * to delete a rule, it will only be fully removed, if pending noteoff / pedal off events have
         * arrived. In the meantime while waiting, it will only let through 'negative' events
         * (noteoff or pedal up).
         */
        if(event->type == NOTE_ON || (event->type == CONTROL_CHANGE
                                      && par1 == SUSTAIN_SWITCH && par2 >= 64))
        {
            /* Noteon or sustain pedal down event generated */
            if(rule->keys_cc[par1] == 0)
            {
                rule->keys_cc[par1] = 1;
                rule->pending_events++;
            }
        }
        else if(event->type == NOTE_OFF || (event->type == CONTROL_CHANGE
                                            && par1 == SUSTAIN_SWITCH && par2 < 64))
        {
            /* Noteoff or sustain pedal up event generated */
            if(rule->keys_cc[par1] > 0)
            {
                rule->keys_cc[par1] = 0;
                rule->pending_events--;

                /* Rule is waiting for negative event to be destroyed? */
                if(rule->waiting)
                {
                    if(rule->pending_events == 0)
                    {
                        /* Remove rule from rule list */
                        if(prev_rule)
                        {
                            prev_rule->next = next_rule;
                        }
                        else
                        {
                            *rulep = next_rule;
                        }

                        /* Add to free list */
                        rule->next = router->free_rules;
                        router->free_rules = rule;

                        rule = prev_rule;   /* Set rule to previous rule, which gets assigned to the next prev_rule value (in for() statement) */
                    }

                    goto send_event;      /* Pass the event to complete the cycle */
                }
            }
        }

        /* Rule is still waiting for negative event? (note off or pedal up) */
        if(rule->waiting)
        {
            continue;    /* Skip (rule is inactive except for matching negative event) */
        }

send_event:

        /* At this point it is decided, what is sent to the synth.
         * Create a new event and make the appropriate call */

        fluid_midi_event_set_type(&new_event, event->type);
        fluid_midi_event_set_channel(&new_event, chan);
        new_event.param1 = par1;
        new_event.param2 = par2;

        /* FIXME - What should be done on failure?  For now continue to process events, but return failure to caller. */
        if(router->event_handler(router->event_handler_data, &new_event) != FLUID_OK)
        {
            ret_val = FLUID_FAILED;
        }
    }

    fluid_mutex_unlock(router->rules_mutex);          /* -- unlock rules */

    return ret_val;
}

/**
 * MIDI event callback function to display event information to stdout
 * @param data MIDI router instance
 * @param event MIDI event data
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * An implementation of the #handle_midi_event_func_t function type, used for
 * displaying MIDI event information between the MIDI driver and router to
 * stdout.  Useful for adding into a MIDI router chain for debugging MIDI events.
 */
int fluid_midi_dump_prerouter(void *data, fluid_midi_event_t *event)
{
    switch(event->type)
    {
    case NOTE_ON:
        fprintf(stdout, "event_pre_noteon %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    case NOTE_OFF:
        fprintf(stdout, "event_pre_noteoff %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    case CONTROL_CHANGE:
        fprintf(stdout, "event_pre_cc %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    case PROGRAM_CHANGE:
        fprintf(stdout, "event_pre_prog %i %i\n", event->channel, event->param1);
        break;

    case PITCH_BEND:
        fprintf(stdout, "event_pre_pitch %i %i\n", event->channel, event->param1);
        break;

    case CHANNEL_PRESSURE:
        fprintf(stdout, "event_pre_cpress %i %i\n", event->channel, event->param1);
        break;

    case KEY_PRESSURE:
        fprintf(stdout, "event_pre_kpress %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    default:
        break;
    }

    return fluid_midi_router_handle_midi_event((fluid_midi_router_t *) data, event);
}

/**
 * MIDI event callback function to display event information to stdout
 * @param data MIDI router instance
 * @param event MIDI event data
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * An implementation of the #handle_midi_event_func_t function type, used for
 * displaying MIDI event information between the MIDI driver and router to
 * stdout.  Useful for adding into a MIDI router chain for debugging MIDI events.
 */
int fluid_midi_dump_postrouter(void *data, fluid_midi_event_t *event)
{
    switch(event->type)
    {
    case NOTE_ON:
        fprintf(stdout, "event_post_noteon %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    case NOTE_OFF:
        fprintf(stdout, "event_post_noteoff %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    case CONTROL_CHANGE:
        fprintf(stdout, "event_post_cc %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    case PROGRAM_CHANGE:
        fprintf(stdout, "event_post_prog %i %i\n", event->channel, event->param1);
        break;

    case PITCH_BEND:
        fprintf(stdout, "event_post_pitch %i %i\n", event->channel, event->param1);
        break;

    case CHANNEL_PRESSURE:
        fprintf(stdout, "event_post_cpress %i %i\n", event->channel, event->param1);
        break;

    case KEY_PRESSURE:
        fprintf(stdout, "event_post_kpress %i %i %i\n",
                event->channel, event->param1, event->param2);
        break;

    default:
        break;
    }

    return fluid_synth_handle_midi_event((fluid_synth_t *) data, event);
}
