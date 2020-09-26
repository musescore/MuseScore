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
#ifndef MU_AUDIO_IMIXER_H
#define MU_AUDIO_IMIXER_H

#include <memory>
#include <complex>
#include "iaudiosource.h"
#include "iaudioinsert.h"
#include "imixerchannel.h"

namespace mu {
namespace audio {
class IMixer : virtual public IAudioSource
{
public:

    enum Mode {
        MONO,
        STEREO,
//TODO: SURROUND, //NOTE: 5.1, 7.1, etc
//TODO: SPATIAL //NOTE: 2 channel output with HRTF
    };

    virtual ~IMixer() = default;

    virtual Mode mode() const = 0;
    virtual void setMode(const Mode& mode) = 0;

    virtual unsigned int streamCount() const override = 0;

    //!set master level
    virtual void setLevel(float level) = 0;

    //! return insert at master
    virtual std::shared_ptr<IAudioInsert> insert(unsigned int number) const = 0;

    //! set master insert
    virtual void setInsert(unsigned int number, std::shared_ptr<IAudioInsert> insert) = 0;

    //! add source to the mix
    virtual unsigned int addChannel(std::shared_ptr<IAudioSource> source) = 0;
    virtual void removeChannel(unsigned int channelId) = 0;

    virtual void setActive(unsigned int channelId, bool active) = 0;
    virtual void setLevel(unsigned int channelId, unsigned int streamId, float level) = 0;
    virtual void setBalance(unsigned int channelId, unsigned int streamId, std::complex<float> balance) = 0;

    virtual std::shared_ptr<IMixerChannel> channel(unsigned int number) const = 0;
};
using IMixerPtr = std::shared_ptr<IMixer>;
}
}

#endif // MU_AUDIO_IMIXER_H
