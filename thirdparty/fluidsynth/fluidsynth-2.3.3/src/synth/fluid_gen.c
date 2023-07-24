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


#include "fluid_gen.h"
#include "fluid_chan.h"


#define _GEN(_name) GEN_ ## _name, #_name


/* See SFSpec21 $8.1.3 */
static const fluid_gen_info_t fluid_gen_info[] =
{
    /* number/name             init  nrpn-scale    min        max         def */
    { _GEN(STARTADDROFS),           1,     1,       0.0f,     1e10f,       0.0f },
    { _GEN(ENDADDROFS),             1,     1,     -1e10f,      0.0f,       0.0f },
    { _GEN(STARTLOOPADDROFS),       1,     1,     -1e10f,     1e10f,       0.0f },
    { _GEN(ENDLOOPADDROFS),         1,     1,     -1e10f,     1e10f,       0.0f },
    { _GEN(STARTADDRCOARSEOFS),     0,     1,       0.0f,     1e10f,       0.0f },
    { _GEN(MODLFOTOPITCH),          1,     2,  -12000.0f,  12000.0f,       0.0f },
    { _GEN(VIBLFOTOPITCH),          1,     2,  -12000.0f,  12000.0f,       0.0f },
    { _GEN(MODENVTOPITCH),          1,     2,  -12000.0f,  12000.0f,       0.0f },
    { _GEN(FILTERFC),               1,     2,    1500.0f,  13500.0f,   13500.0f },
    { _GEN(FILTERQ),                1,     1,       0.0f,    960.0f,       0.0f },
    { _GEN(MODLFOTOFILTERFC),       1,     2,  -12000.0f,  12000.0f,       0.0f },
    { _GEN(MODENVTOFILTERFC),       1,     2,  -12000.0f,  12000.0f,       0.0f },
    { _GEN(ENDADDRCOARSEOFS),       0,     1,     -1e10f,      0.0f,       0.0f },
    { _GEN(MODLFOTOVOL),            1,     1,    -960.0f,    960.0f,       0.0f },
    { _GEN(UNUSED1),                0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(CHORUSSEND),             1,     1,       0.0f,   1000.0f,       0.0f },
    { _GEN(REVERBSEND),             1,     1,       0.0f,   1000.0f,       0.0f },
    { _GEN(PAN),                    1,     1,    -500.0f,    500.0f,       0.0f },
    { _GEN(UNUSED2),                0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(UNUSED3),                0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(UNUSED4),                0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(MODLFODELAY),            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
    { _GEN(MODLFOFREQ),             1,     4,  -16000.0f,   4500.0f,       0.0f },
    { _GEN(VIBLFODELAY),            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
    { _GEN(VIBLFOFREQ),             1,     4,  -16000.0f,   4500.0f,       0.0f },
    { _GEN(MODENVDELAY),            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
    { _GEN(MODENVATTACK),           1,     2,  -12000.0f,   8000.0f,  -12000.0f },
    { _GEN(MODENVHOLD),             1,     2,  -12000.0f,   5000.0f,  -12000.0f },
    { _GEN(MODENVDECAY),            1,     2,  -12000.0f,   8000.0f,  -12000.0f },
    { _GEN(MODENVSUSTAIN),          0,     1,       0.0f,   1000.0f,       0.0f },
    { _GEN(MODENVRELEASE),          1,     2,  -12000.0f,   8000.0f,  -12000.0f },
    { _GEN(KEYTOMODENVHOLD),        0,     1,   -1200.0f,   1200.0f,       0.0f },
    { _GEN(KEYTOMODENVDECAY),       0,     1,   -1200.0f,   1200.0f,       0.0f },
    { _GEN(VOLENVDELAY),            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
    { _GEN(VOLENVATTACK),           1,     2,  -12000.0f,   8000.0f,  -12000.0f },
    { _GEN(VOLENVHOLD),             1,     2,  -12000.0f,   5000.0f,  -12000.0f },
    { _GEN(VOLENVDECAY),            1,     2,  -12000.0f,   8000.0f,  -12000.0f },
    { _GEN(VOLENVSUSTAIN),          0,     1,       0.0f,   1440.0f,       0.0f },
    { _GEN(VOLENVRELEASE),          1,     2,  -12000.0f,   8000.0f,  -12000.0f },
    { _GEN(KEYTOVOLENVHOLD),        0,     1,   -1200.0f,   1200.0f,       0.0f },
    { _GEN(KEYTOVOLENVDECAY),       0,     1,   -1200.0f,   1200.0f,       0.0f },
    { _GEN(INSTRUMENT),             0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(RESERVED1),              0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(KEYRANGE),               0,     0,       0.0f,    127.0f,       0.0f },
    { _GEN(VELRANGE),               0,     0,       0.0f,    127.0f,       0.0f },
    { _GEN(STARTLOOPADDRCOARSEOFS), 0,     1,     -1e10f,     1e10f,       0.0f },
    { _GEN(KEYNUM),                 1,     0,       0.0f,    127.0f,      -1.0f },
    { _GEN(VELOCITY),               1,     1,       0.0f,    127.0f,      -1.0f },
    { _GEN(ATTENUATION),            1,     1,       0.0f,   1440.0f,       0.0f },
    { _GEN(RESERVED2),              0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(ENDLOOPADDRCOARSEOFS),   0,     1,     -1e10f,     1e10f,       0.0f },
    { _GEN(COARSETUNE),             0,     1,    -120.0f,    120.0f,       0.0f },
    { _GEN(FINETUNE),               0,     1,     -99.0f,     99.0f,       0.0f },
    { _GEN(SAMPLEID),               0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(SAMPLEMODE),             0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(RESERVED3),              0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(SCALETUNE),              0,     1,       0.0f,   1200.0f,     100.0f },
    { _GEN(EXCLUSIVECLASS),         0,     0,       0.0f,      0.0f,       0.0f },
    { _GEN(OVERRIDEROOTKEY),        1,     0,       0.0f,    127.0f,      -1.0f },
    { _GEN(PITCH),                  1,     0,       0.0f,    127.0f,       0.0f },
    { _GEN(CUSTOM_BALANCE),         1,     0,    -960.0f,    960.0f,       0.0f },
    { _GEN(CUSTOM_FILTERFC),        1,     2,       0.0f,  22050.0f,       0.0f },
    { _GEN(CUSTOM_FILTERQ),         1,     1,       0.0f,    960.0f,       0.0f }
};

/* fluid_gen_init
 *
 * Set an array of generators to their initial value
 */
void
fluid_gen_init(fluid_gen_t *gen, fluid_channel_t *channel)
{
    int i;

    for(i = 0; i < GEN_LAST; i++)
    {
        gen[i].flags = GEN_UNUSED;
        gen[i].mod = 0.0;
        gen[i].nrpn = (channel == NULL) ? 0.0 : fluid_channel_get_gen(channel, i);
        gen[i].val = fluid_gen_info[i].def;
    }
}

fluid_real_t fluid_gen_scale(int gen, float value)
{
    return (fluid_gen_info[gen].min
            + value * (fluid_gen_info[gen].max - fluid_gen_info[gen].min));
}

fluid_real_t fluid_gen_scale_nrpn(int gen, int data)
{
    data = data - 8192;
    fluid_clip(data, -8192, 8192);
    return (fluid_real_t)(data * fluid_gen_info[gen].nrpn_scale);
}


const char *fluid_gen_name(int gen)
{
    return fluid_gen_info[gen].name;
}
