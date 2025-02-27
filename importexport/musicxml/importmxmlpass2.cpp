//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2015-2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <memory>
#include <utility>

#include "libmscore/accidental.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/box.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/chordline.h"
#include "libmscore/chordlist.h"
#include "libmscore/chordrest.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/fermata.h"
#include "libmscore/figuredbass.h"
#include "libmscore/fingering.h"
#include "libmscore/fret.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/instrchange.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/interval.h"
#include "libmscore/jump.h"
#include "libmscore/keysig.h"
#include "libmscore/line.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/mscore.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/rest.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftext.h"
#include "libmscore/stem.h"
#include "libmscore/sym.h"
#include "libmscore/system.h"
#include "libmscore/tempo.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/textline.h"
#include "libmscore/tie.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/utils.h"
#include "libmscore/volta.h"

#include "importmxmllogger.h"
#include "importmxmlnoteduration.h"
#include "importmxmlnotepitch.h"
#include "importmxmlpass1.h"
#include "importmxmlpass2.h"
#include "musicxmlfonthandler.h"
#include "musicxmlsupport.h"

#include "mscore/preferences.h"

namespace Ms {

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

//#define DEBUG_VOICE_MAPPER true

//---------------------------------------------------------
//   function declarations
//---------------------------------------------------------

static void addTie(const Notation& notation, Note* note, const int track, MusicXMLTieMap& tie,
                   std::vector<Note*>& unstartedTieNotes, std::vector<Note*>& unendedTieNotes, MxmlLogger* logger,
                   const QXmlStreamReader* const xmlreader);

//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

//---------------------------------------------------------
//   MusicXmlTupletDesc
//---------------------------------------------------------

MusicXmlTupletDesc::MusicXmlTupletDesc()
      : type(MxmlStartStop::NONE), direction(Direction::AUTO),
      bracket(TupletBracketType::AUTO_BRACKET), shownumber(TupletNumberType::SHOW_NUMBER)
      {
      // nothing
      }

//---------------------------------------------------------
//   MusicXmlLyricsExtend
//---------------------------------------------------------

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MusicXmlLyricsExtend::init()
      {
      _lyrics.clear();
      }

//---------------------------------------------------------
//   addLyric
//---------------------------------------------------------

// add a single lyric to be extended later
// called when lyric with "extend" or "extend type=start" is found

void MusicXmlLyricsExtend::addLyric(Lyrics* const lyric)
      {
      _lyrics.insert(lyric);
      }

//---------------------------------------------------------
//   lastChordTicks
//---------------------------------------------------------

// find the duration of the chord starting at or after s and ending at tick

static Fraction lastChordTicks(const Segment* s, const Fraction& tick, const int track)
      {
      while (s && s->tick() < tick) {
            for (Element* el : s->elist()) {
                  if (el && el->isChordRest() && el->track() == track) {
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        if (cr->tick() + cr->actualTicks() == tick)
                              return cr->actualTicks();
                        }
                  }
            s = s->nextCR(-1, true);
            }
      return Fraction(0,1);
      }

//---------------------------------------------------------
//   setExtend
//---------------------------------------------------------

// set extend for lyric no in *staff* to end at tick
// called when lyric (with or without "extend") or note with "extend type=stop" is found
// note that no == -1 means all lyrics in this *track*

void MusicXmlLyricsExtend::setExtend(const int no, const int track, const Fraction& tick, const Lyrics* prevAddedLyrics = nullptr)
      {
      QList<Lyrics*> list;
      for (Lyrics* l : qAsConst(_lyrics)) {
            Element* const el = l->parent();
            if (el->type() == ElementType::CHORD || el->type() == ElementType::REST) {
                  const ChordRest* par = static_cast<ChordRest*>(el);
                  // no = -1: stop all extends on this track
                  // otherwise, stop all extends in the stave with the same no and placement
                  if ((no == -1 && par->track() == track)
                      || (l->no() == no && track2staff(par->track()) == track2staff(track) && prevAddedLyrics
                          && prevAddedLyrics->placement() == l->placement())) {
                        Fraction lct = lastChordTicks(l->segment(), tick, track);
                        if (lct > Fraction(0,1)) {
                              // set lyric tick to the total length from the lyric note
                              // plus all notes covered by the melisma minus the last note length
                              l->setTicks(tick - par->tick() - lct);
                              }
                        list.append(l);
                        }
                  }
            }
      // cleanup
      for (Lyrics* l: list) {
            _lyrics.remove(l);
            }
      }

//---------------------------------------------------------
//   MusicXMLStepAltOct2Pitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step (0=C, 1=D, etc.) / \a alter / \a octave to midi pitch.
 Note: same code is in pass 1 and in pass 2.
 TODO: combine
 */

static int MusicXMLStepAltOct2Pitch(int step, int alter, int octave)
      {
      //                       c  d  e  f  g  a   b
      static int table[7]  = { 0, 2, 4, 5, 7, 9, 11 };
      if (step < 0 || step > 6) {
            qDebug("MusicXMLStepAltOct2Pitch: illegal step %d", step);
            return -1;
            }
      int pitch = table[step] + alter + (octave+1) * 12;

      if (pitch < 0)
            pitch = -1;
      if (pitch > 127)
            pitch = -1;

      return pitch;
      }

//---------------------------------------------------------
//   xmlSetPitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch,
 set pitch and tpc.
 Note that n's staff and track have not been set yet
 */

static void xmlSetPitch(Note* n, int step, int alter, qreal tuning, int octave, const int octaveShift, const Instrument* const instr, Interval inferredTranspose = Interval(0))
      {
      //qDebug("xmlSetPitch(n=%p, step=%d, alter=%d, octave=%d, octaveShift=%d)",
      //       n, step, alter, octave, octaveShift);

      //const Staff* staff = n->score()->staff(track / VOICES);
      //const Instrument* instr = staff->part()->instr();

      const Interval intval = instr->transpose();
      const Interval combinedIntval(intval.diatonic + inferredTranspose.diatonic, intval.chromatic + inferredTranspose.chromatic);
      //qDebug("  staff=%p instr=%p dia=%d chro=%d",
      //       staff, instr, static_cast<int>(intval.diatonic), static_cast<int>(intval.chromatic));

      int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
      pitch += intval.chromatic; // assume not in concert pitch
      pitch += 12 * octaveShift; // correct for octave shift
      pitch += inferredTranspose.chromatic;
      // ensure sane values
      pitch = limit(pitch, 0, 127);

      int tpc2 = step2tpc(step, AccidentalVal(alter));
      tpc2 = Ms::transposeTpc(tpc2, inferredTranspose, true);
      int tpc1 = Ms::transposeTpc(tpc2, combinedIntval, true);
      n->setPitch(pitch, tpc1, tpc2);
      n->setTuning(tuning);
      //qDebug("  pitch=%d tpc1=%d tpc2=%d", n->pitch(), n->tpc1(), n->tpc2());
      }

//---------------------------------------------------------
//   fillGap
//---------------------------------------------------------

/**
 Fill one gap (tstart - tend) in this track in this measure with rest(s).
 */

static void fillGap(Measure* measure, int track, const Fraction& tstart, const Fraction& tend)
      {
      Fraction ctick = tstart;
      Fraction restLen = tend - tstart;
      // qDebug("\nfillGIFV     fillGap(measure %p track %d tstart %d tend %d) restLen %d len",
      //        measure, track, tstart, tend, restLen);
      // note: as MScore::division (#ticks in a quarter note) equals 480
      // MScore::division / 64 (#ticks in a 256th note) equals 7.5 but is rounded down to 7
      while (restLen > Fraction(1,256)) {
            Fraction len = restLen;
            TDuration d(TDuration::DurationType::V_INVALID);
            if (measure->ticks() == restLen)
                  d.setType(TDuration::DurationType::V_MEASURE);
            else
                  d.setVal(len.ticks());
            Rest* rest = new Rest(measure->score(), d);
            rest->setTicks(len);
            rest->setTrack(track);
            rest->setVisible(false);
            Segment* s = measure->getSegment(SegmentType::ChordRest, tstart);
            s->add(rest);
            len = rest->globalTicks();
            // qDebug(" %d", len);
            ctick   += len;
            restLen -= len;
            }
      }

//---------------------------------------------------------
//   fillGapsInFirstVoices
//---------------------------------------------------------

/**
 Fill gaps in first voice of every staff in this measure for this part with rest(s).
 */

static void fillGapsInFirstVoices(Measure* measure, Part* part)
      {
      if (!measure)
            return;
      if (!part)
            return;

      Fraction measTick     = measure->tick();
      Fraction measLen      = measure->ticks();
      Fraction nextMeasTick = measTick + measLen;
      int staffIdx = part->score()->staffIdx(part);
      /*
       qDebug("fillGIFV measure %p part %p idx %d nstaves %d tick %d - %d (len %d)",
       measure, part, staffIdx, part->nstaves(),
       measTick, nextMeasTick, measLen);
       */
      for (int st = 0; st < part->nstaves(); ++st) {
            int track = (staffIdx + st) * VOICES;
            Fraction endOfLastCR = measTick;
            for (Segment* s = measure->first(); s; s = s->next()) {
                  // qDebug("fillGIFV   segment %p tp %s", s, s->subTypeName());
                  Element* el = s->element(track);
                  if (el) {
                        // qDebug(" el[%d] %p", track, el);
                        if (s->isChordRestType()) {
                              ChordRest* cr  = static_cast<ChordRest*>(el);
                              Fraction crTick     = cr->tick();
                              Fraction crLen      = cr->globalTicks();
                              Fraction nextCrTick = crTick + crLen;
                              /*
                               qDebug(" chord/rest tick %d - %d (len %d)",
                               crTick, nextCrTick, crLen);
                               */
                              if (crTick > endOfLastCR) {
                                    /*
                                     qDebug(" GAP: track %d tick %d - %d",
                                     track, endOfLastCR, crTick);
                                     */
                                    fillGap(measure, track, endOfLastCR, crTick);
                                    }
                              endOfLastCR = nextCrTick;
                              }
                        }
                  }
            if (nextMeasTick > endOfLastCR) {
                  /*
                   qDebug("fillGIFV   measure end GAP: track %d tick %d - %d",
                   track, endOfLastCR, nextMeasTick);
                   */
                  fillGap(measure, track, endOfLastCR, nextMeasTick);
                  }
            }
      }

//---------------------------------------------------------
//   hasDrumset
//---------------------------------------------------------

/**
 Determine if \a instruments contains a valid drumset.
 This is the case if any instrument has a midi-unpitched element.
 */

static bool hasDrumset(const MusicXMLInstruments& instruments)
      {
      bool res = false;
      MusicXMLInstrumentsIterator ii(instruments);
      while (ii.hasNext()) {
            ii.next();
            // debug: dump the instruments
            //qDebug("instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
            // find valid unpitched values
            int unpitched = ii.value().unpitched;
            if (0 <= unpitched && unpitched <= 127) {
                  res = true;
                  }
            }

      /*
      for (const auto& instr : instruments) {
            // MusicXML elements instrument-name, midi-program, instrument-sound, virtual-library, virtual-name
            // in a shell script use "mscore ... 2>&1 | grep GREP_ME | cut -d' ' -f3-" to extract
            qDebug("GREP_ME '%s',%d,'%s','%s','%s'",
                   qPrintable(instr.name),
                   instr.midiProgram + 1,
                   qPrintable(instr.sound),
                   qPrintable(instr.virtLib),
                   qPrintable(instr.virtName)
                   );
            }
       */

      return res;
      }

//---------------------------------------------------------
//   initDrumset
//---------------------------------------------------------

/**
 Initialize drumset \a drumset.
 */

// first determine if the part contains a drumset
// (this is assummed if any instrument has a valid midi-unpitched element,
// which stored in the MusicXMLInstrument pitch field)
// then if the part contains a drumset, Drumset drumset is initialized

static void initDrumset(Drumset* drumset, const MusicXMLInstruments& instruments)
      {
      drumset->clear();
      MusicXMLInstrumentsIterator ii(instruments);
      while (ii.hasNext()) {
            ii.next();
            // debug: also dump the drumset for this part
            //qDebug("initDrumset: instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
            int unpitched = ii.value().unpitched;
            if (0 <= unpitched && unpitched <= 127) {
                  drumset->drum(ii.value().unpitched)
                        = DrumInstrument(ii.value().name.toLatin1().constData(),
                                         ii.value().notehead, ii.value().line, ii.value().stemDirection);
                  }
            }
      }

//---------------------------------------------------------
//   setStaffTypePercussion
//---------------------------------------------------------

/**
 Set staff type to percussion
 */

static void setStaffTypePercussion(Part* part, Drumset* drumset)
      {
      for (int j = 0; j < part->nstaves(); ++j)
            if (part->staff(j)->lines(Fraction(0,1)) == 5 && !part->staff(j)->isDrumStaff(Fraction(0,1)))
                  part->staff(j)->setStaffType(Fraction(0,1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
      // set drumset for instrument
      part->instrument()->setDrumset(drumset);
      part->instrument()->channel(0)->setBank(128);
      }

//---------------------------------------------------------
//   createInstrument
//---------------------------------------------------------

/**
 Create an Instrument based on the information in \a mxmlInstr.
 */

static Instrument createInstrument(const MusicXMLInstrument& mxmlInstr, const Interval interval)
      {
      Instrument instr;

      InstrumentTemplate* it = nullptr;
      if (!mxmlInstr.sound.isEmpty()) {
          it = Ms::searchTemplateForMusicXmlId(mxmlInstr.sound);
      }

      if (!it) {
          it = Ms::searchTemplateForInstrNameList({mxmlInstr.name, mxmlInstr.abbreviation});
      }

      if (!it) {
          it = Ms::searchTemplateForMidiProgram(mxmlInstr.midiProgram);
      }

      if (it) {
            // initialize from template with matching MusicXmlId
            instr = Instrument::fromTemplate(it);
            // reset transpose, as it is determined later from MusicXML data
            instr.setTranspose(Interval());
            }
      else {
            // set articulations to default (global articulations)
            instr.setArticulation(articulation);
            // set default program
            instr.channel(0)->setProgram(mxmlInstr.midiProgram >= 0 ? mxmlInstr.midiProgram : 0);
            }

      // add / overrule with values read from MusicXML
      instr.channel(0)->setPan(mxmlInstr.midiPan);
      instr.channel(0)->setVolume(mxmlInstr.midiVolume);
      instr.setTrackName(mxmlInstr.name);
      instr.setTranspose(interval);

      return instr;
      }

//---------------------------------------------------------
//   updatePartWithInstrument
//---------------------------------------------------------

static void updatePartWithInstrument(Part* const part, const MusicXMLInstrument& mxmlInstr, const Interval interval, const bool hasDrumset = false)
      {
      Instrument instr = createInstrument(mxmlInstr, interval);
      if (hasDrumset)
            instr.channel(0)->setBank(128);
      part->setInstrument(instr);
      if (mxmlInstr.midiChannel >= 0)
            part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort);
      // note: setMidiProgram() does more than simply setting the MIDI program
      if (mxmlInstr.midiProgram >= 0)
            part->setMidiProgram(mxmlInstr.midiProgram);
      }

//---------------------------------------------------------
//   createInstrumentChange
//---------------------------------------------------------

/**
 Create an InstrumentChange based on the information in \a mxmlInstr.
 */

static InstrumentChange* createInstrumentChange(Score* score, const MusicXMLInstrument& mxmlInstr, const Interval interval, const int track, const Instrument* curInstr)
      {
      const Instrument instr = createInstrument(mxmlInstr, interval);

      if (curInstr->getId() == instr.getId() && instr.longNames() == curInstr->longNames() && instr.shortNames() == curInstr->shortNames())
            return nullptr;

      InstrumentChange* instrChange = new InstrumentChange(instr, score);
      instrChange->setTrack(track);

      // for text use instrument name (if known) else use "Instrument change"
      const QString text = mxmlInstr.name;
      instrChange->setXmlText(text.isEmpty() ? "Instrument change" : text);
      instrChange->setVisible(false);

      return instrChange;
      }

//---------------------------------------------------------
//   updatePartWithInstrumentChange
//---------------------------------------------------------

static void updatePartWithInstrumentChange(Part* const part, const MusicXMLInstrument& mxmlInstr, const Interval interval, Segment* const segment, const int track, const Fraction tick)
      {
      const Instrument* curInstr = part->instrument(tick);
      InstrumentChange* const ic = createInstrumentChange(part->score(), mxmlInstr, interval, track, curInstr);

      if (!ic)
            return;

      segment->add(ic);             // note: includes part::setInstrument(instr);

      // setMidiChannel() depends on setInstrument() already been done
      if (mxmlInstr.midiChannel >= 0) part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort, tick);
      }

//---------------------------------------------------------
//   setPartInstruments
//---------------------------------------------------------

/**
 Set instruments for Part \a part
 Note:
 - MusicXmlInstrList: which instrument plays when
 - MusicXMLInstruments: instrument details from score-part and part
 */

static void setPartInstruments(MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                               Part* part, const QString& partId,
                               Score* score,
                               const MusicXmlInstrList& instrList,
                               const MusicXmlIntervalList& intervList,
                               const MusicXMLInstruments& instruments)
      {
      if (instruments.empty()) {
            // no instrument details found, create a default instrument
            //qDebug("no instrument details");
            updatePartWithInstrument(part, {}, intervList.interval({ 0, 1 }));
            return;
            }

      if (hasDrumset(instruments)) {
            // do not create multiple instruments for a drum part
            //qDebug("hasDrumset");
            MusicXMLInstrument mxmlInstr = instruments.first();
            updatePartWithInstrument(part, mxmlInstr, {}, true);
            return;
            }

      if (instrList.empty()) {
            // instrument details found, but no instrument ids found
            // -> only a single instrument is playing in the part
            //qDebug("single instrument");
            MusicXMLInstrument mxmlInstr = instruments.first();
            updatePartWithInstrument(part, mxmlInstr, intervList.interval({ 0, 1 }));
            return;
            }

      // either a single instrument is playing, or forwards / rests resulted in gaps in the instrument map
      // (and thus multiple entries)
      //qDebug("possibly multiple instruments");
      QString prevInstrId;
      for (auto it = instrList.cbegin(); it != instrList.cend(); ++it) {
            Fraction tick = (*it).first;
            if (it == instrList.cbegin()) {
                  prevInstrId = (*it).second;        // first instrument id
                  MusicXMLInstrument mxmlInstr = instruments.value(prevInstrId);
                  updatePartWithInstrument(part, mxmlInstr, intervList.interval(tick));
                  }
            else {
                  QString instrId = (*it).second;
                  bool mustInsert = instrId != prevInstrId;
                  /*
                   qDebug("tick %s previd %s id %s mustInsert %d",
                   qPrintable(tick.print()),
                   qPrintable(prevInstrId),
                   qPrintable(instrId),
                   mustInsert);
                   */
                  if (mustInsert) {
                        const int staff = score->staffIdx(part);
                        const int track = staff * VOICES;
                        //qDebug("instrument change: tick %s (%d) track %d instr '%s'",
                        //       qPrintable(tick.print()), tick.ticks(), track, qPrintable(instrId));

                        Measure* const m = score->tick2measure(tick);
                        Segment* const segment = m->getSegment(SegmentType::ChordRest, tick);

                        if (!segment)
                              logger->logError(QString("segment for instrument change at tick %1 not found")
                                               .arg(tick.ticks()), xmlreader);
                        else if (!instruments.contains(instrId))
                              logger->logError(QString("changed instrument '%1' at tick %2 not found in part '%3'")
                                               .arg(instrId).arg(tick.ticks()).arg(partId), xmlreader);
                        else {
                              MusicXMLInstrument mxmlInstr = instruments.value(instrId);
                              updatePartWithInstrumentChange(part, mxmlInstr, intervList.interval(tick), segment, track, tick);
                              }
                        }
                  prevInstrId = instrId;
                  }
            }
      }

//---------------------------------------------------------
//   text2syms
//---------------------------------------------------------

/**
 Convert SMuFL code points to MuseScore <sym>...</sym>
 */

static QString text2syms(const QString& t)
      {
      //QTime time;
      //time.start();

      // first create a map from symbol (Unicode) text to symId
      // note that this takes about 1 msec on a Core i5,
      // caching does not gain much

      ScoreFont* sf = ScoreFont::fallbackFont();
      QMap<QString, SymId> map;
      int maxStringSize = 0;        // maximum string size found

      for (int i = int(SymId::noSym); i < int(SymId::lastSym); ++i) {
            SymId id((SymId(i)));
            QString string(sf->toString(id));
            // insert all syms except space to prevent matching all regular spaces
            if (id != SymId::space)
                  map.insert(string, id);
            if (string.size() > maxStringSize)
                  maxStringSize = string.size();
            }

      // Special case Dolet inference (TODO: put behind a setting or export type flag)
      map.insert("$", SymId::segno);
      map.insert("Ø", SymId::coda);

      //qDebug("text2syms map count %d maxsz %d filling time elapsed: %d ms",
      //       map.size(), maxStringSize, time.elapsed());

      // then look for matches
      QString in = t;
      QString res;

      while (!in.isEmpty()) {
            // try to find the largest match possible
            int maxMatch = qMin(in.size(), maxStringSize);
            QString sym;
            while (maxMatch > 0) {
                  QString toBeMatched = in.left(maxMatch);
                  if (map.contains(toBeMatched)) {
                        sym = Sym::id2name(map.value(toBeMatched));
                        break;
                        }
                  maxMatch--;
                  }
            if (maxMatch > 0) {
                  // found a match, add sym to res and remove match from string in
                  res += "<sym>";
                  res += sym;
                  res += "</sym>";
                  in.remove(0, maxMatch);
                  }
            else {
                  // not found, move one char from res to in
                  res += in.leftRef(1);
                  in.remove(0, 1);
                  }
            }

      //qDebug("text2syms total time elapsed: %d ms, res '%s'", time.elapsed(), qPrintable(res));
      return res;
      }

//---------------------------------------------------------
//   decodeEntities
//---------------------------------------------------------

/**
 Decode &#...; in string \a src into UNICODE (utf8) character.
 */

static QString decodeEntities( const QString& src )
      {
      QString ret(src);
      QRegExp re("&#([0-9]+);");
      re.setMinimal(true);

      int pos = 0;
      while ( (pos = re.indexIn(src, pos)) != -1 ) {
            ret = ret.replace(re.cap(0), QChar(re.cap(1).toInt(0,10)));
            pos += re.matchedLength();
            }
      return ret;
      }

//---------------------------------------------------------
//   nextPartOfFormattedString
//---------------------------------------------------------

// TODO: probably should be shared between pass 1 and 2

/**
 Read the next part of a MusicXML formatted string and convert to MuseScore internal encoding.
 */

static QString nextPartOfFormattedString(QXmlStreamReader& e)
      {
      //QString lang       = e.attribute(QString("xml:lang"), "it");
      QString fontWeight = e.attributes().value("font-weight").toString();
      QString fontSize   = e.attributes().value("font-size").toString();
      QString fontStyle  = e.attributes().value("font-style").toString();
      QString underline  = e.attributes().value("underline").toString();
      QString strike     = e.attributes().value("line-through").toString();
      QString fontFamily = e.attributes().value("font-family").toString();
      // TODO: color, enclosure, yoffset in only part of the text, ...

      QString txt        = e.readElementText();
      // replace HTML entities
      txt = decodeEntities(txt);
      QString syms       = text2syms(txt);

      QString importedtext;

      if (!fontSize.isEmpty()) {
            bool ok = true;
            float size = fontSize.toFloat(&ok);
            if (ok)
                  importedtext += QString("<font size=\"%1\"/>").arg(size);
            }

      bool needUseDefaultFont = preferences.getBool(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES);

      if (!fontFamily.isEmpty() && txt == syms && !needUseDefaultFont) {
            // add font family only if no <sym> replacement made
            importedtext += QString("<font face=\"%1\"/>").arg(fontFamily);
            }
      if (fontWeight == "bold")
            importedtext += "<b>";
      if (fontStyle == "italic")
            importedtext += "<i>";
      if (!underline.isEmpty()) {
            bool ok = true;
            int lines = underline.toInt(&ok);
            if (ok && (lines > 0))  // 1, 2, or 3 underlines are imported as single underline
                  importedtext += "<u>";
            else
                  underline.clear();
            }
      if (!strike.isEmpty()) {
            bool ok = true;
            int lines = underline.toInt(&ok);
            if (ok && (lines > 0))  // 1, 2, or 3 strike are imported as single strike
                  importedtext += "<s>";
            else
                  strike.clear();
            }
      if (txt == syms) {
            txt.replace(QString("\r"), QString("")); // convert Windows line break \r\n -> \n
            importedtext += txt.toHtmlEscaped();
            }
      else {
            // <sym> replacement made, should be no need for line break or other conversions
            importedtext += syms;
            }
      if (!strike.isEmpty())
            importedtext += "</s>";
      if (!underline.isEmpty())
            importedtext += "</u>";
      if (fontStyle == "italic")
            importedtext += "</i>";
      if (fontWeight == "bold")
            importedtext += "</b>";
      //qDebug("importedtext '%s'", qPrintable(importedtext));
      return importedtext;
      }

//---------------------------------------------------------
//   addLyric
//---------------------------------------------------------

/**
 Add a single lyric to the score or delete it (if number too high)
 */

static void addLyric(MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                     ChordRest* cr, Lyrics* l, int lyricNo, MusicXmlLyricsExtend& extendedLyrics)
      {
      if (lyricNo > MAX_LYRICS) {
            logger->logError(QString("too much lyrics (>%1)")
                             .arg(MAX_LYRICS), xmlreader);
            delete l;
            }
      else {
            l->setNo(lyricNo);
            cr->add(l);
            extendedLyrics.setExtend(lyricNo, cr->track(), cr->tick(), l);
            }
      }

//---------------------------------------------------------
//   addLyrics
//---------------------------------------------------------

/**
 Add a notes lyrics to the score
 */

static void addLyrics(MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                      ChordRest* cr,
                      const QMap<int, Lyrics*>& numbrdLyrics,
                      const QSet<Lyrics*>& extLyrics,
                      MusicXmlLyricsExtend& extendedLyrics)
      {
      for (const auto& lyricNo : numbrdLyrics.keys()) {
            Lyrics* const lyric = numbrdLyrics.value(lyricNo);
            addLyric(logger, xmlreader, cr, lyric, lyricNo, extendedLyrics);
            if (extLyrics.contains(lyric))
                  extendedLyrics.addLyric(lyric);
            }
      }

static void addGraceNoteLyrics(const QMap<int, Lyrics*>& numberedLyrics, QSet<Lyrics*> extendedLyrics,
                               std::vector<GraceNoteLyrics>& gnLyrics)
      {
      for (const auto& lyricNo : numberedLyrics.keys()) {
            Lyrics* const lyric = numberedLyrics[lyricNo];
            if (lyric) {
                  bool extend = extendedLyrics.contains(lyric);
                  const GraceNoteLyrics gnl = GraceNoteLyrics(lyric, extend, lyricNo);
                  gnLyrics.push_back(gnl);
                  }
            }
      }

//---------------------------------------------------------
//   addElemOffset
//---------------------------------------------------------

static void addElemOffset(Element* el, int track, const QString& placement, Measure* measure, const Fraction& tick)
      {
      if (!measure)
          return;
      /*
       qDebug("addElem el %p track %d placement %s tick %d",
       el, track, qPrintable(placement), tick);
       */

#if 0 // ws: use placement for symbols
      // move to correct position
      // TODO: handle rx, ry
      if (el->isSymbol()) {
            qreal y = 0;
            // calc y offset assuming five line staff and default style
            // note that required y offset is element type dependent
            const qreal stafflines = 5; // assume five line staff, but works OK-ish for other sizes too
            qreal offsAbove = 0;
            qreal offsBelow = 0;
            offsAbove = -2;
            offsBelow =  4 + (stafflines - 1);
            if (placement == "above")
                  y += offsAbove;
            if (placement == "below")
                  y += offsBelow;
            //qDebug("   y = %g", y);
            y *= el->score()->spatium();
            el->setUserOff(QPoint(0, y));
            }
      else {
            el->setPlacement(placement == "above" ? Placement::ABOVE : Placement::BELOW);
            }
#endif
      if (!placement.isEmpty()) {
            el->setPlacement(placement == "above" ? Placement::ABOVE : Placement::BELOW);
            el->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
            if (!el->isSticking())
                  el->resetProperty(Pid::OFFSET);
            }


      el->setTrack(el->isTempoText() ? 0 : track);    // TempoText must be in track 0
      Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
      s->add(el);
      }

//---------------------------------------------------------
//   calculateTupletDuration
//---------------------------------------------------------

/**
 Calculate the duration of all notes in the tuplet combined
 */

static Fraction calculateTupletDuration(const Tuplet* const t)
      {
      Fraction res;

      for (DurationElement* de : t->elements()) {
            if (de->type() == ElementType::CHORD || de->type() == ElementType::REST) {
                  const ChordRest* cr = static_cast<ChordRest*>(de);
                  const Fraction fraction = cr->ticks(); // TODO : take care of nested tuplets
                  if (fraction.isValid()) {
                        res += fraction;
                        }
                  }
            }
      res /= t->ratio();

      return res;
      }

//---------------------------------------------------------
//   determineTupletBaseLen
//---------------------------------------------------------

/**
 Determine tuplet baseLen as determined by the tuplet ratio
 and duration.
 */

static TDuration determineTupletBaseLen(const Tuplet* const t)
      {
      Fraction tupletFraction;
      Fraction tupletFullDuration;
      determineTupletFractionAndFullDuration(calculateTupletDuration(t), tupletFraction, tupletFullDuration);

      Fraction baseLen = tupletFullDuration * Fraction(1, t->ratio().denominator());
      /*
      qDebug("tupletFraction %s tupletFullDuration %s ratio %s baseLen %s",
             qPrintable(tupletFraction.toString()),
             qPrintable(tupletFullDuration.toString()),
             qPrintable(t->ratio().toString()),
             qPrintable(baseLen.toString())
             );
       */

      return TDuration(baseLen);
      }

//---------------------------------------------------------
//   handleTupletStart
//---------------------------------------------------------

static void handleTupletStart(const ChordRest* const cr, Tuplet*& tuplet,
                              const int actualNotes, const int normalNotes,
                              const MusicXmlTupletDesc& tupletDesc)
      {
      tuplet = new Tuplet(cr->score());
      tuplet->setTrack(cr->track());
      tuplet->setRatio(Fraction(actualNotes, normalNotes));
      tuplet->setTick(cr->tick());
      tuplet->setBracketType(tupletDesc.bracket);
      tuplet->setNumberType(tupletDesc.shownumber);
      tuplet->setDirection(tupletDesc.direction);
      tuplet->setParent(cr->measure());
      }

//---------------------------------------------------------
//   handleTupletStop
//---------------------------------------------------------

static void handleTupletStop(Tuplet*& tuplet, const int normalNotes)
      {
      // allow handleTupletStop to be called w/o error of no tuplet active
      if (!tuplet) return;

      // set baselen
      TDuration td = determineTupletBaseLen(tuplet);
      tuplet->setBaseLen(td);
      Fraction f(normalNotes, td.fraction().denominator());
      f.reduce();
      if (!f.isValid()) {
            qDebug("MusicXML::import: tuplet stop but note values too small");
            tuplet = nullptr;
            return;
            }
      tuplet->setTicks(f);
      // TODO determine usefulness of following check
      int totalDuration = 0;
      int ticksPerNote = f.ticks() / tuplet->ratio().numerator();
      bool ticksCorrect = true;
      for (DurationElement* de : tuplet->elements()) {
            if (de->type() == ElementType::CHORD || de->type() == ElementType::REST) {
                  int globalTicks = de->globalTicks().ticks();
                  if (globalTicks != ticksPerNote)
                        ticksCorrect = false;
                  totalDuration += globalTicks;
                  }
            }
      if (totalDuration != f.ticks())
            qDebug("MusicXML::import: tuplet stop but bad duration"); // TODO
      if (!ticksCorrect)
            qDebug("MusicXML::import: tuplet stop but uneven note ticks"); // TODO
      tuplet = 0;
      }

//---------------------------------------------------------
//   setElementPropertyFlags
//---------------------------------------------------------

static void setElementPropertyFlags(ScoreElement* element, const Pid propertyId,
                                    const QString value1, const QString value2 = QString())
      {
      if (value1.isEmpty()) // Set as an implicit value
            element->setPropertyFlags(propertyId, PropertyFlags::STYLED);
      else if (!value1.isNull() || !value2.isNull()) // Set as an explicit value
            element->setPropertyFlags(propertyId, PropertyFlags::UNSTYLED);
      }

//---------------------------------------------------------
//   addArticulationToChord
//---------------------------------------------------------

static void addArticulationToChord(const Notation& notation, ChordRest* cr)
      {
      const SymId articSym = notation.symId();
      const QString dir = notation.attribute("type");
      const QString place = notation.attribute("placement");
      const QColor color = notation.attribute("color");
      Articulation* na = new Articulation(articSym, cr->score());

      if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
            na->setColor(color);
      if (dir == "up" || dir == "down") {
            na->setUp(dir == "up");
            na->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
            }

      // when setting anchor, assume type up/down without explicit placement
      // implies placement above/below
      if (place == "above" || (dir == "up" && place.isEmpty())) {
            na->setAnchor(ArticulationAnchor::TOP_CHORD);
            na->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
            }
      else if (place == "below" || (dir == "down" && place.isEmpty())) {
            na->setAnchor(ArticulationAnchor::BOTTOM_CHORD);
            na->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
            }

      cr->add(na);
      }

//---------------------------------------------------------
//   addFermataToChord
//---------------------------------------------------------

/**
 Add a MusicXML fermata.
 Note: MusicXML common.mod: "The fermata type is upright if not specified."
 */

static void addFermataToChord(const Notation& notation, ChordRest* cr)
      {
      const SymId articSym = notation.symId();
      const QString direction = notation.attribute("type");
      const QColor color = notation.attribute("color");
      Segment* seg = cr->segment();
      Fermata* na = new Fermata(articSym, cr->score());
      na->setTrack(cr->track());
      if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
            na->setColor(color);
      if (!direction.isEmpty()) {
            na->setPlacement(direction == "inverted" ? Placement::BELOW : Placement::ABOVE);
            na->resetProperty(Pid::OFFSET);
            }
      else
            na->setPlacement(na->propertyDefault(Pid::PLACEMENT).value<Placement>());
      setElementPropertyFlags(na, Pid::PLACEMENT, direction);
      if (!seg && cr->isGrace())
            cr->el().push_back(na);       // store for later move to segment
      else if (seg)
            seg->add(na);

      if (!seg)
            return;

      // Move or hide fermata based on existing fermatas.
      bool alreadyAbove = false;
      bool alreadyBelow = false;
      for (Element* e: seg->annotations()) {
            if (e->isFermata() && e != na
            && e->staffIdx() == na->staffIdx() && e->track() != na->track()) {
                  Element* otherCr = cr->segment()->elist()[e->track()];
                  if (toFermata(e)->placement() == Placement::BELOW) alreadyBelow = true;
                  if (toFermata(e)->placement() == Placement::ABOVE) alreadyAbove = true;

                  if (direction.isEmpty() && alreadyAbove)
                        na->setPlacement(Placement::BELOW);
                  else if (direction.isEmpty() && alreadyBelow)
                        na->setPlacement(Placement::ABOVE);

                  if ((otherCr->isChord() && cr->isChord()
                  && toChord(otherCr)->durationType() == toChord(cr)->durationType())
                  || (alreadyAbove && alreadyBelow))
                        na->setVisible(false);
                  }
            }
      }

//---------------------------------------------------------
//   addMordentToChord
//---------------------------------------------------------

/**
 Add Mordent to Chord.
 */

static void addMordentToChord(const Notation& notation, ChordRest* cr)
      {
      const QString name = notation.name();
      const QString attrLong = notation.attribute("long");
      const QString attrAppr = notation.attribute("approach");
      const QString attrDep = notation.attribute("departure");
      SymId articSym = SymId::noSym; // legal but impossible ArticulationType value here indicating "not found"
      if (name == "inverted-mordent") {
            if ((attrLong.isEmpty() || attrLong.isEmpty()) && attrAppr.isEmpty() && attrDep.isEmpty())
                  articSym = SymId::ornamentShortTrill;
            else if (attrLong == "yes" && attrAppr.isEmpty() && attrDep.isEmpty())
                  articSym = SymId::ornamentTremblement;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep.isEmpty())
                  articSym = SymId::ornamentUpPrall;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep.isEmpty())
                  articSym = SymId::ornamentPrecompMordentUpperPrefix;
            else if (attrLong == "yes" && attrAppr.isEmpty() && attrDep == "below")
                  articSym = SymId::ornamentPrallDown;
            else if (attrLong == "yes" && attrAppr.isEmpty() && attrDep == "above")
                  articSym = SymId::ornamentPrallUp;
            }
      else if (name == "mordent") {
            if ((attrLong.isEmpty() || attrLong == "no") && attrAppr.isEmpty() && attrDep.isEmpty())
                  articSym = SymId::ornamentMordent;
            else if (attrLong == "yes" && attrAppr.isEmpty() && attrDep.isEmpty())
                  articSym = SymId::ornamentPrallMordent;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep.isEmpty())
                  articSym = SymId::ornamentUpMordent;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep.isEmpty())
                  articSym = SymId::ornamentDownMordent;
            }
      if (articSym != SymId::noSym) {
            const QString place = notation.attribute("placement");
            const QColor color = notation.attribute("color");
            Articulation* mordent = new Articulation(cr->score());
            mordent->setSymId(articSym);
            if (place == "above")
                  mordent->setAnchor(ArticulationAnchor::TOP_CHORD);
            else if (place == "below")
                  mordent->setAnchor(ArticulationAnchor::BOTTOM_CHORD);
            else
                  mordent->setAnchor(ArticulationAnchor::CHORD);
            if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  mordent->setColor(color);
            cr->add(mordent);
            }
      else
            qDebug("unknown ornament: name '%s' long '%s' approach '%s' departure '%s'",
                   qPrintable(name), qPrintable(attrLong), qPrintable(attrAppr), qPrintable(attrDep));  // TODO
      }

