#include "gptrack.h"

namespace mu::engraving {
void GPTrack::addSound(Sound sound)
{
    String key = sound.path + u";" + sound.name + u";" + sound.role;

    _sounds.insert({ key, sound });
}
}//namespace mu::engraving
