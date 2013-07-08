#ifndef IMPORTMIDI_CHORD_H
#define IMPORTMIDI_CHORD_H


namespace Ms {

class Tie;

class MidiNote {
   public:
      int pitch;
      int velo;
      int len;
      Tie* tie = nullptr;
      };

class MidiChord {
   public:
      int voice = 0;
      QList<MidiNote> notes;
      };

std::multimap<int, MidiChord>::iterator
findFirstChordInRange(int startRangeTick,
                      int endRangeTick,
                      const std::multimap<int, MidiChord>::iterator &startChordIt,
                      const std::multimap<int, MidiChord>::iterator &endChordIt);

std::multimap<int, MidiChord>::iterator
findEndChordInRange(int endRangeTick,
                    const std::multimap<int, MidiChord>::iterator &startChordIt,
                    const std::multimap<int, MidiChord>::iterator &endChordIt);

} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
