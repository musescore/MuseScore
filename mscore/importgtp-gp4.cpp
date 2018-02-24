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
#include <libmscore/score.h>
#include <libmscore/measurebase.h>
#include <libmscore/text.h>
#include <libmscore/box.h>
#include <libmscore/staff.h>
#include <libmscore/part.h>
#include <libmscore/measure.h>
#include <libmscore/timesig.h>
#include <libmscore/tremolo.h>
#include <libmscore/chordline.h>
#include <libmscore/glissando.h>
#include <libmscore/rest.h>
#include <libmscore/chord.h>
#include <libmscore/note.h>
#include <libmscore/stringdata.h>
#include <libmscore/clef.h>
#include <libmscore/lyrics.h>
#include <libmscore/tempotext.h>
#include <libmscore/slur.h>
#include <libmscore/tie.h>
#include <libmscore/tuplet.h>
#include <libmscore/barline.h>
#include <libmscore/excerpt.h>
#include <libmscore/stafftype.h>
#include <libmscore/bracket.h>
#include <libmscore/articulation.h>
#include <libmscore/keysig.h>
#include <libmscore/harmony.h>
#include <libmscore/bend.h>
#include <libmscore/tremolobar.h>
#include <libmscore/segment.h>
#include <libmscore/rehearsalmark.h>
#include <libmscore/dynamic.h>
#include <libmscore/arpeggio.h>
#include <libmscore/volta.h>
#include <libmscore/instrtemplate.h>
#include <libmscore/fingering.h>
#include <libmscore/notedot.h>
#include <libmscore/stafftext.h>
#include <libmscore/sym.h>
#include <libmscore/instrtemplate.h>

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
	      if (last_segment) {
		      score->setTempo(last_segment->tick(), double(tempo) / 60.0f);
			last_segment = nullptr;
			}
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
      //   80 - Right hand or left hand fingering;
      //   40 - Accentuated note
      //   20 - Note type (rest, empty note, normal note);
      //   10 - note dynamic;
      //    8 - Presence of effects linked to the note;
      //    4 - Ghost note;
      //    2 - Dotted note;  ?
      //    1 - Time-independent duration

      if (noteBits & BEAT_TREMOLO) {
            //note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
            note->setGhost(true);
            }

      bool tieNote = false;
      uchar variant = 1;
      if (noteBits & BEAT_EFFECT) {       // 0x20
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
            qDebug("          Time independent note len, len %d t %d", a, b);
            }
      if (noteBits & 0x2) {               // note is dotted
            //readUChar();
            }

      // set dynamic information on note if different from previous note
      if (noteBits & NOTE_DYNAMIC) {            // 0x10
            int d = readChar();
            if (previousDynamic != d) {
                  previousDynamic = d;
                  // addDynamic(note, d);    // velocity? TODO-ws ??
                  }
            }
      else if (previousDynamic) {
            previousDynamic = 0;
		//addDynamic(note, 0);
            }

      int fretNumber = -1;
      if (noteBits & NOTE_FRET) {                 // 0x20
            // TODO: special case if note is tied
            fretNumber = readUChar();
            }

      // check if a note is supposed to be accented, and give it the sforzato type
      if (noteBits & NOTE_SFORZATO) {           // 0x40
            Articulation* art = new Articulation(note->score());
            art->setSymId(SymId::articAccentAbove);
            if (!note->score()->addArticulation(note, art))
                  delete art;
            }

      if (noteBits & NOTE_FINGERING) {          // 0x80
            int leftFinger  = readUChar();
            int rightFinger = readUChar();
            Fingering* f    = new Fingering(score);
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
	uchar modMask2{ 0 };
      if (noteBits & BEAT_EFFECTS) {
            uchar modMask1 = readUChar();
            modMask2 = readUChar();
		if (modMask1 & EFFECT_BEND) {
                  readBend(note);
			}
            if (modMask1 & EFFECT_HAMMER) {
                  slur = true;
			}
            if (modMask1 & EFFECT_LET_RING) {
                  addLetRing(note);
			//note->setLetRing(true);
			}
            if (modMask2 & 0x40)
                  addVibrato(note);
            if (modMask1 & EFFECT_GRACE) {
                  int fret = readUChar();             // grace fret
                  int dynamic = readUChar();          // grace dynamic
                  int transition = readUChar();       // grace transition
                  int duration = readUChar();         // grace duration

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
                  gc->setMag(note->chord()->staff()->mag(0) * score->styleD(StyleIdx::graceNoteMag));
                  note->chord()->add(gc);
                  addDynamic(gn, dynamic);

                  if (transition == 0) {
                        // no transition
                        }
                  else if(transition == 1) {
                        //TODO: Add a 'slide' guitar effect when implemented
//TODO-ws				note->setSlideNote(gn);
                        }
                  else if (transition == 2 && fretNumber>=0 && fretNumber<=255 && fretNumber!=gn->fret()) {
#if 0
                        QList<PitchValue> points;
                        points.append(PitchValue(0,0, false));
                        points.append(PitchValue(60,(fretNumber-gn->fret())*100, false));

                        Bend* b = new Bend(note->score());
                        b->setPoints(points);
                        b->setTrack(gn->track());
                        gn->add(b);
#endif
                        }
                   else if (transition == 3) {
                         // TODO:
                         //     major: replace with a 'hammer-on' guitar effect when implemented
                         //     minor: make slurs for parts

                         ChordRest* cr1 = toChord(gc);
                         ChordRest* cr2 = toChord(note->chord());

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
            if (modMask2 & EFFECT_PALM_MUTE) {
                  //note->setPalmMute(true);
			addPalmMute(note);
			}

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
					  //slides[note->chord()->track()] = slideKind;
					  slideList.push_back(note);
				  if (slideKind == 2)
					  slur = true;
                  }

            if (modMask2 & EFFECT_TRILL) {
                  /* unsigned char a = */ readUChar();
//TODO-ws                  note->setTrillFret(a);      // trill fret
                  /*int period =*/ readUChar();      // trill length

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
      note->setPitch(std::min(pitch, 127));

      if (modMask2 & 0x10) {
	      int type = readUChar();      // harmonic kind
		if (type == 1) //Natural
		      ; // TODO-ws note->setHarmonic(false);
		else if (type == 3) // Tapped
			addTextToNote("T.H.", Align::CENTER, note);
		else if (type == 4) //Pinch
			addTextToNote("P.H.", Align::CENTER, note);
		else if (type == 5) //semi
			addTextToNote("S.H.", Align::CENTER, note);
		else { //Artificial
		      addTextToNote("A.H.", Align::CENTER, note);
			int harmonicFret = note->fret();
			harmonicFret += type - 10;
			Note* harmonicNote = new Note(score);
			note->chord()->add(harmonicNote);
			harmonicNote->setFret(harmonicFret);
			harmonicNote->setString(note->string());
//TODO-ws			harmonicNote->setHarmonic(true);
			harmonicNote->setPitch(note->staff()->part()->instrument()->stringData()->getPitch(note->string(), harmonicFret, nullptr, 0));
			harmonicNote->setTpcFromPitch();
                  }
            }

      if (tieNote) {
	      int staffIdx = note->staffIdx();
		if (slurs[staffIdx]) {
                  score->removeSpanner(slurs[staffIdx]);
			delete slurs[staffIdx];
			slurs[staffIdx] = 0;
		      }
            bool found = false;
		Chord* chord = note->chord();
		Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
		int track = note->track();
		std::vector<ChordRest*> chords;
		Note* true_note = 0;
		while (segment) {
                  Element* e = segment->element(track);
			if (e) {
			      if (e->isChord()) {
				      Chord* chord2 = toChord(e);
					foreach (Note* note2, chord2->notes()) {
					      if (note2->string() == string) {
						      if (chords.empty()) {
                                                Tie* tie = new Tie(score);
								tie->setEndNote(note);
								note2->add(tie);
							      }
                                          note->setFret(note2->fret());
							note->setPitch(note2->pitch());
							true_note = note2;
							found = true;
                                          break;
						      }
					      }
				      }
                        if (found)
				      break;
                        else {
				      if (e)
                                    chords.push_back(toChordRest(e));
                              }
			      }
                  segment = segment->prev1(SegmentType::ChordRest);
                  }
            if (true_note && chords.size()) {
                  Note* end_note = note;
			for (unsigned int i = 0; i < chords.size(); ++i) {
                        Chord* chord = nullptr;
				auto cr = chords.at(i);
				if (cr->isChord())
                              chord = toChord(cr);
                        else {
				      Rest* rest = toRest(cr);
					auto dur = rest->duration();
					auto dut = rest->durationType();
					auto seg = rest->segment();
					seg->remove(rest);
					auto tuplet = rest->tuplet();
					if (tuplet)
                                    tuplet->remove(rest);
                              delete rest;
					chord = new Chord(score);
					chord->setTrack(note->track());
					chord->setDuration(dur);
					chord->setDurationType(dut);
					seg->add(chord);
					if (tuplet)
                                    tuplet->add(chord);
                              }

                        Note* note2 = new Note(score);
				note2->setString(true_note->string());
				note2->setFret(true_note->fret());
				note2->setPitch(true_note->pitch());
				note2->setTpcFromPitch();
				chord->setNoteType(true_note->noteType());
				chord->add(note2);
				Tie* tie = new Tie(score);
				tie->setEndNote(end_note);
//TODO-ws			end_note->setHarmonic(true_note->harmonic());
				end_note = note2;
				note2->add(tie);
                        }
                  Tie* tie = new Tie(score);
			tie->setEndNote(end_note);
//TODO-ws		end_note->setHarmonic(true_note->harmonic());
                  true_note->add(tie);
                  }
            if (!found) {
		      qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
                  return false;
                  }
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
            case 1:
		      return SHIFT_SLIDE;
		case 2:
			return LEGATO_SLIDE;
            case 3:     // slide out downwards
                  return SLIDE_OUT_DOWN;
            case 4:     // slide out upwards
                  return SLIDE_OUT_UP;
            case 254:   // slide in from above
                  return SLIDE_IN_ABOVE;
            case 255:   // slide in from below
                  return SLIDE_IN_BELOW;
            }
      return slide;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro4::read(QFile* fp)
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

	curDynam.resize(staves * VOICES);
	for (auto& i : curDynam)
            i = -1;

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
                  bar.repeats = readUChar() + 1;
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

      setTempo(tempo, score->firstMeasure());

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
                  return false;
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

			std::vector<int> tuning2(strings);
            //int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            StringData stringData(frets, strings, &tuning2[0]);
			createTuningString(strings, &tuning2[0]);
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
                  staff->setStaffType(0, StaffType::preset(StaffTypes::PERC_DEFAULT));
                  }
            else
                  clefId = defaultClef(patch);
            Measure* measure = score->firstMeasure();
            Clef* clef = new Clef(score);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            Segment* segment = measure->getSegment(SegmentType::HeaderClef, 0);
            segment->add(clef);

            if (capo > 0) {
                  Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  StaffText* st = new StaffText(score);
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
	tupleKind.resize(staves);
	for (auto& i : tupleKind)
	      i = 0;
      for (int i = 0; i < staves; ++i)
            slurs[i] = 0;

      Measure* measure = score->firstMeasure();
      bool mixChange = false;
	bool lastSlurAdd = false;
      for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
		  if (!f->isReadable())
			  break;
            const GpBar& gpbar = bars[bar];
            if (!gpbar.marker.isEmpty()) {
                  RehearsalMark* s = new RehearsalMark(score);
                  s->setPlainText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

            std::vector<Tuplet*> tuplets(staves);
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
                  tuplets[staffIdx] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  Fraction measureLen = 0;
                  int tick  = measure->tick();
                  int beats = readInt();
                  int track = staffIdx * VOICES;

                  if (!f->isReadable())
                        break;
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
				if (beatBits & BEAT_TUPLET) {
                              tuple = readInt();
#if 0 // TODO: ws
					if (tupleKind[staffIdx])
                                    --tupleKind[staffIdx];
                              else {
                                    tupleKind[staffIdx] = tuple - 1;
						curTuple = tuple;
						}
#endif
				      }
                        else if (tupleKind[staffIdx]) {
//TODO: ws                              tuple = curTuple;
//					--tupleKind[staffIdx];
					}
                        Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
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
					auto  str = readDelphiString();
//TODO-ws					str.erase(std::remove_if(str.begin(), str.end(), [](char c){return c == '_'; }), str.end());
					lyrics->setPlainText(str);
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
						last_segment = segment;
                        if (beatBits & BEAT_MIX_CHANGE) {
                              readMixChange(measure);
                              mixChange = true;
                              }
#if 0
                        else  {
                              if (bar == 0)
                                    setTempo(tempo, measure);
                              }
#endif
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
                        else
                              tuplets[staffIdx] = 0;  // needed?

                        cr->setDuration(l);
                        if (cr->isRest() && (pause == 0 || l >= measure->len())) {
                              cr->setDurationType(TDuration::DurationType::V_MEASURE);
                              cr->setDuration(measure->len());
                              }
                        else
                              cr->setDurationType(d);
                        if (!segment->cr(track))
                              segment->add(cr);
                        Staff* staff   = cr->staff();
                        int numStrings = staff->part()->instrument()->stringData()->strings();
                        bool hasSlur   = false;
				int dynam      = -1;
if (cr && cr->isChord()) {    // TODO::ws  crashes without if
                        for (int i = 6; i >= 0; --i) {
                              if (strings & (1 << i) && ((6-i) < numStrings)) {
                                    Note* note = new Note(score);
                                    // apply dotted notes to the note
                                    if (dotted) {
                                          // there is at most one dotted note in this guitar pro version
                                          NoteDot* dot = new NoteDot(score);
                                          dot->setParent(note);
                                          dot->setTrack(track);  // needed to know the staff it belongs to (and detect tablature)
                                          dot->setVisible(true);
                                          note->add(dot);
                                          }
                                    toChord(cr)->add(note);

                                    hasSlur = readNote(6-i, staffIdx, note);
						dynam = std::max(dynam, previousDynamic);
                                    note->setTpcFromPitch();
                                    }
                              }
} //ws
                        if (cr && cr->isChord()) {
                              applyBeatEffects(toChord(cr), beatEffects);
					if (dynam != curDynam[track]) {
                                    curDynam[track] = dynam;
						addDynamic(toChord(cr)->notes().front(), dynam);
						}
				      }

                        // if we see that a tied note has been constructed do not create the tie
                        if (slides[track] == -2) {
                              slide = 0;
                              slides[track] = -1;
                              }
                        bool slurSwap = true;
				if (slide != 2) {
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
						if (cr->isChord()) {
                                          lastSlurAdd = true;
                                          slurSwap = false;
                                          }
//TODO-ws                           cr->has_slur = true;
						}
					else if (slurs[staffIdx] && hasSlur) {
					      }
                              }
                        if (cr && (cr->isChord()) && slide > 0) {
                              auto chord = toChord(cr);
					auto effect = convertGP4SlideNum(slide);
					if (slide > 2)
					      createSlide(convertGP4SlideNum(slide), cr, staffIdx);
                              if (slide < 3 || effect == SLIDE_OUT_UP) {
                                    Note* last = chord->upNote();
						auto seg = chord->segment();
						Measure* mes = seg->measure();
						while ((seg = seg->prev()) || (mes = mes->prevMeasure())) {
                                          if (!seg)
                                                break;//seg = mes->last();
                                          if (seg->segmentType() == SegmentType::ChordRest) {
                                                bool br = false;
								Chord* cr = toChord(seg->cr(chord->track()));
								if (cr)
								      for (auto n : cr->notes()) {
									      if (n->string() == last->string()) {
                                                                  Glissando* s = new Glissando(score);
											s->setAnchor(Spanner::Anchor::NOTE);
											s->setStartElement(n);
											s->setTick(seg->tick());
											s->setTrack(chord->track());
											s->setParent(n);
											s->setGlissandoType(GlissandoType::STRAIGHT);
											s->setEndElement(last);
											s->setTick2(chord->segment()->tick());
											s->setTrack2(chord->track());
											score->addElement(s);
											if (slide == 2 || effect == SLIDE_OUT_UP) {
										      	if (!lastSlurAdd) {
											      	createSlur(true, chord->staffIdx(), cr);
												      createSlur(false, chord->staffIdx(), chord);
      											      }
	      									      }
                                                                  }
                                                            br = true;
										break;
									      }
									if (br)
                                                            break;
									}
								}
							}
						}
				if (slurSwap)
                              lastSlurAdd = false;
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

      for (auto n : slideList) {
		Segment* segment = n->chord()->segment();
		Measure* measure = segment->measure();
		int segment_counter = 0;
		while ((segment = segment->next1(SegmentType::ChordRest)) || ((measure = measure->nextMeasure()) && (segment = measure->first()))) {
                  if (!segment->isChordRestType())
                        continue;
                  bool br = false;
			ChordRest* cr = segment->cr(n->track());
                  if (cr && cr->isChord()) {
                        Chord* c = toChord(cr);
                        ++segment_counter;
				if (segment_counter > 2)
				      break;
                        for (Note* nt : c->notes()) {
                              if (nt->string() == n->string()) {
                                    for (auto e : nt->el()) {
						      if (e->isChordLine()) {
                                                auto cl = toChordLine(e);
                                                if (cl->chordLineType() == ChordLineType::PLOP || cl->chordLineType() == ChordLineType::SCOOP) {
								      br = true;
							            break;
      								}
	      					      }
		      				}
                                    }
			            if (br)
                                    break;
#if 1  // TODO-ws: crash
                              Glissando* s = new Glissando(score);
		      		s->setAnchor(Spanner::Anchor::NOTE);
			      	s->setStartElement(n);
				      s->setTick(n->chord()->segment()->tick());
      				s->setTrack(n->track());
	      			s->setParent(n);
		      		s->setGlissandoType(GlissandoType::STRAIGHT);
			      	s->setEndElement(nt);
				      s->setTick2(nt->chord()->segment()->tick());
      				s->setTrack2(n->track());
	      			score->addElement(s);
#endif
		      		br = true;
			      	break;
				      }
      		      if (br)
                              break;
		            }
		      }
	      }

      return true;
      }

}
