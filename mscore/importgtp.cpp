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
      repeatFlags = 0;
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
      char c;
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
                  qFatal("unsupported tuplet %d", tuple);
            }
      }

//---------------------------------------------------------
//   addDynamic
//---------------------------------------------------------

void GuitarPro::addDynamic(Note* note, int d)
      {
      Dynamic* dyn = new Dynamic(score);
      QString map_dyn[] = {"f","ppp","pp","p","mp","mf","f","ff","fff"};
      dyn->setDynamicType(map_dyn[d]);
      dyn->setTrack(note->track());
      note->chord()-> segment()-> add(dyn);
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
            volta->setText(voltaTextString);
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
      Bend* bend = new Bend(gscore);
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
      readInt();        // lyric track
      for (int i = 0; i < 5; ++i) {
            readInt();
            readWordPascalString();
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
            //case  6: l.set(1, 512);  break;
            //case  7: l.set(1, 1024);  break;
            //case  8: l.set(1, 2048);  break;
            default:
                  qFatal("unknown beat len: %d", len);
            }
      return l;
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro::readMixChange(Measure* measure)
      {
      /*char patch   =*/ readChar();
      char volume  = readChar();
      char pan     = readChar();
      char chorus  = readChar();
      char reverb  = readChar();
      char phase   = readChar();
      char tremolo = readChar();
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
      }

//---------------------------------------------------------
//   createMeasures
//---------------------------------------------------------

void GuitarPro::createMeasures()
      {
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
                        Staff* staff = score->staff(staffIdx);
                        StaffType* staffType = staff->staffType();
qDebug("staff %d group %d timesig %d", staffIdx, int(staffType->group()), staffType->genTimesig());
                        if (staffType->genTimesig()) {
                              TimeSig* t = new TimeSig(score);
                              t->setTrack(staffIdx * VOICES);
                              t->setSig(nts);
                              Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                              s->add(t);
                              }
                        }
                  }
            if (i == 0 || (bars[i].keysig != GP_INVALID_KEYSIG)) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        KeySig* t = new KeySig(score);
                        int keysig = bars[i].keysig != GP_INVALID_KEYSIG ? bars[i].keysig : key;
                        t->setKey(Key(keysig));
                        t->setTrack(staffIdx * VOICES);
                        Segment* s = m->getSegment(SegmentType::KeySig, tick);
                        s->add(t);
                        }
                  }
            readVolta(&bars[i].volta, m);
            m->setRepeatFlags(bars[i].repeatFlags);
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

            score->add(m);
            tick += nts.ticks();
            ts = nts;
            }
      }

//---------------------------------------------------------
//   applyBeatEffects
//---------------------------------------------------------

void GuitarPro::applyBeatEffects(Chord* chord, int beatEffect)
      {
      if (beatEffect == 0)
            return;

      Articulation* a = new Articulation(chord->score());
      chord->add(a);
      switch (beatEffect) {
#if 0 // TODO-smufl
            case 1:
                  a->setArticulationType(ArticulationType::Tapping);
                  break;
            case 2:
                  a->setArticulationType(ArticulationType::Slapping);
                  break;
            case 3:
                  a->setArticulationType(ArticulationType::Popping);
                  break;
#endif
            default:
                  qDebug("GuitarPro import: unknown beat effect %d", beatEffect);
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
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
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
            Instrument* instr = part->instr();
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
                        Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                        s->add(t);
                        }
                  }

            score->add(m);
            tick += nts.ticks();
            ts = nts;
            }

      previousTempo = tempo;
      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
//                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              /*pause =*/ readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                        if (beatBits & 0x2) {
                              int numStrings = score->staff(staffIdx)->part()->instr()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, staffIdx * VOICES, numStrings, name, false);
                                    }
                              else  {
                                    skip(25);
                                    name = readPascalString(34);
                                    readChord(segment, staffIdx * VOICES, numStrings, name, true);
                                    skip(36);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & 0x4) {
                              lyrics = new Lyrics(score);
                              lyrics->setText(readDelphiString());
                              }
                        if (beatBits & 0x8)
                              readBeatEffects(staffIdx * VOICES, segment);
                        if (beatBits & 0x10)
                              readMixChange(measure);
                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);
                        ChordRest* cr;
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES);
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
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);  //TODOxxx
                              }

                        cr->setDuration(l);
                        cr->setDurationType(d);
                        segment->add(cr);
                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instr()->stringData()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);
                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        tick += cr->actualTicks();
                        }
                  }
            }
      setTempo(tempo, score->firstMeasure());
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void GuitarPro::setTempo(int tempo, Measure* measure)
      {
      TempoText* tt = new TempoText(score);
      tt->setTempo(double(tempo)/60.0);
      tt->setText(QString("<sym>noteQuarterUp</sym> = %1").arg(tempo));

      tt->setTrack(0);
      Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
      segment->add(tt);
      score->setTempo(measure->tick(), tt->tempo());
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
            score->setMetaTag("copyright", QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

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
            if (barBits & 0x1)
                  tnumerator = readUChar();
            if (barBits & 0x2)
                  tdenominator = readUChar();
            if (barBits & 0x4) {                // begin reapeat
qDebug("BeginRepeat=============================================");
                  }
            if (barBits & 0x8)                  // number of repeats
                  /*uchar c =*/ readUChar();
            if (barBits & 0x10)                 // alternative ending to
                  /*uchar c =*/ readUChar();
            if (barBits & 0x20) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color =*/ readInt();    // color?
                  }
            if (barBits & 0x40) {
                  bar.keysig = readUChar();
                  /*uchar c    =*/ readUChar();        // minor
                  }
            if (barBits & 0x80)
                  bar.barLine = BarLineType::DOUBLE;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
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
                        Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                        s->add(t);
                        }
                  }

            score->add(m);
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
            /*int midiPort     =*/ readInt(); //  - 1;
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
            Instrument* instr = part->instr();
            instr->setStringData(stringData);
            part->setPartName(name);
            instr->setTranspose(Interval(capo));
            part->setLongName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            ClefType clefId = ClefType::G;
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  clefId = ClefType::PERC;
                  instr->setUseDrumset(true);
                        staff->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = ClefType::G3;
            else if (patch >= 32 && patch < 40)
                  clefId = ClefType::F8;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegmentType::Clef, 0);
            segment->add(clef);
            clef->staff()->setClef(0, clef->clefTypeList());

            Channel& ch = instr->channel(0);
            if (midiChannel == int(StaffTypes::PERC_DEFAULT)) {
                  ch.program = 0;
                  ch.bank    = 128;
                  }
            else {
                  ch.program = patch;
                  ch.bank    = 0;
                  }
            ch.volume  = channelDefaults[midiChannel].volume;
            ch.pan     = channelDefaults[midiChannel].pan;
            ch.chorus  = channelDefaults[midiChannel].chorus;
            ch.reverb  = channelDefaults[midiChannel].reverb;
            // missing: phase, tremolo
            ch.updateInitList();
            }

      previousTempo = tempo;
      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