//---------------------------------------------------------
//   addTurnToChord
//---------------------------------------------------------

/**
 Add Turn to Chord.
 */

static void addTurnToChord(const Notation& notation, ChordRest* cr)
      {
      const SymId turnSym = notation.symId();
      const QColor color = notation.attribute("color");
      const QString place = notation.attribute("placement");
      Articulation* turn =  new Articulation(cr->score());
      turn->setSymId(turnSym);
      if (place == "above")
            turn->setAnchor(ArticulationAnchor::TOP_CHORD);
      else if (place == "below")
            turn->setAnchor(ArticulationAnchor::BOTTOM_CHORD);
      else
            turn->setAnchor(ArticulationAnchor::CHORD);
      if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
            turn->setColor(color);
      cr->add(turn);
      }

//---------------------------------------------------------
//   addOtherOrnamentToChord
//---------------------------------------------------------

/**
 Add Other Ornament to Chord.
 */

static void addOtherOrnamentToChord(const Notation& notation, ChordRest* cr)
      {
      const QString name = notation.name();
      const QString symname = notation.attribute("smufl");
      SymId sym = SymId::noSym;   // legal but impossible ArticulationType value here indicating "not found"
      sym = Sym::name2id(symname);

      if (sym != SymId::noSym) {
            const QColor color = notation.attribute("color");
            Articulation* ornam = new Articulation(cr->score());
            ornam ->setSymId(sym);
            if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  ornam->setColor(color);
            cr->add(ornam);
            }
      else {
            qDebug("unknown ornament: name '%s': '%s'.", qPrintable(name), qPrintable(symname));
            }
      }

//---------------------------------------------------------
//   convertArticulationToSymId
//---------------------------------------------------------

/**
 Convert a MusicXML articulation to a chord as a "simple" MuseScore articulation.
 These are the articulations that can be
 - represented by an enum class SymId
 - added to a ChordRest
 Return true (articulation recognized and converted)
 or false (articulation not recognized).
 Note simple implementation: MusicXML syntax is not strictly
 checked, the articulations parent element does not matter.
 */

static bool convertArticulationToSymId(const QString& mxmlName, SymId& id)
      {
      // map MusicXML notations name to MuseScore symbol
      QMap<QString, SymId> map;
      // ornaments
      map["delayed-turn"]           = SymId::ornamentTurn;
      map["inverted-turn"]          = SymId::ornamentTurnInverted;
      map["vertical-turn"]          = SymId::ornamentTurnUp;
      map["inverted-vertical-turn"] = SymId::ornamentTurnUpS;
      map["turn"]                   = SymId::ornamentTurn;
      map["schleifer"]              = SymId::ornamentPrecompSlide;
      map["haydn"]                  = SymId::ornamentHaydn;
      // articulations
      map["accent"]                 = SymId::articAccentAbove;
      map["strong-accent"]          = SymId::articMarcatoAbove;
      map["staccato"]               = SymId::articStaccatoAbove;
      map["tenuto"]                 = SymId::articTenutoAbove;
      map["detached-legato"]        = SymId::articTenutoStaccatoAbove;
      map["staccatissimo"]          = SymId::articStaccatissimoAbove;
      map["spiccato"]               = SymId::articStaccatissimoStrokeAbove;
      map["stress"]                 = SymId::articStressAbove;
      map["unstress"]               = SymId::articUnstressAbove;
      map["soft-accent"]            = SymId::articSoftAccentAbove;
      // technical
      map["up-bow"]                 = SymId::stringsUpBow;
      map["down-bow"]               = SymId::stringsDownBow;
      map["open-string"]            = SymId::brassMuteOpen;
      map["thumb-position"]         = SymId::stringsThumbPosition ;
      map["double-tongue"]          = SymId::doubleTongueAbove;
      map["triple-tongue"]          = SymId::tripleTongueAbove ;
      map["stopped"]                = SymId::brassMuteClosed;
      map["snap-pizzicato"]         = SymId::pluckedSnapPizzicatoAbove;
      map["heal"]                   = SymId::keyboardPedalHeel1 ;
      map["toe"]                    = SymId::keyboardPedalToe2 ;
      map["fingernails"]            = SymId::pluckedWithFingernails  ;
      map["brass-bend"]             = SymId::brassBend ;
      map["flip"]                   = SymId::brassFlip;
      map["smear"]                  = SymId::brassSmear ;
      map["open"]                   = SymId::brassMuteOpen;

      map["belltree"]               = SymId::handbellsBelltree;
      map["damp"]                   = SymId::handbellsDamp3;
      map["echo"]                   = SymId::handbellsEcho1;
      map["gyro"]                   = SymId::handbellsGyro;
      map["hand martellato"]        = SymId::handbellsHandMartellato;
      map["mallet lift"]            = SymId::handbellsMalletLft;
      map["mallet table"]           = SymId::handbellsMalletBellOnTable;
      map["martellato"]             = SymId::handbellsMartellato;
      map["martellato lift"]        = SymId::handbellsMartellatoLift;
      map["muted martellato"]       = SymId::handbellsMutedMartellato;
      map["pluck lift"]             = SymId::handbellsPluckLift;
      map["swing"]                  = SymId::handbellsSwing;

      if (map.contains(mxmlName)) {
            id = map.value(mxmlName);
            return true;
            }
      else {
            id = SymId::noSym;
            return false;
            }
      }

//---------------------------------------------------------
//   convertFermataToSymId
//---------------------------------------------------------

/**
 Convert a MusicXML fermata name to a MuseScore fermata.
 */

static SymId convertFermataToSymId(const QString& mxmlName)
      {
      QMap<QString, SymId> map; // map MusicXML fermata name to MuseScore symbol
      map["normal"]           = SymId::fermataAbove;
      map["angled"]           = SymId::fermataShortAbove;
      map["square"]           = SymId::fermataLongAbove;
      map["double-angled"]    = SymId::fermataVeryShortAbove;
      map["double-square"]    = SymId::fermataVeryLongAbove;
      map["double-dot"]       = SymId::fermataLongHenzeAbove;
      map["half-curve"]       = SymId::fermataShortHenzeAbove;
      map["curlew"]           = SymId::curlewSign;

      if (map.contains(mxmlName))
            return map.value(mxmlName);
      return SymId::fermataAbove;
      }

//---------------------------------------------------------
//   convertNotehead
//---------------------------------------------------------

/**
 Convert a MusicXML notehead name to a MuseScore headgroup.
 */

static NoteHead::Group convertNotehead(QString mxmlName)
      {
      QMap<QString, int> map; // map MusicXML notehead name to a MuseScore headgroup
      map["slash"] = int(NoteHead::Group::HEAD_SLASH);
      map["triangle"] = int(NoteHead::Group::HEAD_TRIANGLE_UP);
      map["diamond"] = int(NoteHead::Group::HEAD_DIAMOND);
      map["cross"] = int(NoteHead::Group::HEAD_PLUS);
      map["x"] = int(NoteHead::Group::HEAD_CROSS);
      map["circled"] = int(NoteHead::Group::HEAD_CIRCLED);
      map["circle-x"] = int(NoteHead::Group::HEAD_XCIRCLE);
      map["inverted triangle"] = int(NoteHead::Group::HEAD_TRIANGLE_DOWN);
      map["slashed"] = int(NoteHead::Group::HEAD_SLASHED1);
      map["back slashed"] = int(NoteHead::Group::HEAD_SLASHED2);
      map["normal"] = int(NoteHead::Group::HEAD_NORMAL);
      map["rectangle"] = int(NoteHead::Group::HEAD_LA);
      map["do"] = int(NoteHead::Group::HEAD_DO);
      map["re"] = int(NoteHead::Group::HEAD_RE);
      map["mi"] = int(NoteHead::Group::HEAD_MI);
      map["fa"] = int(NoteHead::Group::HEAD_FA);
      map["fa up"] = int(NoteHead::Group::HEAD_FA);
      map["so"] = int(NoteHead::Group::HEAD_SOL);
      map["la"] = int(NoteHead::Group::HEAD_LA);
      map["ti"] = int(NoteHead::Group::HEAD_TI);

      if (map.contains(mxmlName))
            return NoteHead::Group(map.value(mxmlName));
      else
            qDebug("unknown notehead %s", qPrintable(mxmlName));  // TODO
      // default: return 0
      return NoteHead::Group::HEAD_NORMAL;
      }

//---------------------------------------------------------
//   addTextToNote
//---------------------------------------------------------

/**
 Add Text to Note.
 */

static void addTextToNote(long l, long c, QString txt, QString placement, QString fontWeight,
                          qreal fontSize, QString fontStyle, QString fontFamily, QColor color, Tid subType, Score* score, Note* note)
      {
      if (note) {
            if (!txt.isEmpty()) {
                  TextBase* t = new Fingering(score, subType);
                  t->setPlainText(txt);

                  bool needUseDefaultFont = preferences.getBool(PREF_MIGRATION_APPLY_EDWIN_FOR_XML_FILES);

                  if (!fontFamily.isEmpty() && !needUseDefaultFont) {
                        t->setFamily(fontFamily);
                        t->setPropertyFlags(Pid::FONT_FACE, PropertyFlags::UNSTYLED);
                        }
                  if (std::isnormal(fontSize) && fontSize > 0.0) {
                        t->setSize(fontSize);
                        t->setPropertyFlags(Pid::FONT_SIZE, PropertyFlags::UNSTYLED);
                        }
                  if (!fontWeight.isEmpty()) {
                        t->setBold(fontWeight == "bold");
                        t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
                        }
                  if (!fontStyle.isEmpty()) {
                        t->setItalic(fontStyle == "italic");
                        t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
                        }
                  if (!placement.isEmpty()) {
                        t->setPlacement(placement == "below" ? Placement::BELOW : Placement::ABOVE);
                        t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                        t->resetProperty(Pid::OFFSET);
                        }
                  if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/) {
                        t->setColor(color);
                        t->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
                        }
                  note->add(t);
                  }
            }
      else
            qDebug("%s", qPrintable(QString("Error at line %1 col %2: no note for text").arg(l).arg(c)));       // TODO
      }

//---------------------------------------------------------
//   setSLinePlacement
//---------------------------------------------------------

/**
 Helper for direction().
 SLine placement is modified by changing the first segment's user offset
 As the SLine has just been created, it does not have any segment yet
 */

static void setSLinePlacement(SLine* sli, const QString placement)
      {
      /*
       qDebug("setSLinePlacement sli %p type %d s=%g pl='%s'",
       sli, sli->type(), sli->score()->spatium(), qPrintable(placement));
       */

#if 0
      // calc y offset assuming five line staff and default style
      // note that required y offset is element type dependent
      if (sli->type() == ElementType::HAIRPIN) {
            if (placement == "above") {
                  const qreal stafflines = 5;       // assume five line staff, but works OK-ish for other sizes too
                  qreal offsAbove = -6 - (stafflines - 1);
                  qreal y = 0;
                  y +=  offsAbove;
                  // add linesegment containing the user offset
                  LineSegment* tls = sli->createLineSegment();
                  //qDebug("   y = %g", y);
                  tls->setAutoplace(false);
                  y *= sli->score()->spatium();
                  tls->setUserOff2(QPointF(0, y));
                  sli->add(tls);
                  }
            }
      else {
            sli->setPlacement(placement == "above" ? Placement::ABOVE : Placement::BELOW);
            }
#endif
      if (placement == "above" || placement == "below") {
            sli->setPlacement(placement == "above" ? Placement::ABOVE : Placement::BELOW);
            sli->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::NOSTYLE);
            sli->resetProperty(Pid::OFFSET);
            }
      }

//---------------------------------------------------------
//   handleSpannerStart
//---------------------------------------------------------

// note that in case of overlapping spanners, handleSpannerStart is called for every spanner
// as spanners QMap allows only one value per key, this does not hurt at all

static void handleSpannerStart(SLine* new_sp, int track, QString placement, const Fraction& tick, MusicXmlSpannerMap& spanners)
      {
      //qDebug("handleSpannerStart(sp %p, track %d, tick %s (%d))", new_sp, track, qPrintable(tick.print()), tick.ticks());
      new_sp->setTrack(track);
      setSLinePlacement(new_sp, placement);
      spanners[new_sp] = QPair<int, int>(tick.ticks(), -1);
      }

//---------------------------------------------------------
//   handleSpannerStop
//---------------------------------------------------------

static void handleSpannerStop(SLine* cur_sp, int track2, const Fraction& tick, MusicXmlSpannerMap& spanners)
      {
      //qDebug("handleSpannerStop(sp %p, track2 %d, tick %s (%d))", cur_sp, track2, qPrintable(tick.print()), tick.ticks());
      if (!cur_sp)
            return;

      cur_sp->setTrack2(track2);
      spanners[cur_sp].second = tick.ticks();
      }

//---------------------------------------------------------
//   The MusicXML parser, pass 2
//---------------------------------------------------------

//---------------------------------------------------------
//   MusicXMLParserPass2
//---------------------------------------------------------

MusicXMLParserPass2::MusicXMLParserPass2(Score* score, MusicXMLParserPass1& pass1, MxmlLogger* logger)
      : _divs(0), _score(score), _pass1(pass1), _logger(logger)
      {
      // nothing
      }

//---------------------------------------------------------
//   addError
//---------------------------------------------------------

void MusicXMLParserPass2::addError(const QString& error)
      {
      if (!error.isEmpty()) {
            _logger->logError(error, &_e);
            QString errorWithLocation = xmlReaderLocation(_e) + ' ' + error + '\n';
            _errors += errorWithLocation;
            }
      }

//---------------------------------------------------------
//   setChordRestDuration
//---------------------------------------------------------

/**
 * Set \a cr duration
 */

static void setChordRestDuration(ChordRest* cr, TDuration duration, const Fraction dura)
      {
      if (duration.type() == TDuration::DurationType::V_MEASURE) {
            cr->setDurationType(duration);
            cr->setTicks(dura);
            }
      else {
            cr->setDurationType(duration);
            cr->setTicks(cr->durationType().fraction());
            }
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

/**
 * Add a rest to the score
 * TODO: beam handling
 * TODO: display step handling
 * TODO: visible handling
 * TODO: whole measure rest handling
 */

static Rest* addRest(Score* score, Measure* m,
                     const Fraction& tick, const int track, const int move,
                     const TDuration duration, const Fraction dura)
      {
      Segment* s = m->getSegment(SegmentType::ChordRest, tick);
      // Sibelius might export two rests at the same place, ignore the 2nd one
      // <?DoletSibelius Two NoteRests in same voice at same position may be an error?>
      // Same issue may result from trying to import incomplete tuplets
      if (s->element(track)) {
            qDebug("cannot add rest at tick %s (%d) track %d: element already present", qPrintable(tick.print()), tick.ticks(), track); // TODO
            return nullptr;
            }

      Rest* cr = new Rest(score);
      setChordRestDuration(cr, duration, dura);
      cr->setTrack(track);
      cr->setStaffMove(move);
      s->add(cr);
      return cr;
      }

//---------------------------------------------------------
//   resetTuplets
//---------------------------------------------------------

static void resetTuplets(Tuplets& tuplets)
      {
      for (auto& pair : tuplets) {
            Tuplet* tuplet = pair.second;
            if (tuplet) {
                  const Fraction actualDuration = tuplet->elementsDuration() / tuplet->ratio();
                  const Fraction missingDuration = missingTupletDuration(actualDuration);
                  qDebug("tuplet %p not stopped at end of measure, tick %s duration %s missing %s",
                         tuplet,
                         qPrintable(tuplet->tick().print()),
                         qPrintable(actualDuration.print()), qPrintable(missingDuration.print()));
                  if (actualDuration > Fraction(0, 1) && missingDuration > Fraction(0, 1)) {
                        qDebug("add missing %s to previous tuplet", qPrintable(missingDuration.print()));
                        const DurationElement* firstElement = tuplet->elements().at(0);
                        // appended the rest to the current end of the tuplet (firstElement->tick() + actualDuration)
                        Rest* const extraRest = addRest(firstElement->score(), firstElement->measure(), firstElement->tick() + actualDuration, firstElement->track(), 0,
                                                       TDuration { missingDuration* tuplet->ratio() }, missingDuration);
                        if (extraRest) {
                              extraRest->setTuplet(tuplet);
                              tuplet->add(extraRest);
                              }
                        }
                  const int normalNotes = tuplet->ratio().denominator();
                  handleTupletStop(tuplet, normalNotes);
                  }
            }
      }

//---------------------------------------------------------
//   cleanFretDiagrams
//---------------------------------------------------------
/**
 PVG scores sometimes display fretboards for all chords at
 the beginning. These often fail to translate correctly to
 MusicXML, so we delete them here.
 */

static void cleanFretDiagrams(Measure* measure)
      {
      if (!measure || measure->no() > 0)
            return;
      // Case 1: Dummy hidden first measure with all fretboards attached
      bool isDummyMeasure = toMeasureBase(measure)->lineBreak();
      for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            if (!isDummyMeasure) break;
            for (Element* e : s->elist()) {
                  if (e && e->isChord() && e->visible()) {
                        isDummyMeasure = false;
                        break;
                        }
                  }
            }
      if (isDummyMeasure) {
            for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                  for (Element* e : s->annotations()) {
                        if (e->isFretDiagram()) {
                              s->remove(e);
                              delete e;
                              }
                        }
                  }
            }

      // Case 2: All the fretboards attached to first beat
      Segment* firstBeat = measure->first(SegmentType::ChordRest);
      QList<FretDiagram*> beat1FretDiagrams;
      int fretDiagramsTrack = -1;
      for (Element* e : firstBeat->annotations()) {
            if (e->isFretDiagram()
            && (fretDiagramsTrack == e->track() || fretDiagramsTrack == -1)) {
                  beat1FretDiagrams.append(toFretDiagram(e));
                  fretDiagramsTrack = e->track();
                  }
            }
      if (beat1FretDiagrams.length() > 1 && fretDiagramsTrack != -1) {
            for (FretDiagram* fd : beat1FretDiagrams) {
                  firstBeat->remove(fd);
                  delete fd;
                  }
            }
      }

//---------------------------------------------------------
//   initPartState
//---------------------------------------------------------

/**
 Initialize members as required for reading the MusicXML part element.
 TODO: factor out part reading into a separate class
 TODO: preferably use automatically initialized variables
 Note that Qt automatically initializes new elements in QVector (tuplets).
 */

void MusicXMLParserPass2::initPartState(const QString& partId)
      {
      Q_UNUSED(partId);
      _timeSigDura = Fraction(0, 0);             // invalid
      _ties.clear();
      _unstartedTieNotes.clear();
      _unendedTieNotes.clear();
      _lastVolta = 0;
      _hasDrumset = false;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _slurs[i] = SlurDesc();
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _trills[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _glissandi[i][0] = _glissandi[i][1] = 0;
      _pedalContinue = 0;
      _harmony = 0;
      _tremStart = 0;
      _figBass = 0;
      _delayedOttava = 0;
      _multiMeasureRestCount = -1;
      _measureStyleSlash = MusicXmlSlash::NONE;
      _extendedLyrics.init();
      _graceNoteLyrics.clear();
      }

//---------------------------------------------------------
//   findIncompleteSpannersInStack
//---------------------------------------------------------

static void findIncompleteSpannersInStack(const QString& spannerType, SpannerStack& stack, SpannerSet& res)
      {
      for (MusicXmlExtendedSpannerDesc& desc : stack) {
            if (desc._sp) {
                  qDebug("%s not terminated at end of part", qPrintable(spannerType));
                  res.insert(desc._sp);
                  desc = {};
                  }
            }
      }

//---------------------------------------------------------
//   findIncompleteSpannersAtPartEnd
//---------------------------------------------------------

SpannerSet MusicXMLParserPass2::findIncompleteSpannersAtPartEnd()
      {
      SpannerSet res;
      findIncompleteSpannersInStack("bracket", _brackets, res);
      findIncompleteSpannersInStack("wedge", _hairpins, res);
      findIncompleteSpannersInStack("octave-shift", _ottavas, res);
      if (_pedal._sp) {
            qDebug("pedal not terminated at end of part");
            res.insert(_pedal._sp);
            _pedal = {};
            }
      return res;
      }

//---------------------------------------------------------
//   addArticLaissezVibrer
//---------------------------------------------------------

static void addArticLaissezVibrer(const Note* const note)
      {
      if (!note)
            return;
      Chord* chord = note->chord();
      if (!hasLaissezVibrer(chord)) {
            Articulation* na = new Articulation(SymId::articLaissezVibrerBelow, chord->score());
            chord->add(na);
            }

      }

//---------------------------------------------------------
//   cleanupUnterminatedTie
//---------------------------------------------------------
/**
 Delete tie and add Laissez Vibrer where it was
 */

static void cleanupUnterminatedTie(Tie*& tie, const Score* score, bool fixForCrossStaff = false)
      {
      Note* unterminatedTieNote = tie->startNote();
      const Chord* unterminatedChord = unterminatedTieNote->chord();

      // Dolet 6 doesn't export cross staff information
      // If a tie is unterminated, try to find a candidate to tie it to on a different track/stave
      if (fixForCrossStaff) {
            const Segment* nextSeg = score->tick2leftSegment(unterminatedChord->tick() + unterminatedChord->ticks());
            if (nextSeg) {
                  const Part* part = unterminatedTieNote->part();
                  for (int track = part->startTrack(); track <= part->endTrack(); ++track) {
                        const Element* el = nextSeg->element(track);
                        if (el && el->isChord()) {
                              Note* matchingNote = toChord(el)->findNote(unterminatedTieNote->pitch());
                              if (matchingNote && matchingNote->tpc() == unterminatedTieNote->tpc()) {
                                    tie->setEndNote(matchingNote);
                                    matchingNote->setTieBack(tie);
                                    return;
                                    }
                              }
                        }
                  }
            }

      // Delete unterminated ties pending fully featured l.v. ties & ties over repeats
      unterminatedTieNote->remove(tie);
      delete tie;
      }

//---------------------------------------------------------
//   isLikelyIncorrectPartName
//---------------------------------------------------------
/**
 Sibelius exports part names of the form "P#" rather than
 specifying print-object="no". This finds those.
 */

static bool isLikelyIncorrectPartName(const QString& partName)
      {
      static const QRegularExpression re("^P[0-9]+$");
      return partName.contains(re);
      }

//---------------------------------------------------------
//   detectLayoutOverflow
//---------------------------------------------------------

static void detectLayoutOverflow(Score* score, bool& hasLineOverflow, bool& hasPageOverflow)
      {
      hasLineOverflow = false;
      hasPageOverflow = false;
      for (auto pp = score->pages().begin(); pp != score->pages().end() - 1; ++pp) {
            Page* page = *pp;
            Page* nextPage = *(pp + 1);
            MeasureBase* lastMeasureOfPage = page->systems().back() ? page->systems().back()->lastMeasure() : 0;

            if (!hasPageOverflow
                && lastMeasureOfPage
                && !lastMeasureOfPage->pageBreak()
                && nextPage
                && nextPage->systems().size() < 2)
                  hasPageOverflow = true;

            for (auto sp = page->systems().begin(); sp != page->systems().end() - 1; ++sp) {
                  if (hasPageOverflow && hasLineOverflow)
                        return;
                  System* system = *sp;
                  System* nextSystem = *(sp + 1);
                  if (!hasLineOverflow
                      && system->lastMeasure()
                      && !system->lastMeasure()->lineBreak() && !system->lastMeasure()->pageBreak()
                      && nextSystem
                      && nextSystem->measures().size() < 2)
                        hasLineOverflow = true;
                 }
            }
      }

//---------------------------------------------------------
//   cleanUpLayoutBreaks
//---------------------------------------------------------
/**
 Some scores may have explicit system or page breaks that don't
 correctly lay out when rendered in MuseScore. This function detects
 these scenarios and tweaks the layout, margin and font-size compromises
 to incorrect system/page breaks. Returns whether changes have been made.
 Currently, if this doesn't fix all line and page overflows it reverts
 any changes it makes. Possible TODO: accept partial fixes
 (i.e. count the *number* of overflows and accept the changes
 if the number is reduced).
 To consider: making this a member function of Score and moving it
 to layout.cpp. It could be a nice utility for users.
 */

static bool cleanUpLayoutBreaks(Score* score, MxmlLogger* logger)
      {
      score->doLayout();

      bool hasExplicitLineBreaks = false;
      bool hasExplicitPageBreaks = false;
      bool hasLineOverflow;
      bool hasPageOverflow;

      std::map<Sid, QVariant> initialStyles;
      VBox* copyrightVBox = 0;
      bool copyrightVBoxAutoSizeEnabledInitial = true;
      Spatium copyrightVBoxBoxHeightInitial = Spatium(10);

      for (const auto& system : qAsConst(score->systems())) {
            if (!system->lastMeasure())
                  continue;
            else if (system->lastMeasure()->lineBreak() && !hasExplicitLineBreaks)
                  hasExplicitLineBreaks = true;
            else if (system->lastMeasure()->pageBreak() && !hasExplicitPageBreaks)
                  hasExplicitPageBreaks = true;

            if (hasExplicitLineBreaks && hasExplicitPageBreaks)
                  break;
            }

      if (!hasExplicitLineBreaks || !hasExplicitPageBreaks) {
            logger->logDebugInfo("No explicit layout breaks. Skipping layout break cleanup.");
            return false;
            }

      detectLayoutOverflow(score, hasLineOverflow, hasPageOverflow);
      if (!hasLineOverflow && !hasPageOverflow)
            return false;
      bool initialHasLineOverflow = hasLineOverflow;
      bool initialHasPageOverflow = hasPageOverflow;

      // First compromise margins: reduce to 80%
      const qreal marginReductionFactor = 0.8;  // Possible TODO: make this incremental and customizable
      if (hasLineOverflow) {
            const std::vector<Sid> horizontalMarginSids({Sid::pageEvenLeftMargin, Sid::pageOddLeftMargin});
            for (Sid marginSid : horizontalMarginSids) {
                  qreal initialMargin = score->style().value(marginSid).toReal();
                  initialStyles[marginSid] = initialMargin;
                  score->style().set(marginSid, QVariant(initialMargin * marginReductionFactor));
                  }
            initialStyles[Sid::pagePrintableWidth] = score->styleD(Sid::pagePrintableWidth);
            score->style().set(Sid::pagePrintableWidth, score->styleD(Sid::pageWidth) - (2 * score->styleD(Sid::pageEvenLeftMargin)));
            score->styleChanged();
            score->doLayout();
            detectLayoutOverflow(score, hasLineOverflow, hasPageOverflow);
            }
      if (hasPageOverflow) {
            const std::vector<Sid> verticalMarginSids({Sid::pageEvenTopMargin, Sid::pageOddTopMargin,
                                                  Sid::pageEvenBottomMargin, Sid::pageOddBottomMargin});
            for (Sid marginSid : verticalMarginSids) {
                  qreal initialMargin = score->styleD(marginSid);
                  initialStyles[marginSid] = initialMargin;
                  score->style().set(marginSid, QVariant(initialMargin * marginReductionFactor));
                  }
            score->styleChanged();
            score->doLayout();
            detectLayoutOverflow(score, hasLineOverflow, hasPageOverflow);
            }

      // Next: collapse copyright VBox to 20% of height, if present
      MeasureBase* copyrightVBoxCandidate = score->pages().begin() != score->pages().end() && score->pages().begin() + 1 != score->pages().end() ? (*(score->pages().begin() + 1))->systems().first()->measure(0) : 0;
      if (hasPageOverflow && copyrightVBoxCandidate && copyrightVBoxCandidate->isVBox()) {
            copyrightVBox = toVBox(copyrightVBoxCandidate);
            copyrightVBoxAutoSizeEnabledInitial = copyrightVBox->isAutoSizeEnabled();
            copyrightVBox->setAutoSizeEnabled(false);
            copyrightVBoxBoxHeightInitial = copyrightVBox->boxHeight();
            copyrightVBox->setBoxHeight(copyrightVBoxBoxHeightInitial * 0.2);
            copyrightVBox->setPropertyFlags(Pid::BOX_HEIGHT, PropertyFlags::UNSTYLED);
            score->styleChanged();
            score->doLayout();
            detectLayoutOverflow(score, hasLineOverflow, hasPageOverflow);
            }

      // If that doesn't fix it, compromise lyric font size: reduce by 95%
      if (hasLineOverflow) {
            const qreal lyricFontSizeReductionFactor = 0.95;
            const std::vector<Sid> fontSizeSids({Sid::lyricsOddFontSize, Sid::lyricsEvenFontSize});
            for (Sid fontSizeSid : fontSizeSids) {
                  qreal initialFontSize = score->style().value(fontSizeSid).toReal();
                  initialStyles[fontSizeSid] = initialFontSize;
                  score->style().set(fontSizeSid, QVariant(initialFontSize * lyricFontSizeReductionFactor));
                  }
            score->styleChanged();
            score->doLayout();
            detectLayoutOverflow(score, hasLineOverflow, hasPageOverflow);
            }

      // If neither fixed it, reset styles.
      if (hasLineOverflow != initialHasLineOverflow
          || hasPageOverflow != initialHasPageOverflow)
            return true;
      else {
            for (const auto& initialStyle : initialStyles)
                  score->style().set(initialStyle.first, initialStyle.second);
            if (copyrightVBox) {
                  copyrightVBox->setAutoSizeEnabled(copyrightVBoxAutoSizeEnabledInitial);
                  copyrightVBox->setBoxHeight(copyrightVBoxBoxHeightInitial);
                  }
            score->styleChanged();
            score->doLayout();
            logger->logDebugInfo("Attempts to cleanup layout breaks had no effect. Reverting.");
            return false;
            }
      }

//---------------------------------------------------------
// multi-measure rest state handling
//---------------------------------------------------------

// If any multi-measure rest is found, the "create multi-measure rest" style setting is enabled.
// First measure in a multi-measure rest gets setBreakMultiMeasureRest(true), then count down
// the remaining number of measures.
// The first measure after a multi-measure rest gets setBreakMultiMeasureRest(true).
// For all other measures breakMultiMeasureRest is unchanged (stays default (false)).

//---------------------------------------------------------
//   setMultiMeasureRestCount
//---------------------------------------------------------

/**
 Set the multi-measure rest counter.
 */


void MusicXMLParserPass2::setMultiMeasureRestCount(int count)
      {
      _multiMeasureRestCount = count;
      }

//---------------------------------------------------------
//   getAndDecMultiMeasureRestCount
//---------------------------------------------------------

/**
 Return current multi-measure rest counter.
 Decrement counter if possible (not beyond -1).
 */

int MusicXMLParserPass2::getAndDecMultiMeasureRestCount()
      {
      int res = _multiMeasureRestCount;
      if (_multiMeasureRestCount >= 0)
            _multiMeasureRestCount--;
      return res;
      }

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXMLParserDirection::skipLogCurrElem()
      {
      //_logger->logDebugInfo(QString("skipping '%1'").arg(_e.name().toString()), &_e);
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXMLParserPass2::skipLogCurrElem()
      {
      //_logger->logDebugInfo(QString("skipping '%1'").arg(_e.name().toString()), &_e);
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Parse MusicXML in \a device and extract pass 2 data.
 */

Score::FileError MusicXMLParserPass2::parse(QIODevice* device)
      {
      //qDebug("MusicXMLParserPass2::parse()");
      _e.setDevice(device);
      Score::FileError res = parse();
      //qDebug("MusicXMLParserPass2::parse() res %d", int(res));
      return res;
      }

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Start the parsing process, after verifying the top-level node is score-partwise
 */

Score::FileError MusicXMLParserPass2::parse()
      {
      bool found = false;
      while (_e.readNextStartElement()) {
            if (_e.name() == "score-partwise") {
                  found = true;
                  scorePartwise();
                  }
            else {
                  _logger->logError("this is not a MusicXML score-partwise file", &_e);
                  _e.skipCurrentElement();
                  return Score::FileError::FILE_BAD_FORMAT;
                  }
            }

      if (!found) {
            _logger->logError("this is not a MusicXML score-partwise file", &_e);
            return Score::FileError::FILE_BAD_FORMAT;
            }

      return Score::FileError::FILE_NO_ERROR;
      }

//---------------------------------------------------------
//   createBarline
//---------------------------------------------------------

/*
 * Create a barline of the specified type.
 */

static std::unique_ptr<BarLine> createBarline(Score* score, const int track, const BarLineType type, const bool visible,
                                              const QString& barStyle, int spanStaff)
      {
      std::unique_ptr<BarLine> barline(new BarLine(score));
      barline->setTrack(track);
      barline->setBarLineType(type);
      barline->setSpanStaff(spanStaff);
      barline->setVisible(visible);
      if (barStyle == "tick") {
            barline->setSpanFrom(BARLINE_SPAN_TICK1_FROM);
            barline->setSpanTo(BARLINE_SPAN_TICK1_TO);
            }
      else if (barStyle == "short") {
            barline->setSpanFrom(BARLINE_SPAN_SHORT1_FROM);
            barline->setSpanTo(BARLINE_SPAN_SHORT1_TO);
            }
      return barline;
      }

//---------------------------------------------------------
//   addBarlineToMeasure
//---------------------------------------------------------

/*
 * Add barline to the measure at tick.
 */

static void addBarlineToMeasure(Measure* measure, const Fraction tick, std::unique_ptr<BarLine> barline)
      {
      SegmentType st = SegmentType::BarLine;
      if (tick == measure->endTick())
            st = SegmentType::EndBarLine;
      else if (tick == measure->tick())
            st = SegmentType::BeginBarLine;
      Segment* const segment = measure->getSegment(st, tick);
      barline->layout();
      segment->add(barline.release());
      }

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Parse the MusicXML top-level (XPath /score-partwise) node.
 */

void MusicXMLParserPass2::scorePartwise()
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "part") {
                  part();
                  }
            else if (_e.name() == "part-list")
                  partList();
            else
                  skipLogCurrElem();
            }
      // set last measure barline to normal or MuseScore will generate light-heavy EndBarline
      // this creates non-generated barlines spanning only the current instrument
      // BarLine::_spanStaff is set using the default in Staff::_barLineSpan
      Measure* const lm = _score->lastMeasure();
      if (lm && lm->endBarLineType() == BarLineType::NORMAL) {
            for (int staffidx = 0; staffidx < _score->nstaves(); ++staffidx) {
                  const Staff* staff = _score->staff(staffidx);
                  auto b = createBarline(_score, staffidx * VOICES, BarLineType::NORMAL, true, "", staff->barLineSpan());
                  addBarlineToMeasure(lm, lm->endTick(), std::move(b));
                  }
           }

      _score->connectArpeggios();
      _score->fixupLaissezVibrer();
      cleanFretDiagrams(_score->firstMeasure());

      cleanUpLayoutBreaks(_score, _logger);

      addError(checkAtEndElement(_e, "score-partwise"));
      }

