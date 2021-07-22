#ifndef MU_AUDIO_SYNTHURIPROVIDER_H
#define MU_AUDIO_SYNTHURIPROVIDER_H

#include <map>

#include "async/asyncable.h"

#include "iaudioresourceprovider.h"

namespace mu::audio::synth {
class AudioResourceProvider : public IAudioResourceProvider, public async::Asyncable
{
public:
    async::Promise<AudioResourceIdList> resourceIdList(const AudioSourceType type) const override;
    void registerResolver(const AudioSourceType type, IResourceResolverPtr resolver) override;
    void refreshResolvers() override;

private:
    std::map<AudioSourceType, IResourceResolverPtr> m_resolvers;
};
}

#endif // MU_AUDIO_SYNTHURIPROVIDER_H
