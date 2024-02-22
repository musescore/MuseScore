#ifndef MU_ENGRAVING_TREMOLOCOMPAT_H
#define MU_ENGRAVING_TREMOLOCOMPAT_H

namespace mu::engraving {
class Chord;
class TremoloSingleChord;
class TremoloTwoChord;
}

namespace mu::engraving::compat {
struct TremoloCompat {
    Chord* parent = nullptr;
    TremoloSingleChord* single = nullptr;
    TremoloTwoChord* two = nullptr;
};
}

#endif // MU_ENGRAVING_TREMOLOCOMPAT_H
