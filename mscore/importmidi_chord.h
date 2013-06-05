#ifndef IMPORTMIDI_CHORD_H
#define IMPORTMIDI_CHORD_H


namespace Ms {

class Tie;

class MidiNote {
   public:
      int pitch, velo;
      int onTime, len;
      Tie* tie = 0;
      };

class MidiChord {
   public:
      int voice = 0;
      int onTime;
      int duration;
      QList<MidiNote> notes;
      };

} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
