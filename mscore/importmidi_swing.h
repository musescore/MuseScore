#ifndef IMPORTMIDI_SWING_H
#define IMPORTMIDI_SWING_H

#include "importmidi_operation.h"
#include "libmscore/fraction.h"


namespace Ms {

class Score;
class Staff;
class ChordRest;

namespace Swing {

class SwingDetector
      {
   public:
      SwingDetector(MidiOperation::Swing st);

      void add(ChordRest *cr);
      bool wasSwingApplied() const { return swingApplied; }

   private:
      std::vector<ChordRest *> elements;
      Fraction sumLen;
      const Fraction FULL_LEN = Fraction(1, 4);
      MidiOperation::Swing swingType;
      bool swingApplied = false;

      void reset();
      void append(ChordRest *cr);
      void checkNormalSwing();
      void checkShuffle();
      void applySwing();
      bool areAllTuplets() const;
      bool areAllNonTuplets() const;
      };

void detectSwing(Staff *staff, MidiOperation::Swing swingType);

} // namespace Swing
} // namespace Ms


#endif // IMPORTMIDI_SWING_H
