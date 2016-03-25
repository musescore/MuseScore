#ifndef IMPORTMIDI_CLEF_H
#define IMPORTMIDI_CLEF_H


namespace Ms {

class Staff;
class InstrumentTemplate;
enum class ClefType : signed char;

namespace MidiClef {

bool hasGFclefs(const InstrumentTemplate *templ);
void createClefs(Staff *staff, int indexOfOperation, bool isDrumTrack);
ClefType clefTypeFromAveragePitch(int averagePitch);

} // namespace MidiClef
} // namespace Ms


#endif // IMPORTMIDI_CLEF_H