//---------------------------------------------------------
//   partList
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list node.
 */

void MusicXMLParserPass2::partList()
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "score-part")
                  scorePart();
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   scorePart
//---------------------------------------------------------

// Parse the /score-partwise/part-list/score-part node.
// TODO: nothing required for pass 2 ?

void MusicXMLParserPass2::scorePart()
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "midi-instrument")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "score-instrument")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "part-name")
                  _e.skipCurrentElement();  // skip but don't log
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   part
//---------------------------------------------------------

/**
 Parse the /score-partwise/part node.
 */

void MusicXMLParserPass2::part()
      {
      const QString id = _e.attributes().value("id").toString();

      if (!_pass1.hasPart(id)) {
            _logger->logError(QString("MusicXMLParserPass2::part cannot find part '%1'").arg(id), &_e);
            skipLogCurrElem();
            }

      initPartState(id);

      const MusicXMLInstruments& instruments = _pass1.getInstruments(id);
      _hasDrumset = hasDrumset(instruments);
      MusicXmlPart mxmlPart = _pass1.getMusicXmlPart(id);
      Part* msPart = _pass1.getPart(id);

      // set the parts first instrument

      if (_pass1.supportsTranspose() == "no")
            _pass1.addInferredTranspose(id);
      setPartInstruments(_logger, &_e, msPart, id, _score, _pass1.getInstrList(id), _pass1.getIntervals(id), instruments);

      // set the part name
      msPart->setPartName(mxmlPart.getName());
      if (mxmlPart.getPrintName() && !isLikelyIncorrectPartName(mxmlPart.getName()))
            msPart->setLongNameAll(mxmlPart.getName());
      else
            msPart->setLongNameAll(""); // Delete possibly inferred names in setPartInstruments
      if (mxmlPart.getPrintAbbr())
            msPart->setPlainShortNameAll(mxmlPart.getAbbr());
      else
            msPart->setPlainShortNameAll(""); // Delete possibly inferred names in setPartInstruments
      // try to prevent an empty track name
      if (msPart->partName().isEmpty()) {
            QString instrId = _pass1.getInstrList(id).instrument(Fraction(0, 1));
            msPart->setPartName(instruments[instrId].name);
            }

#ifdef DEBUG_VOICE_MAPPER
      VoiceList voicelist = _pass1.getVoiceList(id);
      // debug: print voice mapper contents
      qDebug("voiceMapperStats: part '%s'", qPrintable(id));
      for (QMap<QString, Ms::VoiceDesc>::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
            qDebug("voiceMapperStats: voice %s staff data %s",
                   qPrintable(i.key()), qPrintable(i.value().toString()));
            }
#endif

      // read the measures
      int nr = 0; // current measure sequence number (always increments by one for each measure)
      _measureNumber = 0; // written measure number (doesn't always increment by 1)
      while (_e.readNextStartElement()) {
            if (_e.name() == "measure") {
                  Fraction t = _pass1.getMeasureStart(nr);
                  if (t.isValid())
                        measure(id, t);
                  else {
                        _logger->logError(QString("no valid start time for measure %1").arg(nr + 1), &_e);
                        _e.skipCurrentElement();
                        }
                  ++nr;
                  }
            else
                  skipLogCurrElem();
            }

      // stop all remaining extends for this part and add remaining ottava if present
      Measure* lm = msPart->score()->lastMeasure();
      if (lm) {
            int strack = _pass1.trackForPart(id);
            int etrack = strack + msPart->nstaves() * VOICES;
            Fraction lastTick = lm->endTick();
            for (int trk = strack; trk < etrack; trk++)
                  _extendedLyrics.setExtend(-1, trk, lastTick);
            if (_delayedOttava && _delayedOttava->tick2() < lastTick) {
                  handleSpannerStop(_delayedOttava, _delayedOttava->track2(), lastTick, _spanners);
                 _delayedOttava = nullptr;
                  }
            }

      if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE)) {
            for (Hairpin* hp : _inferredHairpins)
                  hp->score()->addElement(hp);
            }

      const SpannerSet incompleteSpanners =  findIncompleteSpannersAtPartEnd();
      //qDebug("spanner list:");
      auto i = _spanners.constBegin();
      while (i != _spanners.constEnd()) {
            SLine* const sp = i.key();
            Fraction tick1 = Fraction::fromTicks(i.value().first);
            Fraction tick2 = Fraction::fromTicks(i.value().second);
            if (sp->isPedal() && toPedal(sp)->endHookType() == HookType::HOOK_45) {
                  // Handle pedal change end tick (slightly hacky)
                  // Find CR on the end tick of
                  ChordRest* terminatingCR = _score->findCR(tick2, sp->effectiveTrack2());
                  for (int track = msPart->startTrack(); track <= msPart->endTrack(); ++track) {
                        ChordRest* tempCR = _score->findCR(tick2, track);
                        if (!terminatingCR
                        || (tempCR && tempCR->tick() > terminatingCR->tick())
                        || (tempCR && tempCR->tick() == terminatingCR->tick() && tempCR->ticks() < terminatingCR->ticks()))
                              terminatingCR = tempCR;
                        }
                  tick2 += terminatingCR->ticks();
                  sp->setTrack2(terminatingCR->track());
                  }
            //qDebug("spanner %p tp %d tick1 %s tick2 %s track1 %d track2 %d",
            //       sp, sp->type(), qPrintable(tick1.print()), qPrintable(tick2.print()), sp->track(), sp->track2());
            if (incompleteSpanners.find(sp) == incompleteSpanners.end()) {
                  // complete spanner -> add to score
                  sp->setTick(tick1);
                  sp->setTick2(tick2);
                  sp->score()->addElement(sp);
                  }
            else {
                  // incomplete spanner -> cleanup
                  delete sp;
                  }
            ++i;
            }
      _spanners.clear();

      // Clean up any remaining ties
      MusicXMLTieMap tieMap = _ties;
      for (auto& tie : tieMap) {
            if (tie.second) {
                  _unendedTieNotes.push_back(tie.second->startNote());
                  _ties.erase(tie.first);
                  }
            }
      // Find ties between different voices which may have been missed
      for (Note* startNote : _unendedTieNotes) {
            Tie* unendedTie = startNote->tieFor();
            if (!unendedTie)
                  continue;
            const Chord* startChord = startNote->chord();
            const Measure* startMeasure = startChord ? startChord->measure() : nullptr;
            for (Note* endNote : _unstartedTieNotes) {
                  if (endNote->tieBack())
                        continue;
                  const Chord* endChord = endNote->chord();
                  if (startChord && startNote->pitch() == endNote->pitch()
                      && endChord && (startMeasure == endChord->measure() || startChord->tick() + startChord->actualTicks() == endChord->tick())) {
                        unendedTie->setEndNote(endNote);
                        endNote->setTieBack(unendedTie);
                        }
                  }

            if (startChord && !unendedTie->endNote()) {
                  // Tie started with no matching end tags
                  // Find a note of the same pitch in the same voice immediately following the start chord
                  Segment* nextSeg = startChord->segment();
                  while (nextSeg && nextSeg->tick() < startChord->tick() + startChord->ticks())
                        nextSeg = nextSeg->nextCR(startChord->track(), true);
                  Element* nextEl = nextSeg ? nextSeg->element(startNote->track()) : nullptr;
                  Chord* nextChord = nextEl && nextEl->isChord() ? toChord(nextEl) : nullptr;
                  Note* matchingNote = nextChord ? nextChord->findNote(startNote->pitch()) : nullptr;

                  if (matchingNote && matchingNote->tpc() == startNote->tpc() && matchingNote != startNote) {
                        unendedTie->setEndNote(matchingNote);
                        matchingNote->setTieBack(unendedTie);
                        }
                  else {
                        // try other voices in the stave
                        const Part* part = startChord->part();
                        for (int track = part->startTrack(); track < part->endTrack() + VOICES; track++) {
                              nextEl = nextSeg ? nextSeg->element(track) : nullptr;
                              nextChord = nextEl && nextEl->isChord() ? toChord(nextEl) : nullptr;
                              if (nextChord && nextChord->vStaffIdx() != startChord->vStaffIdx())
                                    continue;
                              matchingNote = nextChord ? nextChord->findNote(startNote->pitch()) : nullptr;
                              if (matchingNote && matchingNote->tpc() == startNote->tpc() && matchingNote != startNote) {
                                    unendedTie->setEndNote(matchingNote);
                                    matchingNote->setTieBack(unendedTie);
                                    }
                              }
                        }
                  }

            if (!unendedTie->endNote())
                  cleanupUnterminatedTie(unendedTie, _score, _pass1.exporterSoftware() == MusicXMLExporterSoftware::DOLET6);
            }

      _ties.clear();
      _unstartedTieNotes.clear();
      _unendedTieNotes.clear();

      if (_hasDrumset) {
            Drumset* drumset = new Drumset;
            const MusicXMLInstruments& instrumentsAfterPass2 = _pass1.getInstruments(id);
            initDrumset(drumset, instrumentsAfterPass2);
            // set staff type to percussion if incorrectly imported as pitched staff
            // Note: part has been read, staff type already set based on clef type and staff-details
            // but may be incorrect for a percussion staff that does not use a percussion clef
            setStaffTypePercussion(msPart, drumset);
            }

      bool showPart = false;
            for (const Staff* staff : qAsConst(*msPart->staves())) {
            if (!staff->invisible(Fraction(0,1))) {
                  showPart = true;
                  break;
            }
      }

      msPart->setShow(showPart);

      addError(checkAtEndElement(_e, "part"));
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

/**
 In Score \a score find the measure starting at \a tick.
 */

static Measure* findMeasure(Score* score, const Fraction& tick)
      {
      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            if (m->tick() == tick)
                  return m;
            }
      return 0;
      }

//---------------------------------------------------------
//   removeBeam
//---------------------------------------------------------

/**
 Set beam mode for all elements and remove the beam
 */

static void removeBeam(Beam*& beam)
      {
      for (int i = 0; i < beam->elements().size(); ++i)
            beam->elements().at(i)->setBeamMode(Beam::Mode::NONE);
      delete beam;
      beam = nullptr;
      }

//---------------------------------------------------------
//   handleBeamAndStemDir
//---------------------------------------------------------

