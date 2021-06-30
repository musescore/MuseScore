#ifndef MU_AUDIO_ISEQUENCEIO_H
#define MU_AUDIO_ISEQUENCEIO_H

#include <memory>

#include "retval.h"

#include "audiotypes.h"

namespace mu::audio {
class ISequenceIO
{
public:
    virtual RetVal<AudioInputParams> inputParams(const TrackId id) const = 0;
    virtual RetVal<AudioOutputParams> outputParams(const TrackId id) const = 0;

    virtual void setInputParams(const TrackId id, const AudioInputParams& params) = 0;
    virtual void setOutputParams(const TrackId id, const AudioOutputParams& params) = 0;

    virtual async::Channel<TrackId, AudioInputParams> inputParamsChanged() const = 0;
    virtual async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    // outputDbLevelStream
};

using ISequenceIOPtr = std::shared_ptr<ISequenceIO>;
}

#endif // MU_AUDIO_ISEQUENCEIO_H
