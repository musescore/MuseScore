//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2015 Werner Schweer and others
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

#include "libmscore/arpeggio.h"
#include "libmscore/accidental.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/chordline.h"
#include "libmscore/chordlist.h"
#include "libmscore/chordrest.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
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
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/mscore.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/rest.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftext.h"
#include "libmscore/sym.h"
#include "libmscore/tempotext.h"
#include "libmscore/tie.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/utils.h"
#include "libmscore/volta.h"
#include "libmscore/textline.h"
#include "libmscore/barline.h"
#include "libmscore/articulation.h"
#include "libmscore/ottava.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/fermata.h"

#include "importmxmllogger.h"
#include "importmxmlnoteduration.h"
#include "importmxmlnotepitch.h"
#include "importmxmlpass2.h"
#include "musicxmlfonthandler.h"
#include "musicxmlsupport.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

//#define DEBUG_VOICE_MAPPER true

//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

//---------------------------------------------------------
//   MusicXmlTupletDesc
//---------------------------------------------------------

MusicXmlTupletDesc::MusicXmlTupletDesc()
      : type(MxmlStartStop::NONE), placement(Placement::BELOW),
      bracket(Tuplet::BracketType::AUTO_BRACKET), shownumber(Tuplet::NumberType::SHOW_NUMBER)
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

// find the duration of the chord starting at or after s in track and ending at tick

static int lastChordTicks(const Segment* s, const int track, const int tick)
      {
      while (s && s->tick() < tick) {
            Element* el = s->element(track);
            if (el && el->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(el);
                  if (cr->tick() + cr->actualTicks() == tick)
                        return cr->actualTicks();
                  }
            s = s->nextCR(track, true);
            }
      return 0;
      }

//---------------------------------------------------------
//   setExtend
//---------------------------------------------------------

// set extend for lyric no in track to end at tick
// called when lyric (with or without "extend") or note with "extend type=stop" is found
// note that no == -1 means all lyrics in this track

