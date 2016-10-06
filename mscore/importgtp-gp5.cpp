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
            score->setMetaTag("copyright", QString("%1").arg(copyright));

      transcriber  = readDelphiString();
      instructions = readDelphiString();
      int n = readInt();
      for (int i = 0; i < n; ++i)
            comments.append(readDelphiString());
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro5::readBeatEffects(int track, Segment* segment)
      {
      int effects = 0;

      uchar fxBits1 = readUChar();
      uchar fxBits2 = readUChar();
      if (fxBits1 & BEAT_FADE)
             effects = 4; // fade in
      if (fxBits1 & BEAT_EFFECT)
            effects = readUChar();
      if (fxBits2 & BEAT_TREMOLO)
            readTremoloBar(track, segment);       // readBend();
      if (fxBits1 & BEAT_ARPEGGIO) {
                  int strokeup = readUChar();            // up stroke length
                  int strokedown = readUChar();            // down stroke length

                  Arpeggio* a = new Arpeggio(score);
                  // representation is different in guitar pro 5 - the up/down order below is correct
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
            effects = readChar();            // stroke pick direction
            effects += 4; //1 or 2 for effects becomes 4 or 5
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

int GuitarPro5::readBeat(int tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets, bool /*mixChange*/)
      {
      uchar beatBits = readUChar();
      bool dotted    = beatBits & BEAT_DOTTED;

      slide = -1;
      int track = staffIdx * VOICES + voice;
      if (slides.contains(track))
            slide = slides.take(track);

      int pause = -1;
      if (beatBits & BEAT_PAUSE)
            pause = readUChar();

      // readDuration
      int len   = readChar();
      int tuple = 0;
      if (beatBits & BEAT_TUPLET)
            tuple = readInt();

      Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
      if (beatBits & BEAT_CHORD) {
            int numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
            skip(17);
            QString name = readPascalString(21);
            skip(4);
            // no header to be read in the GP5 format - default to true.
            readChord(segment, staffIdx * VOICES, numStrings, name, true);
            skip(32);
            }
      Lyrics* lyrics = 0;
      if (beatBits & BEAT_LYRICS) {
            QString txt = readDelphiString();
            lyrics = new Lyrics(score);
            lyrics->setPlainText(txt);
            }
      gpLyrics.beatCounter++;
      if (gpLyrics.beatCounter >= gpLyrics.fromBeat && gpLyrics.lyricTrack == staffIdx+1) {
            int index = gpLyrics.beatCounter - gpLyrics.fromBeat;
            if (index < gpLyrics.lyrics.size()) {
                  lyrics = new Lyrics(score);
                  lyrics->setPlainText(gpLyrics.lyrics[index]);
                  }
            }
      int beatEffects = 0;
      if (beatBits & BEAT_EFFECTS)
            beatEffects = readBeatEffects(track, segment);

      if (beatBits & BEAT_MIX_CHANGE)
            readMixChange(measure);

      int strings = readUChar();   // used strings mask

      Fraction l    = len2fraction(len);

      // Some beat effects could add a Chord before this
      ChordRest* cr = segment->cr(track);
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
            cr->setTrack(track);

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

                        hasSlur = readNote(6-i, note);
                        note->setTpcFromPitch();
                        }
                  }
            createSlur(hasSlur, staffIdx, cr);
            if (lyrics)
                  cr->add(lyrics);
            }
      int rr = readChar();
      if (cr && (cr->type() == Element::Type::CHORD)) {
            Chord* chord = static_cast<Chord*>(cr);
            applyBeatEffects(chord, beatEffects);
            if (rr == ARPEGGIO_DOWN)
                  chord->setStemDirection(Direction::DOWN);
            else if (rr == ARPEGGIO_UP)
                  chord->setStemDirection(Direction::UP);
            }
      int r = readChar();
      if (r & 0x8) {
            int rrr = readChar();
qDebug("  3beat read 0x%02x", rrr);
           }
      if (cr && (cr->type() == Element::Type::CHORD) && slide > 0)
            createSlide(slide, cr, staffIdx);
      restsForEmptyBeats(segment, measure, cr, l, track, tick);
      return cr ? cr->actualTicks() : measure->ticks();
      }

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

void GuitarPro5::readMeasure(Measure* measure, int staffIdx, Tuplet** tuplets, bool mixChange)
      {
      for (int voice = 0; voice < 2; ++voice) {
            Fraction measureLen = 0;
            int tick = measure->tick();
            int beats = readInt();
            for (int beat = 0; beat < beats; ++beat) {
                  int ticks = readBeat(tick, voice, measure, staffIdx, tuplets, mixChange);
                  tick += ticks;
                  measureLen += Fraction::fromTicks(ticks);
                  }
            if (measureLen < measure->len()) {
                  score->setRest(tick, staffIdx * VOICES + voice, measure->len() - measureLen, false, nullptr, false);
                  }
            }
      }

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

