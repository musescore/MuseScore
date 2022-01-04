#include "gpbeat.h"

namespace Ms {
std::pair<int, GPRhytm::RhytmType> GPBeat::lenth() const
{
    return _rhytm->length();
}

GPRhytm::Tuplet GPBeat::tuplet() const
{
    return _rhytm->tuplet();
}
}
