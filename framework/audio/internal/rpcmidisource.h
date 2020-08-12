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

#ifndef MU_AUDIO_RPCMIDISOURCE_H
#define MU_AUDIO_RPCMIDISOURCE_H

#include <memory>
#include "imidisource.h"
#include "midi/miditypes.h"
#include "internal/worker/rpcsourcebase.h"

#include "modularity/ioc.h"
#include "midi/imidiportdatasender.h"

namespace mu {
namespace audio {
class RpcMidiSource : public IMidiSource, public worker::RpcSourceBase, public std::enable_shared_from_this<RpcMidiSource>
{
    INJECT(audio, midi::IMidiPortDataSender, midiPortDataSender)

public:
    RpcMidiSource(const std::string& name = std::string());

    std::shared_ptr<IAudioSource> audioSource() override;

    void loadMIDI(const std::shared_ptr<midi::MidiStream>& midi) override;

    float playbackSpeed() const override;
    void setPlaybackSpeed(float speed) override;

    void setIsTrackMuted(uint16_t ti, bool mute) override;
    void setTrackVolume(uint16_t ti, float volume) override;
    void setTrackBalance(uint16_t ti, float balance) override;

private:

    void onGetAudio(const Context& ctx) override;

    float m_playbackSpeed = 1;
};
}
}

#endif // MU_AUDIO_RPCMIDISOURCE_H
