#ifndef MU_AUDIO_ISYNTHURIPROVIDER_H
#define MU_AUDIO_ISYNTHURIPROVIDER_H

#include <memory>

#include "modularity/imoduleexport.h"
#include "async/promise.h"

#include "audiotypes.h"

namespace mu::audio::synth {
class IAudioResourceProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISynthUriProvider)

public:
    virtual ~IAudioResourceProvider() = default;

    class IResourceResolver
    {
    public:
        virtual ~IResourceResolver() = default;

        virtual void refresh() = 0;
        virtual AudioResourceIdList resolve() const = 0;
    };
    using IResourceResolverPtr = std::shared_ptr<IResourceResolver>;

    virtual async::Promise<AudioResourceIdList> resourceIdList(const AudioSourceType type) const = 0;

    virtual void registerResolver(const AudioSourceType type, IResourceResolverPtr resolver) = 0;
    virtual void refreshResolvers() = 0;
};
}

#endif // MU_AUDIO_ISYNTHURIPROVIDER_H
