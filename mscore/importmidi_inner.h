#ifndef IMPORTMIDI_INNER_H
#define IMPORTMIDI_INNER_H

#include "importmidi_fraction.h"
#include "importmidi_tuplet.h"


// ---------------------------------------------------------------------------------------
// These inner classes definitions are used in cpp files only
// Include this header to link tests
// ---------------------------------------------------------------------------------------


namespace Ms {
namespace Meter {

            // max level for tuplets: duration cannot go over the tuplet boundary
            // this level should be greater than any other level
const int TUPLET_BOUNDARY_LEVEL = 10;

struct MaxLevel
      {
      int level = 0;                   // 0 - the biggest, whole bar level; other: -1, -2, ...
      int levelCount = 0;              // number of positions with 'level' value
      ReducedFraction pos = {-1, 1};   // first position with value 'level'; -1 - undefined pos
      };

struct DivLengthInfo
      {
      ReducedFraction len;
      int level;
      };

struct DivisionInfo
      {
      ReducedFraction onTime;        // division start tick (tick is counted from the beginning of bar)
      ReducedFraction len;           // length of this whole division
      bool isTuplet = false;
      std::vector<DivLengthInfo> divLengths;    // lengths of 'len' subdivisions
      };

enum class DurationType;

} // namespace Meter

class Staff;
class Score;
class MidiTrack;
class DurationElement;
class MidiChord;
class MidiEvent;
class TDuration;
class Measure;

class MTrack {
   public:
      int program = 0;
      Staff* staff = nullptr;
      const MidiTrack* mtrack = nullptr;
      QString name;
      bool hasKey = false;
      int indexOfOperation = 0;
      int division = 0;
      int initLyricTrackIndex = -1;

      std::multimap<ReducedFraction, MidiChord> chords;
      std::multimap<ReducedFraction, MidiTuplet::TupletData> tuplets;   // <tupletOnTime, ...>

      void convertTrack(const ReducedFraction &lastTick);
      void processPendingNotes(QList<MidiChord>& midiChords,
                               int voice,
                               const ReducedFraction &startChordTickFrac,
                               const ReducedFraction &nextChordTick);
      void processMeta(int tick, const MidiEvent& mm);
      void fillGapWithRests(Score *score, int voice, const ReducedFraction &startChordTickFrac,
                            const ReducedFraction &restLength, int track);
      QList<std::pair<ReducedFraction, TDuration> >
            toDurationList(const Measure *measure, int voice, const ReducedFraction &startTick,
                           const ReducedFraction &len, Meter::DurationType durationType);
      void createKeys(int accidentalType);
      };

namespace MidiTuplet {

struct TupletInfo
      {
      ReducedFraction onTime = {-1, 1};  // invalid
      ReducedFraction len = {-1, 1};
      int tupletNumber = -1;
      ReducedFraction tupletQuant;
      ReducedFraction regularQuant;
                  // <chord onTime, chord iterator>
      std::map<ReducedFraction, std::multimap<ReducedFraction, MidiChord>::iterator> chords;
      ReducedFraction tupletSumError;
      ReducedFraction regularSumError;
      ReducedFraction sumLengthOfRests;
      int firstChordIndex = -1;
      };

} // namespace MidiTuplet

namespace MidiCharset {

QString convertToCharset(const std::string &text);
QString defaultCharset();
std::string fromUchar(const uchar *text);

} // namespace MidiCharset
} // namespace Ms


#endif // IMPORTMIDI_INNER_H
