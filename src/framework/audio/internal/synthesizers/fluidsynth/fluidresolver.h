#ifndef MU_AUDIO_FLUIDSYNTHCREATOR_H
#define MU_AUDIO_FLUIDSYNTHCREATOR_H

#include <unordered_map>

#include "async/asyncable.h"
#include "async/channel.h"
#include "modularity/ioc.h"
#include "system/ifilesystem.h"

#include "isynthfactory.h"
#include "iaudioresourceprovider.h"

namespace mu::audio::synth {
class FluidResolver : public ISynthFactory::ISynthCreator, public IAudioResourceProvider::IResourceResolver, public async::Asyncable
{
    INJECT(audio, system::IFileSystem, fileSystem)
public:
    explicit FluidResolver(const io::paths& soundFontDirs, async::Channel<io::paths> sfDirsChanges);

    ISynthesizerPtr create(const AudioInputParams& params) override;

    void refresh() override;
    AudioResourceIdList resolve() const override;

private:
    void updateCaches(const std::string& fileExtension);

    io::paths m_soundFontDirs;
    std::unordered_map<AudioResourceId, io::path> m_resourcesCache;
};
}

#endif // MU_AUDIO_FLUIDSYNTHCREATOR_H
