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

#ifndef MU_AUDIO_MIDISTREAMCONTROLLER_H
#define MU_AUDIO_MIDISTREAMCONTROLLER_H

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <vector>

#include "streamcontrollerbase.h"
#include "workertypes.h"
#include "internal/midisource.h"

namespace mu {
namespace audio {
namespace worker {
class MidiStreamController : public StreamControllerBase
{
public:
    MidiStreamController();

    void loadMIDI(const StreamID& id, const std::shared_ptr<midi::MidiStream>& data);

    void setPlaybackSpeed(const StreamID& id, float speed);

    void setIsTrackMuted(const StreamID& id, uint16_t ti, bool mute);
    void setTrackVolume(const StreamID& id, uint16_t ti, float volume);
    void setTrackBalance(const StreamID& id, uint16_t ti, float balance);

private:

    std::shared_ptr<IAudioSource> makeSource(const StreamID& id, const std::string& name) const override;
    void fillAudioContext(const std::shared_ptr<Stream>& s, Context* ctx) override;

    MidiSource* midiStream(const StreamID& id) const;
};
}
}
}

#endif // MU_AUDIO_MIDISTREAMCONTROLLER_H
