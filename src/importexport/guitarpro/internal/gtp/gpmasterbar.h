#ifndef MU_IMPORTEXPORT_GPMASTERBAR_H
#define MU_IMPORTEXPORT_GPMASTERBAR_H

#include "gpbar.h"

namespace mu::iex::guitarpro {
class GPMasterBar
{
public:
    struct Repeat {
        enum class Type {
            None, Start, End, StartEnd
        };
        Type type{ Type::None };
        int count{ 1 };
    };
    struct KeySig {
        enum class Accidentals {
            C_B = -7,
            G_B, D_B, A_B, E_B, B_B, F, C,
            G, D, A, E, B, F_S, C_S,
        };
        enum class Mode {
            Major,
            Minor
        };
        Accidentals accidentalCount{ Accidentals::C };
        Mode mode{ Mode::Major };
    };
    enum class TripletFeelType {
        Triplet8th,
        Triplet16th,
        Dotted8th,
        Dotted16th,
        Scottish8th,
        Scottish16th,
        None
    };

    enum class BarlineType {
        NORMAL,
        DOUBLE
    };

    struct TimeSig {
        int numerator{ 0 };
        int denominator{ 0 };
    };

    struct Fermata {
        enum class Type {
            Short, Medium, Long
        };
        Type type{ Type::Medium };
        float length{ 0 };
        int offsetNum; //numerator of offset field in GP
        int offsetDenom; //denominator of offset field in GP
    };

    struct Direction {
        enum class Type {
            Repeat, Jump, Marker
        };

        Type type = Type::Repeat;
        muse::String name;
    };

    ~GPMasterBar() = default;

    void addGPBar(std::unique_ptr<GPBar>&& b) { _bars.push_back(std::move(b)); }
    void setTimeSig(const GPMasterBar::TimeSig& sig) { _timeSig = sig; }
    TimeSig timeSig() const { return _timeSig; }
    bool useFlats() const { return _useFlats; }

    void setKeySig(GPMasterBar::KeySig sig, bool useFlats = false) { _keySig = sig; _useFlats = useFlats; }
    KeySig keySig() const { return _keySig; }

    void setFermatas(std::vector<Fermata>&& f) { _fermatas.swap(f); }
    const std::vector<Fermata>& fermatas() const { return _fermatas; }

    void setRepeat(Repeat r) { _repeat = r; }
    Repeat repeat() const { return _repeat; }

    void setTripletFeel(TripletFeelType t) { _tripletFeel = t; }
    TripletFeelType tripletFeel() const { return _tripletFeel; }

    void setBarlineType(BarlineType t) { _barlineType = t; }
    BarlineType barlineType() const { return _barlineType; }

    void setFreeTime(bool freeTime) { _freeTime = freeTime; }
    bool freeTime() const { return _freeTime; }

    void setAlternativeEnding(std::vector<int>&& r) { _alternateEndings.swap(r); }
    const std::vector<int>& alternateEnding() const { return _alternateEndings; }

    void setSection(std::pair<muse::String, muse::String>&& s) { _section.swap(s); }
    const std::pair<muse::String, muse::String>& section() const { return _section; }

    void setDirections(std::vector<Direction>&& d) { _directions.swap(d); }
    const std::vector<Direction>& directions() const { return _directions; }

    void setId(int id) { _id = id; }
    int id() const { return _id; } //debug helper

    const std::vector<std::unique_ptr<GPBar> >& bars() const { return _bars; }

private:

    int _id{ -1 };
    std::vector<std::unique_ptr<GPBar> > _bars;
    std::vector<Fermata> _fermatas;
    std::vector<Direction> _directions;
    TimeSig _timeSig;
    KeySig _keySig;
    bool _useFlats = false;
    Repeat _repeat;
    std::vector<int> _alternateEndings;
    TripletFeelType _tripletFeel = TripletFeelType::None;
    BarlineType _barlineType = BarlineType::NORMAL;
    bool _freeTime = false;
    std::pair<muse::String, muse::String> _section;
    Direction _direction;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPMASTERBAR_H