static void handleBeamAndStemDir(ChordRest* cr, const Beam::Mode bm, const Direction sd, Beam*& beam, bool hasBeamingInfo, QColor beamColor)
      {
      if (!cr) return;
      // create a new beam
      if (bm == Beam::Mode::BEGIN) {
            // if currently in a beam, delete it
            if (beam) {
                  qDebug("handleBeamAndStemDir() new beam, removing previous incomplete beam %p", beam);
                  removeBeam(beam);
                  }
            // create a new beam
            beam = new Beam(cr->score());
            beam->setTrack(cr->track());
            beam->setBeamDirection(sd);
            if (beamColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  beam->setColor(beamColor);
            }
      // add ChordRest to beam
      if (beam) {
            // verify still in the same track (if still in the same voice)
            // and in a beam ...
            // (note no check is done on correct order of beam begin/continue/end)
            // TODO: Some BEGINs are being skipped
            if (cr->track() != beam->track()) {
                  qDebug("handleBeamAndStemDir() from track %d to track %d -> abort beam",
                         beam->track(), cr->track());
                  // reset beam mode for all elements and remove the beam
                  removeBeam(beam);
                  }
            else if (bm == Beam::Mode::NONE) {
                  qDebug("handleBeamAndStemDir() in beam, bm Beam::Mode::NONE -> abort beam");
                  // reset beam mode for all elements and remove the beam
                  removeBeam(beam);
                  }
            else if (!(bm == Beam::Mode::BEGIN || bm == Beam::Mode::MID || bm == Beam::Mode::END || bm == Beam::Mode::BEGIN32 || bm == Beam::Mode::BEGIN64)) {
                  qDebug("handleBeamAndStemDir() in beam, bm %d -> abort beam", static_cast<int>(bm));
                  // reset beam mode for all elements and remove the beam
                  removeBeam(beam);
                  }
            else {
                  // actually add cr to the beam
                  beam->add(cr);
                  }
            }
      // if no beam, set stem direction on chord itself
      if (!beam) {
            static_cast<Chord*>(cr)->setStemDirection(sd);
            // set beam to none if score has beaming information and note can get beam, otherwise
            // set to auto
            bool canGetBeam = (cr->durationType().type() >= TDuration::DurationType::V_EIGHTH &&
                               cr->durationType().type() <= TDuration::DurationType::V_1024TH);
            if (hasBeamingInfo && canGetBeam)
                  cr->setBeamMode(Beam::Mode::NONE);
            else
                  cr->setBeamMode(Beam::Mode::AUTO);
            }
      // terminate the current beam and add to the score
      if (beam && bm == Beam::Mode::END)
            beam = nullptr;
      }


//---------------------------------------------------------
//   markUserAccidentals
//---------------------------------------------------------

/**
 Check for "superfluous" accidentals to mark them as USER accidentals.
 The candidate map alterMap is ordered on note address. Check it here segment after segment.
 */

static void markUserAccidentals(const int firstStaff,
                                const int staves,
                                const Key key,
                                const Measure* measure,
                                const QHash<Note*, int>& alterMap
                                )
      {
      QMap<int, bool> accTmp;

      AccidentalState currAcc;
      currAcc.init(key);
      SegmentType st = SegmentType::ChordRest;
      for (Ms::Segment* segment = measure->first(st); segment; segment = segment->next(st)) {
            for (int track = 0; track < staves * VOICES; ++track) {
                  Element* e = segment->element(firstStaff * VOICES + track);
                  if (!e || e->type() != Ms::ElementType::CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  for (Note* nt : chord->notes()) {
                        if (alterMap.contains(nt)) {
                              int alter = alterMap.value(nt);
                              int ln  = absStep(nt->tpc(), nt->pitch());
                              bool error = false;
                              AccidentalVal currAccVal = currAcc.accidentalVal(ln, error);
                              if (error)
                                    continue;
                              if ((alter == -1
                                   && currAccVal == AccidentalVal::FLAT
                                   && nt->accidental()->accidentalType() == AccidentalType::FLAT
                                   && !accTmp.value(ln, false))
                                  || (alter ==  0
                                      && currAccVal == AccidentalVal::NATURAL
                                      && nt->accidental()->accidentalType() == AccidentalType::NATURAL
                                      && !accTmp.value(ln, false))
                                  || (alter ==  1
                                      && currAccVal == AccidentalVal::SHARP
                                      && nt->accidental()->accidentalType() == AccidentalType::SHARP
                                      && !accTmp.value(ln, false))) {
                                    nt->accidental()->setRole(AccidentalRole::USER);
                                    }
                              else if (Accidental::isMicrotonal(nt->accidental()->accidentalType())
                                       && nt->accidental()->accidentalType() < AccidentalType::END) {
                                    // microtonal accidental
                                    nt->accidental()->setRole(AccidentalRole::USER);
                                    accTmp.insert(ln, false);
                                    }
                              else {
                                    accTmp.insert(ln, true);
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   coerceGraceCue
//---------------------------------------------------------

/**
 If the mainChord is small and/or silent, the grace note should likely
 match this. Exporters tend to incorrectly omit <cue> or <type size="cue">
 from grace notes.
 */

static void coerceGraceCue(Chord* mainChord, Chord* graceChord)
      {
      if (mainChord->isSmall())
            graceChord->setSmall(true);
      bool anyPlays = false;
      for (const Note* n : mainChord->notes())
            anyPlays |= n->play();
      if (!anyPlays)
            for (Note* gn : graceChord->notes())
                  gn->setPlay(false);
      }

//---------------------------------------------------------
//   addGraceChordsAfter
//---------------------------------------------------------

/**
 Move \a gac grace chords from grace chord list \a gcl
 to the chord \a c grace note after list
 */

static void addGraceChordsAfter(Chord* c, GraceChordList& gcl, int& gac)
      {
      if (!c)
            return;

      while (gac > 0) {
            if (gcl.size() > 0) {
                  Chord* graceChord = gcl.first();
                  gcl.removeFirst();
                  graceChord->toGraceAfter();
                  c->add(graceChord);        // TODO check if same voice ?
                  coerceGraceCue(c, graceChord);
                  qDebug("addGraceChordsAfter chord %p grace after chord %p", c, graceChord);
                  }
            gac--;
            }
      }

//---------------------------------------------------------
//   addGraceChordsBefore
//---------------------------------------------------------

/**
 Move grace chords from grace chord list \a gcl
 to the chord \a c grace note before list
 */

static void addGraceChordsBefore(Chord* c, GraceChordList& gcl)
      {
      for (int i = gcl.size() - 1; i >= 0; i--) {
            Chord* gc = gcl.at(i);
            for (Element* e : gc->el()) {
                  if (e->isFermata()) {
                        c->segment()->add(e);
                        gc->el().remove(e);
                        break;                  // out of the door, line on the left, one cross each
                        }
                  }
            c->add(gc);        // TODO check if same voice ?
            coerceGraceCue(c, gc);
            }
      gcl.clear();
      }

//---------------------------------------------------------
//   hasTempoTextAtTick
//---------------------------------------------------------

static bool hasTempoTextAtTick(const TempoMap* const tempoMap, const int tick)
      {
      return tempoMap->count(tick) > 0;
      }

static bool canAddTempoText(const TempoMap* const tempoMap, const int tick)
{
    if (tempoMap->count(tick) > 0) {
        return true;
    }

    return tempoMap->tempo(tick) == Score::defaultTempo();
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure node.
 */

void MusicXMLParserPass2::measure(const QString& partId,
                                  const Fraction time)
      {
      // "measure numbers" don't have to be actual numbers in MusicXML
      bool isNumericMeasureNumber = false;
      int parsedMeasureNumber = 0;
      QStringRef numberA = _e.attributes().value("number");
      if (!numberA.isEmpty() && numberA.at(0).toLatin1() != 'X')
            parsedMeasureNumber = numberA.toInt(&isNumericMeasureNumber);

      //qDebug("measure %d start", parsedMeasureNumber);

      Measure* measure = findMeasure(_score, time);
      if (!measure) {
            _logger->logError(QString("measure at tick %1 not found!").arg(time.ticks()), &_e);
            skipLogCurrElem();
            return;
            }

      // handle implicit measure
      if (_e.attributes().value("implicit") == "yes") {
            // Implicit measure: expect measure number to be unchanged.
            measure->setIrregular(true);
            }
      else {
            // Normal measure: expect number to have increased by one.
            ++_measureNumber;
            }

      if (isNumericMeasureNumber) {
            // Actual measure number may differ from expected value.
            measure->setNoOffset(parsedMeasureNumber - _measureNumber);
            _measureNumber = parsedMeasureNumber;
            }

      // set measure's RepeatFlag to none because musicXML is allowing single measure repeat and no ordering in repeat start and end barlines
      measure->setRepeatStart(false);
      measure->setRepeatEnd(false);

      /* TODO: for cutaway measures, i believe we can expect the staff to continue to be cutaway until another
       * print-object="yes" attribute is found. Here is the code that does that, though I don't want to actually commit this until
       * we have the exporter dealing with this sort of stuff as well.
       *
       * When print-object="yes" is encountered, the measure will explicitly be set to visible (see MusicXMLParserPass2::staffDetails)
      MeasureBase* prevBase = measure->prev();
      if (prevBase) {
            Part* part = _pass1.getPart(partId);
            staff_idx_t staffIdx = _score->staffIdx(part);
            if (!toMeasure(prevBase)->visible(staffIdx))
                 measure->setStaffVisible(staffIdx, false);
            } */

      Fraction mTime; // current time stamp within measure
      Fraction prevTime; // time stamp within measure previous chord
      Chord* prevChord = 0;       // previous chord
      Fraction mDura; // current total measure duration
      GraceChordList gcl; // grace chords collected sofar
      int gac = 0;       // grace after count in the grace chord list
      Beams beams;       // Current beam for each voice in the current part
      QString cv = "1";       // current voice for chords, default is 1
      FiguredBassList fbl;               // List of figured bass elements under a single note
      MxmlTupletStates tupletStates;       // Tuplet state for each voice in the current part
      Tuplets tuplets;       // Current tuplet for each voice in the current part

      // collect candidates for courtesy accidentals to work out at measure end
      QHash<Note*, int> alterMap;
      DelayedDirectionsList delayedDirections; // Directions to be added to score *after* collecting all and sorting
      InferredFingeringsList inferredFingerings; // Directions to be reinterpreted as Fingerings
      HarmonyMap delayedHarmony;

      while (_e.readNextStartElement()) {
            if (_e.name() == "attributes")
                  attributes(partId, measure, time + mTime);
            else if (_e.name() == "direction") {
                  MusicXMLParserDirection dir(_e, _score, _pass1, *this, _logger);
                  dir.direction(partId, measure, time + mTime, _spanners, delayedDirections, inferredFingerings, delayedHarmony);
                  }
            else if (_e.name() == "figured-bass") {
                  FiguredBass* fb = figuredBass();
                  if (fb)
                        fbl.append(fb);
                  }
            else if (_e.name() == "harmony")
                  harmony(partId, measure, time + mTime, delayedHarmony);
            else if (_e.name() == "note") {
                  // Correct delayed ottava tick
                  if (_delayedOttava && _delayedOttava->tick2() < time + mTime) {
                        handleSpannerStop(_delayedOttava, _delayedOttava->track2(), time + mTime, _spanners);
                        _delayedOttava = nullptr;
                        }
                  Fraction missingPrev;
                  Fraction dura;
                  Fraction missingCurr;
                  int alt = -10;                    // any number outside range of xml-tag "alter"
                  // note: chord and grace note handling done in note()
                  // dura > 0 iff valid rest or first note of chord found
                  Note* n = note(partId, measure, time + mTime, time + prevTime, missingPrev, dura, missingCurr, cv, gcl, gac, beams, fbl, alt,
                                 tupletStates, tuplets);
                  if (n && !n->chord()->isGrace())
                        prevChord = n->chord();  // remember last non-grace chord
                  if (n && n->accidental() && n->accidental()->accidentalType() != AccidentalType::NONE)
                        alterMap.insert(n, alt);
                  if (missingPrev.isValid()) {
                        mTime += missingPrev;
                        }
                  if (dura.isValid() && dura > Fraction(0, 1)) {
                        prevTime = mTime; // save time stamp last chord created
                        mTime += dura;
                        if (mTime > mDura)
                              mDura = mTime;
                        }
                  if (missingCurr.isValid()) {
                        mTime += missingCurr;
                        }
                  //qDebug("added note %p chord %p gac %d", n, n ? n->chord() : 0, gac);
                  }
            else if (_e.name() == "forward") {
                  Fraction dura;
                  forward(dura);
                  if (dura.isValid()) {
                        mTime += dura;
                        if (mTime > mDura)
                              mDura = mTime;
                        }
                  }
            else if (_e.name() == "backup") {
                  Fraction dura;
                  backup(dura);
                  if (dura.isValid()) {
                        if (dura <= mTime)
                              mTime -= dura;
                        else {
                              _logger->logError("backup beyond measure start", &_e);
                              mTime.set(0, 1);
                              }
                        // check if the tick position is smaller than the minimum division resolution
                        // (possibly caused by rounding errors) and in that case set position to 0
                        if (mTime.isNotZero() && (_divs > 0) && (mTime < Fraction(1, 4*_divs))) {
                              _logger->logError("backup to a fractional tick smaller than the minimum division", &_e);
                              mTime.set(0, 1);
                              }
                        }
                  }
            else if (_e.name() == "sound") {
                  QString tempo = _e.attributes().value("tempo").toString();

                  if (!tempo.isEmpty()) {
                        // sound tempo="..."
                        // create an invisible default TempoText
                        // to prevent duplicates, only if none is present yet
                        Fraction tick = time + mTime;
                        if (hasTempoTextAtTick(_score->tempomap(), tick.ticks())) {
                              _logger->logError(QString("duplicate tempo at tick %1").arg(tick.ticks()), &_e);
                              }
                        else {
                              double tpo = tempo.toDouble() / 60;
                              TempoText* t = new TempoText(_score);
                              t->setXmlText(QString("%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(TDuration::DurationType::V_QUARTER)), tempo));
                              t->setVisible(false);
                              t->setTempo(tpo);
                              t->setFollowText(true);

                              _score->setTempo(tick, tpo);

                              addElemOffset(t, _pass1.trackForPart(partId), "above", measure, tick);
                              }
                        }
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "barline")
                  barline(partId, measure, time + mTime);
            else if (_e.name() == "print")
                  _e.skipCurrentElement();
            else
                  skipLogCurrElem();

            /*
             qDebug("mTime %s (%s) mDura %s (%s)",
             qPrintable(mTime.print()),
             qPrintable(mTime.reduced().print()),
             qPrintable(mDura.print()),
             qPrintable(mDura.reduced().print()));
             */
            mDura.reduce();
            mTime.reduce();
            }

      // convert remaining grace chords to grace after
      gac = gcl.size();
      addGraceChordsAfter(prevChord, gcl, gac);

      // prevent tuplets from crossing measure boundaries
      resetTuplets(tuplets);

      // fill possible gaps in voice 1
      Part* part = _pass1.getPart(partId); // should not fail, we only get here if the part exists
      fillGapsInFirstVoices(measure, part);

      // Prevent any beams from extending into the next measure
      for (Beam*& beam : beams)
            if (beam)
                  removeBeam(beam);

      // Sort and add inferred fingerings
      std::sort(inferredFingerings.begin(), inferredFingerings.end(),
            // Lambda: sort by absolute value of totalY
            [](const MusicXMLInferredFingering* a, const MusicXMLInferredFingering* b) -> bool {
                  return std::abs(a->totalY()) < std::abs(b->totalY());
                  }
            );
      for (MusicXMLInferredFingering*& inferredFingering : inferredFingerings) {
            if (!inferredFingering->findAndAddToNotes(measure))
                  // Could not find notes to add to; print as direction
                  delayedDirections.push_back(inferredFingering->toDelayedDirection());
            delete inferredFingering;
            }

      for (auto& harmony : delayedHarmony) {
            HarmonyDesc harmonyDesc = harmony.second;
            Fraction tick = Fraction::fromTicks(harmony.first);
            if (harmonyDesc._fretDiagram) {
                  harmonyDesc._fretDiagram->setTrack(harmonyDesc._track);
                  Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
                  harmonyDesc._harmony->setProperty(Pid::ALIGN, int(Align::HCENTER | Align::TOP));
                  s->add(harmonyDesc._fretDiagram);
                  }

            if (harmonyDesc._harmony) {
                  harmonyDesc._harmony->setTrack(harmonyDesc._track);
                  Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
                  s->add(harmonyDesc._harmony);
                  }
            }

      // Sort and add delayed directions
      delayedDirections.combineTempoText();
      std::sort(delayedDirections.begin(), delayedDirections.end(),
                // Lambda: sort by absolute value of totalY
                [](const MusicXMLDelayedDirectionElement* a, const MusicXMLDelayedDirectionElement* b) -> bool {
                      return std::abs(a->totalY()) < std::abs(b->totalY());
                      }
                );
      for (MusicXMLDelayedDirectionElement* direction : qAsConst(delayedDirections)) {
            direction->addElem();
            delete direction;
            }

      // TODO:
      // - how to handle _timeSigDura.isZero (shouldn't happen ?)
      // - how to handle unmetered music
      if (_timeSigDura.isValid() && !_timeSigDura.isZero())
            measure->setTimesig(_timeSigDura);

      // mark superfluous accidentals as user accidentals
      const int scoreRelStaff = _score->staffIdx(part);
      const Key key = _score->staff(scoreRelStaff)->keySigEvent(time).key();
      markUserAccidentals(scoreRelStaff, part->nstaves(), key, measure, alterMap);

      // multi-measure rest handling
      if (getAndDecMultiMeasureRestCount() == 0) {
            // measure is first measure after a multi-measure rest
            measure->setBreakMultiMeasureRest(true);
            }

      addError(checkAtEndElement(_e, "measure"));
      }

//---------------------------------------------------------
//   attributes
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes node.
 */

/* Notes:
 * Number of staves has already been set in pass 1
 * MusicXML order is key, time, clef
 * -> check if it is necessary to insert them in order
 */

void MusicXMLParserPass2::attributes(const QString& partId, Measure* measure, const Fraction& tick)
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "clef")
                  clef(partId, measure, tick);
            else if (_e.name() == "divisions")
                  divisions();
            else if (_e.name() == "key")
                  key(partId, measure, tick);
            else if (_e.name() == "measure-style")
                  measureStyle(measure);
            else if (_e.name() == "staff-details")
                  staffDetails(partId, measure);
            else if (_e.name() == "time")
                  time(partId, measure, tick);
            else if (_e.name() == "transpose")
                  _e.skipCurrentElement();  // skip but don't log
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   setStaffLines
//---------------------------------------------------------

/**
 Set stafflines and barline span for a single staff
 */

static void setStaffLines(Score* score, int staffIdx, int stafflines)
      {
      score->staff(staffIdx)->setLines(Fraction(0,1), stafflines);
      score->staff(staffIdx)->setBarLineTo(0);        // default
      }

//---------------------------------------------------------
//   staffDetails
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details node.
 */

void MusicXMLParserPass2::staffDetails(const QString& partId, Measure* measure)
      {
      //logDebugTrace("MusicXMLParserPass2::staffDetails");

      Part* part = _pass1.getPart(partId);
      if (!part)
            return;
      int staves = part->nstaves();

      QString strNumber = _e.attributes().value("number").toString();
      int n = 0;  // default
      if (!strNumber.isEmpty()) {
            n = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strNumber.toInt());
            if (n < 0 || n >= staves) {
                  _logger->logError(QString("invalid staff-details number %1 (may be hidden)").arg(strNumber), &_e);
                  n = 0;
                  }
            }

      int staffIdx = _score->staffIdx(part) + n;

      StringData stringData;
      QString visible = _e.attributes().value("print-object").toString();
      QString spacing = _e.attributes().value("print-spacing").toString();
      if (visible == "no" ) {
            // EITHER:
            //  1) this indicates an empty staff that is hidden
            //  2) this indicates a cutaway measure. if it is a cutaway measure then print-spacing will be yes
            if (spacing == "yes") {
                  measure->setStaffVisible(staffIdx, false);
                  }
            else if (measure && !measure->hasVoices(staffIdx) && measure->isOnlyRests(staffIdx * VOICES)) {
                  // measures with print-object="no" are generally exported by exporters such as dolet when empty staves are hidden.
                  // for this reason, if we see print-object="no" (and no print-spacing), we can assume that this indicates we should set
                  // the hide empty staves style.
                  _score->style().set(Sid::hideEmptyStaves, true);
                  _score->style().set(Sid::dontHideStavesInFirstSystem, false);
                  }
            else {
                  // this doesn't apply to a measure, so we'll assume the entire staff has to be hidden.
                  _score->staff(staffIdx)->setInvisible(Fraction(0,1), true);
                  }
            }
      else if (visible == "yes" || visible.isEmpty()) {
            if (measure) {
                  _score->staff(staffIdx)->setInvisible(Fraction(0,1), false);
                  measure->setStaffVisible(staffIdx, true);
                  }
            }
      else
            _logger->logError(QString("print-object should be \"yes\" or \"no\""));

      int staffLines = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "staff-lines") {
                  // save staff lines for later
                  staffLines = _e.readElementText().toInt();
                  // for a TAB staff also resize the string table and init with zeroes
                  if (0 < staffLines)
                        stringData.stringList() = QVector<instrString>(staffLines).toList();
                  else
                        _logger->logError(QString("illegal staff-lines %1").arg(staffLines), &_e);
                  }
            else if (_e.name() == "staff-tuning")
                  staffTuning(&stringData);
            else if (_e.name() == "staff-size") {
                  const double val = _e.readElementText().toDouble() / 100;
                  _score->staff(staffIdx)->setProperty(Pid::MAG, val);
                  }
            else
                  skipLogCurrElem();
            }

      if (staffLines > 0) {
            setStaffLines(_score, staffIdx, staffLines);
            }

      Instrument* i = part->instrument();
      if (_score->staff(staffIdx)->isTabStaff(Fraction(0, 1))) {
            if (i->stringData()->frets() == 0)
                  stringData.setFrets(25);
            else
                  stringData.setFrets(i->stringData()->frets());
            }
      if (stringData.strings() > 0)
            i->setStringData(stringData);
      else if (stringData.strings() > 0)
            _logger->logError("trying to change string data for non-TAB staff (not supported)", &_e);
      }
//---------------------------------------------------------
//   staffTuning
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details/staff-tuning node.
 */

void MusicXMLParserPass2::staffTuning(StringData* t)
      {
      //logDebugTrace("MusicXMLParserPass2::staffTuning");

      // ignore <staff-tuning> if not a TAB staff
      if (!t) {
            _logger->logError("<staff-tuning> on non-TAB staff", &_e);
            skipLogCurrElem();
            return;
            }

      int line   = _e.attributes().value("line").toInt();
      int step   = 0;
      int alter  = 0;
      int octave = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "tuning-alter")
                  alter = _e.readElementText().trimmed().toInt();
            else if (_e.name() == "tuning-octave")
                  octave = _e.readElementText().toInt();
            else if (_e.name() == "tuning-step") {
                  QString strStep = _e.readElementText();
                  int pos = QString("CDEFGAB").indexOf(strStep);
                  if (strStep.size() == 1 && pos >=0 && pos < 7)
                        step = pos;
                  else
                        _logger->logError(QString("invalid step '%1'").arg(strStep), &_e);
                  }
            else
                  skipLogCurrElem();
            }

      if (0 < line && line <= t->stringList().size()) {
            int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
            if (pitch >= 0)
                  t->stringList()[line - 1].pitch = pitch;
            else
                  _logger->logError(QString("invalid string %1 tuning step/alter/oct %2/%3/%4")
                                    .arg(line).arg(step).arg(alter).arg(octave),
                                    &_e);
            }
      }

//---------------------------------------------------------
//   measureStyle
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/measure-style node.
 - Initializes the "in multi-measure rest" state
 - Set/reset the "rhythmic/slash notation" state
 */

void MusicXMLParserPass2::measureStyle(Measure* measure)
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "multiple-rest") {
                  int multipleRest = _e.readElementText().toInt();
                  if (multipleRest > 1) {
                        _multiMeasureRestCount = multipleRest;
                        _score->style().set(Sid::createMultiMeasureRests, true);
                        measure->setBreakMultiMeasureRest(true);
                        }
                  else
                        _logger->logError(QString("multiple-rest %1 not supported").arg(multipleRest), &_e);
                  }
            else if (_e.name() == "slash") {
                  QString type = _e.attributes().value("type").toString();
                  QString stems = _e.attributes().value("use-stems").toString();
                  _measureStyleSlash = type == "start" ? (stems == "yes" ? MusicXmlSlash::RHYTHM : MusicXmlSlash::SLASH) : MusicXmlSlash::NONE;
                  _e.skipCurrentElement();
                  }
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   preventNegativeTick
//---------------------------------------------------------
/**
  Prevent an offset that would result in a negative tick (set offset to -tick instead, resulting in a tick of 0)
 */

static void preventNegativeTick(const Fraction& tick, Fraction& offset, MxmlLogger* logger)
      {
      if (tick + offset < Fraction(0, 1)) {
            logger->logError(QString("illegal offset %1 at tick %2").arg(offset.ticks()).arg(tick.ticks()));
            offset = -tick;
            }
      }

//---------------------------------------------------------
//   isTempoOrphanCandidate
//---------------------------------------------------------

bool MusicXMLDelayedDirectionElement::isTempoOrphanCandidate() const
      {
      return _element->isStaffText()
            && _placement == "above"
            && isBold();
      }

//---------------------------------------------------------
//   addElem
//---------------------------------------------------------

void MusicXMLDelayedDirectionElement::addElem()
      {
      addElemOffset(_element, _track, _placement, _measure, _tick);
      }

QString MusicXMLParserDirection::placement() const
      {
      if (_placement.isEmpty() && hasTotalY())
            return totalY() < 0 ? "above" : "below";
      else return _placement;
      }

//---------------------------------------------------------
//   combineTempoText
//---------------------------------------------------------
/**
 Combine potentially separated tempo text.
 */

void DelayedDirectionsList::combineTempoText()
      {
      // Iterate through candidates
      for (auto ddi1 = rbegin(), ddi1Next = ddi1; ddi1 != rend(); ddi1 = ddi1Next) {
            ddi1Next = std::next(ddi1);
            if ((*ddi1)->isTempoOrphanCandidate()) {
                  for (auto ddi2 = rbegin(), ddi2Next = ddi2; ddi2 != rend(); ddi2 = ddi2Next) {
                        ddi2Next = std::next(ddi2);
                        // Combine with tempo text if present
                        if (ddi1 != ddi2
                            && (*ddi2)->tick() == (*ddi1)->tick()
                            && (*ddi2)->element()->isTempoText()) {
                              TempoText* tt = toTempoText((*ddi2)->element());
                              StaffText* st = toStaffText((*ddi1)->element());
                              QString sep = tt->plainText().endsWith(' ') || st->plainText().startsWith(' ') ? "" : " ";
                              tt->setXmlText(tt->xmlText() + sep + st->xmlText());
                              delete st;
                              ddi1Next = decltype(ddi1){ erase(std::next(ddi1).base()) };
                              break;
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction node.
 */

void MusicXMLParserDirection::direction(const QString& partId,
                                        Measure* measure,
                                        const Fraction& tick,
                                        MusicXmlSpannerMap& spanners,
                                        DelayedDirectionsList& delayedDirections,
                                        InferredFingeringsList& inferredFingerings,
                                        HarmonyMap& harmonyMap)
      {
      //qDebug("direction tick %s", qPrintable(tick.print()));

      _placement = _e.attributes().value("placement").toString();
      int track = _pass1.trackForPart(partId);
      bool isVocalStaff = _pass1.isVocalStaff(partId);
      bool isExpressionText = false;
      bool delayOttava = _pass1.exporterSoftware() == MusicXMLExporterSoftware::SIBELIUS;
      _systemDirection = _e.attributes().value("system").toString() == "only-top";
      //qDebug("direction track %d", track);
      QList<MusicXmlSpannerDesc> starts;
      QList<MusicXmlSpannerDesc> stops;

      // note: file order is direction-type first, then staff
      // this means staff is still unknown when direction-type is handled
      // easiest solution is to put spanners on a stop and start list
      // and handle these after the while loop

      // note that placement is a <direction> attribute (which is currently supported by MS)
      // but the <direction-type> children also have formatting attributes
      // (currently NOT supported by MS, at least not for spanners when <direction> children)

      while (_e.readNextStartElement()) {
            if (_e.name() == "direction-type")
                  directionType(starts, stops);
            else if (_e.name() == "offset") {
                  _offset = _pass1.calcTicks(_e.readElementText().toInt(), _pass2.divs(), &_e);
                  preventNegativeTick(tick, _offset, _logger);
                  }
            else if (_e.name() == "sound")
                  sound();
            else if (_e.name() == "staff") {
                  QString strStaff = _e.readElementText();
                  int staff = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt());
                  if (staff >= 0)
                        track += staff * VOICES;
                  else
                        _logger->logError(QString("invalid staff %1").arg(strStaff), &_e);
                  }
            else
                  skipLogCurrElem();
            }

      handleRepeats(measure, track, tick + _offset);
      handleNmiCmi(measure, track, tick + _offset, delayedDirections);
      handleChordSym(track, tick + _offset, harmonyMap);
      handleFraction();

      // fix for Sibelius 7.1.3 (direct export) which creates metronomes without <sound tempo="..."/>:
      // if necessary, use the value calculated by metronome()
      // note: no floating point comparisons with 0 ...
      if (_tpoSound < 0.1 && _tpoMetro > 0.1)
            _tpoSound = _tpoMetro;

      //qDebug("words '%s' rehearsal '%s' metro '%s' tpo %g",
      //       qPrintable(_wordsText), qPrintable(_rehearsalText), qPrintable(_metroText), _tpoSound);

      // create text if any text was found
      if (isLyricBracket())
            return;
      else if (isLikelyCredit(tick)) {
            Text* inferredText = addTextToHeader(Tid::COMPOSER);
            if (inferredText) {
                  _pass1.setHasInferredHeaderText(true);
                  hideRedundantHeaderText(inferredText, {"lyricist", "composer", "poet"});
                  }
            }
      else if (isLikelySubtitle(tick)) {
            Text* inferredText = addTextToHeader(Tid::SUBTITLE);
            if (inferredText) {
                  _pass1.setHasInferredHeaderText(true);
                  if (_score->metaTag("source").isEmpty())
                        _score->setMetaTag("source", inferredText->plainText());
                  hideRedundantHeaderText(inferredText, {"source"});
                  }
            }
      else if (isLikelyLegallyDownloaded(tick)) {
            // Ignore (TBD: print to footer?)
            return;
            }
      else if (isLikelyTempoText(track)) {
            TempoText* tt = new TempoText(_score);
            tt->setXmlText(_wordsText + _metroText);
            if (_tpoSound > 0 && canAddTempoText(_score->tempomap(), tick.ticks())) {
                  double tpo = _tpoSound / 60;
                  tt->setTempo(tpo);
                  if (tt->plainText().contains('='))
                        tt->setFollowText(true);
                  }
            tt->setVisible(_visible);

            addElemOffset(tt, track, placement(), measure, tick + _offset);
            }
      else if (!_wordsText.isEmpty() || !_rehearsalText.isEmpty() || !_metroText.isEmpty()) {
            TextBase* t = 0;
            if (_tpoSound > 0.1 || attemptTempoTextCoercion(tick)) {
                  // to prevent duplicates, only create a TempoText if none is present yet
                  if (hasTempoTextAtTick(_score->tempomap(), tick.ticks())) {
                        _logger->logError(QString("duplicate tempo at tick %1").arg(tick.ticks()), &_e);
                        }
                  else {
                        t = new TempoText(_score);
                        QString rawWordsText = _wordsText;
                        static const QRegularExpression re("(<.*?>)");
                        rawWordsText.remove(re);
                        QString sep = !_metroText.isEmpty() && !rawWordsText.isEmpty()
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                                    && rawWordsText.back()
#else
                                    && rawWordsText.at(rawWordsText.size() - 1)
#endif
                                    != ' ' ? " " : QString();
                        t->setXmlText(_wordsText + sep + _metroText);
                        if (_tpoSound > 0.1) {
                              _tpoSound /= 60;
                              ((TempoText*) t)->setTempo(_tpoSound);
                              _score->setTempo(tick, _tpoSound);
                              }
                        else {
                              ((TempoText*) t)->setTempo(_score->tempo(tick)); // Maintain tempo (somewhat hacky)
                              }
                        if (t->plainText().contains('='))
                              ((TempoText*)t)->setFollowText(true);
                        }
                  }
            else if (!_wordsText.isEmpty() || !_metroText.isEmpty()) {
                  isExpressionText = _wordsText.contains("<i>") && _metroText.isEmpty();
                  if (isExpressionText)
                        t = new StaffText(_score, Tid::EXPRESSION);
                  else if (_systemDirection)
                        t = new SystemText(_score);
                  else
                        t = new StaffText(_score);
                  t->setXmlText(_wordsText + _metroText);
                  }
            else {
                  t = new RehearsalMark(_score);
                  if (!_rehearsalText.contains("<b>"))
                        _rehearsalText = "<b></b>" + _rehearsalText;  // explicitly turn bold off
                  t->setXmlText(_rehearsalText);
                  if (!_hasDefaultY) {
                        t->setPlacement(Placement::ABOVE);  // crude way to force placement TODO improve ?
                        t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                        t->resetProperty(Pid::OFFSET);
                        }
                  }

            if (t) {
                  if (_enclosure == "circle") {
                        t->setFrameType(FrameType::CIRCLE);
                        }
                  else if (_enclosure == "none") {
                        t->setFrameType(FrameType::NO_FRAME);
                        }
                  else if (_enclosure == "rectangle") {
                        t->setFrameType(FrameType::SQUARE);
                        t->setFrameRound(0);
                        }

                  if (_color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/) {
                        t->setColor(_color);
                        t->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
                        }

                  t->setVisible(_visible);

                  QString wordsPlacement = placement();
                  // Case-based defaults
                  if (wordsPlacement.isEmpty()) {
                        if (isVocalStaff)
                              wordsPlacement = "above";
                        else if (isExpressionText)
                              wordsPlacement = "below";
                        }

                  QString fingeringStr = _wordsText;
                  static const QRegularExpression xmlFormating("(<.*?>)");
                  fingeringStr.remove(xmlFormating).remove(u'\u00A0');
                  if (isLikelyFingering(fingeringStr)) {
                        _logger->logDebugInfo(QString("Inferring fingering: %1").arg(fingeringStr));
                        t->setXmlText(fingeringStr);
                        MusicXMLInferredFingering* inferredFingering = new MusicXMLInferredFingering(totalY(), t, fingeringStr, track,
                                                                                                     wordsPlacement, measure, tick + _offset);
                        inferredFingerings.push_back(inferredFingering);
                        }
                  else if (directionToDynamic()) {
                        delete t;
                        }
                  else {
                        // Add element to score later, after collecting all the others and sorting by default-y
                        // This allows default-y to be at least respected by the order of elements
                        MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(hasTotalY() ? totalY() : 100, t, track, wordsPlacement, measure, tick + _offset, _isBold);
                        delayedDirections.push_back(delayedDirection);
                        }
                  }
            }
      else if (_tpoSound > 0) {
            // direction without text but with sound tempo="..."
            // create an invisible default TempoText
            if (hasTempoTextAtTick(_score->tempomap(), tick.ticks())) {
                  _logger->logError(QString("duplicate tempo at tick %1").arg(tick.ticks()), &_e);
                  }
            else {
                  double tpo = _tpoSound / 60;
                  TempoText* t = new TempoText(_score);
                  t->setXmlText(QString("%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(TDuration::DurationType::V_QUARTER))).arg(_tpoSound));
                  t->setVisible(false);
                  t->setTempo(tpo);
                  t->setFollowText(true);

                  // TBD may want to use tick + _offset if sound is affected
                  _score->setTempo(tick, tpo);

                  addElemOffset(t, track, placement(), measure, tick + _offset);
                  }
            }

      // do dynamics
      // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
      for (QStringList::Iterator it = _dynamicsList.begin(); it != _dynamicsList.end(); ++it ) {
            Dynamic* dyn = new Dynamic(_score);
            dyn->setDynamicType(*it);
            if (!_dynaVelocity.isEmpty()) {
                  int dynaValue = round(_dynaVelocity.toDouble() * 0.9);
                  if (dynaValue > 127)
                        dynaValue = 127;
                  else if (dynaValue < 0)
                        dynaValue = 0;
                  dyn->setVelocity(dynaValue);
                  }

            dyn->setVisible(_visible);

            QString dynamicsPlacement = placement();
            // Case-based defaults
            if (dynamicsPlacement.isEmpty())
                  dynamicsPlacement = isVocalStaff ? "above" : "below";

            // Check staff and end any cresc lines which are waiting
            if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE)) {
                  // To avoid extending lines which aren't intended to be terminated by dynamics,
                  // only extend lines to dynamics within 24 quarter notes
                  static const Fraction MAX_INFERRED_LINE_LEN = Fraction(24, 4);
                  InferredHairpinsStack hairpins = _pass2.getInferredHairpins();
                  for (Hairpin* h : hairpins) {
                        Fraction diff = tick + _offset - h->tick();
                        if (h && h->staffIdx() == track2staff(track) && h->ticks() == Fraction(0, 1) && diff <= MAX_INFERRED_LINE_LEN) {
                              h->setTrack2(track);
                              h->setTick2(tick + _offset);
                              }
                        }
                  }

            // Add element to score later, after collecting all the others and sorting by default-y
            // This allows default-y to be at least respected by the order of elements
            MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(hasTotalY() ? totalY() : 100, dyn, track, dynamicsPlacement, measure, tick + _offset, _isBold);
            delayedDirections.push_back(delayedDirection);
            }

      addInferredCrescLine(track, tick, isVocalStaff);

      // handle the elems
      for (Element* elem : qAsConst(_elems)) {
            // Add element to score later, after collecting all the others and sorting by default-y
            // This allows default-y to be at least respected by the order of elements
            if (hasTotalY()) {
                  MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(
                                    totalY(), elem, track, placement(), measure, tick + _offset, _isBold);
                  delayedDirections.push_back(delayedDirection);
                  }
            else
                  addElemOffset(elem, track, placement(), measure, tick + _offset);
            }

      // handle the spanner stops first
      for (MusicXmlSpannerDesc& desc : stops) {
            MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ desc._tp, desc._nr });
            if (spdesc._isStopped) {
                  _logger->logError("spanner already stopped", &_e);
                  delete desc._sp;
                  }
            else {
                  if (spdesc._isStarted) {
                        // Adjustments to ottavas by the offset value are unwanted
                        const Fraction spTick = spdesc._sp && spdesc._sp->isOttava() ? tick : tick + _offset;
                        if (spdesc._sp && spdesc._sp->isOttava() && delayOttava) {
                              // Sibelius writes ottava ends 1 note too early
                              _pass2.setDelayedOttava(spdesc._sp);
                              _pass2.delayedOttava()->setTrack2(track);
                              _pass2.delayedOttava()->setTick2(spTick);
                              // need to set tick again later
                              _pass2.clearSpanner(desc);
                              }
                        else {
                              handleSpannerStop(spdesc._sp, track, spTick, spanners);
                              _pass2.clearSpanner(desc);
                              }
                        }
                  else {
                        spdesc._sp = desc._sp;
                        const Fraction spTick = spdesc._sp && spdesc._sp->isOttava() ? tick : tick + _offset;
                        spdesc._tick2 = spTick;
                        spdesc._track2 = track;
                        spdesc._isStopped = true;
                        }
                  }
            }

      // then handle the spanner starts
      // TBD handle offset ?
      for (MusicXmlSpannerDesc& desc : starts) {
            MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ desc._tp, desc._nr });
            if (spdesc._isStarted) {
                  _logger->logError("spanner already started", &_e);
                  delete desc._sp;
                  }
            else {
                  QString spannerPlacement = placement();
                  // Case-based defaults
                  if (spannerPlacement.isEmpty()) {
                        if (desc._sp->isHairpin())
                              spannerPlacement = isVocalStaff ? "above" : "below";
                        else
                              spannerPlacement = totalY() < 0 ? "above" : "below";
                        }
                  desc._sp->setVisible(_visible);
                  if (spdesc._isStopped) {
                        _pass2.addSpanner(desc);
                        // handleSpannerStart and handleSpannerStop must be called in order
                        // due to allocation of elements in the map
                        handleSpannerStart(desc._sp, track, spannerPlacement, tick + _offset, spanners);
                        handleSpannerStop(spdesc._sp, spdesc._track2, spdesc._tick2, spanners);
                        _pass2.clearSpanner(desc);
                        }
                  else {
                        _pass2.addSpanner(desc);
                        handleSpannerStart(desc._sp, track, spannerPlacement, tick + _offset, spanners);
                        spdesc._isStarted = true;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   directionType
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type node.
 */

void MusicXMLParserDirection::directionType(QList<MusicXmlSpannerDesc>& starts,
                                            QList<MusicXmlSpannerDesc>& stops)
      {
      while (_e.readNextStartElement()) {
            // Prevent multi-word directions from overwriting y-values.
            bool hasDefaultYCandidate = false;
            bool hasRelativeYCandidate = false;
            qreal defaultYCandidate = _e.attributes().value("default-y").toDouble(&hasDefaultYCandidate) * -0.1;
            qreal relativeYCandidate =_e.attributes().value("relative-y").toDouble(&hasRelativeYCandidate) * -0.1;
            if (hasDefaultYCandidate && !_hasDefaultY)
                  _defaultY = defaultYCandidate;
            if (hasRelativeYCandidate && !_hasRelativeY)
                  _relativeY = relativeYCandidate;
            _hasDefaultY |= hasDefaultYCandidate;
            _hasRelativeY |= hasRelativeYCandidate;
            _isBold &= _e.attributes().value("font-weight").toString() == "bold";
            _visible = _e.attributes().value("print-object").toString() != "no";
            QString number = _e.attributes().value("number").toString();
            int n = 0;
            if (!number.isEmpty()) {
                  n = number.toInt();
                  if (n <= 0)
                        _logger->logError(QString("invalid number %1").arg(number), &_e);
                  else
                        n--;  // make zero-based
                  }
            QString type = _e.attributes().value("type").toString();
            _color       = _e.attributes().value("color").toString();

            if  (_e.name() == "metronome")
                  _metroText = metronome(_tpoMetro);
            else if (_e.name() == "words") {
                  _enclosure      = _e.attributes().value("enclosure").toString();
                  QString nextPart = nextPartOfFormattedString(_e);
                  textToDynamic(nextPart);
                  textToCrescLine(nextPart);
                  _wordsText += nextPart;
                  }
            else if (_e.name() == "rehearsal") {
                  _enclosure      = _e.attributes().value("enclosure").toString();
                  if (_enclosure.isEmpty())
                        _enclosure = "square";  // note different default
                  _rehearsalText += nextPartOfFormattedString(_e);
                  }
            else if (_e.name() == "pedal")
                  pedal(type, n, starts, stops);
            else if (_e.name() == "octave-shift")
                  octaveShift(type, n, starts, stops);
            else if (_e.name() == "dynamics")
                  dynamics();
            else if (_e.name() == "bracket")
                  bracket(type, n, starts, stops);
            else if (_e.name() == "dashes")
                  dashes(type, n, starts, stops);
            else if (_e.name() == "wedge")
                  wedge(type, n, starts, stops);
            else if (_e.name() == "coda") {
                  _wordsText += "<sym>coda</sym>";
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "segno") {
                  _wordsText += "<sym>segno</sym>";
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "symbol") {
                  const QString smufl = _e.readElementText();
                  if (!smufl.isEmpty())
                        _wordsText += "<sym>" + smufl + "</sym>";
                  }
            else if (_e.name() == "other-direction")
                  otherDirection();
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   sound
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/sound node.
 */

void MusicXMLParserDirection::sound()
      {
      _sndCoda = _e.attributes().value("coda").toString();
      _sndDacapo = _e.attributes().value("dacapo").toString();
      _sndDalsegno = _e.attributes().value("dalsegno").toString();
      _sndFine = _e.attributes().value("fine").toString();
      _sndSegno = _e.attributes().value("segno").toString();
      _sndToCoda = _e.attributes().value("tocoda").toString();
      _tpoSound = _e.attributes().value("tempo").toDouble();
      _dynaVelocity = _e.attributes().value("dynamics").toString();

      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   dynamics
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/dynamics node.
 */

void MusicXMLParserDirection::dynamics()
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "other-dynamics")
                  _dynamicsList.push_back(_e.readElementText());
            else {
                  _dynamicsList.push_back(_e.name().toString());
                  _e.skipCurrentElement();
                  }
            }
      }

void MusicXMLParserDirection::otherDirection()
      {
      // <other-direction> element is used to define any <direction> symbols not yet in the MusicXML format
      const QString smufl = _e.attributes().value("smufl").toString();
      const QColor color = _e.attributes().value("color").toString();

      // Read smufl symbol
      if (!smufl.isEmpty()) {
            SymId id { SymId::noSym };
            if (convertArticulationToSymId(_e.name().toString(), id) && id != SymId::noSym) {
                  Symbol* smuflSym = new Symbol(_score);
                  smuflSym->setSym(id);
                  if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        smuflSym->setColor(color);
                  _elems.push_back(smuflSym);
                  }
            _e.skipCurrentElement();
            }
      else {
            // TODO: Multiple sets of maps for exporters other than Dolet 6/Sibelius
            // TODO: Add more symbols from Sibelius
            QMap<QString, QString> otherDirectionStrings;
            if (_pass1.dolet()) {
                  otherDirectionStrings = {
                        { QString("To Coda"), QString("To Coda") },
                        { QString("Segno"), QString("<sym>segno</sym>") },
                        { QString("CODA"), QString("<sym>coda</sym>") },
                  };
                  }
            static const QMap<QString, SymId> otherDirectionSyms = {
                  { QString("Rhythm dot"), SymId::augmentationDot },
                  { QString("Whole rest"), SymId::restWhole },
                  { QString("l.v. down"), SymId::articLaissezVibrerBelow },
                  { QString("8vb"), SymId::ottavaBassaVb },
                  { QString("Treble clef"), SymId::gClef },
                  { QString("Bass clef"), SymId::fClef },
                  { QString("Caesura"), SymId::caesura },
                  { QString("Thick caesura"), SymId::caesuraThick }
                  };
            QString t = _e.readElementText();
            QString val = otherDirectionStrings.value(t);
            if (!val.isEmpty())
                  _wordsText += val;
            else {
                  SymId sym = otherDirectionSyms.value(t);
                  Symbol* smuflSym = new Symbol(_score);
                  smuflSym->setSym(sym);
                  if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        smuflSym->setColor(color);
                  _elems.push_back(smuflSym);
                  }
            }
      }

//---------------------------------------------------------
//   matchRepeat
//---------------------------------------------------------

/**
 Do a wild-card match with known repeat texts.
 */

QString MusicXMLParserDirection::matchRepeat() const
      {
      QString plainWords = MScoreTextToMXML::toPlainText(_wordsText.toLower().simplified());
      static QRegularExpression daCapo("^(d\\.? ?|da )(c\\.?|capo)$");
      static QRegularExpression daCapoAlFine("^(d\\.? ?|da )(c\\.? ?|capo )al fine$");
      static QRegularExpression daCapoAlCoda("^(d\\.? ?|da )(c\\.? ?|capo )al coda$");
      static QRegularExpression dalSegno("^(d\\.? ?|d[ae]l )(s\\.?|segno)$");
      static QRegularExpression dalSegnoAlFine("^(d\\.? ?|d[ae]l )(s\\.?|segno\\.?) al fine$");
      static QRegularExpression dalSegnoAlCoda("^(d\\.? ?|d[ae]l )(s\\.?|segno\\.?) al coda$");
      static QRegularExpression fine("^fine$");
      static QRegularExpression segno("^segno( segno)?$");
      static QRegularExpression toCoda("^to coda( coda)?$");
      static QRegularExpression coda("^coda( coda)?$");
      if (plainWords.contains(daCapo))          return "daCapo";
      if (plainWords.contains(daCapoAlFine))    return "daCapoAlFine";
      if (plainWords.contains(daCapoAlCoda))    return "daCapoAlCoda";
      if (plainWords.contains(dalSegno))        return "dalSegno";
      if (plainWords.contains(dalSegnoAlFine))  return "dalSegnoAlFine";
      if (plainWords.contains(dalSegnoAlCoda))  return "dalSegnoAlCoda";
      if (plainWords.contains(segno))           return "segno";
      if (plainWords.contains(fine))            return "fine";
      if (plainWords.contains(toCoda))          return "toCoda";
      if (plainWords.contains(coda))            return "coda";
      return QString();
      }

//---------------------------------------------------------
//   findJump
//---------------------------------------------------------

/**
 Try to find a Jump in \a repeat.
 */

static Jump* findJump(const QString& repeat, Score* score)
      {
      Jump* jp = 0;
      if (repeat == "daCapo") {
            jp = new Jump(score);
            jp->setJumpType(Jump::Type::DC);
            }
      else if (repeat == "daCapoAlCoda") {
            jp = new Jump(score);
            jp->setJumpType(Jump::Type::DC_AL_CODA);
            }
      else if (repeat == "daCapoAlFine") {
            jp = new Jump(score);
            jp->setJumpType(Jump::Type::DC_AL_FINE);
            }
      else if (repeat == "dalSegno") {
            jp = new Jump(score);
            jp->setJumpType(Jump::Type::DS);
            }
      else if (repeat == "dalSegnoAlCoda") {
            jp = new Jump(score);
            jp->setJumpType(Jump::Type::DS_AL_CODA);
            }
      else if (repeat == "dalSegnoAlFine") {
            jp = new Jump(score);
            jp->setJumpType(Jump::Type::DS_AL_FINE);
            }
      return jp;
      }

//---------------------------------------------------------
//   findMarker
//---------------------------------------------------------

/**
 Try to find a Marker in \a repeat.
 */

static Marker* findMarker(const QString& repeat, Score* score)
      {
      Marker* m = 0;
      if (repeat == "segno") {
            m = new Marker(score);
            // note: Marker::read() also contains code to set text style based on type
            // avoid duplicated code
            // apparently this MUST be after setTextStyle
            m->setMarkerType(Marker::Type::SEGNO);
            }
      else if (repeat == "coda") {
            m = new Marker(score);
            m->setMarkerType(Marker::Type::CODA);
            }
      else if (repeat == "fine") {
            m = new Marker(score, Tid::REPEAT_RIGHT);
            m->setMarkerType(Marker::Type::FINE);
            }
      else if (repeat == "toCoda") {
            m = new Marker(score, Tid::REPEAT_RIGHT);
            m->setMarkerType(Marker::Type::TOCODA);
            }
      return m;
      }

//---------------------------------------------------------
//   textToDynamic
//---------------------------------------------------------
/**
 Attempts to convert text to dynamic text. No-op if unable.
 */
void MusicXMLParserDirection::textToDynamic(QString& text) const
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;
      QString simplifiedText = text.simplified();
      for (auto dyn : dynList) {
            if (dyn.tag == simplifiedText) {
                  text = text.replace(simplifiedText, dyn.text);
                  }
            }
      }

//---------------------------------------------------------
//   directionToDynamic
//---------------------------------------------------------
/**
 Attempts to convert direction to dynamic. True if successful,
 else false.
 */
bool MusicXMLParserDirection::directionToDynamic()
      {
      if (!_metroText.isEmpty() || !_rehearsalText.isEmpty() || !_enclosure.isEmpty())
            return false;
      QString simplifiedText = _wordsText.simplified();
      for (auto dyn : dynList) {
            if (dyn.tag == simplifiedText || dyn.text == simplifiedText) {
                  _dynaVelocity = QString::number(round(dyn.velocity / 0.9));
                  _dynamicsList.push_back(simplifiedText);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   isLyricBracket
//    Dolet exports lyric brackets as staff text,
//    which we ought not render.
//---------------------------------------------------------

bool MusicXMLParserDirection::isLyricBracket() const
      {
      if (_wordsText.isEmpty())
            return false;

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
      if (!(_wordsText.front() == '}' || _wordsText.back() == '{'))
#else
      if (!(_wordsText.at(0) == '}' || _wordsText.at(_wordsText.size() - 1) == '{'))
#endif
            return false;

      return _rehearsalText.isEmpty()
            && _metroText.isEmpty()
            && _dynamicsList.isEmpty()
            && _tpoSound < 0.1;
      }

//---------------------------------------------------------
//   isLikelyFingering
//---------------------------------------------------------

bool MusicXMLParserDirection::isLikelyFingering(const QString& fingeringStr) const
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return false;
      // One or more newline-separated digits (or changes), possibly lead or trailed by whitespace
      static const QRegularExpression re(
          "^(?:\\s|\u00A0)*[-–]?(?:\\s|\u00A0)?[0-5pimac](?:[-–][0-5pimac])?(?:\n[-–]?(?:\\s|\u00A0)?[0-5pimac](?:[-–][0-5pimac])?)*(?:\\s|\u00A0)*$");
      return fingeringStr.contains(re)
            && _rehearsalText.isEmpty()
            && _metroText.isEmpty()
            && _tpoSound < 0.1;
      }

//---------------------------------------------------------
//   isLikelySubtitle
//---------------------------------------------------------

bool MusicXMLParserDirection::isLikelySubtitle(const Fraction& tick) const
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return false;
      return (tick + _offset < Fraction(5, 1)) // Only early in the piece
            && _rehearsalText.isEmpty()
            && _metroText.isEmpty()
            && _tpoSound < 0.1
            && isLikelySubtitleText(_wordsText, false);
      }

//---------------------------------------------------------
//   isLikelyCredit
//---------------------------------------------------------

bool MusicXMLParserDirection::isLikelyCredit(const Fraction& tick) const
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return false;
      return (tick + _offset < Fraction(5, 1)) // Only early in the piece
            && _rehearsalText.isEmpty()
            && _metroText.isEmpty()
            && _tpoSound < 0.1
            && isLikelyCreditText(_wordsText, false);
      }

//---------------------------------------------------------
//   isLikelyLegallyDownloaded
//---------------------------------------------------------

bool MusicXMLParserDirection::isLikelyLegallyDownloaded(const Fraction& tick) const
      {
      static const QRegularExpression legallyDownloaded("This music has been legally downloaded\\.(?:\\s|\u00A0)Do not photocopy\\.");
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return false;
      return (tick + _offset < Fraction(5, 1)) // Only early in the piece
            && _rehearsalText.isEmpty()
            && _metroText.isEmpty()
            && _tpoSound < 0.1
            && _wordsText.contains(legallyDownloaded);
      }

bool MusicXMLParserDirection::isLikelyTempoText(const int track) const
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE)
          || _wordsText.contains("<i>")
          || _wordsText.contains("“")
          || _wordsText.contains("”")
          || placement() == "below"
          || track2staff(track) != 0) {
            return false;
            }

      const QString plainText = MScoreTextToMXML::toPlainText(_wordsText.simplified());
      static const std::array<QString,
                  31> tempoStrs =
                  { "accel", "adag", "alleg", "andant", "a tempo", "ballad", "brisk", "determination", "dolce", "expressive",
                    "fast", "free", "gently", "grave", "larg", "lento", "stesso tempo", "lively", "maestoso", "moderat", "mosso",
                    "prest", "rit", "rubato", "slow", "straight", "tango", "tempo i", "tenderly", "triumphant", "vivace" };
      for (const QString& str : tempoStrs) {
            if (plainText.contains(str, Qt::CaseInsensitive))
                  return true;
            }
      return false;
      }

void MusicXMLParserDirection::handleFraction()
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;

      QString rawWordsText = _wordsText;
      static const QRegularExpression re("(<.*?>)");
      rawWordsText.remove(re);
      rawWordsText = rawWordsText.simplified();

      //                         UTF-16 encoding: 0x00BC, 0x00BD, 0x00BE, 0x2150, 0x2151, 0x2152,  etc...
      static const std::array<QString, 18> fracs { "1/4", "1/2", "3/4", "1/7", "1/9", "1/10", "1/3", "2/3", "1/5", "2/5", "3/5",
                                                   "4/5", "1/6", "5/6", "1/8", "3/8", "5/8", "7/8" };

      for (size_t n = 0; n < fracs.size(); n++) {
            if (rawWordsText.contains(fracs.at(n))) {
                  size_t p = n <= 2 ? 0x00BC + n : 0x2150 + n - 3;
                  rawWordsText.replace(fracs.at(n), QString(char16_t(p)));
                  _wordsText = rawWordsText;
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   addTextToHeader
//---------------------------------------------------------

Text* MusicXMLParserDirection::addTextToHeader(const Tid tid)
      {
      Text* t = new Text(_score, tid);
      t->setXmlText(_wordsText.trimmed());
      MeasureBase* const firstMeasure = _score->measures()->first();
      VBox* vbox = firstMeasure->isVBox() ? toVBox(firstMeasure) : MusicXMLParserPass1::createAndAddVBoxForCreditWords(_score);
      t->layout();
      vbox->layout();
      vbox->setBoxHeight(vbox->boxHeight() + Spatium(t->height()/_score->spatium())); // add the height of the text
      vbox->add(t);
      return t;
      }

//---------------------------------------------------------
//   hideRedundantHeaderText
//    After inferring header text, hide redundant text.
//    Redundant text is detected by checking all the
//    Text elements of the inferred text's VBox against
//    the contents of the eligible metaTags
//---------------------------------------------------------

void MusicXMLParserDirection::hideRedundantHeaderText(const Text* inferredText, const std::vector<QString> metaTags)
      {
      if (!inferredText->parent()->isVBox())
            return;

      for (Element* e : toVBox(inferredText->parent())->el()) {
            if (e == inferredText || !e->isText())
                  continue;

            Text* t = toText(e);
            for (const QString& metaTag : metaTags) {
                  if (t->plainText() == _score->metaTag(metaTag)) {
                        t->setVisible(false);
                        continue;
                        }
                  }
            }
      }
//---------------------------------------------------------
//   MusicXMLInferredFingering
//---------------------------------------------------------

MusicXMLInferredFingering::MusicXMLInferredFingering(qreal totalY,
                                                     Element* element,
                                                     QString text,
                                                     int track,
                                                     QString placement,
                                                     Measure* measure,
                                                     Fraction tick)
      : _totalY(totalY), _element(element), _text(text), _track(track), _placement(placement), _measure(measure), _tick(tick)
      {
      _fingerings = _text.simplified().split(" ");
      }

//---------------------------------------------------------
//   roundTick
//---------------------------------------------------------
/**
 Round tick to multiple of gcd of measure
 */

void MusicXMLInferredFingering::roundTick(Measure* measure)
      {
      measure->computeTicks();
      long gcdTicks = Fraction(1, 1).ticks();
      for (auto s = measure->segments().begin(); s != measure->segments().end(); ++s) {
            if ((*s).isChordRestType())
                  gcdTicks = gcd(gcdTicks, (*s).ticks().ticks());
            }
      if (!gcdTicks || gcdTicks == Fraction(1, 1).ticks() || !(_tick.ticks() % gcdTicks)) return;
      int roundedTick = std::round(static_cast<double>(_tick.ticks())/static_cast<double>(gcdTicks)) * (gcdTicks);
      _tick = Fraction::fromTicks(roundedTick);
      }


//---------------------------------------------------------
//   findAndAddToNotes
//---------------------------------------------------------
/**
 Attempts to find an eligible collection of notes to add inferred
 fingerings to. Adds notes and returns true if successful, else no-op
 and returns false.
 */
bool MusicXMLInferredFingering::findAndAddToNotes(Measure* measure)
      {
      roundTick(measure);
      std::vector<Note*> collectedNotes;
      for (int track = _track; track < _track + 4; ++track) {
            Chord* candidateChord = measure->findChord(tick(), track);
            if (candidateChord) {
                  if (static_cast<int>(candidateChord->notes().size()) >= fingerings().size()) {
                        addToNotes(candidateChord->notes());
                        return true;
                        }
                  else {
                        collectedNotes.insert(collectedNotes.begin(),
                                              candidateChord->notes().begin(),
                                              candidateChord->notes().end());
                        if (static_cast<int>(collectedNotes.size()) >= fingerings().size()) {
                              addToNotes(collectedNotes);
                              return true;
                              }
                        }
                  }
            }
      // No suitable notes found
      return false;
      }

//---------------------------------------------------------
//   addToNotes
//---------------------------------------------------------
/**
 Add the n fingerings to the first n collected notes
 */
void MusicXMLInferredFingering::addToNotes(std::vector<Note*>& notes) const
      {
      Q_ASSERT(static_cast<int>(notes.size()) >= _fingerings.size());
      for (int i = 0; i < _fingerings.size(); ++i) {
            // Fingerings in reverse order
            addTextToNote(-1, -1, _fingerings[_fingerings.size() - 1 - i], _placement, "", -1, "", "",
                        QColor(), Tid::FINGERING, notes[i]->score(), notes[i]);
            }
      }

//---------------------------------------------------------
//   toDelayedDirection
//---------------------------------------------------------

MusicXMLDelayedDirectionElement* MusicXMLInferredFingering::toDelayedDirection()
      {
      MusicXMLDelayedDirectionElement* dd = new MusicXMLDelayedDirectionElement(_totalY, _element, _track, _placement, _measure, _tick, false);
      return dd;
      }

//---------------------------------------------------------
//   convertTextToNotes
//---------------------------------------------------------
/**
 Converts note characters in _wordsText to proper symbols and
 returns the tempo value of the resulting notes
 */

double MusicXMLParserDirection::convertTextToNotes()
      {
      static const QRegularExpression notesRegex("(?<note>[yxeqhwW]\\.{0,2})((?:\\s|\u00A0)*=)");
      QString notesSubstring = notesRegex.match(_wordsText).captured("note");

      QList<QPair<QString, QString>> noteSyms{{"q", QString("<sym>metNoteQuarterUp</sym>")},   // note4_Sym
                                              {"e", QString("<sym>metNote8thUp</sym>")},       // note8_Sym
                                              {"h", QString("<sym>metNoteHalfUp</sym>")},      // note2_Sym
                                              {"y", QString("<sym>metNote32ndUp</sym>")},      // note32_Sym
                                              {"x", QString("<sym>metNote16thUp</sym>")},      // note16_Sym
                                              {"w", QString("<sym>metNoteWhole</sym>")},
                                              {"W", QString("<sym>metNoteDoubleWhole</sym>")}};
      for (const auto& noteSym : noteSyms) {
            if (notesSubstring.contains(noteSym.first)) {
                  notesSubstring.replace(noteSym.first, noteSym.second);
                  break;
                  }
            }
      notesSubstring.replace(".", QString("<sym>metAugmentationDot</sym>")); // dot
      _wordsText.replace(notesRegex, notesSubstring + "\\2");

      double tempoValue = TempoText::findTempoValue(_wordsText);
      if (!tempoValue) tempoValue = 1.0 / 60.0; // default to quarter note
      return tempoValue;
      }

//---------------------------------------------------------
//   attemptTempoTextCoercion
//---------------------------------------------------------
/**
 Infers if a direction is likely tempo text, possibly changing
 the _wordsText to the appropriate note symbol and inferring the _tpoSound.
 */

bool MusicXMLParserDirection::attemptTempoTextCoercion(const Fraction& tick)
      {
      QList<QString> tempoWords{"rit", "rall", "accel", "tempo", "allegr", "poco", "molto", "più", "meno", "mosso", "rubato"};
      static const QRegularExpression re("[yxeqhwW.]+(?:\\s|\u00A0)*=(?:\\s|\u00A0)*\\d+");
      if (_wordsText.contains(re)) {
            static const QRegularExpression tempoValRegex("=(?:\\s|\u00A0)*(?<tempo>\\d+)");
            double tempoVal = tempoValRegex.match(_wordsText).captured("tempo").toDouble();
            double noteVal = convertTextToNotes() * 60.0;
            _tpoSound = tempoVal / noteVal;
            return true;
            }
      else if (placement() == "above" && _isBold) {
            if (tick == Fraction(0, 1)) return true;
            for (const auto& tempoWord : tempoWords)
                  if (_wordsText.contains(tempoWord, Qt::CaseInsensitive))
                        return true;
            }
      return false;
      }

void MusicXMLParserDirection::textToCrescLine(QString& text)
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;
      QString simplifiedText = MScoreTextToMXML::toPlainText(text).simplified();
      bool cresc = simplifiedText.contains("cresc");
      bool dim = simplifiedText.contains("dim");
      if (!cresc && !dim)
            return;

      // Create line
      text.clear();
      Hairpin* line = new Hairpin(_score);

      line->setHairpinType(cresc ? HairpinType::CRESC_LINE : HairpinType::DECRESC_LINE);
      line->setBeginText(simplifiedText);
      line->setContinueText("");
      line->setProperty(Pid::LINE_VISIBLE, false);
      _inferredHairpinStart = line;
      }

void MusicXMLParserDirection::addInferredCrescLine(const int track, const Fraction& tick, const bool isVocalStaff)
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;
      if (!_inferredHairpinStart)
            return;

      _inferredHairpinStart->setTrack(track);
      _inferredHairpinStart->setTick(tick + _offset);

      QString spannerPlacement = _placement;
      if (_placement.isEmpty())
            spannerPlacement = isVocalStaff ? "above" : "below";
      setSLinePlacement(_inferredHairpinStart, _placement);

      _pass2.addInferredHairpin(_inferredHairpinStart);
      }

//---------------------------------------------------------
//   handleRepeats
//---------------------------------------------------------

void MusicXMLParserDirection::handleRepeats(Measure* measure, const int track, const Fraction tick)
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;
      // Try to recognize the various repeats
      QString repeat;
      if (!_sndCoda.isEmpty())
            repeat = "coda";
      else if (!_sndDacapo.isEmpty())
            repeat = "daCapo";
      else if (!_sndDalsegno.isEmpty())
            repeat = "dalSegno";
      else if (!_sndFine.isEmpty())
            repeat = "fine";
      else if (!_sndSegno.isEmpty())
            repeat = "segno";
      else if (!_sndToCoda.isEmpty())
            repeat = "toCoda";
      // As sound may be missing, next do a wild-card match with known repeat texts
      else
            repeat = matchRepeat();

      // Create Jump or Marker and assign it _wordsText (invisible if no _wordsText)
      if (!repeat.isEmpty()) {
            TextBase* tb = nullptr;
            if ((tb = findJump(repeat, _score)) || (tb = findMarker(repeat, _score))) {
                  tb->setTrack(track);
                  if (!_wordsText.isEmpty()) {
                        tb->setXmlText(_wordsText);
                        _wordsText.clear();

                        // Temporary fix for symbol sizing issues
                        qreal symSize = _score->style().value(Sid::repeatLeftFontSize).toReal();
                        qreal textSize = _score->style().value(Sid::repeatRightFontSize).toReal();
                        if (tb->xmlText() != "<sym>coda</sym>" && tb->xmlText() != "<sym>segno</sym>")
                              tb->setSize(textSize);
                        else
                              tb->setSize(symSize);
                        tb->setPropertyFlags(Pid::FONT_SIZE, PropertyFlags::UNSTYLED);
                        }
                  else
                        tb->setVisible(false);

                  // Sometimes Jumps and Markers are exported on the incorrect side
                  // of the barline (i.e. end of mm. 29 vs. beginning of mm. 30).
                  // This fixes that.
                  bool closerToLeft = tick - measure->tick() < measure->endTick() - tick;
                  if (tb->tid() == Tid::REPEAT_RIGHT && closerToLeft && measure->prevMeasure())
                        measure = measure->prevMeasure();
                  else if (tb->tid() == Tid::REPEAT_LEFT && !closerToLeft && measure->nextMeasure())
                        measure = measure->nextMeasure();
                  tb->setVisible(_visible);
                  measure->add(tb);
                  }
            }
      }

//---------------------------------------------------------
//   handleNmiCmi
//    Dolet strangely exports N.C. chord symbols as the
//    text direction "NmiCmi".
//---------------------------------------------------------

void MusicXMLParserDirection::handleNmiCmi(Measure* measure, const int track, const Fraction tick, DelayedDirectionsList& delayedDirections)
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;
      if (!_wordsText.contains("NmiCmi"))
            return;
      Harmony* ha = new Harmony(_score);
      ha->setRootTpc(Tpc::TPC_INVALID);
      ha->setId(-1);
      ha->setTextName("N.C.");
      ha->setTrack(track);
      MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(totalY(), ha, track, "above", measure, tick, false);
      delayedDirections.push_back(delayedDirection);
      _wordsText.replace("NmiCmi", "");
      }

void MusicXMLParserDirection::handleChordSym(const int track, const Fraction tick, HarmonyMap& harmonyMap)
      {
      if (!preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE))
            return;

      static const QRegularExpression re("^([abcdefg])(([#b♯♭])\3?)?(maj|min|m)?[769]?((add[#b♯♭]?(9|11|))|(sus[24]?))?(\\(.*\\))?$");
      QString plainWords = _wordsText.simplified().toLower();
      if (!plainWords.contains(re))
            return;

      Harmony* ha = new Harmony(_score);
      ha->setHarmony(_wordsText);
      ha->setTrack(track);
      ha->setPlacement(placement() == "above" ? Placement::ABOVE : Placement::BELOW);
      ha->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      ha->resetProperty(Pid::OFFSET);
      ha->setVisible(_visible);
      HarmonyDesc newHarmonyDesc(track, ha, nullptr);

      const int ticks = tick.ticks();
      bool insert = true;
      for (auto itr = harmonyMap.begin(); itr != harmonyMap.end(); itr++) {
            if (itr->first != ticks)
                  continue;
            HarmonyDesc& foundHarmonyDesc = itr->second;

            // Don't insert if there is a matching chord symbol
            // This symbol doesn't have a fret diagram, so no need to check that here
            if (track2staff(foundHarmonyDesc._track) == track2staff(track) && foundHarmonyDesc._harmony->descr() == ha->descr())
                  insert = false;
            }

      if (insert)
            harmonyMap.insert(std::pair<int, HarmonyDesc>(ticks, newHarmonyDesc));
      _wordsText.clear();
      }

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/bracket node.
 This creates a TextLine for all line-types except "wavy", for which it creates a Trill
 */

void MusicXMLParserDirection::bracket(const QString& type, const int number,
                                      QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops)
      {
      QStringRef lineEnd = _e.attributes().value("line-end");
      QStringRef lineType = _e.attributes().value("line-type");
      const bool isWavy = lineType == "wavy";
      const ElementType elementType = isWavy ? ElementType::TRILL : ElementType::TEXTLINE;
      const MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ elementType, number });
      if (type == "start") {
            SLine* sline = spdesc._isStopped ? spdesc._sp : 0;
            if ((sline && sline->isTrill()) || (!sline && isWavy)) {
                  if (!sline)
                        sline = new Trill(_score);
                  Trill* trill = toTrill(sline);
                  trill->setTrillType(Trill::Type::PRALLPRALL_LINE);

                  if (!lineEnd.isEmpty() && lineEnd != "none")
                        _logger->logError(QString("line-end not supported for line-type \"wavy\""));
                  }
            else if ((sline && sline->isTextLine()) || (!sline && !isWavy)) {
                  if (!sline) {
                        sline = new TextLine(_score);
                        sline->setSystemFlag(_systemDirection);
                        }
                  TextLine* textLine = toTextLine(sline);
                  // if (placement.isEmpty()) placement = "above";  // TODO ? set default

                  textLine->setBeginHookType(lineEnd != "none" ? HookType::HOOK_90 : HookType::NONE);
                  if (lineEnd == "up")
                        textLine->setBeginHookHeight(-1 * textLine->beginHookHeight());

                  // hack: combine with a previous words element
                  if (!_wordsText.isEmpty()) {
                        // TextLine supports only limited formatting, remove all (compatible with 1.3)
                        textLine->setBeginText(MScoreTextToMXML::toPlainText(_wordsText));
                        _wordsText.clear();
                        }

                  if (lineType == "solid")
                        textLine->setLineStyle(Qt::SolidLine);
                  else if (lineType == "dashed")
                        textLine->setLineStyle(Qt::DashLine);
                  else if (lineType == "dotted")
                        textLine->setLineStyle(Qt::DotLine);
                  else if (lineType != "wavy")
                        _logger->logError(QString("unsupported line-type: %1").arg(lineType.toString()), &_e);

                  const QColor color = _e.attributes().value("color").toString();
                  if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        textLine->setLineColor(color);
                  }

            starts.append(MusicXmlSpannerDesc(sline, elementType, number));
            }
      else if (type == "stop") {
            SLine* sline = spdesc._isStarted ? spdesc._sp : 0;
            if ((sline && sline->isTrill()) || (!sline && isWavy)) {
                  if (!sline)
                        sline = new Trill(_score);
                  if (!lineEnd.isEmpty() && lineEnd != "none")
                        _logger->logError(QString("line-end not supported for line-type \"wavy\""));
                  }
            else if ((sline && sline->isTextLine()) || (!sline && !isWavy)) {
                  if (!sline)
                        sline = new TextLine(_score);
                  TextLine* textLine = toTextLine(sline);
                  textLine->setEndHookType(lineEnd != "none" ? HookType::HOOK_90 : HookType::NONE);
                  if (lineEnd == "up")
                        textLine->setEndHookHeight(-1 * textLine->endHookHeight());
                  }

            stops.append(MusicXmlSpannerDesc(sline, elementType, number));
            }
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   dashes
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/dashes node.
 */

