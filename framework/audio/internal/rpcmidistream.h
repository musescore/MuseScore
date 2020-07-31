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

#ifndef MU_AUDIO_RPCMIDISTREAM_H
#define MU_AUDIO_RPCMIDISTREAM_H

#include "midi/miditypes.h"
#include "internal/worker/rpcstreambase.h"

namespace mu {
namespace audio {
class RpcMidiStream : public worker::RpcStreamBase
{
public:
    RpcMidiStream(const std::string& name = std::string());

    void loadMIDI(const std::shared_ptr<midi::MidiStream>& midi);
    void init(float samplerate);

    float playbackSpeed() const;
    void setPlaybackSpeed(float speed);

    void setIsTrackMuted(uint16_t ti, bool mute);
    void setTrackVolume(uint16_t ti, float volume);
    void setTrackBalance(uint16_t ti, float balance);

private:

    void onGetAudio(const Context& ctx) override;

    float m_playbackSpeed = 1;
    std::shared_ptr<midi::MidiStream> m_midiStream;
};
}
}

#endif // MU_AUDIO_RPCMIDISTREAM_H
