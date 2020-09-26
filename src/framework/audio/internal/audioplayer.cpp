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
    setStatus(Stoped);
    m_position = 0;
    m_stream = stream;
    m_streamsCountChanged.send(streamCount());

    return Ret(Ret::Code::Ok);
}

void AudioPlayer::play()
{
    if (m_stream && status() != Status::Error) {
        setStatus(Status::Running);
    }
}

void AudioPlayer::seek(unsigned long miliseconds)
{
    if (m_stream) {
        m_position = miliseconds * m_stream->sampleRate() / 1000;
    }
}

void AudioPlayer::stop()
{
    if (status() != Status::Error) {
        setStatus(Status::Stoped);
    }
}

unsigned long AudioPlayer::miliseconds() const
{
    if (!m_stream) {
        return 0;
    }
    return m_position * 1000 / m_stream->sampleRate();
}

void AudioPlayer::forwardTime(unsigned long miliseconds)
{
    UNUSED(miliseconds)

    //position changed by calling forward method
    //here can be placed methods for preparing automatization
}

unsigned int AudioPlayer::streamCount() const
{
    if (m_stream) {
        return m_stream->channelsCount();
    }
    return 0;
}

void AudioPlayer::forward(unsigned int sampleCount)
{
    //copy shared_ptr in case it can be changed during forward
    auto stream = m_stream;
    if (!stream) {
        return;
    }
    std::fill(m_buffer.begin() + 0, m_buffer.end(), 0.f);

    if (status() != Running) {
        return;
    }

    auto displacement = stream->copySamples(m_buffer.data(), m_position, sampleCount, m_sampleRate);
    m_position += displacement;

    if (!displacement) {
        setStatus(Stoped);
    }
}
