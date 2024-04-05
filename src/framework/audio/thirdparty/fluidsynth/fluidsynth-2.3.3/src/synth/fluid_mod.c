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

#include "fluid_mod.h"
#include "fluid_chan.h"
#include "fluid_voice.h"

/**
 * Clone the modulators destination, sources, flags and amount.
 *
 * @param mod the modulator to store the copy to
 * @param src the source modulator to retrieve the information from
 *
 * @note The \c next member of \c mod will be left unchanged.
 */
void
fluid_mod_clone(fluid_mod_t *mod, const fluid_mod_t *src)
{
    mod->dest = src->dest;
    mod->src1 = src->src1;
    mod->flags1 = src->flags1;
    mod->src2 = src->src2;
    mod->flags2 = src->flags2;
    mod->amount = src->amount;
}

/**
 * Set a modulator's primary source controller and flags.
 *
 * @param mod The modulator instance
 * @param src Modulator source (#fluid_mod_src or a MIDI controller number)
 * @param flags Flags determining mapping function and whether the source
 *   controller is a general controller (#FLUID_MOD_GC) or a MIDI CC controller
 *   (#FLUID_MOD_CC), see #fluid_mod_flags.
 */
void
fluid_mod_set_source1(fluid_mod_t *mod, int src, int flags)
{
    mod->src1 = src;
    mod->flags1 = flags;
}

/**
 * Set a modulator's secondary source controller and flags.
 *
 * @param mod The modulator instance
 * @param src Modulator source (#fluid_mod_src or a MIDI controller number)
 * @param flags Flags determining mapping function and whether the source
 *   controller is a general controller (#FLUID_MOD_GC) or a MIDI CC controller
 *   (#FLUID_MOD_CC), see #fluid_mod_flags.
 */
void
fluid_mod_set_source2(fluid_mod_t *mod, int src, int flags)
{
    mod->src2 = src;
    mod->flags2 = flags;
}

/**
 * Set the destination effect of a modulator.
 *
 * @param mod The modulator instance
 * @param dest Destination generator (#fluid_gen_type)
 */
void
fluid_mod_set_dest(fluid_mod_t *mod, int dest)
{
    mod->dest = dest;
}

/**
 * Set the scale amount of a modulator.
 *
 * @param mod The modulator instance
 * @param amount Scale amount to assign
 */
void
fluid_mod_set_amount(fluid_mod_t *mod, double amount)
{
    mod->amount = (double) amount;
}

/**
 * Get the primary source value from a modulator.
 *
 * @param mod The modulator instance
 * @return The primary source value (#fluid_mod_src or a MIDI CC controller value).
 */
int
fluid_mod_get_source1(const fluid_mod_t *mod)
{
    return mod->src1;
}

/**
 * Get primary source flags from a modulator.
 *
 * @param mod The modulator instance
 * @return The primary source flags (#fluid_mod_flags).
 */
int
fluid_mod_get_flags1(const fluid_mod_t *mod)
{
    return mod->flags1;
}

/**
 * Get the secondary source value from a modulator.
 *
 * @param mod The modulator instance
 * @return The secondary source value (#fluid_mod_src or a MIDI CC controller value).
 */
int
fluid_mod_get_source2(const fluid_mod_t *mod)
{
    return mod->src2;
}

/**
 * Get secondary source flags from a modulator.
 *
 * @param mod The modulator instance
 * @return The secondary source flags (#fluid_mod_flags).
 */
int
fluid_mod_get_flags2(const fluid_mod_t *mod)
{
    return mod->flags2;
}

/**
 * Get destination effect from a modulator.
 *
 * @param mod The modulator instance
 * @return Destination generator (#fluid_gen_type)
 */
int
fluid_mod_get_dest(const fluid_mod_t *mod)
{
    return mod->dest;
}

/**
 * Get the scale amount from a modulator.
 *
 * @param mod The modulator instance
 * @return Scale amount
 */
double
fluid_mod_get_amount(const fluid_mod_t *mod)
{
    return (double) mod->amount;
}

/*
 * retrieves the initial value from the given source of the modulator
 */