void MusicXmlLyricsExtend::setExtend(const int no, const int track, const int tick)
      {
      QList<Lyrics*> list;
      foreach(Lyrics* l, _lyrics) {
            Element* const el = l->parent();
            if (el->type() == ElementType::CHORD) {       // TODO: rest also possible ?
                  ChordRest* const par = static_cast<ChordRest*>(el);
                  if (par->track() == track && (no == -1 || l->no() == no)) {
                        int lct = lastChordTicks(l->segment(), track, tick);
                        if (lct > 0) {
                              // set lyric tick to the total length fron the lyric note
                              // plus all notes covered by the melisma minus the last note length
                              l->setTicks(tick - par->tick() - lct);
                              }
                        list.append(l);
                        }
                  }
            }
      // cleanup
      foreach(Lyrics* l, list) {
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

static void xmlSetPitch(Note* n, int step, int alter, int octave, const int octaveShift, const Instrument* instr)
      {
      //qDebug("xmlSetPitch(n=%p, step=%d, alter=%d, octave=%d, octaveShift=%d)",
      //       n, step, alter, octave, octaveShift);

      //const Staff* staff = n->score()->staff(track / VOICES);
      //const Instrument* instr = staff->part()->instr();

      const Interval intval = instr->transpose();     // TODO: tick

      //qDebug("  staff=%p instr=%p dia=%d chro=%d",
      //       staff, instr, (int) intval.diatonic, (int) intval.chromatic);

      int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
      pitch += intval.chromatic; // assume not in concert pitch
      pitch += 12 * octaveShift; // correct for octave shift
      // ensure sane values
      pitch = limit(pitch, 0, 127);

      int tpc2 = step2tpc(step, AccidentalVal(alter));
      int tpc1 = Ms::transposeTpc(tpc2, intval, true);
      n->setPitch(pitch, tpc1, tpc2);
      //qDebug("  pitch=%d tpc1=%d tpc2=%d", n->pitch(), n->tpc1(), n->tpc2());
      }

//---------------------------------------------------------
//   fillGap
//---------------------------------------------------------

/**
 Fill one gap (tstart - tend) in this track in this measure with rest(s).
 */

static void fillGap(Measure* measure, int track, int tstart, int tend)
      {
      int ctick = tstart;
      int restLen = tend - tstart;
      // qDebug("\nfillGIFV     fillGap(measure %p track %d tstart %d tend %d) restLen %d len",
      //        measure, track, tstart, tend, restLen);
      // note: as MScore::division (#ticks in a quarter note) equals 480
      // MScore::division / 64 (#ticks in a 256th note) uequals 7.5 but is rounded down to 7
      while (restLen > MScore::division / 64) {
            int len = restLen;
            TDuration d(TDuration::DurationType::V_INVALID);
            if (measure->ticks() == restLen)
                  d.setType(TDuration::DurationType::V_MEASURE);
            else
                  d.setVal(len);
            Rest* rest = new Rest(measure->score(), d);
            rest->setDuration(Fraction::fromTicks(len));
            rest->setTrack(track);
            rest->setVisible(false);
            Segment* s = measure->getSegment(SegmentType::ChordRest, tstart);
            s->add(rest);
            len = rest->globalDuration().ticks();
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
      Q_ASSERT(measure);
      Q_ASSERT(part);

      int measTick     = measure->tick();
      int measLen      = measure->ticks();
      int nextMeasTick = measTick + measLen;
      int staffIdx = part->score()->staffIdx(part);
      /*
       qDebug("fillGIFV measure %p part %p idx %d nstaves %d tick %d - %d (len %d)",
       measure, part, staffIdx, part->nstaves(),
       measTick, nextMeasTick, measLen);
       */
      for (int st = 0; st < part->nstaves(); ++st) {
            int track = (staffIdx + st) * VOICES;
            int endOfLastCR = measTick;
            for (Segment* s = measure->first(); s; s = s->next()) {
                  // qDebug("fillGIFV   segment %p tp %s", s, s->subTypeName());
                  Element* el = s->element(track);
                  if (el) {
                        // qDebug(" el[%d] %p", track, el);
                        if (s->isChordRestType()) {
                              ChordRest* cr  = static_cast<ChordRest*>(el);
                              int crTick     = cr->tick();
                              int crLen      = cr->globalDuration().ticks();
                              int nextCrTick = crTick + crLen;
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
 Determine if \a mxmlDrumset contains a valid drumset.
 This is the case if any instrument has a midi-unpitched element,
 (which stored in the MusicXMLDrumInstrument pitch field).
 */

static bool hasDrumset(const MusicXMLDrumset& mxmlDrumset)
      {
      bool res = false;
      MusicXMLDrumsetIterator ii(mxmlDrumset);
      while (ii.hasNext()) {
            ii.next();
            // debug: dump the drumset
            //qDebug("hasDrumset: instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
            int pitch = ii.value().pitch;
            if (0 <= pitch && pitch <= 127) {
                  res = true;
                  }
            }

      /*
      for (const auto& instr : mxmlDrumset) {
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

// determine if the part contains a drumset
// this is the case if any instrument has a midi-unpitched element,
// (which stored in the MusicXMLDrumInstrument pitch field)
// if the part contains a drumset, Drumset drumset is initialized

static void initDrumset(Drumset* drumset, const MusicXMLDrumset& mxmlDrumset)
      {
      drumset->clear();
      MusicXMLDrumsetIterator ii(mxmlDrumset);
      while (ii.hasNext()) {
            ii.next();
            // debug: also dump the drumset for this part
            //qDebug("initDrumset: instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
            int pitch = ii.value().pitch;
            if (0 <= pitch && pitch <= 127) {
                  drumset->drum(ii.value().pitch)
                        = DrumInstrument(ii.value().name.toLatin1().constData(),
                                         ii.value().notehead, ii.value().line, ii.value().stemDirection);
                  }
            }
      }

//---------------------------------------------------------
//   createInstrument
//---------------------------------------------------------

/**
 Create an Instrument based on the information in \a mxmlInstr.
 */

static Instrument createInstrument(const MusicXMLDrumInstrument& mxmlInstr)
      {
      Instrument instr;

      InstrumentTemplate* it {};
      if (!mxmlInstr.sound.isEmpty()) {
            it = Ms::searchTemplateForMusicXmlId(mxmlInstr.sound);
            }

      /*
      qDebug("sound '%s' it %p trackname '%s' program %d",
             qPrintable(mxmlInstr.sound), it,
             it ? qPrintable(it->trackName) : "",
             mxmlInstr.midiProgram);
       */

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
            instr.channel(0)->program = mxmlInstr.midiProgram >= 0 ? mxmlInstr.midiProgram : 0;
            }

      // add / overrule with values read from MusicXML
      instr.channel(0)->pan = mxmlInstr.midiPan;
      instr.channel(0)->volume = mxmlInstr.midiVolume;
      instr.setTrackName(mxmlInstr.name);

      return instr;
      }

//---------------------------------------------------------
//   setFirstInstrument
//---------------------------------------------------------

/**
 Set first instrument for Part \a part
 */

static void setFirstInstrument(MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                               Part* part, const QString& partId,
                               const QString& instrId, const MusicXMLDrumset& mxmlDrumset)
      {
      if (mxmlDrumset.size() > 0) {
            //qDebug("setFirstInstrument: initial instrument '%s'", qPrintable(instrId));
            MusicXMLDrumInstrument mxmlInstr;
            if (instrId == "")
                  mxmlInstr = mxmlDrumset.first();
            else if (mxmlDrumset.contains(instrId))
                  mxmlInstr = mxmlDrumset.value(instrId);
            else {
                  logger->logError(QString("initial instrument '%1' not found in part '%2'")
                                   .arg(instrId).arg(partId), xmlreader);
                  mxmlInstr = mxmlDrumset.first();
                  }

            Instrument instr = createInstrument(mxmlInstr);
            part->setInstrument(instr);
            if (mxmlInstr.midiChannel >= 0) part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort);
            // note: setMidiProgram() does more than simply setting the MIDI program
            if (mxmlInstr.midiProgram >= 0) part->setMidiProgram(mxmlInstr.midiProgram);
            }
      else
            logger->logError(QString("no instrument found for part '%1'")
                             .arg(partId), xmlreader);
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
            if (part->staff(j)->lines(0) == 5 && !part->staff(j)->isDrumStaff(0))
                  part->staff(j)->setStaffType(0, StaffType::preset(StaffTypes::PERC_DEFAULT));
      // set drumset for instrument
      part->instrument()->setDrumset(drumset);
      part->instrument()->channel(0)->bank = 128;
      part->instrument()->channel(0)->updateInitList();
      }

//---------------------------------------------------------
//   findDeleteStaffText
//---------------------------------------------------------

/**
 Find a non-empty staff text in \a s at \a track (which originates as MusicXML <words>).
 If found, delete it and return its text.
 */

static QString findDeleteStaffText(Segment* s, int track)
      {
      //qDebug("findDeleteWords(s %p track %d)", s, track);
      foreach (Element* e, s->annotations()) {
            //qDebug("findDeleteWords e %p type %hhd track %d", e, e->type(), e->track());
            if (e->type() != ElementType::STAFF_TEXT || e->track() < track || e->track() >= track+VOICES)
                  continue;
            Text* t = static_cast<Text*>(e);
            //qDebug("findDeleteWords t %p text '%s'", t, qPrintable(t->text()));
            QString res = t->xmlText();
            if (res != "") {
                  s->remove(t);
                  return res;
                  }
            }
      return "";
      }

//---------------------------------------------------------
//   setPartInstruments
//---------------------------------------------------------

static void setPartInstruments(MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                               Part* part, const QString& partId,
                               Score* score, const MusicXmlInstrList& il, const MusicXMLDrumset& mxmlDrumset)
      {
      QString prevInstrId;
      for (auto it = il.cbegin(); it != il.cend(); ++it) {
            Fraction f = (*it).first;
            if (f == Fraction(0, 1))
                  prevInstrId = (*it).second;  // instrument id at t = 0
            else if (f > Fraction(0, 1)) {
                  auto instrId = (*it).second;
                  bool mustInsert = instrId != prevInstrId;
                  /*
                  qDebug("f %s previd %s id %s mustInsert %d",
                         qPrintable(f.print()),
                         qPrintable(prevInstrId),
                         qPrintable(instrId),
                         mustInsert);
                   */
                  if (mustInsert) {
                        const int staff = score->staffIdx(part);
                        const int track = staff * VOICES;
                        const int tick = f.ticks();
                        //qDebug("instrument change: tick %s (%d) track %d instr '%s'",
                        //       qPrintable(f.print()), tick, track, qPrintable(instrId));
                        auto segment = score->tick2segment(tick, true, SegmentType::ChordRest, true);
                        if (!segment)
                              logger->logError(QString("segment for instrument change at tick %1 not found")
                                               .arg(tick), xmlreader);
                        else if (!mxmlDrumset.contains(instrId))
                              logger->logError(QString("changed instrument '%1' at tick %2 not found in part '%3'")
                                               .arg(instrId).arg(tick).arg(partId), xmlreader);
                        else {
                              MusicXMLDrumInstrument mxmlInstr = mxmlDrumset.value(instrId);
                              Instrument instr = createInstrument(mxmlInstr);
                              //qDebug("instr %p", &instr);

                              InstrumentChange* ic = new InstrumentChange(instr, score);
                              ic->setTrack(track);

                              // if there is already a staff text at this tick / track,
                              // delete it and use its text here instead of "Instrument change"
                              QString text = findDeleteStaffText(segment, track);
                              ic->setXmlText(text.isEmpty() ? "Instrument change" : text);
                              segment->add(ic); // note: includes part::setInstrument(instr);

                              // setMidiChannel() depends on setInstrument() already been done
                              if (mxmlInstr.midiChannel >= 0) part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort, tick);
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
      //qDebug("text2syms map count %d maxsz %d filling time elapsed: %d ms",
      //       map.size(), maxStringSize, time.elapsed());

      // then look for matches
      QString in = t;
      QString res;

      while (in != "") {
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
                  res += in.left(1);
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
      if (!fontFamily.isEmpty() && txt == syms) {
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
            if (ok && (lines > 0))  // 1,2, or 3 underlines are imported as single underline
                  importedtext += "<u>";
            else
                  underline = "";
            }
      if (txt == syms) {
            txt.replace(QString("\r"), QString("")); // convert Windows line break \r\n -> \n
            importedtext += txt.toHtmlEscaped();
            }
      else {
            // <sym> replacement made, should be no need for line break or other conversions
            importedtext += syms;
            }
      if (underline != "")
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
            extendedLyrics.setExtend(lyricNo, cr->track(), cr->tick());
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
                      QMap<int, Lyrics*>& numbrdLyrics,
                      QSet<Lyrics*>& extLyrics,
                      MusicXmlLyricsExtend& extendedLyrics)
      {
      int lyricNo = -1;
      for (QMap<int, Lyrics*>::const_iterator i = numbrdLyrics.constBegin(); i != numbrdLyrics.constEnd(); ++i) {
            lyricNo = i.key();
            Lyrics* l = i.value();
            addLyric(logger, xmlreader, cr, l, lyricNo, extendedLyrics);
            if (extLyrics.contains(l))
                  extendedLyrics.addLyric(l);
            }
      }

//---------------------------------------------------------
//   addElemOffset
//---------------------------------------------------------

static void addElemOffset(Element* el, int track, const QString& placement, Measure* measure, int tick)
      {
      /*
       qDebug("addElem el %p track %d placement %s tick %d",
       el, track, qPrintable(placement), tick);
       */

      // move to correct position
      // TODO: handle rx, ry
      if (el->type() == ElementType::SYMBOL) {
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
            el->setPlacement(placement == "above"
                             ? Placement::ABOVE : Placement::BELOW);
            }

      el->setTrack(track);
      Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
      s->add(el);
      }

//---------------------------------------------------------
//   tupletAssert -- check assertions for tuplet handling
//---------------------------------------------------------

/**
 Check assertions for tuplet handling. If this fails, MusicXML
 import will almost certainly break in non-obvious ways.
 Should never happen, thus it is OK to quit the application.
 */

#if 0
static void tupletAssert()
      {
      if (!(int(TDuration::DurationType::V_BREVE)      == int(TDuration::DurationType::V_LONG)    + 1
            && int(TDuration::DurationType::V_WHOLE)   == int(TDuration::DurationType::V_BREVE)   + 1
            && int(TDuration::DurationType::V_HALF)    == int(TDuration::DurationType::V_WHOLE)   + 1
            && int(TDuration::DurationType::V_QUARTER) == int(TDuration::DurationType::V_HALF)    + 1
            && int(TDuration::DurationType::V_EIGHTH)  == int(TDuration::DurationType::V_QUARTER) + 1
            && int(TDuration::DurationType::V_16TH)    == int(TDuration::DurationType::V_EIGHTH)  + 1
            && int(TDuration::DurationType::V_32ND)    == int(TDuration::DurationType::V_16TH)    + 1
            && int(TDuration::DurationType::V_64TH)    == int(TDuration::DurationType::V_32ND)    + 1
            && int(TDuration::DurationType::V_128TH)   == int(TDuration::DurationType::V_64TH)    + 1
            && int(TDuration::DurationType::V_256TH)   == int(TDuration::DurationType::V_128TH)   + 1
            )) {
            qFatal("tupletAssert() failed");
            }
      }
#endif

//---------------------------------------------------------
//   smallestTypeAndCount
//---------------------------------------------------------

/**
 Determine the smallest note type and the number of those
 present in a ChordRest.
 For a note without dots the type equals the note type
 and count is one.
 For a single dotted note the type equals half the note type
 and count is three.
 A double dotted note is similar.
 Note: code assumes when duration().type() is incremented,
 the note length is divided by two, checked by tupletAssert().
 */

static void smallestTypeAndCount(ChordRest const* const cr, int& type, int& count)
      {
      type = int(cr->durationType().type());
      count = 1;
      switch (cr->durationType().dots()) {
            case 0:
                  // nothing to do
                  break;
            case 1:
                  type += 1; // next-smaller type
                  count = 3;
                  break;
            case 2:
                  type += 2; // next-next-smaller type
                  count = 7;
                  break;
            default:
                  qDebug("smallestTypeAndCount() does not support more than 2 dots");
            }
      }

//---------------------------------------------------------
//   matchTypeAndCount
//---------------------------------------------------------

/**
 Given two note types and counts, if the types are not equal,
 make them equal by successively doubling the count of the
 largest type.
 */

static void matchTypeAndCount(int& type1, int& count1, int& type2, int& count2)
      {
      while (type1 < type2) {
            type1++;
            count1 *= 2;
            }
      while (type2 < type1) {
            type2++;
            count2 *= 2;
            }
      }

//---------------------------------------------------------
//   determineTupletTypeAndCount
//---------------------------------------------------------

/**
 Determine type and number of smallest notes in the tuplet
 */

static void determineTupletTypeAndCount(Tuplet* t, int& tupletType, int& tupletCount)
      {
      int elemCount   = 0; // number of tuplet elements handled

      foreach (DurationElement* de, t->elements()) {
            if (de->type() == ElementType::CHORD || de->type() == ElementType::REST) {
                  ChordRest* cr = static_cast<ChordRest*>(de);
                  if (elemCount == 0) {
                        // first note: init variables
                        smallestTypeAndCount(cr, tupletType, tupletCount);
                        }
                  else {
                        int noteType = 0;
                        int noteCount = 0;
                        smallestTypeAndCount(cr, noteType, noteCount);
                        // match the types
                        matchTypeAndCount(tupletType, tupletCount, noteType, noteCount);
                        tupletCount += noteCount;
                        }
                  }
            elemCount++;
            }
      }

//---------------------------------------------------------
//   determineTupletBaseLen
//---------------------------------------------------------

/**
 Determine tuplet baseLen as determined by the tuplet ratio,
 and type and number of smallest notes in the tuplet.

 Example: baselen of a 3:2 tuplet with 1/16, 1/8, 1/8 and 1/16
 is 1/8. For this tuplet smallest note is 1/16, count is 6.
 */

// TODO: this is defined twice, remove one

static TDuration determineTupletBaseLen(Tuplet* t)
      {
      int tupletType  = 0; // smallest note type in the tuplet
      int tupletCount = 0; // number of smallest notes in the tuplet

      // first determine type and number of smallest notes in the tuplet
      determineTupletTypeAndCount(t, tupletType, tupletCount);

      // sanity check:
      // for a 3:2 tuplet, count must be a multiple of 3
      if (tupletCount % t->ratio().numerator()) {
            qDebug("determineTupletBaseLen(%p) cannot divide count %d by %d", t, tupletCount, t->ratio().numerator());
            return TDuration();
            }

      // calculate baselen in smallest notes
      tupletCount /= t->ratio().numerator();

      // normalize
      while (tupletCount > 1 && (tupletCount % 2) == 0) {
            tupletCount /= 2;
            tupletType  -= 1;
            }

      return TDuration(TDuration::DurationType(tupletType));
      }

//---------------------------------------------------------
//   isTupletFilled
//---------------------------------------------------------

/**
 Determine if the tuplet contains the required number of notes,
 either (1) of the specified normal type
 or (2) the amount of the smallest notes in the tuplet equals
 actual notes.

 Example (1): a 3:2 tuplet with a 1/4 and a 1/8 note is filled
 if normal type is 1/8, it is not filled if normal
 type is 1/4.

 Example (2): a 3:2 tuplet with a 1/4 and a 1/8 note is filled.

 Use note types instead of duration to prevent errors due to rounding.
 */

// TODO: this is defined twice, remove one

static bool isTupletFilled(Tuplet* t, TDuration normalType)
      {
      if (!t) return false;

      int tupletType  = 0; // smallest note type in the tuplet
      int tupletCount = 0; // number of smallest notes in the tuplet

      // first determine type and number of smallest notes in the tuplet
      determineTupletTypeAndCount(t, tupletType, tupletCount);

      // then compare ...
      if (normalType.isValid()) {
            int matchedNormalType  = int(normalType.type());
            int matchedNormalCount = t->ratio().numerator();
            // match the types
            matchTypeAndCount(tupletType, tupletCount, matchedNormalType, matchedNormalCount);
            // ... result scenario (1)
            return tupletCount >= matchedNormalCount;
            }
      else {
            // ... result scenario (2)
            return tupletCount >= t->ratio().numerator();
            }
      }

//---------------------------------------------------------
//   addTupletToChord
//---------------------------------------------------------

/**
 Handle tuplet(s) using parse result tupletDesc
 Tuplets with <actual-notes> and <normal-notes> but without <tuplet>
 are handled correctly.
 TODO Nested tuplets are not (yet) supported.

 Note that cr must be initialized: fields measure, score, tick
 and track are used.
 */

void addTupletToChord(ChordRest* cr, Tuplet*& tuplet, bool& tuplImpl,
                      const Fraction& timeMod, const MusicXmlTupletDesc& tupletDesc,
                      const TDuration normalType)
      {
      int actualNotes = timeMod.denominator();
      int normalNotes = timeMod.numerator();

      // check for obvious errors
      if (tupletDesc.type == MxmlStartStop::START && tuplet) {
            qDebug("tuplet already started"); // TODO
            // TODO: how to recover ?
            }
      if (tupletDesc.type == MxmlStartStop::STOP && !tuplet) {
            qDebug("tuplet stop but no tuplet started"); // TODO
            // TODO: how to recover ?
            }

      // Tuplet are either started by the tuplet start
      // or when the time modification is first found.
      if (!tuplet) {
            if (tupletDesc.type == MxmlStartStop::START
                || (!tuplet && (actualNotes != 1 || normalNotes != 1))) {
                  if (tupletDesc.type != MxmlStartStop::START) {
                        tuplImpl = true;
                        // report missing start
                        qDebug("implicit tuplet start cr %p tick %d track %d", cr, cr->tick(), cr->track()); // TODO
                        }
                  else
                        tuplImpl = false;
                  // create a new tuplet
                  tuplet = new Tuplet(cr->score());
                  tuplet->setTrack(cr->track());
                  tuplet->setRatio(Fraction(actualNotes, normalNotes));
                  tuplet->setTick(cr->tick());
                  tuplet->setBracketType(tupletDesc.bracket);
                  tuplet->setNumberType(tupletDesc.shownumber);
                  // TODO type, placement, bracket
                  tuplet->setParent(cr->measure());
                  }
            }

      // Add chord to the current tuplet.
      // Must also check for actual/normal notes to prevent
      // adding one chord too much if tuplet stop is missing.
      if (tuplet && !(actualNotes == 1 && normalNotes == 1)) {
            cr->setTuplet(tuplet);
            tuplet->add(cr);
            }

      // Tuplets are stopped by the tuplet stop
      // or when the tuplet is filled completely
      // (either with knowledge of the normal type
      // or as a last resort calculated based on
      // actual and normal notes plus total duration)
      // or when the time-modification is not found.
      if (tuplet) {
            if (tupletDesc.type == MxmlStartStop::STOP
                || (tuplImpl && isTupletFilled(tuplet, normalType))
                || (actualNotes == 1 && normalNotes == 1)) {
                  // set baselen
                  TDuration td = determineTupletBaseLen(tuplet);
                  // qDebug("stop tuplet %p basetype %d", tuplet, tupletType);
                  tuplet->setBaseLen(td);
                  Fraction f(normalNotes, td.fraction().denominator());
                  f.reduce();
                  tuplet->setDuration(f);
                  // TODO determine usefulness of following check
                  int totalDuration = 0;
                  foreach (DurationElement* de, tuplet->elements()) {
                        if (de->type() == ElementType::CHORD || de->type() == ElementType::REST) {
                              totalDuration+=de->globalDuration().ticks();
                              }
                        }
                  if (!(totalDuration && normalNotes)) {
                        qDebug("MusicXML::import: tuplet stop but bad duration"); // TODO
                        }
                  tuplet = 0;
                  }
            }
      }

//---------------------------------------------------------
//   addArticulationToChord
//---------------------------------------------------------

static void addArticulationToChord(ChordRest* cr, SymId articSym, QString dir)
      {
      Articulation* na = new Articulation(articSym, cr->score());
      if (dir == "up") {
            na->setUp(true);
            na->setAnchor(ArticulationAnchor::TOP_STAFF);
            }
      else if (dir == "down") {
            na->setUp(false);
            na->setAnchor(ArticulationAnchor::BOTTOM_STAFF);
            }
      cr->add(na);
      }

//---------------------------------------------------------
//   addFermataToChord
//---------------------------------------------------------

static void addFermataToChord(ChordRest* cr, SymId articSym, bool up)
      {
      Fermata* na = new Fermata(articSym, cr->score());
      na->setTrack(cr->track());
      na->setPlacement(up ? Placement::ABOVE : Placement::BELOW);
      cr->segment()->add(na);
      }

//---------------------------------------------------------
//   addMordentToChord
//---------------------------------------------------------

/**
 Add Mordent to Chord.
 */

static void addMordentToChord(ChordRest* cr, QString name, QString attrLong, QString attrAppr, QString attrDep)
      {
      SymId articSym = SymId::noSym; // legal but impossible ArticulationType value here indicating "not found"
      if (name == "inverted-mordent") {
            if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "")
                  articSym = SymId::ornamentMordent;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "")
                  articSym = SymId::ornamentTremblement;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep == "")
                  articSym = SymId::ornamentUpPrall;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep == "")
                  articSym = SymId::ornamentPrecompMordentUpperPrefix;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "below")
                  articSym = SymId::ornamentPrallDown;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "above")
                  articSym = SymId::ornamentPrallUp;
            }
      else if (name == "mordent") {
            if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "")
                  articSym = SymId::ornamentMordentInverted;
            else if (attrLong == "yes" && attrAppr == "" && attrDep == "")
                  articSym = SymId::ornamentPrallMordent;
            else if (attrLong == "yes" && attrAppr == "below" && attrDep == "")
                  articSym = SymId::ornamentUpMordent;
            else if (attrLong == "yes" && attrAppr == "above" && attrDep == "")
                  articSym = SymId::ornamentDownMordent;
            }
      if (articSym != SymId::noSym) {
            Articulation* na = new Articulation(cr->score());
            na->setSymId(articSym);
            cr->add(na);
            }
      else
            qDebug("unknown ornament: name '%s' long '%s' approach '%s' departure '%s'",
                   qPrintable(name), qPrintable(attrLong), qPrintable(attrAppr), qPrintable(attrDep));  // TODO
      }

//---------------------------------------------------------
//   addMxmlArticulationToChord
//---------------------------------------------------------

/**
 Add a MusicXML articulation to a chord as a "simple" MuseScore articulation.
 These are the articulations that can be
 - represented by an enum ArticulationType
 - added to a ChordRest
 Return true (articulation recognized and handled)
 or false (articulation not recognized).
 Note simple implementation: MusicXML syntax is not strictly
 checked, the articulations parent element does not matter.
 */

static bool addMxmlArticulationToChord(ChordRest* cr, QString mxmlName)
      {
      QMap<QString, SymId> map; // map MusicXML articulation name to MuseScore symbol
      map["accent"]           = SymId::articAccentAbove;
      map["staccatissimo"]    = SymId::articStaccatissimoAbove;
      map["staccato"]         = SymId::articStaccatoAbove;
      map["tenuto"]           = SymId::articTenutoAbove;
      map["turn"]             = SymId::ornamentTurn;
      map["inverted-turn"]    = SymId::ornamentTurnInverted;
      map["stopped"]          = SymId::brassMuteClosed;
      // TODO map["harmonic"]         = SymId::stringsHarmonic;
      map["up-bow"]           = SymId::stringsUpBow;
      map["down-bow"]         = SymId::stringsDownBow;
      map["detached-legato"]  = SymId::articTenutoStaccatoAbove;
      map["spiccato"]         = SymId::articStaccatissimoAbove;
      map["snap-pizzicato"]   = SymId::pluckedSnapPizzicatoAbove;
      map["schleifer"]        = SymId::ornamentPrecompSlide;
      map["open-string"]      = SymId::brassMuteOpen;
      map["thumb-position"]   = SymId::stringsThumbPosition;

      if (map.contains(mxmlName)) {
            addArticulationToChord(cr, map.value(mxmlName), "");
            return true;
            }
      else
            return false;
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
      map["circle-x"] = int(NoteHead::Group::HEAD_XCIRCLE);
      map["inverted triangle"] = int(NoteHead::Group::HEAD_TRIANGLE_DOWN);
      map["slashed"] = int(NoteHead::Group::HEAD_SLASHED1);
      map["back slashed"] = int(NoteHead::Group::HEAD_SLASHED2);
      map["normal"] = int(NoteHead::Group::HEAD_NORMAL);
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

static void addTextToNote(int l, int c, QString txt, SubStyle style, Score* score, Note* note)
      {
      if (note) {
            if (!txt.isEmpty()) {
                  TextBase* t = new Fingering(score);
                  t->initSubStyle(style);
                  t->setPlainText(txt);
                  note->add(t);
                  }
            }
      else
            qDebug("%s", qPrintable(QString("Error at line %1 col %2: no note for text").arg(l).arg(c)));       // TODO
      }

//---------------------------------------------------------
//   addFermata
//---------------------------------------------------------

/**
 Add a MusicXML fermata.
 Note: MusicXML common.mod: "The fermata type is upright if not specified."
 */

static void addFermata(ChordRest* cr, const QString type, const SymId articSym)
      {
      if (type == "upright" || type == "")
            addFermataToChord(cr, articSym, true);
      else if (type == "inverted")
            addFermataToChord(cr, articSym, false);
      else
            qDebug("unknown fermata type '%s'", qPrintable(type));
      }

//---------------------------------------------------------
//   setSLinePlacement
//---------------------------------------------------------

/**
 Helper for direction().
 SLine placement is modified by changing the first segments user offset
 As the SLine has just been created, it does not have any segment yet
 */

static void setSLinePlacement(SLine* sli, const QString placement)
      {
      /*
       qDebug("setSLinePlacement sli %p type %d s=%g pl='%s'",
       sli, sli->type(), sli->score()->spatium(), qPrintable(placement));
       */

      // calc y offset assuming five line staff and default style
      // note that required y offset is element type dependent
      if (sli->type() == ElementType::HAIRPIN) {
            if (placement == "above") {
                  const qreal stafflines = 5;       // assume five line staff, but works OK-ish for other sizes too
                  qreal offsAbove = -6 - (stafflines - 1);
                  qreal y = 0;
                  y +=  offsAbove;
                  // add linesegment containing the user offset
                  LineSegment* tls= sli->createLineSegment();
                  //qDebug("   y = %g", y);
                  tls->setAutoplace(false);
                  y *= sli->score()->spatium();
                  tls->setUserOff(QPointF(0, y));
                  sli->add(tls);
                  }
            }
      else {
            sli->setPlacement(placement == "above"
                              ? Placement::ABOVE : Placement::BELOW);
            }
      }

//---------------------------------------------------------
//   handleSpannerStart
//---------------------------------------------------------

// note that in case of overlapping spanners, handleSpannerStart is called for every spanner
// as spanners QMap allows only one value per key, this does not hurt at all

static void handleSpannerStart(SLine* new_sp, int track, QString& placement, int tick, MusicXmlSpannerMap& spanners)
      {
      //qDebug("handleSpannerStart(sp %p, track %d, tick %d)", new_sp, track, tick);
      new_sp->setTrack(track);
      setSLinePlacement(new_sp, placement);
      spanners[new_sp] = QPair<int, int>(tick, -1);
      }

//---------------------------------------------------------
//   handleSpannerStop
//---------------------------------------------------------

static void handleSpannerStop(SLine* cur_sp, int track2, int tick, MusicXmlSpannerMap& spanners)
      {
      //qDebug("handleSpannerStop(sp %p, track2 %d, tick %d)", cur_sp, track2, tick);
      if (!cur_sp)
            return;

      cur_sp->setTrack2(track2);
      spanners[cur_sp].second = tick;
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
      _timeSigDura = Fraction(0, 0);             // invalid
      int nstaves = _pass1.getPart(partId)->nstaves();
      _tuplets.resize(nstaves * VOICES);
      _tuplImpls.resize(nstaves * VOICES);
      _tie    = 0;
      _lastVolta = 0;
      _hasDrumset = false;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _slurs[i] = SlurDesc();
      for (int i = 0; i < MAX_BRACKETS; ++i)
            _brackets[i] = 0;
      for (int i = 0; i < MAX_DASHES; ++i)
            _dashes[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _ottavas[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _hairpins[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _trills[i] = 0;
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            _glissandi[i][0] = _glissandi[i][1] = 0;
      _pedal = 0;
      _pedalContinue = 0;
      _harmony = 0;
      _tremStart = 0;
      _figBass = 0;
      //      glissandoText = "";
      //      glissandoColor = "";
      _multiMeasureRestCount = -1;
      _extendedLyrics.init();
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
//   scorePartwise
//---------------------------------------------------------

/**
 Parse the MusicXML top-level (XPath /score-partwise) node.
 */

void MusicXMLParserPass2::scorePartwise()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "score-partwise");

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
      // TODO, handle other tracks?
      if (_score->lastMeasure()->endBarLineType() == BarLineType::NORMAL)
            _score->lastMeasure()->setEndBarLineType(BarLineType::NORMAL, 0);
      }

//---------------------------------------------------------
//   partList
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list node.
 */

void MusicXMLParserPass2::partList()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "part-list");

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
      Q_ASSERT(_e.isStartElement() && _e.name() == "score-part");

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
      Q_ASSERT(_e.isStartElement() && _e.name() == "part");
      const QString id = _e.attributes().value("id").toString();

      if (!_pass1.hasPart(id)) {
            _logger->logError(QString("MusicXMLParserPass2::part cannot find part '%1'").arg(id), &_e);
            skipLogCurrElem();
            }

      initPartState(id);

      const MusicXMLDrumset& mxmlDrumset = _pass1.getDrumset(id);
      _hasDrumset = hasDrumset(mxmlDrumset);

      // set the parts first instrument
      QString instrId = _pass1.getInstrList(id).instrument(Fraction(0, 1));
      setFirstInstrument(_logger, &_e, _pass1.getPart(id), id, instrId, mxmlDrumset);

      // set the part name
      auto mxmlPart = _pass1.getMusicXmlPart(id);
      _pass1.getPart(id)->setPartName(mxmlPart.getName());
      if (mxmlPart.getPrintName())
            _pass1.getPart(id)->setLongName(mxmlPart.getName());
      if (mxmlPart.getPrintAbbr())
            _pass1.getPart(id)->setPlainShortName(mxmlPart.getAbbr());
      // try to prevent an empty track name
      if (_pass1.getPart(id)->partName() == "")
            _pass1.getPart(id)->setPartName(mxmlDrumset[instrId].name);

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
      int nr = 0; // current measure sequence number
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

      // stop all remaining extends for this part
      Measure* lm = _pass1.getPart(id)->score()->lastMeasure();
      if (lm) {
            int strack = _pass1.trackForPart(id);
            int etrack = strack + _pass1.getPart(id)->nstaves() * VOICES;
            int lastTick = lm->tick() + lm->ticks();
            for (int trk = strack; trk < etrack; trk++)
                  _extendedLyrics.setExtend(-1, trk, lastTick);
            }

      //qDebug("spanner list:");
      auto i = _spanners.constBegin();
      while (i != _spanners.constEnd()) {
            Spanner* sp = i.key();
            int tick1 = i.value().first;
            int tick2 = i.value().second;
            //qDebug("spanner %p tp %hhd tick1 %d tick2 %d track %d track2 %d",
            //       sp, sp->type(), tick1, tick2, sp->track(), sp->track2());
            sp->setTick(tick1);
            sp->setTick2(tick2);
            sp->score()->addElement(sp);
            ++i;
            }
      _spanners.clear();

      // determine if the part contains a drumset
      // this is the case if any instrument has a midi-unpitched element,
      // (which stored in the MusicXMLDrumInstrument pitch field)
      // if the part contains a drumset, Drumset drumset is initialized

      Drumset* drumset = new Drumset;
      const MusicXMLDrumset& mxmlDrumsetAfterPass2 = _pass1.getDrumset(id);
      initDrumset(drumset, mxmlDrumsetAfterPass2);

      // debug: dump the instrument map
      /*
            {
            qDebug("instrlist");
            auto il = _pass1.getInstrList(id);
            for (auto it = il.cbegin(); it != il.cend(); ++it) {
                  Fraction f = (*it).first;
                  qDebug("pass2: instrument map: tick %s (%d) instr '%s'", qPrintable(f.print()), f.ticks(), qPrintable((*it).second));
                  }
            }
      */

      if (_hasDrumset) {
            // set staff type to percussion if incorrectly imported as pitched staff
            // Note: part has been read, staff type already set based on clef type and staff-details
            // but may be incorrect for a percussion staff that does not use a percussion clef
            setStaffTypePercussion(_pass1.getPart(id), drumset);
            }
      else {
            // drumset is not needed
            delete drumset;
            // set the instruments for this part
            setPartInstruments(_logger, &_e, _pass1.getPart(id), id, _score, _pass1.getInstrList(id), mxmlDrumset);
            }
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

/**
 In Score \a score find the measure starting at \a tick.
 */

static Measure* findMeasure(Score* score, const int tick)
      {
      for (Measure* m = score->firstMeasure();; m = m->nextMeasure()) {
            if (m && m->tick() == tick)
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
      beam = 0;
      }

//---------------------------------------------------------
//   handleBeamAndStemDir
//---------------------------------------------------------

static void handleBeamAndStemDir(ChordRest* cr, const Beam::Mode bm, const Direction sd, Beam*& beam)
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
            }
      // add ChordRest to beam
      if (beam) {
            // verify still in the same track (switching voices in the middle of a beam is not supported)
            // and in a beam ...
            // (note no check is done on correct order of beam begin/continue/end)
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
            else if (!(bm == Beam::Mode::BEGIN || bm == Beam::Mode::MID || bm == Beam::Mode::END)) {
                  qDebug("handleBeamAndStemDir() in beam, bm %d -> abort beam", static_cast<int>(bm));
                  // reset beam mode for all elements and remove the beam
                  removeBeam(beam);
                  }
            else {
                  // actually add cr to the beam
                  beam->add(cr);
                  }
            }
      // if no beam, set stem direction on chord itself and set beam to auto
      if (!beam) {
            static_cast<Chord*>(cr)->setStemDirection(sd);
            cr->setBeamMode(Beam::Mode::AUTO);
            }
      // terminate the currect beam and add to the score
      if (beam && bm == Beam::Mode::END)
            beam = 0;
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
                                const QMap<Note*, int>& alterMap
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
                  foreach (Note* nt, chord->notes()) {
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
      for (int i = gcl.size() - 1; i >= 0; i--)
            c->add(gcl.at(i));        // TODO check if same voice ?
      gcl.clear();
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
      Q_ASSERT(_e.isStartElement() && _e.name() == "measure");
      QString number = _e.attributes().value("number").toString();
      //qDebug("measure %s start", qPrintable(number));

      Measure* measure = findMeasure(_score, time.ticks());
      if (!measure) {
            _logger->logError(QString("measure at tick %1 not found!").arg(time.ticks()), &_e);
            skipLogCurrElem();
            }

      // handle implicit measure
      if (_e.attributes().value("implicit") == "yes")
            measure->setIrregular(true);

      // set measure's RepeatFlag to none because musicXML is allowing single measure repeat and no ordering in repeat start and end barlines
      measure->setRepeatStart(false);
      measure->setRepeatEnd(false);

      Fraction mTime; // current time stamp within measure
      Fraction prevTime; // time stamp within measure previous chord
      Chord* prevChord = 0;       // previous chord
      Fraction mDura; // current total measure duration
      GraceChordList gcl; // grace chords collected sofar
      int gac = 0;       // grace after count in the grace chord list
      Beam* beam = 0;       // current beam
      QString cv = "1";       // current voice for chords, default is 1
      FiguredBassList fbl;               // List of figured bass elements under a single note

      // collect candidates for courtesy accidentals to work out at measure end
      QMap<Note*, int> alterMap;

      while (_e.readNextStartElement()) {
            if (_e.name() == "attributes")
                  attributes(partId, measure, (time + mTime).ticks());
            else if (_e.name() == "direction") {
                  MusicXMLParserDirection dir(_e, _score, _pass1, *this, _logger);
                  dir.direction(partId, measure, (time + mTime).ticks(), _spanners);
                  }
            else if (_e.name() == "figured-bass") {
                  FiguredBass* fb = figuredBass();
                  if (fb)
                        fbl.append(fb);
                  }
            else if (_e.name() == "harmony")
                  harmony(partId, measure, time + mTime);
            else if (_e.name() == "note") {
                  Fraction dura;
                  int alt = -10;                    // any number outside range of xml-tag "alter"
                  // note: chord and grace note handling done in note()
                  // dura > 0 iff valid rest or first note of chord found
                  Note* n = note(partId, measure, time + mTime, time + prevTime, dura, cv, gcl, gac, beam, fbl, alt);
                  if (n && !n->chord()->isGrace())
                        prevChord = n->chord();  // remember last non-grace chord
                  if (n && n->accidental() && n->accidental()->accidentalType() != AccidentalType::NONE)
                        alterMap.insert(n, alt);
                  if (dura.isValid() && dura > Fraction(0, 1)) {
                        prevTime = mTime; // save time stamp last chord created
                        mTime += dura;
                        if (mTime > mDura)
                              mDura = mTime;
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
                        }
                  }
            else if (_e.name() == "sound") {
                  QString tempo = _e.attributes().value("tempo").toString();

                  if (!tempo.isEmpty()) {
                        double tpo = tempo.toDouble() / 60;
                        int tick = (time + mTime).ticks();

                        TempoText* t = new TempoText(_score);
                        t->setXmlText(QString("%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(TDuration::DurationType::V_QUARTER))).arg(tempo));
                        t->setTempo(tpo);
                        t->setFollowText(true);

                        _score->setTempo(tick, tpo);

                        addElemOffset(t, _pass1.trackForPart(partId), "above", measure, tick);
                        }
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "barline")
                  barline(partId, measure);
            else if (_e.name() == "print")
                  print(measure);
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

      // fill possible gaps in voice 1
      Part* part = _pass1.getPart(partId); // should not fail, we only get here if the part exists
      fillGapsInFirstVoices(measure, part);

      // can't have beams extending into the next measure
      if (beam)
            removeBeam(beam);

      // TODO:
      // - how to handle _timeSigDura.isZero (shouldn't happen ?)
      // - how to handle unmetered music
      if (_timeSigDura.isValid() && !_timeSigDura.isZero())
            measure->setTimesig(_timeSigDura);

      // mark superfluous accidentals as user accidentals
      const int scoreRelStaff = _score->staffIdx(part);
      const Key key = _score->staff(scoreRelStaff)->keySigEvent(time.ticks()).key();
      markUserAccidentals(scoreRelStaff, part->nstaves(), key, measure, alterMap);

      // multi-measure rest handling
      if (getAndDecMultiMeasureRestCount() == 0) {
            // measure is first measure after a multi-measure rest
            measure->setBreakMultiMeasureRest(true);
            }

      Q_ASSERT(_e.isEndElement() && _e.name() == "measure");
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

void MusicXMLParserPass2::attributes(const QString& partId, Measure* measure, const int tick)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "attributes");

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
                  staffDetails(partId);
            else if (_e.name() == "time")
                  time(partId, measure, tick);
            else if (_e.name() == "transpose")
                  transpose(partId);
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
      score->staff(staffIdx)->setLines(0, stafflines);
      score->staff(staffIdx)->setBarLineTo(0);        // default
      }

//---------------------------------------------------------
//   staffDetails
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details node.
 */

void MusicXMLParserPass2::staffDetails(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "staff-details");
      //logDebugTrace("MusicXMLParserPass2::staffDetails");

      Part* part = _pass1.getPart(partId);
      Q_ASSERT(part);
      int staves = part->nstaves();

      QString number = _e.attributes().value("number").toString();
      int n = 1;  // default
      if (number != "") {
            n = number.toInt();
            if (n <= 0 || n > staves) {
                  _logger->logError(QString("invalid staff-details number %1").arg(number), &_e);
                  n = 1;
                  }
            }
      n--;         // make zero-based

      int staffIdx = _score->staffIdx(part) + n;

      StringData* t = 0;
      if (_score->staff(staffIdx)->isTabStaff(0)) {
            t = new StringData;
            t->setFrets(25);  // sensible default
            }

      int staffLines = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "staff-lines") {
                  // save staff lines for later
                  staffLines = _e.readElementText().toInt();
                  // for a TAB staff also resize the string table and init with zeroes
                  if (t) {
                        if (0 < staffLines)
                              t->stringList() = QVector<instrString>(staffLines).toList();
                        else
                              _logger->logError(QString("illegal staff-lines %1").arg(staffLines), &_e);
                        }
                  }
            else if (_e.name() == "staff-tuning")
                  staffTuning(t);
            else
                  skipLogCurrElem();
            }

      if (staffLines > 0) {
            setStaffLines(_score, staffIdx, staffLines);
            }

      if (t) {
            Instrument* i = part->instrument();
            i->setStringData(*t);
            }
      }
//---------------------------------------------------------
//   staffTuning
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details/staff-tuning node.
 */

void MusicXMLParserPass2::staffTuning(StringData* t)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "staff-tuning");
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
                  alter = _e.readElementText().toInt();
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
 Initializes the "in multi-measure rest" state
 */

void MusicXMLParserPass2::measureStyle(Measure* measure)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "measure-style");

      while (_e.readNextStartElement()) {
            if (_e.name() == "multiple-rest") {
                  int multipleRest = _e.readElementText().toInt();
                  if (multipleRest > 1) {
                        _multiMeasureRestCount = multipleRest;
                        _score->style().set(StyleIdx::createMultiMeasureRests, true);
                        measure->setBreakMultiMeasureRest(true);
                        }
                  else
                        _logger->logError(QString("multiple-rest %1 not supported").arg(multipleRest), &_e);
                  }
            else
                  skipLogCurrElem();
            }
      }


//---------------------------------------------------------
//   print
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/print node.
 */

void MusicXMLParserPass2::print(Measure* measure)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "print");

      bool newSystem = _e.attributes().value("new-system") == "yes";
      bool newPage   = _e.attributes().value("new-page") == "yes";
      int blankPage = _e.attributes().value("blank-page").toInt();
      //
      // in MScore the break happens _after_ the marked measure:
      //
      MeasureBase* pm = measure->prevMeasure();        // We insert VBox only for title, no HBox for the moment
      if (pm == 0) {
            _logger->logDebugInfo("break on first measure", &_e);
            if (blankPage == 1) {       // blank title page, insert a VBOX if needed
                  pm = measure->prev();
                  if (pm == 0) {
                        _score->insertMeasure(ElementType::VBOX, measure);
                        pm = measure->prev();
                        }
                  }
            }
      if (pm) {
            if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTBREAKS) && (newSystem || newPage)) {
                  if (!pm->lineBreak() && !pm->pageBreak()) {
                        LayoutBreak* lb = new LayoutBreak(_score);
                        lb->setLayoutBreakType(newSystem ? LayoutBreak::Type::LINE : LayoutBreak::Type::PAGE);
                        pm->add(lb);
                        }
                  }
            }

      while (_e.readNextStartElement()) {
            skipLogCurrElem();
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
                                        const int tick,
                                        MusicXmlSpannerMap& spanners)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "direction");
      //qDebug("direction tick %d", tick);

      QString placement = _e.attributes().value("placement").toString();
      int track = _pass1.trackForPart(partId);
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
            else if (_e.name() == "staff") {
                  int nstaves = _pass1.getPart(partId)->nstaves();
                  QString strStaff = _e.readElementText();
                  int staff = strStaff.toInt();
                  if (0 < staff && staff <= nstaves)
                        track += (staff - 1) * VOICES;
                  else
                        _logger->logError(QString("invalid staff %1").arg(strStaff), &_e);
                  }
            else if (_e.name() == "sound")
                  sound();
            else
                  skipLogCurrElem();
            }

      handleRepeats(measure, track);

      // fix for Sibelius 7.1.3 (direct export) which creates metronomes without <sound tempo="..."/>:
      // if necessary, use the value calculated by metronome()
      // note: no floating point comparisons with 0 ...
      if (_tpoSound < 0.1 && _tpoMetro > 0.1)
            _tpoSound = _tpoMetro;

      //qDebug("words '%s' rehearsal '%s' metro '%s' tpo %g",
      //       qPrintable(_wordsText), qPrintable(_rehearsalText), qPrintable(_metroText), _tpoSound);

      // create text if any text was found

      if (_wordsText != "" || _rehearsalText != "" || _metroText != "") {
            TextBase* t = 0;
            if (_tpoSound > 0.1) {
                  _tpoSound /= 60;
                  t = new TempoText(_score);
                  t->setXmlText(_wordsText + _metroText);
                  ((TempoText*) t)->setTempo(_tpoSound);
                  ((TempoText*) t)->setFollowText(true);
                  _score->setTempo(tick, _tpoSound);
                  }
            else {
                  if (_wordsText != "" || _metroText != "") {
                        t = new StaffText(_score);
                        t->setXmlText(_wordsText + _metroText);
                        }
                  else {
                        t = new RehearsalMark(_score);
                        if (!_rehearsalText.contains("<b>"))
                              _rehearsalText = "<b></b>" + _rehearsalText;  // explicitly turn bold off
                        t->setXmlText(_rehearsalText);
                        if (!_hasDefaultY)
                              t->setPlacement(Placement::ABOVE);  // crude way to force placement TODO improve ?
                        }
                  }

            if (_enclosure == "circle") {
                  t->setHasFrame(true);
                  t->setCircle(true);
                  }
            else if (_enclosure == "none") {
                  t->setHasFrame(false);
                  }
            else if (_enclosure == "rectangle") {
                  t->setHasFrame(true);
                  t->setFrameRound(0);
                  }

//TODO:ws            if (_hasDefaultY) t->textStyle().setYoff(_defaultY);
            addElemOffset(t, track, placement, measure, tick);
            }
      else if (_tpoSound > 0) {
            double tpo = _tpoSound / 60;
            TempoText* t = new TempoText(_score);
            t->setXmlText(QString("%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(TDuration::DurationType::V_QUARTER))).arg(_tpoSound));
            t->setTempo(tpo);
            t->setFollowText(true);

            _score->setTempo(tick, tpo);

            addElemOffset(t, track, placement, measure, tick);
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
                  dyn->setVelocity( dynaValue );
                  }
//TODO:ws            if (_hasDefaultY) dyn->textStyle().setYoff(_defaultY);
            addElemOffset(dyn, track, placement, measure, tick);
            }

      // handle the elems
      foreach( auto elem, _elems) {
            // TODO (?) if (_hasDefaultY) elem->setYoff(_defaultY);
            addElemOffset(elem, track, placement, measure, tick);
            }

      // handle the spanner stops first
      foreach (auto desc, stops) {
            SLine* sp = _pass2.getSpanner(desc);
            if (sp) {
                  handleSpannerStop(sp, track, tick, spanners);
                  _pass2.clearSpanner(desc);
                  }
            else
                  _logger->logError("spanner stop without spanner start", &_e);
            }

      // then handle the spanner starts
      foreach (auto desc, starts) {
            SLine* sp = _pass2.getSpanner(desc);
            if (!sp) {
                  _pass2.addSpanner(desc);
                  handleSpannerStart(desc.sp, track, placement, tick, spanners);
                  }
            else
                  _logger->logError("spanner already started", &_e);
            }

      Q_ASSERT(_e.isEndElement() && _e.name() == "direction");
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
      Q_ASSERT(_e.isStartElement() && _e.name() == "direction-type");

      while (_e.readNextStartElement()) {
            _defaultY = _e.attributes().value("default-y").toDouble(&_hasDefaultY) * -0.1;
            QString number = _e.attributes().value("number").toString();
            int n = 0;
            if (number != "") {
                  n = number.toInt();
                  if (n <= 0)
                        _logger->logError(QString("invalid number %1").arg(number), &_e);
                  else
                        n--;  // make zero-based
                  }
            QString type = _e.attributes().value("type").toString();
            if  (_e.name() == "metronome")
                  _metroText = metronome(_tpoMetro);
            else if (_e.name() == "words") {
                  _enclosure      = _e.attributes().value("enclosure").toString();
                  _wordsText += nextPartOfFormattedString(_e);
                  }
            else if (_e.name() == "rehearsal") {
                  _enclosure      = _e.attributes().value("enclosure").toString();
                  if (_enclosure == "")
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
                  _coda = true;
                  _e.skipCurrentElement();
                  }
            else if (_e.name() == "segno") {
                  _segno = true;
                  _e.skipCurrentElement();
                  }
            else
                  skipLogCurrElem();
            }

      Q_ASSERT(_e.isEndElement() && _e.name() == "direction-type");
      }

