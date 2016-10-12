//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importgtp.h"
#include "globals.h"
#include "libmscore/score.h"
#include "libmscore/measurebase.h"
#include "libmscore/text.h"
#include "libmscore/box.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/stringdata.h"
#include "libmscore/clef.h"
#include "libmscore/lyrics.h"
#include "libmscore/tempotext.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/barline.h"
#include "libmscore/excerpt.h"
#include "libmscore/stafftype.h"
#include "libmscore/bracket.h"
#include "libmscore/articulation.h"
#include "libmscore/keysig.h"
#include "libmscore/harmony.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/segment.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/dynamic.h"
#include "libmscore/arpeggio.h"
#include "libmscore/volta.h"
#include "libmscore/fret.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/glissando.h"
#include "libmscore/chordline.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/hairpin.h"
#include "libmscore/ottava.h"
#include "libmscore/notedot.h"
#include "libmscore/stafftext.h"
#include "libmscore/sym.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   errmsg
//---------------------------------------------------------

const char* GuitarPro::errmsg[] = {
      "no error",
      "unknown file format",
      "unexpected end of file",
      "bad number of strings",
      };

//---------------------------------------------------------
//   GpBar
//---------------------------------------------------------

GpBar::GpBar()
      {
      barLine = BarLineType::NORMAL;
      keysig  = GP_INVALID_KEYSIG;
      timesig = Fraction(4,4);
      repeatFlags = Repeat::NONE;
      repeats = 2;
      }

//---------------------------------------------------------
//   GuitarPro
//---------------------------------------------------------

GuitarPro::GuitarPro(Score* s, int v)
      {
      score   = s;
      version = v;
      _codec = QTextCodec::codecForName(preferences.importCharsetGP.toLatin1());
      voltaSequence = 1;
      }

GuitarPro::~GuitarPro()
      {
      }

//---------------------------------------------------------
//   skip
//---------------------------------------------------------

void GuitarPro::skip(qint64 len)
      {
      char c;
      while (len--)
            read(&c, 1);
      }

//---------------------------------------------------------
//    read
//---------------------------------------------------------

void GuitarPro::read(void* p, qint64 len)
      {
      if (len == 0)
            return;
      qint64 rv = f->read((char*)p, len);
#ifdef QT_NO_DEBUG
      Q_UNUSED(rv); // avoid warning about unused variable in RELEASE mode
#endif
      Q_ASSERT(rv == len);
      curPos += len;
      }

//---------------------------------------------------------
//   readChar
//---------------------------------------------------------

int GuitarPro::readChar()
      {
      signed char c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readUChar
//---------------------------------------------------------

int GuitarPro::readUChar()
      {
      uchar c;
      read(&c, 1);
      return c;
      }

//---------------------------------------------------------
//   readPascalString
//---------------------------------------------------------

QString GuitarPro::readPascalString(int n)
      {
      uchar l = readUChar();
      char s[l + 1];
      read(s, l);
      s[l] = 0;
      skip(n - l);
      if(_codec)
            return _codec->toUnicode(s);
      else
            return QString(s);
      }

//---------------------------------------------------------
//   readWordPascalString
//---------------------------------------------------------

QString GuitarPro::readWordPascalString()
      {
      int l = readInt();
      char c[l+1];
      read(c, l);
      c[l] = 0;
      if(_codec)
            return _codec->toUnicode(c);
      else
            return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readBytePascalString
//---------------------------------------------------------

QString GuitarPro::readBytePascalString()
      {
      int l = readUChar();
      char c[l+1];
      read(c, l);
      c[l] = 0;
      if(_codec)
            return  _codec->toUnicode(c);
      else
            return QString::fromLocal8Bit(c);
      }

//---------------------------------------------------------
//   readDelphiString
//---------------------------------------------------------

QString GuitarPro::readDelphiString()
      {
      int maxl = readInt();
      uchar l = readUChar();
      if (maxl != l + 1)
            qFatal("readDelphiString: first word doesn't match second byte");
      char c[l + 1];
      read(c, l);
      c[l] = 0;
      if(_codec)
            return  _codec->toUnicode(c);
      else
            return QString::fromLatin1(c);
      }

//---------------------------------------------------------
//   readInt
//---------------------------------------------------------

int GuitarPro::readInt()
      {
      uchar x;
      read(&x, 1);
      int r = x;
      read(&x, 1);
      r += x << 8;
      read(&x, 1);
      r += x << 16;
      read(&x, 1);
      r += x << 24;
      return r;
      }

//---------------------------------------------------------
//   initGuitarProDrumset
//---------------------------------------------------------

void GuitarPro::initGuitarProDrumset()
      {
      gpDrumset = new Drumset;
      for (int i = 0; i < 128; ++i) {
            gpDrumset->drum(i).notehead = NoteHead::Group::HEAD_INVALID;
            gpDrumset->drum(i).line     = 0;
            gpDrumset->drum(i).shortcut = 0;
            gpDrumset->drum(i).voice    = 0;
            gpDrumset->drum(i).stemDirection = Direction::UP;
            }
      // new drumset determined via guitar pro (third argument specifies position on staff, 10 = C3, 9 = D3, 8 = E3,...)
      gpDrumset->drum(27) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Q"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(28) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Slap"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(29) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Scratch Push"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(30) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Scratch Pull"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(31) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Sticks"), NoteHead::Group::HEAD_CROSS, 3, Direction::UP);
      gpDrumset->drum(32) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Square Click"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(33) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Metronome Click"), NoteHead::Group::HEAD_CROSS, 3, Direction::UP);
      gpDrumset->drum(34) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Metronome Bell"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(35) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Bass Drum"), NoteHead::Group::HEAD_NORMAL, 7, Direction::UP);
      gpDrumset->drum(36) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Bass Drum 1"), NoteHead::Group::HEAD_NORMAL, 7, Direction::UP);
      gpDrumset->drum(37) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Side Stick"), NoteHead::Group::HEAD_CROSS, 3, Direction::UP);
      gpDrumset->drum(38) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Snare"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(39) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hand Clap"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(40) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Electric Snare"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(41) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Floor Tom"), NoteHead::Group::HEAD_NORMAL, 6, Direction::UP);
      gpDrumset->drum(42) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Closed Hi-Hat"), NoteHead::Group::HEAD_CROSS, -1, Direction::UP);
      gpDrumset->drum(43) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Floor Tom"), NoteHead::Group::HEAD_NORMAL, 5, Direction::UP);
      gpDrumset->drum(44) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Pedal Hi-Hat"), NoteHead::Group::HEAD_CROSS, 7, Direction::UP);
      gpDrumset->drum(45) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Tom"), NoteHead::Group::HEAD_NORMAL, 4, Direction::UP);
      gpDrumset->drum(46) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Hi-Hat"), NoteHead::Group::HEAD_CROSS, -1, Direction::UP);
      gpDrumset->drum(47) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low-Mid Tom"), NoteHead::Group::HEAD_NORMAL, 2, Direction::UP);
      gpDrumset->drum(48) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Mid Tom"), NoteHead::Group::HEAD_NORMAL, 1, Direction::UP);
      gpDrumset->drum(49) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash Cymbal 1"), NoteHead::Group::HEAD_DIAMOND, -1, Direction::UP);
      gpDrumset->drum(50) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Tom"), NoteHead::Group::HEAD_NORMAL, 0, Direction::UP);
      gpDrumset->drum(51) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Cymbal 1"), NoteHead::Group::HEAD_DIAMOND, -1, Direction::UP);
      gpDrumset->drum(52) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Chinese Cymbal"), NoteHead::Group::HEAD_CROSS, -1, Direction::UP);
      gpDrumset->drum(53) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Bell"), NoteHead::Group::HEAD_DIAMOND, -1, Direction::UP);
      gpDrumset->drum(54) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tambourine"), NoteHead::Group::HEAD_CROSS, 2, Direction::UP);
      gpDrumset->drum(55) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Splash Cymbal"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(56) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Cowbell"), NoteHead::Group::HEAD_NORMAL, 1, Direction::UP);
      gpDrumset->drum(57) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash Cymbal 2"), NoteHead::Group::HEAD_DIAMOND, -1, Direction::UP);
      gpDrumset->drum(58) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Vibraslap"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(59) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride Cymbal 2"), NoteHead::Group::HEAD_DIAMOND, -1, Direction::UP);
      gpDrumset->drum(60) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi Bongo"), NoteHead::Group::HEAD_NORMAL, 8, Direction::UP);
      gpDrumset->drum(61) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Bongo"), NoteHead::Group::HEAD_NORMAL, 9, Direction::UP);
      gpDrumset->drum(62) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Mute Hi Conga"), NoteHead::Group::HEAD_CROSS, 5, Direction::UP);
      gpDrumset->drum(63) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Hi Conga"), NoteHead::Group::HEAD_CROSS, 4, Direction::UP);
      gpDrumset->drum(64) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Conga"), NoteHead::Group::HEAD_CROSS, 6, Direction::UP);
      gpDrumset->drum(65) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Timbale"), NoteHead::Group::HEAD_CROSS, 8, Direction::UP);
      gpDrumset->drum(66) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Timbale"), NoteHead::Group::HEAD_CROSS, 9, Direction::UP);
      gpDrumset->drum(67) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "High Agogo"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(68) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Agogo"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(69) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Cabasa"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(70) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Maracas"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(71) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Short Whistle"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(72) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Long Whistle"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(73) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Short Güiro"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(74) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Long Güiro"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(75) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Claves"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(76) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi Wood Block"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(77) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Low Wood Block"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(78) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Mute Cuica"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(79) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Cuica"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(80) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Mute Triangle"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(81) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Triangle"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(82) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Shaker"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(83) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Sleigh Bell"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(84) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Bell Tree"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(85) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Castanets"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(86) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Mute Surdo"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      gpDrumset->drum(87) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Open Surdo"), NoteHead::Group::HEAD_NORMAL, 3, Direction::UP);
      }



