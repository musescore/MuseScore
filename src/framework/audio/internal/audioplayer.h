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
#ifndef MU_AUDIO_AUDIOPLAYER_H
#define MU_AUDIO_AUDIOPLAYER_H

#include "iaudioplayer.h"
#include "abstractplayer.h"
#include "abstractaudiosource.h"

namespace mu::audio {
class AudioPlayer : public AbstractPlayer, public AbstractAudioSource, public IAudioPlayer
{
public:
    AudioPlayer();

    void unload() override;
    Ret load(const std::shared_ptr<audio::IAudioStream>& stream) override;

    void play() override;
    void seek(unsigned long miliseconds) override;
    void stop() override;

    unsigned long miliseconds() const override;
    void forwardTime(unsigned long) override;

    unsigned int streamCount() const override;

    void forward(unsigned int sampleCount) override;

private:
    std::shared_ptr<audio::IAudioStream> m_stream = nullptr;
    unsigned long m_position = 0;
};
}

#endif // MU_AUDIO_AUDIOPLAYER_H