static fluid_real_t
fluid_mod_get_source_value(const unsigned char mod_src,
                           const unsigned char mod_flags,
                           fluid_real_t *range,
                           const fluid_voice_t *voice
                          )
{
    const fluid_channel_t *chan = voice->channel;
    fluid_real_t val;

    if(mod_flags & FLUID_MOD_CC)
    {
        val = fluid_channel_get_cc(chan, mod_src);

        if(mod_src == PORTAMENTO_CTRL)
        {
            // an invalid portamento fromkey should be treated as 0 when it's actually used for moulating
            if(!fluid_channel_is_valid_note(val))
            {
                val = 0;
            }
        }
    }
    else
    {
        switch(mod_src)
        {
        case FLUID_MOD_NONE:         /* SF 2.01 8.2.1 item 0: src enum=0 => value is 1 */
            val = *range;
            break;

        case FLUID_MOD_VELOCITY:
            val = fluid_voice_get_actual_velocity(voice);
            break;

        case FLUID_MOD_KEY:
            val = fluid_voice_get_actual_key(voice);
            break;

        case FLUID_MOD_KEYPRESSURE:
            val = fluid_channel_get_key_pressure(chan, voice->key);
            break;

        case FLUID_MOD_CHANNELPRESSURE:
            val = fluid_channel_get_channel_pressure(chan);
            break;

        case FLUID_MOD_PITCHWHEEL:
            val = fluid_channel_get_pitch_bend(chan);
            *range = 0x4000;
            break;

        case FLUID_MOD_PITCHWHEELSENS:
            val = fluid_channel_get_pitch_wheel_sensitivity(chan);
            break;

        default:
            FLUID_LOG(FLUID_ERR, "Unknown modulator source '%d', disabling modulator.", mod_src);
            val = 0.0;
        }
    }

    return val;
}

/**
 * transforms the initial value retrieved by \c fluid_mod_get_source_value into [0.0;1.0]
 */