//---------------------------------------------------------
//   addPalmMate
//---------------------------------------------------------

void GuitarPro::addPalmMute(Note* note)
      {
      QString palmMute = "P.M.";
      TextStyle textStyle;
      textStyle.setAlign(AlignmentFlags::CENTER);
      addTextToNote(palmMute, textStyle, note);
      }

//---------------------------------------------------------
//   addLetRing
//---------------------------------------------------------

void GuitarPro::addLetRing(Note* note)
      {
      QString letRing = "let ring";
      TextStyle textStyle;
      textStyle.setItalic(true);
      textStyle.setAlign(AlignmentFlags::CENTER);
      addTextToNote(letRing, textStyle, note);
      }

//---------------------------------------------------------
//   addTap
//---------------------------------------------------------

void GuitarPro::addTap(Note* note)
      {
      QString tap = "T";
      TextStyle textStyle;
      textStyle.setAlign(AlignmentFlags::CENTER);
      addTextToNote(tap, textStyle, note);
      }

//---------------------------------------------------------
//   addSlap
//---------------------------------------------------------

void GuitarPro::addSlap(Note* note)
      {
      QString slap = "S";
      TextStyle textStyle;
      textStyle.setAlign(AlignmentFlags::CENTER);
      addTextToNote(slap, textStyle, note);
      }

//---------------------------------------------------------
//   addPop
//---------------------------------------------------------

void GuitarPro::addPop(Note* note)
      {
      QString pop = "P";
      TextStyle textStyle;
      textStyle.setAlign(AlignmentFlags::CENTER);
      addTextToNote(pop, textStyle, note);
      }

//---------------------------------------------------------
//   addTextToNote
//---------------------------------------------------------

void GuitarPro::addTextToNote(QString string, TextStyle textStyle, Note* note)
      {
      Text* text = new Text(score);
      text->setTextStyle(textStyle);
      text->setPlainText(string);
      text->setAbove(true);
      note->add(text);
      }

//---------------------------------------------------------
//   setTuplet
//---------------------------------------------------------

void GuitarPro::setTuplet(Tuplet* tuplet, int tuple)
      {
      switch(tuple) {
            case 3:
                  tuplet->setRatio(Fraction(3,2));
                  break;
            case 5:
                  tuplet->setRatio(Fraction(5,4));
                  break;
            case 6:
                  tuplet->setRatio(Fraction(6,4));
                  break;
            case 7:
                  tuplet->setRatio(Fraction(7,4));
                  break;
            case 9:
                  tuplet->setRatio(Fraction(9,8));
                  break;
            case 10:
                  tuplet->setRatio(Fraction(10,8));
                  break;
            case 11:
                  tuplet->setRatio(Fraction(11,8));
                  break;
            case 12:
                  tuplet->setRatio(Fraction(12,8));
                  break;
            case 13:
                  tuplet->setRatio(Fraction(13,8));
                  break;
            default:
                  qFatal("unsupported tuplet %d\n", tuple);
            }
      }

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

void GuitarPro::addDynamic(Note* note, int d)
      {
      if (!note->chord()){
            qDebug() << "addDynamics: No chord associated with this note";
            return;
            }
      Dynamic* dyn = new Dynamic(score);
      // guitar pro only allows their users to go from ppp to fff
      QString map_dyn[] = {"f","ppp","pp","p","mp","mf","f","ff","fff"};
      dyn->setDynamicType(map_dyn[d]);
      dyn->setTrack(note->track());
      if (note->chord()->isGrace()) {
            Chord* parent = static_cast<Chord*>(note->chord()->parent());
            parent->segment()->add(dyn);
            }
      else
            note->chord()->segment()->add(dyn);
      }

void GuitarPro::readVolta(GPVolta* gpVolta, Measure* m)
      {
      /* Volta information is at most eight bits
       * signifying which numbers should appear in the
       * volta. A single bit 1 represents we should show
       * 1, 100 represents 3, 10000 represents 6, 10101
       * represents 1,3,5 etc. */
      if (gpVolta->voltaInfo.length() != 0) {
            // we have volta information - set up a volta
            Ms::Volta* volta = new Ms::Volta(score);
            volta->endings().clear();
            QString voltaTextString = "";
            // initialise count to 1 as the first bit processed with represesnt first time volta
            int count = 0;
            int binaryNumber = 0;
             // iterate through the volta information and determine the decimal numbers for voltas
            auto iter = gpVolta->voltaInfo.begin();
            while (iter != gpVolta->voltaInfo.end()) {
                  switch (gpVolta->voltaType) {
                        case GP_VOLTA_FLAGS:
                              count++;
                              if (*iter == 1) {   // we want this number to be displayed in the volta
                                    if (voltaTextString == "")
                                          voltaTextString += QString::number(count);
                                    else
                                          voltaTextString += "," + QString::number(count);
                                    // add the decimal number to the endings field of voltas as well as the text
                                    volta->endings().append(count);
                                    }
                              ++iter;
                              break;
                        case GP_VOLTA_BINARY:
                              // find the binary number in decimal
                              if (*iter == 1) {
                                    binaryNumber += pow(2,count);
                                    }
                              ++iter;
                              if (iter == gpVolta->voltaInfo.end()) {
                                    // display all numbers in the volta from voltaSequence to the decimal
                                    while (voltaSequence <= binaryNumber) {
                                          if (voltaTextString == "")
                                                voltaTextString = QString::number(voltaSequence);
                                          else
                                                voltaTextString += "," + QString::number(voltaSequence);
                                          volta->endings().append(voltaSequence);
                                          voltaSequence++;
                                          }
                                    }
                              count++;
                              break;
                        }
                  }
            volta->setText(Xml::xmlString(voltaTextString));
            volta->setTick(m->tick());
            volta->setTick2(m->tick() + m->ticks());
            score->addElement(volta);
            }
      }

//---------------------------------------------------------
//   readBend
//    bend graph
//---------------------------------------------------------

void GuitarPro::readBend(Note* note)
      {
      readUChar();                        // icon
      readInt();                          // shown aplitude
      int numPoints = readInt();          // the number of points in the bend

      // there are no notes in the bend, exit the function
      if (numPoints == 0)
            return;
      Bend* bend = new Bend(note->score());
      for (int i = 0; i < numPoints; ++i) {
            int bendTime  = readInt();
            int bendPitch = readInt();
            int bendVibrato = readUChar();
            bend->points().append(PitchValue(bendTime, bendPitch, bendVibrato));
            }
      bend->setTrack(note->track());
      note->add(bend);
      }

//---------------------------------------------------------
//   readLyrics
//---------------------------------------------------------

void GuitarPro::readLyrics()
      {
      gpLyrics.lyricTrack = readInt();        // lyric track
      gpLyrics.fromBeat = readInt();
      gpLyrics.beatCounter = 0;

      QString lyrics = readWordPascalString();
      lyrics.replace(QRegExp("\n"), " ");
      lyrics.replace(QRegExp("\r"), " ");
      gpLyrics.lyrics = lyrics.split(" ", QString::KeepEmptyParts);

      for (int i = 0; i < 4; ++i) {
            readInt();
            readWordPascalString();
            }
      }

