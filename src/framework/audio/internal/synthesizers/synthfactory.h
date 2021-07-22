#ifndef MU_AUDIO_SYNTHFACTORY_H
#define MU_AUDIO_SYNTHFACTORY_H

#include <map>

#include "async/asyncable.h"

#include "synthtypes.h"
#include "isynthfactory.h"

namespace mu::audio::synth {
class SynthFactory : public ISynthFactory, public async::Asyncable
{
public:
    void init(const AudioInputParams& defaultInputParams) override;

    ISynthesizerPtr createNew(const AudioInputParams& params) const override;
    ISynthesizerPtr createDefault() const override;

    void registerCreator(const AudioSourceType type, ISynthCreatorPtr creator) override;

private:
    std::map<AudioSourceType, ISynthCreatorPtr> m_creators;

    AudioInputParams m_defaultInputParams;
};
}

#endif // MU_AUDIO_SYNTHFACTORY_H