//---------------------------------------------------------
//   sound
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/sound node.
 */

void MusicXMLParserDirection::sound()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "sound");

      _sndCapo = _e.attributes().value("capo").toString();
      _sndCoda = _e.attributes().value("coda").toString();
      _sndDacapo = _e.attributes().value("dacapo").toString();
      _sndDalsegno = _e.attributes().value("dalsegno").toString();
      _sndFine = _e.attributes().value("fine").toString();
      _sndSegno = _e.attributes().value("segno").toString();
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
      Q_ASSERT(_e.isStartElement() && _e.name() == "dynamics");

      while (_e.readNextStartElement()) {
            if (_e.name() == "other-dynamics")
                  _dynamicsList.push_back(_e.readElementText());
            else {
                  _dynamicsList.push_back(_e.name().toString());
                  _e.skipCurrentElement();
                  }
            }
      }

//---------------------------------------------------------
//   matchRepeat
//---------------------------------------------------------

/**
 Do a wild-card match with known repeat texts.
 */

static QString matchRepeat(const QString& lowerTxt)
      {
      QString repeat;
      QRegExp daCapo("d\\.? *c\\.?|da *capo");
      QRegExp daCapoAlFine("d\\.? *c\\.? *al *fine|da *capo *al *fine");
      QRegExp daCapoAlCoda("d\\.? *c\\.? *al *coda|da *capo *al *coda");
      QRegExp dalSegno("d\\.? *s\\.?|d[ae]l *segno");
      QRegExp dalSegnoAlFine("d\\.? *s\\.? *al *fine|d[ae]l *segno *al *fine");
      QRegExp dalSegnoAlCoda("d\\.? *s\\.? *al *coda|d[ae]l *segno *al *coda");
      QRegExp fine("fine");
      QRegExp toCoda("to *coda");
      if (daCapo.exactMatch(lowerTxt)) repeat = "daCapo";
      if (daCapoAlFine.exactMatch(lowerTxt)) repeat = "daCapoAlFine";
      if (daCapoAlCoda.exactMatch(lowerTxt)) repeat = "daCapoAlCoda";
      if (dalSegno.exactMatch(lowerTxt)) repeat = "dalSegno";
      if (dalSegnoAlFine.exactMatch(lowerTxt)) repeat = "dalSegnoAlFine";
      if (dalSegnoAlCoda.exactMatch(lowerTxt)) repeat = "dalSegnoAlCoda";
      if (fine.exactMatch(lowerTxt)) repeat = "fine";
      if (toCoda.exactMatch(lowerTxt)) repeat = "toCoda";
      return repeat;
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
            m = new Marker(score);
            m->initSubStyle(SubStyle::REPEAT_RIGHT);
            m->setMarkerType(Marker::Type::FINE);
            }
      else if (repeat == "toCoda") {
            m = new Marker(score);
            m->initSubStyle(SubStyle::REPEAT_RIGHT);
            m->setMarkerType(Marker::Type::TOCODA);
            }
      return m;
      }

