#ifndef MU_AUDIO_ISYNTHFACTORY_H
#define MU_AUDIO_ISYNTHFACTORY_H

#include <memory>

#include "modularity/imoduleexport.h"

#include "isynthesizer.h"
#include "synthtypes.h"

namespace mu::audio::synth {
class ISynthFactory : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISynthFactory)

public:
    virtual ~ISynthFactory() = default;

    class ISynthCreator
    {
    public:
        virtual ~ISynthCreator() = default;

        virtual ISynthesizerPtr create(const SynthUri& uri) = 0;
    };
    using ISynthCreatorPtr = std::shared_ptr<ISynthCreator>;

    virtual void init(const SynthUri& defaultUri) = 0;

    virtual ISynthesizerPtr createNew(const SynthUri& uri) const = 0;
    virtual ISynthesizerPtr createDefault() const = 0;
    virtual void registerCreator(const SynthType type, ISynthCreatorPtr creator) = 0;
};

using ISynthFactoryPtr = std::shared_ptr<ISynthFactory>;
}

#endif // MU_AUDIO_ISYNTHFACTORY_H
