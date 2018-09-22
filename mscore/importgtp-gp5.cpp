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
#include <libmscore/rest.h>
#include <libmscore/chord.h>
#include <libmscore/note.h>
#include <libmscore/stringdata.h>
#include <libmscore/clef.h>
#include <libmscore/lyrics.h>
#include <libmscore/tempotext.h>
#include <libmscore/glissando.h>
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
#include <libmscore/chordline.h>


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
      if (fxBits1 & BEAT_EFFECT) {
            int k = readUChar();
		effects = k + effects * 100;// &effects;
	      }
      if (fxBits1 & BEAT_VIBRATO_TREMOLO) {
            effects = 7 + effects * 100;
            }
      if (fxBits2 & BEAT_TREMOLO)
            readTremoloBar(track, segment);       // readBend();
      if (fxBits2 & 0x01) { // Rasgueado effect
            StaffText* st = new StaffText(score);
		st->setXmlText("rasg.");
		st->setParent(segment);
		st->setTrack(track);
		score->addElement(st);
	      }
      if (fxBits1 & BEAT_ARPEGGIO) {
            int strokeup = readUChar();            // up stroke length
            int strokedown = readUChar();            // down stroke length

            Arpeggio* a = new Arpeggio(score);
            // representation is different in guitar pro 5 - the up/down order below is correct
            if (strokeup > 0) {
                  a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
                  }
            else if (strokedown > 0) {
                  a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
                  }
            else {
                  delete a;
                  a = 0;
                  }
            if (a) {
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

int GuitarPro5::readBeat(int tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets,
   bool /*mixChange*/)
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

      Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
      if (beatBits & BEAT_CHORD) {
            int numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
            skip(17);
#if 0
            uchar header = readUChar(); //1
		uchar sharp = readUChar(); //2
		skip(3); //5
		char root = readChar(); //6
		uchar chordtype = readUChar();//7
		uchar nines = readUChar();//8
		int bass = readInt();//12
		int aug = readInt(); //16
		char add = readChar(); //17
#endif
            QString name;
            {
		      /*auto len =*/ readUChar();
			char c[21];
			f->read(c, 21);
			// if (len > 20)
			//      skip(len - 20);
                  //skip(len - 20);
			c[20] = 0;
			name = c;
			}
            //QString name = readPascalString(21);
            skip(4);
            // no header to be read in the GP5 format - default to true.
            readChord(segment, staffIdx * VOICES, numStrings, name, true);
            skip(32);
            }
      Lyrics* lyrics = 0;
      QString free_text;
      if (beatBits & BEAT_LYRICS) {
            //free_text = readDelphiString();
		QString qs = readDelphiString();
            std::string txt = qs.toUtf8().constData();
     	      txt.erase(std::remove_if(txt.begin(), txt.end(), [](char c) {return c == '_'; }), txt.end());
//		auto pos = txt.find('-');
		auto buffer = txt;
		txt.resize(0);
		const char* c = buffer.c_str();
            while (*c) {
                  if (*c == ' ') {
                        while (*c == ' ')
                              ++c;
                        if (*c == '-') {
				      txt += '-';
                              ++c;
					while (*c == ' ')
                                    ++c;
				      }
                        else if (*c)
                              txt += '-';
			      }
                  else
                        txt += *(c++);
		      }
            if (gpLyrics.lyrics.size() == 0 || (gpLyrics.lyrics.size() == 1 && gpLyrics.lyrics[0].isEmpty())) {
//			gpLyrics.lyrics.resize(0);
                  gpLyrics.lyrics.clear();
			gpLyrics.fromBeat = _beat_counter;
			gpLyrics.lyricTrack = track;
		      }
            while (txt.size() && txt[txt.size() - 1] == '-')
                  txt.resize(txt.size() - 1);
//		  gpLyrics.lyrics.append(txt);
            gpLyrics.lyrics.append(QString::fromUtf8(txt.data(), int(txt.size())));
		gpLyrics.segments.push_back(segment);
            }
#if 0
      gpLyrics.beatCounter++;
      if (false && gpLyrics.beatCounter >= gpLyrics.fromBeat && gpLyrics.lyricTrack == staffIdx+1) {
            int index = gpLyrics.beatCounter - gpLyrics.fromBeat;
            if (index < gpLyrics.lyrics.size()) {
		      lyrics = new Lyrics(score);
			lyrics->setPlainText(gpLyrics.lyrics[index]);
                  }
            }
#endif
      int beatEffects = 0;
      if (beatBits & BEAT_EFFECTS)
            beatEffects = readBeatEffects(track, segment);
      last_segment = segment;
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
            if (cr->isRest() && (pause == 0 || l >= measure->len())) {
                  cr->setDurationType(TDuration::DurationType::V_MEASURE);
                  cr->setDuration(measure->len());
                  }
            else
                  cr->setDurationType(d);

            if (!segment->cr(track))
                  segment->add(cr);

            Staff* staff = cr->staff();
            int numStrings = staff->part()->instrument()->stringData()->strings();
            bool hasSlur = false;
		Note* _note{ nullptr };
		std::vector<Note*> delnote;
            for (int i = 6; i >= 0; --i) {
                  if (strings & (1 << i) && ((6-i) < numStrings)) {
                        Note* note = new Note(score);
				_note = note;
                        if (dotted) {
                              // there is at most one dotted note in this guitar pro version
                              NoteDot* dot = new NoteDot(score);
                              dot->setParent(note);
                              dot->setTrack(track);  // needed to know the staff it belongs to (and detect tablature)
                              dot->setVisible(true);
                              note->add(dot);
                              }
                        toChord(cr)->add(note);

                        hasSlur = (readNote(6-i, note) || hasSlur);
                        if (slideList.size() && slideList.back() == nullptr) {
                              slideList.back() = note;
					hasSlur = true;
					}
                        if (note->fret() == -20) {
                              delnote.push_back(note);
#if 0
					Chord* chord = toChord(cr);
					chord->remove(note);
                              delete note;
					if (chord->notes().empty()) {
                                    chord->segment()->remove(chord);
						delete chord;
						cr = nullptr;
						}
#endif
				      }
			      else
				      note->setTpcFromPitch();
                        }
                  }
            if (delnote.size()) {
                  Chord* chord = toChord(cr);
                  for (auto n : delnote) {
                        chord->remove(n);
                        delete n;
                        }
                  if (chord->notes().empty()) {
                        if (chord->tuplet())
                              chord->tuplet()->remove(chord);
                        chord->segment()->remove(chord);
                        delete chord;
				cr = nullptr;
				}
                  delnote.clear();
			}
            createSlur(hasSlur, staffIdx, cr);
            if (lyrics)
                  cr->add(lyrics);
			if (free_text.length() && _note) {
				addTextToNote(free_text, Align::CENTER, _note);
			      }

            }
      int rr = readChar();
      if (cr && cr->isChord()) {
            Chord* chord = toChord(cr);
            do {
                  applyBeatEffects(chord, beatEffects % 100);
            } while (beatEffects /= 100);
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
      if (cr && cr->isChord()) {
	      if (toChord(cr)->notes().size() == 0) {
                  segment->remove(cr);
                  delete cr;
			cr = 0;
		      }
            else if (slide > 0)
                  createSlide(slide, cr, staffIdx);
	      }
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
			if (beats > 100) return;
            for (int beat = 0; beat < beats; ++beat) {
                  int ticks = readBeat(tick, voice, measure, staffIdx, tuplets, mixChange);
				  ++_beat_counter;
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
            if (last_segment) {
                  score->setTempo(last_segment->tick(), double(tempo) / 60.0);
			last_segment = nullptr;
		      }
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
      readChar();
      if (version > 500) {
            readDelphiString();
            readDelphiString();
            }
      return editedTempo;
      }

//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

bool GuitarPro5::readTracks()
      {
      for (int i = 0; i < staves; ++i) {
            int tuning[GP_MAX_STRING_NUMBER];
            Staff* staff = score->staff(i);
            Part* part = staff->part();

            uchar c = readUChar();   // simulations bitmask
            if (c & 0x2) {           // 12 stringed guitar
                  }
            if (c & 0x4) {           // banjo track
                  }
            if (i == 0 || version == 500)
                  skip(1);
            QString name = readPascalString(40);

            int strings  = readInt();
            if (strings <= 0 || strings > GP_MAX_STRING_NUMBER)
                return false;
            for (int j = 0; j < strings; ++j) {
                  tuning[j] = readInt();
                  }
            for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j)
                  readInt();
            int midiPort     = readInt() -1;
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
			std::vector<int> tuning2(strings);
            //int tuning2[strings];
            for (int k = 0; k < strings; ++k)
                  tuning2[strings-k-1] = tuning[k];
            StringData stringData(frets, strings, &tuning2[0]);
			createTuningString(strings, &tuning2[0]);
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
                  ch->setProgram(0);
                  ch->setBank(128);
                  }
            else {
                  ch->setProgram(patch);
                  ch->setBank(0);
                  }
            ch->setVolume(channelDefaults[midiChannel].volume * 100.0 / 127.0);
            ch->setPan(((channelDefaults[midiChannel].pan / 127.0) - .5) * 360);
            ch->setChorus(channelDefaults[midiChannel].chorus * 100.0 / 127.0);
            ch->setReverb(channelDefaults[midiChannel].reverb * 100.0 / 127.0);
            staff->part()->setMidiChannel(midiChannel, midiPort);

            //qDebug("default2: %d", channelDefaults[i].reverb);
            // missing: phase, tremolo
            ch->updateInitList();
            }
      skip(version == 500 ? 2 : 1);

      return true;
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
                  RehearsalMark* s = new RehearsalMark(score);
                  s->setPlainText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
                  segment->add(s);
                  }

			std::vector<Tuplet*> tuplets(staves * 2);
            //Tuplet* tuplets[staves * 2];     // two voices
            for (int track = 0; track < staves*2; ++track)
                  tuplets[track] = 0;

            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  _beat_counter = 0;
                  readMeasure(measure, staffIdx, &tuplets[0], mixChange);
                  if (!(((bar == (measures-1)) && (staffIdx == (staves-1))))) {
                        /*int a = */  readChar();
                        // qDebug("    ======skip %02x", a);
                        }
                  }
            if (bar == 1 && !mixChange)
                  setTempo(tempo, score->firstMeasure());
            }

      if (gpLyrics.segments.size()) {
            auto size = std::min(int(gpLyrics.segments.size()), int(gpLyrics.lyrics.size()));
		for (int i = 0; i < size; ++i) {
                  std::string str = gpLyrics.lyrics[i].toUtf8().constData();
			auto seg = gpLyrics.segments[i];
			auto mes = seg->measure();
			while (str.size() && seg && seg->segmentType() == SegmentType::ChordRest) {
                        auto cr = seg->cr(gpLyrics.lyricTrack);
				if (cr) {
                              if (str[0] != '-') {
                                    auto lyr = new Lyrics(score);

						std::string text;
						auto pos = str.find('-');
						auto pos2 = str.find('\n');
						if (pos2 < pos)
                                          pos = pos2;
					      if (pos != std::string::npos) {
                                          const char* c = &str.c_str()[pos + 1];
							if (*c == 0) {
                                                pos = std::string::npos;
								text = str;
                                                }
                                          else {
                                                text = str.substr(0, pos);
                                                str = str.substr(pos + 1);
                                                }
                                          }
                                    else
                                          text = str;
                                    if (pos == std::string::npos)
                                          str.resize(0);
                                    lyr->setPlainText(QString::fromUtf8(text.data(), int(text.size())));
                                    cr->add(lyr);
                                    }
                              else {
                                    str = str.substr(1);
                                    }
                              }
                        seg = seg->next();
                        if (!seg) {
                              mes = mes->nextMeasure();
                              if (!mes)
                                    break;
                              seg = mes->first();
                              }
                        }
                  }
            }
      else {
            int counter = 0;
//            int index = 0;
//TODO-ws ???		gpLyrics.lyricTrack -= 1;
		auto mes = score->firstMeasure();
		auto beg = mes->first();

		do {
		      if (beg->isChordRestType() && beg->cr(gpLyrics.lyricTrack)) {
                        ChordRest* cr = beg->cr(gpLyrics.lyricTrack);
				++counter;
				if (!cr->isChord())
                              continue;
                        bool is_tied = false;
				Chord* chord = toChord(cr);
				for (auto n : chord->notes()) {
				      if (n->tiedNotes().size() > 1 && n->tiedNotes()[0] != n) {
                                    is_tied = true;
						break;
					      }
                              }
                        if (is_tied)
                              continue;
#if 0 // TODO-ws
                        if (counter >= gpLyrics.fromBeat) {
                              if (gpLyrics.lyrics[index][0] != '-') {
                                    auto lyr = new Lyrics(score);

                                    std::string text;
						auto pos  = gpLyrics.lyrics[index].find('-');
						auto pos2 = gpLyrics.lyrics[index].find('\n');
						if (pos2 < pos)
                                          pos = pos2;
						if (pos != std::string::npos) {
                                          const char* c = &gpLyrics.lyrics[index].c_str()[pos + 1];
							if (*c == 0) {
                                                pos = std::string::npos;
								text = gpLyrics.lyrics[index];
							      }
                                          else {
                                                text = gpLyrics.lyrics[index].substr(0, pos);
								auto str = gpLyrics.lyrics[index].substr(pos + 1);
								gpLyrics.lyrics[index] = str;
								if (str.length())
								      gpLyrics.lyrics[index][0] = str[0];
							      }
						      }
						else
                                          text = gpLyrics.lyrics[index];
                                    if (pos == std::string::npos)
                                          ++index;
                                    lyr->setPlainText(text);
						cr->add(lyr);
						if (index >= gpLyrics.lyrics.size())
							  break;
					      }
                              else {
                                    //TODO: Need studio new release, bug here
						std::string s = &gpLyrics.lyrics[index].c_str()[1];
						gpLyrics.lyrics[index] = s;
						if (s.length())
                                          gpLyrics.lyrics[index][0] = s[0];
					      }
				      }
#endif
			      }
		      } while ((beg = beg->next()) || ((mes = toMeasure(mes->next())) && (beg = mes->first())));
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro5::read(QFile* fp)
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

      std::vector<unsigned int> articulations;
	articulations.resize(19);
	{
            unsigned int r; unsigned char x;
		for (int i = 0; i < 19; ++i) {
                  x = readUChar();
			r = x;
			x = readUChar();
			r += x << 8;

			articulations[i] = r;
		      }
	      }

      //skip(42);
      skip(4);

      measures = readInt();
      staves  = readInt();

	for (int str = 0; str < 7; ++str) {
	      for (int staff = 0; staff < staves; ++staff)
		      dead_end[{staff, str}] = true;
            }

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
            if (barBits & 0x3) {
                  skip(4);
                  }
            if ((barBits & 0x10) == 0) {
                  skip(1);
			}

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
      if (!readTracks()) {
            return false;
            }
      readMeasures(tempo);
      for (auto n : slideList) {
//		Note* next = nullptr;
            auto segment = n->chord()->segment();
		auto measure = segment->measure();
		while ((segment = segment->next1(SegmentType::ChordRest)) || ((measure = measure->nextMeasure() ) && (segment = measure->first()))) {
                  if (segment->segmentType() != SegmentType::ChordRest)
			      continue;
                  bool br = false;
			ChordRest* cr = toChordRest(segment->cr(n->track()));
			if (cr && cr->isChord()) {
                        Chord* c = toChord(cr);
			      for (auto nt : c->notes()) {
				      if (nt->string() == n->string()) {
                                    for (auto e : nt->el())
						      if (e->isChordLine()) {
                                                ChordLine* cl = toChordLine(e);
								if (cl->chordLineType() == ChordLineType::PLOP || cl->chordLineType() == ChordLineType::SCOOP) {
                                                      br = true;
									break;
								      }
							      }
				            if (br)
                                          break;
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
						br = true;
						break;
                                    }
                              }
                        }
			if (br)
                        break;
		      }
            }

	  std::map<int, int> counter;
	  for (int i = 0; i < 19; ++i) {
		  if (articulations[i] != 0xFFFF) {
                  Measure* measure = toMeasure(score->measure(articulations[i] - 1));
			if (i < 4) {
                        Segment* segment = measure->getSegment(SegmentType::BarLine, measure->tick());
                        Symbol* sym = new Symbol(score);
				if (i == 0)
                              sym->setSym(SymId::coda);
                        else if (i == 1) {
#if 0
				      sym->setSym(SymId::coda);
					Symbol* s2 = new Symbol(score);
					s2->setSym(SymId::coda);
					s2->setXoffset(5.5f);
					auto iter = counter.find(articulations[i]);
					if (iter != counter.end())
					      s2->setElYOffset(-7.0f * iter->second);
                              s2->setParent(measure);
				      s2->setTrack(0);
					segment->add(s2);
#endif
					sym->setSym(SymId::codaSquare);
				      }
				else if (i == 2)
                              sym->setSym(SymId::segno);
				else {
#if 0
					sym->setSym(SymId::segno);
                              Symbol* s2 = new Symbol(score);
					s2->setSym(SymId::segno);
					s2->setXoffset(5.5f);
					auto iter = counter.find(articulations[i]);
					if (iter != counter.end())
						  s2->setElYOffset(-7.0f * iter->second);
					s2->setParent(measure);
					s2->setTrack(0);
					segment->add(s2);
#endif
					sym->setSym(SymId::segnoSerpent2);
				      }


                        sym->setParent(measure);
				sym->setTrack(0);
				segment->add(sym);
				auto iter = counter.find(articulations[i]);
				if (iter == counter.end())
				      counter[articulations[i]] = 1;
                        else {
//TODO-ws		            sym->setElYOffset(-7.0f * counter[articulations[i]]);
					counter[articulations[i]] += 1;
				      }
                        }
                  else {
                        Segment* s = measure->getSegment(SegmentType::KeySig, measure->tick());
				StaffText* st = new StaffText(score);
				static constexpr char text[][22] = {
					"fine", "Da Capo", "D.C. al Coda", "D.C. al Double Coda",
					"D.C. al Fine", "Da Segno", "D.S. al Coda", "D.S. al Double Coda",
					"D.S. al Fine", "Da Segno Segno", "D.S.S. al Coda", "D.S.S. al Double Coda",
					"D.S.S. al Fine", "Da Coda", "Da Double Coda"
				      };
			      st->setPlainText(text[i - 4]);
				st->setParent(s);
				st->setTrack(0);
//TODO-ws			st->_measureEnd = true;
				auto iter = counter.find(articulations[i]);
				if (iter == counter.end())
				      counter[articulations[i]] = 1;
                        else {
//TODO-ws                     st->setElYOffset(-7.0f * counter[articulations[i]]);
					counter[articulations[i]] += 1;
				      }
                        score->addElement(st);
			      }
		      }
	      }
      return true;
      }

