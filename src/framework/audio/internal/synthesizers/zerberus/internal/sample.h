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

#ifndef MU_ZERBERUS_SAMPLE_H
#define MU_ZERBERUS_SAMPLE_H

#include <QString>

namespace mu::zerberus {
//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

class Sample
{
    int _channel      { 0 };
    short* _data      { nullptr };
    long long _frames { 0 };
    int _sampleRate   { 44100 };
    long long _loopStart { 0 };
    long long _loopEnd   { 0 };
    int _loopMode     { 0 };

public:
    Sample(int ch, short* val, int f, int sr)
        : _channel(ch), _data(val), _frames(f), _sampleRate(sr) {}
    ~Sample();
    bool read(const QString&);
    long long frames() const { return _frames; }
    short* data() const { return _data + _channel; }
    int channel() const { return _channel; }
    int sampleRate() const { return _sampleRate; }

    void setLoopStart(int v) { _loopStart = v; }
    void setLoopEnd(int v) { _loopEnd = v; }
    void setLoopMode(int v) { _loopMode = v; }
    long long loopStart() { return _loopStart; }
    long long loopEnd() { return _loopEnd; }
    int loopMode() { return _loopMode; }
};
}

#endif //MU_ZERBERUS_SAMPLE_H
