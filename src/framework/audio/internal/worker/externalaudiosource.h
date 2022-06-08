#ifndef EXTERNALAUDIOSOURCE_H
#define EXTERNALAUDIOSOURCE_H
#include "track.h"
#include "audiotypes.h"
#include "thirdparty/dr_libs/dr_wav.h"
namespace mu::audio {
class ExternalAudioSource : public ITrackAudioInput, public async::Asyncable
{
public:
    ExternalAudioSource();
    explicit ExternalAudioSource(const TrackId trackId, const io::Device*);

    bool isActive() const override;
    void setIsActive(const bool active) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;

    void seek(const msecs_t newPositionMsecs) override;

    const AudioInputParams& inputParams() const override;
    void applyInputParams(const AudioInputParams& requiredParams) override;
    async::Channel<AudioInputParams> inputParamsChanged() const override;
private:
    TrackId m_trackId = -1;
    const io::Device* m_playbackData;
    samples_t m_sampleRate = 0;
    bool m_active = false;
    int m_channels = 0;
    AudioInputParams m_params;
    async::Channel<AudioInputParams> m_paramsChanges;
    drwav m_wav;
    QByteArray b;
    async::Channel<unsigned int> m_audioChannelsCountChange;
};
}
#endif // EXTERNALAUDIOSOURCE_H
