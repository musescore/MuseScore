/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#pragma once

#include <memory>

#include "global/async/asyncable.h"
#include "global/async/channel.h"
#include "global/async/notification.h"

#include "audio/common/audiotypes.h"

#include "../iaudiosource.h"

namespace muse::audio::worker {
enum TrackType {
    Undefined = -1,
    Event_track,
    Sound_track
};

class ITrackAudioInput : public IAudioSource
{
public:
    virtual ~ITrackAudioInput() = default;

    virtual void seek(const msecs_t newPositionMsecs) = 0;
    virtual void flush() = 0;

    virtual const AudioInputParams& inputParams() const = 0;
    virtual void applyInputParams(const AudioInputParams& requiredParams) = 0;
    virtual async::Channel<AudioInputParams> inputParamsChanged() const = 0;

    virtual void prepareToPlay() = 0;
    virtual bool readyToPlay() const = 0;
    virtual async::Notification readyToPlayChanged() const = 0;

    virtual InputProcessingProgress inputProcessingProgress() const = 0;
};

class ITrackAudioOutput : public IAudioSource
{
public:
    virtual ~ITrackAudioOutput() = default;

    virtual const AudioOutputParams& outputParams() const = 0;
    virtual void applyOutputParams(const AudioOutputParams& requiredParams) = 0;
    virtual async::Channel<AudioOutputParams> outputParamsChanged() const = 0;

    virtual AudioSignalChanges audioSignalChanges() const = 0;
};

using ITrackAudioInputPtr = std::shared_ptr<ITrackAudioInput>;
using ITrackAudioOutputPtr = std::shared_ptr<ITrackAudioOutput>;

struct Track : public async::Asyncable
{
public:
    Track(const TrackType trackType = Undefined)
        : type(trackType) {}
    virtual ~Track() = default;

    TrackId id = -1;
    TrackType type = Undefined;
    TrackName name;

    bool isValid() const
    {
        return id != -1
               && type != Undefined
               && inputHandler
               && outputHandler;
    }

    virtual PlaybackData playbackData() const = 0;
    virtual bool setPlaybackData(const PlaybackData& data) = 0;

    AudioInputParams inputParams() const
    {
        if (!inputHandler) {
            return {};
        }

        return inputHandler->inputParams();
    }

    bool setInputParams(const AudioInputParams& requiredParams)
    {
        if (!inputHandler) {
            return false;
        }

        inputHandler->applyInputParams(requiredParams);

        return true;
    }

    AudioOutputParams outputParams() const
    {
        if (!outputHandler) {
            return {};
        }

        return outputHandler->outputParams();
    }

    bool setOutputParams(const AudioOutputParams& requiredParams)
    {
        if (!outputHandler) {
            return false;
        }

        outputHandler->applyOutputParams(requiredParams);

        return true;
    }

    ITrackAudioInputPtr inputHandler = nullptr;
    ITrackAudioOutputPtr outputHandler = nullptr;

    async::Channel<AudioInputParams> inputParamsChanged()
    {
        if (!inputHandler) {
            return {};
        }

        return inputHandler->inputParamsChanged();
    }

    async::Channel<AudioOutputParams> outputParamsChanged()
    {
        if (!outputHandler) {
            return {};
        }

        return outputHandler->outputParamsChanged();
    }

    async::Channel<PlaybackData> playbackDataChanged;
};

struct EventTrack : public Track
{
public:
    EventTrack()
        : Track(Event_track) {}

    PlaybackData playbackData() const override
    {
        return m_data;
    }

    bool setPlaybackData(const PlaybackData& data) override
    {
        mpe::PlaybackData newData = std::get<mpe::PlaybackData>(data);

        if (m_data == newData) {
            return false;
        }

        m_data = newData;
        playbackDataChanged.send(std::move(newData));

        return true;
    }

private:
    mpe::PlaybackData m_data;
};

struct SoundTrack : public Track
{
    SoundTrack()
        : Track(Sound_track) {}

    PlaybackData playbackData() const override
    {
        return m_ioDevice;
    }

    bool setPlaybackData(const PlaybackData& data) override
    {
        io::IODevice* newDevice = std::get<io::IODevice*>(data);

        if (m_ioDevice == newDevice) {
            return false;
        }

        m_ioDevice->close();
        m_ioDevice = newDevice;
        playbackDataChanged.send(std::move(newDevice));

        return true;
    }

private:
    io::IODevice* m_ioDevice = nullptr;
};

using TrackPtr = std::shared_ptr<Track>;
using EventTrackPtr = std::shared_ptr<EventTrack>;
using SoundTrackPtr = std::shared_ptr<SoundTrack>;

using TracksMap = std::map<TrackId, TrackPtr>;
}
