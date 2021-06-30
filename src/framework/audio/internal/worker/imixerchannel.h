#ifndef MU_AUDIO_IMIXERCHANNEL_H
#define MU_AUDIO_IMIXERCHANNEL_H

#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class IMixerChannel
{
public:
    virtual ~IMixerChannel() = default;

    virtual MixerChannelId id() const = 0;

    // root mean square of a processed sample block
    virtual async::Channel<audioch_t, float> signalAmplitudeRmsChanged() const = 0;

    // root mean square of a processed sample block in the "decibels relative to full scale" units
    virtual async::Channel<audioch_t, volume_dbfs_t> volumePressureDbfsChanged() const = 0;
};

using IMixerChannelPtr = std::shared_ptr<IMixerChannel>;
}

#endif // MU_AUDIO_IMIXERCHANNEL_H
