#ifndef IMPORTMIDI_CLEF_H
#define IMPORTMIDI_CLEF_H

#include "libmscore/mscore.h"


namespace Ms {

class Staff;

namespace MidiClef {

void createClefs(Staff *staff, int indexOfOperation);
int midPitch();
ClefType clefTypeFromAveragePitch(int averagePitch);

} // namespace MidiClef
} // namespace Ms


#endif // IMPORTMIDI_CLEF_H
