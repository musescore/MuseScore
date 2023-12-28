#ifndef MU_ENGRAVING_GETTREMOLODISPATCHER_H
#define MU_ENGRAVING_GETTREMOLODISPATCHER_H

#include "dom/chord.h"
#include "dom/tremolo.h"
#include "dom/tremolotwochord.h"
#include "dom/tremolosinglechord.h"

namespace mu::engraving::rendering::stable {
inline TremoloDispatcher* tremoloDispatcher(Chord* c)
{
    if (!c) {
        return nullptr;
    }

    if (c->tremoloTwoChord()) {
        return c->tremoloTwoChord()->dispatcher();
    } else if (c->tremoloSingleChord()) {
        return c->tremoloSingleChord()->dispatcher();
    }

    return nullptr;
}
}

#endif // MU_ENGRAVING_GETTREMOLODISPATCHER_H
