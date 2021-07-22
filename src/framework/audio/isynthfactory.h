#ifndef MU_AUDIO_ISYNTHFACTORY_H
#define MU_AUDIO_ISYNTHFACTORY_H

#include <memory>

#include "modularity/imoduleexport.h"

#include "isynthesizer.h"
#include "audiotypes.h"

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

        virtual ISynthesizerPtr create(const AudioInputParams& params) = 0;
    };
    using ISynthCreatorPtr = std::shared_ptr<ISynthCreator>;

    virtual void init(const AudioInputParams& defaultInputParams) = 0;

    virtual ISynthesizerPtr createNew(const AudioInputParams& params) const = 0;
    virtual ISynthesizerPtr createDefault() const = 0;
    virtual void registerCreator(const AudioSourceType type, ISynthCreatorPtr creator) = 0;
};

using ISynthFactoryPtr = std::shared_ptr<ISynthFactory>;
}

#endif // MU_AUDIO_ISYNTHFACTORY_H
