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

void createTupletNotes(Staff *staff,
                       const std::multimap<ReducedFraction, TupletData> &tuplets);

#ifdef QT_DEBUG
bool haveTupletsEnoughElements(const Staff *staff);
#endif

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_TONOTES_H
