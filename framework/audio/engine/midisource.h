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

#ifndef MU_AUDIO_MIDISOURCE_H
#define MU_AUDIO_MIDISOURCE_H

#include <string>
#include <memory>

#include "iaudiosource.h"

#include "modularity/ioc.h"
#include "audio/midi/isequencer.h"
#include "audio/midi/miditypes.h"

namespace mu {
namespace audio {
namespace engine {
class MidiSource : public IAudioSource
{
    INJECT(audio_engine, midi::ISequencer, sequencer)

public:
    MidiSource();

    void setSampleRate(float samplerate) override;
    SoLoud::AudioSource* source() override;

    void loadMIDI(const std::shared_ptr<midi::MidiStream>& stream);
    void init(float samplerate);

    float playbackSpeed() const;
    void setPlaybackSpeed(float speed);

    void setIsTrackMuted(int ti, bool mute);
    void setTrackVolume(int ti, float volume);
    void setTrackBalance(int ti, float balance);

private:

    struct SL;
    struct SLInstance;
    std::shared_ptr<SL> m_sl;
    std::shared_ptr<midi::ISequencer> m_seq;
};
}
}
}

#endif // MU_AUDIO_MIDISOURCE_H
