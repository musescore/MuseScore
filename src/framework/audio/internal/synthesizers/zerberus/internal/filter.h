/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_ZERBERUS_MFILTER_H
#define MU_ZERBERUS_MFILTER_H

namespace mu::zerberus {
struct Zone;
class Zerberus;

//---------------------------------------------------------
//   Envelope
//---------------------------------------------------------

//Biquad filter implementation
class ZFilter
{
public:
    ZFilter();

    void initialize(const Zerberus* zerberus, const Zone* z, int velocity);

    void update();
    float apply(float inputValue, bool leftChannel);
    float interpolate(unsigned phase, short prevVal, short currVal, short nextVal, short nextNextVal) const;   //pure function

private:
    const Zerberus* zerberus;
    const Zone* sampleZone;

    float resonanceF = 0.f;          // the resonance frequency in Hz
    float last_resonanceF = 0.f;     // current resonance frequency of the filter

    // Serves as a flag: A deviation between fres and last_fres
    // indicates, that the filter has to be recalculated.
    float q_lin = 0.f;               // the q-factor on a linear scale
    float gain = 0.f;         // Gain correction factor, depends on q

    // Flag: If set, the filter will be set directly.
    //       Else it changes smoothly
    bool firstRun = true;

    struct FilterData {
        float histX1 = 0.f;
        float histX2 = 0.f;
        float histY1 = 0.f;
        float histY2 = 0.f;
    };
    FilterData monoL;
    FilterData monoR;

    // normalized filter coefficients (bX = bX/a0 and aX = aX/a0)
    // see Robert Bristow-Johnson's 'Cookbook formulae for audio EQ biquad filter coefficients'
    float b0 = 0.f;                // b0 / a0
    float b1 = 0.f;                // b1 / a0
    float b2 = 0.f;                // b2 / a0
    float a1 = 0.f;                // a0 / a0
    float a2 = 0.f;                // a1 / a0

    float b0_incr = 0.f;
    float b1_incr = 0.f;
    float b2_incr = 0.f;
    float a1_incr = 0.f;
    float a2_incr = 0.f;
    int filter_coeff_incr_count = 0;
};
}

#endif //MU_ZERBERUS_MFILTER_H