//---------------------------------------------------------
//   createSlide
//---------------------------------------------------------

void GuitarPro::createSlide(int slide, ChordRest* cr, int staffIdx)
      {
      // shift / legato slide
      if (slide == SHIFT_SLIDE || slide == LEGATO_SLIDE) {
            Glissando* s = new Glissando(score);
/*            s->setXmlText("");
            s->setGlissandoType(Glissando::Type::STRAIGHT);
            cr->add(s); */
            s->setAnchor(Spanner::Anchor::NOTE);
            Segment* prevSeg = cr->segment()->prev1(Segment::Type::ChordRest);
            Element* prevElem = prevSeg->element(staffIdx);
            if (prevElem) {
                  if (prevElem->type() == Element::Type::CHORD) {
                        Chord* prevChord = static_cast<Chord*>(prevElem);
                        /** TODO we should not just take the top note here
                        * but the /correct/ note need to check whether GP
                        * supports multi-note gliss. I think it can in modern
                        * versions */
                        s->setStartElement(prevChord->upNote());
                        s->setTick(prevSeg->tick());
                        s->setTrack(staffIdx);
                        s->setParent(prevChord->upNote());
                        s->setText("");
                        s->setGlissandoType(Glissando::Type::STRAIGHT);
                        if (slide == LEGATO_SLIDE)
                              createSlur(true, staffIdx, prevChord);
                        }
                  }

            Chord* chord = (Chord*) cr;
            /* TODO again here, we should not just set the up note but the
             * /correct/ note need to check whether GP supports
             * multi-note gliss. I think it can in modern versions */
            s->setEndElement(chord->upNote());
            s->setTick2(chord->segment()->tick());
            s->setTrack2(staffIdx);
            score->addElement(s);
            if (slide == LEGATO_SLIDE)
                  createSlur(false, staffIdx, cr);
            }
      // slide out downwards (fall)
      else if (slide == SLIDE_OUT_DOWN) {
            ChordLine* cl = new ChordLine(score);
            cl->setChordLineType(ChordLineType::FALL);
            cl->setStraight(true);
            cr->add(cl);
            }
      // slide out upwards (doit)
      else if (slide == SLIDE_OUT_UP) {
            ChordLine* cl = new ChordLine(score);
            cl->setChordLineType(ChordLineType::DOIT);
            cl->setStraight(true);
            cr->add(cl);
            }
      // slide in from below (plop)
      else if (slide == SLIDE_IN_BELOW) {
            ChordLine* cl = new ChordLine(score);
            cl->setChordLineType(ChordLineType::PLOP);
            cl->setStraight(true);
            cr->add(cl);
            }
      // slide in from above (scoop)
      else if (slide == SLIDE_IN_ABOVE) {
            ChordLine* cl = new ChordLine(score);
            cl->setChordLineType(ChordLineType::SCOOP);
            cl->setStraight(true);
            cr->add(cl);
            }
      }

//---------------------------------------------------------
//   readChannels
//---------------------------------------------------------

void GuitarPro::readChannels()
      {
      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            channelDefaults[i].patch   = readInt();
            channelDefaults[i].volume  = readUChar() * 8 - 1;
            channelDefaults[i].pan     = readUChar() * 8 - 1;
            channelDefaults[i].chorus  = readUChar() * 8 - 1;
            channelDefaults[i].reverb  = readUChar() * 8 - 1;
            channelDefaults[i].phase   = readUChar() * 8 - 1;
            channelDefaults[i].tremolo = readUChar() * 8 - 1;

            // defaults of 255, or any value above 127, are set to 0 (Musescore range is 0-127)
            if (channelDefaults[i].patch > 127)   { channelDefaults[i].patch = 0; }
            if (channelDefaults[i].volume > 127)  { channelDefaults[i].volume = 0; }
            if (channelDefaults[i].pan > 127)     { channelDefaults[i].pan = 0; }
            if (channelDefaults[i].chorus > 127)  { channelDefaults[i].chorus = 0; }
            if (channelDefaults[i].reverb > 127)  { channelDefaults[i].reverb = 0; }
            if (channelDefaults[i].phase > 127)   { channelDefaults[i].phase = 0; }
            if (channelDefaults[i].tremolo > 127) { channelDefaults[i].tremolo = 0; }

            // skip over blank information included for backwards compatibility with 3.0
            skip(2);
            }
      }

//---------------------------------------------------------
//   len2fraction
//---------------------------------------------------------

Fraction GuitarPro::len2fraction(int len)
      {
      Fraction l;
      switch(len) {
            case -2: l.set(1, 1);    break;
            case -1: l.set(1, 2);    break;
            case  0: l.set(1, 4);    break;
            case  1: l.set(1, 8);    break;
            case  2: l.set(1, 16);   break;
            case  3: l.set(1, 32);   break;
            case  4: l.set(1, 64);   break;
            case  5: l.set(1, 128);  break;
            // set to len - in some cases we get whacky numbers for this (40, 28...)
            default:
                  l.set(1,len);
            }
      return l;
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

bool GuitarPro::readMixChange(Measure* measure)
      {
      /*char patch   =*/ readChar();
      signed char volume  = readChar();
      signed char pan     = readChar();
      signed char chorus  = readChar();
      signed char reverb  = readChar();
      signed char phase   = readChar();
      signed char tremolo = readChar();
      int tempo    = readInt();

      if (volume >= 0)
            readChar();
      if (pan >= 0)
            readChar();
      if (chorus >= 0)
             readChar();
      if (reverb >= 0)
            readChar();
      if (phase >= 0)
            readChar();
      if (tremolo >= 0)
            readChar();
      if (tempo >= 0) {
            if (tempo != previousTempo) {
                  previousTempo = tempo;
                  setTempo(tempo, measure);
                  }
            readChar();
            }
      return true;
      }

//---------------------------------------------------------
//   createMeasures
//---------------------------------------------------------

void GuitarPro::createMeasures()
      {
      int tick = 0;
      Fraction ts;
//      for (int i = 0; i < measures; ++i) {
      for (int i = 0; i < bars.size(); ++i) {   // ?? (ws)
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        Staff* staff = score->staff(staffIdx);
                        StaffType* staffType = staff->staffType();
                        if (staffType->genTimesig()) {
                              TimeSig* t = new TimeSig(score);
                              t->setTrack(staffIdx * VOICES);
                              t->setSig(nts);
                              Segment* s = m->getSegment(Segment::Type::TimeSig, tick);
                              s->add(t);
                              }
                        }
                  }
            if (i == 0 || (bars[i].keysig != GP_INVALID_KEYSIG)) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        int keysig = bars[i].keysig != GP_INVALID_KEYSIG ? bars[i].keysig : key;
                        if (tick == 0 || score->staff(staffIdx)->key(tick) != Key(keysig)) {
                              KeySig* t = new KeySig(score);
                              t->setKey(Key(keysig));
                              t->setTrack(staffIdx * VOICES);
                              Segment* s = m->getSegment(Segment::Type::KeySig, tick);
                              s->add(t);
                              }
                        }
                  }
            readVolta(&bars[i].volta, m);
            m->setRepeatEnd(bars[i].repeatFlags & Repeat::END);
            m->setRepeatStart(bars[i].repeatFlags & Repeat::START);
            m->setRepeatCount(bars[i].repeats);       // supported in gp5

            // reset the volta sequence if we have an opening repeat
            if (bars[i].repeatFlags == Repeat::START)
                  voltaSequence = 1;
            // otherwise, if we see an end repeat symbol, only reset if the bar after it does not contain a volta
            else if (bars[i].repeatFlags == Repeat::END && i < bars.length() - 1) {
                  if (bars[i+1].volta.voltaInfo.length() == 0) {
                    voltaSequence = 1;      // reset  the volta count
                        }
            }

            score->measures()->add(m);
            tick += nts.ticks();
            ts = nts;
            }
      }

//---------------------------------------------------------
//   applyBeatEffects
//---------------------------------------------------------

