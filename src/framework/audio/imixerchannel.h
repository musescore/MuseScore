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
#ifndef MU_AUDIO_IMIXERCHANNEL_H
#define MU_AUDIO_IMIXERCHANNEL_H

#include <complex>
#include <memory>
#include "async/asyncable.h"
#include "iaudiosource.h"
#include "iaudioinsert.h"

namespace mu {
namespace audio {
class IMixerChannel : virtual public IAudioSource
{
public:
    virtual ~IMixerChannel() = default;

    virtual void setSource(std::shared_ptr<IAudioSource> source) = 0;

    virtual bool active() const = 0;
    virtual void setActive(bool active) = 0;

    virtual float level(unsigned int streamId) const = 0;
    virtual void setLevel(float level) = 0;
    virtual void setLevel(unsigned int streamId, float level) = 0;

    virtual std::complex<float> balance(unsigned int streamId) const = 0;
    virtual void setBalance(std::complex<float> value) = 0;
    virtual void setBalance(unsigned int streamId, std::complex<float> value) = 0;

    virtual std::shared_ptr<IAudioInsert> insert(unsigned int number) const = 0;
    virtual void setInsert(unsigned int number, std::shared_ptr<IAudioInsert> insert) = 0;
};
} // namespace audio
} // namespace mu

#endif // MU_AUDIO_IMIXERCHANNEL_H
