#ifndef MU_AUDIO_ISYNTHURIPROVIDER_H
#define MU_AUDIO_ISYNTHURIPROVIDER_H

#include <memory>

#include "modularity/imoduleexport.h"
#include "async/promise.h"

#include "synthtypes.h"

namespace mu::audio::synth {
class ISynthUriProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISynthUriProvider)

public:
    virtual ~ISynthUriProvider() = default;

    class IUriResolver
    {
    public:
        virtual ~IUriResolver() = default;

        virtual void refresh() = 0;
        virtual SynthUriList resolve() const = 0;
    };
    using IUriResolverPtr = std::shared_ptr<IUriResolver>;

    virtual async::Promise<SynthUriList> uriList(const SynthType type) const = 0;

    virtual void registerResolver(const SynthType type, IUriResolverPtr resolver) = 0;
    virtual void refreshResolvers() = 0;
};
}

#endif // MU_AUDIO_ISYNTHURIPROVIDER_H
