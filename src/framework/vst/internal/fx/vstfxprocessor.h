#ifndef MU_VST_VSTFXPROCESSOR_H
#define MU_VST_VSTFXPROCESSOR_H

#include <memory>

#include "async/asyncable.h"
#include "audio/ifxprocessor.h"

#include "internal/vstaudioclient.h"
#include "internal/vstplugin.h"
#include "vsttypes.h"

namespace mu::vst {
class VstFxProcessor : public audio::IFxProcessor, public async::Asyncable
{
public:
    explicit VstFxProcessor(VstPluginPtr&& pluginPtr, const audio::AudioFxParams& params);

    void init();

    audio::AudioFxType type() const override;
    const audio::AudioFxParams& params() const override;
    async::Channel<audio::AudioFxParams> paramsChanged() const override;
    void setSampleRate(unsigned int sampleRate) override;
    bool active() const override;
    void setActive(bool active) override;
    void process(float* buffer, size_t bufferSize, unsigned int sampleCount) override;

private:
    bool m_inited = false;

    VstPluginPtr m_pluginPtr = nullptr;
    std::unique_ptr<VstAudioClient> m_vstAudioClient = nullptr;

    audio::AudioFxParams m_params;
    async::Channel<audio::AudioFxParams> m_paramsChanges;
};

using VstFxPtr = std::shared_ptr<VstFxProcessor>;
}

#endif // MU_VST_VSTFXPROCESSOR_H
