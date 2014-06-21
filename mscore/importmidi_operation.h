#ifndef IMPORTMIDI_OPERATION_H
#define IMPORTMIDI_OPERATION_H

#include "importmidi_fraction.h"

#include <set>


namespace Ms {

// all enums below should have default indexes like 0, 1, 2...
// text names for enum items are in TracksModel class

namespace MidiOperations {

enum class QuantValue {
      FROM_PREFERENCES = 0,
      Q_4,
      Q_8,
      Q_16,
      Q_32,
      Q_64,
      Q_128
      };

enum class StaffSplitMethod {
      HAND_WIDTH = 0,
      SPECIFIED_PITCH
      };

enum class StaffSplitOctave {
      C_1 = 0,
      C0,
      C1,
      C2,
      C3,
      C4,
      C5,
      C6,
      C7,
      C8,
      C9
      };

enum class StaffSplitNote {
      C = 0,
      Cis,
      D,
      Dis,
      E,
      F,
      Fis,
      G,
      Gis,
      A,
      Ais,
      H
      };

enum class VoiceCount {
      V_1 = 0,
      V_2,
      V_3,
      V_4
      };

enum class Swing {
      NONE = 0,
      SWING,
      SHUFFLE
      };

enum class TimeSigNumerator {
      _2 = 0,
      _3,
      _4,
      _5,
      _6,
      _7,
      _9,
      _12,
      _15,
      _21
      };

enum class TimeSigDenominator {
      _2 = 0,
      _4,
      _8,
      _16,
      _32
      };

} // namespace MidiOperations
} // namespace Ms


#endif // IMPORTMIDI_OPERATION_H