static fluid_real_t
fluid_mod_transform_source_value(fluid_real_t val, unsigned char mod_flags, const fluid_real_t range)
{
    /* normalized value, i.e. usually in the range [0;1] */
    const fluid_real_t val_norm = val / range;

    /* we could also only switch case the lower nibble of mod_flags, however
     * this would keep us from adding further mod types in the future
     *
     * instead just remove the flag(s) we already took care of
     */
    mod_flags &= ~FLUID_MOD_CC;

    switch(mod_flags/* & 0x0f*/)
    {
    case FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =0 */
        val = val_norm;
        break;

    case FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =1 */
        val = 1.0f - val_norm;
        break;

    case FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =2 */
        val = -1.0f + 2.0f * val_norm;
        break;

    case FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =3 */
        val = 1.0f - 2.0f * val_norm;
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =4 */
        val = fluid_concave(127 * (val_norm));
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =5 */
        val = fluid_concave(127 * (1.0f - val_norm));
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =6 */
        val = (val_norm > 0.5f) ?  fluid_concave(127 * 2 * (val_norm - 0.5f))
              : -fluid_concave(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =7 */
        val = (val_norm > 0.5f) ? -fluid_concave(127 * 2 * (val_norm - 0.5f))
              :  fluid_concave(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =8 */
        val = fluid_convex(127 * (val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =9 */
        val = fluid_convex(127 * (1.0f - val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =10 */
        val = (val_norm > 0.5f) ?  fluid_convex(127 * 2 * (val_norm - 0.5f))
              : -fluid_convex(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =11 */
        val = (val_norm > 0.5f) ? -fluid_convex(127 * 2 * (val_norm - 0.5f))
              :  fluid_convex(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =12 */
        val = (val_norm >= 0.5f) ? 1.0f : 0.0f;
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =13 */
        val = (val_norm >= 0.5f) ? 0.0f : 1.0f;
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =14 */
        val = (val_norm >= 0.5f) ? 1.0f : -1.0f;
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =15 */
        val = (val_norm >= 0.5f) ? -1.0f : 1.0f;
        break;

    /*
     * MIDI CCs only have a resolution of 7 bits. The closer val_norm gets to 1,
     * the less will be the resulting change of the sinus. When using this sin()
     * for scaling the cutoff frequency, there will be no audible difference between
     * MIDI CCs 118 to 127. To avoid this waste of CCs multiply with 0.87
     * (at least for unipolar) which makes sin() never get to 1.0 but to 0.98 which
     * is close enough.
     */
    case FLUID_MOD_SIN | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* custom sin(x) */
        val = FLUID_SIN((FLUID_M_PI / 2.0f * 0.87f) * val_norm);
        break;

    case FLUID_MOD_SIN | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* custom */
        val = FLUID_SIN((FLUID_M_PI / 2.0f * 0.87f) * (1.0f - val_norm));
        break;

    case FLUID_MOD_SIN | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* custom */
        val = (val_norm > 0.5f) ?  FLUID_SIN(FLUID_M_PI * (val_norm - 0.5f))
              : -FLUID_SIN(FLUID_M_PI * (0.5f - val_norm));
        break;

    case FLUID_MOD_SIN | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* custom */
        val = (val_norm > 0.5f) ? -FLUID_SIN(FLUID_M_PI * (val_norm - 0.5f))
              :  FLUID_SIN(FLUID_M_PI * (0.5f - val_norm));
        break;

    default:
        FLUID_LOG(FLUID_ERR, "Unknown modulator type '%d', disabling modulator.", mod_flags);
        val = 0.0f;
        break;
    }

    return val;
}

/*
 * fluid_mod_get_value.
 * Computes and return modulator output following SF2.01
 * (See SoundFont Modulator Controller Model Chapter 9.5).
 *
 * Output = Transform(Amount * Map(primary source input) * Map(secondary source input))
 *
 * Notes:
 * 1)fluid_mod_get_value, ignores the Transform operator. The result is:
 *
 *   Output = Amount * Map(primary source input) * Map(secondary source input)
 *
 * 2)When primary source input (src1) is set to General Controller 'No Controller',
 *   output is forced to 0.
 *
 * 3)When secondary source input (src2) is set to General Controller 'No Controller',
 *   output is forced to +1.0 
 */
fluid_real_t
fluid_mod_get_value(fluid_mod_t *mod, fluid_voice_t *voice)
{
    extern fluid_mod_t default_vel2filter_mod;

    fluid_real_t v1 = 0.0, v2 = 1.0;
    /* The wording of the default modulators refers to a range of 127/128.
     * And the table in section 9.5.3 suggests, that this mapping should be applied
     * to all unipolar and bipolar mappings respectively.
     *
     * Thinking about this further, this is actually pretty clever, as this is properly
     * addresses MIDI Recommended Practice (RP-036) Default Pan Formula
     * "Since MIDI controller values range from 0 to 127, the exact center
     * of the range, 63.5, cannot be represented."
     *
     * When changing the overall range to 127/128 however, the "middle pan" value of 64
     * can be correctly represented.
     */
    fluid_real_t range1 = 128.0, range2 = 128.0;

    /* 'special treatment' for default controller
     *
     *  Reference: SF2.01 section 8.4.2
     *
     * The GM default controller 'vel-to-filter cut off' is not clearly
     * defined: If implemented according to the specs, the filter
     * frequency jumps between vel=63 and vel=64.  To maintain
     * compatibility with existing sound fonts, the implementation is
     * 'hardcoded', it is impossible to implement using only one
     * modulator otherwise.
     *
     * I assume here, that the 'intention' of the paragraph is one
     * octave (1200 cents) filter frequency shift between vel=127 and
     * vel=64.  'amount' is (-2400), at least as long as the controller
     * is set to default.
     *
     * Further, the 'appearance' of the modulator (source enumerator,
     * destination enumerator, flags etc) is different from that
     * described in section 8.4.2, but it matches the definition used in
     * several SF2.1 sound fonts (where it is used only to turn it off).
     * */
    if(fluid_mod_test_identity(mod, &default_vel2filter_mod))
    {
// S. Christian Collins' mod, to stop forcing velocity based filtering
        /*
            if (voice->vel < 64){
              return (fluid_real_t) mod->amount / 2.0;
            } else {
              return (fluid_real_t) mod->amount * (127 - voice->vel) / 127;
            }
        */
        return 0; // (fluid_real_t) mod->amount / 2.0;
    }

// end S. Christian Collins' mod

    /* get the initial value of the first source */
    if(mod->src1 > 0)
    {
        v1 = fluid_mod_get_source_value(mod->src1, mod->flags1, &range1, voice);

        /* transform the input value */
        v1 = fluid_mod_transform_source_value(v1, mod->flags1, range1);
    }
    /* When primary source input (src1) is set to General Controller 'No Controller',
       output is forced to 0.0
    */
    else
    {
        return 0.0;
    }

    /* no need to go further */
    if(v1 == 0.0f)
    {
        return 0.0f;
    }

    /* get the second input source */
    if(mod->src2 > 0)
    {
        v2 = fluid_mod_get_source_value(mod->src2, mod->flags2, &range2, voice);

        /* transform the second input value */
        v2 = fluid_mod_transform_source_value(v2, mod->flags2, range2);
    }
    /* When secondary source input (src2) is set to General Controller 'No Controller',
       output is forced to +1.0
    */
    else
    {
        v2 = 1.0f;
    }

    /* it's as simple as that: */
    return (fluid_real_t) mod->amount * v1 * v2;
}

/**
 * Create a new uninitialized modulator structure.
 *
 * @return New allocated modulator or NULL if out of memory
 */
fluid_mod_t *
new_fluid_mod()
{
    fluid_mod_t *mod = FLUID_NEW(fluid_mod_t);

    if(mod == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    return mod;
}

/**
 * Free a modulator structure.
 *
 * @param mod Modulator to free
 */
void
delete_fluid_mod(fluid_mod_t *mod)
{
    FLUID_FREE(mod);
}

/**
 * Returns the size of the fluid_mod_t structure.
 *
 * @return Size of fluid_mod_t in bytes
 *
 * Useful in low latency scenarios e.g. to allocate a modulator on the stack.
 */
size_t fluid_mod_sizeof()
{
    return sizeof(fluid_mod_t);
}

/**
 * Checks if modulator with source 1 other than CC is FLUID_MOD_NONE.
 *
 * @param mod, modulator.
 * @return TRUE if modulator source 1 other than cc is FLUID_MOD_NONE, FALSE otherwise.
 */
static int
fluid_mod_is_src1_none(const fluid_mod_t *mod)
{
    return(((mod->flags1 & FLUID_MOD_CC) == 0) && (mod->src1 == FLUID_MOD_NONE));
}

/**
 * Checks if modulators source other than CC source is invalid.
 *
 * @param mod, modulator.
 * @param src1_select, source input selection to check.
 *   1 to check src1 source.
 *   0 to check src2 source.
 * @return FALSE if selected modulator source other than cc is invalid, TRUE otherwise.
 *
 * (specs SF 2.01  7.4, 7.8, 8.2.1)
 */
static int
fluid_mod_check_non_cc_source(const fluid_mod_t *mod, unsigned char src1_select)
{
    unsigned char flags, src;

    if(src1_select)
    {
        flags = mod->flags1;
        src = mod->src1;
    }
    else
    {
        flags = mod->flags2;
        src = mod->src2;
    }

    return(((flags & FLUID_MOD_CC) != 0)  /* src is a CC */
           /* SF2.01 section 8.2.1: Constant value */
           || ((src == FLUID_MOD_NONE)
               || (src == FLUID_MOD_VELOCITY)        /* Note-on velocity */
               || (src == FLUID_MOD_KEY)             /* Note-on key number */
               || (src == FLUID_MOD_KEYPRESSURE)     /* Poly pressure */
               || (src == FLUID_MOD_CHANNELPRESSURE) /* Channel pressure */
               || (src == FLUID_MOD_PITCHWHEEL)      /* Pitch wheel */
               || (src == FLUID_MOD_PITCHWHEELSENS)  /* Pitch wheel sensitivity */
              ));
}

/**
 * Checks if modulator CC source is invalid (specs SF 2.01  7.4, 7.8, 8.2.1).
 *
 * @param mod, modulator.
 * @src1_select, source input selection:
 *   1 to check src1 source or
 *   0 to check src2 source.
 * @return FALSE if selected modulator's source CC is invalid, TRUE otherwise.
 */
static int
fluid_mod_check_cc_source(const fluid_mod_t *mod, unsigned char src1_select)
{
    unsigned char flags, src;

    if(src1_select)
    {
        flags = mod->flags1;
        src = mod->src1;
    }
    else
    {
        flags = mod->flags2;
        src = mod->src2;
    }

    return(((flags & FLUID_MOD_CC) == 0)  /* src is non CC */
           || ((src != BANK_SELECT_MSB)
               && (src != BANK_SELECT_LSB)
               && (src != DATA_ENTRY_MSB)
               && (src != DATA_ENTRY_LSB)
               /* is src not NRPN_LSB, NRPN_MSB, RPN_LSB, RPN_MSB */
               && ((src < NRPN_LSB) || (RPN_MSB < src))
               /* is src not ALL_SOUND_OFF, ALL_CTRL_OFF, LOCAL_CONTROL, ALL_NOTES_OFF ? */
               /* is src not OMNI_OFF, OMNI_ON, POLY_OFF, POLY_ON ? */
               && (src < ALL_SOUND_OFF)
               /* CC lsb shouldn't allowed to modulate (spec SF 2.01 - 8.2.1)
                  However, as long fluidsynth will use only CC 7 bits resolution,
                  it is safe to ignore these SF recommendations on CC receive.
                  See explanations in fluid_synth_cc_LOCAL() */
               /* uncomment next line to forbid CC lsb  */
               /* && ((src < 32) || (63 < src)) */
              ));
}

/**
 * Checks valid modulator sources (specs SF 2.01  7.4, 7.8, 8.2.1)
 *
 * @param mod, modulator.
 * @param name,if not NULL, pointer on a string displayed as a warning.
 * @return TRUE if modulator sources src1, src2 are valid, FALSE otherwise.
 */
int fluid_mod_check_sources(const fluid_mod_t *mod, char *name)
{
    static const char invalid_non_cc_src[] =
        "Invalid modulator, using non-CC source %s.src%d=%d";
    static const char invalid_cc_src[] =
        "Invalid modulator, using CC source %s.src%d=%d";
    static const char src1_is_none[] =
        "Modulator with source 1 none %s.src1=%d";

    /* checks valid non cc sources */
    if(!fluid_mod_check_non_cc_source(mod, 1)) /* check src1 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_non_cc_src, name, 1, mod->src1);
        }

        return FALSE;
    }

    /*
      When src1 is non CC source FLUID_MOD_NONE, the modulator is valid but
      the output of this modulator will be forced to 0 at synthesis time.
      Also this modulator cannot be used to overwrite a default modulator (as
      there is no default modulator with src1 source equal to FLUID_MOD_NONE).
      Consequently it is useful to return FALSE to indicate this modulator
      being useless. It will be removed later with others invalid modulators.
    */
    if(fluid_mod_is_src1_none(mod))
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, src1_is_none, name, mod->src1);
        }

        return FALSE;
    }

    if(!fluid_mod_check_non_cc_source(mod, 0)) /* check src2 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_non_cc_src, name, 2, mod->src2);
        }

        return FALSE;
    }

    /* checks valid cc sources */
    if(!fluid_mod_check_cc_source(mod, 1)) /* check src1 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_cc_src, name, 1, mod->src1);
        }

        return FALSE;
    }

    if(!fluid_mod_check_cc_source(mod, 0)) /* check src2 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_cc_src, name, 2, mod->src2);
        }

        return FALSE;
    }

    return TRUE;
}

/**
 * Checks if two modulators are identical in sources, flags and destination.
 *
 * @param mod1 First modulator
 * @param mod2 Second modulator
 * @return TRUE if identical, FALSE otherwise
 *
 * SF2.01 section 9.5.1 page 69, 'bullet' 3 defines 'identical'.
 */
int
fluid_mod_test_identity(const fluid_mod_t *mod1, const fluid_mod_t *mod2)
{
    return mod1->dest == mod2->dest
           && mod1->src1 == mod2->src1
           && mod1->src2 == mod2->src2
           && mod1->flags1 == mod2->flags1
           && mod1->flags2 == mod2->flags2;
}

/**
 * Check if the modulator has the given source.
 *
 * @param mod The modulator instance
 * @param cc Boolean value indicating if ctrl is a CC controller or not
 * @param ctrl The source to check for (if \c cc == FALSE : a value of type #fluid_mod_src, else the value of the MIDI CC to check for)
 *
 * @return TRUE if the modulator has the given source, FALSE otherwise.
 */
int fluid_mod_has_source(const fluid_mod_t *mod, int cc, int ctrl)
{
    return
        (
            (
                ((mod->src1 == ctrl) && ((mod->flags1 & FLUID_MOD_CC) != 0) && (cc != 0))
                || ((mod->src1 == ctrl) && ((mod->flags1 & FLUID_MOD_CC) == 0) && (cc == 0))
            )
            ||
            (
                ((mod->src2 == ctrl) && ((mod->flags2 & FLUID_MOD_CC) != 0) && (cc != 0))
                || ((mod->src2 == ctrl) && ((mod->flags2 & FLUID_MOD_CC) == 0) && (cc == 0))
            )
        );
}

/**
 * Check if the modulator has the given destination.
 *
 * @param mod The modulator instance
 * @param gen The destination generator of type #fluid_gen_type to check for
 * @return TRUE if the modulator has the given destination, FALSE otherwise.
 */
int fluid_mod_has_dest(const fluid_mod_t *mod, int gen)
{
    return mod->dest == gen;
}


/* debug function: Prints the contents of a modulator */
#ifdef DEBUG
void fluid_dump_modulator(fluid_mod_t *mod)
{
    int src1 = mod->src1;
    int dest = mod->dest;
    int src2 = mod->src2;
    int flags1 = mod->flags1;
    int flags2 = mod->flags2;
    fluid_real_t amount = (fluid_real_t)mod->amount;

    printf("Src: ");

    if(flags1 & FLUID_MOD_CC)
    {
        printf("MIDI CC=%i", src1);
    }
    else
    {
        switch(src1)
        {
        case FLUID_MOD_NONE:
            printf("None");
            break;

        case FLUID_MOD_VELOCITY:
            printf("note-on velocity");
            break;

        case FLUID_MOD_KEY:
            printf("Key nr");
            break;

        case FLUID_MOD_KEYPRESSURE:
            printf("Poly pressure");
            break;

        case FLUID_MOD_CHANNELPRESSURE:
            printf("Chan pressure");
            break;

        case FLUID_MOD_PITCHWHEEL:
            printf("Pitch Wheel");
            break;

        case FLUID_MOD_PITCHWHEELSENS:
            printf("Pitch Wheel sens");
            break;

        default:
            printf("(unknown: %i)", src1);
        }; /* switch src1 */
    }; /* if not CC */

    if(flags1 & FLUID_MOD_NEGATIVE)
    {
        printf("- ");
    }
    else
    {
        printf("+ ");
    };

    if(flags1 & FLUID_MOD_BIPOLAR)
    {
        printf("bip ");
    }
    else
    {
        printf("unip ");
    };

    printf("-> ");

    switch(dest)
    {
    case GEN_FILTERQ:
        printf("Q");
        break;

    case GEN_FILTERFC:
        printf("fc");
        break;

    case GEN_CUSTOM_FILTERQ:
        printf("custom-Q");
        break;

    case GEN_CUSTOM_FILTERFC:
        printf("custom-fc");
        break;

    case GEN_VIBLFOTOPITCH:
        printf("VibLFO-to-pitch");
        break;

    case GEN_MODENVTOPITCH:
        printf("ModEnv-to-pitch");
        break;

    case GEN_MODLFOTOPITCH:
        printf("ModLFO-to-pitch");
        break;

    case GEN_CHORUSSEND:
        printf("Chorus send");
        break;

    case GEN_REVERBSEND:
        printf("Reverb send");
        break;

    case GEN_PAN:
        printf("pan");
        break;

    case GEN_CUSTOM_BALANCE:
        printf("balance");
        break;

    case GEN_ATTENUATION:
        printf("att");
        break;

    default:
        printf("dest %i", dest);
    }; /* switch dest */

    printf(", amount %f flags %i src2 %i flags2 %i\n", amount, flags1, src2, flags2);
};
#endif

