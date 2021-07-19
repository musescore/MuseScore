/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "audioplayer.h"
#include "audioerrors.h"
#include "log.h"

using namespace mu::audio;

AudioPlayer::AudioPlayer()
{
}

void AudioPlayer::unload()
{
    load(nullptr);
}

mu::Ret AudioPlayer::load(const std::shared_ptr<IAudioStream>& stream)
{
    m_position = 0;
    m_stream = stream;
    m_streamsCountChanged.send(audioChannelsCount());

    return Ret(Ret::Code::Ok);
}

unsigned long AudioPlayer::milliseconds() const
{
    if (!m_stream) {
        return 0;
    }
    return m_position * 1000 / m_stream->sampleRate();
}

void AudioPlayer::forwardTime(unsigned long milliseconds)
{
    UNUSED(milliseconds)

    //position changed by calling forward method
    //here can be placed methods for preparing automatization
}

unsigned int AudioPlayer::audioChannelsCount() const
{
    if (m_stream) {
        return m_stream->channelsCount();
    }
    return 0;
}

void AudioPlayer::process(float* buffer, unsigned int sampleCount)
{
    //copy shared_ptr in case it can be changed during forward
    auto stream = m_stream;
    if (!stream) {
        return;
    }

    auto displacement = stream->copySamplesToBuffer(buffer, m_position, sampleCount, m_sampleRate);
    m_position += displacement;

    if (!displacement) {
    }
}
