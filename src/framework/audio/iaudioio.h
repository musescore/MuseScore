#ifndef MU_AUDIO_IAUDIOIO_H
#define MU_AUDIO_IAUDIOIO_H

#include <memory>

#include "async/promise.h"
#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class IAudioIO
{
public:
    virtual ~IAudioIO() = default;

    virtual async::Promise<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const = 0;

    virtual async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual async::Promise<AudioOutputParams> globalOutputParams() const = 0;
    virtual void setGlobalOutputParams(const AudioOutputParams& params) = 0;
    virtual async::Channel<AudioOutputParams> globalOutputParamsChanged() const = 0;

    virtual async::Channel<audioch_t, float> masterSignalAmplitudeChanged() const = 0;
    virtual async::Channel<audioch_t, volume_dbfs_t> masterVolumePressureChanged() const = 0;
};

using IAudioIOPtr = std::shared_ptr<IAudioIO>;
}

#endif // MU_AUDIO_IAUDIOIO_H
