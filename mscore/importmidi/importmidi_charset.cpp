#include "importmidi_charset.h"
#include "importmidi_lyrics.h"
#include "mscore/preferences.h"
#include "thirdparty/universalchardet/nsUniversalDetector.h"


namespace Ms {
namespace MidiCharset {

class Detector : public nsUniversalDetector
      {
   public:
      Detector(PRUint32 aLanguageFilter) : nsUniversalDetector(aLanguageFilter) {}

      int analyseText(const std::string &text)
            {
            if (HandleData(text.c_str(), text.size()) == NS_ERROR_OUT_OF_MEMORY)   // error
                  return -1;
            if (mDone)                    // detected early
                  return 0;
            return 1;                     // need more data
            }

      QString getCharsetName()
            {
            DataEnd();
            if (!mDone) {      // not enough info
                  if (mInputState == eEscAscii)
                        return "ibm850";
                  else if (mInputState == ePureAscii)
                        return "ASCII";
                  // we not sure here - is the answer right, but we can try
                  }
            return QString::fromLocal8Bit(mDetectedCharset);
            }
   protected:
      void Report(const char* aCharset)
            {
            mDone = PR_TRUE;
            mDetectedCharset = aCharset;
            }
      };

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

QString detectCharset(const std::string &text)
      {
      Detector det(NS_FILTER_ALL);
      const int result = det.analyseText(text);
      QString name;
      if (result > 0)
            name = det.getCharsetName();
      if (name != "")
            return name;
      return defaultCharset();
      }

void tryToDetectCharset()
      {
      std::string text;
      auto &data = *preferences.midiImportOperations.data();

      for (int i = 0; i != data.trackCount; ++i)
            text += data.trackOpers.staffName.value(i) + " ";
      text += data.textForCharsetDetection;

      QList<std::string> lyrics = MidiLyrics::makeLyricsListForUI(
                                              std::numeric_limits<size_t>::max());
      for (const auto &lyric: lyrics)
            text += lyric + " ";

      QString charsetGuess = detectCharset(text);
      data.charset = charsetGuess;
      }

} // namespace MidiCharset
} // namespace Ms
