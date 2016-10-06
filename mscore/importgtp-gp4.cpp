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
#include "libmscore/instrtemplate.h"
#include "libmscore/fingering.h"
#include "libmscore/notedot.h"
#include "libmscore/stafftext.h"
#include "libmscore/sym.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

bool GuitarPro4::readMixChange(Measure* measure)
      {
      /*char patch   =*/ readChar();
      signed char volume  = readChar();
      signed char pan     = readChar();
      signed char chorus  = readChar();
      signed char reverb  = readChar();
      signed char phase   = readChar();
      signed char tremolo = readChar();
      int tempo    = readInt();

      bool tempoEdited = false;

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
            tempoEdited = true;
            }

      readChar();       // bitmask: what should be applied to all tracks
      return tempoEdited;
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro4::readBeatEffects(int track, Segment* segment)
      {
      int effects = 0;
      uchar fxBits1 = readUChar();
      uchar fxBits2 = readUChar();
      if (fxBits1 & BEAT_FADE)
            effects = 4; // fade in
      if (fxBits1 & BEAT_EFFECT) {
            effects = readUChar();      // effect 1-tapping, 2-slapping, 3-popping
            }
      if (fxBits2 & BEAT_TREMOLO)
            readTremoloBar(track,segment);
      if (fxBits1 & BEAT_ARPEGGIO) {
            int strokeup = readUChar();            // up stroke length
            int strokedown = readUChar();            // down stroke length

            Arpeggio* a = new Arpeggio(score);
            if( strokeup > 0 ) {
                  a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
                  }
            else if( strokedown > 0 ) {
                  a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
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
      if (fxBits2 & BEAT_STROKE_DIR) {
            effects = readUChar();            // stroke pick direction
            effects += 4;    //1 or 2 for effects becomes 4 or 5
            }
      if (fxBits1 & 0x01) {         // GP3 column-wide vibrato
            }
      if (fxBits1 & 0x2) {          // GP3 column-wide wide vibrato (="tremolo" in GP3)
            }
      return effects;
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

bool GuitarPro4::readNote(int string, int staffIdx, Note* note)
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

      if (noteBits & BEAT_TREMOLO) {
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      uchar variant = 1;
      if (noteBits & BEAT_EFFECT) {
            variant = readUChar();
            if (variant == 1) {     // normal note
                  }
            else if (variant == 2) {
                  /* guitar pro 4 bundles tied notes with slides in the representation
                   * we take note when we see ties and do not create slides for these notes. */
                  slides[staffIdx * VOICES] = -2;
                  tieNote = true;
                  }
            else if (variant == 3) {                 // dead notes = ghost note
                  note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
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

      // check if a note is supposed to be accented, and give it the sforzato type
      if (noteBits & NOTE_SFORZATO) {
            Articulation* art = new Articulation(note->score());
            art->setSymId(SymId::articAccentAbove);
            if (!note->score()->addArticulation(note, art))
                  delete art;
            }

      if (noteBits & NOTE_FINGERING) {
            int leftFinger = readUChar();
            int rightFinger = readUChar();
            Text* f = new Fingering(score);
            QString finger;
            // if there is a valid left hand fingering
            if (leftFinger < 5) {
                  if (leftFinger == 0)
                        finger = "T";
                  else if (leftFinger == 1)
                        finger = "1";
                  else if (leftFinger == 2)
                        finger = "2";
                  else if (leftFinger == 3)
                        finger = "3";
                  else if (leftFinger == 4)
                        finger = "4";
                  }
            else  {
                  if (rightFinger == 0)
                        finger = "T";
                  else if (rightFinger == 1)
                        finger = "I";
                  else if (rightFinger == 2)
                        finger = "M";
                  else if (rightFinger == 3)
                        finger = "A";
                  else if (rightFinger == 4)
                        finger = "O";
                  }
            f->setPlainText(finger);
            note->add(f);
            f->reset();
            }
      bool slur = false;
      if (noteBits & BEAT_EFFECTS) {
            uchar modMask1 = readUChar();
            uchar modMask2 = readUChar();
            if (modMask1 & EFFECT_BEND)
                  readBend(note);
            if (modMask1 & EFFECT_HAMMER)
                  slur = true;
            if (modMask1 & EFFECT_LET_RING)
                  addLetRing(note);
            if (modMask1 & EFFECT_GRACE) {
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
                  int grace_pitch = note->part()->instrument()->stringData()->getPitch(string, fret, nullptr, 0);
                  gn->setPitch(grace_pitch);
                  gn->setTpcFromPitch();

                  Chord* gc = new Chord(score);
                  gc->setTrack(note->chord()->track());
                  gc->add(gn);
                  gc->setParent(note->chord());

                  TDuration d;
                  d.setVal(grace_len);
                  if(grace_len == MScore::division/6)
                        d.setDots(1);
                  gc->setDurationType(d);
                  gc->setDuration(d.fraction());
                  gc->setNoteType(NoteType::ACCIACCATURA);
                  gc->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
                  note->chord()->add(gc);
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
                         slur->setAnchor(Spanner::Anchor::CHORD);
                         slur->setStartElement(cr1);
                         slur->setEndElement(cr2);
                         slur->setTick(cr1->tick());
                         slur->setTick2(cr2->tick());
                         slur->setTrack(cr1->track());
                         slur->setTrack2(cr2->track());
                         // this case specifies only two-note slurs, don't set a parent
                         score->undoAddElement(slur);
                         }
                  }
            if (modMask2 & EFFECT_STACATTO) {   // staccato
                  Chord* chord = note->chord();
                  Articulation* a = new Articulation(chord->score());
                  a->setSymId(SymId::articStaccatoAbove);
                  chord->add(a);
                  }
            if (modMask2 & EFFECT_PALM_MUTE)
                  addPalmMute(note);

            if (modMask2 & EFFECT_TREMOLO) {    // tremolo picking length
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
            if (modMask2 & EFFECT_SLIDE) {
                  int slideKind = readUChar();
                  // if slide >= 4 then we are not dealing with legato slide nor shift slide
                  if (slideKind >= 3 || slideKind == 254 || slideKind == 255) {
                        slide = slideKind;
                        }
                  else
                        slides[note->chord()->track()] = slideKind;
                  }
            if (modMask2 & 0x10)
                  readUChar();      // harmonic kind
            if (modMask2 & EFFECT_TRILL) {
                  readUChar();      // trill fret
                  readUChar();      // trill length

                  // add the trill articulation to the note
                  Articulation* art = new Articulation(note->score());
                  art->setSymId(SymId::ornamentTrill);
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
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }
      // dead note represented as high numbers - fix to zero
      if (fretNumber > 99 || fretNumber == -1)
            fretNumber = 0;
      int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber, nullptr, 0);
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
            if (!found)
                  qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            }
      return slur;
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
            score->setMetaTag("copyright", QString("%1").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      }

//---------------------------------------------------------
//   convertGP4SlideNum
//---------------------------------------------------------

int GuitarPro4::convertGP4SlideNum(int slide)
      {
      switch (slide) {
            // slide out downwards
            case 3:
                  return SLIDE_OUT_DOWN;
                  break;
            // slide out upwards
            case 4:
                  return SLIDE_OUT_UP;
                  break;
            // slide in from above
            case 254:
                  return SLIDE_IN_ABOVE;
                  break;
            // slide in from below
            case 255:
                  return SLIDE_IN_BELOW;
                  break;
            }
      return slide;
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

      //previousDynamic = new int [staves * VOICES];
      // initialise the dynamics to 0
      //for (int i = 0; i < staves * VOICES; i++)
      //      previousDynamic[i] = 0;
      previousDynamic = -1;
      previousTempo = -1;

      readChannels();
      measures = readInt();
      staves   = readInt();

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
                  while (voltaNumber > 0) {
                        // volta information is represented as a binary number
                        bar.volta.voltaType = GP_VOLTA_BINARY;
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & SCORE_MARKER) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color = */ readInt();    // color?
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

      slurs = new Slur*[staves];
      for (int i = 0; i < staves; ++i)
            slurs[i] = 0;
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
                  int tick  = measure->tick();
                  int beats = readInt();
                  int track = staffIdx * VOICES;
                  for (int beat = 0; beat < beats; ++beat) {
                        slide = -1;
                        if (slides.contains(track))
                              slide = slides.take(track);

                        uchar beatBits = readUChar();
                        bool dotted = beatBits & 0x1;
                        int pause = -1;
                        if (beatBits & BEAT_PAUSE)
                              pause = readUChar();
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
                        if (beatBits & BEAT_LYRICS) {
                              lyrics = new Lyrics(score);
                              lyrics->setPlainText(readDelphiString());
                              }
                        gpLyrics.beatCounter++;
                        if (gpLyrics.beatCounter >= gpLyrics.fromBeat && gpLyrics.lyricTrack == staffIdx + 1) {
                              int index = gpLyrics.beatCounter - gpLyrics.fromBeat;
                              if (index < gpLyrics.lyrics.size()) {
                                    lyrics = new Lyrics(score);
                                    lyrics->setPlainText(gpLyrics.lyrics[index]);
                                    }
                              }
                        int beatEffects = 0;
                        if (beatBits & BEAT_EFFECTS)
                              beatEffects = readBeatEffects(track, segment);
                        if (beatBits & BEAT_MIX_CHANGE) {
                              readMixChange(measure);
                              mixChange = true;
                              }
                        else  {
                              if (bar == 0)
                                    setTempo(tempo, measure);
                              }


                        int strings = readUChar();   // used strings mask
                        Fraction l  = len2fraction(len);

                        // Some beat effects could add a Chord before this
                        ChordRest* cr = segment->cr(track);

                        if (strings == 0) {
                              if(segment->cr(track)){
                                    segment->remove(segment->cr(track));
                                    delete cr;
                                    cr = 0;
                                    }
                              cr = new Rest(score);
                              }
                        else {
                              if(!segment->cr(track))
                                    cr = new Chord(score);
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
                        if (cr->type() == Element::Type::REST && (pause == 0 || l >= measure->len())) {
                              cr->setDurationType(TDuration::DurationType::V_MEASURE);
                              cr->setDuration(measure->len());
                              }
                        else
                              cr->setDurationType(d);
                        if(!segment->cr(track))
                              segment->add(cr);
                        Staff* staff = cr->staff();
                        int numStrings = staff->part()->instrument()->stringData()->strings();
                        bool hasSlur = false;

                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    // apply dotted notes to the note
                                    if (dotted) {
                                          // there is at most one dotted note in this guitar pro version
                                          NoteDot* dot = new NoteDot(score);
                                          // dot->setIdx(0);
                                          dot->setParent(note);
                                          dot->setTrack(track);  // needed to know the staff it belongs to (and detect tablature)
                                          dot->setVisible(true);
                                          note->add(dot);
                                          }
                                    static_cast<Chord*>(cr)->add(note);

                                    hasSlur = readNote(6-i, staffIdx, note);
                                    note->setTpcFromPitch();
                                    }
                              }
                        if (cr && (cr->type() == Element::Type::CHORD))
                              applyBeatEffects(static_cast<Chord*>(cr), beatEffects);

                        // if we see that a tied note has been constructed do not create the tie
                        if (slides[track] == -2) {
                              slide = 0;
                              slides[track] = -1;
                        }
                        if (hasSlur && (slurs[staffIdx] == 0)) {
                              Slur* slur = new Slur(score);
                              slur->setParent(0);
                              slur->setTrack(track);
                              slur->setTrack2(track);
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
                        if (cr && (cr->type() == Element::Type::CHORD) && slide > 0)
                              createSlide(convertGP4SlideNum(slide), cr, staffIdx);
                        restsForEmptyBeats(segment, measure, cr, l, track, tick);
                        createSlur(hasSlur, staffIdx, cr);
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

}
