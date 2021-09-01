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

#ifndef MU_AUDIO_TRACK_H
#define MU_AUDIO_TRACK_H

#include <unordered_map>
#include <memory>
#include <vector>

#include "async/channel.h"

#include "iaudiosource.h"
#include "imixerchannel.h"
#include "audiotypes.h"

namespace mu::audio {
enum TrackType {
    Undefined = -1,
    Midi,
    Audio
};

class ITrackAudioInput : public IAudioSource
{
public:
    virtual ~ITrackAudioInput() = default;

    virtual void seek(const msecs_t newPositionMsecs) = 0;
    virtual void applyInputParams(const AudioInputParams& originParams, AudioInputParams& resultParams) = 0;
};

class ITrackAudioOutput : public IAudioSource
{
public:
    virtual ~ITrackAudioOutput() = default;

    virtual void applyOutputParams(const AudioOutputParams& originParams, AudioOutputParams& resultParams) = 0;

    // root mean square of a processed sample block
    virtual async::Channel<audioch_t, float> signalAmplitudeRmsChanged() const = 0;

    // root mean square of a processed sample block in the "decibels relative to full scale" units
    virtual async::Channel<audioch_t, volume_dbfs_t> volumePressureDbfsChanged() const = 0;
};

using ITrackAudioInputPtr = std::shared_ptr<ITrackAudioInput>;
using ITrackAudioOutputPtr = std::shared_ptr<ITrackAudioOutput>;

struct Track
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
        return m_inParams;
    }

    bool setInputParams(const AudioInputParams& requiredParams)
    {
        if (m_inParams.isValid() && m_inParams == requiredParams) {
            return false;
        }

        AudioInputParams appliedParams;
        inputHandler->applyInputParams(requiredParams, appliedParams);
        m_inParams = std::move(appliedParams);
        inputParamsChanged.send(m_inParams);

        return true;
    }

    AudioOutputParams outputParams() const
    {
        return m_outParams;
    }

    bool setOutputParams(const AudioOutputParams& requiredParams)
    {
        if (m_outParams == requiredParams) {
            return false;
        }

        AudioOutputParams appliedParams;
        outputHandler->applyOutputParams(requiredParams, appliedParams);
        m_outParams = std::move(appliedParams);
        outputParamsChanged.send(m_outParams);

        return true;
    }

    async::Channel<AudioInputParams> inputParamsChanged;
    async::Channel<AudioOutputParams> outputParamsChanged;
    async::Channel<PlaybackData> playbackDataChanged;

    ITrackAudioInputPtr inputHandler = nullptr;
    ITrackAudioOutputPtr outputHandler = nullptr;

private:
    AudioInputParams m_inParams;
    AudioOutputParams m_outParams;
};

struct MidiTrack : public Track
{
public:
    MidiTrack()
        : Track(Midi) {}

    PlaybackData playbackData() const override
    {
        return m_midiData;
    }

    bool setPlaybackData(const PlaybackData& data) override
    {
        midi::MidiData newMidiData = std::get<midi::MidiData>(data);

        if (m_midiData == newMidiData) {
            return false;
        }

        m_midiData = newMidiData;
        playbackDataChanged.send(std::move(newMidiData));

        return true;
    }

private:
    midi::MidiData m_midiData;
};

struct AudioTrack : public Track
{
    AudioTrack()
        : Track(Audio) {}

    PlaybackData playbackData() const override
    {
        return m_ioDevice;
    }

    bool setPlaybackData(const PlaybackData& data) override
    {
        io::Device* newDevice = std::get<io::Device*>(data);

        if (m_ioDevice == newDevice) {
            return false;
        }

        m_ioDevice->close();
        m_ioDevice = newDevice;
        playbackDataChanged.send(std::move(newDevice));

        return true;
    }

private:
    io::Device* m_ioDevice = nullptr;
};

using TrackPtr = std::shared_ptr<Track>;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using AudioTrackPtr = std::shared_ptr<AudioTrack>;

using TracksMap = std::unordered_map<TrackId, TrackPtr>;
}

#endif // MU_AUDIO_TRACK_H
