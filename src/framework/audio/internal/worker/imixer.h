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
#include "iaudioprocessor.h"
#include "imixerchannel.h"

namespace mu::audio {
class IMixer
{
public:

    //! number of mix channel
    using ChannelID = unsigned int;

    enum Mode {
        MONO,
        STEREO,
//TODO: SURROUND, //NOTE: 5.1, 7.1, etc
//TODO: SPATIAL //NOTE: 2 channel output with HRTF
    };

    virtual ~IMixer() = default;

    virtual Mode mode() const = 0;
    virtual void setMode(const Mode& mode) = 0;

    //! set master level
    virtual void setLevel(float level) = 0;

    //! return processor at master
    virtual IAudioProcessorPtr processor(unsigned int number) const = 0;

    //! set master processors
    virtual void setProcessor(unsigned int number, IAudioProcessorPtr proc) = 0;

    //! add source to the mix
    virtual ChannelID addChannel(std::shared_ptr<IAudioSource> source) = 0;
    virtual void removeChannel(ChannelID channelId) = 0;
    virtual std::shared_ptr<IMixerChannel> channel(ChannelID channelId) const = 0;

    //! mixed source
    virtual IAudioSourcePtr mixedSource() = 0;

    virtual void setActive(ChannelID channelId, bool active) = 0;
    virtual void setLevel(ChannelID channelId, unsigned int streamId, float level) = 0;
    virtual void setBalance(ChannelID channelId, unsigned int streamId, std::complex<float> balance) = 0;
};

using IMixerPtr = std::shared_ptr<IMixer>;
}

#endif // MU_AUDIO_IMIXER_H
