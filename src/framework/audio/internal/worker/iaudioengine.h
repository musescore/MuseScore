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
#ifndef MU_AUDIO_IAUDIOENGINE_H
#define MU_AUDIO_IAUDIOENGINE_H

#include <string>

#include "modularity/imoduleexport.h"

#include "ret.h"
#include "async/channel.h"
#include "isequencer.h"
#include "iaudiodriver.h"
#include "internal/iaudiobuffer.h"
#include "imixer.h"
#include "isynthesizer.h"

namespace mu::audio {
class IAudioEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioEngine)

public:
    virtual ~IAudioEngine() = default;

    virtual bool isInited() const = 0;
    virtual async::Channel<bool> initChanged() const = 0;
    virtual unsigned int sampleRate() const = 0;
    virtual ISequencerPtr sequencer() const = 0;
    virtual IMixerPtr mixer() const = 0;
    virtual IAudioBufferPtr buffer() const = 0;
    virtual void setAudioBuffer(IAudioBufferPtr) = 0;
};
}
#endif // MU_AUDIO_IAUDIOENGINE_H
