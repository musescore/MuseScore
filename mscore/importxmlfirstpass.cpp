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

MusicXmlPart::MusicXmlPart(QString id, QString name)
      : id(id), name(name)
      {
      octaveShifts.resize(MAX_STAVES);
      }


void MusicXmlPart::addMeasureNumberAndDuration(QString measureNumber, Fraction measureDuration)
      {
      measureNumbers.append(measureNumber);
      measureDurations.append(measureDuration);
      }

Fraction MusicXmlPart::measureDuration(int i) const
      {
      if (i >= 0 && i < measureDurations.size())
            return measureDurations.at(i);
      return Fraction(0, 0); // return invalid fraction
      }

QString MusicXmlPart::toString() const
      {
      QString res;
      res = QString("part id '%1' name '%2'\n").arg(id).arg(name);

      for (VoiceList::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
            res += QString("voice %1 map staff data %2\n")
                  .arg(i.key() + 1)
                  .arg(i.value().toString());
            }

      for (int i = 0; i < measureNumbers.size(); ++i) {
            if (i > 0)
                  res += "\n";
            res += QString("measure %1 duration %2 (%3)")
                  .arg(measureNumbers.at(i))
                  .arg(measureDurations.at(i).print())
                  .arg(measureDurations.at(i).ticks());
            }

      return res;
      }

int MusicXmlPart::octaveShift(const int staff, const Fraction f) const
      {
      if (staff < 0 || MAX_STAVES <= staff)
            return 0;
      if (f < Fraction(0, 1))
            return 0;
      return octaveShifts[staff].octaveShift(f);
      }

void MusicXmlPart::addOctaveShift(const int staff, const int shift, const Fraction f)
      {
      if (staff < 0 || MAX_STAVES <= staff)
            return;
      if (f < Fraction(0, 1))
            return;
      octaveShifts[staff].addOctaveShift(shift, f);
      }

void MusicXmlPart::calcOctaveShifts()
      {
      for (int i = 0; i < MAX_STAVES; ++i) {
            octaveShifts[i].calcOctaveShiftShifts();
            }
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const QString MusicXmlInstrList::instrument(const Fraction f) const
      {
      if (empty())
            return "";
      auto i = upper_bound(f);
      if (i == begin())
            return "";
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
            qDebug("MusicXmlInstrList::setInstrument instr '%s', tick %s (%d): element already exists",
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
      Q_ASSERT(Fraction(0, 1) <= f);

      qDebug("addOctaveShift(shift %d f %s)", shift, qPrintable(f.print()));
      auto i = find(f);
      if (i == end()) {
            qDebug("addOctaveShift: not found, inserting");
            insert({ f, shift });
            }
      else {
            qDebug("addOctaveShift: found %d, adding", (*this)[f]);
            (*this)[f] += shift;
            qDebug("addOctaveShift: res %d", (*this)[f]);
            }
      }

void MusicXmlOctaveShiftList::calcOctaveShiftShifts()
      {
      for (auto i = cbegin(); i != cend(); ++i)
            qDebug(" [%s : %d]", qPrintable((*i).first.print()), (*i).second);

      // to each MusicXmlOctaveShiftList entry, add the sum of all previous ones
      int currentShift = 0;
      for (auto i = begin(); i != end(); ++i) {
            currentShift += i->second;
            i->second = currentShift;
            }

      for (auto i = cbegin(); i != cend(); ++i)
            qDebug(" [%s : %d]", qPrintable((*i).first.print()), (*i).second);

      }

}