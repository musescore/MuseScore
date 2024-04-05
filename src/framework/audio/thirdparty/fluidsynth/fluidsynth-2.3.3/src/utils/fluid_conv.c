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

#include "fluid_conv.h"
#include "fluid_sys.h"
#include "fluid_conv_tables.inc.h"

/*
 * Converts absolute cents to Hertz
 * 
 * As per sfspec section 9.3:
 * 
 * ABSOLUTE CENTS - An absolute logarithmic measure of frequency based on a
 * reference of MIDI key number scaled by 100.
 * A cent is 1/1200 of an octave [which is the twelve hundredth root of two],
 * and value 6900 is 440 Hz (A-440).
 * 
 * Implemented below basically is the following:
 *   440 * 2^((cents-6900)/1200)
 * = 440 * 2^((int)((cents-6900)/1200)) * 2^(((int)cents-6900)%1200))
 * = 2^((int)((cents-6900)/1200)) * (440 * 2^(((int)cents-6900)%1200)))
 *                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *                           This second factor is stored in the lookup table.
 *
 * The first factor can be implemented with a fast shift when the exponent
 * is always an int. This is the case when using 440/2^6 Hz rather than 440Hz
 * reference.
 */
fluid_real_t
fluid_ct2hz_real(fluid_real_t cents)
{
    if(FLUID_UNLIKELY(cents < 0))
    {
        return fluid_act2hz(cents);
    }
    else
    {
        unsigned int mult, fac, rem;
        unsigned int icents = (unsigned int)cents;
        icents += 300u;

        // don't use stdlib div() here, it turned out have poor performance
        fac = icents / 1200u;
        rem = icents % 1200u;

        // Think of "mult" as the factor that we multiply (440/2^6)Hz with,
        // or in other words mult is the "first factor" of the above
        // functions comment.
        //
        // Assuming sizeof(uint)==4 this will give us a maximum range of
        // 32 * 1200cents - 300cents == 38100 cents == 29,527,900,160 Hz
        // which is much more than ever needed. For bigger values, just
        // safely wrap around (the & is just a replacement for the quick
        // modulo operation % 32).
        mult = 1u << (fac & (sizeof(mult)*8u - 1u));

        // don't use ldexp() either (poor performance)
        return mult * fluid_ct2hz_tab[rem];
    }
}

/*
 * fluid_ct2hz
 */
fluid_real_t
fluid_ct2hz(fluid_real_t cents)
{
    /* Filter fc limit: SF2.01 page 48 # 8 */
    if(cents >= 13500)
    {
        cents = 13500;             /* 20 kHz */
    }
    else if(cents < 1500)
    {
        cents = 1500;              /* 20 Hz */
    }

    return fluid_ct2hz_real(cents);
}

/*
 * fluid_cb2amp
 *
 * in: a value between 0 and 1440, 0 is no attenuation
 * out: a value between 1 and 0
 */
fluid_real_t
fluid_cb2amp(fluid_real_t cb)
{
    /*
     * cb: an attenuation in 'centibels' (1/10 dB)
     * SF2.01 page 49 # 48 limits it to 144 dB.
     * 96 dB is reasonable for 16 bit systems, 144 would make sense for 24 bit.
     */

    /* minimum attenuation: 0 dB */
    if(cb < 0)
    {
        return 1.0;
    }

    if(cb >= FLUID_CB_AMP_SIZE)
    {
        return 0.0;
    }

    return fluid_cb2amp_tab[(int) cb];
}

/*
 * fluid_tc2sec_delay
 */
fluid_real_t
fluid_tc2sec_delay(fluid_real_t tc)
{
    /* SF2.01 section 8.1.2 items 21, 23, 25, 33
     * SF2.01 section 8.1.3 items 21, 23, 25, 33
     *
     * The most negative number indicates a delay of 0. Range is limited
     * from -12000 to 5000 */
    if(tc <= -32768.0f)
    {
        return (fluid_real_t) 0.0f;
    };

    if(tc < -12000.f)
    {
        tc = (fluid_real_t) -12000.0f;
    }

    if(tc > 5000.0f)
    {
        tc = (fluid_real_t) 5000.0f;
    }

    return FLUID_POW(2.f, tc / 1200.f);
}

/*
 * fluid_tc2sec_attack
 */
