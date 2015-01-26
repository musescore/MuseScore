#ifndef IMPORTMIDI_OPERATION_H
#define IMPORTMIDI_OPERATION_H

#include "importmidi_fraction.h"

#include <set>


namespace Ms {

// all enums below should have default indexes like 0, 1, 2...
// text names for enum items are in TracksModel class

namespace MidiOperations {

enum class QuantValue {
      Q_4 = 0,
      Q_8,
      Q_16,
      Q_32,
      Q_64,
      Q_128
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
