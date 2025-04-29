#include "gptrack.h"

namespace mu::iex::guitarpro {
void GPTrack::addSound(Sound sound)
{
    muse::String key = sound.path;

    _sounds.insert({ key, sound });
}
} // namespace mu::iex::guitarpro
