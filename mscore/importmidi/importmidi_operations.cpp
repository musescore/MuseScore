#include "importmidi_operations.h"


namespace Ms {

MidiOperations::Data midiImportOperations;

namespace MidiOperations {

static int readBoolFromXml(QXmlStreamReader &xml)
      {
      int value = -1;
      const QString name = xml.name().toString();
      xml.readNext();

      if (xml.tokenType() == QXmlStreamReader::Characters) {
            if (xml.text() == "true")
                  value = 1;
            else if (xml.text() == "false")
                  value = 0;
            else
                  qDebug() << "Load MIDI import operations from file: unknown" << name << "value";
            }
      return value;
      }

// Terms: <name attribute="value">text</name>

static void setOperationsFromFile(const QString &fileName, Opers &opers)
      {
      if (fileName.isEmpty())
            return;

      QFile file(fileName);
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("Load MIDI import operations from file: cannot open input file");
            return;
            }

      QXmlStreamReader xml(&file);

      while (!xml.atEnd() && !xml.hasError()) {
            const auto token = xml.readNext();

            if (token == QXmlStreamReader::StartDocument)
                  continue;
            if (token != QXmlStreamReader::StartElement)        // like <elem>
                  continue;
            if (xml.name() == "MidiOptions")
                  continue;

            if (xml.name() == "QuantValue") {
                  xml.readNext();
                  if (xml.tokenType() == QXmlStreamReader::Characters) {
                        bool ok = false;
                        const int index = xml.text().toString().toInt(&ok);
                        if (ok) {
                              switch (index) {
                                    case 0:
                                          opers.quantValue.setDefaultValue(QuantValue::Q_4, false);
                                          break;
                                    case 1:
                                          opers.quantValue.setDefaultValue(QuantValue::Q_8, false);
                                          break;
                                    case 2:
                                          opers.quantValue.setDefaultValue(QuantValue::Q_16, false);
                                          break;
                                    case 3:
                                          opers.quantValue.setDefaultValue(QuantValue::Q_32, false);
                                          break;
                                    case 4:
                                          opers.quantValue.setDefaultValue(QuantValue::Q_64, false);
                                          break;
                                    case 5:
                                          opers.quantValue.setDefaultValue(QuantValue::Q_128, false);
                                          break;
                                    default:
                                          qDebug("Load MIDI import operations from file: "
                                                 "unknown max quantization value");
                                          break;
                                    }
                              }
                        }
                  }
            else if (xml.name() == "VoiceCount") {
                  xml.readNext();
                  if (xml.tokenType() == QXmlStreamReader::Characters) {
                        bool ok = false;
                        const int index = xml.text().toString().toInt(&ok);
                        if (ok) {
                              switch (index) {
                                    case 0:
                                          opers.maxVoiceCount.setDefaultValue(VoiceCount::V_1, false);
                                          break;
                                    case 1:
                                          opers.maxVoiceCount.setDefaultValue(VoiceCount::V_2, false);
                                          break;
                                    case 2:
                                          opers.maxVoiceCount.setDefaultValue(VoiceCount::V_3, false);
                                          break;
                                    case 3:
                                          opers.maxVoiceCount.setDefaultValue(VoiceCount::V_4, false);
                                          break;
                                    default:
                                          qDebug("Load MIDI import operations from file: "
                                                 "unknown max voice count");
                                          break;
                                    }
                              }
                        }
                  }
            else if (xml.name() == "Duplets") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.search2plets.setDefaultValue(value, false);
                  }
            else if (xml.name() == "Triplets") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.search3plets.setDefaultValue(value, false);
                  }
            else if (xml.name() == "Quadruplets") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.search4plets.setDefaultValue(value, false);
                  }
            else if (xml.name() == "Quintuplets") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.search5plets.setDefaultValue(value, false);
                  }
            else if (xml.name() == "Septuplets") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.search7plets.setDefaultValue(value, false);
                  }
            else if (xml.name() == "Nonuplets") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.search9plets.setDefaultValue(value, false);
                  }
            else if (xml.name() == "HumanPerformance") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.isHumanPerformance.setDefaultValue(value, false);
                  }
            else if (xml.name() == "MeasureCount2xLess") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.measureCount2xLess.setDefaultValue(value, false);
                  }
            else if (xml.name() == "SplitStaff") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0) {
                        opers.doStaffSplit.setDefaultValue(value, false);
                        }
                  }
            else if (xml.name() == "ClefChanges") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.changeClef.setDefaultValue(value, false);
                  }
            else if (xml.name() == "SimplifyDurations") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.simplifyDurations.setDefaultValue(value, false);
                  }
            else if (xml.name() == "ShowStaccato") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.showStaccato.setDefaultValue(value, false);
                  }
            else if (xml.name() == "DottedNotes") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.useDots.setDefaultValue(value, false);
                  }
            else if (xml.name() == "RecognizePickupBar") {
                  const int value = readBoolFromXml(xml);
                  if (value >= 0)
                        opers.searchPickupMeasure.setDefaultValue(value, false);
                  }
            else if (xml.name() == "Swing") {
                  xml.readNext();
                  if (xml.tokenType() == QXmlStreamReader::Characters) {
                        bool ok = false;
                        const int index = xml.text().toString().toInt(&ok);
                        if (ok) {
                              switch (index) {
                                    case 0:
                                          opers.swing.setDefaultValue(Swing::NONE, false);
                                          break;
                                    case 1:
                                          opers.swing.setDefaultValue(Swing::SHUFFLE, false);
                                          break;
                                    case 2:
                                          opers.swing.setDefaultValue(Swing::SWING, false);
                                          break;
                                    default:
                                          qDebug("Load MIDI import operations from file: "
                                                 "unknown swing");
                                          break;
                                    }
                              }
                        }
                  }
            }
      if (xml.hasError()) {
            qDebug("Load MIDI import operations from file: cannot parse input file");
            return;
            }

      xml.clear();      // remove any device() or data from the reader; reset state to initial
      }

