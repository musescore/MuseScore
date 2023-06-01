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
} // namespace mu::iex::guitarpro
