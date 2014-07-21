#include "importmidi_inner.h"
#include "importmidi_operations.h"
#include "preferences.h"
#include "libmscore/durationtype.h"
#include "midi/midifile.h"


namespace Ms {
namespace Meter {

ReducedFraction userTimeSigToFraction(
            MidiOperations::TimeSigNumerator timeSigNumerator,
            MidiOperations::TimeSigDenominator timeSigDenominator)
      {
      int numerator = 4;
      int denominator = 4;

      switch (timeSigNumerator) {
            case MidiOperations::TimeSigNumerator::_2:
                  numerator = 2;
                  break;
            case MidiOperations::TimeSigNumerator::_3:
                  numerator = 3;
                  break;
            case MidiOperations::TimeSigNumerator::_4:
                  numerator = 4;
                  break;
            case MidiOperations::TimeSigNumerator::_5:
                  numerator = 5;
                  break;
            case MidiOperations::TimeSigNumerator::_6:
                  numerator = 6;
                  break;
            case MidiOperations::TimeSigNumerator::_7:
                  numerator = 7;
                  break;
            case MidiOperations::TimeSigNumerator::_9:
                  numerator = 9;
                  break;
            case MidiOperations::TimeSigNumerator::_12:
                  numerator = 12;
                  break;
            case MidiOperations::TimeSigNumerator::_15:
                  numerator = 15;
                  break;
            case MidiOperations::TimeSigNumerator::_21:
                  numerator = 21;
                  break;
            default:
                  break;
            }

      switch (timeSigDenominator) {
            case MidiOperations::TimeSigDenominator::_2:
                  denominator = 2;
                  break;
            case MidiOperations::TimeSigDenominator::_4:
                  denominator = 4;
                  break;
            case MidiOperations::TimeSigDenominator::_8:
                  denominator = 8;
                  break;
            case MidiOperations::TimeSigDenominator::_16:
                  denominator = 16;
                  break;
            case MidiOperations::TimeSigDenominator::_32:
                  denominator = 32;
                  break;
            default:
                  break;
            }

      return ReducedFraction(numerator, denominator);
      }

MidiOperations::TimeSigNumerator fractionNumeratorToUserValue(int n)
      {
      MidiOperations::TimeSigNumerator numerator = MidiOperations::TimeSigNumerator::_4;

      if (n == 2)
            numerator = MidiOperations::TimeSigNumerator::_2;
      else if (n == 3)
            numerator = MidiOperations::TimeSigNumerator::_3;
      else if (n == 4)
            numerator = MidiOperations::TimeSigNumerator::_4;
      else if (n == 5)
            numerator = MidiOperations::TimeSigNumerator::_5;
      else if (n == 6)
            numerator = MidiOperations::TimeSigNumerator::_6;
      else if (n == 7)
            numerator = MidiOperations::TimeSigNumerator::_7;
      else if (n == 9)
            numerator = MidiOperations::TimeSigNumerator::_9;
      else if (n == 12)
            numerator = MidiOperations::TimeSigNumerator::_12;
      else if (n == 15)
            numerator = MidiOperations::TimeSigNumerator::_15;
      else if (n == 21)
            numerator = MidiOperations::TimeSigNumerator::_21;
      else
            Q_ASSERT_X(false, "Meter::fractionNumeratorToUserValue", "Unknown numerator");

      return numerator;
      }

MidiOperations::TimeSigDenominator fractionDenominatorToUserValue(int z)
      {
      MidiOperations::TimeSigDenominator denominator = MidiOperations::TimeSigDenominator::_4;

      if (z == 2)
            denominator = MidiOperations::TimeSigDenominator::_2;
      else if (z == 4)
            denominator = MidiOperations::TimeSigDenominator::_4;
      else if (z == 8)
            denominator = MidiOperations::TimeSigDenominator::_8;
      else if (z == 16)
            denominator = MidiOperations::TimeSigDenominator::_16;
      else if (z == 32)
            denominator = MidiOperations::TimeSigDenominator::_32;
      else
            Q_ASSERT_X(false, "Meter::fractionDenominatorToUserValue", "Unknown denominator");

      return denominator;
      }

} // namespace Meter

namespace MidiTuplet {

bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction> &interval1,
                      const std::pair<ReducedFraction, ReducedFraction> &interval2,
                      bool strictComparison)
      {
      if (strictComparison)
            return interval1.second > interval2.first && interval1.first < interval2.second;
      return interval1.second >= interval2.first && interval1.first <= interval2.second;
      }

bool haveIntersection(
            const std::pair<ReducedFraction, ReducedFraction> &interval,
            const std::vector<std::pair<ReducedFraction, ReducedFraction>> &intervals,
            bool strictComparison)
      {
      for (const auto &i: intervals) {
            if (haveIntersection(i, interval, strictComparison))
                  return true;
            }
      return false;
      }

}

namespace MidiCharset {

QString convertToCharset(const std::string &text)
      {
                  // charset for the current MIDI file
      QString charset = preferences.midiImportOperations.data()->charset;
      auto *codec = QTextCodec::codecForName(charset.toLatin1());
      if (codec)
            return codec->toUnicode(text.c_str());
      else
            return QString::fromStdString(text);
      }

QString defaultCharset()
      {
      return "UTF-8";
      }

std::string fromUchar(const uchar *text)
      {
      return reinterpret_cast<const char*>(text);
      }

} // namespace MidiCharset

namespace MidiTempo {

ReducedFraction time2Tick(double time, double ticksPerSec)
      {
      return ReducedFraction::fromTicks(int(ticksPerSec * time));
      }

// tempo in beats per second

double findBasicTempo(const std::multimap<int, MTrack> &tracks)
      {
      for (const auto &track: tracks) {
            for (const auto &ie : track.second.mtrack->events()) {
                  const MidiEvent &e = ie.second;
                  if (e.type() == ME_META && e.metaType() == META_TEMPO) {
                        const uchar* data = (uchar*)e.edata();
                        const unsigned tempo = data[2] + (data[1] << 8) + (data[0] << 16);
                        return 1000000.0 / double(tempo);
                        }
                  }
            }

      return 2;   // default beats per second = 120 beats per minute
      }

} // namespace MidiTempo

namespace MidiBar {

ReducedFraction findBarStart(const ReducedFraction &time, const TimeSigMap *sigmap)
      {
      int barIndex, beat, tick;
      sigmap->tickValues(time.ticks(), &barIndex, &beat, &tick);
      return ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
      }

} // namespace MidiBar
namespace MidiDuration {

double durationCount(const QList<std::pair<ReducedFraction, TDuration> > &durations)
      {
      double count = durations.size();
      for (const auto &d: durations) {
            if (d.second.dots())
                  count += 0.5;
            }
      return count;
      }

} // namespace MidiDuration
} // namespace Ms
