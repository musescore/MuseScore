#ifndef MU_IMPORTEXPORT_GPRHYTHM_H
#define MU_IMPORTEXPORT_GPRHYTHM_H

#include <utility>

namespace mu::iex::guitarpro {
class GPRhythm
{
public:
    enum class RhytmType {
        SixtyFourth, ThirtySecond, Sixteenth, Eighth, Quarter, Half, Whole
    };
    struct Tuplet {
        int num{ -1 };
        int denom{ -1 };
    };

    void setRhytm(RhytmType t) { _rhythm = t; }

    std::pair<int, RhytmType> length() const { return { _dotCount, _rhythm }; }

    RhytmType type() const { return _rhythm; }

    void setTuplet(Tuplet t) { _tuplet = t; }
    Tuplet tuplet() const { return _tuplet; }

    void setDotCount(int c) { _dotCount = c; }
    int dotCount() const { return _dotCount; }

private:
    RhytmType _rhythm{ RhytmType::Whole };
    Tuplet _tuplet;
    int _dotCount{ 0 };
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GPRHYTHM_H
