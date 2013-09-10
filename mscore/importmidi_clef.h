#ifndef IMPORTMIDI_CLEF_H
#define IMPORTMIDI_CLEF_H


namespace Ms {

class Staff;
enum class ClefType : signed char;

namespace MidiClef {

void createClefs(Staff *staff, int indexOfOperation, bool isDrumTrack);
int midPitch();
ClefType clefTypeFromAveragePitch(int averagePitch);

} // namespace MidiClef
} // namespace Ms


#endif // IMPORTMIDI_CLEF_H
