#include "fluidsynthcreator.h"

#include "internal/audiosanitizer.h"
#include "fluidsynth.h"

using namespace mu::audio::synth;

ISynthesizerPtr FluidSynthCreator::create(const SoundFontPath& sfPath)
{
    ONLY_AUDIO_WORKER_THREAD;

    ISynthesizerPtr synth = std::make_shared<FluidSynth>();
    synth->init();
    synth->addSoundFonts({ sfPath });

    return synth;
}
