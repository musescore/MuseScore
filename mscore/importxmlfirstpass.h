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
#include "libmscore/score.h"
#include "musicxmlsupport.h"

namespace Ms {

typedef QMap<QString, VoiceDesc> VoiceList;

class MusicXmlInstrList : public std::map<Fraction, QString> {
public:
      MusicXmlInstrList() {}
      const QString instrument(const Fraction f) const;
      void setInstrument(const QString instr, const Fraction f);
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
private:
      QString id;
      QString name;
      QStringList measureNumbers;             // MusicXML measure number attribute
      QList<Fraction> measureDurations;       // duration in fraction for every measure
      };

class MxmlReaderFirstPass {
public:
      MxmlReaderFirstPass();
      bool determineMeasureLength(QVector<Fraction>& ml) const;
      void initVoiceMapperAndMapVoices(QDomElement e, int partNr);
      VoiceList getVoiceList(const int n) const;
      VoiceList getVoiceList(const QString id) const;
      MusicXmlInstrList getInstrList(const QString id) const;
      int nParts() const { return parts.size(); }
      void parsePart(QDomElement e, QString& partName, int partNr);
      void parsePartList(QDomElement e);
      void parseFile();
      Score::FileError setContent(QIODevice* d);
private:
      QDomDocument doc;
      QList<MusicXmlPart> parts;
      };


} // namespace Ms
#endif