//                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              /*pause =*/ readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                        if (beatBits & 0x2) {
                              int numStrings = score->staff(staffIdx)->part()->instr()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, staffIdx * VOICES, numStrings, name, false);
                                    }
                              else  {
                                    skip(25);
                                    name = readPascalString(34);
                                    readChord(segment, staffIdx * VOICES, numStrings, name, true);
                                    skip(36);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              lyrics = new Lyrics(score);
                              lyrics->setText(txt);
                              }
                        if (beatBits & 0x8)
                              readBeatEffects(staffIdx * VOICES, segment);
                        if (beatBits & 0x10)
                              readMixChange(measure);
                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);
                        ChordRest* cr;
                        if (strings)
                              cr = new Chord(score);
                        else
                              cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES);
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
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);
                              }

                        cr->setDuration(l);
                        cr->setDurationType(d);
                        segment->add(cr);
                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instr()->stringData()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);
                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        tick += cr->actualTicks();
                        }
                  }
            }
      setTempo(tempo, score->firstMeasure());
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro1::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();

      if (noteBits & 0x04) {
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      uchar variant = 1;
      if (noteBits & 0x20) {
            variant = readUChar();
            if (variant == 1) {     // normal note
                  }
            else if (variant == 2) {
                  tieNote = true;
                  }
            else if (variant == 3) {                 // dead notes
                  note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                  note->setGhost(true);
                  qDebug("DeathNote tick %d pitch %d", note->chord()->segment()->tick(), note->pitch());
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
            }

      // set dynamic information on note if different from previous note
      if (noteBits & 0x10) {
            int d = readChar();
            if (previousDynamic != d) {
                  previousDynamic = d;
                  addDynamic(note, d);
                  }
            }

      int fretNumber = -1;
      if (noteBits & 0x20)
            fretNumber = readUChar();

      if (noteBits & 0x80) {              // fingering
            int a = readUChar();
            int b = readUChar();
            qDebug("Fingering=========%d %d", a, b);
            }
      if (noteBits & 0x8) {
            uchar modMask1 = readUChar();
            uchar modMask2 = 0;
            if (version >= 400)
                  modMask2 = readUChar();
            if (modMask1 & 0x1)
                  readBend(note);
            if (modMask1 & 0x10) {
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
                        gn->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                        gn->setGhost(true);
                        }
                  gn->setFret((fret != 255)?fret:0);
                  gn->setString(string);
                  int grace_pitch = note->staff()->part()->instr()->stringData()->getPitch(string, fret);
                  gn->setPitch(grace_pitch);
                  gn->setTpcFromPitch();

                  Chord* gc = new Chord(score);
                  gc->setTrack(note->chord()->track());
                  gc->add(gn);
                  gc->setParent(note->chord());
                  note->chord()->add(gc);

                  // TODO: Add dynamic. Dynamic now can be added only to a segment, not directly to a grace note
                  addDynamic(gn, dynamic);

                  TDuration d;
                  d.setVal(grace_len);
                  if(grace_len == MScore::division/6)
                        d.setDots(1);
                  gc->setDurationType(d);
                  gc->setDuration(d.fraction());
                  gc->setNoteType(NoteType::ACCIACCATURA);
                  gc->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));

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
                         slur->setAnchor(Spanner::Anchor::CHORD);
                         slur->setStartChord(static_cast<Chord*>(cr1));
                         slur->setEndChord(static_cast<Chord*>(cr2));
                         slur->setTick(cr1->tick());
                         slur->setTick2(cr2->tick());
                         slur->setTrack(cr1->track());
                         slur->setTrack2(cr2->track());
                         slur->setParent(cr1);
                         score->undoAddElement(slur);
                         }
                  }
            if (modMask1 & 0x2) {         // hammer on / pull off
                  }
            if (modMask1 & 0x8) {         // let ring
                  }

            if (version >= 400) {
                  if (modMask2 & 0x1) {   // staccato - palm mute
                        }
                  if (modMask2 & 0x2) {   // palm mute - mute the whole column
                        }
                  if (modMask2 & 0x4)     // tremolo picking length
                        readUChar();
                  if (modMask2 & 0x8)
                        readUChar();      // slide kind
                  if (modMask2 & 0x10)
                        readUChar();      // harmonic kind
                  if (modMask2 & 0x20) {
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
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
            }
      int pitch = staff->part()->instr()->stringData()->getPitch(string, fretNumber);
      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      if (tieNote) {
            bool found = false;
            Chord* chord     = note->chord();
            Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
            int track        = note->track();
            while (segment) {
                  Element* e = segment->element(track);
                  if (e) {
                        if (e->type() == ElementType::CHORD) {
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
                  segment = segment->prev1(SegmentType::ChordRest);
                  }
            if (!found)
                  qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            }
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro1::readBeatEffects(int, Segment*)
      {
      uchar fxBits1 = readUChar();
      if (fxBits1 & 0x20) {
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
      if (fxBits1 & 0x40) {
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
            score->setMetaTag("copyright", QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

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

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & 0x1)
                  tnumerator = readUChar();
            if (barBits & 0x2)
                  tdenominator = readUChar();
            if (barBits & 0x4)
                  bar.repeatFlags |= Repeat::START;
            if (barBits & 0x8) {                // number of repeats
                  bar.repeatFlags |= Repeat::END;
                  bar.repeats = readUChar();
                  }
            if (barBits & 0x10) {                      // a volta
                  uchar voltaNumber = readUChar();
                  // voltas are represented as a binary number
                  bar.volta.voltaType = GP_VOLTA_BINARY;
                  while (voltaNumber > 0) {
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & 0x20) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color =*/ readInt();    // color?
                  }
            if (barBits & 0x40) {
                  int currentKey = readUChar();
                  /* key signatures are specified as
                   * 1# = 1, 2# = 2, ..., 7# = 7
                   * 1b = 255, 2b = 254, ... 7b = 249 */
                  bar.keysig = currentKey <= 7 ? currentKey : -256+currentKey;
                  readUChar();        // specifies major/minor mode
                  }

            if (barBits & 0x80)
                  bar.barLine = BarLineType::DOUBLE;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
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
                        Segment* s = m->getSegment(SegmentType::TimeSig, tick);
                        s->add(t);
                        }
                  }
            if (i == 0 && key) {
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        KeySig* t = new KeySig(score);
                        t->setKey(Key(key));
                        t->setTrack(staffIdx * VOICES);
                        Segment* s = m->getSegment(SegmentType::KeySig, tick);
                        s->add(t);
                        }
                  }

            readVolta(&bars[i].volta, m);
            m->setRepeatFlags(bars[i].repeatFlags);
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

            score->add(m);
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
            /*int midiPort     =*/ readInt(); // - 1;
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
            Instrument* instr = part->instr();
            instr->setStringData(stringData);
            part->setPartName(name);
            part->setLongName(name);
            instr->setTranspose(Interval(capo));

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            ClefType clefId = ClefType::G;
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  clefId = ClefType::PERC;
                  instr->setUseDrumset(true);
                        staff->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = ClefType::G3;
            else if (patch >= 32 && patch < 40)
                  clefId = ClefType::F8;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegmentType::Clef, 0);
            segment->add(clef);
            clef->staff()->setClef(0, clef->clefTypeList());

            Channel& ch = instr->channel(0);
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  ch.program = 0;
                  ch.bank    = 128;
                  }
            else {
                  ch.program = patch;
                  ch.bank    = 0;
                  }
            ch.volume  = channelDefaults[midiChannel].volume;
            ch.pan     = channelDefaults[midiChannel].pan;
            ch.chorus  = channelDefaults[midiChannel].chorus;
            ch.reverb  = channelDefaults[midiChannel].reverb;
            // missing: phase, tremolo
            ch.updateInitList();
            }

      previousTempo = tempo;
      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
//                        int pause = 0;
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & 0x1;
                        if (beatBits & 0x40)
                              /*pause =*/ readUChar();

                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();

                        Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                        if (beatBits & 0x2) {
                              int numStrings = score->staff(staffIdx)->part()->instr()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, staffIdx * VOICES, numStrings, name, false);
                                    }
                              else  {
                                    skip(25);
                                    name = readPascalString(34);
                                    readChord(segment, staffIdx * VOICES, numStrings, name, true);
                                    skip(36);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & 0x4) {
                              QString txt = readDelphiString();
                              lyrics = new Lyrics(score);
                              lyrics->setText(txt);
                              }
                        if (beatBits & 0x8)
                              readBeatEffects(staffIdx * VOICES, segment);
                        if (beatBits & 0x10)
                              readMixChange(measure);
                        int strings = readUChar();   // used strings mask

                        Fraction l = len2fraction(len);

                        // Some beat effects could add a Chord before this
                        ChordRest* cr = segment->cr(staffIdx * VOICES);
                        // if (!pause || strings)
                        if (strings) {
                              if(!segment->cr(staffIdx * VOICES))
                                    cr = new Chord(score);
                              }
                        else
                              {
                              if(segment->cr(staffIdx * VOICES)){
                                    segment->remove(segment->cr(staffIdx * VOICES));
                                    delete cr;
                                    cr = 0;
                                    }
                              cr = new Rest(score);
                              }
                        cr->setTrack(staffIdx * VOICES);
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
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);
                              }

                        cr->setDuration(l);
                        cr->setDurationType(d);

                        if(!segment->cr(staffIdx * VOICES))
                              segment->add(cr);

                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instr()->stringData()->strings();
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);
                                    readNote(6-i, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        tick += cr->actualTicks();
                        }
                  }
            }
      setTempo(tempo, score->firstMeasure());
      }