void GuitarPro::applyBeatEffects(Chord* chord, int beatEffect)
      {
      /* tap/slap/pop implemented as text until SMuFL has
       * specifications and we can add them to fonts. Note that
       * tap/slap/pop are just added to the top note in the chord,
       * technically these can be applied to individual notes on the
       * UI, but Guitar Pro has no way to express that on the
       * score. To get the same result, we should just add the marking
       * to above the top note.
       */
      if (beatEffect == 1)
            addTap(chord->upNote());
      else if (beatEffect == 2)
            addSlap(chord->upNote());
      else if (beatEffect == 3)
            addPop(chord->upNote());
      else if (beatEffect == 4) {
            Articulation* a = new Articulation(chord->score());
            a->setSymId(SymId::guitarFadeIn);
            a->setAnchor(ArticulationAnchor::TOP_STAFF);
            chord->add(a);
            }
      else if (beatEffect == 5) {
            Articulation* a = new Articulation(chord->score());
            a->setSymId(SymId::stringsUpBow);
            chord->add(a);
            }
      else if (beatEffect == 6) {
            Articulation* art = new Articulation(chord->score());
            art->setSymId(SymId::stringsDownBow);
            chord->add(art);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro1::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      title  = readDelphiString();
      artist = readDelphiString();
      readDelphiString();

      int tempo = readInt();
      /*uchar num =*/ readUChar();      // Shuffle rhythm feel

      // int octave = 0;
      key    = 0;
      if (version > 102)
            key = readInt();    // key

      staves  = version > 102 ? 8 : 1;

      //int tnumerator   = 4;
      //int tdenominator = 4;

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s   = new Staff(score);
            s->setPart(part);
            part->insertStaff(s, 0);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;

      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            int strings  = version > 101 ? readInt() : 6;
            for (int j = 0; j < strings; ++j)
                  tuning[j] = readInt();
            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];

            int frets = 32;   // TODO
            StringData stringData(frets, strings, tuning2);
            Part* part = score->staff(i)->part();
            Instrument* instr = part->instrument();
            instr->setStringData(stringData);
            }

      measures = readInt();

      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        TimeSig* t = new TimeSig(score);
                        t->setTrack(staffIdx * VOICES);
                        t->setSig(nts);
                        Segment* s = m->getSegment(Segment::Type::TimeSig, tick);
                        s->add(t);
                        }
                  }

            score->measures()->add(m);
            tick += nts.ticks();
            ts = nts;
            }

      previousTempo = tempo;
      Measure* measure = score->firstMeasure();
      bool mixChange = false;
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setPlainText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  Fraction measureLen = 0;
                  int track = staffIdx * VOICES;
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
//                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & BEAT_DOTTED;
                        if (beatBits & BEAT_PAUSE)
                              /*pause =*/ readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & BEAT_TUPLET)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                        if (beatBits & BEAT_CHORD) {
                              int numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, track, numStrings, name, false);
                                    }
                              else  {
                                    skip(25);
                                    name = readPascalString(34);
                                    readChord(segment, track, numStrings, name, true);
                                    skip(36);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & BEAT_LYRICS) {
                              lyrics = new Lyrics(score);
                              lyrics->setPlainText(readDelphiString());
                              }
                        if (beatBits & BEAT_EFFECTS)
                              readBeatEffects(track, segment);

                        if (beatBits & BEAT_MIX_CHANGE) {
                              readMixChange(measure);
                              mixChange = true;
                              }

                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);
                        ChordRest* cr;
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(track);
                        if (lyrics)
                              cr->add(lyrics);

                        TDuration d(l);
                        d.setDots(dotted ? 1 : 0);

                        if (dotted)
                              l = l + (l/2);

                        if (tuple) {
                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    setTuplet(tuplet, tuple);
                                    tuplet->setParent(measure);
                                    }
                              tuplet->setTrack(track);
                              tuplet->setBaseLen(l);
                              tuplet->setDuration(l * tuplet->ratio().denominator());
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);  //TODOxxx
                              }

                        cr->setDuration(l);
                        cr->setDurationType(d);
                        segment->add(cr);
                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instrument()->stringData()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);
                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        restsForEmptyBeats(segment, measure, cr, l, track, tick);
                        tick += cr->actualTicks();
                        measureLen += cr->actualFraction();
                        }
                  if (measureLen < measure->len()) {
                        score->setRest(tick, track, measure->len() - measureLen, false, nullptr, false);
                        }
                  }
            if (bar == 1 && !mixChange)
                  setTempo(tempo, score->firstMeasure());
            }
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void GuitarPro::setTempo(int tempo, Measure* measure)
      {
      TempoText* tt = new TempoText(score);
      tt->setTempo(double(tempo)/60.0);
      tt->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(tempo));

      tt->setTrack(0);
      Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
      segment->add(tt);
      score->setTempo(measure->tick(), tt->tempo());
      previousTempo = tempo;
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro::readChord(Segment* seg, int track, int numStrings, QString name, bool gpHeader)
      {
      FretDiagram* fret = new FretDiagram(score);
      fret->setTrack(track);
      fret->setStrings(numStrings);

      int firstFret = readInt();
      fret->setOffset(firstFret-1);
      if (firstFret != 0 || gpHeader) {
            for (int i = 0; i < (gpHeader ? 7 : 6); ++i) {
                  int currentFret =  readInt();
                  // read the frets and add them to the fretboard
                  // substract 1 extra from numStrings as we count from 0
                  if (i > numStrings - 1) {}
                  else if (currentFret > 0) {
                        fret->setDot(numStrings - 1 - i, currentFret-firstFret+1);
                        }
                  else if (currentFret == 0) {
                        fret->setDot(numStrings - 1 - i, 0);
                        fret->setMarker(numStrings - 1 - i, '0');
                       }
                  else if (currentFret == -1) {
                        fret->setDot(numStrings - 1 - i, 0);
                        fret->setMarker(numStrings - 1 - i, 'X');
                        }
                  }
            }
      seg->add(fret);
      if (name.isEmpty())
            return;
      Harmony* harmony = new Harmony(seg->score());
      harmony->setHarmony(name);
      harmony->setTrack(track);
      seg->add(harmony);
      }

//---------------------------------------------------------
//   restsForEmptyBeats
//---------------------------------------------------------

void GuitarPro::restsForEmptyBeats(Segment* seg, Measure* measure, ChordRest* cr, Fraction& l, int track, int tick)
      {
      /* this can happen as Guitar Pro versions 5 and below allows
       * users to create empty segments. Here, we create rests and
       * make them invisible so users get the same visual if they are
       * at a valid tick of the score. */
      if (seg->empty()) {
            if (tick < measure->first()->tick() + measure->ticks()) {
                  cr = new Rest(score);
                  cr->setTrack(track);
                  TDuration d(l);
                  cr->setDurationType(d);
                  cr->setVisible(false);
                  seg->add(cr);
                  }
            else
                  measure->remove(seg);
            }
      }

//---------------------------------------------------------
//   createSlur
//---------------------------------------------------------

void GuitarPro::createSlur(bool hasSlur, int staffIdx, ChordRest* cr)
      {
      if (hasSlur && (slurs[staffIdx] == 0)) {
            Slur* slur = new Slur(score);
            slur->setParent(0);
            slur->setTrack(staffIdx * VOICES);
            slur->setTrack2(staffIdx * VOICES);
            slur->setTick(cr->tick());
            slur->setTick2(cr->tick());
            slurs[staffIdx] = slur;
            score->addElement(slur);
            }
      else if (slurs[staffIdx] && !hasSlur) {
            Slur* s = slurs[staffIdx];
            slurs[staffIdx] = 0;
            s->setTick2(cr->tick());
            s->setTrack2(cr->track());
            }
      }

//---------------------------------------------------------
//   createOttava
//---------------------------------------------------------

