#include "gpbeat.h"

namespace Ms {
std::pair<int, GPRhythm::RhytmType> GPBeat::lenth() const
{
    return _rhythm->length();
}

GPRhythm::Tuplet GPBeat::tuplet() const
{
    return _rhythm->tuplet();
}
}