void MusicXMLParserDirection::dashes(const QString& type, const int number,
                                     QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops)
      {
      const MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ ElementType::HAIRPIN, number });
      if (type == "start") {
            TextLine* b = spdesc._isStopped ? toTextLine(spdesc._sp) : new TextLine(_score);
            // if (placement.isEmpty()) placement = "above";  // TODO ? set default

            // hack: combine with a previous words element
            if (!_wordsText.isEmpty()) {
                  // TextLine supports only limited formatting, remove all (compatible with 1.3)
                  b->setBeginText(MScoreTextToMXML::toPlainText(_wordsText));
                  _wordsText.clear();
                  }

            b->setBeginHookType(HookType::NONE);
            b->setEndHookType(HookType::NONE);
            b->setLineStyle(Qt::DashLine);
            // TODO brackets and dashes now share the same storage
            // because they both use ElementType::TEXTLINE
            // use mxml specific type instead
            starts.append(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
            }
      else if (type == "stop") {
            TextLine* b = spdesc._isStarted ? toTextLine(spdesc._sp) : new TextLine(_score);
            stops.append(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
            }
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   octaveShift
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/octave-shift node.
 */

void MusicXMLParserDirection::octaveShift(const QString& type, const int number,
                                          QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops)
      {
      const MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ ElementType::OTTAVA, number });
      if (type == "up" || type == "down") {
            int ottavasize = _e.attributes().value("size").toInt();
            if (!(ottavasize == 8 || ottavasize == 15)) {
                  _logger->logError(QString("unknown octave-shift size %1").arg(ottavasize), &_e);
                  }
            else {
                  Ottava* o = spdesc._isStopped ? toOttava(spdesc._sp) : new Ottava(_score);

                  if (type == "down") {
                        _placement = _placement.isEmpty() ? "above" : _placement;
                        if (ottavasize ==  8)
                              o->setOttavaType(OttavaType::OTTAVA_8VA);
                        else if (ottavasize == 15)
                              o->setOttavaType(OttavaType::OTTAVA_15MA);
                        }
                  else /*if (type == "up")*/ {
                        _placement = _placement.isEmpty() ? "below" : _placement;
                        if (ottavasize ==  8)
                              o->setOttavaType(OttavaType::OTTAVA_8VB);
                        else if (ottavasize == 15)
                              o->setOttavaType(OttavaType::OTTAVA_15MB);
                        }

                  const QColor color = _e.attributes().value("color").toString();
                  if (color.isValid())
                        o->setLineColor(color);

                  starts.append(MusicXmlSpannerDesc(o, ElementType::OTTAVA, number));
                  }
            }
      else if (type == "stop") {
            Ottava* o = spdesc._isStarted ? toOttava(spdesc._sp) : new Ottava(_score);
            stops.append(MusicXmlSpannerDesc(o, ElementType::OTTAVA, number));
            }
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/pedal node.
 */

void MusicXMLParserDirection::pedal(const QString& type, const int /* number */,
                                    QList<MusicXmlSpannerDesc>& starts,
                                    QList<MusicXmlSpannerDesc>& stops)
      {
      const int number { 0 };
      QStringRef line = _e.attributes().value("line");
      QString sign = _e.attributes().value("sign").toString();
      const QColor color = _e.attributes().value("color").toString();

      if (_placement.isEmpty())
            _placement = "below";

      // We have found that many exporters omit "sign" even when one is originally present,
      // therefore we will default to "yes", even though this is technically against the spec.
      bool overrideDefaultSign = true; // TODO: set this flag based on the exporting software
      if (sign.isEmpty()) {
            if (line != "yes" || (overrideDefaultSign && type == "start"))
                  sign = "yes";                       // MusicXML 2.0 compatibility
            else if (line == "yes")
                  sign = "no";                        // MusicXML 2.0 compatibility
            }
      MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ ElementType::PEDAL, number });
      if (type == "start" || type == "resume" || type == "sostenuto") {
            if (spdesc._isStarted && !spdesc._isStopped) {
                  // Previous pedal unterminated
                  // if previous pedal was a change, create a new change instead of a new pedal start
                  if (toPedal(spdesc._sp)->beginHookType() == HookType::HOOK_45) {
                        Pedal* p = new Pedal(_score);
                        p->setBeginHookType(HookType::HOOK_45);
                        p->setEndHookType(HookType::HOOK_90);
                        if (line == "yes") {
                              p->setLineVisible(true);
                              } else {
                              p->setLineVisible(false);
                              }
                        if (sign == "no") {
                              p->setBeginText("");
                              p->setContinueText("");
                              p->setEndText("");
                              }
                        if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/) {
                              p->setColor(color);
                              }
                        starts.push_back(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
                        _e.skipCurrentElement();

                        return;
                        }
                  else {
                        // likely an unrecorded "discontinue", so delete the line.
                        _pass2.deleteHandledSpanner(spdesc._sp);
                        spdesc._isStarted = false;
                        }
                  }
            Pedal* p = spdesc._isStopped ? toPedal(spdesc._sp) : new Pedal(_score);
            if (line == "yes")
                  p->setLineVisible(true);
            else
                  p->setLineVisible(false);
            if (!p->lineVisible() || sign == "yes") {
                  p->setBeginText("<sym>keyboardPedalPed</sym>");
                  p->setContinueText("(<sym>keyboardPedalPed</sym>)");
                  if (type == "sostenuto") {
                        p->setBeginText("<sym>keyboardPedalSost</sym>");
                        p->setContinueText("(<sym>keyboardPedalSost</sym>)");
                        }
                  }
            else {
                  p->setBeginText("");
                  p->setContinueText("");
                  p->setBeginHookType(type == "resume" ? HookType::NONE : HookType::HOOK_90);
                  }
            p->setEndHookType(HookType::NONE);

            if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  p->setLineColor(color);
            starts.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
            }
      else if (type == "stop" || type == "discontinue") {
            Pedal* p = spdesc._isStarted ? toPedal(spdesc._sp) : new Pedal(_score);
            if (line == "yes")
                  p->setLineVisible(true);
            else if (line == "no")
                  p->setLineVisible(false);
            if ((!p->lineVisible() || sign == "yes") && p->endHookType() == HookType::NONE)
                  p->setEndText("<sym>keyboardPedalUp</sym>");
            else
                  p->setEndHookType(type == "discontinue" ? HookType::NONE : HookType::HOOK_90);
            stops.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
            }
      else if (type == "change") {
            // pedal change is implemented as two separate pedals
            // first stop the first one
            if (spdesc._isStarted && !spdesc._isStopped) {
                  Pedal* p = toPedal(spdesc._sp);
                  p->setEndHookType(HookType::HOOK_45);
                  if (line == "yes")
                        p->setLineVisible(true);
                  else if (line == "no")
                        p->setLineVisible(false);
                  stops.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
                  }
            else
                  _logger->logError(QString("\"change\" type pedal created without existing pedal"), &_e);
            // then start a new one
            Pedal* p = new Pedal(_score);
            p->setBeginHookType(HookType::HOOK_45);
            p->setEndHookType(HookType::HOOK_90);
            if (line == "yes")
                  p->setLineVisible(true);
            else
                  p->setLineVisible(false);

            if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  p->setLineColor(color);
            if (sign == "no") {
                  p->setBeginText("");
                  p->setContinueText("");
                  }
            starts.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
            }
      else if (type == "continue") {
            // ignore
            }
      else
            qDebug("unknown pedal type %s", qPrintable(type));

      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   wedge
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/wedge node.
 */