//---------------------------------------------------------
//   handleRepeats
//---------------------------------------------------------

void MusicXMLParserDirection::handleRepeats(Measure* measure, const int track)
      {
      // Try to recognize the various repeats
      QString repeat = "";
      // Easy cases first
      if (_coda) repeat = "coda";
      if (_segno) repeat = "segno";
      // As sound may be missing, next do a wild-card match with known repeat texts
      QString txt = MScoreTextToMXML::toPlainText(_wordsText.toLower());
      if (repeat == "") repeat = matchRepeat(txt.toLower());
      // If that did not work, try to recognize a sound attribute
      if (repeat == "" && _sndCoda != "") repeat = "coda";
      if (repeat == "" && _sndDacapo != "") repeat = "daCapo";
      if (repeat == "" && _sndDalsegno != "") repeat = "dalSegno";
      if (repeat == "" && _sndFine != "") repeat = "fine";
      if (repeat == "" && _sndSegno != "") repeat = "segno";
      // If a repeat was found, assume words is no longer needed
      if (repeat != "") _wordsText = "";

      /*
       qDebug(" txt=%s repeat=%s",
       qPrintable(txt),
       qPrintable(repeat)
       );
       */

      if (repeat != "") {
            if (Jump* jp = findJump(repeat, _score)) {
                  jp->setTrack(track);
                  qDebug("jumpsMarkers adding jm %p meas %p",jp, measure);
                  // TODO jumpsMarkers.append(JumpMarkerDesc(jp, measure));
                  measure->add(jp);
                  }
            if (Marker* m = findMarker(repeat, _score)) {
                  m->setTrack(track);
                  qDebug("jumpsMarkers adding jm %p meas %p",m, measure);
                  // TODO jumpsMarkers.append(JumpMarkerDesc(m, measure));
                  measure->add(m);
                  }
            }
      }

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/bracket node.
 */

