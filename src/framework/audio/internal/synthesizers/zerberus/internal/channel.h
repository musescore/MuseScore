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

#ifndef MU_ZERBERUS_MCHANNEL_H
#define MU_ZERBERUS_MCHANNEL_H

namespace mu::zerberus {
class Zerberus;
class ZInstrument;

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class Channel
{
    Zerberus* _msynth;
    ZInstrument* _instrument;
    float _gain;
    float _panLeftGain;
    float _panRightGain;
    float _midiVolume;
    char ctrl[128];

    int _idx;                 // channel index
#if 0 // yet (?) unused
    int _sustain;
#endif

public:
    Channel(Zerberus*, int idx);

    void pitchBend(int);
    void controller(int ctrl, int val);
    ZInstrument* instrument() const { return _instrument; }
    void setInstrument(ZInstrument* i) { _instrument = i; resetCC(); }
    Zerberus* msynth() const { return _msynth; }
    int sustain() const;
    float gain() const { return _gain * _midiVolume; }
    float panLeftGain() const { return _panLeftGain; }
    float panRightGain() const { return _panRightGain; }
    int idx() const { return _idx; }
    int getCtrl(int CTRL) const;
    void resetCC();
};
}

#endif //MU_ZERBERUS_MCHANNEL_H
