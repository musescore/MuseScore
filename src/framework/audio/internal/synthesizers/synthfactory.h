#ifndef MU_AUDIO_SYNTHFACTORY_H
#define MU_AUDIO_SYNTHFACTORY_H

#include <map>

#include "async/asyncable.h"

#include "isynthfactory.h"

namespace mu::audio::synth {
class SynthFactory : public ISynthFactory, public async::Asyncable
{
public:
    void init(const SynthType defaultType, ISynthCreatorPtr defaultCreator, SoundFontPath defaultSoundFontPath) override;

    ISynthesizerPtr createNew(const SynthType type, const SoundFontPath& sfPath) const override;
    ISynthesizerPtr createDefault() const override;

    void registerCreator(const SynthType type, ISynthCreatorPtr creator) override;

private:
    std::map<SynthType, ISynthCreatorPtr> m_creators;

    SynthType m_defaultSynthType = SynthType::Undefined;
    SoundFontPath m_defaultSoundFontPath;
};
}

#endif // MU_AUDIO_SYNTHFACTORY_H
