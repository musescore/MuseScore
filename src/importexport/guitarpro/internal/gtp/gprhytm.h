#ifndef GPRHYTM_H
#define GPRHYTM_H

namespace Ms {
class GPRhytm
{
public:
    enum class RhytmType {
        SixtyFourth, ThirtySecond, Sixteenth, Eighth, Quarter, Half, Whole
    };
    struct Tuplet {
        int num{ -1 };
        int denum{ -1 };
    };

    void setRhytm(RhytmType t) { _rhytm = t; }

    std::pair<int, RhytmType> length() const { return { _dotCount, _rhytm }; }

    RhytmType type() const { return _rhytm; }

    void setTuplet(Tuplet t) { _tuplet = t; }
    Tuplet tuplet() const { return _tuplet; }

    void setDotCount(int c) { _dotCount = c; }
    int dotCount() const { return _dotCount; }

private:
    RhytmType _rhytm{ RhytmType::Whole };
    Tuplet _tuplet;
    int _dotCount{ 0 };
};
}
#endif // GPRHYTM_H
