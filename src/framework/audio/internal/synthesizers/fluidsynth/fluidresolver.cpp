#include "fluidresolver.h"

#include "internal/audiosanitizer.h"
#include "fluidsynth.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::synth;

static const std::map<SoundFontFormat, std::string> FLUID_SF_FILE_EXTENSIONS =
{
    { SoundFontFormat::SF2, "*.sf2" },
    { SoundFontFormat::SF3, "*.sf3" }
};

FluidResolver::FluidResolver(const io::paths& soundFontDirs, async::Channel<io::paths> sfDirsChanges)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_soundFontDirs = soundFontDirs;

    sfDirsChanges.onReceive(this, [this](const io::paths& newSfDirs) {
        m_soundFontDirs = newSfDirs;

        refresh();
    });
}

ISynthesizerPtr FluidResolver::create(const AudioInputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    ISynthesizerPtr synth = std::make_shared<FluidSynth>();
    synth->init();

    auto search = m_resourcesCache.find(params.resourceId);

    if (search == m_resourcesCache.end()) {
        LOGE() << "Not found: " << params.resourceId;
        return synth;
    }

    synth->addSoundFonts({ search->second });

    return synth;
}

void FluidResolver::refresh()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_resourcesCache.clear();

    for (const auto& pair : FLUID_SF_FILE_EXTENSIONS) {
        updateCaches(pair.second);
    }
}

AudioResourceIdList FluidResolver::resolve() const
{
    ONLY_AUDIO_WORKER_THREAD;

    AudioResourceIdList result(m_resourcesCache.size());

    for (const auto& pair : m_resourcesCache) {
        result.push_back(pair.first);
    }

    return result;
}

void FluidResolver::updateCaches(const std::string& fileExtension)
{
    for (const io::path& path : m_soundFontDirs) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, { QString::fromStdString(fileExtension) });
        if (!files.ret) {
            continue;
        }

        for (const io::path& filePath : files.val) {
            m_resourcesCache.emplace(io::basename(filePath).toStdString(), filePath);
        }
    }
}