//---------------------------------------------------------
//   readNoteEffects
//---------------------------------------------------------

bool GuitarPro5::readNoteEffects(Note* note)
      {
      uchar modMask1 = readUChar();
      uchar modMask2 = readUChar();
      bool slur = false;

      if (modMask1 & EFFECT_BEND) {
            readBend(note);
            }
      if (modMask1 & EFFECT_HAMMER) {
            slur = true;
#if 0
		Symbol* s = new Symbol(score);
		s->setSym(SymId::articLaissezVibrerBelow);
		s->setParent(note);
		note->add(s);
#endif
	      }
      if (modMask1 & EFFECT_LET_RING)
	      addLetRing(note);
		//note->setLetRing(true);

      if (modMask1 & EFFECT_GRACE) {
            int fret = readUChar();            // grace fret
            /*int dynamic =*/ readUChar();            // grace dynamic
            int transition = readUChar();            // grace transition
            /*int duration =*/ readUChar();            // grace duration
            int gflags = readUChar();

		NoteType note_type = NoteType::ACCIACCATURA;

            if (gflags & NOTE_APPOGIATURA) //on beat
                  note_type = NoteType::APPOGGIATURA;

#if 0
            int grace_len = MScore::division/8;
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
#endif
            int grace_pitch = note->staff()->part()->instrument()->stringData()->getPitch(note->string(), fret, nullptr, 0);
#if 0
            gn->setPitch(grace_pitch);
            gn->setTpcFromPitch();

            Chord* gc = new Chord(score);
            gc->setTrack(note->chord()->track());
            gc->add(gn);
            gc->setParent(note->chord());

            TDuration d;
            d.setVal(grace_len);
            if (grace_len == MScore::division/6)
                  d.setDots(1);
            gc->setDurationType(d);
            gc->setDuration(d.fraction());
            gc->setNoteType(note_type);
            gc->setMag(note->chord()->staff()->mag(0) * score->styleD(Sid::graceNoteMag));
            note->chord()->add(gc);
            addDynamic(gn, dynamic);
#endif
		auto gnote = score->setGraceNote(note->chord(), grace_pitch, note_type, MScore::division / 2);
		gnote->setString(note->string());
		auto sd = note->part()->instrument()->stringData();
		gnote->setFret(grace_pitch - sd->stringList().at(sd->stringList().size() - note->string() - 1).pitch);
            if (transition == 0) {
                  // no transition
                  }
            else if (transition == 1) {
			}
	      else if (transition == 3) {
		      Slur* slur = new Slur(score);
			slur->setAnchor(Spanner::Anchor::CHORD);
			slur->setStartElement(gnote->chord());
			slur->setEndElement(note->chord());
			slur->setParent(0);
			slur->setTrack(note->staffIdx());
			slur->setTrack2(note->staffIdx());
			slur->setTick(gnote->chord()->tick());
			slur->setTick2(note->chord()->tick());
//TODO-ws		note->chord()->has_slur = true;
			score->addElement(slur);
                  //TODO: Add a 'slide' guitar effect when implemented
			//note->setSlideNote(gn);
                  }
#if 0
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
#endif
            }
      if (modMask2 & EFFECT_STACATTO) {
            Chord* chord = note->chord();
            Articulation* a = new Articulation(chord->score());
            a->setSymId(SymId::articStaccatoAbove);
		bool add = true;
		for (auto a : chord->articulations()) {
		      if (a->symId() == SymId::articStaccatoAbove) {
			      add = false;
				break;
				}
                  }
            if (add)
                  chord->add(a);
            }
      if (modMask2 & EFFECT_PALM_MUTE)
	      addPalmMute(note);
		//note->setPalmMute(true);

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
//      bool skip = false;
      if (modMask2 & EFFECT_SLIDE) {
            int slideKind = readUChar();
		if (slideKind & SLIDE_OUT_DOWN) {
		      slideKind &= ~SLIDE_OUT_DOWN;
			ChordLine* cl = new ChordLine(score);
			cl->setChordLineType(ChordLineType::FALL);
			cl->setStraight(true);
			note->add(cl);
//			skip = true;
			}
	      // slide out upwards (doit)
		if (slideKind & SLIDE_OUT_UP) {
		      slideKind &= ~SLIDE_OUT_UP;
			ChordLine* cl = new ChordLine(score);
			cl->setChordLineType(ChordLineType::DOIT);
			cl->setStraight(true);
			note->add(cl);
//			skip = true;
			}
            // slide in from below (plop)
		if (slideKind & SLIDE_IN_BELOW) {
		      slideKind &= ~SLIDE_IN_BELOW;
			ChordLine* cl = new ChordLine(score);
			cl->setChordLineType(ChordLineType::PLOP);
			cl->setStraight(true);
			note->add(cl);
//			skip = true;
			}
	      // slide in from above (scoop)
		if (slideKind & SLIDE_IN_ABOVE) {
		      slideKind &= ~SLIDE_IN_ABOVE;
			ChordLine* cl = new ChordLine(score);
			cl->setChordLineType(ChordLineType::SCOOP);
			cl->setStraight(true);
			note->add(cl);
//			skip = true;
			}

	      if (false && !slideList.empty() && slideList.back()->chord()->segment() != note->chord()->segment()) {
                  Note* start = slideList.front();
			slideList.pop_front();
			bool skip = false;
			for (auto e : start->el()) {
			      if (e->isChordLine())
                              skip = true;
                        }
                  if (!skip) {
			      Glissando* s = new Glissando(score);
				s->setAnchor(Spanner::Anchor::NOTE);
				s->setStartElement(start);
				s->setTick(start->chord()->segment()->tick());
				s->setTrack(start->staffIdx());
				s->setParent(start);
				s->setGlissandoType(GlissandoType::STRAIGHT);
				s->setEndElement(note);
				s->setTick2(note->chord()->segment()->tick());
				s->setTrack2(note->staffIdx());
				score->addElement(s);
				}
			}
            if (slideKind & LEGATO_SLIDE) {
                  slideKind &= ~LEGATO_SLIDE;
			slideList.push_back(nullptr);
			createSlur(true, note->staffIdx(), note->chord());
			}
	      if (slideKind & SHIFT_SLIDE) {
                  slideKind &= ~SHIFT_SLIDE;
			slideList.push_back(note);
			}
#if 0
            if (slideKind)
		      int k = 1;
            if (slideKind > 4)
                  int k = 1;
            // if slide >= 4 then we are not dealing with legato slide nor shift slide
            if (slideKind >= 4)
                  slide = slideKind;
            else
                  slides[note->chord()->track()] = slideKind;
#endif
            }

      if (modMask2 & EFFECT_ARTIFICIAL_HARMONIC) {
            int type = readArtificialHarmonic();
		if (type == 2) {
                  auto harmNote = readUChar();
			/*auto sharp =*/ readChar();
			auto octave = readUChar();

			auto harmonicNote = new Note(score);
//TODO-ws		harmonicNote->setHarmonic(true);
			note->chord()->add(harmonicNote);
			auto staff = note->staff();
//			int string = staff->part()->instrument()->stringData()->strings() - 1 - note->string();
			int fret = note->fret();
			switch (harmNote) {
                        case 0: fret += 24; break;
			      case 2: fret += 34; break;
			      case 4: fret += 38; break;
			      case 5: fret += 12; break;
			      case 7: fret += 32; break;
			      case 9: fret += 28; break;
			      default: fret += octave * 12;
			      }
		      harmonicNote->setString(note->string());
			harmonicNote->setFret(fret);
			harmonicNote->setPitch(staff->part()->instrument()->stringData()->getPitch(note->string(), fret, nullptr, 0));
			harmonicNote->setTpcFromPitch();
			addTextToNote("A.H.", Align::CENTER, harmonicNote);
		      }
            if (type == 1 || type == 4 || type == 5) {
                  //TextStyle textStyle;
			//textStyle.setAlign(Align::CENTER);
			//addTextToNote("N.H.", textStyle, note);
//TODO-ws		note->setHarmonic(true);
		      }
	      }

            if (modMask2 & 0x40)
                  addVibrato(note);

      if (modMask2 & EFFECT_TRILL) {
//TODO-ws            note->setTrillFret(readUChar());      // trill fret
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
      if (noteBits & NOTE_FRET) {
	      fretNumber = readChar();
            }

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

      if (noteBits & 0x1)     // Time independent duration
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
			note->add(art);
            // if (!note->score()->addArticulation(note, art))
            //      delete art;
            }

      readUChar(); //skip

      Staff* staff = note->staff();
      if (fretNumber == 255 || fretNumber < 0) {
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
      if (noteBits & NOTE_SLUR) {
            slur = readNoteEffects(note);
	      }

      if (tieNote) {
            auto staffIdx = note->staffIdx();
		if (dead_end[{staffIdx, string}]) {
		      note->setFret(-20);
			return false;
		      }
            if (slurs[staffIdx]) {
                  score->removeSpanner(slurs[staffIdx]);
			delete slurs[staffIdx];
			slurs[staffIdx] = 0;
		      }
            bool found = false;
            Chord* chord     = note->chord();
            Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
            int track        = note->track();
		std::vector<ChordRest*> chords;
		Note* true_note = nullptr;
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
				      auto rest = toRest(cr);
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
		      note->setFret(-20);
			dead_end[{staffIdx, string}] = true;
			qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
			return false;
			}
            }
      dead_end[{note->staffIdx(), string}] = false;
      return slur;
      }

//---------------------------------------------------------
//   readArtificialHarmonic
//---------------------------------------------------------

int GuitarPro5::readArtificialHarmonic()
      {
      int type = readChar();
      switch(type) {
            case 1:           // natural
                  break;
            case 2:           // artificial
                  //skip(3);
                  break;
            case 3:           // tapped
                  skip(1);
                  break;
            case 4:           // pinch
                  break;
            case 5:           // semi
                  break;
            }
	  return type;
      }

}
