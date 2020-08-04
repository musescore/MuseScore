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
#ifndef MU_AUDIO_IMIDISOURCE_H
#define MU_AUDIO_IMIDISOURCE_H

#include "modularity/imoduleexport.h"
#include "midi/miditypes.h"
#include "iaudiosource.h"

namespace mu {
namespace audio {
class IMidiSource : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiSource)

public:
    virtual ~IMidiSource() = default;

    virtual std::shared_ptr<IAudioSource> audioSource() = 0;

    virtual void loadMIDI(const std::shared_ptr<midi::MidiStream>& midi) = 0;

    virtual float playbackSpeed() const = 0;
    virtual void setPlaybackSpeed(float speed) = 0;

    virtual void setIsTrackMuted(uint16_t trackIndex, bool mute) = 0;
    virtual void setTrackVolume(uint16_t trackIndex, float volume) = 0;
    virtual void setTrackBalance(uint16_t trackIndex, float balance) = 0;
};
}
}

#endif // MU_AUDIO_IMIDISOURCE_H
