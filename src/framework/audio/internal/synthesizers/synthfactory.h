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
    void init(const SynthUri& defaultUri) override;

    ISynthesizerPtr createNew(const SynthUri& uri) const override;
    ISynthesizerPtr createDefault() const override;

    void registerCreator(const SynthType type, ISynthCreatorPtr creator) override;

private:
    std::map<SynthType, ISynthCreatorPtr> m_creators;

    SynthUri m_defaultUri;
};
}

#endif // MU_AUDIO_SYNTHFACTORY_H
