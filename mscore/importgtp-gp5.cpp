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
            score->setMetaTag("copyright", QString("Copyright %1\nAll Rights Reserved - International Copyright Secured").arg(copyright));

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
      if (fxBits1 & 0x10)
             effects = 4; // fade in
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

      slide = -1;
      int track = staffIdx * VOICES + voice;
      if (slides.contains(track))
            slide = slides.take(track);


      int pause = -1;
      if (beatBits & 0x40)
            pause = readUChar();

      // readDuration
      int len   = readChar();
      int tuple = 0;
      if (beatBits & 0x20)
            tuple = readInt();

      Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
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
            beatEffects = readBeatEffects(track, segment);
      if (beatBits & 0x10)
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
                  cr->setTuplet(tuplet);
                  tuplet->add(cr);
                  }

            cr->setDuration(l);
            if (cr->type() == Element::Type::REST && pause == 0)
                  cr->setDurationType(TDuration::DurationType::V_MEASURE);
            else
                  cr->setDurationType(d);

            if(!segment->cr(track))
                  segment->add(cr);

            Staff* staff = cr->staff();
            int numStrings = staff->part()->instr()->stringData()->strings();
            bool hasSlur = false;
            for (int i = 6; i >= 0; --i) {
                  if (strings & (1 << i) && ((6-i) < numStrings)) {
                        Note* note = new Note(score);
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
            if (rr == 0x2)
                  chord->setStemDirection(MScore::Direction::DOWN);
            else if (rr == 0xa)
                  chord->setStemDirection(MScore::Direction::UP);
            else {
                  ; // qDebug("  1beat read 0x%02x", rr);
                  }
            }
      int r = readChar();
      if (r & 0x8) {
            int rrr = readChar();
qDebug("  3beat read 0x%02x", rrr);
            }
      if (slide)
            createSlide(slide, cr, staffIdx);
      restsForEmptyBeats(segment, measure, cr, l, track, tick);
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
            Segment* segment = measure->getSegment(Segment::Type::Clef, 0);
            segment->add(clef);

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
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
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

      previousDynamic = -1;
      previousTempo = -1;
      int tempo = readInt();
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
            if (barBits & 0x1)
                  tnumerator = readUChar();
            if (barBits & 0x2)
                  tdenominator = readUChar();
            if (barBits & 0x4)
                  bar.repeatFlags = bar.repeatFlags | Repeat::START;
            if (barBits & 0x8) {                // number of repeats
                  bar.repeatFlags = bar.repeatFlags |Repeat::END;
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
//   readNoteEffects
//---------------------------------------------------------

bool GuitarPro5::readNoteEffects(Note* note)
      {
      uchar modMask1 = readUChar();
      uchar modMask2 = readUChar();
      bool slur = false;
      if (modMask1 & 0x1)
            readBend(note);
      if (modMask1 & 0x2)         // hammer on / pull off
            slur = true;
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
                  gn->setHeadGroup(NoteHead::Group::HEAD_CROSS);
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
      if (modMask2 & 0x8) {
            int slideKind = readUChar();
            // if slide >= 4 then we are not dealing with legato slide nor shift slide
            if (slideKind >= 4)
                  slide = slideKind;
            else
                  slides[note->chord()->track()] = slideKind;
            }
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

      if (noteBits & 0x04) {
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
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
                  note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
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
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }
      int pitch = staff->part()->instr()->stringData()->getPitch(string, fretNumber);
      note->setFret(fretNumber);
      note->setString(string);
      note->setPitch(pitch);

      // This function uses string and fret number, so it should be set before this
      bool slur = false;
      if (noteBits & 0x8)
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