void MusicXMLParserDirection::bracket(const QString& type, const int number,
                                      QList<MusicXmlSpannerDesc>& starts, QList<MusicXmlSpannerDesc>& stops)
      {
      QStringRef lineEnd = _e.attributes().value("line-end");
      QStringRef lineType = _e.attributes().value("line-type");
      if (type == "start") {
            TextLine* b = new TextLine(_score);
            // if (placement == "") placement = "above";  // TODO ? set default

            b->setBeginHookType(lineEnd != "none" ? HookType::HOOK_90 : HookType::NONE);
            if (lineEnd == "up")
                  b->setBeginHookHeight(-1 * b->beginHookHeight());

            // hack: combine with a previous words element
            if (!_wordsText.isEmpty()) {
                  // TextLine supports only limited formatting, remove all (compatible with 1.3)
                  b->setBeginText(MScoreTextToMXML::toPlainText(_wordsText));
                  _wordsText = "";
                  }

            if (lineType == "solid")
                  b->setLineStyle(Qt::SolidLine);
            else if (lineType == "dashed")
                  b->setLineStyle(Qt::DashLine);
            else if (lineType == "dotted")
                  b->setLineStyle(Qt::DotLine);
            else
                  _logger->logError(QString("unsupported line-type: %1").arg(lineType.toString()), &_e);
            starts.append(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
            }
      else if (type == "stop") {
            TextLine* b = static_cast<TextLine*>(_pass2.getSpanner(MusicXmlSpannerDesc(ElementType::TEXTLINE, number)));
            if (b) {
                  b->setEndHookType(lineEnd != "none" ? HookType::HOOK_90 : HookType::NONE);
                  if (lineEnd == "up")
                        b->setEndHookHeight(-1 * b->endHookHeight());
                  }
            stops.append(MusicXmlSpannerDesc(ElementType::TEXTLINE, number));
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
      if (type == "start") {
            TextLine* b = new TextLine(_score);
            // if (placement == "") placement = "above";  // TODO ? set default

            // hack: combine with a previous words element
            if (!_wordsText.isEmpty()) {
                  // TextLine supports only limited formatting, remove all (compatible with 1.3)
                  b->setBeginText(MScoreTextToMXML::toPlainText(_wordsText));
                  _wordsText = "";
                  }

            b->setBeginHookType(HookType::NONE);
            b->setEndHookType(HookType::NONE);
            b->setLineStyle(Qt::DashLine);
            // TODO brackets and dashes now share the same storage
            // because they both use ElementType::TEXTLINE
            // use mxml specific type instead
            starts.append(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
            }
      else if (type == "stop")
            stops.append(MusicXmlSpannerDesc(ElementType::TEXTLINE, number));
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
      if (type == "up" || type == "down") {
            int ottavasize = _e.attributes().value("size").toInt();
            if (!(ottavasize == 8 || ottavasize == 15)) {
                  _logger->logError(QString("unknown octave-shift size %1").arg(ottavasize), &_e);
                  }
            else {
                  Ottava* o = new Ottava(_score);

                  // if (placement == "") placement = "above";  // TODO ? set default

                  if (type == "down" && ottavasize ==  8) o->setOttavaType(OttavaType::OTTAVA_8VA);
                  if (type == "down" && ottavasize == 15) o->setOttavaType(OttavaType::OTTAVA_15MA);
                  if (type ==   "up" && ottavasize ==  8) o->setOttavaType(OttavaType::OTTAVA_8VB);
                  if (type ==   "up" && ottavasize == 15) o->setOttavaType(OttavaType::OTTAVA_15MB);

                  starts.append(MusicXmlSpannerDesc(o, ElementType::OTTAVA, number));
                  }
            }
      else if (type == "stop")
            stops.append(MusicXmlSpannerDesc(ElementType::OTTAVA, number));
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
      QStringRef line = _e.attributes().value("line");
      QString sign = _e.attributes().value("sign").toString();
      if (line != "yes" && sign == "") sign = "yes";       // MusicXML 2.0 compatibility
      if (line == "yes" && sign == "") sign = "no";        // MusicXML 2.0 compatibility
      if (line == "yes") {
            if (type == "start") {
                  Pedal* p = new Pedal(_score);
                  if (sign == "yes")
                        p->setBeginText("<sym>keyboardPedalPed</sym>");
                  else
                        p->setBeginHookType(HookType::HOOK_90);
                  p->setEndHookType(HookType::HOOK_90);
                  // if (placement == "") placement = "below";  // TODO ? set default
                  starts.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, 0));
                  }
            else if (type == "stop")
                  stops.append(MusicXmlSpannerDesc(ElementType::PEDAL, 0));
            else if (type == "change") {
#if 0
                  TODO
                  // pedal change is implemented as two separate pedals
                  // first stop the first one
                  if (pedal) {
                        pedal->setEndHookType(HookType::HOOK_45);
                        handleSpannerStop(pedal, "pedal", track, tick, spanners);
                        pedalContinue = pedal; // mark for later fixup
                        pedal = 0;
                        }
                  // then start a new one
                  pedal = static_cast<Pedal*>(checkSpannerOverlap(pedal, new Pedal(score), "pedal"));
                  pedal->setBeginHookType(HookType::HOOK_45);
                  pedal->setEndHookType(HookType::HOOK_90);
                  if (placement == "") placement = "below";
                  handleSpannerStart(pedal, "pedal", track, placement, tick, spanners);
#endif
                  }
            else if (type == "continue") {
                  // ignore
                  }
            else
                  qDebug("unknown pedal type %s", qPrintable(type));
            }
      else {
            // TBD: what happens when an unknown pedal type is found ?
            Symbol* s = new Symbol(_score);
            s->setAlign(Align::LEFT | Align::BASELINE);
            s->setOffsetType(OffsetType::SPATIUM);
            if (type == "start")
                  s->setSym(SymId::keyboardPedalPed);
            else if (type == "stop")
                  s->setSym(SymId::keyboardPedalUp);
            else
                  _logger->logError(QString("unknown pedal type %1").arg(type), &_e);
            _elems.append(s);
            }

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
      if (type == "crescendo" || type == "diminuendo") {
            Hairpin* h = new Hairpin(_score);
            h->setHairpinType(type == "crescendo"
                              ? HairpinType::CRESC_HAIRPIN : HairpinType::DECRESC_HAIRPIN);
            if (niente == "yes")
                  h->setHairpinCircledTip(true);
            starts.append(MusicXmlSpannerDesc(h, ElementType::HAIRPIN, number));
            }
      else if (type == "stop") {
            Hairpin* h = static_cast<Hairpin*>(_pass2.getSpanner(MusicXmlSpannerDesc(ElementType::HAIRPIN, number)));
            if (niente == "yes")
                  h->setHairpinCircledTip(true);
            stops.append(MusicXmlSpannerDesc(ElementType::HAIRPIN, number));
            }
      _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void MusicXMLParserPass2::addSpanner(const MusicXmlSpannerDesc& d)
      {
      if (d.tp == ElementType::HAIRPIN && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL)
            _hairpins[d.nr] = d.sp;
      else if (d.tp == ElementType::OTTAVA && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL)
            _ottavas[d.nr] = d.sp;
      else if (d.tp == ElementType::PEDAL && 0 == d.nr)
            _pedal = d.sp;
      // TODO: check MAX_BRACKETS vs MAX_NUMBER_LEVEL
      else if (d.tp == ElementType::TEXTLINE && 0 <= d.nr && d.nr < MAX_BRACKETS)
            _brackets[d.nr] = d.sp;
      }

//---------------------------------------------------------
//   getSpanner
//---------------------------------------------------------

SLine* MusicXMLParserPass2::getSpanner(const MusicXmlSpannerDesc& d)
      {
      if (d.tp == ElementType::HAIRPIN && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL)
            return _hairpins[d.nr];
      else if (d.tp == ElementType::OTTAVA && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL)
            return _ottavas[d.nr];
      else if (d.tp == ElementType::PEDAL && 0 == d.nr)
            return _pedal;
      // TODO: check MAX_BRACKETS vs MAX_NUMBER_LEVEL
      else if (d.tp == ElementType::TEXTLINE && 0 <= d.nr && d.nr < MAX_BRACKETS)
            return _brackets[d.nr];
      return 0;
      }

//---------------------------------------------------------
//   clearSpanner
//---------------------------------------------------------

void MusicXMLParserPass2::clearSpanner(const MusicXmlSpannerDesc& d)
      {
      if (d.tp == ElementType::HAIRPIN && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL)
            _hairpins[d.nr] = 0;
      else if (d.tp == ElementType::OTTAVA && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL)
            _ottavas[d.nr] = 0;
      else if (d.tp == ElementType::PEDAL && 0 == d.nr)
            _pedal = 0;
      // TODO: check MAX_BRACKETS vs MAX_NUMBER_LEVEL
      else if (d.tp == ElementType::TEXTLINE && 0 <= d.nr && d.nr < MAX_BRACKETS)
            _brackets[d.nr] = 0;
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
      Q_ASSERT(_e.isStartElement() && _e.name() == "metronome");

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
      else if (perMinute != "") {
            tempoText += " = ";
            tempoText += perMinute;
            }
      if (dur1.isValid() && !dur2.isValid() && perMinute != "") {
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
       else if (barStyle == "heavy-heavy")
       ;
       */
      else if (barStyle == "none") {
            type = BarLineType::NORMAL;
            visible = false;
            }
      else if (barStyle == "") {
            if (repeat == "backward")
                  type = BarLineType::END_REPEAT;
            else if (repeat == "forward")
                  type = BarLineType::START_REPEAT;
            else {
                  qDebug("empty bar type");       // TODO
                  return false;
                  }
            }
      else if (barStyle == "tick") {
            }
      else if (barStyle == "short") {
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

void MusicXMLParserPass2::barline(const QString& partId, Measure* measure)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "barline");

      QString loc = _e.attributes().value("location").toString();
      if (loc == "")
            loc = "right";
      QString barStyle;
      QString endingNumber;
      QString endingType;
      QString endingText;
      QString repeat;
      QString count;

      while (_e.readNextStartElement()) {
            if (_e.name() == "bar-style")
                  barStyle = _e.readElementText();
            else if (_e.name() == "ending") {
                  endingNumber = _e.attributes().value("number").toString();
                  endingType   = _e.attributes().value("type").toString();
                  endingText = _e.readElementText();
                  }
            else if (_e.name() == "repeat") {
                  repeat = _e.attributes().value("direction").toString();
                  count = _e.attributes().value("times").toString();
                  if (count.isEmpty()) {
                        count = "2";
                        }
                  measure->setRepeatCount(count.toInt());
                  _e.skipCurrentElement();
                  }
            else
                  skipLogCurrElem();
            }

      BarLineType type = BarLineType::NORMAL;
      bool visible = true;
      if (determineBarLineType(barStyle, repeat, type, visible)) {
            if (type == BarLineType::START_REPEAT) {
                  // combine start_repeat flag with current state initialized during measure parsing
                  measure->setRepeatStart(true);
                  }
            else if (type == BarLineType::END_REPEAT) {
                  // combine end_repeat flag with current state initialized during measure parsing
                  measure->setRepeatEnd(true);
                  }
            else {
                  int track = _pass1.trackForPart(partId);
                  if (barStyle == "tick") {
                        BarLine* b = new BarLine(measure->score());
                        int track = _pass1.trackForPart(partId);
                        b->setTrack(track);
                        b->setBarLineType(BarLineType::NORMAL);
                        b->setSpanStaff(false);
                        b->setSpanFrom(BARLINE_SPAN_TICK1_FROM);
                        b->setSpanTo(BARLINE_SPAN_TICK1_TO);
                        Segment* segment = measure->getSegment(SegmentType::EndBarLine, measure->endTick());
                        segment->add(b);
                        }
                  else if (barStyle == "short") {
                        BarLine* b = new BarLine(measure->score());
                        int track = _pass1.trackForPart(partId);
                        b->setTrack(track);
                        b->setBarLineType(BarLineType::NORMAL);
                        b->setSpanStaff(0);
                        b->setSpanFrom(BARLINE_SPAN_SHORT1_FROM);
                        b->setSpanTo(BARLINE_SPAN_SHORT1_TO);
                        Segment* segment = measure->getSegment(SegmentType::EndBarLine, measure->endTick());
                        segment->add(b);
                        }
                  else if (loc == "right")
                        measure->setEndBarLineType(type, track, visible);
                  else if (measure->prevMeasure())
                        measure->prevMeasure()->setEndBarLineType(type, track, visible);
                  }
            }

      doEnding(partId, measure, endingNumber, endingType, endingText);
      }

//---------------------------------------------------------
//   doEnding
//---------------------------------------------------------

void MusicXMLParserPass2::doEnding(const QString& partId, Measure* measure,
                                   const QString& number, const QString& type, const QString& text)
      {
      if (!(number.isEmpty() && type.isEmpty())) {
            if (number.isEmpty())
                  _logger->logError("empty ending number", &_e);
            else if (type.isEmpty())
                  _logger->logError("empty ending type", &_e);
            else {
                  QStringList sl = number.split(",", QString::SkipEmptyParts);
                  QList<int> iEndingNumbers;
                  bool unsupported = false;
                  foreach(const QString &s, sl) {
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
                        if (type == "start") {
                              Volta* volta = new Volta(_score);
                              volta->setTrack(_pass1.trackForPart(partId));
                              volta->setText(text.isEmpty() ? number : text);
                              // LVIFIX TODO also support endings "1 - 3"
                              volta->endings().clear();
                              volta->endings().append(iEndingNumbers);
                              volta->setTick(measure->tick());
                              _score->addElement(volta);
                              _lastVolta = volta;
                              }
                        else if (type == "stop") {
                              if (_lastVolta) {
                                    _lastVolta->setVoltaType(Volta::Type::CLOSED);
                                    _lastVolta->setTick2(measure->tick() + measure->ticks());
                                    _lastVolta = 0;
                                    }
                              else
                                    _logger->logError("ending stop without start", &_e);
                              }
                        else if (type == "discontinue") {
                              if (_lastVolta) {
                                    _lastVolta->setVoltaType(Volta::Type::OPEN);
                                    _lastVolta->setTick2(measure->tick() + measure->ticks());
                                    _lastVolta = 0;
                                    }
                              else
                                    _logger->logError("ending discontinue without start", &_e);
                              }
                        else
                              _logger->logError(QString("unsupported ending type '%1'").arg(type), &_e);
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

static void addSymToSig(KeySigEvent& sig, const QString& step, const QString& alter, const QString& accid)
      {
      //qDebug("addSymToSig(step '%s' alt '%s' acc '%s')",
      //       qPrintable(step), qPrintable(alter), qPrintable(accid));

      SymId id = mxmlString2accSymId(accid);
      if (id == SymId::noSym) {
            bool ok;
            double d;
            d = alter.toDouble(&ok);
            AccidentalType accTpAlter = ok ? microtonalGuess(d) : AccidentalType::NONE;
            id = mxmlString2accSymId(accidentalType2MxmlString(accTpAlter));
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

static void addKey(const KeySigEvent key, const bool printObj, Score* score, Measure* measure, const int staffIdx, const int tick)
      {
      Key oldkey = score->staff(staffIdx)->key(tick);
      // TODO only if different custom key ?
      if (oldkey != key.key() || key.custom() || key.isAtonal()) {
            // new key differs from key in effect at this tick
            KeySig* keysig = new KeySig(score);
            keysig->setTrack((staffIdx) * VOICES);
            keysig->setKeySigEvent(key);
            keysig->setVisible(printObj);
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

static void flushAlteredTone(KeySigEvent& kse, QString& step, QString& alt, QString& acc)
      {
      //qDebug("flushAlteredTone(step '%s' alt '%s' acc '%s')",
      //       qPrintable(step), qPrintable(alt), qPrintable(acc));

      if (step == "" && alt == "" && acc == "")
            return;  // nothing to do

      // step and alt are required, but also accept step and acc
      if (step != "" && (alt != "" || acc != "")) {
            addSymToSig(kse, step, alt, acc);
            }
      else {
            qDebug("flushAlteredTone invalid combination of step '%s' alt '%s' acc '%s')",
                   qPrintable(step), qPrintable(alt), qPrintable(acc)); // TODO
            }

      // clean up
      step = "";
      alt  = "";
      acc  = "";
      }

//---------------------------------------------------------
//   key
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/key node.
 */

// TODO: check currKeySig handling

void MusicXMLParserPass2::key(const QString& partId, Measure* measure, const int tick)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "key");

      QString strKeyno = _e.attributes().value("number").toString();
      int keyno = -1; // assume no number (see below)
      if (strKeyno != "") {
            keyno = strKeyno.toInt();
            if (keyno == 0) {
                  // conversion error (0), assume staff 1
                  _logger->logError(QString("invalid key number '%1'").arg(strKeyno), &_e);
                  keyno = 1;
                  }
            // convert to 0-based
            keyno--;
            }
      bool printObject = _e.attributes().value("print-object") != "no";

      // for custom keys, a single altered tone is described by
      // key-step (required),  key-alter (required) and key-accidental (optional)
      // none, one or more altered tone may be present
      // a simple state machine is required to detect them
      KeySigEvent key;
      QString keyStep;
      QString keyAlter;
      QString keyAccidental;

      while (_e.readNextStartElement()) {
            if (_e.name() == "fifths")
                  key.setKey(Key(_e.readElementText().toInt()));
            else if (_e.name() == "mode") {
                  QString m = _e.readElementText();
                  if (m == "none") {
                        key.setCustom(true);
                        key.setMode(KeyMode::NONE);
                        }
                  else if (m == "major") {
                        key.setMode(KeyMode::MAJOR);
                        }
                  else if (m == "minor") {
                        key.setMode(KeyMode::MINOR);
                        }
                  else {
                        _logger->logError(QString("Unsupported mode '%1'").arg(m), &_e);
                        }
                  }
            else if (_e.name() == "cancel")
                  skipLogCurrElem();  // TODO ??
            else if (_e.name() == "key-step") {
                  flushAlteredTone(key, keyStep, keyAlter, keyAccidental);
                  keyStep = _e.readElementText();
                  }
            else if (_e.name() == "key-alter")
                  keyAlter = _e.readElementText();
            else if (_e.name() == "key-accidental")
                  keyAccidental = _e.readElementText();
            else
                  skipLogCurrElem();
            }
      flushAlteredTone(key, keyStep, keyAlter, keyAccidental);

      int nstaves = _pass1.getPart(partId)->nstaves();
      int staffIdx = _pass1.trackForPart(partId) / VOICES;
      if (keyno == -1) {
            // apply key to all staves in the part
            for (int i = 0; i < nstaves; ++i) {
                  addKey(key, printObject, _score, measure, staffIdx + i, tick);
                  }
            }
      else if (keyno < nstaves)
            addKey(key, printObject, _score, measure, staffIdx + keyno, tick);
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 */

void MusicXMLParserPass2::clef(const QString& partId, Measure* measure, const int tick)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "clef");

      Part* part = _pass1.getPart(partId);
      Q_ASSERT(part);

      // TODO: check error handling for
      // - single staff
      // - multi-staff with same clef
      QString strClefno = _e.attributes().value("number").toString();
      int clefno = 1; // default
      if (strClefno != "")
            clefno = strClefno.toInt();
      if (clefno <= 0 || clefno > part->nstaves()) {
            // conversion error (0) or other issue, assume staff 1
            // Also for Cubase 6.5.5 which generates clef number="2" in a single staff part
            // Same fix is required in pass 1 and pass 2
            _logger->logError(QString("invalid clef number '%1'").arg(strClefno), &_e);
            clefno = 1;
            }
      // convert to 0-based
      clefno--;

      ClefType clef   = ClefType::G;
      StaffTypes st = StaffTypes::STANDARD;

      QString c;
      int i = 0;
      int line = -1;

      while (_e.readNextStartElement()) {
            if (_e.name() == "sign")
                  c = _e.readElementText();
            else if (_e.name() == "line")
                  line = _e.readElementText().toInt();
            else if (_e.name() == "clef-octave-change") {
                  i = _e.readElementText().toInt();
                  if (i && !(c == "F" || c == "G"))
                        qDebug("clef-octave-change only implemented for F and G key");  // TODO
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
            else if (line == 4)
                  clef = ClefType::C4;
            else if (line == 3)
                  clef = ClefType::C3;
            else if (line == 2)
                  clef = ClefType::C2;
            else if (line == 1)
                  clef = ClefType::C1;
            }
      else if (c == "percussion") {
            clef = ClefType::PERC;
            st = StaffTypes::PERC_DEFAULT;
            }
      else if (c == "TAB") {
            clef = ClefType::TAB;
            st= StaffTypes::TAB_DEFAULT;
            }
      else
            qDebug("clef: unknown clef <sign=%s line=%d oct ch=%d>", qPrintable(c), line, i);  // TODO

      Clef* clefs = new Clef(_score);
      clefs->setClefType(clef);
      int track = _pass1.trackForPart(partId) + clefno * VOICES;
      clefs->setTrack(track);
      Segment* s = measure->getSegment(tick ? SegmentType::Clef : SegmentType::HeaderClef, tick);
      s->add(clefs);

      // set the correct staff type
      // note that this overwrites the staff lines value set in pass 1
      // also note that clef handling should probably done in pass1
      int staffIdx = _score->staffIdx(part) + clefno;
      int lines = _score->staff(staffIdx)->lines(0);
      if (st == StaffTypes::TAB_DEFAULT || (_hasDrumset && st == StaffTypes::PERC_DEFAULT)) {
            _score->staff(staffIdx)->setStaffType(0, StaffType::preset(st));
            _score->staff(staffIdx)->setLines(0, lines); // preserve previously set staff lines
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
      if (beats == "2" && beatType == "2" && timeSymbol == "cut") {
            st = TimeSigType::ALLA_BREVE;
            bts = 2;
            btp = 2;
            return true;
            }
      else if (beats == "4" && beatType == "4" && timeSymbol == "common") {
            st = TimeSigType::FOUR_FOUR;
            bts = 4;
            btp = 4;
            return true;
            }
      else {
            if (!timeSymbol.isEmpty() && timeSymbol != "normal") {
                  qDebug("determineTimeSig: time symbol <%s> not recognized with beats=%s and beat-type=%s",
                         qPrintable(timeSymbol), qPrintable(beats), qPrintable(beatType)); // TODO
                  return false;
                  }

            btp = beatType.toInt();
            QStringList list = beats.split("+");
            for (int i = 0; i < list.size(); i++)
                  bts += list.at(i).toInt();
            }

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

void MusicXMLParserPass2::time(const QString& partId, Measure* measure, const int tick)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "time");

      QString beats;
      QString beatType;
      QString timeSymbol = _e.attributes().value("symbol").toString();
      bool printObject = _e.attributes().value("print-object") != "no";

      while (_e.readNextStartElement()) {
            if (_e.name() == "beats")
                  beats = _e.readElementText();
            else if (_e.name() == "beat-type")
                  beatType = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      if (beats != "" && beatType != "") {
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
                        int track = _pass1.trackForPart(partId) + i * VOICES;
                        timesig->setTrack(track);
                        timesig->setSig(fractionTSig, st);
                        // handle simple compound time signature
                        if (beats.contains(QChar('+'))) {
                              timesig->setNumeratorString(beats);
                              timesig->setDenominatorString(beatType);
                              }
                        Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
                        s->add(timesig);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/transpose node.
 */

void MusicXMLParserPass2::transpose(const QString& partId)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "transpose");

      Interval interval;
      bool diatonic = false;
      bool chromatic = false;
      while (_e.readNextStartElement()) {
            int i = _e.readElementText().toInt();
            if (_e.name() == "diatonic") {
                  interval.diatonic = i;
                  diatonic = true;
                  }
            else if (_e.name() == "chromatic") {
                  interval.chromatic = i;
                  chromatic = true;
                  }
            else if (_e.name() == "octave-change") {
                  interval.diatonic += i * 7;
                  interval.chromatic += i * 12;
                  }
            else
                  skipLogCurrElem();
            }

      if (chromatic && !diatonic)
            interval.diatonic += chromatic2diatonic(interval.chromatic);

      _pass1.getPart(partId)->instrument()->setTranspose(interval);
      }

//---------------------------------------------------------
//   divisions
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/divisions node.
 */

void MusicXMLParserPass2::divisions()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "divisions");

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

static bool isWholeMeasureRest(const bool rest, const QString& type, const Fraction dura, const Fraction mDura)
      {
      if (!rest)
            return false;

      if (!dura.isValid())
            return false;

      if (!mDura.isValid())
            return false;

      return ((type == "" && dura == mDura)
              || (type == "whole" && dura == mDura && dura != Fraction(1, 1)));
      }

//---------------------------------------------------------
//   determineDuration
//---------------------------------------------------------

/**
 * Determine duration for a note or rest.
 * This includes whole measure rest detection.
 */

static TDuration determineDuration(const bool rest, const QString& type, const int dots, const Fraction dura, const Fraction mDura)
      {
      //qDebug("determineDuration rest %d type '%s' dots %d dura %s mDura %s",
      //       rest, qPrintable(type), dots, qPrintable(dura.print()), qPrintable(mDura.print()));

      TDuration res;
      if (rest) {
            if (isWholeMeasureRest(rest, type, dura, mDura))
                  res.setType(TDuration::DurationType::V_MEASURE);
            else if (type == "") {
                  // If no type, set duration type based on duration.
                  // Note that sometimes unusual duration (e.g. 261/256) are found.
                  res.setVal(dura.ticks());
                  }
            else {
                  res.setType(type);
                  res.setDots(dots);
                  }
            }
      else {
            res.setType(type);
            res.setDots(dots);
            if (res.type() == TDuration::DurationType::V_INVALID)
                  res.setType(TDuration::DurationType::V_QUARTER);  // default, TODO: use dura ?
            }

      //qDebug("-> dur %hhd (%s) dots %d ticks %s",
      //       res.type(), qPrintable(res.name()), res.dots(), qPrintable(dura.print()));

      return res;
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
            cr->setDuration(dura);
            }
      else {
            cr->setDurationType(duration);
            cr->setDuration(cr->durationType().fraction());
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
                     const int tick, const int track, const int move,
                     const TDuration duration, const Fraction dura)
      {
      Segment* s = m->getSegment(SegmentType::ChordRest, tick);
      // Sibelius might export two rests at the same place, ignore the 2nd one
      // <?DoletSibelius Two NoteRests in same voice at same position may be an error?>
      if (s->element(track)) {
            qDebug("cannot add rest at tick %d track %d: element already present", tick, track);       // TODO
            return 0;
            }

      Rest* cr = new Rest(score);
      setChordRestDuration(cr, duration, dura);
      cr->setTrack(track);
      cr->setStaffMove(move);
      s->add(cr);
      return cr;
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
                                const int tick, const int track, const int move,
                                const TDuration duration, const Fraction dura,
                                Beam::Mode bm)
      {
      //qDebug("findOrCreateChord tick %d track %d dur ticks %d ticks %s bm %hhd",
      //       tick, track, duration.ticks(), qPrintable(dura.print()), bm);
      Chord* c = m->findChord(tick, track);
      if (c == 0) {
            c = new Chord(score);
            // better not to force beam end, as the beam palette does not support it
            if (bm == Beam::Mode::END)
                  c->setBeamMode(Beam::Mode::AUTO);
            else
                  c->setBeamMode(bm);
            c->setTrack(track);

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
      if (slash)
            nt = NoteType::ACCIACCATURA;
      if (duration.type() == TDuration::DurationType::V_QUARTER) {
            nt = NoteType::GRACE4;
            }
      else if (duration.type() == TDuration::DurationType::V_16TH) {
            nt = NoteType::GRACE16;
            }
      else if (duration.type() == TDuration::DurationType::V_32ND) {
            nt = NoteType::GRACE32;
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
                               const TDuration duration, const bool slash)
      {
      Chord* c = new Chord(score);
      c->setNoteType(graceNoteType(duration, slash));
      c->setTrack(track);
      // note grace notes have no durations, use default fraction 0/1
      setChordRestDuration(c, duration, Fraction());
      return c;
      }

//---------------------------------------------------------
//   elementMustBePostponed
//---------------------------------------------------------

/**
 Check if handling the current element must be postponed
 until after allocating the note.
 */

static bool elementMustBePostponed(const QXmlStreamReader& e)
      {
      return e.name() == "notations"
             || e.name() == "lyric"
             || e.name() == "play";
      }

//---------------------------------------------------------
//   handleDisplayStep
//---------------------------------------------------------

/**
 * convert display-step and display-octave to staff line
 */

static void handleDisplayStep(ChordRest* cr, int step, int octave, int tick, qreal spatium)
      {
      if (0 <= step && step <= 6 && 0 <= octave && octave <= 9) {
            //qDebug("rest step=%d oct=%d", step, octave);
            ClefType clef = cr->staff()->clef(tick);
            int po = ClefInfo::pitchOffset(clef);
            //qDebug(" clef=%hhd po=%d step=%d", clef, po, step);
            int dp = 7 * (octave + 2) + step;
            //qDebug(" dp=%d po-dp=%d", dp, po-dp);
            cr->setUserYoffset((po - dp + 3) * spatium / 2);
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
      const auto score = note->score();

      if (noteheadColor != QColor::Invalid)
            note->setColor(noteheadColor);
      if (noteheadParentheses) {
            auto s = new Symbol(score);
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
//   addFiguredBassElemens
//---------------------------------------------------------

/**
 Add the figured bass elements.
 */

static void addFiguredBassElemens(FiguredBassList& fbl, const Fraction noteStartTime, const int msTrack,
                                  const Fraction dura, Measure* measure)
      {
      if (!fbl.isEmpty()) {
            auto sTick = noteStartTime.ticks();              // starting tick
            foreach (FiguredBass* fb, fbl) {
                  fb->setTrack(msTrack);
                  // No duration tag defaults ticks() to 0; set to note value
                  if (fb->ticks() == 0)
                        fb->setTicks(dura.ticks());
                  // TODO: set correct onNote value
                  Segment* s = measure->getSegment(SegmentType::ChordRest, sTick);
                  s->add(fb);
                  sTick += fb->ticks();
                  }
            fbl.clear();
            }
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
                                Fraction& dura,
                                QString& currentVoice,
                                GraceChordList& gcl,
                                int& gac,
                                Beam*& currBeam,
                                FiguredBassList& fbl,
                                int& alt
                                )
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "note");

      if (_e.attributes().value("print-spacing") == "no") {
            notePrintSpacingNo(dura);
            return 0;
            }

      bool chord = false;
      bool cue = false;
      bool small = false;
      bool grace = false;
      bool rest = false;
      int staff = 1;
      QString type;
      QString voice;
      Direction stemDir = Direction::AUTO;
      bool noStem = false;
      NoteHead::Group headGroup = NoteHead::Group::HEAD_NORMAL;
      QColor noteColor = QColor::Invalid;
      noteColor.setNamedColor(_e.attributes().value("color").toString());
      QColor noteheadColor = QColor::Invalid;
      bool noteheadParentheses = false;
      QString noteheadFilled;
      int velocity = round(_e.attributes().value("dynamics").toDouble() * 0.9);
      bool graceSlash = false;
      bool printObject = _e.attributes().value("print-object") != "no";
      Beam::Mode bm  = Beam::Mode::AUTO;
      QString instrId;

      mxmlNoteDuration mnd(_divs, _logger);
      mxmlNotePitch mnp(_logger);

      while (_e.readNextStartElement() && !elementMustBePostponed(_e)) {
            if (mnp.readProperties(_e, _score)) {
                  // element handled
                  }
            else if (mnd.readProperties(_e)) {
                  // element handled
                  }
            else if (_e.name() == "beam")
                  beam(bm);
            else if (_e.name() == "chord") {
                  chord = true;
                  _e.readNext();
                  }
            else if (_e.name() == "cue") {
                  cue = true;
                  _e.readNext();
                  }
            else if (_e.name() == "grace") {
                  grace = true;
                  graceSlash = _e.attributes().value("slash") == "yes";
                  _e.readNext();
                  }
            else if (_e.name() == "instrument") {
                  instrId = _e.attributes().value("id").toString();
                  _e.readNext();
                  }
            else if (_e.name() == "notehead") {
                  noteheadColor.setNamedColor(_e.attributes().value("color").toString());
                  noteheadParentheses = _e.attributes().value("parentheses") == "yes";
                  noteheadFilled = _e.attributes().value("filled").toString();
                  headGroup = convertNotehead(_e.readElementText());
                  }
            else if (_e.name() == "rest") {
                  rest = true;
                  mnp.displayStepOctave(_e);
                  }
            else if (_e.name() == "staff") {
                  auto ok = false;
                  auto strStaff = _e.readElementText();
                  staff = strStaff.toInt(&ok);
                  if (!ok) {
                        // error already reported in pass 1
                        staff = 1;
                        }
                  }
            else if (_e.name() == "stem")
                  stem(stemDir, noStem);
            else if (_e.name() == "type") {
                  small = _e.attributes().value("size") == "cue";
                  type = _e.readElementText();
                  }
            else if (_e.name() == "voice")
                  voice = _e.readElementText();
            else
                  skipLogCurrElem();
            }

      // convert staff to zero-based (in case of error, staff will be -1)
      staff--;

      // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
      if (!chord)
            // remember voice
            currentVoice = voice;
      else if (voice == "")
            // use voice from last note w/o <chord>
            voice = currentVoice;

      // Assume voice 1 if voice is empty (legal in a single voice part)
      if (voice == "")
            voice = "1";

      // check for timing error(s) and set dura
      // keep in this order as checkTiming() might change dura
      auto errorStr = mnd.checkTiming(type, rest, grace);
      dura = mnd.dura();
      if (errorStr != "")
            _logger->logError(errorStr, &_e);

      // At this point all checks have been done, the note should be added
      // note: in case of error exit from here, the postponed <note> children
      // must still be skipped

      int msMove = 0;
      int msTrack = 0;
      int msVoice = 0;

      if (!_pass1.determineStaffMoveVoice(partId, staff, voice, msMove, msTrack, msVoice)) {
            _logger->logDebugInfo(QString("could not map staff %1 voice '%2'").arg(staff + 1).arg(voice), &_e);
            // begin experimental fix for postponed <note> children
            // TODO test /repair
#if 0
            while (_e.tokenType() == QXmlStreamReader::StartElement) {

                  //qDebug("in second loop element '%s'", qPrintable(_e.name().toString()));
                  skipLogCurrElem();

                  // skip to either start of next <note> child or end of <note>
                  // currently at end of last <note> child handled
                  //qDebug("::note before skip tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));
                  do
                        _e.readNext();
                  while (!(_e.tokenType() == QXmlStreamReader::StartElement)
                         && !(_e.tokenType() == QXmlStreamReader::EndElement && _e.name() == "note"));
                  //qDebug("::note after skip tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));


                  }
#endif
            // end experimental fix for testVoiceMapper*

            Q_ASSERT(_e.isEndElement() && _e.name() == "note");
            return 0;
            }
      else {
            }

      TDuration duration = determineDuration(rest, type, mnd.dots(), dura, Fraction::fromTicks(measure->ticks()));

      ChordRest* cr = 0;
      Note* note = 0;

      // start time for note:
      // - sTime for non-chord / first chord note
      // - prevTime for others
      const Fraction noteStartTime = chord ? prevSTime : sTime;

      if (rest) {
            int track = msTrack + msVoice;
            cr = addRest(_score, measure, noteStartTime.ticks(), track, msMove,
                         duration, dura);
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
                  cr->setSmall(small);
                  if (noteColor != QColor::Invalid)
                        cr->setColor(noteColor);
                  cr->setVisible(printObject);
                  handleDisplayStep(cr, mnp.displayStep(), mnp.displayOctave(), noteStartTime.ticks(), _score->spatium());
                  }
            }
      else {
            Chord* c;
            if (!grace) {
                  // regular note
                  // if there is already a chord just add to it
                  // else create a new one
                  // this basically ignores <chord/> errors
                  c = findOrCreateChord(_score, measure,
                                        noteStartTime.ticks(),
                                        msTrack + msVoice, msMove,
                                        duration, dura, bm);
                  // handle beam
                  if (!chord)
                        handleBeamAndStemDir(c, bm, stemDir, currBeam);

                  // append any grace chord after chord to the previous chord
                  Chord* prevChord = measure->findChord(prevSTime.ticks(), msTrack + msVoice);
                  if (prevChord && prevChord != c)
                        addGraceChordsAfter(prevChord, gcl, gac);

                  // append any grace chord
                  addGraceChordsBefore(c, gcl);
                  }
            else {
                  // grace note
                  // TODO: check if explicit stem direction should also be set for grace notes
                  // (the DOM parser does that, but seems to have no effect on the autotester)
                  if (!chord || gcl.isEmpty()) {
                        c = createGraceChord(_score, msTrack + msVoice, duration, graceSlash);
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
            note->setSmall(small);
            note->setHeadGroup(headGroup);
            if (noteColor != QColor::Invalid)
                  note->setColor(noteColor);
            setNoteHead(note, noteheadColor, noteheadParentheses, noteheadFilled);
            note->setVisible(printObject); // TODO also set the stem to invisible

            if (velocity > 0) {
                  note->setVeloType(Note::ValueType::USER_VAL);
                  note->setVeloOffset(velocity);
                  }

            const MusicXMLDrumset& mxmlDrumset = _pass1.getDrumset(partId);
            if (mnp.unpitched()) {
                  //&& drumsets.contains(partId)
                  if (_hasDrumset
                      && mxmlDrumset.contains(instrId)) {
                        // step and oct are display-step and ...-oct
                        // get pitch from instrument definition in drumset instead
                        int pitch = mxmlDrumset[instrId].pitch;
                        note->setPitch(pitch);
                        // TODO - does this need to be key-aware?
                        note->setTpc(pitch2tpc(pitch, Key::C, Prefer::NEAREST)); // TODO: necessary ?
                        }
                  else {
                        //qDebug("disp step %d oct %d", displayStep, displayOctave);
                        xmlSetPitch(note, mnp.displayStep(), 0, mnp.displayOctave(), 0, _pass1.getPart(partId)->instrument());
                        }
                  }
            else {
                  int ottavaStaff = (msTrack - _pass1.trackForPart(partId)) / VOICES;
                  int octaveShift = _pass1.octaveShift(partId, ottavaStaff, noteStartTime);
                  xmlSetPitch(note, mnp.step(), mnp.alter(), mnp.octave(), octaveShift, _pass1.getPart(partId)->instrument());
                  }

            // set drumset information
            // note that in MuseScore, the drumset contains defaults for notehead,
            // line and stem direction, while a MusicXML file contains actuals.
            // the MusicXML values for each note are simply copied to the defaults

            if (mnp.unpitched()) {
                  // determine staff line based on display-step / -octave and clef type
                  ClefType clef = c->staff()->clef(noteStartTime.ticks());
                  int po = ClefInfo::pitchOffset(clef);
                  int pitch = MusicXMLStepAltOct2Pitch(mnp.displayStep(), 0, mnp.displayOctave());
                  int line = po - absStep(pitch);

                  // correct for number of staff lines
                  // see ExportMusicXml::unpitch2xml for explanation
                  // TODO handle other # staff lines ?
                  int staffLines = c->staff()->lines(0);
                  if (staffLines == 1) line -= 8;
                  if (staffLines == 3) line -= 2;

                  // the drum palette cannot handle stem direction AUTO,
                  // overrule if necessary
                  if (stemDir == Direction::AUTO) {
                        if (line > 4)
                              stemDir = Direction::DOWN;
                        else
                              stemDir = Direction::UP;
                        }

                  /*
                  if (drumsets.contains(partId)
                       && mxmlDrumset.contains(instrId)) {
                        mxmlDrumset[instrId].notehead = headGroup;
                        mxmlDrumset[instrId].line = line;
                        mxmlDrumset[instrId].stemDirection = sd;
                  }
                   */
                  // this should be done in pass 1, would make _pass1 const here
                  _pass1.setDrumsetDefault(partId, instrId, headGroup, line, stemDir);
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
                  note->add(acc);
                  // save alter value for user accidental
                  if (acc->accidentalType() != AccidentalType::NONE)
                        alt = mnp.alter();
                  }

            c->add(note);
            //c->setStemDirection(stemDir); // already done in handleBeamAndStemDir()
            c->setNoStem(noStem);
            cr = c;
            }

      // cr can be 0 here (if a rest cannot be added)
      // TODO: complete and cleanup handling this case
      if (cr) {
            cr->setVisible(printObject);
            if (cue) cr->setSmall(cue);  // only once per chord
            }

      // handle the postponed children of <note>
      // if one of these was found, the first while loop was terminated
      // at a StartElement instead of the usual EndElement

      QMap<int, Lyrics*> numberedLyrics; // lyrics with valid number
      QSet<Lyrics*> extendedLyrics;      // lyrics with the extend flag set
      MusicXmlTupletDesc tupletDesc;
      bool lastGraceAFter = false;       // set by notations() if end of grace after sequence found

      while (_e.tokenType() == QXmlStreamReader::StartElement) {

            //qDebug("in second loop element '%s'", qPrintable(_e.name().toString()));
            if (_e.name() == "lyric") {
                  // lyrics on grace notes not (yet) supported by MuseScore
                  if (!grace)
                        lyric(partId, numberedLyrics, extendedLyrics);  // TODO: move track handling to addlyric
                  else {
                        _logger->logDebugInfo("ignoring lyrics on grace notes", &_e);
                        skipLogCurrElem();
                        }
                  }
            else if (_e.name() == "notations")
                  notations(note, cr, noteStartTime.ticks(), tupletDesc, lastGraceAFter);
            else
                  skipLogCurrElem();

            // skip to either start of next <note> child or end of <note>
            // currently at end of last <note> child handled
            //qDebug("::note before skip tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));
            do
                  _e.readNext();
            while (!(_e.tokenType() == QXmlStreamReader::StartElement)
                   && !(_e.tokenType() == QXmlStreamReader::EndElement && _e.name() == "note"));
            //qDebug("::note after skip tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));

            }

      //qDebug("::note after second loop tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));

      // handle grace after state: remember current grace list size
      if (grace && lastGraceAFter)
            gac = gcl.size();

      if (!chord && !grace) {
            // do tuplet if valid time-modification is not 1/1 and is not 1/2 (tremolo)
            auto timeMod = mnd.timeMod();
            if (timeMod.isValid() && timeMod != Fraction(1, 1) && timeMod != Fraction(1, 2)) {
                  // find part-relative track
                  Part* part = _pass1.getPart(partId);
                  Q_ASSERT(part);
                  int scoreRelStaff = _score->staffIdx(part); // zero-based number of parts first staff in the score
                  int partRelTrack = msTrack + msVoice - scoreRelStaff * VOICES;
                  addTupletToChord(cr, _tuplets[partRelTrack], _tuplImpls[partRelTrack], timeMod, tupletDesc, mnd.normalType());
                  }
            }

      // add lyrics found by lyric
      if (cr) {
            // add lyrics and stop corresponding extends
            addLyrics(_logger, &_e, cr, numberedLyrics, extendedLyrics, _extendedLyrics);
            if (rest) {
                  // stop all extends
                  _extendedLyrics.setExtend(-1, cr->track(), cr->tick());
                  }
            }

      // add figured bass element
      addFiguredBassElemens(fbl, noteStartTime, msTrack, dura, measure);

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);

      if (!(_e.isEndElement() && _e.name() == "note"))
            qDebug("name %s line %lld", qPrintable(_e.name().toString()), _e.lineNumber());
      Q_ASSERT(_e.isEndElement() && _e.name() == "note");

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
      Q_ASSERT(_e.isStartElement() && _e.name() == "note");
      //_logger->logDebugTrace("MusicXMLParserPass1::notePrintSpacingNo", &_e);

      bool chord = false;
      bool grace = false;

      while (_e.readNextStartElement()) {
            if (_e.name() == "chord") {
                  chord = true;
                  _e.readNext();
                  }
            else if (_e.name() == "duration")
                  duration(dura);
            else if (_e.name() == "grace") {
                  grace = true;
                  _e.readNext();
                  }
            else
                  _e.skipCurrentElement();        // skip but don't log
            }

      // don't count chord or grace note duration
      // note that this does not check the MusicXML requirement that notes in a chord
      // cannot have a duration longer than the first note in the chord
      if (chord || grace)
            dura.set(0, 1);

      Q_ASSERT(_e.isEndElement() && _e.name() == "note");
      }

//---------------------------------------------------------
//   calcTicks
//---------------------------------------------------------

static Fraction calcTicks(const QString& text, int divs)
      {
      Fraction dura(0, 0);        // invalid unless set correctly

      int intDura = text.toInt();
      if (divs > 0)
            dura.set(intDura, 4 * divs);
      else
            qDebug("illegal or uninitialized divisions (%d)", divs);       // TODO

      return dura;
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXMLParserPass2::duration(Fraction& dura)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "duration");

      dura.set(0, 0);        // invalid unless set correctly
      int intDura = _e.readElementText().toInt();
      if (intDura > 0) {
            if (_divs > 0) {
                  dura.set(intDura, 4 * _divs);
                  dura.reduce(); // prevent overflow in later Fraction operations
                  }
            else
                  _logger->logError(QString("illegal or uninitialized divisions (%1)").arg(_divs), &_e);
            }
      else
            _logger->logError(QString("illegal duration %1").arg(dura.print()), &_e);
      //qDebug("duration %s valid %d", qPrintable(dura.print()), dura.isValid());
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
      Q_ASSERT(_e.isStartElement() && _e.name() == "figure");

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
                  QString val = _e.readElementText();
                  int iVal = val.toInt();
                  // MusicXML spec states figure-number is a number
                  // MuseScore can only handle single digit
                  if (1 <= iVal && iVal <= 9)
                        fgi->setDigit(iVal);
                  else
                        _logger->logError(QString("incorrect figure-number '%1'").arg(val), &_e);
                  }
            else if (_e.name() == "prefix")
                  fgi->setPrefix(fgi->MusicXML2Modifier(_e.readElementText()));
            else if (_e.name() == "suffix")
                  fgi->setSuffix(fgi->MusicXML2Modifier(_e.readElementText()));
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
      Q_ASSERT(_e.isStartElement() && _e.name() == "figured-bass");

      FiguredBass* fb = new FiguredBass(_score);

      bool parentheses = _e.attributes().value("parentheses") == "yes";
      QString normalizedText;
      int idx = 0;
      while (_e.readNextStartElement()) {
            if (_e.name() == "duration") {
                  Fraction dura;
                  duration(dura);
                  if (dura.isValid() && dura > Fraction(0, 1))
                        fb->setTicks(dura.ticks());
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
 */

FretDiagram* MusicXMLParserPass2::frame()
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "frame");

      FretDiagram* fd = new FretDiagram(_score);

      while (_e.readNextStartElement()) {
            if (_e.name() == "frame-frets") {
                  int val = _e.readElementText().toInt();
                  if (val > 0)
                        fd->setFrets(val);
                  else
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-fret %1").arg(val), &_e);
                  }
            else if (_e.name() == "frame-note") {
                  int fret   = -1;
                  int string = -1;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "fret")
                              fret = _e.readElementText().toInt();
                        else if (_e.name() == "string")
                              string = _e.readElementText().toInt();
                        else
                              skipLogCurrElem();
                        }
                  _logger->logDebugInfo(QString("FretDiagram::readMusicXML string %1 fret %2").arg(string).arg(fret), &_e);
                  if (string > 0) {
                        if (fret == 0)
                              fd->setMarker(fd->strings() - string, 79 /* ??? */);
                        else if (fret > 0)
                              fd->setDot(fd->strings() - string, fret);
                        }
                  }
            else if (_e.name() == "frame-strings") {
                  int val = _e.readElementText().toInt();
                  if (val > 0) {
                        fd->setStrings(val);
                        for (int i = 0; i < val; ++i)
                              fd->setMarker(i, 88 /* ??? */);
                        }
                  else
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-strings %1").arg(val), &_e);
                  }
            else
                  skipLogCurrElem();
            }

      return fd;
      }

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/harmony node.
 */

void MusicXMLParserPass2::harmony(const QString& partId, Measure* measure, const Fraction sTime)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "harmony");

      int track = _pass1.trackForPart(partId);

      // placement:
      // in order to work correctly, this should probably be adjusted to account for spatium
      // but in any case, we don't support import relative-x/y for other elements
      // no reason to do so for chord symbols
#if 0 // TODO:ws
      double rx = 0.0;        // 0.1 * e.attribute("relative-x", "0").toDouble();
      double ry = 0.0;        // -0.1 * e.attribute("relative-y", "0").toDouble();

      double styleYOff = _score->textStyle(SubStyle::HARMONY).offset().y();
      OffsetType offsetType = _score->textStyle(SubStyle::HARMONY).offsetType();
      if (offsetType == OffsetType::ABS) {
            styleYOff = styleYOff * DPMM / _score->spatium();
            }

      // TODO: check correct dy handling
      // previous code: double dy = -0.1 * e.attribute("default-y", QString::number(styleYOff* -10)).toDouble();
      double dy = -0.1 * _e.attributes().value("default-y").toDouble();
#endif
      bool printObject = _e.attributes().value("print-object") != "no";
      QString printFrame = _e.attributes().value("print-frame").toString();
      QString printStyle = _e.attributes().value("print-style").toString();

      QString kind, kindText, symbols, parens;
      QList<HDegree> degreeList;

      /* TODO ?
      if (harmony) {
            qDebug("MusicXML::import: more than one harmony");
            return;
      }
       */

      FretDiagram* fd = 0;
      Harmony* ha = new Harmony(_score);
//TODO:ws      ha->setUserOff(QPointF(rx, ry + dy - styleYOff));
      Fraction offset;
      while (_e.readNextStartElement()) {
            if (_e.name() == "root") {
                  QString step;
                  int alter = 0;
                  bool invalidRoot = false;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "root-step") {
                              // attributes: print-style
                              step = _e.readElementText();
                              /* TODO: check if this is required
                              if (ee.hasAttribute("text")) {
                                    QString rtext = ee.attribute("text");
                                    if (rtext == "") {
                                          invalidRoot = true;
                                    }
                              }
                               */
                              }
                        else if (_e.name() == "root-alter") {
                              // attributes: print-object, print-style
                              //             location (left-right)
                              alter = _e.readElementText().toInt();
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
                  // attributes: print-style
                  skipLogCurrElem();
                  }
            else if (_e.name() == "kind") {
                  // attributes: use-symbols  yes-no
                  //             text, stack-degrees, parentheses-degree, bracket-degrees,
                  //             print-style, halign, valign

                  kindText = _e.attributes().value("text").toString();
                  symbols = _e.attributes().value("use-symbols").toString();
                  parens = _e.attributes().value("parentheses-degrees").toString();
                  kind = _e.readElementText();
                  }
            else if (_e.name() == "inversion") {
                  // attributes: print-style
                  skipLogCurrElem();
                  }
            else if (_e.name() == "bass") {
                  QString step;
                  int alter = 0;
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "bass-step") {
                              // attributes: print-style
                              step = _e.readElementText();
                              }
                        else if (_e.name() == "bass-alter") {
                              // attributes: print-object, print-style
                              //             location (left-right)
                              alter = _e.readElementText().toInt();
                              }
                        else
                              skipLogCurrElem();
                        }
                  ha->setBaseTpc(step2tpc(step, AccidentalVal(alter)));
                  }
            else if (_e.name() == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  while (_e.readNextStartElement()) {
                        if (_e.name() == "degree-value") {
                              degreeValue = _e.readElementText().toInt();
                              }
                        else if (_e.name() == "degree-alter") {
                              degreeAlter = _e.readElementText().toInt();
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
                  fd = frame();
            else if (_e.name() == "level")
                  skipLogCurrElem();
            else if (_e.name() == "offset")
                  offset = calcTicks(_e.readElementText(), _divs);
            else if (_e.name() == "staff") {
                  int nstaves = _pass1.getPart(partId)->nstaves();
                  QString strStaff = _e.readElementText();
                  int staff = strStaff.toInt();
                  if (0 < staff && staff <= nstaves)
                        track += (staff - 1) * VOICES;
                  else
                        _logger->logError(QString("invalid staff %1").arg(strStaff), &_e);
                  }
            else
                  skipLogCurrElem();
            }

      if (fd) {
            fd->setTrack(track);
            Segment* s = measure->getSegment(SegmentType::ChordRest, (sTime + offset).ticks());
            s->add(fd);
            }

      const ChordDescription* d = 0;
      if (ha->rootTpc() != Tpc::TPC_INVALID)
            d = ha->fromXml(kind, kindText, symbols, parens, degreeList);
      if (d) {
            ha->setId(d->id);
            ha->setTextName(d->names.front());
            }
      else {
            ha->setId(-1);
            ha->setTextName(kindText);
            }
      ha->render();

      ha->setVisible(printObject);

      // TODO-LV: do this only if ha points to a valid harmony
      // harmony = ha;
      ha->setTrack(track);
      Segment* s = measure->getSegment(SegmentType::ChordRest, (sTime + offset).ticks());
      s->add(ha);
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/beam node.
 Sets beamMode in case of begin, continue or end beam number 1.
 */

void MusicXMLParserPass2::beam(Beam::Mode& beamMode)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "beam");

      int beamNo = _e.attributes().value("number").toInt();

      if (beamNo == 1) {
            QString s = _e.readElementText();
            if (s == "begin")
                  beamMode = Beam::Mode::BEGIN;
            else if (s == "end")
                  beamMode = Beam::Mode::END;
            else if (s == "continue")
                  beamMode = Beam::Mode::MID;
            else if (s == "backward hook")
                  ;
            else if (s == "forward hook")
                  ;
            else
                  _logger->logError(QString("unknown beam keyword '%1'").arg(s), &_e);
            }
      else
            _e.skipCurrentElement();
      }

//---------------------------------------------------------
//   forward
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/forward node.
 */

void MusicXMLParserPass2::forward(Fraction& dura)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "forward");

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
      Q_ASSERT(_e.isStartElement() && _e.name() == "backup");

      while (_e.readNextStartElement()) {
            if (_e.name() == "duration")
                  duration(dura);
            else
                  skipLogCurrElem();
            }
      }

//---------------------------------------------------------
//   lyric -- parse a MusicXML lyric element
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/lyric node.
 */

void MusicXMLParserPass2::lyric(const QString& partId,
                                QMap<int, Lyrics*>& numbrdLyrics,
                                QSet<Lyrics*>& extLyrics)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "lyric");

      Lyrics* l = new Lyrics(_score);
      // TODO in addlyrics: l->setTrack(trk);

      bool hasExtend = false;
      QString lyricNumber = _e.attributes().value("number").toString();
      QColor lyricColor = QColor::Invalid;
      lyricColor.setNamedColor(_e.attributes().value("color").toString());
      QString extendType;
      QString formattedText;

      while (_e.readNextStartElement()) {
            if (_e.name() == "elision") {
                  // TODO verify elision handling
                  /*
                   QString text = _e.readElementText();
                   if (text.isEmpty())
                   formattedText += " ";
                   else
                   */
                  formattedText += nextPartOfFormattedString(_e);
                  }
            else if (_e.name() == "extend") {
                  hasExtend = true;
                  extendType = _e.attributes().value("type").toString();
                  _e.readNext();
                  }
            else if (_e.name() == "syllabic") {
                  QString syll = _e.readElementText();
                  if (syll == "single")
                        l->setSyllabic(Lyrics::Syllabic::SINGLE);
                  else if (syll == "begin")
                        l->setSyllabic(Lyrics::Syllabic::BEGIN);
                  else if (syll == "end")
                        l->setSyllabic(Lyrics::Syllabic::END);
                  else if (syll == "middle")
                        l->setSyllabic(Lyrics::Syllabic::MIDDLE);
                  else
                        qDebug("unknown syllabic %s", qPrintable(syll));  // TODO
                  }
            else if (_e.name() == "text")
                  formattedText += nextPartOfFormattedString(_e);
            else
                  skipLogCurrElem();
            }

      // if no lyric read (e.g. only 'extend "type=stop"'), no further action required
      if (formattedText == "") {
            delete l;
            return;
            }

      auto mxmlPart = _pass1.getMusicXmlPart(partId);
      auto lyricNo = mxmlPart.lyricNumberHandler().getLyricNo(lyricNumber);
      if (lyricNo < 0) {
            _logger->logError("invalid lyrics number (<0)", &_e);
            delete l;
            return;
            }
      else if (lyricNo > MAX_LYRICS) {
            _logger->logError(QString("too much lyrics (>%1)").arg(MAX_LYRICS), &_e);
            delete l;
            return;
            }
      else if (numbrdLyrics.contains(lyricNo)) {
            _logger->logError(QString("duplicate lyrics number (%1)").arg(lyricNumber), &_e);
            delete l;
            return;
            }

      numbrdLyrics[lyricNo] = l;

      if (hasExtend && (extendType == "" || extendType == "start"))
            extLyrics.insert(l);

      //qDebug("formatted lyric '%s'", qPrintable(formattedText));
      l->setXmlText(formattedText);
      if (lyricColor != QColor::Invalid)
            l->setColor(lyricColor);
      }

//---------------------------------------------------------
//   slur
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/slur node.
 */

void MusicXMLParserPass2::slur(ChordRest* cr, const int tick, const int track, bool& lastGraceAFter)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "slur");

      int slurNo   = _e.attributes().value("number").toString().toInt();
      if (slurNo > 0) slurNo--;
      QString slurType = _e.attributes().value("type").toString();
      QString lineType  = _e.attributes().value("line-type").toString();
      if (lineType == "") lineType = "solid";

      // PriMus Music-Notation by Columbussoft (build 10093) generates overlapping
      // slurs that do not have a number attribute to distinguish them.
      // The duplicates must be ignored, to prevent memory allocation issues,
      // which caused a MuseScore crash
      // Similar issues happen with Sibelius 7.1.3 (direct export)

      if (slurType == "start") {
            if (_slurs[slurNo].isStart())
                  // slur start when slur already started: report error
                  _logger->logError(QString("ignoring duplicate slur start"), &_e);
            else if (_slurs[slurNo].isStop()) {
                  // slur start when slur already stopped: wrap up
                  Slur* newSlur = _slurs[slurNo].slur();
                  newSlur->setTick(tick);
                  newSlur->setStartElement(cr);
                  _slurs[slurNo] = SlurDesc();
                  }
            else {
                  // slur start for new slur: init
                  Slur* newSlur = new Slur(_score);
                  if (cr->isGrace())
                        newSlur->setAnchor(Spanner::Anchor::CHORD);
                  if (lineType == "dotted")
                        newSlur->setLineType(1);
                  else if (lineType == "dashed")
                        newSlur->setLineType(2);
                  newSlur->setTick(tick);
                  newSlur->setStartElement(cr);
                  QString pl = _e.attributes().value("placement").toString();
                  if (pl == "above")
                        newSlur->setSlurDirection(Direction::UP);
                  else if (pl == "below")
                        newSlur->setSlurDirection(Direction::DOWN);
                  newSlur->setTrack(track);
                  newSlur->setTrack2(track);
                  _slurs[slurNo].start(newSlur);
                  _score->addElement(newSlur);
                  }
            }
      else if (slurType == "stop") {
            if (_slurs[slurNo].isStart()) {
                  // slur stop when slur already started: wrap up
                  Slur* newSlur = _slurs[slurNo].slur();
                  if (!(cr->isGrace())) {
                        newSlur->setTick2(tick);
                        newSlur->setTrack2(track);
                        }
                  newSlur->setEndElement(cr);
                  _slurs[slurNo] = SlurDesc();
                  }
            else if (_slurs[slurNo].isStop())
                  // slur stop when slur already stopped: report error
                  _logger->logError(QString("ignoring duplicate slur stop"), &_e);
            else {
                  // slur stop for new slur: init
                  Slur* newSlur = new Slur(_score);
                  if (!(cr->isGrace())) {
                        newSlur->setTick2(tick);
                        newSlur->setTrack2(track);
                        }
                  newSlur->setEndElement(cr);
                  _slurs[slurNo].stop(newSlur);
                  }
            // any grace note containing a slur stop means
            // last note of a grace after set has been found
            if (cr->isGrace())
                  lastGraceAFter = true;
            }
      else if (slurType == "continue")
            ;        // ignore
      else
            _logger->logError(QString("unknown slur type %1").arg(slurType), &_e);

      _e.readNext();
      }

//---------------------------------------------------------
//   tied
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/tied node.
 */

void MusicXMLParserPass2::tied(Note* note, const int track)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "tied");

      QString tiedType = _e.attributes().value("type").toString();
      if (tiedType == "start") {
            if (_tie) {
                  _logger->logError(QString("Tie already active"), &_e);
                  }
            else if (note) {
                  _tie = new Tie(_score);
                  note->setTieFor(_tie);
                  _tie->setStartNote(note);
                  _tie->setTrack(track);
                  QString tiedOrientation = _e.attributes().value("orientation").toString();
                  if (tiedOrientation == "over")
                        _tie->setSlurDirection(Direction::UP);
                  else if (tiedOrientation == "under")
                        _tie->setSlurDirection(Direction::DOWN);
                  else if (tiedOrientation == "auto")
                        ;        // ignore
                  else if (tiedOrientation == "")
                        ;        // ignore
                  else
                        _logger->logError(QString("unknown tied orientation: %1").arg(tiedOrientation), &_e);

                  QString lineType  = _e.attributes().value("line-type").toString();
                  if (lineType == "dotted")
                        _tie->setLineType(1);
                  else if (lineType == "dashed")
                        _tie->setLineType(2);
                  _tie = 0;
                  }
            }
      else if (tiedType == "stop")
            ;        // ignore
      else
            _logger->logError(QString("unknown tied type %").arg(tiedType), &_e);

      _e.readNext();
      }