void GuitarPro::createOttava(bool hasOttava, int track, ChordRest* cr, QString value)
      {
      if (hasOttava && (ottava[track] == 0)) {
            Ottava* newOttava = new Ottava(score);
            newOttava->setTrack(track);
            if (!value.compare("8va"))
                  newOttava->setOttavaType(Ottava::Type::OTTAVA_8VA);
            else if (!value.compare("8vb"))
                  newOttava->setOttavaType(Ottava::Type::OTTAVA_8VB);
            else if (!value.compare("15ma"))
                  newOttava->setOttavaType(Ottava::Type::OTTAVA_15MA);
            else if (!value.compare("15mb"))
                  newOttava->setOttavaType(Ottava::Type::OTTAVA_15MB);
            newOttava->setTick(cr->tick());
            /* we set the second tick when we encounter the next note
               without an ottava. We also allow the ottava to continue
               over rests, as that's what Guitar Pro does. */
            newOttava->setTick2(cr->tick());
            ottava[track] = newOttava;
            score->addElement(newOttava);
            }
      else if (ottava[track] && !hasOttava) {
            Ottava* currentOttava = ottava[track];
            ottava[track] = 0;
            currentOttava->setTick2(cr->tick());
            currentOttava->setProperty(P_ID::LINE_WIDTH,0.1);
            //ottava[track]->staff()->updateOttava(ottava[track]);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro2::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", QString("%1").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());

      /*uchar num =*/ readUChar();      // Shuffle rhythm feel

      int tempo = readInt();

      // int octave = 0;
      /*int key =*/ readInt();    // key

      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            channelDefaults[i].patch   = readInt();
            channelDefaults[i].volume  = readUChar() * 8 - 1;
            channelDefaults[i].pan     = readUChar() * 8 - 1;
            channelDefaults[i].chorus  = readUChar() * 8 - 1;
            channelDefaults[i].reverb  = readUChar() * 8 - 1;
            channelDefaults[i].phase   = readUChar() * 8 - 1;
            channelDefaults[i].tremolo = readUChar() * 8 - 1;
            readUChar();      // padding
            readUChar();
            }
      measures   = readInt();
      staves = readInt();

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & SCORE_TIMESIG_NUMERATOR)
                  tnumerator = readUChar();
            if (barBits & SCORE_TIMESIG_DENOMINATOR)
                  tdenominator = readUChar();
            if (barBits & SCORE_REPEAT_START)
                  bar.repeatFlags = bar.repeatFlags | Repeat::START;
            if (barBits & SCORE_REPEAT_END) {
                  bar.repeatFlags = bar.repeatFlags | Repeat::END;
                  bar.repeats = readUChar();
                  }
            if (barBits & SCORE_VOLTA) {
                  uchar voltaNumber = readUChar();
                  while (voltaNumber > 0) {
                        // volta information is represented as a binary number
                        bar.volta.voltaType = GP_VOLTA_BINARY;
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & SCORE_MARKER) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color =*/ readInt();    // color?
                  }
            if (barBits & SCORE_KEYSIG) {
                  bar.keysig = readUChar();
                  /*uchar c    =*/ readUChar();        // minor
                  }
            if (barBits & SCORE_DOUBLE_BAR)
                  bar.barLine = BarLineType::DOUBLE;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score);
            s->setPart(part);
            part->insertStaff(s, 0);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        TimeSig* t = new TimeSig(score);
                        t->setTrack(staffIdx * VOICES);
                        t->setSig(nts);
                        Segment* s = m->getSegment(Segment::Type::TimeSig, tick);
                        s->add(t);
                        }
                  }

            score->measures()->add(m);
            tick += nts.ticks();
            ts = nts;
            }

      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            uchar c      = readUChar();   // simulations bitmask
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            QString name = readPascalString(40);
            int strings  = readInt();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                  throw GuitarProError::GP_BAD_NUMBER_OF_STRINGS ;
            for (int j = 0; j < strings; ++j)
                  tuning[j] = readInt();
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readInt();
            int midiPort     = readInt() - 1;
            int midiChannel  = readInt() - 1;
            /*int midiChannel2 =*/ readInt(); // - 1;
            int frets        = readInt();
            int capo         = readInt();
            /*int color        =*/ readInt();

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            StringData stringData(frets, strings, tuning2);
            Part* part = score->staff(i)->part();
            Instrument* instr = part->instrument();
            instr->setStringData(stringData);
            part->setPartName(name);
            part->setPlainLongName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            ClefType clefId = ClefType::G;
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  clefId = ClefType::PERC;
                  // instr->setUseDrumset(DrumsetKind::GUITAR_PRO);
                  instr->setDrumset(gpDrumset);
                  staff->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = ClefType::G8_VB;
            else if (patch >= 32 && patch < 40)
                  clefId = ClefType::F8_VB;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(Segment::Type::Clef, 0);
            segment->add(clef);

            if (capo > 0) {
                  Segment* s = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  StaffText* st = new StaffText(score);
                  st->setTextStyleType(TextStyleType::STAFF);
                  st->setPlainText(QString("Capo. fret ") + QString::number(capo));
                  st->setParent(s);
                  st->setTrack(i * VOICES);
                  measure->add(st);
            }

            Channel* ch = instr->channel(0);
            if (midiChannel == int(StaffTypes::PERC_DEFAULT)) {
                  ch->program = 0;
                  ch->bank    = 128;
                  }
            else {
                  ch->program = patch;
                  ch->bank    = 0;
                  }
            ch->volume  = channelDefaults[midiChannel].volume;
            ch->pan     = channelDefaults[midiChannel].pan;
            ch->chorus  = channelDefaults[midiChannel].chorus;
            ch->reverb  = channelDefaults[midiChannel].reverb;
            staff->part()->setMidiChannel(midiChannel, midiPort);
            // missing: phase, tremolo
            ch->updateInitList();
            }

      previousTempo = tempo;
      Measure* measure = score->firstMeasure();
      bool mixChange = false;
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setPlainText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  Fraction measureLen = 0;
                  int track = staffIdx * VOICES;
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
//                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & BEAT_DOTTED;
                        if (beatBits & BEAT_PAUSE)
                              /*pause =*/ readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & BEAT_TUPLET)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                        if (beatBits & BEAT_CHORD) {
                              int numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, track, numStrings, name, false);
                                    }
                              else  {
                                    skip(25);
                                    name = readPascalString(34);
                                    readChord(segment, track, numStrings, name, true);
                                    skip(36);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & BEAT_LYRICS) {
                              QString txt = readDelphiString();
                              lyrics = new Lyrics(score);
                              lyrics->setPlainText(txt);
                              }
                        if (beatBits & BEAT_EFFECTS)
                              readBeatEffects(track, segment);

                        if (beatBits & BEAT_MIX_CHANGE) {
                              readMixChange(measure);
                              mixChange = true;
                              }

                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);
                        ChordRest* cr;
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(track);
                        if (lyrics)
                              cr->add(lyrics);

                        TDuration d(l);
                        d.setDots(dotted ? 1 : 0);

                        if (dotted)
                              l = l + (l/2);

                        if (tuple) {
                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    setTuplet(tuplet, tuple);
                                    tuplet->setParent(measure);
                                    }
                              tuplet->setTrack(track);
                              tuplet->setBaseLen(l);
                              tuplet->setDuration(l * tuplet->ratio().denominator());
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);
                              }

                        cr->setDuration(l);
                        cr->setDurationType(d);
                        segment->add(cr);
                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instrument()->stringData()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);
                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        restsForEmptyBeats(segment, measure, cr, l, track, tick);
                        tick += cr->actualTicks();
                        measureLen += cr->actualFraction();
                        }
                  if (measureLen < measure->len()) {
                        score->setRest(tick, track, measure->len() - measureLen, false, nullptr, false);
                        }
                  }
            if (bar == 1 && !mixChange)
                  setTempo(tempo, score->firstMeasure());
            }
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro1::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();

      if (noteBits & NOTE_GHOST) {
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      uchar variant = 1;
      if (noteBits & NOTE_DEAD) {
            variant = readUChar();
            if (variant == 1) {     // normal note
                  }
            else if (variant == 2) {
                  tieNote = true;
                  }
            else if (variant == 3) {                 // dead notes
                  note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
                  note->setGhost(true);
                  }
            else
                  qDebug("unknown note variant: %d", variant);
            }

      //
      // noteBits:
      //    7 - Right hand or left hand fingering;
      //    6 - Accentuated note
      //    5 - Note type (rest, empty note, normal note);
      //    4 - note dynamic;
      //    3 - Presence of effects linked to the note;
      //    2 - Ghost note;
      //    1 - Dotted note;  ?
      //    0 - Time-independent duration

      if (noteBits & 0x1) {               // note != beat
            int a = readUChar();          // length
            int b = readUChar();          // t
            qDebug("Time independend note len, len %d t %d", a, b);
            }
      if (noteBits & 0x2) {               // note is dotted
            //readUChar();
            }

      // set dynamic information on note if different from previous note
      if (noteBits & NOTE_DYNAMIC) {
            int d = readChar();
            if (previousDynamic != d) {
                  previousDynamic = d;
                  addDynamic(note, d);
                  }
            }

      int fretNumber = -1;
      if (noteBits & NOTE_FRET)
            fretNumber = readUChar();

      if (noteBits & NOTE_FINGERING) {              // fingering
            int a = readUChar();
            int b = readUChar();
            qDebug("Fingering=========%d %d", a, b);
            }
      if (noteBits & BEAT_EFFECTS) {
            uchar modMask1 = readUChar();
            uchar modMask2 = 0;
            if (version >= 400)
                  modMask2 = readUChar();
            if (modMask1 & EFFECT_BEND)
                  readBend(note);
            if (modMask1 & EFFECT_GRACE) {
                  // GP3 grace note
                  int fret = readUChar();            // grace fret
                  int dynamic = readUChar();            // grace dynamic
                  int transition = readUChar();            // grace transition
                  int duration = readUChar();            // grace duration

                  int grace_len = MScore::division/8;
                  if (duration == 1)
                        grace_len = MScore::division/8; //32th
                  else if (duration == 2)
                        grace_len = MScore::division/6; //24th
                  else if (duration == 3)
                        grace_len = MScore::division/4; //16th

                  Note* gn = new Note(score);

                  if (fret == 255) {
                        gn->setHeadGroup(NoteHead::Group::HEAD_CROSS);
                        gn->setGhost(true);
                        }
                  if (fret == 255)
                        fret = 0;
                  gn->setFret(fret);
                  gn->setString(string);
                  int grace_pitch = note->staff()->part()->instrument()->stringData()->getPitch(string, fret, nullptr, 0);
                  gn->setPitch(grace_pitch);
                  gn->setTpcFromPitch();

                  Chord* gc = new Chord(score);
                  gc->add(gn);

                  TDuration d;
                  d.setVal(grace_len);
                  if(grace_len == MScore::division/6)
                        d.setDots(1);
                  gc->setDurationType(d);
                  gc->setDuration(d.fraction());
                  gc->setNoteType(NoteType::ACCIACCATURA);
                  gc->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
                  note->chord()->add(gc); // sets parent + track
                  addDynamic(gn, dynamic);

                  if (transition == 0) {
                        // no transition
                        }
                  else if(transition == 1){
                        //TODO: Add a 'slide' guitar effect when implemented
                        }
                  else if (transition == 2 && fretNumber>=0 && fretNumber<=255 && fretNumber!=gn->fret()) {
                        QList<PitchValue> points;
                        points.append(PitchValue(0,0, false));
                        points.append(PitchValue(60,(fretNumber-gn->fret())*100, false));

                        Bend* b = new Bend(note->score());
                        b->setPoints(points);
                        b->setTrack(gn->track());
                        gn->add(b);
                        }
                   else if (transition == 3) {
                         // TODO:
                         //     major: replace with a 'hammer-on' guitar effect when implemented
                         //     minor: make slurs for parts

                         ChordRest* cr1 = static_cast<Chord*>(gc);
                         ChordRest* cr2 = static_cast<Chord*>(note->chord());

                         Slur* slur = new Slur(score);
                         slur->setStartElement(cr1);
                         slur->setEndElement(cr2);
                         slur->setTick(cr1->tick());
                         slur->setTick2(cr2->tick());
                         slur->setTrack(cr1->track());
                         slur->setTrack2(cr2->track());
                         score->addElement(slur);
                         }
                  }
            if (modMask1 & EFFECT_HAMMER) {         // hammer on / pull off
                  }
            if (modMask1 & EFFECT_LET_RING) {         // let ring
                  }
            if (modMask1 & EFFECT_SLIDE_OLD)
                  slides[note->chord()->track()] = SHIFT_SLIDE;

            if (version >= 400) {
                  if (modMask2 & EFFECT_STACATTO) {
                        }
                  if (modMask2 & EFFECT_PALM_MUTE) {
                        }
                  if (modMask2 & EFFECT_TREMOLO)
                        readUChar();
                  if (modMask2 & EFFECT_ARTIFICIAL_HARMONIC)
                        readUChar();
                  if (modMask2 & EFFECT_TRILL) {
                        readUChar();      // trill fret
                        readUChar();      // trill length
                        }
                  }
            }
      if (fretNumber == -1) {
            qDebug("Note: no fret number, tie %d", tieNote);
            }
      Staff* staff = note->staff();
      if (fretNumber == 255) {
            fretNumber = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }
      // dead note represented as high numbers - fix to zero
      if (fretNumber > 99 || fretNumber == -1)
            fretNumber = 0;
      int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber, nullptr, 0);

      /* it's possible to specifiy extraordinarily high pitches by
      specifying fret numbers that don't exist. This is an issue that
      comes from tuxguitar. Just set to maximum pitch. GP6 actually
      sets the fret number to 0 also, so that's what I've opted to do
      here. */
      if (pitch > MAX_PITCH) {
            fretNumber = 0;
            pitch = MAX_PITCH;
            }

      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      if (tieNote) {
            bool found = false;
            Chord* chord     = note->chord();
            Segment* segment = chord->segment()->prev1(Segment::Type::ChordRest);
            int track        = note->track();
            while (segment) {
                  Element* e = segment->element(track);
                  if (e) {
                        if (e->type() == Element::Type::CHORD) {
                              Chord* chord2 = static_cast<Chord*>(e);
                              foreach(Note* note2, chord2->notes()) {
                                    if (note2->string() == string) {
                                          Tie* tie = new Tie(score);
                                          tie->setEndNote(note);
                                          note2->add(tie);
                                          note->setFret(note2->fret());
                                          note->setPitch(note2->pitch());
                                          found = true;
                                          break;
                                          }
                                    }
                              }
                        if (found)
                              break;
                        }
                  segment = segment->prev1(Segment::Type::ChordRest);
                  }
            }
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro1::readBeatEffects(int, Segment*)
      {
      uchar fxBits1 = readUChar();
      if (fxBits1 & BEAT_EFFECT) {
            uchar num = readUChar();
            switch(num) {
                  case 0:           // tremolo bar
                        readInt();
                        break;
                  default:
                        readInt();
                        break;
                  }
            }
      if (fxBits1 & BEAT_ARPEGGIO) {
            readUChar();            // down stroke length
            readUChar();            // up stroke length
            }
      return 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro3::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", QString("%1").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());

      /*uchar num =*/ readUChar();      // Shuffle rhythm feel

      int tempo = readInt();

      // int octave = 0;
      key = readInt();    // key

      for (int i = 0; i < GP_MAX_TRACK_NUMBER * 2; ++i) {
            channelDefaults[i].patch   = readInt();
            channelDefaults[i].volume  = readUChar() * 8 - 1;
            channelDefaults[i].pan     = readUChar() * 8 - 1;
            channelDefaults[i].chorus  = readUChar() * 8 - 1;
            channelDefaults[i].reverb  = readUChar() * 8 - 1;
            channelDefaults[i].phase   = readUChar() * 8 - 1;
            channelDefaults[i].tremolo = readUChar() * 8 - 1;
            readUChar();      // padding
            readUChar();
            }
      measures   = readInt();
      staves = readInt();

      //previousDynamic = new int [staves * VOICES];
      // initialise the dynamics to 0
      //for (int i = 0; i < staves * VOICES; i++)
      //      previousDynamic[i] = 0;
      previousDynamic = -1;

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & SCORE_TIMESIG_NUMERATOR)
                  tnumerator = readUChar();
            if (barBits & SCORE_TIMESIG_DENOMINATOR)
                  tdenominator = readUChar();
            if (barBits & SCORE_REPEAT_START)
                  bar.repeatFlags = bar.repeatFlags | Repeat::START;
            if (barBits & SCORE_REPEAT_END) {                // number of repeats
                  bar.repeatFlags = bar.repeatFlags | Repeat::END;
                  bar.repeats = readUChar();
                  }
            if (barBits & SCORE_VOLTA) {                      // a volta
                  uchar voltaNumber = readUChar();
                  // voltas are represented as a binary number
                  bar.volta.voltaType = GP_VOLTA_BINARY;
                  while (voltaNumber > 0) {
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & SCORE_MARKER) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color =*/ readInt();    // color?
                  }
            if (barBits & SCORE_KEYSIG) {
                  int currentKey = readUChar();
                  /* key signatures are specified as
                   * 1# = 1, 2# = 2, ..., 7# = 7
                   * 1b = 255, 2b = 254, ... 7b = 249 */
                  bar.keysig = currentKey <= 7 ? currentKey : -256+currentKey;
                  readUChar();        // specifies major/minor mode
                  }

            if (barBits & SCORE_DOUBLE_BAR)
                  bar.barLine = BarLineType::DOUBLE;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score);
            s->setPart(part);
            part->insertStaff(s, 0);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      int tick = 0;
      Fraction ts;
      for (int i = 0; i < measures; ++i) {
            Fraction nts = bars[i].timesig;
            Measure* m = new Measure(score);
            m->setTick(tick);
            m->setTimesig(nts);
            m->setLen(nts);

            if (i == 0 || ts != nts) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        TimeSig* t = new TimeSig(score);
                        t->setTrack(staffIdx * VOICES);
                        t->setSig(nts);
                        Segment* s = m->getSegment(Segment::Type::TimeSig, tick);
                        s->add(t);
                        }
                  }
            if (i == 0 && key) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        KeySig* t = new KeySig(score);
                        t->setKey(Key(key));
                        t->setTrack(staffIdx * VOICES);
                        Segment* s = m->getSegment(Segment::Type::KeySig, tick);
                        s->add(t);
                        }
                  }

            readVolta(&bars[i].volta, m);
            m->setRepeatEnd(bars[i].repeatFlags & Repeat::END);
            m->setRepeatStart(bars[i].repeatFlags & Repeat::START);
            m->setRepeatCount(bars[i].repeats);

            // reset the volta sequence if we have an opening repeat
            if (bars[i].repeatFlags == Repeat::START)
                  voltaSequence = 1;
            // otherwise, if we see an end repeat symbol, only reset if the bar after it does not contain a volta
            else if (bars[i].repeatFlags == Repeat::END && i < bars.length() - 1) {
                  if (bars[i+1].volta.voltaInfo.length() == 0) {
                        voltaSequence = 1;
                        }
                  }

            score->measures()->add(m);
            tick += nts.ticks();
            ts = nts;
            }

      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            uchar c      = readUChar();   // simulations bitmask
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            QString name = readPascalString(40);
            int strings  = readInt();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                  throw GuitarProError::GP_BAD_NUMBER_OF_STRINGS ;
            for (int j = 0; j < strings; ++j)
                  tuning[j] = readInt();
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readInt();
            int midiPort     = readInt() - 1;
            int midiChannel  = readInt() - 1;
            /*int midiChannel2 =*/ readInt(); // - 1;
            int frets        = readInt();
            int capo         = readInt();
            /*int color        =*/ readInt();

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            StringData stringData(frets, strings, tuning2);
            Part* part = score->staff(i)->part();
            Instrument* instr = part->instrument();
            instr->setStringData(stringData);
            part->setPartName(name);
            part->setPlainLongName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            ClefType clefId = ClefType::G;
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  clefId = ClefType::PERC;
                  // instr->setUseDrumset(DrumsetKind::GUITAR_PRO);
                  instr->setDrumset(gpDrumset);
                  staff->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = ClefType::G8_VB;
            else if (patch >= 32 && patch < 40)
                  clefId = ClefType::F8_VB;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(Segment::Type::Clef, 0);
            segment->add(clef);

            if (capo > 0) {
                  Segment* s = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  StaffText* st = new StaffText(score);
                  st->setTextStyleType(TextStyleType::STAFF);
                  st->setPlainText(QString("Capo. fret ") + QString::number(capo));
                  st->setParent(s);
                  st->setTrack(i * VOICES);
                  measure->add(st);
            }

            Channel* ch = instr->channel(0);
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  ch->program = 0;
                  ch->bank    = 128;
                  }
            else {
                  ch->program = patch;
                  ch->bank    = 0;
                  }
            ch->volume  = channelDefaults[midiChannel].volume;
            ch->pan     = channelDefaults[midiChannel].pan;
            ch->chorus  = channelDefaults[midiChannel].chorus;
            ch->reverb  = channelDefaults[midiChannel].reverb;
            staff->part()->setMidiChannel(midiChannel, midiPort);
            // missing: phase, tremolo
            ch->updateInitList();
            }

      previousTempo = tempo;
      Measure* measure = score->firstMeasure();
      bool mixChange = false;
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setPlainText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  Fraction measureLen = 0;
                  int track = staffIdx * VOICES;
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
//                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & BEAT_DOTTED;
                        if (beatBits & BEAT_PAUSE)
                              /*pause =*/ readUChar();

                        slide = -1;
                        if (slides.contains(track))
                              slide = slides.take(track);

                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & BEAT_TUPLET)
                              tuple = readInt();

                        Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                        if (beatBits & BEAT_CHORD) {
                              int numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, track, numStrings, name, false);
                                    }
                              else  {
                                    skip(25);
                                    name = readPascalString(34);
                                    readChord(segment, track, numStrings, name, false);
                                    skip(36);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & BEAT_LYRICS) {
                              QString txt = readDelphiString();
                              lyrics = new Lyrics(score);
                              lyrics->setPlainText(txt);
                              }
                        int beatEffects = 0;
                        if (beatBits & BEAT_EFFECTS)
                              beatEffects = readBeatEffects(track, segment);

                        if (beatBits & BEAT_MIX_CHANGE) {
                              readMixChange(measure);
                              mixChange = true;
                              }

                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);

                        // Some beat effects could add a Chord before this
                        ChordRest* cr = segment->cr(track);
                        // if (!pause || strings)
                        if (strings) {
                              if(!segment->cr(track))
                                    cr = new Chord(score);
                              }
                        else
                              {
                              if(segment->cr(track)){
                                    segment->remove(segment->cr(track));
                                    delete cr;
                                    cr = 0;
                                    }
                              cr = new Rest(score);
                              }
                        cr->setTrack(track);
                        if (lyrics)
                              cr->add(lyrics);

                        TDuration d(l);
                        d.setDots(dotted ? 1 : 0);

                        if (dotted)
                              l = l + (l/2);

                        if (tuple) {
                              Tuplet* tuplet = tuplets[staffIdx];
                              if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                                    tuplet = new Tuplet(score);
                                    tuplet->setTrack(cr->track());
                                    tuplets[staffIdx] = tuplet;
                                    setTuplet(tuplet, tuple);
                                    tuplet->setParent(measure);
                                    }
                              tuplet->setTrack(track);
                              tuplet->setBaseLen(l);
                              tuplet->setDuration(l * tuplet->ratio().denominator());
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);
                              }

                        cr->setDuration(l);

                        if (cr->type() == Element::Type::REST && l >= measure->len()) {
                              cr->setDurationType(TDuration::DurationType::V_MEASURE);
                              cr->setDuration(measure->len());
                              }
                        else
                              cr->setDurationType(d);

                        if(!segment->cr(track))
                              segment->add(cr);

                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instrument()->stringData()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    if (dotted) {
                                          NoteDot* dot = new NoteDot(score);
                                          // there is at most one dotted note in this guitar pro version - set 0 index
                                          // dot->setIdx(0);
                                          dot->setParent(note);
                                          dot->setTrack(track);  // needed to know the staff it belongs to (and detect tablature)
                                          dot->setVisible(true);
                                          note->add(dot);
                                          }
                                    static_cast<Chord*>(cr)->add(note);
                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        if (cr && (cr->type() == Element::Type::CHORD)) {
                              applyBeatEffects(static_cast<Chord*>(cr), beatEffects);
                              if (slide > 0)
                                    createSlide(slide, cr, staffIdx);
                              }

                        restsForEmptyBeats(segment, measure, cr, l, track, tick);
                        tick += cr->actualTicks();
                        measureLen += cr->actualFraction();
                        }
                  if (measureLen < measure->len()) {
                        score->setRest(tick, track, measure->len() - measureLen, false, nullptr, false);
                        }
                  }
            if (bar == 1 && !mixChange)
                  setTempo(tempo, score->firstMeasure());
            }
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro3::readBeatEffects(int track, Segment* segment)
      {
      int effects = 0;
      uchar fxBits = readUChar();

      if (fxBits & BEAT_EFFECT) {
            effects = readUChar();      // effect 1-tapping, 2-slapping, 3-popping
            readInt(); // we don't need this integer
            }

      if (fxBits & BEAT_ARPEGGIO) {
            int strokeup = readUChar();            // up stroke length
            int strokedown = readUChar();            // down stroke length

            Arpeggio* a = new Arpeggio(score);
            if( strokeup > 0 ) {
                  a->setArpeggioType(ArpeggioType::UP);
                  }
            else if( strokedown > 0 ) {
                  a->setArpeggioType(ArpeggioType::DOWN);
                  }
            else {
                  delete a;
                  a = 0;
                  }

            if(a) {
                  ChordRest* cr = new Chord(score);
                  cr->setTrack(track);
                  cr->add(a);
                  segment->add(cr);
                  }
            }
      if (fxBits & BEAT_TREMOLO) {
            }
      if (fxBits & BEAT_FADE) {
            }
      if (fxBits & BEAT_DOTTED) {
            }
      if (fxBits & BEAT_CHORD) {
            }
      return effects;
      }