fluid_real_t
fluid_tc2sec_attack(fluid_real_t tc)
{
    /* SF2.01 section 8.1.2 items 26, 34
     * SF2.01 section 8.1.3 items 26, 34
     * The most negative number indicates a delay of 0
     * Range is limited from -12000 to 8000 */
    if(tc <= -32768.f)
    {
        return (fluid_real_t) 0.f;
    };

    if(tc < -12000.f)
    {
        tc = (fluid_real_t) -12000.f;
    };

    if(tc > 8000.f)
    {
        tc = (fluid_real_t) 8000.f;
    };

    return FLUID_POW(2.f, tc / 1200.f);
}

/*
 * fluid_tc2sec
 */
fluid_real_t
fluid_tc2sec(fluid_real_t tc)
{
    /* No range checking here! */
    return FLUID_POW(2.f, tc / 1200.f);
}

/*
 * fluid_tc2sec_release
 */
fluid_real_t
fluid_tc2sec_release(fluid_real_t tc)
{
    /* SF2.01 section 8.1.2 items 30, 38
     * SF2.01 section 8.1.3 items 30, 38
     * No 'most negative number' rule here!
     * Range is limited from -12000 to 8000 */
    if(tc <= -32768.f)
    {
        return (fluid_real_t) 0.f;
    };

    if(tc < -12000.f)
    {
        tc = (fluid_real_t) -12000.f;
    };

    if(tc > 8000.f)
    {
        tc = (fluid_real_t) 8000.f;
    };

    return FLUID_POW(2.f, tc / 1200.f);
}

/*
 * fluid_act2hz
 *
 * Convert from absolute cents to Hertz
 * 
 * The inverse operation, converting from Hertz to cents, was unused and implemented as
 *
fluid_hz2ct(fluid_real_t f)
{
    return 6900.f + (1200.f / FLUID_M_LN2) * FLUID_LOGF(f / 440.0f));
}
 */
double
fluid_act2hz(double c)
{
    // do not use FLUID_POW, otherwise the unit tests will fail when compiled in single precision
    return 8.1757989156437073336828122976032719176391831357 * pow(2.f, c / 1200.f);
}

/*
 * fluid_pan
 */
fluid_real_t
fluid_pan(fluid_real_t c, int left)
{
    if(left)
    {
        c = -c;
    }

    if(c <= -500.f)
    {
        return (fluid_real_t) 0.f;
    }
    else if(c >= 500.f)
    {
        return (fluid_real_t) 1.f;
    }
    else
    {
        return fluid_pan_tab[(int)(c) + 500];
    }
}

/*
 * Return the amount of attenuation based on the balance for the specified
 * channel. If balance is negative (turned toward left channel, only the right
 * channel is attenuated. If balance is positive, only the left channel is
 * attenuated.
 *
 * @params balance left/right balance, range [-960;960] in absolute centibels
 * @return amount of attenuation [0.0;1.0]
 */
fluid_real_t fluid_balance(fluid_real_t balance, int left)
{
    /* This is the most common case */
    if(balance == 0.f)
    {
        return 1.0f;
    }

    if((left && balance < 0.f) || (!left && balance > 0.f))
    {
        return 1.0f;
    }

    if(balance < 0.f)
    {
        balance = -balance;
    }

    return fluid_cb2amp(balance);
}

/*
 * fluid_concave
 */
fluid_real_t
fluid_concave(fluid_real_t val)
{
    int ival = (int)val;
    if(val < 0.f)
    {
        return 0.f;
    }
    else if (ival >= FLUID_VEL_CB_SIZE - 1)
    {
        return fluid_concave_tab[FLUID_VEL_CB_SIZE - 1];
    }

    return fluid_concave_tab[ival] + (fluid_concave_tab[ival + 1] - fluid_concave_tab[ival]) * (val - ival);
}

/*
 * fluid_convex
 */
fluid_real_t
fluid_convex(fluid_real_t val)
{
    int ival = (int)val;
    if(val < 0.f)
    {
        return 0.f;
    }
    else if (ival >= FLUID_VEL_CB_SIZE - 1)
    {
        return fluid_convex_tab[FLUID_VEL_CB_SIZE - 1];
    }

    // interpolation between convex steps: fixes bad sounds with modenv and filter cutoff
    return fluid_convex_tab[ival] + (fluid_convex_tab[ival + 1] - fluid_convex_tab[ival]) * (val - ival);
}

