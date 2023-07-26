#include "gpbeat.h"

namespace mu::iex::guitarpro {
std::pair<int, GPRhythm::RhytmType> GPBeat::lenth() const
{
    return _rhythm->length();
}

GPRhythm::Tuplet GPBeat::tuplet() const
{
    return _rhythm->tuplet();
}

void GPBeat::addHarmonicMarkType(GPBeat::HarmonicMarkType type)
{
    switch (type) {
    case GPBeat::HarmonicMarkType::Artificial:
        _harmonicMarkInfo.artificial = true;
        break;
    case GPBeat::HarmonicMarkType::Pinch:
        _harmonicMarkInfo.pinch = true;
        break;
    case GPBeat::HarmonicMarkType::Tap:
        _harmonicMarkInfo.tap = true;
        break;
    case GPBeat::HarmonicMarkType::Semi:
        _harmonicMarkInfo.semi = true;
        break;
    case GPBeat::HarmonicMarkType::FeedBack:
        _harmonicMarkInfo.feedback = true;
        break;
    case GPBeat::HarmonicMarkType::None:
        break;
    }
}

void GPBeat::sortGPNotes()
{
    std::sort(_notes.begin(), _notes.end(), comparePitch);
}
} // namespace mu::iex::guitarpro
