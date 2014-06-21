#ifndef IMPORTMIDI_DATA_H
#define IMPORTMIDI_DATA_H

#include "midi/midifile.h"
#include "importmidi_fraction.h"
#include "importmidi_operation.h"

#include <set>


namespace Ms {

namespace MidiCharset {
      QString defaultCharset();
}

struct TrackData;

class MidiData
      {
   public:
      void setTracksData(const QString &fileName, const QList<TrackData> &tracksData);
      void saveHHeaderState(const QString &fileName, const QByteArray &headerData);
      void saveVHeaderState(const QString &fileName, const QByteArray &headerData);
      void excludeFile(const QString &fileName);
      QList<TrackData> tracksData(const QString &fileName) const;
      QByteArray HHeaderData(const QString &fileName) const;
      QByteArray VHeaderData(const QString &fileName) const;
      int selectedRow(const QString &fileName) const;
      void setSelectedRow(const QString &fileName, int row);
      void setMidiFile(const QString &fileName, const MidiFile &midiFile);
      const MidiFile *midiFile(const QString &fileName) const;

                  // lyrics
      void addTrackLyrics(const QString &fileName,
                          const std::multimap<ReducedFraction, std::string> &trackLyrics);
      const QList<std::multimap<ReducedFraction, std::string> >*
            getLyrics(const QString &fileName);
      QString charset(const QString &fileName) const;
      void setCharset(const QString &fileName, const QString &charset);

                  // human performance: is MIDI unaligned
      bool isHumanPerformance(const QString &fileName) const;
      void setHumanPerformance(const QString &fileName, bool value);

      const std::set<ReducedFraction>* getHumanBeats(const QString &fileName) const;
      void setHumanBeats(const QString &fileName, const HumanBeatData &beatData);
                  // time sig can be zero (not specified) if, when opened,
                  // MIDI file was not detected as human-performed
      ReducedFraction timeSignature(const QString &fileName) const;
      void setTimeSignature(const QString &fileName, const ReducedFraction &value);

                  // quantization
      MidiOperation::QuantValue quantValue(const QString &fileName) const;
      void setQuantValue(const QString &fileName, MidiOperation::QuantValue value);

                  // left/right hand separation
      bool needToSplit(const QString &fileName, int trackIndex) const;
      void setNeedToSplit(const QString &fileName, int trackIndex, bool value);

   private:
      static MidiOperation::QuantValue defaultQuantValue();

      struct MidiDataStore
            {
            QByteArray HHeaderData;
            QByteArray VHeaderData;
            QList<TrackData> tracksData;
                        // tracks of <tick, lyric fragment> from karaoke files
                        // QList of lyric tracks - there can be multiple lyric tracks,
                        // lyric track count != MIDI chord track count in general
            QList<std::multimap<ReducedFraction, std::string>> lyricTracks;
                        // default values - when MIDI is opened
            int selectedRow = 0;
            MidiFile midiFile;
            QString charset = MidiCharset::defaultCharset();
            bool isHumanPerformance = false;
            std::map<int, bool> needLRhandSplit;      // <track index, value = false by default>
            MidiOperation::QuantValue quantValue = defaultQuantValue();
                        // human performance
            HumanBeatData humanBeatData;
            };

      QMap<QString, MidiDataStore> data;    // <file name, tracks data>
      };

} // namespace Ms


#endif // IMPORTMIDI_DATA_H