//-------------------------------------------------------------------------------------------

FileData* Data::data()
      {
      const auto it = _data.find(_currentMidiFile);
      if (it != _data.end())
            return &it->second;
      return nullptr;
      }

const FileData* Data::data() const
      {
      const auto it = _data.find(_currentMidiFile);
      if (it != _data.end())
            return &it->second;
      return nullptr;
      }

void Data::addNewMidiFile(const QString &fileName)
      {
      if (_data.find(fileName) != _data.end())
            return;
      FileData fileData;
      setOperationsFromFile(_midiOperationsFile, fileData.trackOpers);
      _data.insert({fileName, fileData});
      }

const MidiFile* Data::midiFile(const QString &fileName)
      {
      const auto it = _data.find(fileName);
      if (it != _data.end())
            return &it->second.midiFile;
      return nullptr;
      }

QStringList Data::allMidiFiles() const
      {
      QStringList list;
      for (const auto &d: _data)
            list.append(d.first);
      return list;
      }

void Data::setMidiFileData(const QString &fileName, const MidiFile &midiFile)
      {
      _data[fileName].midiFile = midiFile;
      }

void Data::excludeMidiFile(const QString &fileName)
      {
      _data.erase(fileName);
      }

bool Data::hasMidiFile(const QString &fileName)
      {
      return _data.find(fileName) != _data.end();
      }

int Data::currentTrack() const
      {

      Q_ASSERT_X(_currentTrack >= 0,
                 "Data::currentTrack", "Invalid current track index");

      return _currentTrack;
      }

void Data::setOperationsFile(const QString &fileName)
      {
      if (QFile::exists(fileName))
            _midiOperationsFile = fileName;
      }

} // namespace MidiOperations
} // namespace Ms