int GuitarPro3::readBeatEffects(int track, Segment* segment)
      {
      int effects = 0;
      uchar fxBits = readUChar();

      if (fxBits & 0x20) {
            effects = readUChar();      // effect 1-tapping, 2-slapping, 3-popping
            }

      if (fxBits & 0x40) {
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
      if (fxBits & 0x04) { // natural harmonic
            }
      if (fxBits & 0x08) {  // artificial harmonic
            }
      if (fxBits & 0x10) { //fade in
            }
      if (fxBits & 0x01) {         // GP3 column-wide vibrato
            }
      if (fxBits & 0x2) {          // GP3 column-wide wide vibrato (="tremolo" in GP3)
            }
      return effects;
      }


//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro4::readMixChange(Measure* measure)
      {
      /*char patch   =*/ readChar();
      char volume  = readChar();
      char pan     = readChar();
      char chorus  = readChar();
      char reverb  = readChar();
      char phase   = readChar();
      char tremolo = readChar();
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

      readChar();       // bitmask: what should be applied to all tracks
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro4::readBeatEffects(int track, Segment* segment)
      {
      int effects = 0;
      uchar fxBits1 = readUChar();
      uchar fxBits2 = readUChar();
      if (fxBits1 & 0x20) {
            effects = readUChar();      // effect 1-tapping, 2-slapping, 3-popping
            }
      if (fxBits2 & 0x04)
            readTremoloBar(track,segment);
      if (fxBits1 & 0x40) {
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
      if (fxBits2 & 0x02)
            readUChar();            // stroke pick direction
      if (fxBits1 & 0x01) {         // GP3 column-wide vibrato
            }
      if (fxBits1 & 0x2) {          // GP3 column-wide wide vibrato (="tremolo" in GP3)
            }
      return effects;
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro4::readNote(int string, Note* note, GpNote* gpNote)
      {
      uchar noteBits = readUChar();

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

      if (noteBits & 0x04) {
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      uchar variant = 1;
      if (noteBits & 0x20) {
            variant = readUChar();
            if (variant == 1) {     // normal note
                  }
            else if (variant == 2) {
                  tieNote = true;
                  }
            else if (variant == 3) {                 // dead notes = ghost note
                  note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                  note->setGhost(true);
                  }
            else
                  qDebug("unknown note variant: %d", variant);
            }

      if (noteBits & 0x1) {               // note != beat
            int a = readUChar();          // length
            int b = readUChar();          // t
            qDebug("          Time independend note len, len %d t %d", a, b);
            }
      if (noteBits & 0x2) {               // note is dotted
            }

      // set dynamic information on note if different from previous note
      if (noteBits & 0x10) {
            int d = readChar();
            if (previousDynamic != d) {
                  previousDynamic = d;
                  addDynamic(note, d);
                  }
            }

      int fretNumber = -1;
      if (noteBits & 0x20)
            fretNumber = readUChar();

      // check if a note is supposed to be accented, and give it the sforzato type
      if (noteBits & 0x40) {
            Articulation* art = new Articulation(note->score());
            art->setArticulationType(ArticulationType::Sforzatoaccent);
            if (!note->score()->addArticulation(note, art))
                  delete art;
            }

      if (noteBits & 0x80) {              // fingering
            int a = readUChar();
            int b = readUChar();
            qDebug("Fingering=========%d %d", a, b);
            }
      gpNote->slur = false;
      if (noteBits & 0x8) {
            uchar modMask1 = readUChar();
            uchar modMask2 = readUChar();
            if (modMask1 & 0x1)
                  readBend(note);
            if (modMask1 & 0x2) {         // hammer on / pull off
                  gpNote->slur = true;
                  }
            if (modMask1 & 0x8) {         // let ring
                  }
            if (modMask1 & 0x10) {
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
                        gn->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                        gn->setGhost(true);
                        }
                  gn->setFret((fret != 255)?fret:0);
                  gn->setString(string);
                  int grace_pitch = note->staff()->part()->instr()->stringData()->getPitch(string, fret);
                  gn->setPitch(grace_pitch);
                  gn->setTpcFromPitch();

                  Chord* gc = new Chord(score);
                  gc->setTrack(note->chord()->track());
                  gc->add(gn);
                  gc->setParent(note->chord());
                  note->chord()->add(gc);

                  // TODO: Add dynamic. Dynamic now can be added only to a segment, not directly to a grace note
                  addDynamic(gn, dynamic);

                  TDuration d;
                  d.setVal(grace_len);
                  if(grace_len == MScore::division/6)
                        d.setDots(1);
                  gc->setDurationType(d);
                  gc->setDuration(d.fraction());
                  gc->setNoteType(NoteType::ACCIACCATURA);
                  gc->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));

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
                         slur->setAnchor(Spanner::Anchor::CHORD);
                         slur->setStartChord(static_cast<Chord*>(cr1));
                         slur->setEndChord(static_cast<Chord*>(cr2));
                         slur->setTick(cr1->tick());
                         slur->setTick2(cr2->tick());
                         slur->setTrack(cr1->track());
                         slur->setTrack2(cr2->track());
                         slur->setParent(cr1);
                         score->undoAddElement(slur);
                         }
                  }
            if (modMask2 & 0x1) {   // staccato
                  }
            if (modMask2 & 0x2) {   // palm mute - mute the whole column
                  }
            if (modMask2 & 0x4) {    // tremolo picking length
                  int tremoloDivision = readUChar();
                  Chord* chord = note->chord();
                  Tremolo* t = new Tremolo(chord->score());
                  if (tremoloDivision == 1) {
                        t->setTremoloType(TremoloType::R8);
                        chord->add(t);
                        }
                  else if (tremoloDivision == 2) {
                        t->setTremoloType(TremoloType::R16);
                        chord->add(t);
                        }
                  else if (tremoloDivision == 3) {
                        t->setTremoloType(TremoloType::R32);
                        chord->add(t);
                        }
                  else
                        qDebug("Unknown tremolo value");
                  }
            if (modMask2 & 0x8)
                  readUChar();      // slide kind
            if (modMask2 & 0x10)
                  readUChar();      // harmonic kind
            if (modMask2 & 0x20) {
                  readUChar();      // trill fret
                  readUChar();      // trill length

                  // add the trill articulation to the note
                  Articulation* art = new Articulation(note->score());
                  art->setArticulationType(ArticulationType::Trill);
                  if (!note->score()->addArticulation(note, art))
                        delete art;

                  }
            }
      if (fretNumber == -1) {
            qDebug("Note: no fret number, tie %d", tieNote);
            }
      Staff* staff = note->staff();
      if (fretNumber == 255) {
            fretNumber = 0;
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
            }
      int pitch = staff->part()->instr()->stringData()->getPitch(string, fretNumber);
      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      if (tieNote) {
            bool found = false;
            Chord* chord     = note->chord();
            Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
            int track        = note->track();
            while (segment) {
                  Element* e = segment->element(track);
                  if (e) {
                        if (e->type() == ElementType::CHORD) {
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
                  segment = segment->prev1(SegmentType::ChordRest);
                  }
            if (!found)
                  qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            }
      }

//---------------------------------------------------------
//   readInfo
//---------------------------------------------------------

void GuitarPro4::readInfo()
      {
      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      }


//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro4::read(QFile* fp)
      {
      f      = fp;
      curPos = 30;

      readInfo();
      readUChar();      // triplet feeling
      readLyrics();

      int tempo  = readInt();
      key        = readInt();
      /*int octave =*/ readUChar();    // octave

      readChannels();
      measures = readInt();
      staves   = readInt();

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & 0x1)
                  tnumerator = readUChar();
            if (barBits & 0x2)
                  tdenominator = readUChar();
            if (barBits & 0x4)
                  bar.repeatFlags |= Repeat::START;
            if (barBits & 0x8) {                // number of repeats
                  bar.repeatFlags |= Repeat::END;
                  bar.repeats = readUChar();
                  }
            if (barBits & 0x10) {                      // a volta
                  uchar voltaNumber = readUChar();
                  while (voltaNumber > 0) {
                        // volta information is represented as a binary number
                        bar.volta.voltaType = GP_VOLTA_BINARY;
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & 0x20) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color = */ readInt();    // color?
                  }
            if (barBits & 0x40) {
                  int currentKey = readUChar();
                  /* key signatures are specified as
                   * 1# = 1, 2# = 2, ..., 7# = 7
                   * 1b = 255, 2b = 254, ... 7b = 249 */
                  bar.keysig = currentKey <= 7 ? currentKey : -256+currentKey;
                  readUChar();        // specifies major/minor mode
                  }
            if (barBits & 0x80)
                  bar.barLine = BarLineType::DOUBLE;
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      createMeasures();

      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];

            uchar c = readUChar();   // simulations bitmask
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
            /*int midiPort     =*/ readInt(); // - 1;
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
            Instrument* instr = part->instr();
            instr->setStringData(stringData);
            part->setPartName(name);
            instr->setTranspose(Interval(capo));
            part->setLongName(name);

            //
            // determine clef
            //
            Staff* staff = score->staff(i);
            int patch = channelDefaults[midiChannel].patch;
            ClefType clefId = ClefType::G;
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  clefId = ClefType::PERC;
                  instr->setUseDrumset(true);
                        staff->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = ClefType::G3;
            else if (patch >= 32 && patch < 40)
                  clefId = ClefType::F8;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegmentType::Clef, 0);
            segment->add(clef);
            clef->staff()->setClef(0, clef->clefTypeList());


            Channel& ch = instr->channel(0);
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  ch.program = 0;
                  ch.bank    = 128;
                  }
            else {
                  ch.program = patch;
                  ch.bank    = 0;
                  }
            ch.volume  = channelDefaults[midiChannel].volume;
            ch.pan     = channelDefaults[midiChannel].pan;
            ch.chorus  = channelDefaults[midiChannel].chorus;
            ch.reverb  = channelDefaults[midiChannel].reverb;
            // missing: phase, tremolo
            ch.updateInitList();
            }

      Slur* slurs[staves];
      for (int i = 0; i < staves; ++i)
            slurs[i] = 0;
      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves];
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  int tick  = measure->tick();
                  int beats = readInt();
                  for (int beat = 0; beat < beats; ++beat) {
                        uchar beatBits = readUChar();
                        bool dotted = beatBits & 0x1;
                        int pause = -1;
                        if (beatBits & 0x40)
                              pause = readUChar();
                        int len = readChar();
                        int tuple = 0;
                        if (beatBits & 0x20)
                              tuple = readInt();
                        Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                        if (beatBits & 0x2){
                              int numStrings = score->staff(staffIdx)->part()->instr()->stringData()->strings();
                              int header = readUChar();
                              QString name;
                              if ((header & 1) == 0) {
                                    name = readDelphiString();
                                    readChord(segment, staffIdx * VOICES, numStrings, name, false);
                                    }
                              else  {
                                    skip(16);
                                    name = readPascalString(21);
                                    skip(4);
                                    readChord(segment, staffIdx * VOICES, numStrings, name, true);
                                    skip(32);
                                    }
                              }
                        Lyrics* lyrics = 0;
                        if (beatBits & 0x4) {
                              lyrics = new Lyrics(score);
                              lyrics->setText(readDelphiString());
                              }
                        if (beatBits & 0x8)
                              readBeatEffects(staffIdx * VOICES, segment);
                        if (beatBits & 0x10)
                              readMixChange(measure);
                        int strings = readUChar();   // used strings mask
                        Fraction l  = len2fraction(len);

                        // Some beat effects could add a Chord before this
                        ChordRest* cr = segment->cr(staffIdx * VOICES);

                        if (strings == 0) {
                              if(segment->cr(staffIdx * VOICES)){
                                    segment->remove(segment->cr(staffIdx * VOICES));
                                    delete cr;
                                    cr = 0;
                                    }
                              cr = new Rest(score);
                              }
                        else {
                              if(!segment->cr(staffIdx * VOICES))
                                    cr = new Chord(score);
                              }

                        cr->setTrack(staffIdx * VOICES);
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
                              tuplet->setTrack(staffIdx * VOICES);
                              tuplet->setBaseLen(l);
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);
                              }


                        cr->setDuration(l);
                        if (cr->type() == ElementType::REST && (pause == 0 || l == measure->len()))
                              cr->setDurationType(TDuration::DurationType::V_MEASURE);
                        else
                              cr->setDurationType(d);
                        if(!segment->cr(staffIdx * VOICES))
                              segment->add(cr);
                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instr()->stringData()->strings();
                        bool hasSlur = false;
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    static_cast<Chord*>(cr)->add(note);

                                    GpNote gpNote;
                                    readNote(6-i, note, &gpNote);
                                    if (gpNote.slur)
                                          hasSlur = true;
                                    note->setTpcFromPitch();
                                    }
                              }
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
                              // TODO: check slur
                              Slur* s = slurs[staffIdx];
                              slurs[staffIdx] = 0;
                              s->setTick2(cr->tick());
                              s->setTrack2(cr->track());
                              }
                        else if (slurs[staffIdx] && hasSlur) {
                              }
                        tick += cr->actualTicks();
                        }
                  }
            }
      setTempo(tempo, score->firstMeasure());
      }

