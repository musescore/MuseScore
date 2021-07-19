#ifndef FLUIDSYNTHCREATOR_H
#define FLUIDSYNTHCREATOR_H

#include "isynthfactory.h"

namespace mu::audio::synth {
class FluidSynthCreator : public ISynthFactory::ISynthCreator
{
public:
    ISynthesizerPtr create(const SoundFontPath& sfPath);
};
}

#endif // FLUIDSYNTHCREATOR_H