//---------------------------------------------------------
//   dynamics
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/dynamics node.
 */

void MusicXMLParserPass2::dynamics(QString& placement, QStringList& dynamicslist)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "dynamics");

      placement = _e.attributes().value("placement").toString();
      if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)) {
            // ry        = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
            // rx        = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
            // yoffset   = _e.attributes().value("default-y").toDouble(&hasYoffset) * -0.1;
            // xoffset   = ee.attribute("default-x", "0.0").toDouble() * 0.1;
            }
      while (_e.readNextStartElement()) {
            if (_e.name() == "other-dynamics")
                  dynamicslist.push_back(_e.readElementText());
            else {
                  dynamicslist.push_back(_e.name().toString());
                  _e.readNext();
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

void MusicXMLParserPass2::articulations(ChordRest* cr, SymId& breath, QString& chordLineType)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "articulations");

      while (_e.readNextStartElement()) {
            if (addMxmlArticulationToChord(cr, _e.name().toString())) {
                  _e.readNext();
                  continue;
                  }
            else if (_e.name() == "breath-mark") {
                  breath = SymId::breathMarkComma;
                  _e.readElementText();
                  // TODO: handle value read (note: encoding unknown, only "comma" found)
                  }
            else if (_e.name() == "caesura") {
                  breath = SymId::caesura;
                  _e.readNext();
                  }
            else if (_e.name() == "doit"
                     || _e.name() == "falloff"
                     || _e.name() == "plop"
                     || _e.name() == "scoop") {
                  chordLineType = _e.name().toString();
                  _e.readNext();
                  }
            else if (_e.name() == "strong-accent") {
                  QString strongAccentType = _e.attributes().value("type").toString();
                  if (strongAccentType == "up" || strongAccentType == "")
                        addArticulationToChord(cr, SymId::articMarcatoAbove, "up");
                  else if (strongAccentType == "down")
                        addArticulationToChord(cr, SymId::articMarcatoAbove, "down");
                  else
                        _logger->logError(QString("unknown mercato type %1").arg(strongAccentType), &_e);
                  _e.readNext();
                  }
            else
                  skipLogCurrElem();
            }
      //qDebug("::notations tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));
      }

