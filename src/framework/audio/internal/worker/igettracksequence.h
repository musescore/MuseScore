#ifndef MU_AUDIO_IGETTRACKSEQUENCE_H
#define MU_AUDIO_IGETTRACKSEQUENCE_H

#include "itracksequence.h"
#include "audiotypes.h"

namespace mu::audio {
class IGetTrackSequence
{
public:
    virtual ITrackSequencePtr sequence(const TrackSequenceId id) const = 0;
};

using IGetTrackSequencePtr = std::shared_ptr<IGetTrackSequence>;
}

#endif // MU_AUDIO_IGETTRACKSEQUENCE_H
