#ifndef IMPORTMIDI_TIE_H
#define IMPORTMIDI_TIE_H

#include <set>


namespace Ms {

class Segment;
class ChordRest;
class Staff;

namespace MidiTie {


bool isTiedFor(const Segment *seg, int strack, int voice);
bool isTiedBack(const Segment *seg, int strack, int voice);


class TieStateMachine
      {
   public:
      enum class State : char
            {
            UNTIED, TIED_FOR, TIED_BOTH, TIED_BACK
            };

      void addSeg(const Segment *seg, int strack);
      State state() const { return state_; }

   private:
      std::set<int> tiedVoices;
      State state_ = State::UNTIED;
      };


#ifdef QT_DEBUG
bool areTiesConsistent(const Staff *staff);
#endif


} // namespace MidiTie
} // namespace Ms


#endif // IMPORTMIDI_TIE_H
