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

IPlayer::Status AudioPlayer::status() const
{
    return m_status;
}

void AudioPlayer::setStatus(const Status& status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;
    m_statusChanged.send(m_status);
}

mu::async::Channel<IPlayer::Status> AudioPlayer::statusChanged() const
{
    return m_statusChanged;
}

bool AudioPlayer::isRunning() const
{
    return m_status == Status::Running;
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

IAudioSourcePtr AudioPlayer::audioSource()
{
    return shared_from_this();
}

void AudioPlayer::run()
{
    if (m_stream && status() != Status::Error) {
        setStatus(Status::Running);
    }
}

void AudioPlayer::seek(unsigned long milliseconds)
{
    if (m_stream) {
        m_position = milliseconds * m_stream->sampleRate() / 1000;
    }
}

void AudioPlayer::stop()
{
    if (status() != Status::Error) {
        setStatus(Status::Stoped);
    }
}

void AudioPlayer::pause()
{
    if (status() != Status::Error) {
        setStatus(Status::Paused);
    }
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

    auto displacement = stream->copySamplesToBuffer(m_buffer.data(), m_position, sampleCount, m_sampleRate);
    m_position += displacement;

    if (!displacement) {
        setStatus(Stoped);
    }
}
