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

#ifndef __IMPORTXMLFIRSTPASS_H__
#define __IMPORTXMLFIRSTPASS_H__

#include "libmscore/fraction.h"
#include "libmscore/interval.h"
#include "musicxmlsupport.h"

namespace Ms {

typedef QMap<QString, VoiceDesc> VoiceList;
//using Intervals = std::map<Fraction, Interval>;

class MusicXmlIntervalList : public std::map<Fraction, Interval> {
public:
      MusicXmlIntervalList() {}
      Interval interval(const Fraction f) const;
      };

class MusicXmlInstrList : public std::map<Fraction, QString> {
public:
      MusicXmlInstrList() {}
      const QString instrument(const Fraction f) const;
      void setInstrument(const QString instr, const Fraction f);
      };

class MusicXmlOctaveShiftList : public std::map<Fraction, int> {
public:
      MusicXmlOctaveShiftList() {}
      int octaveShift(const Fraction f) const;
      void addOctaveShift(const int shift, const Fraction f);
      void calcOctaveShiftShifts();
      };

class LyricNumberHandler {
public:
      LyricNumberHandler() {}
      void addNumber(const QString number);
      QString toString() const;
      int getLyricNo(const QString& number) const;
      void determineLyricNos();
private:
      std::map<QString, int> _numberToNo;
      };

class MusicXmlPart {
public:
      MusicXmlPart(QString id = "", QString name = "");
      void addMeasureNumberAndDuration(QString measureNumber, Fraction measureDuration);
      QString getId() const { return id; }
      QString toString() const;
      VoiceList voicelist;         // the voice map information TODO: make private
      Fraction measureDuration(int i) const;
      int nMeasures() const { return measureDurations.size(); }
      MusicXmlInstrList _instrList; // TODO: make private
      MusicXmlIntervalList _intervals;                     ///< Transpositions
      Interval interval(const Fraction f) const;
      int octaveShift(const int staff, const Fraction f) const;
      void addOctaveShift(const int staff, const int shift, const Fraction f);
      void calcOctaveShifts();
      void setName(QString nm) { name = nm; }
      QString getName() const { return name; }
      void setPrintName(bool b) { printName = b; }
      bool getPrintName() const { return printName; }
      void setAbbr(QString ab) { abbr = ab; }
      QString getAbbr() const { return abbr; }
      void setPrintAbbr(bool b) { printAbbr = b; }
      bool getPrintAbbr() const { return printAbbr; }
      LyricNumberHandler& lyricNumberHandler() { return _lyricNumberHandler; }
      const LyricNumberHandler& lyricNumberHandler() const { return _lyricNumberHandler; }
      void setMaxStaff(const int staff);
      int maxStaff() const { return _maxStaff; }
private:
      QString id;
      QString name;
      bool printName = true;
      QString abbr;
      bool printAbbr = true;
      QStringList measureNumbers;             // MusicXML measure number attribute
      QList<Fraction> measureDurations;       // duration in fraction for every measure
      QVector<MusicXmlOctaveShiftList> octaveShifts; // octave shift list for every staff
      LyricNumberHandler _lyricNumberHandler;
      int _maxStaff = 0;                      // maximum staff value found (1 based), 0 = none
      };

} // namespace Ms
#endif
