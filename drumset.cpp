//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: drumset.cpp 5384 2012-02-27 12:21:49Z wschweer $
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "drumset.h"
#include "xml.h"
#include "note.h"

Drumset* smDrumset;           // standard midi drumset

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(Xml& xml)
      {
      for (int i = 0; i < 128; ++i) {
            if (!isValid(i))
                  continue;
            xml.stag(QString("Drum pitch=\"%1\"").arg(i));
            xml.tag("head", noteHead(i));
            xml.tag("line", line(i));
            xml.tag("voice", voice(i));
            xml.tag("name", name(i));
            xml.tag("stem", int(stemDirection(i)));
            if (shortcut(i)) {
                  switch (shortcut(i)) {
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                        case 'G':
                        case 'A':
                        case 'B':
                              {
                              char a[2];
                              a[0] = shortcut(i);
                              a[1] = 0;
                              xml.tag("shortcut", a);
                              }
                              break;
                        default:
                              qDebug("illegal drum shortcut\n");
                              break;
                        }
                  }
            xml.etag();
            }
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Drumset::load(const QDomElement& de)
      {
      int pitch = de.attribute("pitch", "-1").toInt();
      if (pitch < 0 || pitch > 127) {
            qDebug("load drumset: invalid pitch %d\n", pitch);
            return;
            }
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            bool isNum;
            int i = val.toInt(&isNum);

            if (tag == "head")
                  _drum[pitch].notehead = NoteHeadGroup(i);
            else if (tag == "line")
                  _drum[pitch].line = i;
            else if (tag == "voice")
                  _drum[pitch].voice = i;
            else if (tag == "name")
                  _drum[pitch].name = val;
            else if (tag == "stem")
                  _drum[pitch].stemDirection = Direction(i);
            else if (tag == "shortcut")
                  _drum[pitch].shortcut = isNum ? i : toupper(val[0].toAscii());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Drumset::clear()
      {
      for (int i = 0; i < 128; ++i) {
            _drum[i].name = "";
            _drum[i].notehead = HEAD_INVALID;
            _drum[i].shortcut = 0;
            }
      }

//---------------------------------------------------------
//   nextPitch
//---------------------------------------------------------

int Drumset::nextPitch(int ii)
      {
      for (int i = ii + 1; i < 127; ++i) {
            if (isValid(i))
                  return i;
            }
      for (int i = 0; i <= ii; ++i) {
            if (isValid(i))
                  return i;
            }
      return 0;
      }

//---------------------------------------------------------
//   prevPitch
//---------------------------------------------------------

int Drumset::prevPitch(int ii)
      {
      for (int i = ii - 1; i >= 0; --i) {
            if (isValid(i))
                  return i;
            }
      for (int i = 127; i >= ii; --i) {
            if (isValid(i))
                  return i;
            }
      return 0;
      }

//---------------------------------------------------------
//   initDrumset
//    initialize standard midi drumset
//---------------------------------------------------------

void initDrumset()
      {
      smDrumset = new Drumset;
      for (int i = 0; i < 128; ++i) {
            smDrumset->drum(i).notehead = HEAD_INVALID;
            smDrumset->drum(i).line     = 0;
            smDrumset->drum(i).shortcut = 0;
            smDrumset->drum(i).voice    = 0;
            smDrumset->drum(i).stemDirection = UP;
            }
      smDrumset->drum(35) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Bass Drum"), HEAD_NORMAL,   7, DOWN, 1);
      smDrumset->drum(36) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Bass Drum"),          HEAD_NORMAL,   7, DOWN, 1, Qt::Key_C);
      smDrumset->drum(37) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Side Stick"),         HEAD_CROSS,    3, UP);
      smDrumset->drum(38) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Snare (Acoustic)"),   HEAD_NORMAL,   3, UP);
      smDrumset->drum(40) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Snare (Electric)"),   HEAD_NORMAL,   3, UP);
      smDrumset->drum(41) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 5"),              HEAD_NORMAL,   5, UP);
      smDrumset->drum(42) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Hat Closed"),      HEAD_CROSS,   -1, UP);
      smDrumset->drum(43) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 4"),              HEAD_NORMAL,   5, DOWN, 1);
      smDrumset->drum(44) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Hat Pedal"),       HEAD_CROSS,    9, DOWN, 1);
      smDrumset->drum(45) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 3"),              HEAD_NORMAL,   2, UP);
      smDrumset->drum(46) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Hat Open"),        HEAD_CROSS,    1, UP);
      smDrumset->drum(47) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 2"),              HEAD_NORMAL,   1, UP);
      smDrumset->drum(48) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 1"),              HEAD_NORMAL,   0, UP);
      smDrumset->drum(49) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash 1"),            HEAD_CROSS,   -2, UP);
      smDrumset->drum(50) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom"),                HEAD_NORMAL,   0, UP);
      smDrumset->drum(51) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride"),               HEAD_CROSS,    0, UP, 0, Qt::Key_D);
      smDrumset->drum(52) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "China"),              HEAD_CROSS,   -3, UP);
      smDrumset->drum(53) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_DIAMOND,  0, UP);
      smDrumset->drum(54) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tambourine"),         HEAD_DIAMOND,  2, UP);
      smDrumset->drum(55) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_CROSS,   -3, UP);
      smDrumset->drum(56) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_TRIANGLE, 1, UP);
      smDrumset->drum(57) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_CROSS,   -3, UP);
      smDrumset->drum(59) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_CROSS,    2, UP);
      smDrumset->drum(63) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "open high conga"),    HEAD_CROSS,    4, UP);
      smDrumset->drum(64) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "low conga"),          HEAD_CROSS,    6, UP);
      }

