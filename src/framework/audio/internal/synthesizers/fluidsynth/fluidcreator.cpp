#include "fluidcreator.h"

#include "internal/audiosanitizer.h"
#include "fluidsynth.h"

using namespace mu::audio::synth;

ISynthesizerPtr FluidCreator::create(const SynthUri& uri)
{
    ONLY_AUDIO_WORKER_THREAD;

    ISynthesizerPtr synth = std::make_shared<FluidSynth>();
    synth->init();
    synth->addSoundFonts({ uri.param("sfpath").toPath() });

    return synth;
}
