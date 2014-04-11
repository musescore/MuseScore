#ifndef IMPORTMIDI_TUPLET_TONOTES_H
#define IMPORTMIDI_TUPLET_TONOTES_H


namespace Ms {

class ReducedFraction;
class DurationElement;
class Staff;

namespace MidiTuplet {

struct TupletData;

void addElementToTuplet(int voice,
                        const ReducedFraction &onTime,
                        const ReducedFraction &len,
                        DurationElement *el,
                        std::multimap<ReducedFraction, TupletData> &tuplets);

void createTuplets(Staff *staff,
                   const std::multimap<ReducedFraction, TupletData> &tuplets);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_TONOTES_H
