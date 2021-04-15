/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_AUDIO_AUDIOPLAYER_H
#define MU_AUDIO_AUDIOPLAYER_H

#include "iaudioplayer.h"
#include "abstractaudiosource.h"

namespace mu::audio {
class AudioPlayer : public IAudioPlayer, public AbstractAudioSource, public std::enable_shared_from_this<AudioPlayer>
{
public:
    AudioPlayer();

    // IPlayer
    Status status() const override;
    async::Channel<Status> statusChanged() const override;

    bool isRunning() const override;

    void run() override;
    void seek(unsigned long milliseconds) override;
    void stop() override;
    void pause() override;

    unsigned long milliseconds() const override;
    void forwardTime(unsigned long) override;

    // IAudioPlayer
    void unload() override;
    Ret load(const std::shared_ptr<audio::IAudioStream>& stream) override;
    IAudioSourcePtr audioSource() override;

    // IAudioSource (AbstractAudioSource)
    unsigned int streamCount() const override;
    void forward(unsigned int sampleCount) override;

private:
    void setStatus(const Status& status);

    Status m_status = Status::Stoped;
    async::Channel<Status> m_statusChanged;
    std::shared_ptr<audio::IAudioStream> m_stream = nullptr;
    unsigned long m_position = 0;
};
}

#endif // MU_AUDIO_AUDIOPLAYER_H
