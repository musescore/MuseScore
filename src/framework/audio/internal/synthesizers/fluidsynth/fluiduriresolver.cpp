#include "fluiduriresolver.h"

#include "fluidsynth.h"
#include "internal/audiosanitizer.h"

using namespace mu::audio::synth;

static const std::string SOUND_FONT_PARAM_KEY("sfpath");

static const std::map<SoundFontFormat, std::string> FLUID_SF_FILE_EXTENSIONS =
{
    { SoundFontFormat::SF2, "*.sf2" },
    { SoundFontFormat::SF3, "*.sf3" }
};

void FluidUriResolver::refresh()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_uriCache.clear();
    io::paths dirPaths = configuration()->soundFontDirectories();

    for (const auto& pair : FLUID_SF_FILE_EXTENSIONS) {
        updateCaches(dirPaths, pair.second);
    }
}

SynthUriList FluidUriResolver::resolve() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_uriCache;
}

void FluidUriResolver::updateCaches(const io::paths& dirPaths, const std::string& fileExtension)
{
    for (const io::path& path : dirPaths) {
        RetVal<io::paths> files = fileSystem()->scanFiles(path, { QString::fromStdString(fileExtension) });
        if (!files.ret) {
            continue;
        }

        for (const io::path& filePath : files.val) {
            SynthUri uri(SynthType::Fluid);
            uri.addParam(SOUND_FONT_PARAM_KEY, Val(filePath));
            m_uriCache.push_back(std::move(uri));
        }
    }
}
