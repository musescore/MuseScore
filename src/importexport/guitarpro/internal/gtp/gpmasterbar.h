#ifndef GPMASTERBAR_H
#define GPMASTERBAR_H

#include "gpbar.h"

namespace Ms {
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
    enum class KeySig {
        C_B = -7,
        G_B, D_B, A_B, E_B, B_B, F,   C,
        G,   D,   A,   E,   B,   F_S, C_S,
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
        int enumerator{ 0 };
        int denumerator{ 0 };
    };

    struct Fermata {
        enum class Type {
            Short, Medium, Long
        };
        Type type{ Type::Medium };
        float lenght{ 0 };
        int offsetEnum; //enumerator of offset field in GP
        int offsetDenum; //denumerator of offset field in GP
    };
    struct Direction {
        QString target;
        QString jump;
    };

    ~GPMasterBar() = default;

    void addGPBar(std::unique_ptr<GPBar>&& b) { _bars.push_back(std::move(b)); }
    void setTimeSig(const GPMasterBar::TimeSig& sig) { _timeSig = sig; }
    TimeSig timeSig() const { return _timeSig; }

    void setKeySig(GPMasterBar::KeySig sig) { _keySig = sig; }
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

    void setSection(std::pair<QString, QString>&& s) { _section.swap(s); }
    const std::pair<QString, QString>& section() const { return _section; }

    void setDirectionTarget(const QString& d) { _direction.target = d; }
    void setDirectionJump(const QString& d) { _direction.jump = d; }
    const Direction& direction() const { return _direction; }

    void setId(int id) { _id = id; }
    int id() const { return _id; } //debug helper

    const std::vector<std::unique_ptr<GPBar> >& bars() const { return _bars; }

private:

    int _id{ -1 };
    std::vector<std::unique_ptr<GPBar> > _bars;
    std::vector<Fermata> _fermatas;
    TimeSig _timeSig;
    KeySig _keySig;
    Repeat _repeat;
    std::vector<int> _alternateEndings;
    TripletFeelType _tripletFeel = TripletFeelType::None;
    BarlineType _barlineType = BarlineType::NORMAL;
    bool _freeTime = false;
    std::pair<QString, QString> _section;
    Direction _direction;
};
}

#endif // GPMASTERBAR_H
