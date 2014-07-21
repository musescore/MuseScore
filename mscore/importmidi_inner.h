#ifndef IMPORTMIDI_INNER_H
#define IMPORTMIDI_INNER_H

#include "importmidi_fraction.h"
#include "importmidi_tuplet.h"
#include "importmidi_operation.h"

#include <vector>
#include <cstddef>
#include <utility>

// ---------------------------------------------------------------------------------------
// These inner classes definitions are used in cpp files only
// Include this header to link tests
// ---------------------------------------------------------------------------------------


namespace Ms {

enum class Key;
struct MidiTimeSig;

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

enum class DurationType : char;

ReducedFraction userTimeSigToFraction(
            MidiOperations::TimeSigNumerator timeSigNumerator,
            MidiOperations::TimeSigDenominator timeSigDenominator);
MidiOperations::TimeSigNumerator fractionNumeratorToUserValue(int n);
MidiOperations::TimeSigDenominator fractionDenominatorToUserValue(int z);

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
      void createKeys(Key);
      };

namespace MidiTuplet {

struct TupletInfo
      {
      int id;
      ReducedFraction onTime = {-1, 1};  // invalid
      ReducedFraction len = {-1, 1};
      int tupletNumber = -1;
                  // <chord onTime, chord iterator>
      std::map<ReducedFraction, std::multimap<ReducedFraction, MidiChord>::iterator> chords;
      ReducedFraction tupletSumError;
      ReducedFraction regularSumError;
      ReducedFraction sumLengthOfRests;
      int firstChordIndex = -1;
      std::map<ReducedFraction, int> staccatoChords;      // <onTime, note index>
      };

bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction> &interval1,
                      const std::pair<ReducedFraction, ReducedFraction> &interval2,
                      bool strictComparison = true);
bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction> &interval,
                      const std::vector<std::pair<ReducedFraction, ReducedFraction>> &intervals,
                      bool strictComparison = true);

} // namespace MidiTuplet

namespace MidiCharset {

QString convertToCharset(const std::string &text);
QString defaultCharset();
std::string fromUchar(const uchar *text);

} // namespace MidiCharset

namespace MidiTempo {

ReducedFraction time2Tick(double time, double ticksPerSec);
double findBasicTempo(const std::multimap<int, MTrack> &tracks);

} // namespace MidiTempo

namespace MidiBar {

ReducedFraction findBarStart(const ReducedFraction &time, const TimeSigMap *sigmap);

} // namespace MidiBar

namespace MidiDuration {

double durationCount(const QList<std::pair<ReducedFraction, TDuration> > &durations);

} // namespace MidiDuration
} // namespace Ms


#endif // IMPORTMIDI_INNER_H