//---------------------------------------------------------
//   readTremoloBar
//---------------------------------------------------------

void GuitarPro::readTremoloBar(int /*track*/, Segment* /*segment*/)
      {
      /*int a1 =*/ readChar();
      /*int a2 =*/ readChar();
      /*int a3 =*/ readChar();
      /*int a4 =*/ readChar();
      /*int a5 =*/ readChar();
      int n  =  readInt();
      QList<PitchValue> points;
      for (int i = 0; i < n; ++i) {
            int time    = readInt();
            int pitch   = readInt();
            int vibrato = readUChar();
            points.append(PitchValue(time, pitch, vibrato));
            }
      //TODO
      /*TremoloBar* b = new TremoloBar(segment->score());
      b->setPoints(points);
      b->setTrack(track);
      segment->add(b);*/
      }

//---------------------------------------------------------
//   createCrecDim
//---------------------------------------------------------

void GuitarPro::createCrecDim(int staffIdx, int track, int tick, bool crec)
      {
      hairpins[staffIdx] = new Hairpin(score);
      if (crec)
            hairpins[staffIdx]->setHairpinType(HairpinType::CRESC_HAIRPIN);
      else
            hairpins[staffIdx]->setHairpinType(HairpinType::DECRESC_HAIRPIN);
      hairpins[staffIdx]->setTick(tick);
      hairpins[staffIdx]->setTick2(tick);
      hairpins[staffIdx]->setTrack(track);
      hairpins[staffIdx]->setTrack(track);
      score->undoAddElement(hairpins[staffIdx]);
      }

