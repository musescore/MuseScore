#ifndef MU_AUDIO_FLUIDSYNTHCREATOR_H
#define MU_AUDIO_FLUIDSYNTHCREATOR_H

#include "isynthfactory.h"

namespace mu::audio::synth {
class FluidCreator : public ISynthFactory::ISynthCreator
{
public:
    ISynthesizerPtr create(const SynthUri& uri);
};
}

#endif // MU_AUDIO_FLUIDSYNTHCREATOR_H
