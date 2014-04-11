#include "importmidi_inner.h"
#include "preferences.h"


namespace Ms {
namespace MidiTuplet {

bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction> &interval1,
                      const std::pair<ReducedFraction, ReducedFraction> &interval2)
      {
      return interval1.second > interval2.first && interval1.first < interval2.second;
      }

bool haveIntersection(
            const std::pair<ReducedFraction, ReducedFraction> &interval,
            const std::vector<std::pair<ReducedFraction, ReducedFraction>> &intervals)
      {
      for (const auto &i: intervals) {
            if (haveIntersection(i, interval))
                  return true;
            }
      return false;
      }

}

namespace MidiCharset {

QString convertToCharset(const std::string &text)
      {
                  // charset for the current MIDI file
      QString charset = preferences.midiImportOperations.charset();
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
} // namespace Ms
