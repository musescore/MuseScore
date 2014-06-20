#include "importmidi_data.h"
#include "importmidi_operations.h"
#include "importmidi_meter.h"
#include "importmidi_inner.h"
#include "importmidi_beat.h"


namespace Ms {

void MidiData::setTracksData(const QString &fileName, const QList<TrackData> &tracksData)
      {
      data[fileName].tracksData = tracksData;
      }

void MidiData::saveHHeaderState(const QString &fileName, const QByteArray &headerData)
      {
      data[fileName].HHeaderData = headerData;
      }

void MidiData::saveVHeaderState(const QString &fileName, const QByteArray &headerData)
      {
      data[fileName].VHeaderData = headerData;
      }

void MidiData::excludeFile(const QString &fileName)
      {
      data.remove(fileName);
      }

QList<TrackData> MidiData::tracksData(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return QList<TrackData>();
      return it.value().tracksData;
      }

QByteArray MidiData::HHeaderData(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return QByteArray();
      return it.value().HHeaderData;
      }

QByteArray MidiData::VHeaderData(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return QByteArray();
      return it.value().VHeaderData;
      }

int MidiData::selectedRow(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return 0;
      return it.value().selectedRow;
      }

void MidiData::setSelectedRow(const QString &fileName, int row)
      {
      data[fileName].selectedRow = row;
      }

void MidiData::setMidiFile(const QString &fileName, const MidiFile &midiFile)
      {
      data[fileName].midiFile = midiFile;
      }

const MidiFile* MidiData::midiFile(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return nullptr;
      return &(it.value().midiFile);
      }

void MidiData::addTrackLyrics(const QString &fileName,
                              const std::multimap<ReducedFraction, std::string> &trackLyrics)
      {
      data[fileName].lyricTracks.push_back(trackLyrics);
      }

const QList<std::multimap<ReducedFraction, std::string> >*
MidiData::getLyrics(const QString &fileName)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return nullptr;
      return &it.value().lyricTracks;
      }

QString MidiData::charset(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return "";
      return it.value().charset;
      }

void MidiData::setCharset(const QString &fileName, const QString &charset)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return;
      it.value().charset = charset;
      }

bool MidiData::isHumanPerformance(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return false;
      return it.value().isHumanPerformance;
      }

void MidiData::setHumanPerformance(const QString &fileName, bool value)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return;
      it.value().isHumanPerformance = value;
      }

MidiOperation::QuantValue MidiData::quantValue(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return Quantization().value;
      return it.value().quantValue;
      }

void MidiData::setQuantValue(const QString &fileName, MidiOperation::QuantValue value)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return;
      it.value().quantValue = value;
      }

bool MidiData::needToSplit(const QString &fileName, int trackIndex) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return false;
      const auto fit = it.value().needLRhandSplit.find(trackIndex);
      if (fit == it.value().needLRhandSplit.end())
            return false;
      return fit->second;
      }

void MidiData::setNeedToSplit(const QString &fileName, int trackIndex, bool value)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return;
      it.value().needLRhandSplit[trackIndex] = value;
      }

MidiOperation::QuantValue MidiData::defaultQuantValue()
      {
      return Quantization().value;
      }

const std::set<ReducedFraction>*
MidiData::getHumanBeats(const QString &fileName) const
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return nullptr;
      return &it.value().humanBeatData.beatSet;
      }

void MidiData::setHumanBeats(const QString &fileName, const HumanBeatData &beatData)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return;
      it.value().humanBeatData = beatData;
      }

ReducedFraction MidiData::timeSignature(const QString &fileName) const
      {
      const auto it = data.find(fileName);
                  // if data was not found - return zero value;
                  // it means that time sig should be taken from MIDI file meta event
                  // or, if no such event, it will be set to default 4/4 later;
                  // so, don't return default time sig here
      if (it == data.end())
            return ReducedFraction(0, 1);
      return it.value().humanBeatData.timeSig;
      }

void MidiData::setTimeSignature(const QString &fileName, const ReducedFraction &timeSig)
      {
      const auto it = data.find(fileName);
      if (it == data.end())
            return;

      HumanBeatData &beatData = it.value().humanBeatData;
      if (beatData.timeSig == timeSig)
            return;

      beatData.timeSig = timeSig;

      auto &beatSet = beatData.beatSet;
      if (beatSet.empty())
            return;

      for (int i = 0; i != beatData.addedFirstBeats; ++i) {

            Q_ASSERT_X(!beatSet.empty(), "MidiData::setTimeSignature",
                       "Empty beat set after deletion first beats");

            beatSet.erase(beatSet.begin());
            }
      for (int i = 0; i != beatData.addedLastBeats; ++i) {

            Q_ASSERT_X(!beatSet.empty(), "MidiData::setTimeSignature",
                       "Empty beat set after deletion last beats");

            beatSet.erase(std::prev(beatSet.end()));
            }

      const int beatsInBar = MidiBeat::beatsInBar(timeSig);
      MidiBeat::addFirstBeats(beatSet, beatData.firstChordTick,
                              beatsInBar, beatData.addedFirstBeats);
      MidiBeat::addLastBeats(beatSet, beatData.lastChordTick,
                             beatsInBar, beatData.addedLastBeats);
      }

} // namespace Ms