//---------------------------------------------------------
//   readInfo
//---------------------------------------------------------

void GuitarPro5::readInfo()
      {
      title        = readDelphiString();
      subtitle     = readDelphiString();
      artist       = readDelphiString();
      album        = readDelphiString();
      composer     = readDelphiString();
      readDelphiString();
      QString copyright = readDelphiString();
      if (!copyright.isEmpty())
            score->setMetaTag("copyright", QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      }

//---------------------------------------------------------
//   readNoteEffects
//---------------------------------------------------------

void GuitarPro5::readNoteEffects(Note* note)
      {
      uchar modMask1 = readUChar();
      uchar modMask2 = readUChar();
      if (modMask1 & 0x1)
            readBend(note);
      if (modMask1 & 0x2) {         // hammer on / pull off
            }
      if (modMask1 & 0x8) {         // let ring
            }
      if (modMask1 & 0x10) {
            int fret = readUChar();            // grace fret
            int dynamic = readUChar();            // grace dynamic
            int transition = readUChar();            // grace transition
            int duration = readUChar();            // grace duration
            int gflags = readUChar();

            int grace_len = MScore::division/8;
            NoteType note_type =  NoteType::ACCIACCATURA;

            if(gflags & 0x02) //on beat
                  note_type = NoteType::APPOGGIATURA;

            if (duration == 1)
                  grace_len = MScore::division/8; //32th
            else if (duration == 2)
                  grace_len = MScore::division/6; //24th
            else if (duration == 3)
                  grace_len = MScore::division/4; //16th

            Note* gn = new Note(score);

            if (gflags & 0x01) {
                  gn->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                  gn->setGhost(true);
                  }
            gn->setFret(fret);
            gn->setString(note->string());
            int grace_pitch = note->staff()->part()->instr()->stringData()->getPitch(note->string(), fret);
            gn->setPitch(grace_pitch);
            gn->setTpcFromPitch();

            Chord* gc = new Chord(score);
            gc->setTrack(note->chord()->track());
            gc->add(gn);
            gc->setParent(note->chord());
            note->chord()->add(gc);

            // TODO: Add dynamic. Dynamic now can be added only to a segment, not directly to a grace note
            addDynamic(gn, dynamic);

            TDuration d;
            d.setVal(grace_len);
            if(grace_len == MScore::division/6)
                  d.setDots(1);
            gc->setDurationType(d);
            gc->setDuration(d.fraction());
            gc->setNoteType(note_type);
            gc->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
            if (transition == 0) {
                  // no transition
                  }
            else if(transition == 1){
                  //TODO: Add a 'slide' guitar effect when implemented
                  }
            else if (transition == 2 && note->fret()>=0 && note->fret()<=255 && note->fret()!=gn->fret()) {
                  QList<PitchValue> points;
                  points.append(PitchValue(0,0, false));
                  points.append(PitchValue(60,(note->fret()-gn->fret())*100, false));

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
                   slur->setAnchor(Spanner::Anchor::CHORD);
                   slur->setStartChord(static_cast<Chord*>(cr1));
                   slur->setEndChord(static_cast<Chord*>(cr2));
                   slur->setTick(cr1->tick());
                   slur->setTick2(cr2->tick());
                   slur->setTrack(cr1->track());
                   slur->setTrack2(cr2->track());
                   slur->setParent(cr1);
                   score->undoAddElement(slur);
                   }
            }
      if (modMask2 & 0x1) {   // staccato - palm mute
            Chord* chord = note->chord();
            Articulation* a = new Articulation(chord->score());
            a->setArticulationType(ArticulationType::Staccato);
            chord->add(a);
            }
      if (modMask2 & 0x2) {   // palm mute - mute the whole column
            }
      if (modMask2 & 0x4) {    // tremolo picking length
            int tremoloDivision = readUChar();
            Chord* chord = note->chord();
            Tremolo* t = new Tremolo(chord->score());
            if (tremoloDivision == 1) {
                  t->setTremoloType(TremoloType::R8);
                  chord->add(t);
                  }
            else if (tremoloDivision == 2) {
                  t->setTremoloType(TremoloType::R16);
                  chord->add(t);
                  }
            else if (tremoloDivision == 3) {
                  t->setTremoloType(TremoloType::R32);
                  chord->add(t);
                  }
            else
                  qDebug("Unknown tremolo value");
      }
      if (modMask2 & 0x8)
            readUChar();      // slide kind
      if (modMask2 & 0x10)
            readArtificialHarmonic();
      if (modMask2 & 0x20) {
            readUChar();      // trill fret
            int period = readUChar();      // trill length

            // add the trill articulation to the note
            Articulation* art = new Articulation(note->score());
            art->setArticulationType(ArticulationType::Trill);
            if (!note->score()->addArticulation(note, art))
                  delete art;

            switch(period) {
                  case 1:           // 16
                        break;
                  case 2:           // 32
                        break;
                  case 3:           // 64
                        break;
                  default:
                        qDebug("unknown trill period %d", period);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

void GuitarPro5::readNote(int string, Note* note)
      {
      uchar noteBits = readUChar();
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

      if (noteBits & 0x04) {
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      if (noteBits & 0x20) {
            uchar noteType = readUChar();
            if (noteType == 1) {
                  }
            else if (noteType == 2) {
                  tieNote = true;
                  }
            else if (noteType == 3) {                 // dead notes
                  note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                  note->setGhost(true);
                  }
            else
                  qDebug("unknown note type: %d", noteType);
            }

      if (noteBits & 0x10) {          // velocity
            int d = readChar();
            if (previousDynamic != d) {
                  previousDynamic = d;
                  addDynamic(note, d);
                  }
            }

      int fretNumber = 0;
      if (noteBits & 0x20)
            fretNumber = readChar();

      if (noteBits & 0x80) {              // fingering
            int a = readUChar();
            int b = readUChar();
            qDebug("   Fingering=========%d %d", a, b);
            }
      if (noteBits & 0x1)
            skip(8);

      // check if a note is supposed to be accented, and give it the marcato type
      if (noteBits & 0x02) {
            Articulation* art = new Articulation(note->score());
            art->setArticulationType(ArticulationType::Marcato);
            if (!note->score()->addArticulation(note, art))
                  delete art;
      }

      // check if a note is supposed to be accented, and give it the sforzato type
      if (noteBits & 0x40) {
            Articulation* art = new Articulation(note->score());
            art->setArticulationType(ArticulationType::Sforzatoaccent);
            if (!note->score()->addArticulation(note, art))
                  delete art;
            }

      readUChar(); //skip

      Staff* staff = note->staff();
      if (fretNumber == 255) {
            fretNumber = 0;
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setGhost(true);
            }
      int pitch = staff->part()->instr()->stringData()->getPitch(string, fretNumber);
      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      // This function uses string and fret number, so it should be set before this
      if (noteBits & 0x8) {
            readNoteEffects(note);
            }
      if (tieNote) {
            bool found = false;
            Chord* chord     = note->chord();
            Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
            int track        = note->track();
            while (segment) {
                  Element* e = segment->element(track);
                  if (e) {
                        if (e->type() == ElementType::CHORD) {
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
                  segment = segment->prev1(SegmentType::ChordRest);
                  }
            if (!found)
                  qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            }
      }

//---------------------------------------------------------
//   readArtificialHarmonic
//---------------------------------------------------------

void GuitarPro5::readArtificialHarmonic()
      {
      int type = readChar();
      switch(type) {
            case 1:           // natural
                  break;
            case 2:           // artificial
                  skip(3);
                  break;
            case 3:           // tapped
                  skip(1);
                  break;
            case 4:           // pinch
                  break;
            case 5:           // semi
                  break;
            }
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
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro5::readBeatEffects(int track, Segment* segment)
      {
      int effects = 0;

      uchar fxBits1 = readUChar();
      uchar fxBits2 = readUChar();
      if (fxBits1 & 0x20) {
            effects = readUChar();
            // 1 - tapping
            // 2 - slapping
            // 3 - popping
            }
      if (fxBits2 & 0x04)
            readTremoloBar(track, segment);       // readBend();
      if (fxBits1 & 0x40) {
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
      if (fxBits2 & 0x02) {
            int a = readChar();            // stroke pick direction
            qDebug("  0x02: 0x%02x", a);
            }
      return effects;
      }

//---------------------------------------------------------
//   readPageSetup
//---------------------------------------------------------

void GuitarPro5::readPageSetup()
      {
      skip(version > 500 ? 49 : 30);
      for (int i = 0; i < 11; ++i) {
            skip(4);
            readBytePascalString();
            }
      }

//---------------------------------------------------------
//   readBeat
//---------------------------------------------------------

int GuitarPro5::readBeat(int tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets)
      {
      uchar beatBits = readUChar();
      bool dotted    = beatBits & 0x1;

      int pause = -1;
      if (beatBits & 0x40)
            pause = readUChar();

      // readDuration
      int len   = readChar();
      int tuple = 0;
      if (beatBits & 0x20)
            tuple = readInt();

      Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
      if (beatBits & 0x2) {
            int numStrings = score->staff(staffIdx)->part()->instr()->stringData()->strings();
            skip(17);
            QString name = readPascalString(21);
            skip(4);
            // no header to be read in the GP5 format - default to true.
            readChord(segment, staffIdx * VOICES, numStrings, name, true);
            skip(32);
            }
      Lyrics* lyrics = 0;
      if (beatBits & 0x4) {
            QString txt = readDelphiString();
            lyrics = new Lyrics(score);
            lyrics->setText(txt);
            }
      int beatEffects = 0;
      if (beatBits & 0x8)
            beatEffects = readBeatEffects(staffIdx * VOICES + voice, segment);
      if (beatBits & 0x10)
            readMixChange(measure);

      int strings = readUChar();   // used strings mask

      Fraction l    = len2fraction(len);

      // Some beat effects could add a Chord before this
      ChordRest* cr = segment->cr(staffIdx * VOICES + voice);
      if (voice != 0 && pause == 0 && strings == 0)
            cr = 0;
      else {
            if (strings == 0) {
                  if (cr) {
                        segment->remove(cr);
                        delete cr;
                        cr = 0;
                        }
                  cr = new Rest(score);
                  }
            else  {
                  if (!cr)
                        cr = new Chord(score);
                  }
            cr->setTrack(staffIdx * VOICES + voice);

            TDuration d(l);
            d.setDots(dotted ? 1 : 0);

            if (dotted)
                  l = l + (l/2);


            if (tuple) {
                  Tuplet* tuplet = tuplets[staffIdx * 2 + voice];
                  if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                        tuplet = new Tuplet(score);
                        // int track = staffIdx * 2 + voice;
                        tuplets[staffIdx * 2 + voice] = tuplet;
                        tuplet->setTrack(cr->track());
                        setTuplet(tuplet, tuple);
                        tuplet->setParent(measure);
                        }
                  tuplet->setTrack(cr->track());
                  tuplet->setBaseLen(l);
                  cr->setTuplet(tuplet);
                  tuplet->add(cr);
                  }

            cr->setDuration(l);
            if (cr->type() == ElementType::REST && pause == 0)
                  cr->setDurationType(TDuration::DurationType::V_MEASURE);
            else
                  cr->setDurationType(d);

            if(!segment->cr(staffIdx * VOICES + voice))
                  segment->add(cr);

            Staff* staff = cr->staff();
            int numStrings = staff->part()->instr()->stringData()->strings();
            for (int i = 6; i >= 0; --i) {
                  if (strings & (1 << i) && ((6-i) < numStrings)) {
                        Note* note = new Note(score);
                        static_cast<Chord*>(cr)->add(note);

                        readNote(6-i, note);
                        note->setTpcFromPitch();
                        }
                  }
            if (lyrics)
                  cr->add(lyrics);
            }
      int rr = readChar();
      if (cr && (cr->type() == ElementType::CHORD)) {
            Chord* chord = static_cast<Chord*>(cr);
            applyBeatEffects(chord, beatEffects);
            if (rr == 0x2)
                  chord->setStemDirection(Direction::DOWN);
            else if (rr == 0xa)
                  chord->setStemDirection(Direction::UP);
            else {
                  ; // qDebug("  1beat read 0x%02x", rr);
                  }
            }
      int r = readChar();
      if (r & 0x8) {
            int rrr = readChar();
qDebug("  3beat read 0x%02x", rrr);
            }
      return cr ? cr->actualTicks() : measure->ticks();
      }

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

void GuitarPro5::readMeasure(Measure* measure, int staffIdx, Tuplet** tuplets)
      {
      for (int voice = 0; voice < 2; ++voice) {
            int tick = measure->tick();
            int beats = readInt();
            for (int beat = 0; beat < beats; ++beat) {
                  tick += readBeat(tick, voice, measure, staffIdx, tuplets);
                  }
            }
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

void GuitarPro5::readMixChange(Measure* measure)
      {
      /*char patch   =*/ readChar();
      skip(16);
      char volume  = readChar();
      char pan     = readChar();
      char chorus  = readChar();
      char reverb  = readChar();
      char phase   = readChar();
      char tremolo = readChar();
      readDelphiString();                 // tempo name

      int tempo = readInt();

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
            if (version > 500)
                  readChar();
            }
      readChar();
      skip(1);
      if (version > 500) {
            readDelphiString();
            readDelphiString();
            }
      }


//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

void GuitarPro5::readTracks()
      {
      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];
            Staff* staff = score->staff(i);
            Part* part = staff->part();

            uchar c = readUChar();   // simulations bitmask
            if (c & 0x2) {                // 12 stringed guitar
                  }
            if (c & 0x4) {                // banjo track
                  }
            if (i == 0 || version == 500)
                  skip(1);
            QString name = readPascalString(40);

            int strings  = readInt();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                  throw GuitarProError::GP_BAD_NUMBER_OF_STRINGS ;
            for (int j = 0; j < strings; ++j) {
                  tuning[j] = readInt();
                  }
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readInt();
            /*int midiPort     =*/ readInt();   // -1
            int midiChannel  = readInt() - 1;
            /*int midiChannel2 =*/ readInt();   // -1

            int frets        = readInt();
            int capo         = readInt();
            /*int color        =*/ readInt();

            skip(version > 500 ? 49 : 44);
            if (version > 500) {
                  readDelphiString();
                  readDelphiString();
                  }

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            StringData stringData(frets, strings, tuning2);
            Instrument* instr = part->instr();
            instr->setStringData(stringData);
            part->setPartName(name);
            part->setLongName(name);
            instr->setTranspose(Interval(capo));

            //
            // determine clef
            //
            int patch = channelDefaults[midiChannel].patch;
            ClefType clefId = ClefType::G;
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  clefId = ClefType::PERC;
                  instr->setUseDrumset(true);
                        staff->setStaffType(StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else if (patch >= 24 && patch < 32)
                  clefId = ClefType::G3;
            else if (patch >= 32 && patch < 40)
                  clefId = ClefType::F8;
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegmentType::Clef, 0);
            segment->add(clef);
            clef->staff()->setClef(0, clef->clefTypeList());

            Channel& ch = instr->channel(0);
            if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
                  ch.program = 0;
                  ch.bank    = 128;
                  }
            else {
                  ch.program = patch;
                  ch.bank    = 0;
                  }
            ch.volume  = channelDefaults[midiChannel].volume;
            ch.pan     = channelDefaults[midiChannel].pan;
            ch.chorus  = channelDefaults[midiChannel].chorus;
            ch.reverb  = channelDefaults[midiChannel].reverb;
            // missing: phase, tremolo
            ch.updateInitList();
            }
      skip(version == 500 ? 2 : 1);
      }

//---------------------------------------------------------
//   readMeasures
//---------------------------------------------------------

void GuitarPro5::readMeasures()
      {
      Measure* measure = score->firstMeasure();
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            Tuplet* tuplets[staves * 2];     // two voices
            for (int track = 0; track < staves*2; ++track)
                  tuplets[track] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  readMeasure(measure, staffIdx, tuplets);
                  if (!(((bar == (measures-1)) && (staffIdx == (staves-1))))) {
                        /*int a = */  readChar();
                        // qDebug("    ======skip %02x", a);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro5::read(QFile* fp)
      {
      f = fp;
      readInfo();
      readLyrics();
      readPageSetup();

      int tempo = readInt();
      if (version > 500)
            skip(1);

      key    = readInt();
      /* int octave =*/ readChar();    // octave

      readChannels();
      skip(42);

      measures = readInt();
      staves  = readInt();

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            if (i > 0)
                  skip(1);
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & 0x1)
                  tnumerator = readUChar();
            if (barBits & 0x2)
                  tdenominator = readUChar();
            if (barBits & 0x4)
                  bar.repeatFlags |= Repeat::START;
            if (barBits & 0x8) {                // number of repeats
                  bar.repeatFlags |= Repeat::END;
                  bar.repeats = readUChar();
                  }
            if (barBits & 0x20) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color =*/ readInt();    // color?
                  }
            if (barBits & 0x10) {                      // a volta
                  uchar voltaNumber = readUChar();
                  while (voltaNumber > 0) {
                        // voltas are represented as flags
                        bar.volta.voltaType = GP_VOLTA_FLAGS;
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & 0x40) {
                  int currentKey = readUChar();
                  /* key signatures are specified as
                   * 1# = 1, 2# = 2, ..., 7# = 7
                   * 1b = 255, 2b = 254, ... 7b = 249 */
                  bar.keysig = currentKey <= 7 ? currentKey : -256+currentKey;
                  readUChar();        // specified major/minor mode
                  }
            if (barBits & 0x80)
                  bar.barLine = BarLineType::DOUBLE;
            if (barBits & 0x3)
                  skip(4);
            if ((barBits & 0x10) == 0)
                  skip(1);
            readChar();             // triple feel  (none, 8, 16)
            bar.timesig = Fraction(tnumerator, tdenominator);
            bars.append(bar);
            }

      //
      // create a part for every staff
      //
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, staffIdx);
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      createMeasures();
      readTracks();
      readMeasures();
      setTempo(tempo, score->firstMeasure());
      }


//---------------------------------------------------------
//   readBit
//---------------------------------------------------------

int GuitarPro6::readBit() {
      // calculate the byte index by dividing the position in bits by the bits per byte
      int byteIndex = this->position / BITS_IN_BYTE;
      // calculate our offset so we know how much to bit shift
      int byteOffset = ((BITS_IN_BYTE-1) - (this->position % BITS_IN_BYTE));
      // calculate the bit which we want to read
      int bit = ((((*buffer)[byteIndex] & 0xff) >> byteOffset) & 0x01);
      // increment our curent position so we know this bit has been read
      this->position++;
      // return the bit we calculated
      return bit;
      }


//---------------------------------------------------------
//   readBits
//---------------------------------------------------------

int GuitarPro6::readBits(int bitsToRead) {
      int bits = 0;
      for (int i = (bitsToRead-1); i>=0; i--) {
            bits |= (readBit() << i);
            }
      return bits;
      }

//---------------------------------------------------------
//   readBitsReversed
//---------------------------------------------------------

int GuitarPro6::readBitsReversed(int bitsToRead) {
      int bits = 0;
      for( int i = 0 ; i < bitsToRead ; i ++ ) {
            bits |= (readBit() << i );
            }
      return bits;
      }

//---------------------------------------------------------
//   getBytes
//---------------------------------------------------------

QByteArray GuitarPro6::getBytes(QByteArray* buffer, int offset, int length) {
      QByteArray newBytes;
      // compute new bytes from our buffer and return byte array
      for (int i = 0; i < length; i++) {
            if (buffer->length() > offset + i) {
                  newBytes.insert(i, ((*buffer)[offset+i]));
                  }
            }
      return newBytes;
      }

//---------------------------------------------------------
//   readInteger
//---------------------------------------------------------

int GuitarPro6::readInteger(QByteArray* buffer, int offset) {
      // assign four bytes and take them from the buffer
      char bytes[4];
      bytes[0] = (*buffer)[offset+0];
      bytes[1] = (*buffer)[offset+1];
      bytes[2] = (*buffer)[offset+2];
      bytes[3] = (*buffer)[offset+3];
      // increment positioning so we keep track of where we are
      this->position+=sizeof(int)*BITS_IN_BYTE;
      // bit shift in order to compute our integer value and return
      return ((bytes[3] & 0xff) << 24) | ((bytes[2] & 0xff) << 16) | ((bytes[1] & 0xff) << 8) | (bytes[0] & 0xff);
      }

//---------------------------------------------------------
//   readString
//---------------------------------------------------------

QByteArray GuitarPro6::readString(QByteArray* buffer, int offset, int length) {
      QByteArray filename;
      // compute the string by iterating through the buffer
      for (int i = 0; i < length; i++) {
            int charValue = (((*buffer)[offset + i]) & 0xff);
            if (charValue == 0)
                  break;
            filename.push_back((char)charValue);
      }
      return filename;
}

//---------------------------------------------------------
//   readGPX
//---------------------------------------------------------

void GuitarPro6::readGPX(QByteArray* buffer) {
      // start by reading the file header. It will tell us if the byte array is compressed.
      int fileHeader = readInteger(buffer, 0);

      if (fileHeader == GPX_HEADER_COMPRESSED) {
            // this is  a compressed file.
            int length = readInteger(buffer, this->position/BITS_IN_BYTE);
            QByteArray* bcfsBuffer = new QByteArray();
            int positionCounter = 0;
            while(!f->error() && (this->position/this->BITS_IN_BYTE) < length) {
                  // read the bit indicating compression information
                  int flag = this->readBits(1);

                  if (flag) {
                        int bits = this->readBits(4);
                        int offs = this->readBitsReversed(bits);
                        int size = this->readBitsReversed(bits);

                        QByteArray bcfsBufferCopy = *bcfsBuffer;
                        int pos = (bcfsBufferCopy.length() - offs );
                        for( int i = 0; i < (size > offs ? offs : size) ; i ++ ) {
                              bcfsBuffer->insert(positionCounter, bcfsBufferCopy[pos + i] ) ;
                              positionCounter++;
                              }
                        }
                  else  {
                        int size = this->readBitsReversed(2);
                        for(int i = 0; i < size; i++) {
                              bcfsBuffer->insert(positionCounter, this->readBits(8));
                              positionCounter++;
                              }
                        }
                  }
             // recurse on the decompressed file stored as a byte array
             readGPX(bcfsBuffer);
             delete bcfsBuffer;
            }
      else if (fileHeader == GPX_HEADER_UNCOMPRESSED) {
            // this is an uncompressed file - strip the header off
            *buffer = buffer->right(buffer->length()-sizeof(int));
            int sectorSize = 0x1000;
            int offset = 0;
            while ((offset = (offset + sectorSize)) + 3 < buffer->length()) {
                  int newInt = readInteger(buffer,offset);
                  if (newInt == 2) {
                        int indexFileName = (offset + 4);
                        int indexFileSize = (offset + 0x8C);
                        int indexOfBlock = (offset + 0x94);

                        // create a byte array and put information about files found in it
                        int block = 0;
                        int blockCount = 0;
                        QByteArray* fileBytes = new QByteArray();
                        while((block = (readInteger(buffer, (indexOfBlock + (4 * (blockCount ++)))))) != 0 ) {
                              fileBytes->push_back(getBytes(buffer, (offset = (block*sectorSize)), sectorSize));
                              }
                        // get file information and read the file
                        int fileSize = readInteger(buffer, indexFileSize);
                        if (fileBytes->length() >= fileSize) {
                              QByteArray filenameBytes = readString(buffer, indexFileName, 127);
                              char* filename = filenameBytes.data();
                              QByteArray data = getBytes(fileBytes, 0, fileSize);
                              parseFile(filename, &data);
                              }
                        delete fileBytes;
                        }
                  }
            }
}

//---------------------------------------------------------
//   parseFile
//---------------------------------------------------------

void GuitarPro6::parseFile(char* filename, QByteArray* data)
      {
      // test to check if we are dealing with the score
      if (!strcmp(filename, "score.gpif")) {
            QFile file("/tmp/score.gpif");
            file.open(QIODevice::WriteOnly);
            file.write(*data);
            file.close();
            readGpif(data);
            }
      }


//---------------------------------------------------------
//   unhandledNode
//---------------------------------------------------------

void GuitarPro6::unhandledNode(QString nodeName)
      {
      qDebug() << "WARNING: Discovered unhandled node name" << nodeName;
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

void GuitarPro6::readScore(QDomNode* scoreNode)
      {
      // loop through the score meta-info, grabbing title, artist, etc
      QDomNode currentNode = scoreNode->firstChild();
      while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (!nodeName.compare("Title"))
                  title = currentNode.toElement().text();
            else if (!nodeName.compare("Subtitle"))
                  subtitle = currentNode.toElement().text();
            else if (!nodeName.compare("Artist"))
                  artist = currentNode.toElement().text();
            else if (!nodeName.compare("Album"))
                  album = currentNode.toElement().text();
            else
                  unhandledNode(nodeName);
            currentNode = currentNode.nextSibling();
            }
      }

//---------------------------------------------------------
//   readMasterTracks
//---------------------------------------------------------

void GuitarPro6::readMasterTracks(QDomNode* masterTrack)
      {
      // inspects MasterTrack, gives information applying to start of score such as tempo
      QDomNode currentNode = masterTrack->firstChild();
      while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (!nodeName.compare("Automations")) {
                  QDomNode currentAutomation = currentNode.firstChild();
                  while (!currentAutomation.isNull()) {
                        if (!currentAutomation.nodeName().compare("Automation")) {
                              if (!currentAutomation.firstChild().nodeName().compare("Tempo"))
                                    tempo = currentAutomation.lastChild().toElement().text().toInt();
                              else
                                    unhandledNode(currentAutomation.nodeName());
                              }
                        else
                              unhandledNode(currentAutomation.nodeName());
                        currentAutomation = currentAutomation.nextSibling();
                        }
                  }
            currentNode = currentNode.nextSibling();
            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro6::readChord(QDomNode* diagram, int track)
      {
      // initialise a new fret diagram for our current track
      FretDiagram* fretDiagram = new FretDiagram(score);
      fretDiagram->setTrack(track);

      // get the identifier to set as the domain in the map
      int id = diagram->attributes().namedItem("id").toAttr().value().toInt();
      QDomNode diagramNode = diagram->firstChild();

      // set the number of strings on this part
      int stringCount = diagramNode.attributes().namedItem("stringCount").toAttr().value().toInt();
      fretDiagram->setStrings(stringCount);

      // set the fret offset
      int baseFret = diagramNode.attributes().namedItem("baseFret").toAttr().value().toInt();
      fretDiagram->setOffset(baseFret);

      QDomNode diagramEntity = diagramNode.firstChild();
      int counter = 0;
      while (!diagramEntity.isNull()) {
            QString nodeName = diagramEntity.nodeName();
            // new fret
            if (!nodeName.compare("Fret")) {
                  // get the string and fret numbers from the arguments to the node as integers
                  int string = diagramEntity.attributes().namedItem("string").toAttr().value().toInt();
                  int fret = diagramEntity.attributes().namedItem("fret").toAttr().value().toInt();

                  // if there are unspecified string values, add the X marker to that string
                  while (counter < string) {
                        fretDiagram->setMarker(counter, 'X');
                        counter++;
                        }

                  // look at the specified string/fret and add to diagram
                  if (fret == 0) {
                        fretDiagram->setMarker(string, '0');
                        counter++;
                        }
                  else  {
                        qDebug("Setting fret (string=%d,fret=%d)", string, fret-baseFret);
                        fretDiagram->setDot(string, fret);
                        counter++;
                        }
                  }
            // move to the next string/fret specification
            diagramEntity = diagramEntity.nextSibling();
            }

      // mark any missing strings as 'X'
      while (counter < stringCount) {
            fretDiagram->setMarker(counter, 'X');
            counter++;
            }

      // insert the fret diagram into the map of diagrams
      fretDiagrams.insert(id, fretDiagram);
      }

//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

void GuitarPro6::readTracks(QDomNode* track)
      {
      QDomNode nextTrack = track->firstChild();
      int trackCounter = 0;
      while (!nextTrack.isNull()) {
            QDomNode currentNode = nextTrack.firstChild();
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, trackCounter);
            while (!currentNode.isNull()) {
                  QString nodeName = currentNode.nodeName();
                  if (nodeName == "Name")
                        part->setLongName(currentNode.toElement().text());
                  else if (nodeName == "ShortName")
                        part->setPartName(currentNode.toElement().text());
                  else if (nodeName == "Properties") {
                        QDomNode currentProperty = currentNode.firstChild();
                        while (!currentProperty.isNull()) {
                              QString propertyName = currentProperty.attributes().namedItem("name").toAttr().value();
                              if (!propertyName.compare("Tuning")) {
                                    // set up the tuning for the part
                                    QString tuningString = currentProperty.firstChild().toElement().text();
                                    QStringList tuningStringList = tuningString.split(" ");
                                    int strings = 0;
                                    int tuning[tuningStringList.length()];
                                    int frets   = 21;
                                    for (auto iter = tuningStringList.begin(); iter != tuningStringList.end(); ++iter) {
                                          int currentString = (*iter).toInt();
                                          tuning[strings] = currentString;
                                          strings++;
                                          }
                                          StringData* stringData = new StringData(frets, strings, tuning);
                                          Instrument* instr = part->instr();
                                          instr->setStringData(*stringData);
                                    }
                              else if (!propertyName.compare("DiagramCollection")) {
                                    QDomNode items = currentProperty.firstChild();
                                    QDomNode currentItem = items.firstChild();
                                    while (!currentItem.isNull()) {
                                          readChord(&currentItem, trackCounter);
                                          currentItem = currentItem.nextSibling();
                                          }
                                    }
                              else
                                    unhandledNode(nodeName);
                              currentProperty = currentProperty.nextSibling();
                              }
                        }
                  else
                        unhandledNode(nodeName);
                  currentNode = currentNode.nextSibling();
                  }

            // add in a new part
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            trackCounter++;
            nextTrack = nextTrack.nextSibling();
            }
      // set the number of staves we need
      staves = score->staves().length();
      }

//---------------------------------------------------------
//   getNode
//---------------------------------------------------------

QDomNode GuitarPro6::getNode(QString id, QDomNode currentNode)
      {
      while (!(currentNode).isNull()) {
            QString currentId = currentNode.attributes().namedItem("id").toAttr().value();
            if (id.compare(currentId) == 0) {
                  return currentNode;
                  }
            currentNode = (currentNode).nextSibling();
            }
      qDebug("WARNING: A null node was returned when search for an identifier. Your Guitar Pro file may be corrupted.");
      return currentNode;
      }

//---------------------------------------------------------
//   findNumMeasures
//---------------------------------------------------------

int GuitarPro6::findNumMeasures(GPPartInfo* partInfo)
      {
            QDomNode finalMasterBar = partInfo->masterBars.nextSibling();
            while (!finalMasterBar.isNull()) {
                  GpBar gpBar;
                  gpBar.keysig = finalMasterBar.firstChild().toElement().text().toInt();
                  QString timeSignature = finalMasterBar.firstChild().nextSibling().toElement().text();
                  QList<QString> timeSignatureList = timeSignature.split("/");
                  gpBar.timesig = Fraction(timeSignatureList.first().toInt(), timeSignatureList.last().toInt());
                  bars.append(gpBar);
                  if (finalMasterBar.nextSibling().isNull())
                        break;
                  finalMasterBar = finalMasterBar.nextSibling();
            }
            QString bars = finalMasterBar.lastChildElement("Bars").toElement().text();
            //work out the number of measures (add 1 as couning from 0, and divide by number of parts)
            int numMeasures = (bars.split(" ").last().toInt() + 1) / score->parts().length();
            return numMeasures;
      }

//---------------------------------------------------------
//   rhythmToDuration
//---------------------------------------------------------

Fraction GuitarPro6::rhythmToDuration(QString value)
      {
            Fraction l;
            if (value.compare("Whole") == 0)
                  l.set(1, 1);
            else if (value.compare("Half") == 0)
                  l.set(1, 2);
            else if (value.compare("Quarter") == 0)
                  l.set(1, 4);
            else if (value.compare("Eighth") == 0)
                  l.set(1,8);
            else if (value.compare("16th") == 0)
                  l.set(1,16);
            else if (value.compare("32nd") == 0)
                  l.set(1,32);
            else if (value.compare("64th") == 0)
                  l.set(1,64);
            else if (value.compare("128th") == 0)
                  l.set(1,128);
            else
                  qFatal( "Error - unknown note length: %s", qPrintable(value));
            return l;
      }

//---------------------------------------------------------
//   readMasterBars
//---------------------------------------------------------

void GuitarPro6::readMasterBars(GPPartInfo* partInfo)
      {
      Measure* measure = score->firstMeasure();
      int bar = 0;
      QDomNode nextMasterBar = partInfo->masterBars;
      nextMasterBar = nextMasterBar.nextSibling();
      int measureCounter = 0;
      ClefType oldClefId[staves];
      do    {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            qDebug("--- On a new master bar ---");
            QDomNode key = nextMasterBar.firstChild();
            KeySig* t = new KeySig(score);
            t->setKey(Key(key.firstChild().toElement().text().toInt()));

            QDomNode time = key.nextSibling();
            measure->setTimesig(bars[measureCounter].timesig);
            measure->setLen(bars[measureCounter].timesig);
            QDomNode barList = time.nextSibling();
            QStringList barsString = barList.toElement().text().split(" ");
            int staffIdx = 0;
            //Tuplet* tuplets[staves * 2];     // is this necessary?
            for (auto iter = barsString.begin(); iter != barsString.end(); ++iter) {
                  int tick = measure->tick();

                  t->setTrack(0);
                  Segment* s = measure->getSegment(SegmentType::KeySig, tick);
                  s->add(t);

                  QDomNode barNode = getNode(*iter, partInfo->bars);
                  QDomNode clef = (barNode).firstChild();
                  QString clefString = clef.toElement().text();
                  QDomNode voices = clef.nextSibling();
                  QString voicesString = voices.toElement().text();
                  QDomNode xproperties = voices.nextSibling();
                  ClefType clefId = ClefType::INVALID;
                  if (!xproperties.isNull())
                        qDebug("Unsupported tag in bars: XProperties");
                  if (!clefString.compare("F4"))
                        clefId = ClefType::F8;
                  else if (!clefString.compare("G2"))
                        clefId = ClefType::G3;
                  else if (!clefString.compare("Neutral"))
                        clefId = ClefType::PERC;
                  else
                        qDebug() << "WARNING: unhandled clef type: " << clefString;
                  Clef* newClef = new Clef(score);
                  newClef->setClefType(clefId);
                  newClef->setTrack(staffIdx * VOICES);
                  Segment* segment = measure->getSegment(SegmentType::Clef, 0);
                  if (measure->prevMeasure()) {
                        if (clefId != oldClefId[staffIdx]) {
                              segment->add(newClef);
                              newClef->staff()->setClef(0, newClef->clefTypeList());
                              oldClefId[staffIdx] = clefId;
                              }
                        }
                  else  {
                        segment->add(newClef);
                        newClef->staff()->setClef(0, newClef->clefTypeList());
                        oldClefId[staffIdx] = clefId;
                        }

                  // parse the voices
                  auto currentVoice = voicesString.split(" ").first();
                  qDebug() << "WARNING: We currently only support one voice. Handling voice: " << currentVoice;
                  QDomNode voice = getNode(currentVoice, partInfo->voices);
                  int voiceNum = 0;
                  QString beats = (voice).firstChild().toElement().text();

                  // we now look at the beat in order to gain note, dynamic, rhythm etc. information
                  auto currentBeatList = beats.split(" ");
                  for (auto currentBeat = currentBeatList.begin(); currentBeat != currentBeatList.end(); currentBeat++) {
                  Fraction l;
                  int dotted = 0;
                  QDomNode beat = getNode(*currentBeat, partInfo->beats);

                  Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                  QDomNode currentNode = beat.firstChild();
                  bool noteSpecified = false;
                  ChordRest* cr = segment->cr(staffIdx * VOICES + voiceNum);
                  while (!currentNode.isNull()) {
                        if (currentNode.nodeName() == "Notes") {
                              noteSpecified = true;
                              auto notesList = currentNode.toElement().text().split(" ");

                              // this could be set by rhythm if we dealt with a tuplet
                              if (!cr)
                                    cr = new Chord(score);
                              cr->setTrack(staffIdx * VOICES + voiceNum);
                              cr->setDuration(l);
                              TDuration d(l);
                              d.setDots(dotted);
                              cr->setDurationType(d);

                              for (auto iter = notesList.begin(); iter != notesList.end(); ++iter) {
                                    // we have found a note
                                    QDomNode note = getNode(*iter, partInfo->notes);
                                    QDomNode currentNode = (note).firstChild();
                                    // if a <Notes> tag is used but there is no <Note>, then we add a rest. This flag will allow us to check this.
                                    while (!currentNode.isNull()) {
                                          if (currentNode.nodeName() == "Properties") {
                                                QDomNode currentProperty = currentNode.firstChild();
                                                // these should not be in this scope - they may not even exist.
                                                QString stringNum;
                                                QString fretNum;
                                                QString tone;
                                                QString octave;
                                                QString midi;
                                                while (!currentProperty.isNull()) {
                                                      QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                                                      if (argument == "String")
                                                            stringNum = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Element") {}
                                                      else if (argument == "Variation") {}
                                                      else if (argument == "Fret")
                                                            fretNum = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Tone")
                                                            tone = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Octave")
                                                            octave = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Midi")
                                                            midi = currentProperty.firstChild().toElement().text();
                                                      else
                                                            qDebug() << "WARNING: Not handling node argument: " << argument << "in node" << currentNode.nodeName();
                                                      currentProperty = currentProperty.nextSibling();
                                                      }

                                                if (stringNum != "") {
                                                      Note* note = new Note(score);
                                                      static_cast<Chord*>(cr)->add(note);
                                                      Staff* staff = note->staff();

                                                      int string =  stringNum.toInt();
                                                      int fretNumber = fretNum.toInt();
                                                      int musescoreString = staff->part()->instr()->stringData()->strings() - 1 - string;
                                                      auto pitch = staff->part()->instr()->stringData()->getPitch(musescoreString, fretNumber);
                                                      note->setFret(fretNumber);
                                                      // we need to turn this string number for GP to the the correct string number for musescore
                                                      note->setString(musescoreString);
                                                      note->setPitch(pitch);
                                                      note->setTpcFromPitch();
                                                      if(!segment->cr(staffIdx * VOICES + voiceNum))
                                                            segment->add(cr);
                                                      }
                                                else if (tone != "") {
                                                      Note* note = new Note(score);
                                                      static_cast<Chord*>(cr)->add(note);
                                                      int toneInt = tone.toInt();
                                                      int octaveInt = octave.toInt();
                                                      // multiply octaves by 12 as 12 semitones in octave
                                                      note->setPitch((octaveInt * 12) + toneInt);
                                                      note->setTpcFromPitch();
                                                      if(!segment->cr(staffIdx * VOICES + voiceNum))
                                                            segment->add(cr);
                                                      }
                                                else if (midi != "") {
                                                      Note* note = new Note(score);
                                                      static_cast<Chord*>(cr)->add(note);
                                                      note->setPitch(midi.toInt());
                                                      note->setTpcFromPitch();
                                                      if(!segment->cr(staffIdx * VOICES + voiceNum))
                                                            segment->add(cr);
                                                      }

                                                currentNode = currentNode.nextSibling();
                                                }
                                          else
                                                unhandledNode(currentNode.nodeName());
                                          currentNode = currentNode.nextSibling();
                                          }
                                    }
                              }
                        else if (!currentNode.nodeName().compare("Chord")) {
                              int key = currentNode.toElement().text().toInt();
                              segment->add(fretDiagrams[key]);
                              }
                        else if (currentNode.nodeName() == "Rhythm") {
                              // we have found a rhythm
                              QString refString = currentNode.attributes().namedItem("ref").toAttr().value();
                              QDomNode rhythm = getNode(refString, partInfo->rhythms);
                              QDomNode currentNode = (rhythm).firstChild();
                              while (!currentNode.isNull()) {
                                    if (currentNode.nodeName() == "NoteValue") {
                                          l = rhythmToDuration(currentNode.toElement().text());
                                          }
                                    else if (currentNode.nodeName() == "AugmentationDot") {
                                          dotted = currentNode.attributes().namedItem("count").toAttr().value().toInt();
                                          for (int count = 1; count <= dotted; count++)
                                                l = l + (l / pow(2, count));
                                    }
                                    else if (currentNode.nodeName() == "PrimaryTuplet") {
                                          qDebug("WARNING: Not handling tuplet note.");
                                          Tuplet* tuplet = new Tuplet(score);
                                          // perhaps this should be a rest too, rhythm comes before notes in score.gpif
                                          cr = new Chord(score);
                                          // pointless? tuplets[staffIdx * voiceNum] = tuplet;
                                          cr->setTrack(staffIdx * VOICES + voiceNum);
                                          tuplet->setTrack(cr->track());
                                          tuplet->setRatio(Fraction(currentNode.attributes().namedItem("num").toAttr().value().toInt(),currentNode.attributes().namedItem("den").toAttr().value().toInt()));
                                          tuplet->setParent(measure);
                                          tuplet->setBaseLen(l);
                                          cr->setTuplet(tuplet);
                                          tuplet->add(cr);
                                          }
                                    else
                                          qDebug() << "WARNING: Not handling node: " << currentNode.nodeName();
                                    currentNode = currentNode.nextSibling();
                                    }
                              }
                        qDebug() << "WARNING: Not handling beat XML tag:" << currentNode.nodeName();
                        currentNode = currentNode.nextSibling();
                        dotted = 0;
                        }
                  // we have handled the note - was there a note?
                  if (!noteSpecified) {
                        // add a rest with length of l
                        cr = new Rest(score);
                        cr->setTrack(staffIdx * VOICES + voiceNum);
                        TDuration d(l);
                        cr->setDuration(l);
                        cr->setDurationType(d);
                        if(!segment->cr(staffIdx * VOICES + voiceNum))
                              segment->add(cr);
                        }
                  tick += cr->actualTicks();
                  }
            staffIdx++;
            }
            measureCounter++;
            nextMasterBar = nextMasterBar.nextSibling();
            measure = measure->nextMeasure();
            bar++;
            } while (!nextMasterBar.isNull());
      }

//---------------------------------------------------------
//   readGpif
//---------------------------------------------------------

void GuitarPro6::readGpif(QByteArray* data)
      {
      QDomDocument qdomDoc;
      qdomDoc.setContent(*data);
      QDomElement qdomElem = qdomDoc.documentElement();
      // GPRevision node
      QDomNode revision = qdomElem.firstChild();
      // Score node
      QDomNode scoreNode = revision.nextSibling();
      readScore(&scoreNode);
      // MasterTrack node
      QDomNode masterTrack = scoreNode.nextSibling();
      readMasterTracks(&masterTrack);
      // Tracks node

      QDomNode eachTrack = masterTrack.nextSibling();
      readTracks(&eachTrack);
      // MasterBars node
      GPPartInfo partInfo;
      QDomNode masterBars = eachTrack.nextSibling();
      QDomNode bars = masterBars.nextSibling();
      QDomNode voices = bars.nextSibling();
      QDomNode beats = voices.nextSibling();
      QDomNode notes = beats.nextSibling();
      QDomNode rhythms = notes.nextSibling();
      partInfo.masterBars = masterBars.firstChild();
      partInfo.bars = bars.firstChild();
      partInfo.voices = voices.firstChild();
      partInfo.beats = beats.firstChild();
      partInfo.notes = notes.firstChild();
      partInfo.rhythms = rhythms.firstChild();

      measures = findNumMeasures(&partInfo);

      createMeasures();
      readMasterBars(&partInfo);
      setTempo(/*tempo*/120, score->firstMeasure());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro6::read(QFile* fp)
      {
      qDebug("reading guitar pro v6 file (.gpx)...");
      f = fp;
      this->buffer = new QByteArray();
      *(this->buffer) = fp->readAll();
      // decompress and read files contained within GPX file
      readGPX(this->buffer);
      delete this->buffer;
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro6::readBeatEffects(int, Segment*)
      {
      qDebug("reading beat effects (.gpx)...");
      return 0;
      }


//---------------------------------------------------------
//   importGTP
//---------------------------------------------------------

Score::FileError importGTP(Score* score, const QString& name)
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
                        return Score::FileError::FILE_BAD_FORMAT;
                        }
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
            if (m->type() != ElementType::VBOX) {
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
            s->setText(gp->title);
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
            s->setText(str);
            m->add(s);
            }
      if (!gp->composer.isEmpty()) {
            Text* s = new Text(score);
            // s->setSubtype(TEXT_COMPOSER);
            s->setTextStyleType(TextStyleType::COMPOSER);
            s->setText(gp->composer);
            m->add(s);
            }
      int idx = 0;

      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure(), ++idx) {
            const GpBar& bar = gp->bars[idx];
            if (bar.barLine != BarLineType::NORMAL)
                  m->setEndBarLineType(bar.barLine, false);
            }
      if (score->lastMeasure())
            score->lastMeasure()->setEndBarLineType(BarLineType::END, false);

      //
      // create parts (excerpts)
      //
      foreach(Part* part, score->parts()) {
            Score* pscore = new Score(score);
            pscore->style()->set(StyleIdx::createMultiMeasureRests, true);

            QList<int> stavesMap;
            Part*   p = new Part(pscore);
            p->setInstrument(*part->instr());

            Staff* staff = part->staves()->front();

            Staff* s = new Staff(pscore, p, 0);
            StaffType* st = staff->staffType();
            s->setStaffType(st);
//            int idx = pscore->staffTypeIdx(st);
//            if (idx == -1)
//                  pscore->addStaffType(st);
            s->linkTo(staff);
            p->staves()->append(s);
            pscore->staves().append(s);
            stavesMap.append(score->staffIdx(staff));
            cloneStaves(score, pscore, stavesMap);

            if (staff->part()->instr()->stringData()->strings() > 0 && part->staves()->front()->staffType()->group() == StaffGroup::STANDARD) {
                  p->setStaves(2);
                  Staff* s1 = p->staff(1);

                  StaffType st = *StaffType::preset(StaffTypes::TAB_DEFAULT);
                  st.setSlashStyle(true);
                  s1->setStaffType(&st);
                  s1->linkTo(s);
                  cloneStaff(s,s1);
                  p->staves()->front()->addBracket(BracketItem(BracketType::NORMAL, 2));
                  }
            pscore->appendPart(p);

            pscore->setName(part->partName());
            Excerpt* excerpt = new Excerpt(pscore);
            excerpt->setTitle(part->partName());
            excerpt->parts().append(part);
            score->excerpts().append(excerpt);

            if (staff->part()->instr()->stringData()->strings() > 0 && part->staves()->front()->staffType()->group() == StaffGroup::STANDARD) {
                  Staff* staff2 = pscore->staff(1);
                  staff2->setStaffType(StaffType::preset(StaffTypes::TAB_DEFAULT));
                  }

            //
            // create excerpt title
            //
            MeasureBase* measure = pscore->first();
            if (!measure || (measure->type() != ElementType::VBOX)) {
                  MeasureBase* mb = new VBox(pscore);
                  mb->setTick(0);
                  pscore->addMeasure(mb, measure);
                  measure = mb;
                  }
            Text* txt = new Text(pscore);
            txt->setTextStyleType(TextStyleType::INSTRUMENT_EXCERPT);
            txt->setText(part->longName());
            measure->add(txt);

            //
            // layout score
            //
            pscore->setPlaylistDirty(true);
            pscore->rebuildMidiMapping();
            pscore->updateChannel();
            pscore->updateNotes();

            pscore->setLayoutAll(true);
            pscore->addLayoutFlags(LayoutFlag::FIX_TICKS | LayoutFlag::FIX_PITCH_VELO);
            pscore->doLayout();
            }

//      album
//      copyright

      score->setSaved(false);
      score->setCreated(true);
      delete gp;

      score->setPlaylistDirty(true);
      score->rebuildMidiMapping();
      score->updateChannel();
      score->updateNotes();

      score->setLayoutAll(true);
      score->addLayoutFlags(LayoutFlag::FIX_TICKS | LayoutFlag::FIX_PITCH_VELO);
      score->doLayout();
      return Score::FileError::FILE_NO_ERROR;
      }
}