//---------------------------------------------------------
//   importGTP
//---------------------------------------------------------

Score::FileError importGTP(MasterScore* score, const QString& name)
      {
      QFile fp(name);
      if(!fp.exists())
            return Score::FileError::FILE_NOT_FOUND;
      if (!fp.open(QIODevice::ReadOnly))
            return Score::FileError::FILE_OPEN_ERROR;

      GuitarPro* gp;
      try   {
            // check to see if we are dealing with a GPX file via the extension
            if (name.endsWith(".gpx", Qt::CaseInsensitive)) {
                  gp = new GuitarPro6(score);
                  gp->read(&fp);
                  fp.close();
                  }
            // otherwise it's an older version - check the header
            else  {
                  uchar l;
                  fp.read((char*)&l, 1);
                  char ss[30];
                  fp.read(ss, 30);
                  ss[l] = 0;
                  QString s(ss);
                  if (s.startsWith("FICHIER GUITAR PRO "))
                        s = s.mid(20);
                  else if (s.startsWith("FICHIER GUITARE PRO "))
                        s = s.mid(21);
                  else {
                        qDebug("unknown gtp format <%s>", ss);
                        fp.close();
                        return Score::FileError::FILE_BAD_FORMAT;
                        }
                  int a = s.left(1).toInt();
                  int b = s.mid(2).toInt();
                  int version = a * 100 + b;
                  if (a == 1)
                        gp = new GuitarPro1(score, version);
                  if (a == 2)
                        gp = new GuitarPro2(score, version);
                  if (a == 3)
                        gp = new GuitarPro3(score, version);
                  else if (a == 4)
                        gp = new GuitarPro4(score, version);
                  else if (a == 5)
                        gp = new GuitarPro5(score, version);
                  else {
                        qDebug("unknown gtp format %d", version);
                        fp.close();
                        return Score::FileError::FILE_BAD_FORMAT;
                        }
                  gp->initGuitarProDrumset();
                  gp->read(&fp);
                  }
            }
      catch(GuitarPro::GuitarProError errNo) {
            if (!MScore::noGui) {
                  QMessageBox::warning(0,
                     QWidget::tr("MuseScore: Import Guitar Pro"),
                     QWidget::tr("Load failed: ") + gp->error(errNo),
                     QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                  }
            fp.close();
            qDebug("guitar pro import error====");
            // avoid another error message box
            return Score::FileError::FILE_NO_ERROR;
            }
      fp.close();

      MeasureBase* m;
      if (!score->measures()->first()) {
            m = new VBox(score);
            m->setTick(0);
            score->addMeasure(m, 0);
            }
      else  {
            m = score->measures()->first();
            if (m->type() != Element::Type::VBOX) {
                  MeasureBase* mb = new VBox(score);
                  mb->setTick(0);
                  score->addMeasure(mb, m);
                  m = mb;
                  }
            }
      if (!gp->title.isEmpty()) {
            Text* s = new Text(score);
            // s->setSubtype(TEXT_TITLE);
            s->setTextStyleType(TextStyleType::TITLE);
            s->setPlainText(gp->title);
            m->add(s);
            }
      if (!gp->subtitle.isEmpty() && !gp->artist.isEmpty() && !gp->album.isEmpty()) {
            Text* s = new Text(score);
            // s->setSubtype(TEXT_SUBTITLE);
            s->setTextStyleType(TextStyleType::SUBTITLE);
            QString str;
            if (!gp->subtitle.isEmpty())
                  str.append(gp->subtitle);
            if (!gp->artist.isEmpty()) {
                  if (!str.isEmpty())
                        str.append("\n");
                  str.append(gp->artist);
                  }
            if (!gp->album.isEmpty()) {
                  if (!str.isEmpty())
                        str.append("\n");
                  str.append(gp->album);
                  }
            s->setPlainText(str);
            m->add(s);
            }
      if (!gp->composer.isEmpty()) {
            Text* s = new Text(score);
            // s->setSubtype(TEXT_COMPOSER);
            s->setTextStyleType(TextStyleType::COMPOSER);
            s->setPlainText(gp->composer);
            m->add(s);
            }
      int idx = 0;

      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure(), ++idx) {
            const GpBar& bar = gp->bars[idx];
            if (bar.barLine != BarLineType::NORMAL && bar.barLine != BarLineType::END_REPEAT && bar.barLine != BarLineType::START_REPEAT && bar.barLine != BarLineType::END_START_REPEAT)
                  m->setEndBarLineType(bar.barLine, 0);
            }

      //
      // create parts (excerpts)
      //
      foreach(Part* part, score->parts()) {
            QMultiMap<int, int> tracks;
            Score* pscore = new Score(static_cast<MasterScore*>(score));
            pscore->style()->set(StyleIdx::createMultiMeasureRests, true);

            QList<int> stavesMap;
            Part*   p = new Part(pscore);
            p->setInstrument(*part->instrument());

            Staff* staff = part->staves()->front();

            Staff* s = new Staff(pscore);
            s->setPart(p);
            StaffType* st = staff->staffType();
            s->setStaffType(st);
//            int idx = pscore->staffTypeIdx(st);
//            if (idx == -1)
//                  pscore->addStaffType(st);
            s->linkTo(staff);
            p->staves()->append(s);
            pscore->staves().append(s);
            stavesMap.append(staff->idx());

            for (int i = staff->idx() * VOICES, j = 0; i < staff->idx() * VOICES + VOICES; i++, j++)
                  tracks.insert(i, j);

//            pscore->setName(part->partName());
            Excerpt* excerpt = new Excerpt(score);
            excerpt->setTracks(tracks);
            excerpt->setPartScore(pscore);
            pscore->setExcerpt(excerpt);
            excerpt->setTitle(part->partName());
            excerpt->parts().append(part);
            score->excerpts().append(excerpt);

            Excerpt::cloneStaves(score, pscore, stavesMap, tracks);

            if (staff->part()->instrument()->stringData()->strings() > 0 && part->staves()->front()->staffType()->group() == StaffGroup::STANDARD) {
                  p->setStaves(2);
                  Staff* s1 = p->staff(1);

                  int lines = staff->part()->instrument()->stringData()->strings();
                  StaffTypes sts = StaffTypes::TAB_DEFAULT;
                  if (lines == 4)
                        sts = StaffTypes::TAB_4COMMON;
                  StaffType st = *StaffType::preset(sts);
                  s1->setStaffType(&st);
                  s1->setLines(lines);
                  Excerpt::cloneStaff(s,s1);
                  p->staves()->front()->addBracket(BracketItem(BracketType::NORMAL, 2));
                  }
            pscore->appendPart(p);

            //
            // create excerpt title
            //
            MeasureBase* measure = pscore->first();
            if (!measure || (measure->type() != Element::Type::VBOX)) {
                  MeasureBase* mb = new VBox(pscore);
                  mb->setTick(0);
                  pscore->addMeasure(mb, measure);
                  measure = mb;
                  }
            Text* txt = new Text(pscore);
            txt->setTextStyleType(TextStyleType::INSTRUMENT_EXCERPT);
            txt->setPlainText(part->longName());
            measure->add(txt);

            //
            // layout score
            //
            pscore->setPlaylistDirty();

            pscore->setLayoutAll();
            pscore->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
            pscore->doLayout();
            }
      score->rebuildMidiMapping();
      score->updateChannel();

//      album
//      copyright

      score->setCreated(true);
      delete gp;

      return Score::FileError::FILE_NO_ERROR;
      }
}

