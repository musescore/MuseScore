#ifndef MU_AUDIO_IGETTRACKS_H
#define MU_AUDIO_IGETTRACKS_H

#include "track.h"
#include "audiotypes.h"

namespace mu::audio {
class IGetTracks
{
public:
    virtual ~IGetTracks() = default;

    virtual TrackPtr track(const TrackId id) const = 0;
    virtual TracksMap allTracks() const = 0;
};
}

#endif // MU_AUDIO_IGETTRACKS_H