bool GuitarPro5::readMixChange(Measure* measure)
      {
      /*char patch   =*/ readChar();
      skip(16);
      signed char volume  = readChar();
      signed char pan     = readChar();
      signed char chorus  = readChar();
      signed char reverb  = readChar();
      signed char phase   = readChar();
      signed char tremolo = readChar();
      readDelphiString();                 // tempo name

      int tempo = readInt();
      bool editedTempo = false;

      if (volume >= 0)
            readChar();
      if (pan >= 0)
            readChar();
      if (chorus >= 0)
            readChar();
      if (reverb >= 0)
            readChar();
      //qDebug("read reverb: %d", reverb);
      if (phase >= 0)
            readChar();
      if (tremolo >= 0)
            readChar();
      if (tempo >= 0) {
            if (tempo != previousTempo) {
                  previousTempo = tempo;
                  setTempo(tempo, measure);
                  editedTempo = true;
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
      return editedTempo;
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
            int midiPort     = readInt() - 1;
            int midiChannel  = readInt() - 1;
            /*int midiChannel2 =*/ readInt();   // -1

            int frets        = readInt();
            int capo         = readInt();
            /*int color        =*/ readInt();

            skip(version > 500 ? 49 : 44);
            if (version > 500) {
                  //  british stack clean / amp tone
                  readDelphiString();
                  readDelphiString();
                  }

            int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            StringData stringData(frets, strings, tuning2);
            Instrument* instr = part->instrument();
            instr->setStringData(stringData);
            part->setPartName(name);
            part->setPlainLongName(name);

            //
            // determine clef
            //
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
            //qDebug("default2: %d", channelDefaults[i].reverb);
            // missing: phase, tremolo
            ch->updateInitList();
            }
      skip(version == 500 ? 2 : 1);
      }

//---------------------------------------------------------
//   readMeasures
//---------------------------------------------------------

void GuitarPro5::readMeasures(int /*startingTempo*/)
      {
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

            Tuplet* tuplets[staves * 2];     // two voices
            for (int track = 0; track < staves*2; ++track)
                  tuplets[track] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  readMeasure(measure, staffIdx, tuplets, mixChange);
                  if (!(((bar == (measures-1)) && (staffIdx == (staves-1))))) {
                        /*int a = */  readChar();
                        // qDebug("    ======skip %02x", a);
                        }
                  }
            if (bar == 1 && !mixChange)
                  setTempo(tempo, score->firstMeasure());
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

      previousDynamic = -1;
      previousTempo = -1;
      //previousDynamic = new int [staves * VOICES];
      // initialise the dynamics to 0
      //for (int i = 0; i < staves * VOICES; i++)
      //      previousDynamic[i] = 0;

      tempo = readInt();
      if (version > 500)
            skip(1);

      key    = readInt();
      /* int octave =*/ readChar();    // octave

      readChannels();
      skip(42);

      measures = readInt();
      staves  = readInt();

      slurs = new Slur*[staves];
      for (int i = 0; i < staves; ++i)
            slurs[i] = 0;

      int tnumerator   = 4;
      int tdenominator = 4;
      for (int i = 0; i < measures; ++i) {
            if (i > 0)
                  skip(1);
            GpBar bar;
            uchar barBits = readUChar();
            if (barBits & SCORE_TIMESIG_NUMERATOR)
                  tnumerator = readUChar();
            if (barBits & SCORE_TIMESIG_DENOMINATOR)
                  tdenominator = readUChar();
            if (barBits & SCORE_REPEAT_START)
                  bar.repeatFlags = bar.repeatFlags | Repeat::START;
            if (barBits & SCORE_REPEAT_END) {                // number of repeats
                  bar.repeatFlags = bar.repeatFlags |Repeat::END;
                  bar.repeats = readUChar();
                  }
            if (barBits & SCORE_MARKER) {
                  bar.marker = readDelphiString();     // new section?
                  /*int color =*/ readInt();    // color?
                  }
            if (barBits & SCORE_VOLTA) {                      // a volta
                  uchar voltaNumber = readUChar();
                  while (voltaNumber > 0) {
                        // voltas are represented as flags
                        bar.volta.voltaType = GP_VOLTA_FLAGS;
                        bar.volta.voltaInfo.append(voltaNumber & 1);
                        voltaNumber >>= 1;
                        }
                  }
            if (barBits & SCORE_KEYSIG) {
                  int currentKey = readUChar();
                  /* key signatures are specified as
                   * 1# = 1, 2# = 2, ..., 7# = 7
                   * 1b = 255, 2b = 254, ... 7b = 249 */
                  bar.keysig = currentKey <= 7 ? currentKey : -256+currentKey;
                  readUChar();        // specified major/minor mode
                  }
            if (barBits & SCORE_DOUBLE_BAR)
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
            Staff* s = new Staff(score);
            s->setPart(part);
            part->insertStaff(s, -1);
            score->staves().push_back(s);
            score->appendPart(part);
            }

      createMeasures();
      readTracks();
      readMeasures(tempo);
      }

