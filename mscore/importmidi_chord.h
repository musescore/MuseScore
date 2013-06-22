#ifndef IMPORTMIDI_CHORD_H
#define IMPORTMIDI_CHORD_H


namespace Ms {

class Tie;

class MidiNote {
   public:
      int pitch;
      int velo;
      int onTime;
      int len;
      Tie* tie = nullptr;
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
