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
//   Drumset
//---------------------------------------------------------

Drumset::Drumset()
      {
      _drum.resize(DRUM_INSTRUMENTS);
      for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
            _drum[i].notehead = NoteHead::Group::HEAD_INVALID;
            _drum[i].line     = 0;
            _drum[i].shortcut = 0;
            _drum[i].voice    = 0;
            _drum[i].stemDirection = MScore::Direction::UP;
            _drum[i].pitch = i;
            }
      }

std::vector<DrumInstrument> Drumset::drumsByIndex() const
      {
      auto d = _drum;
      std::sort(d.begin(), d.end());
      return d;
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(Xml& xml) const
      {
      auto d = drumsByIndex();
      for (int n = 0; n < DRUM_INSTRUMENTS; ++n) {
            const DrumInstrument& di = d[n];
            if (!isValid(di.pitch))
                  continue;
            xml.stag(QString("Drum pitch=\"%1\"").arg(di.pitch));
            const NoteHead::Group nh = di.notehead;
            //write custom as Normal notehead group + noteheads tag to keep compatibility with 2.X versions
            int saveValue = (nh == NoteHead::Group::HEAD_CUSTOM) ? int(NoteHead::Group::HEAD_NORMAL) : int(nh);
            xml.tag("head", saveValue);
            if (nh == NoteHead::Group::HEAD_CUSTOM) {
                  xml.stag("noteheads");
                  for (int j = 0; j < int(NoteHead::Type::HEAD_TYPES); j++) {
                        xml.tag(NoteHead::type2name(NoteHead::Type(j)), Sym::id2name(di.noteheads[int(NoteHead::Type(j))]));
                        }
                  xml.etag();
                  }
            xml.tag("line", di.line);
            xml.tag("voice", di.voice);
            xml.tag("name", di.name);
            xml.tag("stem", int(di.stemDirection));
            if (di.shortcut) {
                  switch (di.shortcut) {
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                        case 'G':
                        case 'A':
                        case 'B':
                              {
                              char a[2];
                              a[0] = di.shortcut;
                              a[1] = 0;
                              xml.tag("shortcut", a);
                              }
                              break;
                        default:
                              qDebug("illegal drum shortcut");
                              break;
                        }
                  }
            auto vs = di.variants;
            if (!vs.isEmpty()) {
                  xml.stag("variants");
                  for (auto v : vs) {
                        xml.stag(QString("variant pitch=\"%1\"").arg(v.pitch));
                        if (!v.articulationName.isEmpty())
                              xml.tag("articulation", v.articulationName);
                        if (v.tremolo != TremoloType::INVALID_TREMOLO)
                              xml.tag("tremolo", Tremolo::type2name(v.tremolo));
                        xml.etag();
                        }
                  xml.etag();
                  }
            xml.etag();
            }
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Drumset::load(XmlReader& e, int index)
      {
      int pitch = e.intAttribute("pitch", -1);
      if (pitch < 0 || pitch > 127) {
            qDebug("load drumset: invalid pitch %d", pitch);
            return;
            }
      _drum[pitch].index = index;
      _drum[pitch].pitch = pitch;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "head")
                  _drum[pitch].notehead = NoteHead::Group(e.readInt());
            else if (tag == "noteheads") {
                  _drum[pitch].notehead = NoteHead::Group::HEAD_CUSTOM;
                  while (e.readNextStartElement()) {
                        const QStringRef& nhTag(e.name());
                        int noteType = int(NoteHead::name2type(nhTag.toString()));
                        if (noteType > int(NoteHead::Type::HEAD_TYPES) - 1 || noteType < 0)
                              return;

                        _drum[pitch].noteheads[noteType] = Sym::name2id(e.readElementText());
                        }
                  }
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
            else if (tag == "variants") {
                  while(e.readNextStartElement()) {
                        const QStringRef& tagv(e.name());
                        if (tagv == "variant") {
                              DrumInstrumentVariant div;
                              div.pitch = e.attribute("pitch").toInt();
                              while (e.readNextStartElement()) {
                                    const QStringRef& taga(e.name());
                                    if (taga == "articulation") {
                                          div.articulationName = e.readElementText();
                                          }
                                    else if (taga == "tremolo") {
                                          div.tremolo = Tremolo::name2Type(e.readElementText());
                                          }
                                    }
                              _drum[pitch].addVariant(div);
                              }
                        }
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
            _drum[i].variants.clear();
            }
      }

//---------------------------------------------------------
//   nextPitch
//---------------------------------------------------------

int Drumset::nextPitch(int ii) const
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

int Drumset::prevPitch(int ii) const
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
//   findVariant
/// find a variant for the given pitch with matching chord articulation and tremolo
//---------------------------------------------------------

DrumInstrumentVariant Drumset::findVariant(int p, const QList<Articulation*> articulations, Tremolo* tremolo) const
      {
      DrumInstrumentVariant div;
      auto vs = variants(p);
      for (auto v : vs) {
            bool matchTremolo = (!tremolo && v.tremolo == TremoloType::INVALID_TREMOLO) || (tremolo && v.tremolo == tremolo->tremoloType());
            bool matchArticulation = v.articulationName.isEmpty() && articulations.isEmpty();
            for (auto a : articulations) {
                  matchArticulation = a->subtypeName() == v.articulationName;
                  if (!matchArticulation)
                        break;
                  }
            if (matchArticulation && matchTremolo) {
                  div = v;
                  break;
                  }
            }
      return div;
      }

//---------------------------------------------------------
//   initDrumset
//    initialize standard midi drumset
//---------------------------------------------------------

void initDrumset()
      {
      smDrumset = new Drumset;
      smDrumset->addDrumInstrument(35, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Bass Drum"), NoteHead::Group::HEAD_NORMAL,   7, MScore::Direction::DOWN, 1));
      smDrumset->addDrumInstrument(36, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Bass Drum 1"),        NoteHead::Group::HEAD_NORMAL,   7, MScore::Direction::DOWN, 1, Qt::Key_B));
      smDrumset->addDrumInstrument(37, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Side Stick"),         NoteHead::Group::HEAD_CROSS,    3, MScore::Direction::UP));
      smDrumset->addDrumInstrument(38, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Snare"),     NoteHead::Group::HEAD_NORMAL,   3, MScore::Direction::UP, 0, Qt::Key_A));
      smDrumset->addDrumInstrument(40, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Electric Snare"),     NoteHead::Group::HEAD_NORMAL,   3, MScore::Direction::UP));
      smDrumset->addDrumInstrument(41, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Floor Tom"),      NoteHead::Group::HEAD_NORMAL,   5, MScore::Direction::UP));
      smDrumset->addDrumInstrument(42, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Closed Hi-Hat"),      NoteHead::Group::HEAD_CROSS,   -1, MScore::Direction::UP, 0, Qt::Key_G));
      smDrumset->addDrumInstrument(43, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Floor Tom"),     NoteHead::Group::HEAD_NORMAL,   5, MScore::Direction::DOWN, 1));
      smDrumset->addDrumInstrument(44, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Pedal Hi-Hat"),       NoteHead::Group::HEAD_CROSS,    9, MScore::Direction::DOWN, 1, Qt::Key_F));
      smDrumset->addDrumInstrument(45, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Tom"),            NoteHead::Group::HEAD_NORMAL,   2, MScore::Direction::UP));
      smDrumset->addDrumInstrument(46, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Hi-Hat"),        NoteHead::Group::HEAD_CROSS,    1, MScore::Direction::UP));
      smDrumset->addDrumInstrument(47, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low-Mid Tom"),        NoteHead::Group::HEAD_NORMAL,   1, MScore::Direction::UP));
      smDrumset->addDrumInstrument(48, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Mid Tom"),         NoteHead::Group::HEAD_NORMAL,   0, MScore::Direction::UP));
      smDrumset->addDrumInstrument(49, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash Cymbal 1"),     NoteHead::Group::HEAD_CROSS,   -2, MScore::Direction::UP, 0, Qt::Key_C));
      smDrumset->addDrumInstrument(50, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Tom"),           NoteHead::Group::HEAD_NORMAL,   0, MScore::Direction::UP, 0, Qt::Key_E));
      smDrumset->addDrumInstrument(51, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Cymbal 1"),      NoteHead::Group::HEAD_CROSS,    0, MScore::Direction::UP, 0, Qt::Key_D));
      smDrumset->addDrumInstrument(52, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Chinese Cymbal"),     NoteHead::Group::HEAD_CROSS,   -3, MScore::Direction::UP));
      smDrumset->addDrumInstrument(53, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Bell"),          NoteHead::Group::HEAD_DIAMOND,  0, MScore::Direction::UP));
      smDrumset->addDrumInstrument(54, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tambourine"),         NoteHead::Group::HEAD_DIAMOND,  2, MScore::Direction::UP));
      smDrumset->addDrumInstrument(55, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Splash Cymbal"),      NoteHead::Group::HEAD_CROSS,   -3, MScore::Direction::UP));
      smDrumset->addDrumInstrument(56, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Cowbell"),            NoteHead::Group::HEAD_TRIANGLE, 1, MScore::Direction::UP));
      smDrumset->addDrumInstrument(57, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash Cymbal 2"),     NoteHead::Group::HEAD_CROSS,   -3, MScore::Direction::UP));
      smDrumset->addDrumInstrument(59, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Cymbal 2"),      NoteHead::Group::HEAD_CROSS,    2, MScore::Direction::UP));
      smDrumset->addDrumInstrument(63, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Hi Conga"),    NoteHead::Group::HEAD_CROSS,    4, MScore::Direction::UP));
      smDrumset->addDrumInstrument(64, DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Conga"),          NoteHead::Group::HEAD_CROSS,    6, MScore::Direction::UP));
      }

}

