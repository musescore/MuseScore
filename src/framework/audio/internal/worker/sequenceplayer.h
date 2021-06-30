#ifndef MU_AUDIO_SEQUENCEPLAYER_H
#define MU_AUDIO_SEQUENCEPLAYER_H

#include "async/asyncable.h"

#include "isequenceplayer.h"
#include "igettracks.h"
#include "iclock.h"

namespace mu::audio {
class SequencePlayer : public ISequencePlayer, public async::Asyncable
{
public:
    explicit SequencePlayer(IGetTracks* getTracks, IClockPtr clock);

    void play() override;
    void seek(const msecs_t newPositionMsecs) override;
    void stop() override;
    void pause() override;

    Ret setLoop(const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetLoop() override;

    async::Channel<msecs_t> playbackPositionMSecs() const override;

private:
    TracksMap tracks() const;

    IGetTracks* m_getTracks = nullptr;
    IClockPtr m_clock = nullptr;
};
}

#endif // MU_AUDIO_SEQUENCEPLAYER_H
