#ifndef MU_AUDIO_SYNTHURIPROVIDER_H
#define MU_AUDIO_SYNTHURIPROVIDER_H

#include <map>

#include "async/asyncable.h"

#include "isynthuriprovider.h"

namespace mu::audio::synth {
class SynthUriProvider : public ISynthUriProvider, public async::Asyncable
{
public:
    async::Promise<SynthUriList> uriList(const SynthType type) const override;
    void registerResolver(const SynthType type, IUriResolverPtr resolver) override;
    void refreshResolvers() override;

private:
    std::map<SynthType, IUriResolverPtr> m_resolvers;
};
}

#endif // MU_AUDIO_SYNTHURIPROVIDER_H