//---------------------------------------------------------
//   ornaments
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/ornaments node.
 */

void MusicXMLParserPass2::ornaments(ChordRest* cr,
                                    QString& wavyLineType,
                                    int& wavyLineNo,
                                    QString& tremoloType,
                                    int& tremoloNr, bool& lastGraceAFter)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "ornaments");

      bool trillMark = false;
      // <trill-mark placement="above"/>
      while (_e.readNextStartElement()) {
            if (addMxmlArticulationToChord(cr, _e.name().toString())) {
                  _e.readNext();
                  continue;
                  }
            else if (_e.name() == "trill-mark") {
                  trillMark = true;
                  _e.readNext();
                  }
            else if (_e.name() == "wavy-line") {
                  wavyLineType = _e.attributes().value("type").toString();
                  wavyLineNo   = _e.attributes().value("number").toString().toInt();
                  if (wavyLineNo > 0) wavyLineNo--;
                  // any grace note containing a wavy-line stop means
                  // last note of a grace after set has been found
                  if (wavyLineType == "stop" && cr->isGrace())
                        lastGraceAFter = true;
                  _e.readNext();
                  }
            else if (_e.name() == "tremolo") {
                  tremoloType = _e.attributes().value("type").toString();
                  tremoloNr = _e.readElementText().toInt();
                  }
            else if (_e.name() == "accidental-mark")
                  skipLogCurrElem();
            else if (_e.name() == "delayed-turn") {
                  // TODO: actually this should be offset a bit to the right
                  addArticulationToChord(cr, SymId::ornamentTurn, "");
                  _e.readNext();
                  }
            else if (_e.name() == "inverted-mordent"
                     || _e.name() == "mordent") {
                  addMordentToChord(cr, _e.name().toString(),
                                    _e.attributes().value("long").toString(),
                                    _e.attributes().value("approach").toString(),
                                    _e.attributes().value("departure").toString());
                  _e.readNext();
                  }
            else
                  skipLogCurrElem();
            }
      //qDebug("::notations tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));
      // note that mscore wavy line already implicitly includes a trillsym
      // so don't add an additional one
      if (trillMark && wavyLineType != "start")
            addArticulationToChord(cr, SymId::ornamentTrill, "");
      }

