#ifndef MU_INSPECTOR_CHORDSYMBOLTYPES_H
#define MU_INSPECTOR_CHORDSYMBOLTYPES_H

#include "qobjectdefs.h"

namespace mu::inspector {
class ChordSymbolTypes
{
    Q_GADGET

public:
    enum class VoicingType {
        VOICING_INVALID = -1,
        VOICING_AUTO,
        VOICING_ROOT_ONLY,
        VOICING_CLOSE,
        VOICING_DROP_TWO,
        VOICING_SIX_NOTE,
        VOICING_FOUR_NOTE,
        VOICING_THREE_NOTE
    };

    enum class DurationType {
        DURATION_INVALID = -1,
        DURATION_UNTIL_NEXT_CHORD_SYMBOL,
        DURATION_STOP_AT_MEASURE_END,
        DURATION_SEGMENT_DURATION
    };

    Q_ENUM(VoicingType)
    Q_ENUM(DurationType)
};
}

#endif // MU_INSPECTOR_CHORDSYMBOLTYPES_H
