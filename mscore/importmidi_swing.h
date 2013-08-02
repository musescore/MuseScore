#ifndef IMPORTMIDI_SWING_H
#define IMPORTMIDI_SWING_H

#include "importmidi_operation.h"


namespace Ms {

class Staff;

namespace Swing {

void detectSwing(Staff *staff, MidiOperation::Swing swingType);

} // namespace Swing
} // namespace Ms


#endif // IMPORTMIDI_SWING_H
