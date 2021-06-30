#ifndef MU_AUDIO_TRACK_H
#define MU_AUDIO_TRACK_H

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

struct Track
{
public:
    Track(const TrackType trackType = Undefined)
        : type(trackType) {}

    TrackId id = -1;
    TrackType type = Undefined;
    TrackName name;

    bool isValid() const
    {
        return id != -1
                && type != Undefined
                && audioSource
                && mixerChannel;
    }

    virtual AudioInputParams inputParams() const = 0;
    virtual bool setInputParams(const AudioInputParams& params) = 0;

    AudioOutputParams outputParams() const
    {
        return m_outParams;
    }

    bool setOutputParams(const AudioOutputParams& params)
    {
        if (m_outParams == params) {
            return false;
        }

        m_outParams = params;
        outputParamsChanged.send(params);

        return true;
    }

    async::Channel<AudioInputParams> inputParamsChanged;
    async::Channel<AudioOutputParams> outputParamsChanged;

    IAudioSourcePtr audioSource = nullptr;
    IMixerChannelPtr mixerChannel = nullptr;

private:
    AudioOutputParams m_outParams;
};

struct MidiTrack : public Track
{
public:
    MidiTrack()
        : Track(Midi) {}

    AudioInputParams inputParams() const
    {
        return m_midiData;
    }

    bool setInputParams(const AudioInputParams& params)
    {
        midi::MidiData newMidiData = std::get<midi::MidiData>(params);

        if (m_midiData == newMidiData) {
            return false;
        }

        m_midiData = newMidiData;
        inputParamsChanged.send(std::move(newMidiData));

        return true;
    }

private:
    midi::MidiData m_midiData;
};

struct AudioTrack : public Track
{
    AudioTrack()
        : Track(Audio) {}

    AudioInputParams inputParams() const override
    {
        return m_filePath;
    }

    bool setInputParams(const AudioInputParams& params) override
    {
        io::path newPath = std::get<io::path>(params);

        if (m_filePath == newPath) {
            return false;
        }

        m_filePath = newPath;
        inputParamsChanged.send(std::move(newPath));

        return true;
    }

private:
    io::path m_filePath;
};

using TrackPtr = std::shared_ptr<Track>;
using MidiTrackPtr = std::shared_ptr<MidiTrack>;
using AudioTrackPtr = std::shared_ptr<AudioTrack>;

using TracksMap = std::unordered_map<TrackId, TrackPtr>;
}

#endif // MU_AUDIO_TRACK_H