void MusicXMLParserDirection::wedge(const QString& type, const int number,
                                    QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops)
      {
      QStringRef niente = _e.attributes().value("niente");
      const MusicXmlExtendedSpannerDesc& spdesc = _pass2.getSpanner({ ElementType::HAIRPIN, number });
      if (type == "crescendo" || type == "diminuendo") {
            Hairpin* h = spdesc._isStopped ? toHairpin(spdesc._sp) : new Hairpin(_score);
            h->setHairpinType(type == "crescendo"
                              ? HairpinType::CRESC_HAIRPIN : HairpinType::DECRESC_HAIRPIN);
            if (niente == "yes")
                  h->setHairpinCircledTip(true);

            const QColor color = _e.attributes().value("color").toString();
            if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  h->setLineColor(color);

            starts.append(MusicXmlSpannerDesc(h, ElementType::HAIRPIN, number));
            }
      else if (type == "stop") {
            Hairpin* h = spdesc._isStarted ? toHairpin(spdesc._sp) : new Hairpin(_score);
            if (niente == "yes")
                  h->setHairpinCircledTip(true);
            stops.append(MusicXmlSpannerDesc(h, ElementType::HAIRPIN, number));
            }
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   toString
//---------------------------------------------------------

QString MusicXmlExtendedSpannerDesc::toString() const
      {
      QString string;
      QTextStream(&string) << _sp;
      return QString("sp %1 tp %2 tick2 %3 track2 %4 %5 %6")
             .arg(string, _tick2.print())
             .arg(_track2)
             .arg(_isStarted ? "started" : "", _isStopped ? "stopped" : "")
      ;
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void MusicXMLParserPass2::addSpanner(const MusicXmlSpannerDesc& d)
      {
      MusicXmlExtendedSpannerDesc& spdesc = getSpanner(d);
      spdesc._sp = d._sp;
      }

//---------------------------------------------------------
//   getSpanner
//---------------------------------------------------------

MusicXmlExtendedSpannerDesc& MusicXMLParserPass2::getSpanner(const MusicXmlSpannerDesc& d)
      {
      if (d._tp == ElementType::HAIRPIN && 0 <= d._nr && d._nr < MAX_NUMBER_LEVEL)
            return _hairpins[d._nr];
      else if (d._tp == ElementType::OTTAVA && 0 <= d._nr && d._nr < MAX_NUMBER_LEVEL)
            return _ottavas[d._nr];
      else if (d._tp == ElementType::PEDAL && 0 == d._nr)
            return _pedal;
      else if ((d._tp == ElementType::TEXTLINE || d._tp == ElementType::TRILL) && 0 <= d._nr && d._nr < MAX_NUMBER_LEVEL)
            return _brackets[d._nr];
      _logger->logError(QString("invalid number %1").arg(d._nr + 1), &_e);
      return _dummyNewMusicXmlSpannerDesc;
      }

//---------------------------------------------------------
//   clearSpanner
//---------------------------------------------------------

void MusicXMLParserPass2::clearSpanner(const MusicXmlSpannerDesc& d)
      {
      MusicXmlExtendedSpannerDesc& spdesc = getSpanner(d);
      spdesc = {};
      }

//---------------------------------------------------------
//   deleteHandledSpanner
//---------------------------------------------------------
/**
 Delete a spanner that's already been added to _spanners.
 This is used to remove pedal markings that are never stopped
 */

void MusicXMLParserPass2::deleteHandledSpanner(SLine* const& spanner)
      {
      _spanners.remove(spanner);
      delete spanner;
      }

void MusicXMLParserPass2::addInferredHairpin(Hairpin* hp)
      {
      _inferredHairpins.push_back(hp);
      }

InferredHairpinsStack MusicXMLParserPass2::getInferredHairpins()
      {
      return _inferredHairpins;
      }

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/metronome node.
 Convert to text and set r to calculated tempo.
 */

QString MusicXMLParserDirection::metronome(double& r)
      {
      r = 0;
      QString tempoText;
      QString perMinute;
      bool parenth = _e.attributes().value("parentheses") == "yes";

      if (parenth)
            tempoText += "(";

      TDuration dur1;
      TDuration dur2;

      while (_e.readNextStartElement()) {
            if (_e.name() == "metronome-note" || _e.name() == "metronome-relation") {
                  skipLogCurrElem();
                  continue;
                  }
            QString txt = _e.readElementText();
            if (_e.name() == "beat-unit") {
                  // set first dur that is still invalid
                  if (!dur1.isValid()) dur1.setType(txt);
                  else if (!dur2.isValid()) dur2.setType(txt);
                  }
            else if (_e.name() == "beat-unit-dot") {
                  if (dur2.isValid()) dur2.setDots(1);
                  else if (dur1.isValid()) dur1.setDots(1);
                  }
            else if (_e.name() == "per-minute")
                  perMinute = txt;
            else
                  skipLogCurrElem();
            }

      if (dur1.isValid())
            tempoText += TempoText::duration2tempoTextString(dur1);
      if (dur2.isValid()) {
            tempoText += " = ";
            tempoText += TempoText::duration2tempoTextString(dur2);
            }
      else if (!perMinute.isEmpty()) {
            tempoText += " = ";
            tempoText += perMinute;
            }
      if (dur1.isValid() && !dur2.isValid() && !perMinute.isEmpty()) {
            bool ok;
            double d = perMinute.toDouble(&ok);
            if (ok) {
                  // convert fraction to beats per minute
                  r = 4 * dur1.fraction().numerator() * d / dur1.fraction().denominator();
                  }
            }

      if (parenth)
            tempoText += ")";

      return tempoText;
      }

//---------------------------------------------------------
//   determineBarLineType
//---------------------------------------------------------

static bool determineBarLineType(const QString& barStyle, const QString& repeat,
                                 BarLineType& type, bool& visible)
      {
      // set defaults
      type = BarLineType::NORMAL;
      visible = true;

      if (barStyle == "light-heavy" && repeat == "backward")
            type = BarLineType::END_REPEAT;
      else if (barStyle == "heavy-light" && repeat == "forward")
            type = BarLineType::START_REPEAT;
      else if (barStyle == "light-heavy" && repeat.isEmpty())
            type = BarLineType::END;
      else if (barStyle == "heavy-light" && repeat.isEmpty())
                  type = BarLineType::REVERSE_END;
      else if (barStyle == "regular")
            type = BarLineType::NORMAL;
      else if (barStyle == "dashed")
            type = BarLineType::BROKEN;
      else if (barStyle == "dotted")
            type = BarLineType::DOTTED;
      else if (barStyle == "light-light")
            type = BarLineType::DOUBLE;
      /*
       else if (barStyle == "heavy-light")
       ;
      */
      else if (barStyle == "heavy-heavy")
            type = BarLineType::DOUBLE_HEAVY;
      else if (barStyle == "heavy")
            type = BarLineType::HEAVY;
      else if (barStyle == "none")
            visible = false;
      else if (barStyle.isEmpty()) {
            if (repeat == "backward")
                  type = BarLineType::END_REPEAT;
            else if (repeat == "forward")
                  type = BarLineType::START_REPEAT;
            else {
                  return false;
                  }
            }
      else if ((barStyle == "tick") || (barStyle == "short")) {
            // handled later (as normal barline with different parameters)
            }
      else {
            qDebug("unsupported bar type <%s>", qPrintable(barStyle));       // TODO
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   barline
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/barline node.
 */

/*
 Following barline types are automatically generated by MuseScore in an EndBarLine segment at the end of a measure:
 - normal (excluding tick and short)
 - start repeat
 - end-start repeat
 - end repeat
 - final (at the end of the score only)
 The other barline types can also be in an EndBarLine segment at the end of a measure, but are NOT generated
 Mid-measure barlines are in a BarLine segment and are NOT generated
 Following barline types can only be at the end of a measure:
 - start repeat
 - end-start repeat
 - end repeat
 - final
Regular barlines should not be added at the start or end of a measure, as that could lead to inconsistent behaviour.
 */

void MusicXMLParserPass2::barline(const QString& partId, Measure* measure, const Fraction& tick)
      {
      QString loc = _e.attributes().value("location").toString();
      if (loc.isEmpty())
            loc = "right";
      // Place barline in correct place
      Fraction locTick = tick;
      if (loc == "left")
          locTick = measure->tick();
      else if (loc == "right")
          locTick = measure->endTick();

      QString barStyle;
      QColor barlineColor;
      QString endingNumber;
      QString endingType;
      QColor endingColor;
      QString endingText;
      QString repeat;
      QString count;
      bool printEnding = true;

      while (_e.readNextStartElement()) {
            if (_e.name() == "bar-style") {
                  barlineColor = _e.attributes().value("color").toString();
                  barStyle     = _e.readElementText();
                  }
            else if (_e.name() == "ending") {
                  endingNumber = _e.attributes().value("number").toString();
                  endingType   = _e.attributes().value("type").toString();
                  endingColor  = _e.attributes().value("color").toString();
                  printEnding  = _e.attributes().value("print-object") != "no";
                  endingText   = _e.readElementText();
                  }
            else if (_e.name() == "fermata") {
                  const QColor fermataColor = _e.attributes().value("color").toString();
                  const QString fermataType = _e.attributes().value("type").toString();
                  Segment* const segment = measure->getSegment(SegmentType::EndBarLine, locTick);
                  const int track = _pass1.trackForPart(partId);
                  Fermata* fermata = new Fermata(measure->score());
                  fermata->setSymId(convertFermataToSymId(_e.readElementText()));
                  fermata->setTrack(track);
                  segment->add(fermata);
                  if (fermataColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        fermata->setColor(fermataColor);
                  if (fermataType == "inverted") {
                        fermata->setPlacement(Placement::BELOW);
                        fermata->resetProperty(Pid::OFFSET);
                        }
                  else if (fermataType.isEmpty())
                        fermata->setPlacement(fermata->propertyDefault(Pid::PLACEMENT).value<Placement>());
                  }
            else if (_e.name() == "repeat") {
                  repeat       = _e.attributes().value("direction").toString();
                  count        = _e.attributes().value("times").toString();
                  if (count.isEmpty())
                        count  = "2";
                  measure->setRepeatCount(count.toInt());
                  _e.skipCurrentElement();
                  }
            else
                  skipLogCurrElem();
            }

      BarLineType type = BarLineType::NORMAL;
      bool visible = true;
      if (determineBarLineType(barStyle, repeat, type, visible)) {
            const int track = _pass1.trackForPart(partId);
            if (type == BarLineType::START_REPEAT) {
                  // combine start_repeat flag with current state initialized during measure parsing
                  measure->setRepeatStart(true);
                  }
            else if (type == BarLineType::END_REPEAT) {
                  // combine end_repeat flag with current state initialized during measure parsing
                  measure->setRepeatEnd(true);
                  }
            else {
                  if (barStyle != "regular" || barlineColor.isValid() || loc == "middle") {
                        // Add barline to the first voice of every staff in the part,
                        // and span every barline except the last
                        const Part* part = _pass1.getPart(partId);
                        int nstaves = part->nstaves();
                        for (int i = 0; i < nstaves; ++i ) {
                              const Staff* staff = part->staff(i);
                              bool spanStaff = nstaves > 1 ? i < nstaves - 1 : staff->barLineSpan();
                              int currentTrack = track + (i * VOICES);
                              auto b = createBarline(measure->score(), currentTrack, type, visible, barStyle, spanStaff);
                              if (barlineColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                                    b->setColor(barlineColor);
                              addBarlineToMeasure(measure, locTick, std::move(b));
                              }
                        }
                  }
            }

      doEnding(partId, measure, endingNumber, endingType, endingColor, endingText, printEnding);
      }

//---------------------------------------------------------
//   findRedundantVolta
//---------------------------------------------------------

static Volta* findRedundantVolta(const int track, const Measure* measure)
      {
      auto spanners = measure->score()->spannerMap().findOverlapping(measure->tick().ticks(), measure->endTick().ticks());
      for (auto spanner : spanners) {
            if (spanner.value->isVolta()
             && track2staff(spanner.value->track()) != track2staff(track))
                  return toVolta(spanner.value);
            }
      return 0;
      }

//---------------------------------------------------------
//   doEnding
//---------------------------------------------------------

void MusicXMLParserPass2::doEnding(const QString& partId, Measure* measure, const QString& number,
                                   const QString& type, const QColor color, const QString& text, const bool print)
      {
      if (!(number.isEmpty() && type.isEmpty())) {
            if (number.isEmpty())
                  _logger->logError("empty ending number", &_e);
            else if (type.isEmpty())
                  _logger->logError("empty ending type", &_e);
            else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
                  QStringList sl = number.split(",", Qt::SkipEmptyParts);
#else
                  QStringList sl = number.split(",", QString::SkipEmptyParts);
#endif
                  QList<int> iEndingNumbers;
                  bool unsupported = false;
                  for (const QString& s : qAsConst(sl)) {
                        int iEndingNumber = s.toInt();
                        if (iEndingNumber <= 0) {
                              unsupported = true;
                              break;
                              }
                        iEndingNumbers.append(iEndingNumber);
                        }

                  if (unsupported)
                        _logger->logError(QString("unsupported ending number '%1'").arg(number), &_e);
                  else {
                        // Ignore if it is hidden and redundant
                        Volta* redundantVolta = findRedundantVolta(_pass1.trackForPart(partId), measure);
                        if (!print && redundantVolta)
                              _logger->logDebugInfo("Ignoring redundant hidden Volta", &_e);
                        else if (type == "start") {
                              Volta* volta = new Volta(_score);
                              volta->setTrack(_pass1.trackForPart(partId));
                              volta->setText(text.isEmpty() ? number : text);
                              // LVIFIX TODO also support endings "1 - 3"
                              volta->endings().clear();
                              volta->endings().append(iEndingNumbers);
                              volta->setTick(measure->tick());
                              _score->addElement(volta);
                              _lastVolta = volta;
                              volta->setVisible(print);
                              if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                                    volta->setLineColor(color);
                              }
                        else if (type == "stop") {
                              if (_lastVolta) {
                                    _lastVolta->setVoltaType(Volta::Type::CLOSED);
                                    _lastVolta->setTick2(measure->tick() + measure->ticks());
                                    // Assume print-object was handled at the start
                                    _lastVolta = 0;
                                    }
                              else if (!redundantVolta)
                                    _logger->logError("ending stop without start", &_e);
                              }
                        else if (type == "discontinue") {
                              if (_lastVolta) {
                                    _lastVolta->setVoltaType(Volta::Type::OPEN);
                                    _lastVolta->setTick2(measure->tick() + measure->ticks());
                                    // Assume print-object was handled at the start
                                    _lastVolta = 0;
                                    }
                              else if (!redundantVolta)
                                    _logger->logError("ending discontinue without start", &_e);
                              }
                        else
                              _logger->logError(QString("unsupported ending type '%1'").arg(type), &_e);

                        // Delete any hidden redundant voltas before
                        while (redundantVolta && !redundantVolta->visible()) {
                              _score->removeElement(redundantVolta);
                              delete redundantVolta;
                              redundantVolta = findRedundantVolta(_pass1.trackForPart(partId), measure);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   addSymToSig
//---------------------------------------------------------

/**
 Add a symbol defined as key-step \a step , -alter \a alter and -accidental \a accid to \a sig.
 */

static void addSymToSig(KeySigEvent& sig, const QString& step, const QString& alter, const QString& accid,
                        const QString& smufl)
      {
      //qDebug("addSymToSig(step '%s' alt '%s' acc '%s')",
      //       qPrintable(step), qPrintable(alter), qPrintable(accid));

      SymId id = mxmlString2accSymId(accid, smufl);
      if (id == SymId::noSym) {
            bool ok;
            double d;
            d = alter.toDouble(&ok);
            AccidentalType accTpAlter = ok ? microtonalGuess(d) : AccidentalType::NONE;
            QString s = accidentalType2MxmlString(accTpAlter);
            if (s == "other")
                  s = accidentalType2SmuflMxmlString(accTpAlter);
            id = mxmlString2accSymId(s);
            }

      if (step.size() == 1 && id != SymId::noSym) {
            const QString table = "FEDCBAG";
            const int line = table.indexOf(step);
            // no auto layout for custom keysig, calculate xpos
            // TODO: use symbol width ?
            const qreal spread = 1.4; // assumed glyph width in space
            const qreal x = sig.keySymbols().size() * spread;
            if (line >= 0) {
                  KeySym ks;
                  ks.sym  = id;
                  ks.spos = QPointF(x, qreal(line) * 0.5);
                  sig.keySymbols().append(ks);
                  sig.setCustom(true);
                  }
            }
      }

//---------------------------------------------------------
//   addKey
//---------------------------------------------------------

/**
 Add a KeySigEvent to the score.
 */

static void addKey(const KeySigEvent key, const QColor keyColor, const bool printObj, Score* score,
                   Measure* measure, const int staffIdx, const Fraction& tick)
      {
      Key oldkey = score->staff(staffIdx)->key(tick);
      // TODO only if different custom key ?
      if (oldkey != key.key() || key.custom() || key.isAtonal()) {
            // new key differs from key in effect at this tick
            KeySig* keysig = new KeySig(score);
            keysig->setTrack((staffIdx) * VOICES);
            keysig->setKeySigEvent(key);
            keysig->setVisible(printObj);
            if (keyColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  keysig->setColor(keyColor);
            Segment* s = measure->getSegment(SegmentType::KeySig, tick);
            s->add(keysig);
            //currKeySig->setKeySigEvent(key);
            }
      }

//---------------------------------------------------------
//   flushAlteredTone
//---------------------------------------------------------

/**
 If a valid key-step, -alter, -accidental combination has been read,
 convert it to a key symbol and add to the key.
 Clear key-step, -alter, -accidental.
 */

static void flushAlteredTone(KeySigEvent& kse, QString& step, QString& alt, QString& acc, QString& smufl)
      {
      //qDebug("flushAlteredTone(step '%s' alt '%s' acc '%s')",
      //       qPrintable(step), qPrintable(alt), qPrintable(acc));

      if (step.isEmpty() && alt.isEmpty() && acc.isEmpty())
            return;  // nothing to do

      // step and alt are required, but also accept step and acc
      if (!step.isEmpty() && (!alt.isEmpty() || !acc.isEmpty())) {
            addSymToSig(kse, step, alt, acc, smufl);
            }
      else {
            qDebug("flushAlteredTone invalid combination of step '%s' alt '%s' acc '%s')",
                   qPrintable(step), qPrintable(alt), qPrintable(acc)); // TODO
            }

      // clean up
      step.clear();
      alt.clear();
      acc.clear();
      }

//---------------------------------------------------------
//   key
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/key node.
 */

// TODO: check currKeySig handling

void MusicXMLParserPass2::key(const QString& partId, Measure* measure, const Fraction& tick)
      {
      QString strKeyno = _e.attributes().value("number").toString();
      int keyno = -1; // assume no number (see below)
      if (!strKeyno.isEmpty()) {
            keyno = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strKeyno.toInt());
            if (keyno < 0) {
                  // conversion error (-1), assume staff 0
                  _logger->logError(QString("invalid key number '%1'").arg(strKeyno), &_e);
                  keyno = 0;
                  }
            }
      const bool printObject = _e.attributes().value("print-object") != "no";
      const QColor keyColor = _e.attributes().value("color").toString();

      // for custom keys, a single altered tone is described by
      // key-step (required),  key-alter (required) and key-accidental (optional)
      // none, one or more altered tone may be present
      // a simple state machine is required to detect them
      KeySigEvent key;
      QString keyStep;
      QString keyAlter;
      QString keyAccidental;
      QString smufl;

      while (_e.readNextStartElement()) {
            if (_e.name() == "fifths")
                  key.setKey(Key(_e.readElementText().toInt()));
            else if (_e.name() == "mode") {
                  QString m = _e.readElementText();
                  if (m == "none") {
                        key.setCustom(true);
                        key.setMode(KeyMode::NONE);
                        }
                  else if (m == "major")
                        key.setMode(KeyMode::MAJOR);
                  else if (m == "minor")
                        key.setMode(KeyMode::MINOR);
                  else if (m == "dorian")
                        key.setMode(KeyMode::DORIAN);
                  else if (m == "phrygian")
                        key.setMode(KeyMode::PHRYGIAN);
                  else if (m == "lydian")
                        key.setMode(KeyMode::LYDIAN);
                  else if (m == "mixolydian")
                        key.setMode(KeyMode::MIXOLYDIAN);
                  else if (m == "aeolian")
                        key.setMode(KeyMode::AEOLIAN);
                  else if (m == "ionian")
                        key.setMode(KeyMode::IONIAN);
                  else if (m == "locrian")
                        key.setMode(KeyMode::LOCRIAN);
                  else
                        _logger->logError(QString("Unsupported mode '%1'").arg(m), &_e);
                  }
            else if (_e.name() == "cancel")
                  skipLogCurrElem();  // TODO ??
            else if (_e.name() == "key-step") {
                  flushAlteredTone(key, keyStep, keyAlter, keyAccidental, smufl);
                  keyStep = _e.readElementText();
                  }
            else if (_e.name() == "key-alter")
                  keyAlter = _e.readElementText().trimmed();
            else if (_e.name() == "key-accidental") {
                  smufl = _e.attributes().value("smufl").toString();
                  keyAccidental = _e.readElementText();
                  }
            else
                  skipLogCurrElem();
            }
      flushAlteredTone(key, keyStep, keyAlter, keyAccidental, smufl);

      int nstaves = _pass1.getPart(partId)->nstaves();
      int staffIdx = _pass1.trackForPart(partId) / VOICES;
      if (keyno == -1) {
            // apply key to all staves in the part
            for (int i = 0; i < nstaves; ++i) {
                  addKey(key, keyColor, printObject, _score, measure, staffIdx + i, tick);
                  }
            }
      else if (keyno < nstaves)
            addKey(key, keyColor, printObject, _score, measure, staffIdx + keyno, tick);
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 */

void MusicXMLParserPass2::clef(const QString& partId, Measure* measure, const Fraction& tick)
      {
      ClefType clef = ClefType::G;
      StaffTypes st = StaffTypes::STANDARD;

      QString c;
      int i = 0;
      int line = -1;

      const QString strClefno = _e.attributes().value("number").toString();
      const bool afterBarline = _e.attributes().value("after-barline") == "yes";
      const bool printObject  = _e.attributes().value("print-object") != "no";
      const QColor clefColor  = _e.attributes().value("color").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "sign")
                  c = _e.readElementText();
            else if (_e.name() == "line")
                  line = _e.readElementText().toInt();
            else if (_e.name() == "clef-octave-change") {
                  i = _e.readElementText().toInt();
                  if (i && !(c == "F" || c == "G" || c == "C"))
                        qDebug("clef-octave-change only implemented for F, G and C clef");  // TODO
                  }
            else
                  skipLogCurrElem();
            }

      //some software (Primus) don't include line and assume some default
      // it's permitted by MusicXML 2.0 XSD
      if (line == -1) {
            if (c == "G")
                  line = 2;
            else if (c == "F")
                  line = 4;
            else if (c == "C")
                  line = 3;
            }

      if (c == "G" && i == 0 && line == 2)
            clef = ClefType::G;
      else if (c == "G" && i == 1 && line == 2)
            clef = ClefType::G8_VA;
      else if (c == "G" && i == 2 && line == 2)
            clef = ClefType::G15_MA;
      else if (c == "G" && i == -1 && line == 2)
            clef = ClefType::G8_VB;
      else if (c == "G" && i == -2 && line == 2)
            clef = ClefType::G15_MB;
      else if (c == "G" && i == 0 && line == 1)
            clef = ClefType::G_1;
      else if (c == "F" && i == 0 && line == 3)
            clef = ClefType::F_B;
      else if (c == "F" && i == 0 && line == 4)
            clef = ClefType::F;
      else if (c == "F" && i == 1 && line == 4)
            clef = ClefType::F_8VA;
      else if (c == "F" && i == 2 && line == 4)
            clef = ClefType::F_15MA;
      else if (c == "F" && i == -1 && line == 4)
            clef = ClefType::F8_VB;
      else if (c == "F" && i == -2 && line == 4)
            clef = ClefType::F15_MB;
      else if (c == "F" && i == 0 && line == 5)
            clef = ClefType::F_C;
      else if (c == "C") {
            if (line == 5)
                  clef = ClefType::C5;
            else if (line == 4) {
                  if (i == -1)
                        clef = ClefType::C4_8VB;
                  else
                        clef = ClefType::C4;
                  }
            else if (line == 3)
                  clef = ClefType::C3;
            else if (line == 2)
                  clef = ClefType::C2;
            else if (line == 1)
                  clef = ClefType::C1;
            }
      else if (c == "percussion") {
            clef = ClefType::PERC;
            if (_hasDrumset) {
                  st = StaffTypes::PERC_DEFAULT;
                  }
            }
      else if (c == "TAB") {
            clef = ClefType::TAB;
            st = StaffTypes::TAB_DEFAULT;
            }
      else
            qDebug("clef: unknown clef <sign=%s line=%d oct ch=%d>", qPrintable(c), line, i);  // TODO


      Part* part = _pass1.getPart(partId);
      if (!part)
            return;

      // TODO: check error handling for
      // - single staff
      // - multi-staff with same clef
      int clefno = 0;   // default
      if (!strClefno.isEmpty())
            clefno = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strClefno.toInt());
      if (clefno <= 0 || clefno > part->nstaves()) {
            // conversion error (0) or other issue, assume staff 1
            // Also for Cubase 6.5.5 which generates clef number="2" in a single staff part
            // Same fix is required in pass 1 and pass 2
            _logger->logError(QString("invalid clef number '%1'").arg(strClefno), &_e);
            clefno = 0;
            }

      Clef* clefs = new Clef(_score);
      clefs->setClefType(clef);
      clefs->setVisible(printObject);
      if (clefColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
            clefs->setColor(clefColor);
      int track = _pass1.trackForPart(partId) + clefno * VOICES;
      clefs->setTrack(track);
      Segment* s;
      // check if the clef change needs to be in the previous measure
      if (!afterBarline && (tick == measure->tick()) && measure->prevMeasure())
            s = measure->prevMeasure()->getSegment(SegmentType::Clef, tick);
      else
            s = measure->getSegment(tick.isNotZero() ? SegmentType::Clef : SegmentType::HeaderClef, tick);
      s->add(clefs);

      // set the correct staff type
      // note that clef handling should probably done in pass1
      int staffIdx = _score->staffIdx(part) + clefno;
      int lines = _score->staff(staffIdx)->lines(Fraction(0,1));
      if (tick.isZero()) { // changing staff type not supported (yet ?)
            _score->staff(staffIdx)->setStaffType(tick, *StaffType::preset(st));
            _score->staff(staffIdx)->setLines(tick, lines); // preserve previously set staff lines
            _score->staff(staffIdx)->setBarLineTo(0);    // default
            }
      }

//---------------------------------------------------------
//   determineTimeSig
//---------------------------------------------------------

/**
 Determine the time signature based on \a beats, \a beatType and \a timeSymbol.
 Sets return parameters \a st, \a bts, \a btp.
 Return true if OK, false on error.
 */

// TODO: share between pass 1 and pass 2

static bool determineTimeSig(const QString beats, const QString beatType, const QString timeSymbol,
                             TimeSigType& st, int& bts, int& btp)
      {
      // initialize
      st  = TimeSigType::NORMAL;
      bts = 0;       // the beats (max 4 separated by "+") as integer
      btp = 0;       // beat-type as integer
      // determine if timesig is valid
      if (timeSymbol == "cut")
            st = TimeSigType::ALLA_BREVE;
      else if (timeSymbol == "common")
            st = TimeSigType::FOUR_FOUR;
      else if (timeSymbol == "single-number")
            ; // let pass
      else if (!timeSymbol.isEmpty() && timeSymbol != "normal") {
            qDebug("determineTimeSig: time symbol <%s> not recognized", qPrintable(timeSymbol)); // TODO
            return false;
            }

      btp = beatType.toInt();
      QStringList list = beats.split("+");
      for (int i = 0; i < list.size(); i++)
            bts += list.at(i).toInt();

      // determine if bts and btp are valid
      if (bts <= 0 || btp <=0) {
            qDebug("determineTimeSig: beats=%s and/or beat-type=%s not recognized",
                   qPrintable(beats), qPrintable(beatType));         // TODO
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/time node.
 */

void MusicXMLParserPass2::time(const QString& partId, Measure* measure, const Fraction& tick)
      {
      QString beats;
      QString beatType;
      QString timeSymbol = _e.attributes().value("symbol").toString();
      bool printObject = _e.attributes().value("print-object") != "no";
      const QColor timeColor = _e.attributes().value("color").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "beats")
                  beats = _e.readElementText();
            else if (_e.name() == "beat-type")
                  beatType = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      if (!beats.isEmpty() && !beatType.isEmpty()) {
            // determine if timesig is valid
            TimeSigType st  = TimeSigType::NORMAL;
            int bts = 0; // total beats as integer (beats may contain multiple numbers, separated by "+")
            int btp = 0; // beat-type as integer
            if (determineTimeSig(beats, beatType, timeSymbol, st, bts, btp)) {
                  _timeSigDura = Fraction(bts, btp);
                  Fraction fractionTSig = Fraction(bts, btp);
                  for (int i = 0; i < _pass1.getPart(partId)->nstaves(); ++i) {
                        TimeSig* timesig = new TimeSig(_score);
                        timesig->setVisible(printObject);
                        if (timeColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                              timesig->setColor(timeColor);
                        int track = _pass1.trackForPart(partId) + i * VOICES;
                        timesig->setTrack(track);
                        timesig->setSig(fractionTSig, st);
                        // handle simple compound and single time signatures
                        if (beats.contains(QChar('+'))) {
                              timesig->setNumeratorString(beats);
                              timesig->setDenominatorString(beatType);
                              }
                        else if (timeSymbol == "single-number")
                              timesig->setNumeratorString(beats);
                        Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
                        s->add(timesig);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   divisions
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/divisions node.
 */

void MusicXMLParserPass2::divisions()
      {
      _divs = _e.readElementText().toInt();
      if (!(_divs > 0))
            _logger->logError("illegal divisions", &_e);
      }

//---------------------------------------------------------
//   isWholeMeasureRest
//---------------------------------------------------------

/**
 * Determine whole measure rest.
 */

// By convention, whole measure rests do not have a "type" element
// As of MusicXML 3.0, this can be indicated by an attribute "measure",
// but for backwards compatibility the "old" convention still has to be supported.
// Also verify the rest fits exactly in the measure, as some programs
// (e.g. Cakewalk SONAR X2 Studio [Version: 19.0.0.306]) leave out
// the type for all rests.
// Sibelius calls all whole-measure rests "whole", even if the duration != 4/4

static bool isWholeMeasureRest(const QString& type, const Fraction restDuration, const Fraction measureDuration)
      {
      if (!restDuration.isValid() || !measureDuration.isValid())
            return false;

      return ((type.isEmpty() && restDuration == measureDuration)
              || (type == "whole" && restDuration == measureDuration));
      }

//---------------------------------------------------------
//   determineDuration
//---------------------------------------------------------

/**
 * Determine duration for a note or rest.
 * This includes whole measure rest detection.
 */

static TDuration determineDuration(const bool isRest, const bool measureRest, const QString& type, const int dots, const Fraction chordRestDuration, const Fraction measureDuration)
      {
      //qDebug("determineDuration %s, type <%s>, dots %d, duration %s, measure duration %s",
      //       isRest ? "rest" : "chord", qPrintable(type), dots, qPrintable(chordRestDuration.print()), qPrintable(measureDuration.print()));

      TDuration res;
      if (isRest && (measureRest || isWholeMeasureRest(type, chordRestDuration, measureDuration)))
            res.setType(TDuration::DurationType::V_MEASURE);
      else if (type.isEmpty()) {
            // If no type, set duration type based on duration.
            // Note that sometimes unusual duration (e.g. 261/256) are found.
            res.setVal(chordRestDuration.ticks());
            }
      else {
            res.setType(type);
            res.setDots(dots);
            if (res.type() == TDuration::DurationType::V_INVALID)
                  res.setType(TDuration::DurationType::V_QUARTER);  // default, TODO: use measureDuration ?
            }

      //qDebug("-> duration %hhd (%s) dots %d ticks %s",
      //       (signed char)res.type(), qPrintable(res.name()), res.dots(), qPrintable(chordRestDuration.print()));

      return res;
      }

//---------------------------------------------------------
//   findOrCreateChord
//---------------------------------------------------------

/**
 * Find (or create if not found) the chord at \a tick and \a track.
 * Note: staff move is a note property in MusicXML, but chord property in MuseScore
 * This is simply ignored here, effectively using the last chords value.
 */

static Chord* findOrCreateChord(Score* score, Measure* m,
                                const Fraction& tick, const int track, const int move,
                                const TDuration duration, const Fraction dura,
                                Beam::Mode bm, bool isSmall)
      {
      //qDebug("findOrCreateChord tick %d track %d dur ticks %d ticks %s bm %hhd",
      //       tick, track, duration.ticks(), qPrintable(dura.print()), bm);
      Chord* c = m->findChord(tick, track);
      if (c == 0) {
            c = new Chord(score);
            if (bm == Beam::Mode::END)
                  // The beam palette does not support beam END, use MID instead which means "beam
                  // current note together with previous note".
                  c->setBeamMode(Beam::Mode::MID);
            else
                  c->setBeamMode(bm);
            c->setTrack(track);
            // Chord is initialized with the smallness of its first note.
            // If a non-small note is added later, this is handled in handleSmallness.
            c->setSmall(isSmall);

            setChordRestDuration(c, duration, dura);
            Segment* s = m->getSegment(SegmentType::ChordRest, tick);
            s->add(c);
            }
      c->setStaffMove(move);
      return c;
      }

//---------------------------------------------------------
//   graceNoteType
//---------------------------------------------------------

/**
 * convert duration and slash to grace note type
 */

NoteType graceNoteType(const TDuration duration, const bool slash)
      {
      NoteType nt = NoteType::APPOGGIATURA;
      if (slash) {
            nt = NoteType::ACCIACCATURA;
            }
      else {
            if (duration.type() == TDuration::DurationType::V_QUARTER) {
                  nt = NoteType::GRACE4;
                  }
            else if (duration.type() == TDuration::DurationType::V_16TH) {
                  nt = NoteType::GRACE16;
                  }
            else if (duration.type() == TDuration::DurationType::V_32ND) {
                  nt = NoteType::GRACE32;
                  }
            }
      return nt;
      }

//---------------------------------------------------------
//   createGraceChord
//---------------------------------------------------------

/**
 * Create a grace chord.
 */

static Chord* createGraceChord(Score* score, const int track,
                               const TDuration duration, const bool slash, const bool isSmall)
      {
      Chord* c = new Chord(score);
      c->setNoteType(graceNoteType(duration, slash));
      c->setTrack(track);
      // Chord is initialized with the smallness of its first note.
      // If a non-small note is added later, this is handled in handleSmallness.
      c->setSmall(isSmall);
      // note grace notes have no durations, use default fraction 0/1
      setChordRestDuration(c, duration, Fraction());
      return c;
      }

//---------------------------------------------------------
//   handleDisplayStep
//---------------------------------------------------------

/**
 * convert display-step and display-octave to staff line
 */

static void handleDisplayStep(ChordRest* cr, int step, int octave, const Fraction& tick, qreal spatium)
      {
      if (0 <= step && step <= 6 && 0 <= octave && octave <= 9) {
            //qDebug("rest step=%d oct=%d", step, octave);
            ClefType clef = cr->staff()->clef(tick);
            int po = ClefInfo::pitchOffset(clef);
            //qDebug(" clef=%hhd po=%d step=%d", clef, po, step);
            int dp = 7 * (octave + 2) + step;
            //qDebug(" dp=%d po-dp=%d", dp, po-dp);
            cr->ryoffset() = (po - dp + 3) * spatium / 2;
            }
      }

//---------------------------------------------------------
//   handleSmallness
//---------------------------------------------------------

/**
 * Handle the distinction between small notes and a small
 * chord, to ensure a chord with all small notes is small.
 * This also handles the fact that a small note being added
 * to a small chord should not itself be small.
 * I.e. a chord is "small until proven otherwise".
 */

static void handleSmallness(bool cueOrSmall, Note* note, Chord* c)
      {
      if (cueOrSmall)
            note->setSmall(!c->isSmall()); // Avoid redundant smallness
      else {
            note->setSmall(false);
            if (c->isSmall()) {
                  // What was a small chord becomes small notes in a non-small chord
                  c->setSmall(false);
                  for (Note* otherNote : c->notes())
                        if (note != otherNote)
                              otherNote->setSmall(true);
                  }
            }
      }



//---------------------------------------------------------
//   setNoteHead
//---------------------------------------------------------

/**
 Set the notehead parameters.
 */

static void setNoteHead(Note* note, const QColor noteheadColor, const bool noteheadParentheses, const QString& noteheadFilled)
      {
      Score* const score = note->score();

      if (noteheadColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
            note->setColor(noteheadColor);
      if (noteheadParentheses) {
            Symbol* s = new Symbol(score);
            s->setSym(SymId::noteheadParenthesisLeft);
            s->setParent(note);
            score->addElement(s);
            s = new Symbol(score);
            s->setSym(SymId::noteheadParenthesisRight);
            s->setParent(note);
            score->addElement(s);
            }

      if (noteheadFilled == "no")
            note->setHeadType(NoteHead::Type::HEAD_HALF);
      else if (noteheadFilled == "yes")
            note->setHeadType(NoteHead::Type::HEAD_QUARTER);
      }

//---------------------------------------------------------
//   computeBeamMode
//---------------------------------------------------------

/**
 Calculate the beam mode based on the collected beamTypes.
 */

static Beam::Mode computeBeamMode(const QMap<int, QString>& beamTypes)
      {
      // Start with uniquely-handled beam modes
      if (beamTypes.value(1) == "continue"
          && beamTypes.value(2) == "begin")
            return Beam::Mode::BEGIN32;
      else if (beamTypes.value(1) == "continue"
               && beamTypes.value(2) == "continue"
               && beamTypes.value(3) == "begin")
            return Beam::Mode::BEGIN64;
      // Generic beam modes are naive to all except the first beam
      else if (beamTypes.value(1) == "begin")
            return Beam::Mode::BEGIN;
      else if (beamTypes.value(1) == "continue")
            return Beam::Mode::MID;
      else if (beamTypes.value(1) == "end")
            return Beam::Mode::END;
      else
            // backward-hook, forward-hook, and other unknown combinations
            return Beam::Mode::AUTO;
      }

//---------------------------------------------------------
//   addFiguredBassElemens
//---------------------------------------------------------

/**
 Add the figured bass elements.
 */

static void addFiguredBassElemens(FiguredBassList& fbl, const Fraction noteStartTime, const int msTrack,
                                  const Fraction dura, Measure* measure)
      {
      if (!fbl.isEmpty()) {
            Fraction sTick = noteStartTime;              // starting tick
            for (FiguredBass* fb : fbl) {
                  fb->setTrack(msTrack);
                  // No duration tag defaults ticks() to 0; set to note value
                  if (fb->ticks().isZero())
                        fb->setTicks(dura);
                  // TODO: set correct onNote value
                  Segment* s = measure->getSegment(SegmentType::ChordRest, sTick);
                  s->add(fb);
                  sTick += fb->ticks();
                  }
            fbl.clear();
            }
      }

//---------------------------------------------------------
//   addTremolo
//---------------------------------------------------------

static void addTremolo(ChordRest* cr,
                       const int tremoloNr, const QString& tremoloType, const QString& tremoloSmufl,
                       Chord*& tremStart,
                       MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                       Fraction& timeMod)
      {
      if (!cr->isChord())
            return;
      if (tremoloNr) {
            //qDebug("tremolo %d type '%s' ticks %d tremStart %p", tremoloNr, qPrintable(tremoloType), ticks, _tremStart);
            if (tremoloNr == 1 || tremoloNr == 2 || tremoloNr == 3 || tremoloNr == 4) {
                  if (tremoloType.isEmpty() || tremoloType == "single") {
                        Tremolo* const tremolo = new Tremolo(cr->score());
                        switch (tremoloNr) {
                              case 1: tremolo->setTremoloType(TremoloType::R8); break;
                              case 2: tremolo->setTremoloType(TremoloType::R16); break;
                              case 3: tremolo->setTremoloType(TremoloType::R32); break;
                              case 4: tremolo->setTremoloType(TremoloType::R64); break;
                              }
                        cr->add(tremolo);
                        }
                  else if (tremoloType == "start") {
                        if (tremStart) logger->logError("MusicXML::import: double tremolo start", xmlreader);
                        tremStart = static_cast<Chord*>(cr);
                        // timeMod takes into account also the factor 2 of a two-note tremolo
                        if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                              timeMod.setDenominator(timeMod.denominator() / 2);
                              }
                        }
                  else if (tremoloType == "stop") {
                        if (tremStart) {
                              Tremolo* const tremolo = new Tremolo(cr->score());
                              switch (tremoloNr) {
                                    case 1: tremolo->setTremoloType(TremoloType::C8); break;
                                    case 2: tremolo->setTremoloType(TremoloType::C16); break;
                                    case 3: tremolo->setTremoloType(TremoloType::C32); break;
                                    case 4: tremolo->setTremoloType(TremoloType::C64); break;
                                    }
                              tremolo->setChords(tremStart, static_cast<Chord*>(cr));
                              // fixup chord duration and type
                              const Fraction tremDur = cr->ticks() * Fraction(1,2);
                              tremolo->chord1()->setDurationType(tremDur);
                              tremolo->chord1()->setTicks(tremDur);
                              tremolo->chord2()->setDurationType(tremDur);
                              tremolo->chord2()->setTicks(tremDur);
                              // add tremolo to first chord (only)
                              tremStart->add(tremolo);
                              // timeMod takes into account also the factor 2 of a two-note tremolo
                              if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                                    timeMod.setDenominator(timeMod.denominator() / 2);
                                    }
                              }
                        else logger->logError("MusicXML::import: double tremolo stop w/o start", xmlreader);
                        tremStart = nullptr;
                        }
                  }
            else
                  logger->logError(QString("unknown tremolo type %1").arg(tremoloNr), xmlreader);
            }
      else if (tremoloNr == 0 && (tremoloType == "unmeasured" || tremoloType.isEmpty() || tremoloSmufl == "buzzRoll")) {
            // Out of all the SMuFL unmeasured tremolos, we only support 'buzzRoll'
            Tremolo* const tremolo = new Tremolo(cr->score());
            tremolo->setTremoloType(TremoloType::BUZZ_ROLL);
            cr->add(tremolo);
            }
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

// TODO: refactor: optimize parameters

static void setPitch(Note* note, MusicXMLParserPass1& pass1, const QString& partId, const QString& instrumentId, const mxmlNotePitch& mnp, const int octaveShift, const Instrument* const instrument)
      {
      const MusicXMLInstruments& instruments = pass1.getInstruments(partId);
      if (mnp.unpitched()) {
            if (hasDrumset(instruments)
                && instruments.contains(instrumentId)) {
                  // step and oct are display-step and ...-oct
                  // get pitch from instrument definition in drumset instead
                  int unpitched = instruments[instrumentId].unpitched;
                  note->setPitch(limit(unpitched, 0, 127));
                  // TODO - does this need to be key-aware?
                  note->setTpc(pitch2tpc(unpitched, Key::C, Prefer::NEAREST));       // TODO: necessary ?
                  }
            else {
                  //qDebug("disp step %d oct %d", displayStep, displayOctave);
                  xmlSetPitch(note, mnp.displayStep(), 0, 0.0, mnp.displayOctave(), 0, instrument);
                  }
            }
      else {
            xmlSetPitch(note, mnp.step(), mnp.alter(), mnp.tuning(), mnp.octave(), octaveShift, instrument, pass1.getMusicXmlPart(partId)._inferredTranspose);
            }
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

// set drumset information
// note that in MuseScore, the drumset contains defaults for notehead,
// line and stem direction, while a MusicXML file contains actuals.
// the MusicXML values for each note are simply copied to the defaults

static void setDrumset(Chord* c, MusicXMLParserPass1& pass1, const QString& partId, const QString& instrumentId,
                       const Fraction noteStartTime, const mxmlNotePitch& mnp, const Direction stemDir, const NoteHead::Group headGroup)
      {
      // determine staff line based on display-step / -octave and clef type
      const ClefType clef = c->staff()->clef(noteStartTime);
      const int po = ClefInfo::pitchOffset(clef);
      const int pitch = MusicXMLStepAltOct2Pitch(mnp.displayStep(), 0, mnp.displayOctave());
      int line = po - absStep(pitch);

      // correct for number of staff lines
      // see ExportMusicXml::unpitch2xml for explanation
      // TODO handle other # staff lines ?
      int staffLines = c->staff()->lines(Fraction(0,1));
      if (staffLines == 1) line -= 8;
      if (staffLines == 3) line -= 2;

      // the drum palette cannot handle stem direction AUTO,
      // overrule if necessary
      Direction overruledStemDir = stemDir;
      if (stemDir == Direction::AUTO) {
            if (line > 4)
                  overruledStemDir = Direction::DOWN;
            else
                  overruledStemDir = Direction::UP;
            }
      // this should be done in pass 1, would make _pass1 const here
      pass1.setDrumsetDefault(partId, instrumentId, headGroup, line, overruledStemDir);
      }

//---------------------------------------------------------
//   note
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node.
 */

Note* MusicXMLParserPass2::note(const QString& partId,
                                Measure* measure,
                                const Fraction sTime,
                                const Fraction prevSTime,
                                Fraction& missingPrev,
                                Fraction& dura,
                                Fraction& missingCurr,
                                QString& currentVoice,
                                GraceChordList& gcl,
                                int& gac,
                                Beams& currBeams,
                                FiguredBassList& fbl,
                                int& alt,
                                MxmlTupletStates& tupletStates,
                                Tuplets& tuplets
                                )
      {
      if (_e.attributes().value("print-spacing") == "no") {
            notePrintSpacingNo(dura);
            return 0;
            }

      bool chord = false;
      bool cue = false;
      bool isSmall = false;
      bool grace = false;
      bool rest = false;
      bool measureRest = false;
      int staff = 0;
      QString type;
      QString voice;
      Direction stemDir = Direction::AUTO;
      bool noStem = false;
      bool hasHead = true;
      NoteHead::Group headGroup = NoteHead::Group::HEAD_NORMAL;
      NoteHead::Scheme headScheme = NoteHead::Scheme::HEAD_AUTO;
      const QColor noteColor = _e.attributes().value("color").toString();
      QColor noteheadColor = QColor::Invalid;
      QColor stemColor = QColor::Invalid;
      QColor beamColor = QColor::Invalid;
      bool noteheadParentheses = false;
      QString noteheadFilled;
      int velocity = round(_e.attributes().value("dynamics").toDouble() * 0.9);
      bool graceSlash = false;
      bool printObject = _e.attributes().value("print-object") != "no";
      Beam::Mode bm;
      QMap<int, QString> beamTypes;
      QString instrumentId;
      QString tieType;
      MusicXMLParserLyric lyric { _pass1.getMusicXmlPart(partId).lyricNumberHandler(), _e, _score, _logger };
      MusicXMLParserNotations notations { _e, _score, _logger, _pass1 };

      mxmlNoteDuration mnd { _divs, _logger, &_pass1 };
      mxmlNotePitch mnp { _logger };

      while (_e.readNextStartElement()) {
            if (mnp.readProperties(_e, _score)) {
                  // element handled
                  }
            else if (mnd.readProperties(_e)) {
                  // element handled
                  }
            else if (_e.name() == "beam") {
                  beamColor.setNamedColor(_e.attributes().value("color").toString());
                  beam(beamTypes);
                  }
            else if (_e.name() == "chord") {
                  chord = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "cue") {
                  cue = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "grace") {
                  grace = true;
                  graceSlash = _e.attributes().value("slash") == "yes";
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "instrument") {
                  instrumentId = _e.attributes().value("id").toString();
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "lyric") {
                  // lyrics on grace notes not (yet) supported by MuseScore
                  // add to main note instead
                  lyric.parse();
                  }
            else if (_e.name() == "notations") {
                  notations.parse();
                  addError(notations.errors());
                  }
            else if (_e.name() == "notehead") {
                  noteheadColor.setNamedColor(_e.attributes().value("color").toString());
                  noteheadParentheses = _e.attributes().value("parentheses") == "yes";
                  noteheadFilled = _e.attributes().value("filled").toString();
                  QString noteheadValue = _e.readElementText();
                  if (noteheadValue == "none")
                        hasHead = false;
                  else if (noteheadValue == "named" && _pass1.exporterSoftware() == MusicXMLExporterSoftware::NOTEFLIGHT)
                        headScheme = NoteHead::Scheme::HEAD_PITCHNAME;
                  else
                        headGroup = convertNotehead(noteheadValue);
                  }
            else if (_e.name() == "rest") {
                  rest = true;
                  measureRest = _e.attributes().value("measure") == "yes";
                  mnp.displayStepOctave(_e);
                  }
            else if (_e.name() == "staff") {
                  bool ok = false;
                  QString strStaff = _e.readElementText();
                  staff = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt(&ok));
                  if (!ok) {
                        // error already reported in pass 1
                        staff = -1;
                        }
                  }
            else if (_e.name() == "stem") {
                  stemColor.setNamedColor(_e.attributes().value("color").toString());
                  stem(stemDir, noStem);
                  }
            else if (_e.name() == "tie") {
                  tieType = _e.attributes().value("type").toString();
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "type") {
                  isSmall = (_e.attributes().value("size") == "cue" || _e.attributes().value("size") == "grace-cue");
                  type = _e.readElementText();
                  }
            else if (_e.name() == "voice")
                  voice = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
      if (!chord)
            // remember voice
            currentVoice = voice;
      else if (voice.isEmpty())
            // use voice from last note w/o <chord>
            voice = currentVoice;

      // Assume voice 1 if voice is empty (legal in a single voice part)
      if (voice.isEmpty())
            voice = "1";

      // Define currBeam based on currentVoice to handle multi-voice beaming (and instantiate if not already)
      if (!currBeams.contains(currentVoice))
            currBeams.insert(currentVoice, (Beam *)nullptr);
      Beam* &currBeam = currBeams[currentVoice];

      bm = computeBeamMode(beamTypes);

      // check for timing error(s) and set dura
      // keep in this order as checkTiming() might change dura
      QString errorStr = mnd.checkTiming(type, rest, grace);
      dura = mnd.duration();
      if (!errorStr.isEmpty())
            _logger->logError(errorStr, &_e);

      if (!_pass1.getPart(partId))
            return nullptr;

      // At this point all checks have been done, the note should be added
      // note: in case of error exit from here, the postponed <note> children
      // must still be skipped

      int msMove = 0;
      int msTrack = 0;
      int msVoice = 0;

      int voiceInt = _pass1.voiceToInt(voice);
      if (!_pass1.determineStaffMoveVoice(partId, staff, voiceInt, msMove, msTrack, msVoice)) {
            _logger->logDebugInfo(QString("could not map staff %1 voice '%2'").arg(staff + 1).arg(voiceInt), &_e);
            addError(checkAtEndElement(_e, "note"));
            return 0;
            }

      // start time for note:
      // - sTime for non-chord / first chord note
      // - prevTime for others
      Fraction noteStartTime = chord ? prevSTime : sTime;
      Fraction timeMod = mnd.timeMod();

      // determine tuplet state, used twice (before and after note allocation)
      MxmlTupletFlags tupletAction;

      // handle tuplet state for the previous chord or rest
      if (!chord && !grace) {
            Tuplet* tuplet = tuplets[voice];
            MxmlTupletState& tupletState = tupletStates[voice];
            tupletAction = tupletState.determineTupletAction(mnd.duration(), timeMod, notations.tupletDesc().type, mnd.normalType(), missingPrev, missingCurr);
            if (tupletAction & MxmlTupletFlag::STOP_PREVIOUS) {
                  // tuplet start while already in tuplet
                  if (missingPrev.isValid() && missingPrev > Fraction(0, 1)) {
                        const int track = msTrack + msVoice;
                        Rest* const extraRest = addRest(_score, measure, noteStartTime, track, msMove,
                                                       TDuration { missingPrev* tuplet->ratio() }, missingPrev);
                        if (extraRest) {
                              extraRest->setTuplet(tuplet);
                              tuplet->add(extraRest);
                              noteStartTime += missingPrev;
                              }
                        }
                  // recover by simply stopping the current tuplet first
                  const int normalNotes = timeMod.numerator();
                  handleTupletStop(tuplet, normalNotes);
                  }
            }

      Chord* c { nullptr };
      ChordRest* cr { nullptr };
      Note* note { nullptr };

      TDuration duration = determineDuration(rest, measureRest, type, mnd.dots(), dura, measure->ticks());

      // begin allocation
      if (rest) {
            if (!grace) {
            const int track = msTrack + msVoice;
            cr = addRest(_score, measure, noteStartTime, track, msMove,
                         duration, dura);
                  }
            else
                  qDebug("ignoring grace rest");
            }
      else {
            if (!grace) {
                  // regular note
                  // if there is already a chord just add to it
                  // else create a new one
                  // this basically ignores <chord/> errors
                  c = findOrCreateChord(_score, measure,
                                        noteStartTime,
                                        msTrack + msVoice, msMove,
                                        duration, dura, bm, isSmall || cue);
                  }
            else {
                  // grace note
                  // TODO: check if explicit stem direction should also be set for grace notes
                  // (the DOM parser does that, but seems to have no effect on the autotester)
                  if (!chord || gcl.isEmpty()) {
                        c = createGraceChord(_score, msTrack + msVoice, duration, graceSlash, isSmall || cue);
                        // TODO FIX
                        // the setStaffMove() below results in identical behaviour as 2.0:
                        // grace note will be at the wrong staff with the wrong pitch,
                        // seems to use the line value calculated for the right staff
                        // leaving it places the note at the wrong staff with the right pitch
                        // this affects only grace notes where staff move differs from
                        // the main note, e.g. DebuMandSample.xml first grace in part 2
                        // c->setStaffMove(msMove);
                        // END TODO
                        gcl.append(c);
                        }
                  else
                        c = gcl.last();

                  }
            note = new Note(_score);
            const int ottavaStaff = (msTrack - _pass1.trackForPart(partId)) / VOICES;
            const int octaveShift = _pass1.octaveShift(partId, ottavaStaff, noteStartTime);
            const Part* part = _pass1.getPart(partId);
            const Instrument* instrument = part->instrument(noteStartTime);
            setPitch(note, _pass1, partId, instrumentId, mnp, octaveShift, instrument);
            c->add(note);
            //c->setStemDirection(stemDir); // already done in handleBeamAndStemDir()
            //c->setNoStem(noStem);
            cr = c;
            }
      // end allocation

      if (rest) {
            const int track = msTrack + msVoice;
            if (cr) {
                  if (currBeam) {
                        if (currBeam->track() == track) {
                              cr->setBeamMode(Beam::Mode::MID);
                              currBeam->add(cr);
                              }
                        else
                              removeBeam(currBeam);
                        }
                  else
                        cr->setBeamMode(Beam::Mode::NONE);
                  cr->setSmall(isSmall);
                  if (noteColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        cr->setColor(noteColor);
                  cr->setVisible(printObject);
                  handleDisplayStep(cr, mnp.displayStep(), mnp.displayOctave(), noteStartTime, _score->spatium());
                  }
            }
      else {
            handleSmallness(cue || isSmall, note, c);
            note->setPlay(!cue);          // cue notes don't play
            note->setHeadGroup(headGroup);
            if (headScheme != NoteHead::Scheme::HEAD_AUTO)
                  note->setHeadScheme(headScheme);
            if (noteColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  note->setColor(noteColor);
            Stem* stem = c->stem();
            if (!stem) {
                  stem = new Stem(_score);
                  if (stemColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        stem->setColor(stemColor);
                  else if (noteColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        stem->setColor(noteColor);
                  c->add(stem);
                  }
            setNoteHead(note, noteheadColor, noteheadParentheses, noteheadFilled);
            note->setVisible(hasHead && printObject); // TODO also set the stem to invisible
            stem->setVisible(printObject);

            if (!grace) {
                  handleSmallness(cue || isSmall, note, c);
                  note->setPlay(!cue);          // cue notes don't play
                  note->setHeadGroup(headGroup);
                  if (noteColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        note->setColor(noteColor);
                  setNoteHead(note, noteheadColor, noteheadParentheses, noteheadFilled);
                  note->setVisible(hasHead && printObject); // TODO also set the stem to invisible

                  // regular note
                  // handle beam
                  if (!chord)
                        handleBeamAndStemDir(c, bm, stemDir, currBeam, _pass1.hasBeamingInfo(), beamColor);

                  // append any grace chord after chord to the previous chord
                  Chord*const prevChord = measure->findChord(prevSTime, msTrack + msVoice);
                  if (prevChord && prevChord != c)
                        addGraceChordsAfter(prevChord, gcl, gac);

                  // append any grace chord
                  addGraceChordsBefore(c, gcl);
                  }

            if (mnd.calculatedDuration().isValid()
               && mnd.specifiedDuration().isValid()
               && mnd.calculatedDuration().isNotZero()
               && mnd.calculatedDuration() != mnd.specifiedDuration()) {

                  // convert duration into note length
                  Fraction durationMult { (mnd.specifiedDuration() / mnd.calculatedDuration()).reduced() };
                  durationMult = (1000 * durationMult).reduced();
                  const int noteLen = durationMult.numerator() / durationMult.denominator();

                  NoteEventList nel;
                  NoteEvent ne;
                  ne.setLen(noteLen);
                  nel.append(ne);
                  note->setPlayEvents(nel);
                  if (c)
                        c->setPlayEventType(PlayEventType::User);
                  }

            if (velocity > 0) {
                  note->setVeloType(Note::ValueType::USER_VAL);
                  note->setVeloOffset(velocity);
                  }

            if (mnp.unpitched()) {
                  setDrumset(c, _pass1, partId, instrumentId, noteStartTime, mnp, stemDir, headGroup);
                  }

            // accidental handling
            //qDebug("note acc %p type %hhd acctype %hhd",
            //       acc, acc ? acc->accidentalType() : static_cast<Ms::AccidentalType>(0), accType);
            Accidental* acc = mnp.acc();
            if (!acc && mnp.accType() != AccidentalType::NONE) {
                  acc = new Accidental(_score);
                  acc->setAccidentalType(mnp.accType());
                  }

            if (acc) {
                  acc->setVisible(printObject);
                  note->add(acc);
                  // save alter value for user accidental
                  if (acc->accidentalType() != AccidentalType::NONE)
                        alt = mnp.alter();
                  }

            c->setNoStem(noStem);
            }

      // cr can be 0 here (if a rest cannot be added)
      // TODO: complete and cleanup handling this case
      if (cr)
            cr->setVisible(printObject);

      // handle notations
      if (cr) {
            notations.addToScore(cr, note, noteStartTime.ticks(), _slurs, _glissandi, _spanners, _trills, _ties, _unstartedTieNotes, _unendedTieNotes);

            // if no tie added yet, convert the "tie" into "tied" and add it.
            if (note && !note->tieFor() && !tieType.isEmpty()) {
                  Notation notation { "tied" };
                  const QString ctype { "type" };
                  notation.addAttribute(&ctype, &tieType);
                  addTie(notation, note, cr->track(), _ties, _unstartedTieNotes, _unendedTieNotes, _logger, &_e);
                  }
            }

      // handle grace after state: remember current grace list size
      if (grace && notations.mustStopGraceAFter()) {
            gac = gcl.size();
            }

      // handle tremolo before handling tuplet (two note tremolos modify timeMod)
      if (cr && notations.hasTremolo()) {
            addTremolo(cr, notations.tremoloNr(), notations.tremoloType(), notations.tremoloSmufl(), _tremStart, _logger, &_e, timeMod);
            }

      // handle tuplet state for the current chord or rest
      if (cr) {
            if (!chord && !grace) {
                  Tuplet*& tuplet = tuplets[voice];
                  // do tuplet if valid time-modification is not 1/1 and is not 1/2 (tremolo)
                  // TODO: check interaction tuplet and tremolo handling
                  if (timeMod.isValid() && timeMod != Fraction(1, 1) && timeMod != Fraction(1, 2)) {
                        const int actualNotes = timeMod.denominator();
                        const int normalNotes = timeMod.numerator();
                        if (tupletAction & MxmlTupletFlag::START_NEW) {
                              // create a new tuplet
                              handleTupletStart(cr, tuplet, actualNotes, normalNotes, notations.tupletDesc());
                              }
                        if (tupletAction & MxmlTupletFlag::ADD_CHORD) {
                              cr->setTuplet(tuplet);
                              tuplet->add(cr);
                              }
                        if (tupletAction & MxmlTupletFlag::STOP_CURRENT) {
                              if (missingCurr.isValid() && missingCurr > Fraction(0, 1)) {
                                    qDebug("add missing %s to current tuplet", qPrintable(missingCurr.print()));
                                    const int track = msTrack + msVoice;
                                    Rest* const extraRest = addRest(_score, measure, noteStartTime + dura, track, msMove,
                                                                   TDuration { missingCurr* tuplet->ratio() }, missingCurr);
                                    if (extraRest) {
                                          extraRest->setTuplet(tuplet);
                                          tuplet->add(extraRest);
                                          }
                                    }
                              handleTupletStop(tuplet, normalNotes);
                              }
                        }
                  else if (tuplet) {
                        // stop any still incomplete tuplet
                        handleTupletStop(tuplet, 2);
                        }
                  }
            }

      // Add all lyrics from grace notes attached to this chord
      if (c && !c->graceNotes().empty() && !_graceNoteLyrics.empty()) {
            for (GraceNoteLyrics gnl : _graceNoteLyrics) {
                  if (gnl.lyric) {
                        addLyric(_logger, &_e, cr, gnl.lyric, gnl.no, _extendedLyrics);
                        if (gnl.extend)
                              _extendedLyrics.addLyric(gnl.lyric);
                        }
                  }
            _graceNoteLyrics.clear();
            }

      // add lyrics found by lyric
      if (cr && !grace) {
            // add lyrics and stop corresponding extends
            addLyrics(_logger, &_e, cr, lyric.numberedLyrics(), lyric.extendedLyrics(), _extendedLyrics);
            if (rest) {
                  // stop all extends
                  _extendedLyrics.setExtend(-1, cr->track(), cr->tick());
                  }
            }
      else if (c && grace) {
            // Add grace note lyrics to main chord later
            addGraceNoteLyrics(lyric.numberedLyrics(), lyric.extendedLyrics(), _graceNoteLyrics);
            }

      // add figured bass element
      addFiguredBassElemens(fbl, noteStartTime, msTrack, dura, measure);

      // convert to slash or rhythmic notation if needed
      // TODO in the case of slash notation, we assume that given notes do in fact correspond to slash beats
      if (c && _measureStyleSlash != MusicXmlSlash::NONE) {
            c->setSlash(true, _measureStyleSlash == MusicXmlSlash::SLASH);
            }

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);

      addError(checkAtEndElement(_e, "note"));

      return note;
      }

//---------------------------------------------------------
//   notePrintSpacingNo
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node for a note with print-spacing="no".
 These are handled like a forward: only moving the time forward.
 */

void MusicXMLParserPass2::notePrintSpacingNo(Fraction& dura)
      {
      //_logger->logDebugTrace("MusicXMLParserPass1::notePrintSpacingNo", &_e);

      bool chord = false;
      bool grace = false;

      while (_e.readNextStartElement()) {
            if (_e.name() == "chord") {
                  chord = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "grace") {
                  grace = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else
                  _e.skipCurrentElement();        // skip but don't log
            }

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);

      addError(checkAtEndElement(_e, "note"));
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXMLParserPass2::duration(Fraction& dura)
      {
      dura.set(0, 0);        // invalid unless set correctly
      const QString elementText = _e.readElementText();
      if (elementText.toInt() > 0)
            dura = _pass1.calcTicks(elementText.toInt(), _divs, &_e);
      else
            _logger->logError(QString("illegal duration %1").arg(dura.print()), &_e);
      //qDebug("duration %s valid %d", qPrintable(dura.print()), dura.isValid());
      }

static FiguredBassItem::Modifier MusicXML2Modifier(const QString prefix)
      {
      if (prefix == "sharp")
            return FiguredBassItem::Modifier::SHARP;
      else if (prefix == "flat")
            return FiguredBassItem::Modifier::FLAT;
      else if (prefix == "natural")
            return FiguredBassItem::Modifier::NATURAL;
      else if (prefix == "double-sharp")
            return FiguredBassItem::Modifier::DOUBLESHARP;
      else if (prefix == "flat-flat")
            return FiguredBassItem::Modifier::DOUBLEFLAT;
      else if (prefix == "sharp-sharp")
            return FiguredBassItem::Modifier::DOUBLESHARP;
      else if (prefix == "cross")
            return FiguredBassItem::Modifier::CROSS;
      else if (prefix == "backslash")
            return FiguredBassItem::Modifier::BACKSLASH;
      else if (prefix == "slash")
            return FiguredBassItem::Modifier::SLASH;
      else
            return FiguredBassItem::Modifier::NONE;
      }

//---------------------------------------------------------
//   figure
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/harmony/figured-bass/figure node.
 Return the result as a FiguredBassItem.
 */

FiguredBassItem* MusicXMLParserPass2::figure(const int idx, const bool paren)
      {
      FiguredBassItem* fgi = new FiguredBassItem(_score, idx);

      // read the figure
      while (_e.readNextStartElement()) {
            if (_e.name() == "extend") {
                  QStringRef type = _e.attributes().value("type");
                  if (type == "start")
                        fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
                  else if (type == "continue")
                        fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
                  else if (type == "stop")
                        fgi->setContLine(FiguredBassItem::ContLine::SIMPLE);
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "figure-number") {
                  const QColor color = _e.attributes().value("color").toString();
                  QString val = _e.readElementText();
                  int iVal = val.toInt();
                  // MusicXML spec states figure-number is a number
                  // MuseScore can only handle single digit
                  if (1 <= iVal && iVal <= 9) {
                        fgi->setDigit(iVal);
                        if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                              fgi->setColor(color);
                        }
                  else
                        _logger->logError(QString("incorrect figure-number '%1'").arg(val), &_e);
                  }
            else if (_e.name() == "prefix")
                  fgi->setPrefix(MusicXML2Modifier(_e.readElementText()));
            else if (_e.name() == "suffix")
                  fgi->setSuffix(MusicXML2Modifier(_e.readElementText()));
            else
                  skipLogCurrElem();
            }

      // set parentheses
      if (paren) {
            // parenthesis open
            if (fgi->prefix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth1(FiguredBassItem::Parenthesis::ROUNDOPEN);        // before prefix
            else if (fgi->digit() != FBIDigitNone)
                  fgi->setParenth2(FiguredBassItem::Parenthesis::ROUNDOPEN);        // before digit
            else if (fgi->suffix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth3(FiguredBassItem::Parenthesis::ROUNDOPEN);        // before suffix
            // parenthesis close
            if (fgi->suffix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth4(FiguredBassItem::Parenthesis::ROUNDCLOSED);        // after suffix
            else if (fgi->digit() != FBIDigitNone)
                  fgi->setParenth3(FiguredBassItem::Parenthesis::ROUNDCLOSED);        // after digit
            else if (fgi->prefix() != FiguredBassItem::Modifier::NONE)
                  fgi->setParenth2(FiguredBassItem::Parenthesis::ROUNDCLOSED);        // after prefix
            }

      return fgi;
      }

//---------------------------------------------------------
//   figuredBass
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/harmony/figured-bass node.
 TODO check description:
 // Set the FiguredBass state based on the MusicXML <figured-bass> node de.
 // Note that onNote and ticks must be set by the MusicXML importer,
 // as the required context is not present in the items DOM tree.
 // Exception: if a <duration> element is present, tick can be set.
 Return the result as a FiguredBass if valid, non-empty figure(s) are found.
 Return 0 in case of error.
 */

FiguredBass* MusicXMLParserPass2::figuredBass()
      {
      FiguredBass* fb = new FiguredBass(_score);

      const bool parentheses = _e.attributes().value("parentheses") == "yes";
      const bool printObject = _e.attributes().value("print-object") != "no";
      const QString placement = _e.attributes().value("placement").toString();
      const QColor color = _e.attributes().value("color").toString();

      fb->setVisible(printObject);
      if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/) {
            fb->setColor(color);
      }

      QString normalizedText;
      int idx = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "duration") {
                  Fraction dura;
                  duration(dura);
                  if (dura.isValid() && dura > Fraction(0, 1))
                        fb->setTicks(dura);
                  }
            else if (_e.name() == "figure") {
                  FiguredBassItem* pItem = figure(idx++, parentheses);
                  pItem->setTrack(0 /* TODO fb->track() */);
                  pItem->setParent(fb);
                  fb->appendItem(pItem);
                  // add item normalized text
                  if (!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
            else {
                  skipLogCurrElem();
                  delete fb;
                  return 0;
                  }
            }

      fb->setXmlText(normalizedText);                        // this is the text to show while editing

      fb->setPlacement(placement == "above" ? Placement::ABOVE : Placement::BELOW);
      fb->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      fb->resetProperty(Pid::OFFSET);

      if (normalizedText.isEmpty()) {
            delete fb;
            return 0;
            }

      return fb;
      }

//---------------------------------------------------------
//   frame
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/harmony/frame node.
 Return the result as a FretDiagram.
 Notes:
 - MusicXML's first-fret is a positive integer equivalent to MuseScore's FretDiagram::_fretOffset
 - it is one-based in MusicXML and zero-based in MuseScore
 - in MusicXML fret numbers are absolute, in MuseScore they are relative to the fretOffset,
   which affects both single strings and barres
 */

FretDiagram* MusicXMLParserPass2::frame(qreal& defaultY, qreal& relativeY)
      {
      FretDiagram* fd = new FretDiagram(_score);

      int fretOffset = 0;
      defaultY += _e.attributes().value("default-y").toDouble() * -0.1;
      relativeY += _e.attributes().value("relative-y").toDouble() * -0.1;

      // Format: fret: string
      std::map<int, int> bStarts;
      std::map<int, int> bEnds;

      while (_e.readNextStartElement()) {
            // xs:sequence (elements will appear in this order, as enforced by XML schema)
            if (_e.name() == "frame-strings") {
                  int val = _e.readElementText().toInt();
                  if (val > 0) {
                        fd->setStrings(val);
                        fd->setPropertyFlags(Pid::FRET_STRINGS, PropertyFlags::UNSTYLED); // Prevent style overwrite
                        for (int i = 0; i < val; ++i) {
                              // MXML Spec: any string without a dot or other marker has a closed string
                              // cross marker above it.
                              fd->setMarker(i, FretMarkerType::CROSS);
                              }
                        }
                  else
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-strings %1").arg(val), &_e);
                  }
            else if (_e.name() == "frame-frets") {
                  int val = _e.readElementText().toInt();
                  if (val > 0) {
                        fd->setFrets(val);
                        fd->setPropertyFlags(Pid::FRET_FRETS, PropertyFlags::UNSTYLED); // Prevent style overwrite
                        }
                  else
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-fret %1").arg(val), &_e);
                  }
            else if (_e.name() == "first-fret") {
                  int firstFret = _e.readElementText().toInt();   // MusicXML (1-indexed)
                  fretOffset = firstFret - 1;                     // MuseScore (0-indexed)
                  if (fretOffset >= 0)
                        fd->setFretOffset(fretOffset);
                  else
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal first-fret %1").arg(firstFret), &_e);

                  }
            else if (_e.name() == "frame-note") {
                  // Handle differences in how MusicXML and MuseScore store fret and string information
                  int mxmlString = -1;    // MusicXML (1 = rightmost)
                  int msString = -1;      // MuseScore (0 = leftmost)
                  int mxmlFret = -1;      // MusicXML (absolute)
                  int msFret = -1;        // MuseScore (relative to offset)
                  bool barre = false;     // Flag to prevent barres being duplicated as dots
                  while (_e.readNextStartElement()) {
                        // xs:sequence  (elements will appear in this order, as enforced by XML schema)
                        if (_e.name() == "string") {
                              mxmlString = _e.readElementText().toInt();
                              msString = fd->strings() - mxmlString;
                              }
                        else if (_e.name() == "fret") {
                              mxmlFret = _e.readElementText().toInt();
                              msFret = mxmlFret - fretOffset;
                              }
                        else if (_e.name() == "barre") {
                              // Keep barres to be added later
                              barre = true;
                              QString t = _e.attributes().value("type").toString();
                              if (t == "start")
                                    bStarts[msFret] = msString;
                              else if (t == "stop")
                                    bEnds[msFret] = msString;
                              else
                                    _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-note barre type %1").arg(t), &_e);
                              skipLogCurrElem();
                              }
                        else
                              skipLogCurrElem();
                        }
                  _logger->logDebugInfo(QString("FretDiagram::readMusicXML string %1 fret %2").arg(mxmlString).arg(mxmlFret), &_e);

                  if (mxmlString > 0) {
                        if (mxmlFret == 0)
                              fd->setMarker(msString, FretMarkerType::CIRCLE);
                        else if (msFret > 0 && !barre) {
                              fd->setDot(msString, msFret, true);
                              fd->setMarker(msString, FretMarkerType::NONE);
                              }
                        }
                  else
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-note string %1").arg(mxmlString), &_e);
                  }
            else
                  skipLogCurrElem();
            }

      // Finally add barres
      for (auto const& i : bStarts) {
            int fret = i.first;  // Already in MuseScore format
            int startString = i.second;

            // If end is missing, skip this one
            if (bEnds.find(fret) == bEnds.end())
                  continue;

            int endString = bEnds[fret];
            fd->setBarre(startString, endString, fret);

            // Reset fret marker of all strings covered by barre
            for (int string = startString; string <= endString; ++string) {
                  fd->setMarker(string, FretMarkerType::NONE);
                  }
            }

      return fd;
      }

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/harmony node.
 */

void MusicXMLParserPass2::harmony(const QString& partId, Measure* measure, const Fraction sTime, HarmonyMap& harmonyMap)
      {
      Q_UNUSED(measure);
      int track = _pass1.trackForPart(partId);
      qreal defaultY = 0;
      qreal relativeY = 0;
      bool hasTotalY = false;
      bool tempHasY = false;
      defaultY += _e.attributes().value("default-y").toDouble(&tempHasY) * -0.1;
      hasTotalY |= tempHasY;
      relativeY += _e.attributes().value("relative-y").toDouble(&tempHasY) * -0.1;
      hasTotalY |= tempHasY;
      qreal totalY = defaultY + relativeY;

      const QString placement = _e.attributes().value("placement").toString();
      const bool printObject = _e.attributes().value("print-object") != "no";
      //QString printFrame = _e.attributes().value("print-frame").toString();
      //QString printStyle = _e.attributes().value("print-style").toString();
      const QColor color = _e.attributes().value("color").toString();

      QString kind, kindText, functionText, inversionText, symbols, parens;
      QList<HDegree> degreeList;

      FretDiagram* fd = nullptr;
      Harmony* ha = new Harmony(_score);
      Fraction offset;
      if (!placement.isEmpty()) {
            ha->setPlacement(placement == "below" ? Placement::BELOW : Placement::ABOVE);
            ha->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
            ha->resetProperty(Pid::OFFSET);
            }
      else if (hasTotalY) {
            ha->setPlacement(totalY > 0 ? Placement::BELOW : Placement::ABOVE);
            ha->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
            }

      while (_e.readNextStartElement()) {
            if (_e.name() == "root") {
                  QString step;
                  int alter = 0;
                  bool invalidRoot = false;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "root-step") {
                              step = _e.readElementText();
                              if (_e.attributes().hasAttribute("text")) {
                                    if (_e.attributes().value("text").toString().isEmpty()) {
                                          invalidRoot = true;
                                          }
                                    }
                              }
                        else if (_e.name() == "root-alter") {
                              // attributes: print-object
                              //             location (left-right)
                              alter = _e.readElementText().trimmed().toInt();
                              }
                        else
                              skipLogCurrElem();
                        }
                  if (invalidRoot)
                        ha->setRootTpc(Tpc::TPC_INVALID);
                  else
                        ha->setRootTpc(step2tpc(step, AccidentalVal(alter)));
                  }
            else if (_e.name() == "function") {
                  // deprecated in MusicXML 4.0
                  ha->setRootTpc(Tpc::TPC_INVALID);
                  ha->setBaseTpc(Tpc::TPC_INVALID);
                  functionText = _e.readElementText();
                  ha->setHarmonyType(HarmonyType::ROMAN);
                  }
            else if (_e.name() == "numeral") {
                  ha->setRootTpc(Tpc::TPC_INVALID);
                  ha->setBaseTpc(Tpc::TPC_INVALID);
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "numeral-root") {
                              functionText = _e.attributes().value("text").toString();
                              const QString numeralRoot = _e.readElementText();
                              if (functionText.isEmpty() || functionText.at(0).isDigit()) {
                                    ha->setHarmonyType(HarmonyType::NASHVILLE);
                                    ha->setFunction(numeralRoot);
                                    }
                              else
                                    ha->setHarmonyType(HarmonyType::ROMAN);
                              }
                        else if (_e.name() == "numeral-alter") {
                              const int alter = _e.readElementText().toInt();
                              switch (alter) {
                                    case -1:
                                          ha->setFunction("b" + ha->hFunction());
                                          break;
                                    case 1:
                                          ha->setFunction("#" + ha->hFunction());
                                          break;
                                    default:
                                          break;
                                    }
                              }
                        else
                              skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "kind") {
                  // attributes: use-symbols  yes-no
                  //             text, stack-degrees, parentheses-degree, bracket-degrees,
                  //             halign, valign
                  kindText = _e.attributes().value("text").toString();
                  symbols = _e.attributes().value("use-symbols").toString();
                  parens = _e.attributes().value("parentheses-degrees").toString();
                  kind = _e.readElementText();
                  if (kind == "none") {
                        ha->setRootTpc(Tpc::TPC_INVALID);
                        }
                  }
            else if (_e.name() == "inversion") {
                  const int inversion = _e.readElementText().toInt();
                  switch (inversion) {
                        case 1: inversionText = "6";
                              break;
                        case 2: inversionText = "64";
                              break;
                        default:
                              inversionText = "";
                              break;
                        }
                  }
            else if (_e.name() == "bass") {
                  QString step;
                  int alter = 0;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "bass-step") {
                              step = _e.readElementText();
                              }
                        else if (_e.name() == "bass-alter") {
                              // attributes: print-object
                              //             location (left-right)
                              alter = _e.readElementText().trimmed().toInt();
                              }
                        else
                              skipLogCurrElem();
                        }
                  ha->setBaseTpc(step2tpc(step, AccidentalVal(alter)));
                  }
            else if (_e.name() == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "degree-value") {
                              degreeValue = _e.readElementText().toInt();
                              }
                        else if (_e.name() == "degree-alter") {
                              degreeAlter = _e.readElementText().trimmed().toInt();
                              }
                        else if (_e.name() == "degree-type") {
                              degreeType = _e.readElementText();
                              }
                        else
                              skipLogCurrElem();
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        _logger->logError(QString("incorrect degree: degreeValue=%1 degreeAlter=%2 degreeType=%3")
                                          .arg(degreeValue).arg(degreeAlter).arg(degreeType), &_e);
                        }
                  else {
                        if (degreeType == "add")
                              degreeList << HDegree(degreeValue, degreeAlter, HDegreeType::ADD);
                        else if (degreeType == "alter")
                              degreeList << HDegree(degreeValue, degreeAlter, HDegreeType::ALTER);
                        else if (degreeType == "subtract")
                              degreeList << HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT);
                        }
                  }
            else if (_e.name() == "frame")
                  fd = frame(defaultY, relativeY);
            else if (_e.name() == "level")
                  skipLogCurrElem();
            else if (_e.name() == "offset") {
                  offset = _pass1.calcTicks(_e.readElementText().toInt(), _divs, &_e);
                  preventNegativeTick(sTime, offset, _logger);
                  }
            else if (_e.name() == "staff") {
                  int nstaves = _pass1.getPart(partId)->nstaves();
                  QString strStaff = _e.readElementText();
                  int staff = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt());
                  if (staff >= 0 && staff < nstaves)
                        track += staff * VOICES;
                  else
                        _logger->logError(QString("invalid staff %1").arg(strStaff), &_e);
                  }
            else
                  skipLogCurrElem();
            }

      const ChordDescription* d = nullptr;
      if (ha->rootTpc() != Tpc::TPC_INVALID || ha->harmonyType() == HarmonyType::NASHVILLE)
            d = ha->fromXml(kind, kindText, symbols, parens, degreeList);
      if (d) {
            ha->setId(d->id);
            ha->setTextName(d->names.front());
            }
      else {
            ha->setId(-1);
            QString textName = functionText + kindText + inversionText;
            ha->setTextName(textName);
            }
      ha->render();

      ha->setVisible(printObject);
      if (placement == "below") {
            ha->setPlacement(Placement::BELOW);
            ha->resetProperty(Pid::OFFSET);
            }
      if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/) {
            ha->setColor(color);
            ha->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
            }

      const HarmonyDesc newHarmonyDesc(track, ha, fd);
      bool insert = true;
      if (_pass1.sibOrDolet()) {
            const int ticks = (sTime + offset).ticks();
            for (auto itr = harmonyMap.begin(); itr != harmonyMap.end(); itr++) {
                  if (itr->first != ticks)
                        continue;
                  HarmonyDesc& foundHarmonyDesc = itr->second;
                  if (track2staff(foundHarmonyDesc._track) == track2staff(track) && foundHarmonyDesc._harmony->descr() == ha->descr()) {
                        if (foundHarmonyDesc._harmony && foundHarmonyDesc.fretDiagramVisible() == newHarmonyDesc.fretDiagramVisible() && !(fd && fd->visible())) {
                              // Matching harmony with a fret diagram already at this tick, no need to add another chord symbol
                              insert = false;
                              }
                        else if (fd && fd->visible() && !foundHarmonyDesc.fretDiagramVisible()) {
                              // Matching harmony without a fret diagram found at this tick, replace with this harmony and its fret diagram
                              foundHarmonyDesc._harmony = ha;
                              foundHarmonyDesc._fretDiagram = fd;
                              foundHarmonyDesc._track = track;
                              insert = false;
                              }
                        }
                  }
            }

      if (insert) // No harmony at this tick, add to the map
            harmonyMap.insert(std::pair<int, HarmonyDesc>((sTime + offset).ticks(), newHarmonyDesc));
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/beam node.
 Collects beamTypes, used in computeBeamMode.
 */

void MusicXMLParserPass2::beam(QMap<int, QString>& beamTypes) {
      bool hasBeamNo;
      int beamNo = _e.attributes().value("number").toInt(&hasBeamNo);
      QString s = _e.readElementText();

      beamTypes.insert(hasBeamNo ? beamNo : 1, s);
      }

//---------------------------------------------------------
//   forward
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/forward node.
 */

void MusicXMLParserPass2::forward(Fraction& dura)
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "staff")
                  _e.skipCurrentElement();  // skip but don't log
            else if (_e.name() == "voice")
                  _e.skipCurrentElement();  // skip but don't log
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   backup
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/backup node.
 */

void MusicXMLParserPass2::backup(Fraction& dura)
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "duration")
                  duration(dura);
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   MusicXMLParserLyric
//---------------------------------------------------------

MusicXMLParserLyric::MusicXMLParserLyric(const LyricNumberHandler lyricNumberHandler,
                                         QXmlStreamReader& e, Score* score, MxmlLogger* logger)
      : _lyricNumberHandler(lyricNumberHandler), _e(e), _score(score), _logger(logger)
      {
      // nothing
      }

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXMLParserLyric::skipLogCurrElem()
      {
      //_logger->logDebugInfo(QString("skipping '%1'").arg(_e.name().toString()), &_e);
      _e.skipCurrentElement();
      }

void MusicXMLParserLyric::readElision(QString& formattedText)
      {
      const QString text = _e.readElementText();
      const QString smufl = _e.attributes().value("smufl").toString();
      if (!text.isEmpty())
            formattedText += text;
      else if (!smufl.isEmpty())
            formattedText += "<sym>" + smufl + "</sym>";
      else
            formattedText += "<sym>lyricsElision</sym>";
      }

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

void MusicXMLParserLyric::parse()
      {
      std::unique_ptr<Lyrics> lyric { new Lyrics(_score) };
      // TODO in addlyrics: l->setTrack(trk);

      bool hasExtend = false;
      const QString lyricNumber = _e.attributes().value("number").toString();
      const QColor lyricColor = _e.attributes().value("color").toString();
      const bool printLyric = _e.attributes().value("print-object") != "no";
      _placement = _e.attributes().value("placement").toString();
      qreal relX = _e.attributes().value("relative-x").toDouble() / 10 * DPMM;
      _relativeY = _e.attributes().value("relative-y").toDouble() / 10 * DPMM;
      _defaultY = _e.attributes().value("default-y").toDouble() * -0.1 * DPMM;

      QString extendType;
      QString formattedText;

      while (_e.readNextStartElement()) {
            if (_e.name() == "elision")
                  readElision(formattedText);
            else if (_e.name() == "extend") {
                  hasExtend = true;
                  extendType = _e.attributes().value("type").toString();
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "syllabic") {
                  QString syll = _e.readElementText();
                  if (syll == "single")
                        lyric->setSyllabic(Lyrics::Syllabic::SINGLE);
                  else if (syll == "begin")
                        lyric->setSyllabic(Lyrics::Syllabic::BEGIN);
                  else if (syll == "end")
                        lyric->setSyllabic(Lyrics::Syllabic::END);
                  else if (syll == "middle")
                        lyric->setSyllabic(Lyrics::Syllabic::MIDDLE);
                  else
                        qDebug("unknown syllabic %s", qPrintable(syll));              // TODO
                  }
            else if (_e.name() == "text")
                  formattedText += nextPartOfFormattedString(_e);
            else
                  skipLogCurrElem();
            }

      // if no lyric read (e.g. only 'extend "type=stop"'), no further action required
      if (formattedText.isEmpty()) {
            return;
            }

      const int lyricNo = _lyricNumberHandler.getLyricNo(lyricNumber);
      if (lyricNo < 0) {
            _logger->logError("invalid lyrics number (<0)", &_e);
            return;
            }
      else if (lyricNo > MAX_LYRICS) {
            _logger->logError(QString("too much lyrics (>%1)").arg(MAX_LYRICS), &_e);
            return;
            }
      else if (_numberedLyrics.contains(lyricNo)) {
            _logger->logError(QString("duplicate lyrics number (%1)").arg(lyricNumber), &_e);
            return;
            }

      //qDebug("formatted lyric '%s'", qPrintable(formattedText));
      lyric->setXmlText(formattedText);
      if (lyricColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/) {
            lyric->setProperty(Pid::COLOR, lyricColor);
            lyric->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
            }
      lyric->setVisible(printLyric);

      lyric->setPlacement(placement() == "above" ? Placement::ABOVE : Placement::BELOW);
      lyric->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      lyric->resetProperty(Pid::OFFSET);

      if (!qFuzzyIsNull(relX)) {
            QPointF offset = lyric->offset();
            offset.setX(relX);
            lyric->setOffset(offset);
            lyric->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
            }

      Lyrics* const l = lyric.release();
      _numberedLyrics[lyricNo] = l;

      if (hasExtend
         && (extendType.isEmpty() || extendType == "start")
         && (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END)
         )
            _extendedLyrics.insert(l);
      }

QString MusicXMLParserLyric::placement() const
      {
      if (_placement == "" && hasTotalY())
            return totalY() < 0 ? "above" : "below";
      else
            return _placement;
      }

//---------------------------------------------------------
//   slur
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/slur node.
 */

void MusicXMLParserNotations::slur()
      {
      Notation notation = Notation::notationWithAttributes(_e.name().toString(), _e.attributes(), "notations");
      _notations.push_back(notation);

      // any grace note containing a slur stop means
      // last note of a grace after set has been found
      // -> remember slur stop
      if (notation.attribute("type") == "stop") {
            _slurStop = true;
            }
      else if (notation.attribute("type") == "start") {
            _slurStart = true;
            }

      _e.skipCurrentElement();  // skip but don't log
      }

//---------------------------------------------------------
//   addSlur
//---------------------------------------------------------

static void addSlur(const Notation& notation, SlurStack& slurs, ChordRest* cr, const int tick,
                    MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      int slurNo = notation.attribute("number").toInt();
      if (slurNo > 0)
            slurNo--;
      const QString slurType = notation.attribute("type");

      const int track = cr->track();
      Score* score = cr->score();

      // PriMus Music-Notation by Columbussoft (build 10093) generates overlapping
      // slurs that do not have a number attribute to distinguish them.
      // The duplicates must be ignored, to prevent memory allocation issues,
      // which caused a MuseScore crash
      // Similar issues happen with Sibelius 7.1.3 (direct export)

      if (slurType == "start") {
            if (slurs[slurNo].isStart())
                  // slur start when slur already started: report error
                  logger->logError(QString("ignoring duplicate slur start"), xmlreader);
            else if (slurs[slurNo].isStop()) {
                  // slur start when slur already stopped: wrap up
                  Slur* newSlur = slurs[slurNo].slur();
                  newSlur->setTick(Fraction::fromTicks(tick));
                  newSlur->setStartElement(cr);
                  slurs[slurNo] = SlurDesc();
                  }
            else {
                  // slur start for new slur: init
                  Slur* newSlur = new Slur(score);
                  if (cr->isGrace())
                        newSlur->setAnchor(Spanner::Anchor::CHORD);
                  const QString lineType = notation.attribute("line-type");
                  if (lineType == "solid" || lineType.isEmpty())
                        newSlur->setLineType(0);
                  else if (lineType == "dotted")
                        newSlur->setLineType(1);
                  else if (lineType == "dashed")
                        newSlur->setLineType(2);
                  const QColor color = notation.attribute("color");
                  if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT))*/)
                        newSlur->setColor(color);
                  newSlur->setTick(Fraction::fromTicks(tick));
                  newSlur->setStartElement(cr);
                  if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)) {
                        const QString orientation = notation.attribute("orientation");
                        const QString placement = notation.attribute("placement");
                        if (orientation == "over" || placement == "above")
                              newSlur->setSlurDirection(Direction::UP);
                        else if (orientation == "under" || placement == "below")
                              newSlur->setSlurDirection(Direction::DOWN);
                        else if (orientation.isEmpty() || placement.isEmpty())
                              ; // ignore
                        else
                              logger->logError(QString("unknown slur orientation/placement: %1/%2").arg(orientation, placement), xmlreader);
                        }

                  newSlur->setTrack(track);
                  newSlur->setTrack2(track);
                  slurs[slurNo].start(newSlur);
                  score->addElement(newSlur);
                  }
            }
      else if (slurType == "stop") {
            if (slurs[slurNo].isStart()) {
                  // slur stop when slur already started: wrap up
                  Slur* newSlur = slurs[slurNo].slur();
                  if (!(cr->isGrace())) {
                        newSlur->setTick2(Fraction::fromTicks(tick));
                        newSlur->setTrack2(track);
                        }
                  newSlur->setEndElement(cr);
                  slurs[slurNo] = SlurDesc();
                  }
            else if (slurs[slurNo].isStop())
                  // slur stop when slur already stopped: report error
                  logger->logError(QString("ignoring duplicate slur stop"), xmlreader);
            else {
                  // slur stop for new slur: init
                  Slur* newSlur = new Slur(score);
                  if (!(cr->isGrace())) {
                        newSlur->setTick2(Fraction::fromTicks(tick));
                        newSlur->setTrack2(track);
                        }
                  newSlur->setEndElement(cr);
                  slurs[slurNo].stop(newSlur);
                  }
            }
      else if (slurType == "continue")
            ;        // ignore
      else
            logger->logError(QString("unknown slur type %1").arg(slurType), xmlreader);
      }

//---------------------------------------------------------
//   tied
//---------------------------------------------------------


/**
 Parse the /score-partwise/part/measure/note/notations/tied node.
 */

void MusicXMLParserNotations::tied()
      {
      Notation notation = Notation::notationWithAttributes(_e.name().toString(), _e.attributes(), "notations");
      // Make sure "stops" get processed before "starts"
      if (notation.attribute("type") == "stop")
            _notations.insert(_notations.begin(), notation);
      else
            _notations.push_back(notation);
      QString tiedType = notation.attribute("type");
      if (tiedType != "start" && tiedType != "stop" && tiedType != "let-ring") {
            _logger->logError(QString("unknown tied type %1").arg(tiedType), &_e);
            }

      _e.skipCurrentElement();  // skip but don't log
      }

//---------------------------------------------------------
//   dynamics
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/dynamics node.
 */

void MusicXMLParserNotations::dynamics()
      {
      _dynamicsColor = _e.attributes().value("color").toString();
      _dynamicsPlacement = _e.attributes().value("placement").toString();

      while (_e.readNextStartElement()) {
            if (_e.name() == "other-dynamics")
                  _dynamicsList.push_back(_e.readElementText());
            else {
                  _dynamicsList.push_back(_e.name().toString());
                  _e.skipCurrentElement();  // skip but don't log
                  }
            }
      }

//---------------------------------------------------------
//   articulations
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/articulations node.
 Note that some notations attach to notes only in MuseScore,
 which means trying to attach them to a rest will crash,
 as in that case note is 0.
 */

void MusicXMLParserNotations::articulations()
      {
      while (_e.readNextStartElement()) {
            SymId id = SymId::noSym;
            if (convertArticulationToSymId(_e.name().toString(), id)) {
                  if (_e.name() == "detached-legato") {
                        _notations.push_back(Notation::notationWithAttributes("tenuto",
                                                                              _e.attributes(), "articulations", SymId::articTenutoAbove));
                        _notations.push_back(Notation::notationWithAttributes("staccato",
                                                                              _e.attributes(), "articulations", SymId::articStaccatoAbove));
                        }
                  else {
                        Notation artic = Notation::notationWithAttributes(_e.name().toString(),
                                                                          _e.attributes(), "articulations", id);
                        _notations.push_back(artic);
                        }
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "breath-mark") {
                  QXmlStreamAttributes attributes = _e.attributes();
                  QString value = _e.readElementText();
                  SymId breath = SymId::noSym;
                  if (value == "tick")
                        breath = SymId::breathMarkTick;
                  else if (value == "upbow")
                        breath = SymId::breathMarkUpbow;
                  else if (value == "salzedo")
                        breath = SymId::breathMarkSalzedo;
                  else // Use comma as the default symbol
                        breath = SymId::breathMarkComma;
                  _notations.push_back(Notation::notationWithAttributes("breath",
                                                                         attributes, "articulations", breath));
                  }
            else if (_e.name() == "caesura") {
                  QXmlStreamAttributes attributes = _e.attributes();
                  QString value = _e.readElementText();
                  SymId caesura = SymId::noSym;
                  if (value == "curved")
                        caesura = SymId::caesuraCurved;
                  else if (value == "short")
                        caesura = SymId::caesuraShort;
                  else if (value == "thick")
                        caesura = SymId::caesuraThick;
                  else if (value == "single")
                        caesura = SymId::caesuraSingleStroke;
                  else // Use as the default symbol
                        caesura = SymId::caesura;
                  _notations.push_back(Notation::notationWithAttributes("breath",
                                                                         attributes, "articulations", caesura));
                  }
            else if (_e.name() == "doit"
                     || _e.name() == "falloff"
                     || _e.name() == "plop"
                     || _e.name() == "scoop") {
                  Notation artic = Notation::notationWithAttributes("chord-line",
                                                                    _e.attributes(), "articulations");
                  artic.setSubType(_e.name().toString());
                  _notations.push_back(artic);
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "other-articulation") {
                  const QString smufl = _e.attributes().value("smufl").toString();

                  if (!smufl.isEmpty()) {
                        Notation artic = Notation::notationWithAttributes(_e.name().toString(),
                                                                               _e.attributes(), "articulations", id);
                        _notations.push_back(artic);
                        }
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else {
                  skipLogCurrElem();
                  }
            }
      }

//---------------------------------------------------------
//   ornaments
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/ornaments node.
 */

void MusicXMLParserNotations::ornaments()
      {
      bool trillMark = false;
      // <trill-mark placement="above"/>
      while (_e.readNextStartElement()) {
            SymId id = SymId::noSym;
            if (convertArticulationToSymId(_e.name().toString(), id)) {
                  Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                       _e.attributes(), "ornaments", id);
                  _notations.push_back(notation);
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "trill-mark") {
                  trillMark = true;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "wavy-line") {
                  bool wavyLineTypeWasStart = (_wavyLineType == "start");
                  _wavyLineType = _e.attributes().value("type").toString();
                  _wavyLineNo   = _e.attributes().value("number").toString().toInt();
                  if (_wavyLineNo > 0) _wavyLineNo--;
                  // any grace note containing a wavy-line stop means
                  // last note of a grace after set has been found
                  // remember wavy-line stop
                  if (_wavyLineType == "stop") {
                        _wavyLineStop = true;
                        }
                  // check for start and stop on same note
                  if (wavyLineTypeWasStart && _wavyLineType == "stop") {
                        _wavyLineType = "startstop";
                        }
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "tremolo") {
                  _hasTremolo = true;
                  _tremoloType = _e.attributes().value("type").toString();
                  _tremoloNr = _e.readElementText().toInt();
                  _tremoloSmufl = _e.attributes().value("smufl").toString();
                  }
            else if (_e.name() == "inverted-mordent"
                     || _e.name() == "mordent") {
                  mordentNormalOrInverted();
                  }
            else if (_e.name() == "other-ornament") {
                  Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                       _e.attributes(), "ornaments");
                  _notations.push_back(notation);
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else {
                  skipLogCurrElem();
                  }
            }

      // note that mscore wavy line already implicitly includes a trillsym
      // so don't add an additional one
      if (trillMark && _wavyLineType != "start" && _wavyLineType != "startstop") {
            Notation ornament = Notation::notationWithAttributes("trill-mark", _e.attributes(), "ornaments", SymId::ornamentTrill);
            _notations.push_back(ornament);
            }
      }

//---------------------------------------------------------
//   technical
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical node.
 */

void MusicXMLParserNotations::technical()
      {
      while (_e.readNextStartElement()) {
            SymId id = SymId::noSym;
            if (convertArticulationToSymId(_e.name().toString(), id)) {
                  Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                       _e.attributes(), "technical", id);
                  _notations.push_back(notation);
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "fingering" || _e.name() == "fret" || _e.name() == "pluck" || _e.name() == "string") {
                  Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                       _e.attributes(), "technical");
                  notation.setText(_e.readElementText());
                  _notations.push_back(notation);
                  }
            else if (_e.name() == "harmonic")
                  harmonic();
            else if (_e.name() == "handbell") {
                  const QXmlStreamAttributes attributes = _e.attributes();
                  convertArticulationToSymId(_e.readElementText(), id);
                  _notations.push_back(Notation::notationWithAttributes(_e.name().toString(),
                                                                        attributes, "technical", id));
                  }
            else if (_e.name() == "harmon-mute")
                  harmonMute();
            else if (_e.name() == "hole")
                  hole();
            else if (_e.name() == "other-technical")
                  otherTechnical();
            else
                  skipLogCurrElem();
            }
      }


void MusicXMLParserNotations::otherTechnical()
      {
      const QString smufl = _e.attributes().value("smufl").toString();

      if (!smufl.isEmpty()) {
            SymId id = Sym::name2id(smufl);
            Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                               _e.attributes(), "technical", id);
            _notations.push_back(notation);
            _e.skipCurrentElement();
            return;
            }

      const QString text = _e.readElementText();

      if (text == "z") {
            // Buzz roll
            _hasTremolo = true;
            _tremoloNr = 0;
            _tremoloType = "unmeasured";
            }
      }

//---------------------------------------------------------
//   harmonic
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical/harmonic node.
 */

void MusicXMLParserNotations::harmonic()
      {
      Notation notation = Notation::notationWithAttributes(_e.name().toString(), _e.attributes(), "technical", SymId::stringsHarmonic);

      while (_e.readNextStartElement()) {
            QString name = _e.name().toString();
            if (name == "natural") {
                  notation.setSubType(name);
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else { // TODO: add artificial harmonic when supported by musescore
                  _logger->logError(QString("unsupported harmonic type/pitch '%1'").arg(name), &_e);
                  _e.skipCurrentElement();
                  }
            }

      if (!notation.subType().isEmpty()) {
            _notations.push_back(notation);
            }
      }

//---------------------------------------------------------
//   harmonMute
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical/harmon-mute node.
 */

void MusicXMLParserNotations::harmonMute()
      {
      SymId mute = SymId::brassHarmonMuteClosed;
      const QXmlStreamAttributes attributes = _e.attributes();
      while (_e.readNextStartElement()) {
            QString name = _e.name().toString();
            if (name == "harmon-closed") {
                  const QString location = _e.attributes().value("location").toString();
                  QString value = _e.readElementText();
                  if (value == "yes")
                        mute = SymId::brassHarmonMuteClosed;
                  else if (value == "no")
                        mute = SymId::brassHarmonMuteStemOpen;
                  else if (value == "half") {
                        if (location == "left")
                              mute = SymId::brassHarmonMuteStemHalfLeft;
                        else if (location == "right")
                              mute = SymId::brassHarmonMuteStemHalfRight;
                        else {
                              _logger->logError(QString("unsupported harmon-closed location '%1'").arg(location), &_e);
                              mute = SymId::brassHarmonMuteStemHalfLeft;
                              }
                        }
                  } else
                  _e.skipCurrentElement();
            }
      _notations.push_back(Notation::notationWithAttributes("harmon-closed", attributes, "technical", mute));
      }

//---------------------------------------------------------
//   hole
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical/hole node.
 */

void MusicXMLParserNotations::hole()
      {
      SymId hole = SymId::noSym;
      const QXmlStreamAttributes attributes = _e.attributes();
      while (_e.readNextStartElement()) {
            if (_e.name() == "hole-closed") {
                  const QString location = _e.attributes().value("location").toString();
                  const QString value = _e.readElementText();
                  if (value == "yes")
                        hole = SymId::windClosedHole;
                  else if (value == "no")
                        hole = SymId::windOpenHole;
                  else if (value == "half") {
                        if (location == "bottom")
                              hole = SymId::windHalfClosedHole2;
                        else if (location == "right")
                              hole = SymId::windHalfClosedHole1;
                        else {
                              _logger->logError(QString("unsupported hole-closed location '%1'").arg(location), &_e);
                              hole = SymId::windHalfClosedHole3;
                              }
                        }
                  }
            else
                  _e.skipCurrentElement();
            }
      _notations.push_back(Notation::notationWithAttributes("hole-closed", attributes, "technical", hole));
      }

//---------------------------------------------------------
//   addTechnical
//---------------------------------------------------------

void MusicXMLParserNotations::addTechnical(const Notation& notation, Note* note)
      {
      const QString placement = notation.attribute("placement");
      const QString fontWeight = notation.attribute("font-weight");
      const qreal fontSize = notation.attribute("font-size").toDouble();
      const QString fontStyle = notation.attribute("font-style");
      const QString fontFamily = notation.attribute("font-family");
      const QColor color = notation.attribute("color");
      if (notation.name() == "fingering") {
            // TODO: distinguish between keyboards (style Tid::FINGERING)
            // and (plucked) strings (style Tid::LH_GUITAR_FINGERING)
            addTextToNote(_e.lineNumber(), _e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                          color, Tid::FINGERING, _score, note);
            }
      else if (notation.name() == "fret") {
            int fret = notation.text().toInt();
            if (note) {
                  if (note->staff()->isTabStaff(Fraction(0,1)))
                        note->setFret(fret);
                  }
            else
                  _logger->logError("no note for fret", &_e);
            }
      else if (notation.name() == "pluck") {
            addTextToNote(_e.lineNumber(), _e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                          color, Tid::RH_GUITAR_FINGERING, _score, note);
            }
      else if (notation.name() == "string") {
            if (note) {
                  if (note->staff()->isTabStaff(Fraction(0,1)))
                        note->setString(notation.text().toInt() - 1);
                  else
                        addTextToNote(_e.lineNumber(), _e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                                      color, Tid::STRING_NUMBER, _score, note);
                  }
            else
                  _logger->logError("no note for string", &_e);
            }
      }

//---------------------------------------------------------
//   mordentNormalOrInverted
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/ornaments/mordent
 and /score-partwise/part/measure/note/notations/ornaments/inverted-mordent nodes.
 */

void MusicXMLParserNotations::mordentNormalOrInverted()
      {
      Notation notation = Notation::notationWithAttributes(_e.name().toString(), _e.attributes(), "ornaments");
      notation.setText(_e.readElementText());
      _notations.push_back(notation);
      }

//---------------------------------------------------------
//   glissandoSlide
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/glissando
 and /score-partwise/part/measure/note/notations/slide nodes.
 */

void MusicXMLParserNotations::glissandoSlide()
      {
      Notation notation = Notation::notationWithAttributes(_e.name().toString(), _e.attributes(), "notations");
      notation.setText(_e.readElementText());
      _notations.push_back(notation);
      }

//---------------------------------------------------------
//   addGlissandoSlide
//---------------------------------------------------------

static void addGlissandoSlide(const Notation& notation, Note* note,
                              Glissando* glissandi[MAX_NUMBER_LEVEL][2], MusicXmlSpannerMap& spanners,
                              MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      int glissandoNumber = notation.attribute("number").toInt();
      if (glissandoNumber > 0) glissandoNumber--;
      const QString glissandoType = notation.attribute("type");
      int glissandoTag = notation.name() == "slide" ? 0 : 1;
      //                  QString lineType  = ee.attribute(QString("line-type"), "solid");
      Glissando*& gliss = glissandi[glissandoNumber][glissandoTag];

      const Fraction tick = note->tick();
      const int track = note->track();
      Score* score = note->score();

      if (glissandoType == "start") {
            const QColor glissandoColor = notation.attribute("color");
            const QString glissandoText = notation.text();
            if (gliss) {
                  logger->logError(QString("overlapping glissando/slide number %1").arg(glissandoNumber+1), xmlreader);
                  }
            else if (!note) {
                  logger->logError(QString("no note for glissando/slide number %1 start").arg(glissandoNumber+1), xmlreader);
                  }
            else {
                  gliss = new Glissando(score);
                  gliss->setAnchor(Spanner::Anchor::NOTE);
                  gliss->setStartElement(note);
                  gliss->setTick(tick);
                  gliss->setTrack(track);
                  gliss->setParent(note);
                  if (glissandoColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        gliss->setLineColor(glissandoColor);
                  gliss->setText(glissandoText);
                  gliss->setGlissandoType(glissandoTag == 0 ? GlissandoType::STRAIGHT : GlissandoType::WAVY);
                  spanners[gliss] = QPair<int, int>(tick.ticks(), -1);
                  // qDebug("glissando/slide=%p inserted at first tick %d", gliss, tick);
                  }
            }
      else if (glissandoType == "stop") {
            if (!gliss) {
                  logger->logError(QString("glissando/slide number %1 stop without start").arg(glissandoNumber+1), xmlreader);
                  }
            else if (!note) {
                  logger->logError(QString("no note for glissando/slide number %1 stop").arg(glissandoNumber+1), xmlreader);
                  }
            else {
                  spanners[gliss].second = tick.ticks() + note->chord()->ticks().ticks();;
                  gliss->setEndElement(note);
                  gliss->setTick2(tick);
                  gliss->setTrack2(track);
                  // qDebug("glissando/slide=%p second tick %d", gliss, tick);
                  gliss = nullptr;
                  }
            }
      else
            logger->logError(QString("unknown glissando/slide type %1").arg(glissandoType), xmlreader);
      }

//---------------------------------------------------------
//   addArpeggio
//---------------------------------------------------------

static void addArpeggio(ChordRest* cr, const QString& arpeggioType, QColor arpeggioColor)
      {
      // no support for arpeggio on rest
      if (!arpeggioType.isEmpty() && cr->type() == ElementType::CHORD) {
            std::unique_ptr<Arpeggio> arpeggio { new Arpeggio(cr->score()) };
            arpeggio->setArpeggioType(ArpeggioType::NORMAL);
            if (arpeggioType == "up")
                  arpeggio->setArpeggioType(ArpeggioType::UP);
            else if (arpeggioType == "down")
                  arpeggio->setArpeggioType(ArpeggioType::DOWN);
            else if (arpeggioType == "non-arpeggiate")
                  arpeggio->setArpeggioType(ArpeggioType::BRACKET);
            if (arpeggioColor.isValid())
                  arpeggio->setColor(arpeggioColor);
            // there can be only one
            if (!(static_cast<Chord*>(cr))->arpeggio()) {
                  cr->add(arpeggio.release());
                  }
            }
      }

//---------------------------------------------------------
//   addTie
//---------------------------------------------------------

static void addTie(const Notation& notation, Note* note, const int track, MusicXMLTieMap& ties,
                   std::vector<Note*>& unstartedTieNotes, std::vector<Note*>& unendedTieNotes, MxmlLogger* logger,
                   const QXmlStreamReader* const xmlreader)
      {
      if (!note)
            return;
      const QString& type = notation.attribute("type");
      //const QString orientation = notation.attribute("orientation");
      //const QString placement = notation.attribute("placement");
      //const QString lineType = notation.attribute("line-type");

      TieLocation loc = TieLocation(note->pitch(), note->track());

      if (type.isEmpty()) {
            // ignore, nothing to do
            }
      else if (type == "start") {
            if (Tie* activeTie = ties[loc]) {
                  // Unterminated tie on this pitch. Clean this up, then resume.
                  logger->logError(QString("Tie already active"), xmlreader);
                  unendedTieNotes.push_back(activeTie->startNote());
                  ties.erase(loc);
                  }
            ties[loc] = new Tie(note->score());
            Tie* currTie = ties[loc];
            note->setTieFor(currTie);
            currTie->setStartNote(note);
            currTie->setTrack(track);

            const QColor color = notation.attribute("color");
            if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  currTie->setColor(color);

            if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)) {
                  const QString& orientation = notation.attribute("orientation");
                  const QString& placement = notation.attribute("placement");
                  if (orientation == "over" || placement == "above")
                        currTie->setSlurDirection(Direction::UP);
                  else if (orientation == "under" || placement == "below")
                        currTie->setSlurDirection(Direction::DOWN);
                  else if (orientation.isEmpty() || placement.isEmpty())
                        ; // ignore
                  else
                        logger->logError(QString("unknown tied orientation/placement: %1/%2").arg(orientation, placement), xmlreader);
                  }

            const QString& lineType = notation.attribute("line-type");
            if (lineType == "solid" || lineType.isEmpty())
                  currTie->setLineType(0);
            if (lineType == "dotted")
                  currTie->setLineType(1);
            else if (lineType == "dashed")
                  currTie->setLineType(2);

            }
      else if (type == "stop") {
            if (Tie* currTie = ties[loc]) {
                  const Note* startNote = currTie->startNote();
                  const Chord* startChord = startNote ? startNote->chord() : nullptr;
                  const Chord* endChord = note->chord();
                  const Measure* startMeasure = startChord ? startChord->measure() : nullptr;
                  qInfo() << "startMeasure: " << startMeasure;
                  qInfo() << "endMeasure: " << endChord->measure();
                  qInfo() << "startChord->tick() + startChord->ticks(): " << (startChord->tick() + startChord->ticks()).toString();
                  qInfo() << "endChord->tick(): " << endChord->tick().toString();
                  if (startMeasure == endChord->measure() || startChord->tick() + startChord->actualTicks() == endChord->tick()) {
                        // only connect if they're in the same bar, or there are no notes/rests in the same voice between them
                        qInfo() << "Connect";
                        currTie->setEndNote(note);
                        note->setTieBack(currTie);
                        }
                  else {
                        logger->logError(QString("Intervening note in voice"), xmlreader);
                        unstartedTieNotes.push_back(note);
                        unendedTieNotes.push_back(currTie->startNote());
                                 }
                  ties.erase(loc);
                  }
            else {
                  unstartedTieNotes.push_back(note);
                  logger->logError(QString("Non-started tie terminated. No-op."), xmlreader);
                  }
            }
      else if (type == "let-ring")
            addArticLaissezVibrer(note);
      else
            logger->logError(QString("unknown tied type %1").arg(type), xmlreader);
      }

//---------------------------------------------------------
//   addWavyLine
//---------------------------------------------------------

static void addWavyLine(ChordRest* cr, const Fraction& tick,
                        const int wavyLineNo, const QString& wavyLineType,
                        MusicXmlSpannerMap& spanners, TrillStack& trills,
                        MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      if (!wavyLineType.isEmpty()) {
            const Fraction ticks = cr->ticks();
            const int track = cr->track();
            const int trk = (track / VOICES) * VOICES;       // first track of staff
            Trill*& trill = trills[wavyLineNo];
            if (wavyLineType == "start" || wavyLineType == "startstop") {
                  if (trill) {
                        logger->logError(QString("overlapping wavy-line number %1").arg(wavyLineNo+1), xmlreader);
                        }
                  else {
                        trill = new Trill(cr->score());
                        trill->setTrack(trk);
                        if (wavyLineType == "start") {
                              spanners[trill] = QPair<int, int>(tick.ticks(), -1);
                              // qDebug("trill=%p inserted at first tick %d", trill, tick);
                              }
                        if (wavyLineType == "startstop") {
                              spanners[trill] = QPair<int, int>(tick.ticks(), tick.ticks() + ticks.ticks());
                              trill = nullptr;
                              // qDebug("trill=%p inserted at first tick %d second tick %d", trill, tick, tick);
                              }
                        }
                  }
            else if (wavyLineType == "stop") {
                  if (!trill) {
                        logger->logError(QString("wavy-line number %1 stop without start").arg(wavyLineNo+1), xmlreader);
                        }
                  else {
                        spanners[trill].second = tick.ticks() + ticks.ticks();
                        // qDebug("trill=%p second tick %d", trill, tick);
                        trill = nullptr;
                        }
                  }
            else
                  logger->logError(QString("unknown wavy-line type %1").arg(wavyLineType), xmlreader);
            }
      }

//---------------------------------------------------------
//   addBreath
//---------------------------------------------------------

static void addBreath(const Notation& notation, ChordRest* cr)
      {
      const SymId breath = notation.symId();
      const QColor color = notation.attribute("color");
      const QString placement = notation.attribute("placement");

      Segment* const seg = cr->measure()->getSegment(SegmentType::Breath, cr->tick() + cr->ticks());
      Breath* const b =  new Breath(seg->score());
      // b->setTrack(trk + voice); TODO check next line
      b->setTrack(cr->track());
      b->setSymId(breath);
      if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
            b->setColor(color);
      b->setPlacement(placement == "below" ? Placement::BELOW : Placement::ABOVE);
      b->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
      seg->add(b);
      }

//---------------------------------------------------------
//   addChordLine
//---------------------------------------------------------

static void addChordLine(const Notation& notation, Note* note,
                         MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      const QString& chordLineType = notation.subType();
      const QColor color = notation.attribute("color");
      if (!chordLineType.isEmpty()) {
            if (note) {
                  ChordLine* const chordline = new ChordLine(note->score());
                  if (chordLineType == "falloff")
                        chordline->setChordLineType(ChordLineType::FALL);
                  else if (chordLineType == "doit")
                        chordline->setChordLineType(ChordLineType::DOIT);
                  else if (chordLineType == "plop")
                        chordline->setChordLineType(ChordLineType::PLOP);
                  else if (chordLineType == "scoop")
                        chordline->setChordLineType(ChordLineType::SCOOP);
                  if (color.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                        chordline->setColor(color);
                  note->chord()->add(chordline);
                  }
            else
                  logger->logError(QString("no note for %1").arg(chordLineType), xmlreader);
            }
      }

//---------------------------------------------------------
//   notationWithAttributes
//---------------------------------------------------------

/**
 Helper function to create Notation with initial attributes.
 */

Notation Notation::notationWithAttributes(const QString& name, const QXmlStreamAttributes attributes,
                                          const QString& parent, const SymId& symId)
      {
      Notation notation { name, parent, symId };
      for (const auto& attr : attributes) {
            notation.addAttribute(attr.name(), attr.value());
            }
      return notation;
      }

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

void Notation::addAttribute(const QStringRef name, const QStringRef value)
      {
      _attributes.emplace(name.toString(), value.toString());
      }

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

void Notation::addAttribute(const QString& name, const QString& value)
      {
      _attributes.emplace(name, value);
      }

//---------------------------------------------------------
//   attribute
//---------------------------------------------------------

QString Notation::attribute(const QString& name) const
      {
      const auto it = _attributes.find(name);
      return (it != _attributes.end()) ? it->second : "";
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

QString Notation::print() const
      {
      QString res { _name };

      for (auto const& pair : _attributes) {
            res += " ";
            res += pair.first;
            res += " ";
            res += pair.second;
            }

      if (!_text.isEmpty()) {
            res += " ";
            res += _text;
            }
      return res;
      }

//---------------------------------------------------------
//   MusicXMLParserNotations
//---------------------------------------------------------

MusicXMLParserNotations::MusicXMLParserNotations(QXmlStreamReader& e, Score* score, MxmlLogger* logger, MusicXMLParserPass1& pass1)
      : _e(e), _pass1(pass1), _score(score), _logger(logger)
      {
      // nothing
      }

//---------------------------------------------------------
//   addError
//---------------------------------------------------------

// notes:
// when the parser gets out of sync, only a single error stack is reported
// even when e.g. two incorrect notations are present
// line number will be added by pass 2

void MusicXMLParserNotations::addError(const QString& error)
      {
      if (!error.isEmpty()) {
            _logger->logError(error, &_e);
            _errors += error;
            }
      }

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXMLParserNotations::skipLogCurrElem()
      {
      //_logger->logDebugInfo(QString("skipping '%1'").arg(_e.name().toString()), &_e);
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

void MusicXMLParserNotations::parse()
      {
      while (_e.readNextStartElement()) {
            if (_e.name() == "arpeggiate") {
                  _arpeggioType = _e.attributes().value("direction").toString();
                  if (_arpeggioType.isEmpty())
                        _arpeggioType = "none";
                  QColor color = _e.attributes().value("color").toString();
                  if (color.isValid())
                        _arpeggioColor = color;
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "articulations") {
                  articulations();
                  }
            else if (_e.name() == "dynamics") {
                  dynamics();
                  }
            else if (_e.name() == "fermata") {
                  fermata();
                  }
            else if (_e.name() == "glissando") {
                  glissandoSlide();
                  }
            else if (_e.name() == "non-arpeggiate") {
                  _arpeggioType = "non-arpeggiate";
                  _e.skipCurrentElement();  // skip but don't log
                  }
            else if (_e.name() == "ornaments") {
                  ornaments();
                  }
            else if (_e.name() == "slur") {
                  slur();
                  }
            else if (_e.name() == "slide") {
                  glissandoSlide();
                  }
            else if (_e.name() == "technical") {
                  technical();
                  }
            else if (_e.name() == "tied") {
                  tied();
                  }
            else if (_e.name() == "tuplet") {
                  tuplet();
                  }
            else if (_e.name() == "other-notation")
                  otherNotation();
            else {
                  skipLogCurrElem();
                  }
            }

      /*
      for (const Notation& notation : _notations) {
            qDebug("%s", qPrintable(notation.print()));
            }
       */

      addError(checkAtEndElement(_e, "notations"));
      }

//---------------------------------------------------------
//   addNotation
//---------------------------------------------------------

void MusicXMLParserNotations::addNotation(const Notation& notation, ChordRest* const cr, Note* const note)
      {
      if (notation.symId() != SymId::noSym) {
            if (notation.name() == "breath")
                  addBreath(notation, cr);
            else if (notation.name() == "fermata")
                  addFermataToChord(notation, cr);
            else if (notation.parent() == "ornament")
                  addTurnToChord(notation, cr);
            else
                  addArticulationToChord(notation, cr);
            }
      else if (notation.parent() == "ornaments") {
            if (notation.name() == "mordent" || notation.name() == "inverted-mordent")
                  addMordentToChord(notation, cr);
            else if (notation.name() == "other-ornament")
                  addOtherOrnamentToChord(notation, cr);
            }
      else if (notation.parent() == "articulations") {
            if (note && notation.name() == "chord-line")
                  addChordLine(notation, note, _logger, &_e);
            }
      else {
            // qDebug("addNotation: notation has been skipped: %s %s", qPrintable(notation.name()), qPrintable(notation.parent()));
            }
      }

//---------------------------------------------------------
//   addToScore
//---------------------------------------------------------

/**
 Add the notations found to the score.
 Note that some notations attach to notes only in MuseScore,
 which means trying to attach them to a rest will crash,
 as in that case note is a nullptr.
 */

void MusicXMLParserNotations::addToScore(ChordRest* const cr, Note* const note, const int tick, SlurStack& slurs,
                                         Glissando* glissandi[MAX_NUMBER_LEVEL][2], MusicXmlSpannerMap& spanners,
                                         TrillStack& trills, MusicXMLTieMap& ties, std::vector<Note*>& unstartedTieNotes,
                                         std::vector<Note*>& unendedTieNotes)
      {
      addArpeggio(cr, _arpeggioType, _arpeggioColor);
      addWavyLine(cr, Fraction::fromTicks(tick), _wavyLineNo, _wavyLineType, spanners, trills, _logger, &_e);

      for (const Notation& notation : _notations) {
            if (notation.symId() != SymId::noSym) {
                  addNotation(notation, cr, note);
                  }
            else if (notation.name() == "slur") {
                  addSlur(notation, slurs, cr, tick, _logger, &_e);
                  }
            else if (note && (notation.name() == "glissando" || notation.name() == "slide")) {
                  addGlissandoSlide(notation, note, glissandi, spanners, _logger, &_e);
                  }
            else if (note && notation.name() == "tied") {
                  addTie(notation, note, cr->track(), ties, unstartedTieNotes, unendedTieNotes, _logger, &_e);
                  }
            else if (note && notation.parent() == "technical") {
                  addTechnical(notation, note);
                  }
            else {
                  addNotation(notation, cr, note);
                  }
            }

      // more than one dynamic ???
      // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
      // TODO remove duplicate code (see MusicXml::direction)
      for (const QString& d : qAsConst(_dynamicsList)) {
            Dynamic* dynamic = new Dynamic(_score);
            dynamic->setDynamicType(d);
//TODO:ws            if (hasYoffset) dyn->textStyle().setYoff(yoffset);
            if (_dynamicsColor.isValid()/* && preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)*/)
                  dynamic->setColor(_dynamicsColor);
            addElemOffset(dynamic, cr->track(), _dynamicsPlacement, cr->measure(), Fraction::fromTicks(tick));
            }
      }

//---------------------------------------------------------
//   stem
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/stem node.
 */

void MusicXMLParserPass2::stem(Direction& sd, bool& nost)
      {
      // defaults
      sd = Direction::AUTO;
      nost = false;

      QString s = _e.readElementText();

      if (s == "up")
            sd = Direction::UP;
      else if (s == "down")
            sd = Direction::DOWN;
      else if (s == "none")
            nost = true;
      else if (s == "double")
            ;
      else
            _logger->logError(QString("unknown stem direction %1").arg(s), &_e);
      }

//---------------------------------------------------------
//   fermata
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/fermata node.
 Note: MusicXML common.mod: "An empty fermata element represents a normal fermata."
 */

void MusicXMLParserNotations::fermata()
      {
      Notation notation = Notation::notationWithAttributes(_e.name().toString(), _e.attributes(), "notations");
      const QString fermataText = _e.readElementText();

      notation.setSymId(convertFermataToSymId(fermataText));
      notation.setText(fermataText);
      _notations.push_back(notation);
      }

//---------------------------------------------------------
//   tuplet
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/tuplet node.
 */

void MusicXMLParserNotations::tuplet()
      {
      const QString tupletType       = _e.attributes().value("type").toString();
      const QString tupletPlacement  = _e.attributes().value("placement").toString();
      const QString tupletBracket    = _e.attributes().value("bracket").toString();
      const QString tupletShowNumber = _e.attributes().value("show-number").toString();

      // ignore possible children (currently not supported)
      _e.skipCurrentElement();

      if (tupletType == "start")
            _tupletDesc.type = MxmlStartStop::START;
      else if (tupletType == "stop")
            _tupletDesc.type = MxmlStartStop::STOP;
      else if (!tupletType.isEmpty() && tupletType != "start" && tupletType != "stop") {
            _logger->logError(QString("unknown tuplet type '%1'").arg(tupletType), &_e);
            }

      // set bracket, leave at default if unspecified
      if (tupletBracket == "yes")
            _tupletDesc.bracket = TupletBracketType::SHOW_BRACKET;
      else if (tupletBracket == "no")
            _tupletDesc.bracket = TupletBracketType::SHOW_NO_BRACKET;

      // set number, default is "actual" (=NumberType::SHOW_NUMBER)
      if (tupletShowNumber == "both")
            _tupletDesc.shownumber = TupletNumberType::SHOW_RELATION;
      else if (tupletShowNumber == "none")
            _tupletDesc.shownumber = TupletNumberType::NO_TEXT;
      else
            _tupletDesc.shownumber = TupletNumberType::SHOW_NUMBER;

      // set number and bracket direction
      if (tupletPlacement == "above")
            _tupletDesc.direction = Direction::UP;
      else if (tupletPlacement == "below")
            _tupletDesc.direction = Direction::DOWN;
      else if (tupletPlacement.isEmpty())
            ; // ignore
      else
            _logger->logError(QString("unknown tuplet placement: %1").arg(tupletPlacement), &_e);
      }

void MusicXMLParserNotations::otherNotation()
      {
      const QString smufl = _e.attributes().value("smufl").toString();
      if (!smufl.isEmpty()) {
            SymId id { SymId::noSym };
            if (convertArticulationToSymId(_e.name().toString(), id) && id != SymId::noSym) {
                  Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                       _e.attributes(), "notations", id);
                  _notations.push_back(notation);
                  _e.skipCurrentElement();
                  }
            }
      }

//---------------------------------------------------------
//   MusicXMLParserDirection
//---------------------------------------------------------

/**
 MusicXMLParserDirection constructor.
 */

MusicXMLParserDirection::MusicXMLParserDirection(QXmlStreamReader& e,
                                                 Score* score,
                                                 MusicXMLParserPass1& pass1,
                                                 MusicXMLParserPass2& pass2,
                                                 MxmlLogger* logger)
      : _e(e), _score(score), _pass1(pass1), _pass2(pass2), _logger(logger),
      _hasDefaultY(false), _defaultY(0.0), _hasRelativeY(false), _relativeY(0.0), _isBold(true),
      _tpoMetro(0), _tpoSound(0), _offset(0, 1)
      {
      // nothing
      }

}
