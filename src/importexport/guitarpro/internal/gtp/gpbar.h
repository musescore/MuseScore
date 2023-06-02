#ifndef MU_IMPORTEXPORT_GPBAR_H
#define MU_IMPORTEXPORT_GPBAR_H

#include "gpvoice.h"

namespace mu::iex::guitarpro {
class GPBar
{
public:
    enum class ClefType {
        G2, F4, C3, C4, Neutral
    };
    enum class OttaviaType {
        ma15, va8, Regular, vb8, mb15
    };
    enum class SimileMark {
        None, Simple, FirstOfDouble, SecondOfDouble
    };

    struct Clef {
        ClefType type{ ClefType::G2 };
        OttaviaType ottavia{ OttaviaType::Regular };

        bool operator==(const GPBar::Clef& c) const { return type == c.type && ottavia == c.ottavia; }
    };

    ~GPBar() = default;
    void addGPVoice(std::unique_ptr<GPVoice>&& v) { _voices.push_back(std::move(v)); }

    void setClefType(GPBar::ClefType clef) { _clef.type = clef; }
    void setOttaviaType(GPBar::OttaviaType ottavia) { _clef.ottavia = ottavia; }
    GPBar::Clef clef() const { return _clef; }

    void setSimileMark(SimileMark s) { _simileMark = s; }
    SimileMark simileMark() const { return _simileMark; }

    void setId(int id) { _id = id; }

    const std::vector<std::unique_ptr<GPVoice> >& voices() const { return _voices; }

private:

    int _id{ -1 };
    std::vector<std::unique_ptr<GPVoice> > _voices;
    Clef _clef;
    SimileMark _simileMark;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPBAR_H
