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

#ifndef MU_MIDI_ISEQUENCER_H
#define MU_MIDI_ISEQUENCER_H

#include <string>
#include <functional>
#include <memory>

#include "modularity/imoduleexport.h"
#include "miditypes.h"
#include "async/channel.h"

namespace mu {
namespace midi {
class ISequencer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISequencer)
public:
    virtual ~ISequencer() = default;

    virtual void loadMIDI(const std::shared_ptr<MidiStream>& stream) = 0;

    virtual bool run(float initSec) = 0;
    virtual void seek(float sec) = 0;
    virtual void stop() = 0;

    struct Context {
        tick_t fromTick = 0;
        tick_t toTick = 0;
        tick_t playTick = 0;
    };

    virtual float getAudio(float sec, float* buf, unsigned int samples, Context* ctx = nullptr) = 0;
    virtual bool hasEnded() const = 0;

    virtual float playbackSpeed() const = 0;
    virtual void setPlaybackSpeed(float speed) = 0;

    virtual void setIsTrackMuted(track_t trackIndex, bool mute) = 0;
    virtual void setTrackVolume(track_t trackIndex, float volume) = 0;
    virtual void setTrackBalance(track_t trackIndex, float balance) = 0;
};
}
}

#endif // MU_MIDI_ISEQUENCER_H
