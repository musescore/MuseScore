#ifndef MU_AUDIO_FLUIDSYNTHURIRESOLVER_H
#define MU_AUDIO_FLUIDSYNTHURIRESOLVER_H

#include <vector>

#include "modularity/ioc.h"
#include "system/ifilesystem.h"

#include "iaudioconfiguration.h"
#include "isynthuriprovider.h"

namespace mu::audio::synth {
class FluidUriResolver : public ISynthUriProvider::IUriResolver
{
    INJECT(audio, IAudioConfiguration, configuration)
    INJECT(audio, system::IFileSystem, fileSystem)

public:
    void refresh() override;
    SynthUriList resolve() const override;

private:
    void updateCaches(const io::paths& dirPaths, const std::string& fileExtension);

    SynthUriList m_uriCache;
};
}

#endif // MU_AUDIO_FLUIDSYNTHURIRESOLVER_H
