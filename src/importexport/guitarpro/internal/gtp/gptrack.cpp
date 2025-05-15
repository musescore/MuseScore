#include "gptrack.h"

namespace mu::iex::guitarpro {
void GPTrack::addSound(Sound sound)
{
    muse::String key = sound.path + u";" + sound.name + u";" + sound.role;

    _sounds.insert({ key, sound });
}
} // namespace mu::iex::guitarpro