//---------------------------------------------------------
//   readNoteEffects
//---------------------------------------------------------

bool GuitarPro5::readNoteEffects(Note* note)
      {
      uchar modMask1 = readUChar();
      uchar modMask2 = readUChar();
      bool slur = false;
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
            int gflags = readUChar();

            int grace_len = MScore::division/8;
            NoteType note_type =  NoteType::ACCIACCATURA;

            if(gflags & NOTE_APPOGIATURA) //on beat
                  note_type = NoteType::APPOGGIATURA;

            if (duration == 1)
                  grace_len = MScore::division/8; //32th
            else if (duration == 2)
                  grace_len = MScore::division/6; //24th
            else if (duration == 3)
                  grace_len = MScore::division/4; //16th

            Note* gn = new Note(score);

            if (gflags & EFFECT_GHOST) {
                  gn->setHeadGroup(NoteHead::Group::HEAD_CROSS);
                  gn->setGhost(true);
                  }
            gn->setFret(fret);
            gn->setString(note->string());
            int grace_pitch = note->staff()->part()->instrument()->stringData()->getPitch(note->string(), fret, nullptr, 0);
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
            gc->setNoteType(note_type);
            gc->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
            note->chord()->add(gc);
            addDynamic(gn, dynamic);

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
      if (modMask2 & EFFECT_STACATTO) {
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
            Tremolo* t = new Tremolo(score);
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
            if (slideKind >= 4)
                  slide = slideKind;
            else
                  slides[note->chord()->track()] = slideKind;
            }
      if (modMask2 & EFFECT_ARTIFICIAL_HARMONIC)
            readArtificialHarmonic();
      if (modMask2 & EFFECT_TRILL) {
            readUChar();      // trill fret
            int period = readUChar();      // trill length

            // add the trill articulation to the note
            Articulation* art = new Articulation(note->score());
            art->setSymId(SymId::ornamentTrill);
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
      return slur;
      }

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

bool GuitarPro5::readNote(int string, Note* note)
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

      if (noteBits & NOTE_GHOST) {
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      if (noteBits & NOTE_DEAD) {
            uchar noteType = readUChar();
            if (noteType == 1) {} //standard note
            else if (noteType == 2) {
                  tieNote = true;
                  }
            else if (noteType == 3) {                 // dead notes
                  note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
                  note->setGhost(true);
                  }
            else
                  qDebug("unknown note type: %d", noteType);
            }

      if (noteBits & NOTE_DYNAMIC) {          // velocity
            int d = readChar();
            if (previousDynamic != d) {
                  previousDynamic = d;
                  addDynamic(note, d);
                  }
            }

      int fretNumber = 0;
      if (noteBits & NOTE_FRET)
            fretNumber = readChar();

      if (noteBits & NOTE_FINGERING) {
            int leftFinger = readUChar();
            int rightFinger = readUChar();
            Fingering* f = new Fingering(score);
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

      if (noteBits & 0x1)
            skip(8);

      // check if a note is supposed to be accented, and give it the marcato type
      if (noteBits & NOTE_MARCATO) {
            Articulation* art = new Articulation(note->score());
            art->setSymId(SymId::articMarcatoAbove);
            if (!note->score()->addArticulation(note, art))
                  delete art;
      }

      // check if a note is supposed to be accented, and give it the sforzato type
      if (noteBits & NOTE_SFORZATO) {
            Articulation* art = new Articulation(note->score());
            art->setSymId(SymId::articAccentAbove);
            if (!note->score()->addArticulation(note, art))
                  delete art;
            }

      readUChar(); //skip

      Staff* staff = note->staff();
      if (fretNumber == 255) {
            fretNumber = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }
      int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber, nullptr, 0);
      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      // This function uses string and fret number, so it should be set before this
      bool slur = false;
      if (noteBits & NOTE_SLUR)
            slur = readNoteEffects(note);

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

}
