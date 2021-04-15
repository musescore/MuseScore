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

#ifndef __AUDIOFILE_H__
#define __AUDIOFILE_H__

#include <QByteArray>

#include <sndfile.h>

//---------------------------------------------------------
//   AudioFile
//---------------------------------------------------------

class AudioFile
{
    enum FormatType
    {
        s16p,
        fltp
    };
    SF_INFO info;
    SNDFILE* sf { nullptr };
    SF_INSTRUMENT inst;
    bool hasInstrument { false };
    QByteArray buf;    // used during read of Sample
    int idx { 0 };
    FormatType _type { fltp };

public:
    AudioFile();
    ~AudioFile();

    bool open(const QByteArray&);
    const char* error() const;
    sf_count_t readData(short* data, sf_count_t frames);

    int channels() const { return info.channels; }
    sf_count_t frames() const { return info.frames; }
    int samplerate() const { return info.samplerate; }

    sf_count_t getFileLen() const { return buf.size(); }
    sf_count_t tell() const { return idx; }
    sf_count_t read(void* ptr, sf_count_t count);
    sf_count_t write(const void* ptr, sf_count_t count);
    sf_count_t seek(sf_count_t offset, int whence);
    unsigned int loopStart(int v = 0) { return hasInstrument ? inst.loops[v].start : -1; }
    unsigned int loopEnd(int v = 0) { return hasInstrument ? inst.loops[v].end : -1; }
    int loopMode(int v = 0) { return hasInstrument ? inst.loops[v].mode : -1; }
};

#endif
