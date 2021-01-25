//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_IAUDIOINSERT_H
#define MU_AUDIO_IAUDIOINSERT_H

namespace mu::audio {
class IAudioInsert
{
public:
    virtual ~IAudioInsert() = default;

    virtual void setSampleRate(unsigned int sampleRate) = 0;

    virtual bool active() const = 0;
    virtual void setActive(bool active) = 0;

    //! return streams count for this insert: 1 for mono, 2 for stereo
    virtual unsigned int streamCount() const = 0;

    virtual void process(float* input, float* output, unsigned int sampleCount) = 0;
};
}

#endif // MU_AUDIO_IAUDIOINSERT_H
