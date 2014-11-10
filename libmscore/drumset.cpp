//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

namespace Ms {

Drumset* smDrumset;           // standard midi drumset
Drumset* gpDrumset;           // guitar pro drumset

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(Xml& xml)
      {
      for (int i = 0; i < 128; ++i) {
            if (!isValid(i))
                  continue;
            xml.stag(QString("Drum pitch=\"%1\"").arg(i));
            xml.tag("head", int(noteHead(i)));
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
                              qDebug("illegal drum shortcut");
                              break;
                        }
                  }
            xml.etag();
            }
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Drumset::load(XmlReader& e)
      {
      int pitch = e.intAttribute("pitch", -1);
      if (pitch < 0 || pitch > 127) {
            qDebug("load drumset: invalid pitch %d", pitch);
            return;
            }
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "head")
                  _drum[pitch].notehead = NoteHead::Group(e.readInt());
            else if (tag == "line")
                  _drum[pitch].line = e.readInt();
            else if (tag == "voice")
                  _drum[pitch].voice = e.readInt();
            else if (tag == "name")
                  _drum[pitch].name = e.readElementText();
            else if (tag == "stem")
                  _drum[pitch].stemDirection = MScore::Direction(e.readInt());
            else if (tag == "shortcut") {
                  bool isNum;
                  QString val(e.readElementText());
                  int i = val.toInt(&isNum);
                  _drum[pitch].shortcut = isNum ? i : toupper(val[0].toLatin1());
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Drumset::clear()
      {
      for (int i = 0; i < 128; ++i) {
            _drum[i].name = "";
            _drum[i].notehead = NoteHead::Group::HEAD_INVALID;
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
            smDrumset->drum(i).notehead = NoteHead::Group::HEAD_INVALID;
            smDrumset->drum(i).line     = 0;
            smDrumset->drum(i).shortcut = 0;
            smDrumset->drum(i).voice    = 0;
            smDrumset->drum(i).stemDirection = MScore::Direction::UP;
            }
      smDrumset->drum(35) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Bass Drum"), NoteHead::Group::HEAD_NORMAL,   7, MScore::Direction::DOWN, 1);
      smDrumset->drum(36) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Bass Drum 1"),        NoteHead::Group::HEAD_NORMAL,   7, MScore::Direction::DOWN, 1, Qt::Key_B);
      smDrumset->drum(37) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Side Stick"),         NoteHead::Group::HEAD_CROSS,    3, MScore::Direction::UP);
      smDrumset->drum(38) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Snare"),     NoteHead::Group::HEAD_NORMAL,   3, MScore::Direction::UP, 0, Qt::Key_A);
      smDrumset->drum(40) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Electric Snare"),     NoteHead::Group::HEAD_NORMAL,   3, MScore::Direction::UP);
      smDrumset->drum(41) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Floor Tom"),      NoteHead::Group::HEAD_NORMAL,   5, MScore::Direction::UP);
      smDrumset->drum(42) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Closed Hi-Hat"),      NoteHead::Group::HEAD_CROSS,   -1, MScore::Direction::UP, 0, Qt::Key_G);
      smDrumset->drum(43) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Floor Tom"),      NoteHead::Group::HEAD_NORMAL,   5, MScore::Direction::DOWN, 1);
      smDrumset->drum(44) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Pedal Hi-Hat"),       NoteHead::Group::HEAD_CROSS,    9, MScore::Direction::DOWN, 1);
      smDrumset->drum(45) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Tom"),            NoteHead::Group::HEAD_NORMAL,   2, MScore::Direction::UP, 0, Qt::Key_F);
      smDrumset->drum(46) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Hi-Hat"),        NoteHead::Group::HEAD_CROSS,    1, MScore::Direction::UP);
      smDrumset->drum(47) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low-Mid Tom"),        NoteHead::Group::HEAD_NORMAL,   1, MScore::Direction::UP);
      smDrumset->drum(48) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Mid Tom"),         NoteHead::Group::HEAD_NORMAL,   0, MScore::Direction::UP);
      smDrumset->drum(49) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash Cymbal 1"),     NoteHead::Group::HEAD_CROSS,   -2, MScore::Direction::UP, 0, Qt::Key_C);
      smDrumset->drum(50) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Tom"),           NoteHead::Group::HEAD_NORMAL,   0, MScore::Direction::UP, 0, Qt::Key_E);
      smDrumset->drum(51) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Cymbal 1"),      NoteHead::Group::HEAD_CROSS,    0, MScore::Direction::UP, 0, Qt::Key_D);
      smDrumset->drum(52) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Chinese Cymbal"),     NoteHead::Group::HEAD_CROSS,   -3, MScore::Direction::UP);
      smDrumset->drum(53) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Bell"),          NoteHead::Group::HEAD_DIAMOND,  0, MScore::Direction::UP);
      smDrumset->drum(54) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tambourine"),         NoteHead::Group::HEAD_DIAMOND,  2, MScore::Direction::UP);
      smDrumset->drum(55) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Splash Cymbal"),      NoteHead::Group::HEAD_CROSS,   -3, MScore::Direction::UP);
      smDrumset->drum(56) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Cowbell"),            NoteHead::Group::HEAD_TRIANGLE, 1, MScore::Direction::UP);
      smDrumset->drum(57) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash Cymbal 2"),     NoteHead::Group::HEAD_CROSS,   -3, MScore::Direction::UP);
      smDrumset->drum(59) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Cymbal 2"),      NoteHead::Group::HEAD_CROSS,    2, MScore::Direction::UP);
      smDrumset->drum(63) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Hi Conga"),    NoteHead::Group::HEAD_CROSS,    4, MScore::Direction::UP);
      smDrumset->drum(64) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Conga"),          NoteHead::Group::HEAD_CROSS,    6, MScore::Direction::UP);
      }

}