//---------------------------------------------------------
//   technical
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical node.
 */

void MusicXMLParserPass2::technical(Note* note, ChordRest* cr)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "technical");

      while (_e.readNextStartElement()) {
            if (addMxmlArticulationToChord(cr, _e.name().toString())) {
                  _e.readNext();
                  continue;
                  }
            else if (_e.name() == "fingering")
                  // TODO: distinguish between keyboards (style SubStyle::FINGERING)
                  // and (plucked) strings (style SubStyle::LH_GUITAR_FINGERING)
                  addTextToNote(_e.lineNumber(), _e.columnNumber(), _e.readElementText(),
                                SubStyle::FINGERING, _score, note);
            else if (_e.name() == "fret") {
                  int fret = _e.readElementText().toInt();
                  if (note) {
                        if (note->staff()->isTabStaff(0))
                              note->setFret(fret);
                        }
                  else
                        _logger->logError("no note for fret", &_e);
                  }
            else if (_e.name() == "pluck")
                  addTextToNote(_e.lineNumber(), _e.columnNumber(), _e.readElementText(),
                                SubStyle::RH_GUITAR_FINGERING, _score, note);
            else if (_e.name() == "string") {
                  QString txt = _e.readElementText();
                  if (note) {
                        if (note->staff()->isTabStaff(0))
                              note->setString(txt.toInt() - 1);
                        else
                              addTextToNote(_e.lineNumber(), _e.columnNumber(), txt,
                                            SubStyle::STRING_NUMBER, _score, note);
                        }
                  else
                        _logger->logError("no note for string", &_e);
                  }
            else if (_e.name() == "pull-off")
                  skipLogCurrElem();
            else
                  skipLogCurrElem();
            }
      //qDebug("::notations tokenString '%s' name '%s'", qPrintable(_e.tokenString()), qPrintable(_e.name().toString()));
      }

//---------------------------------------------------------
//   glissando
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/glissando
 and /score-partwise/part/measure/note/notations/slide nodes.
 */

void MusicXMLParserPass2::glissando(Note* note, const int tick, const int ticks, const int track)
      {
      Q_ASSERT(_e.isStartElement() && (_e.name() == "glissando" || _e.name() == "slide"));

      int n                   = _e.attributes().value("number").toString().toInt();
      if (n > 0) n--;
      QString spannerType     = _e.attributes().value("type").toString();
      int tag                 = _e.name() == "slide" ? 0 : 1;
      //                  QString lineType  = ee.attribute(QString("line-type"), "solid");
      Glissando*& gliss = _glissandi[n][tag];
      if (spannerType == "start") {
            QColor color(_e.attributes().value("color").toString());
            QString glissText = _e.readElementText();
            if (gliss) {
                  _logger->logError(QString("overlapping glissando/slide number %1").arg(n+1), &_e);
                  }
            else if (!note) {
                  _logger->logError(QString("no note for glissando/slide number %1 start").arg(n+1), &_e);
                  }
            else {
                  gliss = new Glissando(_score);
                  gliss->setAnchor(Spanner::Anchor::NOTE);
                  gliss->setStartElement(note);
                  gliss->setTick(tick);
                  gliss->setTrack(track);
                  gliss->setParent(note);
                  if (color.isValid())
                        gliss->setColor(color);
                  gliss->setText(glissText);
                  gliss->setGlissandoType(tag == 0 ? GlissandoType::STRAIGHT : GlissandoType::WAVY);
                  _spanners[gliss] = QPair<int, int>(tick, -1);
                  // qDebug("glissando/slide=%p inserted at first tick %d", gliss, tick);
                  }
            }
      else if (spannerType == "stop") {
            if (!gliss) {
                  _logger->logError(QString("glissando/slide number %1 stop without start").arg(n+1), &_e);
                  }
            else if (!note) {
                  _logger->logError(QString("no note for glissando/slide number %1 stop").arg(n+1), &_e);
                  }
            else {
                  _spanners[gliss].second = tick + ticks;
                  gliss->setEndElement(note);
                  gliss->setTick2(tick);
                  gliss->setTrack2(track);
                  // qDebug("glissando/slide=%p second tick %d", gliss, tick);
                  gliss = 0;
                  }
            }
      else
            _logger->logError(QString("unknown glissando/slide type %1").arg(spannerType), &_e);
      _e.readNext();
      }

//---------------------------------------------------------
//   addArpeggio
//---------------------------------------------------------

static void addArpeggio(ChordRest* cr, const QString& arpeggioType,
                        MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      // no support for arpeggio on rest
      if (!arpeggioType.isEmpty() && cr->type() == ElementType::CHORD) {
            Arpeggio* a = new Arpeggio(cr->score());
            if (arpeggioType == "none")
                  a->setArpeggioType(ArpeggioType::NORMAL);
            else if (arpeggioType == "up")
                  a->setArpeggioType(ArpeggioType::UP);
            else if (arpeggioType == "down")
                  a->setArpeggioType(ArpeggioType::DOWN);
            else if (arpeggioType == "non-arpeggiate")
                  a->setArpeggioType(ArpeggioType::BRACKET);
            else {
                  logger->logError(QString("unknown arpeggio type %1").arg(arpeggioType), xmlreader);
                  delete a;
                  a = 0;
                  }
            if ((static_cast<Chord*>(cr))->arpeggio()) {
                  // there can be only one
                  delete a;
                  a = 0;
                  }
            else
                  cr->add(a);
            }
      }

//---------------------------------------------------------
//   addTremolo
//---------------------------------------------------------

static void addTremolo(ChordRest* cr,
                       const int tremoloNr, const QString& tremoloType, const int ticks,
                       Chord*& tremStart,
                       MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      if (!cr->isChord())
            return;
      if (tremoloNr) {
            //qDebug("tremolo %d type '%s' ticks %d tremStart %p", tremoloNr, qPrintable(tremoloType), ticks, _tremStart);
            if (tremoloNr == 1 || tremoloNr == 2 || tremoloNr == 3 || tremoloNr == 4) {
                  if (tremoloType == "" || tremoloType == "single") {
                        Tremolo* t = new Tremolo(cr->score());
                        switch (tremoloNr) {
                              case 1: t->setTremoloType(TremoloType::R8); break;
                              case 2: t->setTremoloType(TremoloType::R16); break;
                              case 3: t->setTremoloType(TremoloType::R32); break;
                              case 4: t->setTremoloType(TremoloType::R64); break;
                              }
                        cr->add(t);
                        }
                  else if (tremoloType == "start") {
                        if (tremStart) logger->logError("MusicXML::import: double tremolo start", xmlreader);
                        tremStart = static_cast<Chord*>(cr);
                        }
                  else if (tremoloType == "stop") {
                        if (tremStart) {
                              Tremolo* t = new Tremolo(cr->score());
                              switch (tremoloNr) {
                                    case 1: t->setTremoloType(TremoloType::C8); break;
                                    case 2: t->setTremoloType(TremoloType::C16); break;
                                    case 3: t->setTremoloType(TremoloType::C32); break;
                                    case 4: t->setTremoloType(TremoloType::C64); break;
                                    }
                              t->setChords(tremStart, static_cast<Chord*>(cr));
                              // fixup chord duration and type
                              const int tremDur = ticks / 2;
                              t->chord1()->setDurationType(tremDur);
                              t->chord1()->setDuration(Fraction::fromTicks(tremDur));
                              t->chord2()->setDurationType(tremDur);
                              t->chord2()->setDuration(Fraction::fromTicks(tremDur));
                              // add tremolo to first chord (only)
                              tremStart->add(t);
                              }
                        else logger->logError("MusicXML::import: double tremolo stop w/o start", xmlreader);
                        tremStart = 0;
                        }
                  }
            else
                  logger->logError(QString("unknown tremolo type %1").arg(tremoloNr), xmlreader);
            }
      }

//---------------------------------------------------------
//   addWavyLine
//---------------------------------------------------------

static void addWavyLine(ChordRest* cr, const int tick,
                        const int wavyLineNo, const QString& wavyLineType,
                        MusicXmlSpannerMap& spanners, TrillStack& trills,
                        MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      if (!wavyLineType.isEmpty()) {
            const auto ticks = cr->duration().ticks();
            const auto track = cr->track();
            const auto trk = (track / VOICES) * VOICES;       // first track of staff
            Trill*& t = trills[wavyLineNo];
            if (wavyLineType == "start") {
                  if (t) {
                        logger->logError(QString("overlapping wavy-line number %1").arg(wavyLineNo+1), xmlreader);
                        }
                  else {
                        t = new Trill(cr->score());
                        t->setTrack(trk);
                        spanners[t] = QPair<int, int>(tick, -1);
                        // qDebug("wedge trill=%p inserted at first tick %d", trill, tick);
                        }
                  }
            else if (wavyLineType == "stop") {
                  if (!t) {
                        logger->logError(QString("wavy-line number %1 stop without start").arg(wavyLineNo+1), xmlreader);
                        }
                  else {
                        spanners[t].second = tick + ticks;
                        // qDebug("wedge trill=%p second tick %d", trill, tick);
                        t = 0;
                        }
                  }
            else
                  logger->logError(QString("unknown wavy-line type %1").arg(wavyLineType), xmlreader);
            }
      }

//---------------------------------------------------------
//   addBreath
//---------------------------------------------------------

static void addBreath(ChordRest* cr, const int tick, SymId breath)
      {
      if (breath != SymId::noSym && !cr->isGrace()) {
            Breath* b = new Breath(cr->score());
            // b->setTrack(trk + voice); TODO check next line
            b->setTrack(cr->track());
            b->setSymId(breath);
            const auto ticks = cr->duration().ticks();
            auto seg = cr->measure()->getSegment(SegmentType::Breath, tick + ticks);
            seg->add(b);
            }
      }

//---------------------------------------------------------
//   addChordLine
//---------------------------------------------------------

static void addChordLine(Note* note, const QString& chordLineType,
                         MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
      {
      if (chordLineType != "") {
            if (note) {
                  ChordLine* cl = new ChordLine(note->score());
                  if (chordLineType == "falloff")
                        cl->setChordLineType(ChordLineType::FALL);
                  if (chordLineType == "doit")
                        cl->setChordLineType(ChordLineType::DOIT);
                  if (chordLineType == "plop")
                        cl->setChordLineType(ChordLineType::PLOP);
                  if (chordLineType == "scoop")
                        cl->setChordLineType(ChordLineType::SCOOP);
                  note->chord()->add(cl);
                  }
            else
                  logger->logError(QString("no note for %1").arg(chordLineType), xmlreader);
            }
      }

//---------------------------------------------------------
//   notations
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations node.
 Note that some notations attach to notes only in MuseScore,
 which means trying to attach them to a rest will crash,
 as in that case note is a nullptr.
 */

void MusicXMLParserPass2::notations(Note* note, ChordRest* cr, const int tick,
                                    MusicXmlTupletDesc& tupletDesc, bool& lastGraceAFter)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "notations");

      if (cr) {
            lastGraceAFter = false; // ensure default

            Measure* measure = cr->measure();
            int ticks = cr->duration().ticks();
            int track = cr->track();

            QString wavyLineType;
            int wavyLineNo = 0;
            QString arpeggioType;
            SymId breath = SymId::noSym;
            int tremoloNr = 0;
            QString tremoloType;
            QString placement;
            QStringList dynamicslist;
            // qreal rx = 0.0;
            // qreal ry = 0.0;
            // qreal yoffset = 0.0; // actually this is default-y
            // qreal xoffset = 0.0; // not used
            // bool hasYoffset = false;
            QString chordLineType;

            while (_e.readNextStartElement()) {
                  if (_e.name() == "slur") {
                        slur(cr, tick, track, lastGraceAFter);
                        }
                  else if (_e.name() == "tied") {
                        tied(note, track);
                        }
                  else if (_e.name() == "tuplet") {
                        tuplet(tupletDesc);
                        }
                  else if (_e.name() == "dynamics") {
                        placement = _e.attributes().value("placement").toString();
                        if (preferences.getBool(PREF_IMPORT_MUSICXML_IMPORTLAYOUT)) {
                              // ry        = ee.attribute(QString("relative-y"), "0").toDouble() * -.1;
                              // rx        = ee.attribute(QString("relative-x"), "0").toDouble() * .1;
                              // yoffset   = _e.attributes().value("default-y").toDouble(&hasYoffset) * -0.1;
                              // xoffset   = ee.attribute("default-x", "0.0").toDouble() * 0.1;
                              }
                        dynamics(placement, dynamicslist);
                        }
                  else if (_e.name() == "articulations") {
                        articulations(cr, breath, chordLineType);
                        }
                  else if (_e.name() == "fermata")
                        fermata(cr);
                  else if (_e.name() == "ornaments") {
                        ornaments(cr, wavyLineType, wavyLineNo, tremoloType, tremoloNr, lastGraceAFter);
                        }
                  else if (_e.name() == "technical") {
                        technical(note, cr);
                        }
                  else if (_e.name() == "arpeggiate") {
                        arpeggioType = _e.attributes().value("direction").toString();
                        if (arpeggioType == "") arpeggioType = "none";
                        _e.readNext();
                        }
                  else if (_e.name() == "non-arpeggiate") {
                        arpeggioType = "non-arpeggiate";
                        _e.readNext();
                        }
                  else if (_e.name() == "glissando" || _e.name() == "slide") {
                        glissando(note, tick, ticks, track);
                        }
                  else
                        skipLogCurrElem();
                  }

            addArpeggio(cr, arpeggioType, _logger, &_e);
            addWavyLine(cr, tick, wavyLineNo, wavyLineType, _spanners, _trills, _logger, &_e);
            addBreath(cr, tick, breath);
            addTremolo(cr, tremoloNr, tremoloType, ticks, _tremStart, _logger, &_e);
            addChordLine(note, chordLineType, _logger, &_e);

            // more than one dynamic ???
            // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
            // TODO remove duplicate code (see MusicXml::direction)
            for (QStringList::Iterator it = dynamicslist.begin(); it != dynamicslist.end(); ++it ) {
                  Dynamic* dyn = new Dynamic(_score);
                  dyn->setDynamicType(*it);
//TODO:ws            if (hasYoffset) dyn->textStyle().setYoff(yoffset);
                  addElemOffset(dyn, track, placement, measure, tick);
                  }
            }
      else {
            _logger->logDebugInfo("no note to attach to, skipping notations", &_e);
            _e.skipCurrentElement();
            }

      Q_ASSERT(_e.isEndElement() && _e.name() == "notations");
      }

//---------------------------------------------------------
//   stem
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/stem node.
 */

void MusicXMLParserPass2::stem(Direction& sd, bool& nost)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "stem");

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

void MusicXMLParserPass2::fermata(ChordRest* cr)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "fermata");

      QString fermataType = _e.attributes().value("type").toString();
      QString fermata     = _e.readElementText();

      if (fermata == "normal" || fermata == "")
            addFermata(cr, fermataType, SymId::fermataAbove);
      else if (fermata == "angled")
            addFermata(cr, fermataType, SymId::fermataShortAbove);
      else if (fermata == "square")
            addFermata(cr, fermataType, SymId::fermataLongAbove);
      else
            _logger->logError(QString("unknown fermata '%1'").arg(fermata), &_e);
      }

//---------------------------------------------------------
//   tuplet
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/tuplet node.
 */

void MusicXMLParserPass2::tuplet(MusicXmlTupletDesc& tupletDesc)
      {
      Q_ASSERT(_e.isStartElement() && _e.name() == "tuplet");

      QString tupletType       = _e.attributes().value("type").toString();
      // QString tupletPlacement  = _e.attributes().value("placement").toString(); not used (TODO)
      QString tupletBracket    = _e.attributes().value("bracket").toString();
      QString tupletShowNumber = _e.attributes().value("show-number").toString();

      // ignore possible children (currently not supported)
      _e.skipCurrentElement();

      if (tupletType == "start")
            tupletDesc.type = MxmlStartStop::START;
      else if (tupletType == "stop")
            tupletDesc.type = MxmlStartStop::STOP;
      else if (tupletType != "" && tupletType != "start" && tupletType != "stop") {
            _logger->logError(QString("unknown tuplet type '%1'").arg(tupletType), &_e);
            }

      // set bracket, leave at default if unspecified
      if (tupletBracket == "yes")
            tupletDesc.bracket = Tuplet::BracketType::SHOW_BRACKET;
      else if (tupletBracket == "no")
            tupletDesc.bracket = Tuplet::BracketType::SHOW_NO_BRACKET;

      // set number, default is "actual" (=NumberType::SHOW_NUMBER)
      if (tupletShowNumber == "both")
            tupletDesc.shownumber = Tuplet::NumberType::SHOW_RELATION;
      else if (tupletShowNumber == "none")
            tupletDesc.shownumber = Tuplet::NumberType::NO_TEXT;
      else
            tupletDesc.shownumber = Tuplet::NumberType::SHOW_NUMBER;
      }

//---------------------------------------------------------
//   MusicXMLParserDirection
//---------------------------------------------------------

/**
 MusicXMLParserDirection constructor.
 */

MusicXMLParserDirection::MusicXMLParserDirection(QXmlStreamReader& e,
                                                 Score* score,
                                                 const MusicXMLParserPass1& pass1,
                                                 MusicXMLParserPass2& pass2,
                                                 MxmlLogger* logger)
      : _e(e), _score(score), _pass1(pass1), _pass2(pass2), _logger(logger),
      _hasDefaultY(false), _defaultY(0.0), _coda(false), _segno(false),
      _tpoMetro(0), _tpoSound(0)
      {
      // nothing
      }

}
