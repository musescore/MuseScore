//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 - 2015 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importxmlfirstpass.h"

namespace Ms {

// TODO: move somewhere else

static const std::vector<QString> vocalInstrumentNames({"Voice",
                                                        "Soprano",
                                                        "Mezzo-Soprano",
                                                        "Alto",
                                                        "Tenor",
                                                        "Baritone",
                                                        "Bass",
                                                        "Women",
                                                        "Men"});

MusicXmlPart::MusicXmlPart(QString id, QString name)
      : id(id), name(name)
      {
      octaveShifts.resize(MAX_VOICE_DESC_STAVES);
      }


void MusicXmlPart::addMeasureNumberAndDuration(QString measureNumber, Fraction measureDuration)
      {
      measureNumbers.append(measureNumber);
      measureDurations.append(measureDuration);
      }

void MusicXmlPart::setMaxStaff(const int staff)
      {
      if (staff > _maxStaff)
            _maxStaff = staff;
      }

Fraction MusicXmlPart::measureDuration(int i) const
      {
      if (i >= 0 && i < measureDurations.size())
            return measureDurations.at(i);
      return Fraction(0, 0); // return invalid fraction
      }

QString MusicXmlPart::toString() const
      {
      auto res = QString("part id '%1' name '%2' print %3 abbr '%4' print %5 maxStaff %6\n")
            .arg(id, name).arg(_printName).arg(abbr).arg(_printAbbr, _maxStaff);

      for (VoiceList::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
            res += QString("voice %1 map staff data %2\n")
                  .arg(QString(i.key() + 1), i.value().toString());
            }

      for (int i = 0; i < measureNumbers.size(); ++i) {
            if (i > 0)
                  res += "\n";
            res += QString("measure %1 duration %2 (%3)")
                  .arg(measureNumbers.at(i), measureDurations.at(i).print()).arg(measureDurations.at(i).ticks());
            }

      return res;
      }

Interval MusicXmlPart::interval(const Fraction f) const
      {
      return _intervals.interval(f);
      }

int MusicXmlPart::octaveShift(const int staff, const Fraction f) const
      {
      if (staff < 0 || MAX_VOICE_DESC_STAVES <= staff)
            return 0;
      if (f < Fraction(0, 1))
            return 0;
      return octaveShifts[staff].octaveShift(f);
      }

void MusicXmlPart::addOctaveShift(const int staff, const int shift, const Fraction f)
      {
      if (staff < 0 || MAX_VOICE_DESC_STAVES <= staff)
            return;
      if (f < Fraction(0, 1))
            return;
      octaveShifts[staff].addOctaveShift(shift, f);
      }

void MusicXmlPart::calcOctaveShifts()
      {
      for (int i = 0; i < MAX_VOICE_DESC_STAVES; ++i) {
            octaveShifts[i].calcOctaveShiftShifts();
            }
      }

bool MusicXmlPart::isVocalStaff() const
      {
      return (std::find(vocalInstrumentNames.begin(), vocalInstrumentNames.end(), name) != vocalInstrumentNames.end()
              || _hasLyrics);
      }
      
//---------------------------------------------------------
//   interval
//---------------------------------------------------------

Interval MusicXmlIntervalList::interval(const Fraction f) const
      {
      if (empty())
            return {};
      auto i = upper_bound(f);
      if (i == begin())
            return {};
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const QString MusicXmlInstrList::instrument(const Fraction f) const
      {
      if (empty())
            return QString();
      auto i = upper_bound(f);
      if (i == begin())
            return QString();
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void MusicXmlInstrList::setInstrument(const QString instr, const Fraction f)
      {
      // TODO determine how to handle multiple instrument changes at the same time
      // current implementation keeps the first one
      if (!insert({ f, instr }).second)
            qDebug("instr '%s', tick %s (%d): element already exists",
                   qPrintable(instr), qPrintable(f.print()), f.ticks());
      //(*this)[f] = instr;
      }

int MusicXmlOctaveShiftList::octaveShift(const Fraction f) const
      {
      if (empty())
            return 0;
      auto i = upper_bound(f);
      if (i == begin())
            return 0;
      --i;
      return i->second;
      }

void MusicXmlOctaveShiftList::addOctaveShift(const int shift, const Fraction f)
      {
      if (Fraction(0, 1) > f)
            return;

      //qDebug("addOctaveShift(shift %d f %s)", shift, qPrintable(f.print()));
      auto i = find(f);
      if (i == end()) {
            //qDebug("addOctaveShift: not found, inserting");
            insert({ f, shift });
            }
      else {
            //qDebug("addOctaveShift: found %d, adding", (*this)[f]);
            (*this)[f] += shift;
            //qDebug("addOctaveShift: res %d", (*this)[f]);
            }
      }

void MusicXmlOctaveShiftList::calcOctaveShiftShifts()
      {
      /*
      for (auto i = cbegin(); i != cend(); ++i)
            qDebug(" [%s : %d]", qPrintable((*i).first.print()), (*i).second);
       */

      // to each MusicXmlOctaveShiftList entry, add the sum of all previous ones
      int currentShift = 0;
      for (auto i = begin(); i != end(); ++i) {
            currentShift += i->second;
            i->second = currentShift;
            }

      /*
      for (auto i = cbegin(); i != cend(); ++i)
            qDebug(" [%s : %d]", qPrintable((*i).first.print()), (*i).second);
       */

      }

//---------------------------------------------------------
//   staffNumberToIndex
//---------------------------------------------------------

/**
 This handles the mapping from MusicXML staff number to the index
 in a Part's Staff list.
 In most cases, this is a simple decrement from the 1-based staff number
 to the 0-based index.
 However, in some parts some MusicXML staves are discarded, and a mapping
 must be stored from MusicXML staff number to index. When this mapping is
 defined (i.e. size() != 0), it is used. See MusicXMLParserPass1::attributes()
 for more information.
 */

int MusicXmlPart::staffNumberToIndex(const int staffNumber) const
      {
      if (_staffNumberToIndex.size() == 0)
            return staffNumber - 1;
      else if (_staffNumberToIndex.contains(staffNumber))
            return  _staffNumberToIndex[staffNumber];
      else
            return -1;
      }

//---------------------------------------------------------
//   LyricNumberHandler
//   collect lyric numbering information and determine order
//
//   MusicXML lyrics may contain name and number attributes,
//   plus position information (typically default-y).
//   Name and number are simply tokens with no specified usage.
//   Default-y cannot easily be used to determine the lyrics
//   line, as it tends to differ per system depending on the
//   actual notes present.
//
//   Simply collecting all possible lyric number attributes
//   within a MusicXML part and assigning lyrics position
//   based on alphabetically sorting works well for all
//   common MusicXML files.
//---------------------------------------------------------

//---------------------------------------------------------
//   addNumber
//---------------------------------------------------------

void LyricNumberHandler::addNumber(const QString number)
      {
      if (_numberToNo.find(number) == _numberToNo.end())
            _numberToNo[number] = -1;       // unassiged
      }

//---------------------------------------------------------
//   toString
//---------------------------------------------------------

QString LyricNumberHandler::toString() const
      {
      QString res;
      for (const auto& p : _numberToNo) {
            if (!res.isEmpty())
                  res += " ";
            res += QString("%1:%2").arg(p.first, p.second);
            }
      return res;
      }

//---------------------------------------------------------
//   getLyricNo
//---------------------------------------------------------

int LyricNumberHandler::getLyricNo(const QString& number) const
      {
      const auto it = _numberToNo.find(number);
      return it == _numberToNo.end() ? 0 : it->second;
      }

//---------------------------------------------------------
//   determineLyricNos
//---------------------------------------------------------

void LyricNumberHandler::determineLyricNos()
      {
      int i = 0;
      for (auto& p : _numberToNo) {
            p.second = i;
            ++i;
            }
      }

}
