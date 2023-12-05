/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <cmath>
#include <memory>
#include <utility>

#include <QRegularExpression>

#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"
#include "iengravingfont.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/box.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/interval.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/line.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempo.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolo.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/volta.h"

#include "importmxmllogger.h"
#include "importmxmlnoteduration.h"
#include "importmxmlnotepitch.h"
#include "importmxmlpass1.h"
#include "importmxmlpass2.h"
#include "musicxmlfonthandler.h"
#include "musicxmlsupport.h"

#include "modularity/ioc.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/rendering/dev/tlayout.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

namespace mu::engraving {
static std::shared_ptr<mu::iex::musicxml::IMusicXmlConfiguration> configuration()
{
    return mu::modularity::ioc()->resolve<mu::iex::musicxml::IMusicXmlConfiguration>("iex_musicxml");
}

static std::shared_ptr<mu::engraving::IEngravingFontsProvider> engravingFonts()
{
    return mu::modularity::ioc()->resolve<mu::engraving::IEngravingFontsProvider>("iex_musicxml");
}

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

//#define DEBUG_VOICE_MAPPER true

//---------------------------------------------------------
//   function declarations
//---------------------------------------------------------

static void addTie(const Notation& notation, Score* score, Note* note, const track_idx_t track, Tie*& tie, MxmlLogger* logger,
                   const QXmlStreamReader* const xmlreader);

//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

//---------------------------------------------------------
//   MusicXmlTupletDesc
//---------------------------------------------------------

MusicXmlTupletDesc::MusicXmlTupletDesc()
    : type(MxmlStartStop::NONE), direction(DirectionV::AUTO),
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

static Fraction lastChordTicks(const Segment* s, const Fraction& tick)
{
    while (s && s->tick() < tick) {
        for (EngravingItem* el : s->elist()) {
            if (el && el->isChordRest()) {
                ChordRest* cr = static_cast<ChordRest*>(el);
                if (cr->tick() + cr->actualTicks() == tick) {
                    return cr->actualTicks();
                }
            }
        }
        s = s->nextCR(mu::nidx, true);
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   setExtend
//---------------------------------------------------------

// set extend for lyric no in *staff* to end at tick
// called when lyric (with or without "extend") or note with "extend type=stop" is found
// // note that no == -1 means all lyrics in this *track*

void MusicXmlLyricsExtend::setExtend(const int no, const track_idx_t track, const Fraction& tick)
{
    QList<Lyrics*> list;
    foreach (Lyrics* l, _lyrics) {
        EngravingItem* const el = l->parentItem();
        if (el->type() == ElementType::CHORD || el->type() == ElementType::REST) {
            ChordRest* const par = static_cast<ChordRest*>(el);
            if ((no == -1 && par->track() == track)
                || (l->no() == no && track2staff(par->track()) == track2staff(track))) {
                Fraction lct = lastChordTicks(l->segment(), tick);
                if (lct > Fraction(0, 1)) {
                    // set lyric tick to the total length from the lyric note
                    // plus all notes covered by the melisma minus the last note length
                    l->setTicks(tick - par->tick() - lct);
                }
                list.append(l);
            }
        }
    }
    // cleanup
    foreach (Lyrics* l, list) {
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
        LOGD("MusicXMLStepAltOct2Pitch: illegal step %d", step);
        return -1;
    }
    int pitch = table[step] + alter + (octave + 1) * 12;

    if (pitch < 0) {
        pitch = -1;
    }
    if (pitch > 127) {
        pitch = -1;
    }

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

static void xmlSetPitch(Note* n, int step, int alter, int octave, const int octaveShift, const Instrument* const instr)
{
    //LOGD("xmlSetPitch(n=%p, step=%d, alter=%d, octave=%d, octaveShift=%d)",
    //       n, step, alter, octave, octaveShift);

    //const Staff* staff = n->score()->staff(track / VOICES);
    //const Instrument* instr = staff->part()->instr();

    const Interval intval = instr->transpose();

    //LOGD("  staff=%p instr=%p dia=%d chro=%d",
    //       staff, instr, static_cast<int>(intval.diatonic), static_cast<int>(intval.chromatic));

    int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
    pitch += intval.chromatic;   // assume not in concert pitch
    pitch += 12 * octaveShift;   // correct for octave shift
    // ensure sane values
    pitch = std::clamp(pitch, 0, 127);

    int tpc2 = step2tpc(step, AccidentalVal(alter));
    int tpc1 = mu::engraving::transposeTpc(tpc2, intval, true);
    n->setPitch(pitch, tpc1, tpc2);
    //LOGD("  pitch=%d tpc1=%d tpc2=%d", n->pitch(), n->tpc1(), n->tpc2());
}

//---------------------------------------------------------
//   fillGap
//---------------------------------------------------------

/**
 Fill one gap (tstart - tend) in this track in this measure with rest(s).
 */

static void fillGap(Measure* measure, track_idx_t track, const Fraction& tstart, const Fraction& tend)
{
    Fraction ctick = tstart;
    Fraction restLen = tend - tstart;
    // LOGD("\nfillGIFV     fillGap(measure %p track %d tstart %d tend %d) restLen %d len",
    //        measure, track, tstart, tend, restLen);
    // note: as Constants::division (#ticks in a quarter note) equals 480
    // Constants::division / 64 (#ticks in a 256th note) equals 7.5 but is rounded down to 7
    while (restLen > Fraction(1, 256)) {
        Fraction len = restLen;
        TDuration d(DurationType::V_INVALID);
        if (measure->ticks() == restLen) {
            d.setType(DurationType::V_MEASURE);
        } else {
            d.setVal(len.ticks());
        }
        Segment* s = measure->getSegment(SegmentType::ChordRest, tstart);
        Rest* rest = Factory::createRest(s, d);
        rest->setTicks(len);
        rest->setTrack(track);
        rest->setVisible(false);
        s->add(rest);
        len = rest->globalTicks();
        // LOGD(" %d", len);
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
    IF_ASSERT_FAILED(measure) {
        return;
    }
    IF_ASSERT_FAILED(part) {
        return;
    }

    Fraction measTick     = measure->tick();
    Fraction measLen      = measure->ticks();
    Fraction nextMeasTick = measTick + measLen;
    staff_idx_t staffIdx = part->score()->staffIdx(part);
    /*
     LOGD("fillGIFV measure %p part %p idx %d nstaves %d tick %d - %d (len %d)",
     measure, part, staffIdx, part->nstaves(),
     measTick, nextMeasTick, measLen);
     */
    for (staff_idx_t st = 0; st < part->nstaves(); ++st) {
        track_idx_t track = (staffIdx + st) * VOICES;
        Fraction endOfLastCR = measTick;
        for (Segment* s = measure->first(); s; s = s->next()) {
            // LOGD("fillGIFV   segment %p tp %s", s, s->subTypeName());
            EngravingItem* el = s->element(track);
            if (el) {
                // LOGD(" el[%d] %p", track, el);
                if (s->isChordRestType()) {
                    ChordRest* cr  = static_cast<ChordRest*>(el);
                    Fraction crTick     = cr->tick();
                    Fraction crLen      = cr->globalTicks();
                    Fraction nextCrTick = crTick + crLen;
                    /*
                     LOGD(" chord/rest tick %d - %d (len %d)",
                     crTick, nextCrTick, crLen);
                     */
                    if (crTick > endOfLastCR) {
                        /*
                         LOGD(" GAP: track %d tick %d - %d",
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
             LOGD("fillGIFV   measure end GAP: track %d tick %d - %d",
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
        //LOGD("instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
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
          LOGD("GREP_ME '%s',%d,'%s','%s','%s'",
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
// (this is assumed if any instrument has a valid midi-unpitched element,
// which stored in the MusicXMLInstrument pitch field)
// then if the part contains a drumset, Drumset drumset is initialized

static void initDrumset(Drumset* drumset, const MusicXMLInstruments& instruments)
{
    drumset->clear();
    MusicXMLInstrumentsIterator ii(instruments);
    while (ii.hasNext()) {
        ii.next();
        // debug: also dump the drumset for this part
        //LOGD("initDrumset: instrument: %s %s", qPrintable(ii.key()), qPrintable(ii.value().toString()));
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
    for (staff_idx_t j = 0; j < part->nstaves(); ++j) {
        if (part->staff(j)->lines(Fraction(0, 1)) == 5 && !part->staff(j)->isDrumStaff(Fraction(0, 1))) {
            part->staff(j)->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
        }
    }
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
        it = mu::engraving::searchTemplateForMusicXmlId(mxmlInstr.sound);
    }

    if (!it) {
        it = mu::engraving::searchTemplateForInstrNameList({ mxmlInstr.name });
    }

    if (!it) {
        it = mu::engraving::searchTemplateForMidiProgram(0, mxmlInstr.midiProgram);
    }

    if (it) {
        // initialize from template with matching MusicXmlId
        instr = Instrument::fromTemplate(it);
        // reset transpose, as it is determined later from MusicXML data
        instr.setTranspose(Interval());
    } else {
        // set articulations to default (global articulations)
        instr.setArticulation(midiArticulations);
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

static void updatePartWithInstrument(Part* const part, const MusicXMLInstrument& mxmlInstr, const Interval interval,
                                     const bool hasDrumset = false)
{
    Instrument instr = createInstrument(mxmlInstr, interval);
    if (hasDrumset) {
        instr.channel(0)->setBank(128);
    }
    part->setInstrument(instr);
    if (mxmlInstr.midiChannel >= 0) {
        part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort);
    }
    // note: setMidiProgram() does more than simply setting the MIDI program
    if (mxmlInstr.midiProgram >= 0) {
        part->setMidiProgram(mxmlInstr.midiProgram);
    }
}

//---------------------------------------------------------
//   createInstrumentChange
//---------------------------------------------------------

/**
 Create an InstrumentChange based on the information in \a mxmlInstr.
 */

static InstrumentChange* createInstrumentChange(Score* score, const MusicXMLInstrument& mxmlInstr, const Interval interval,
                                                const track_idx_t track)
{
    const Instrument instr = createInstrument(mxmlInstr, interval);
    InstrumentChange* instrChange = Factory::createInstrumentChange(score->dummy()->segment(), instr);
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

static void updatePartWithInstrumentChange(Part* const part, const MusicXMLInstrument& mxmlInstr, const Interval interval,
                                           Segment* const segment, const track_idx_t track, const Fraction tick)
{
    const auto ic = createInstrumentChange(part->score(), mxmlInstr, interval, track);
    segment->add(ic);               // note: includes part::setInstrument(instr);

    // setMidiChannel() depends on setInstrument() already been done
    if (mxmlInstr.midiChannel >= 0) {
        part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort, tick);
    }
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
        //LOGD("no instrument details");
        updatePartWithInstrument(part, {}, intervList.interval({ 0, 1 }));
        return;
    }

    if (hasDrumset(instruments)) {
        // do not create multiple instruments for a drum part
        //LOGD("hasDrumset");
        MusicXMLInstrument mxmlInstr = instruments.first();
        updatePartWithInstrument(part, mxmlInstr, {}, true);
        return;
    }

    if (instrList.empty()) {
        // instrument details found, but no instrument ids found
        // -> only a single instrument is playing in the part
        //LOGD("single instrument");
        MusicXMLInstrument mxmlInstr = instruments.first();
        updatePartWithInstrument(part, mxmlInstr, intervList.interval({ 0, 1 }));
        return;
    }

    // either a single instrument is playing, or forwards / rests resulted in gaps in the instrument map
    // (and thus multiple entries)
    //LOGD("possibly multiple instruments");
    QString prevInstrId;
    for (auto it = instrList.cbegin(); it != instrList.cend(); ++it) {
        Fraction tick = (*it).first;
        if (it == instrList.cbegin()) {
            prevInstrId = (*it).second;              // first instrument id
            MusicXMLInstrument mxmlInstr = instruments.value(prevInstrId);
            updatePartWithInstrument(part, mxmlInstr, intervList.interval(tick));
        } else {
            auto instrId = (*it).second;
            bool mustInsert = instrId != prevInstrId;
            /*
             LOGD("tick %s previd %s id %s mustInsert %d",
             qPrintable(tick.print()),
             qPrintable(prevInstrId),
             qPrintable(instrId),
             mustInsert);
             */
            if (mustInsert) {
                const staff_idx_t staff = score->staffIdx(part);
                const track_idx_t track = staff * VOICES;
                //LOGD("instrument change: tick %s (%d) track %d instr '%s'",
                //       qPrintable(tick.print()), tick.ticks(), track, qPrintable(instrId));

                Measure* const m = score->tick2measure(tick);
                Segment* const segment = m->getSegment(SegmentType::ChordRest, tick);

                if (!segment) {
                    logger->logError(QString("segment for instrument change at tick %1 not found")
                                     .arg(tick.ticks()), xmlreader);
                } else if (!instruments.contains(instrId)) {
                    logger->logError(QString("changed instrument '%1' at tick %2 not found in part '%3'")
                                     .arg(instrId).arg(tick.ticks()).arg(partId), xmlreader);
                } else {
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
namespace xmlpass2 {
static QString text2syms(const QString& t)
{
    //QTime time;
    //time.start();

    // first create a map from symbol (Unicode) text to symId
    // note that this takes about 1 msec on a Core i5,
    // caching does not gain much

    IEngravingFontPtr sf = engravingFonts()->fallbackFont();
    QMap<QString, SymId> map;
    int maxStringSize = 0;          // maximum string size found

    for (int i = int(SymId::noSym); i < int(SymId::lastSym); ++i) {
        SymId id((SymId(i)));
        QString string(sf->toString(id));
        // insert all syms except space to prevent matching all regular spaces
        if (id != SymId::space) {
            map.insert(string, id);
        }
        if (string.size() > maxStringSize) {
            maxStringSize = string.size();
        }
    }

    // Special case Dolet inference (TODO: put behind a setting or export type flag)
    map.insert("$", SymId::segno);
    map.insert("Ø", SymId::coda);

    //LOGD("text2syms map count %d maxsz %d filling time elapsed: %d ms",
    //       map.size(), maxStringSize, time.elapsed());

    // then look for matches
    QString in = t;
    QString res;

    while (in != "") {
        // try to find the largest match possible
        int maxMatch = qMin(in.size(), maxStringSize);
        AsciiStringView sym;
        while (maxMatch > 0) {
            QString toBeMatched = in.left(maxMatch);
            if (map.contains(toBeMatched)) {
                sym = SymNames::nameForSymId(map.value(toBeMatched));
                break;
            }
            maxMatch--;
        }
        if (maxMatch > 0) {
            // found a match, add sym to res and remove match from string in
            res += "<sym>";
            res += sym.ascii();
            res += "</sym>";
            in.remove(0, maxMatch);
        } else {
            // not found, move one char from res to in
            res += in.leftRef(1);
            in.remove(0, 1);
        }
    }

    //LOGD("text2syms total time elapsed: %d ms, res '%s'", time.elapsed(), qPrintable(res));
    return res;
}
}

//---------------------------------------------------------
//   decodeEntities
//---------------------------------------------------------

/**
 Decode &#...; in string \a src into UNICODE (utf8) character.
 */

namespace xmlpass2 {
static QString decodeEntities(const QString& src)
{
    QString ret(src);
    QRegularExpression re("&#([0-9]+);", QRegularExpression::InvertedGreedinessOption);

    int pos = 0;
    QRegularExpressionMatch match;
    while ((pos = src.indexOf(re, pos, &match)) != -1) {
        ret = ret.replace(match.capturedTexts()[0], QChar(match.capturedTexts()[1].toInt(0, 10)));
        pos += match.capturedLength();
    }
    return ret;
}
}

//---------------------------------------------------------
//   nextPartOfFormattedString
//---------------------------------------------------------

// TODO: probably should be shared between pass 1 and 2

/**
 Read the next part of a MusicXML formatted string and convert to MuseScore internal encoding.
 */

namespace xmlpass2 {
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
    txt = xmlpass2::decodeEntities(txt);
    QString syms       = xmlpass2::text2syms(txt);

    QString importedtext;

    if (!fontSize.isEmpty()) {
        bool ok = true;
        float size = fontSize.toFloat(&ok);
        if (ok) {
            importedtext += QString("<font size=\"%1\"/>").arg(size);
        }
    }

    bool needUseDefaultFont = configuration()->needUseDefaultFont();
    if (!fontFamily.isEmpty() && txt == syms && !needUseDefaultFont) {
        // add font family only if no <sym> replacement made
        importedtext += QString("<font face=\"%1\"/>").arg(fontFamily);
    }
    if (fontWeight == "bold") {
        importedtext += "<b>";
    }
    if (fontStyle == "italic") {
        importedtext += "<i>";
    }
    if (!underline.isEmpty()) {
        bool ok = true;
        int lines = underline.toInt(&ok);
        if (ok && (lines > 0)) {    // 1, 2, or 3 underlines are imported as single underline
            importedtext += "<u>";
        } else {
            underline = "";
        }
    }
    if (!strike.isEmpty()) {
        bool ok = true;
        int lines = strike.toInt(&ok);
        if (ok && (lines > 0)) {    // 1, 2, or 3 strikes are imported as single strike
            importedtext += "<s>";
        } else {
            strike = "";
        }
    }
    if (txt == syms) {
        txt.replace(QString("\r"), QString(""));     // convert Windows line break \r\n -> \n
        importedtext += txt.toHtmlEscaped();
    } else {
        // <sym> replacement made, should be no need for line break or other conversions
        importedtext += syms;
    }
    if (strike != "") {
        importedtext += "</s>";
    }
    if (underline != "") {
        importedtext += "</u>";
    }
    if (fontStyle == "italic") {
        importedtext += "</i>";
    }
    if (fontWeight == "bold") {
        importedtext += "</b>";
    }
    //LOGD("importedtext '%s'", qPrintable(importedtext));
    return importedtext;
}
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
    } else {
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
                      const QMap<int, Lyrics*>& numbrdLyrics,
                      const QSet<Lyrics*>& extLyrics,
                      MusicXmlLyricsExtend& extendedLyrics)
{
    for (const auto lyricNo : numbrdLyrics.keys()) {
        const auto lyric = numbrdLyrics.value(lyricNo);
        addLyric(logger, xmlreader, cr, lyric, lyricNo, extendedLyrics);
        if (extLyrics.contains(lyric)) {
            extendedLyrics.addLyric(lyric);
        }
    }
}

//---------------------------------------------------------
//   addElemOffset
//---------------------------------------------------------

static void addElemOffset(EngravingItem* el, track_idx_t track, const QString& placement, Measure* measure, const Fraction& tick)
{
    if (!measure) {
        return;
    }

    if (placement != "") {
        el->setPlacement(placement == "above" ? PlacementV::ABOVE : PlacementV::BELOW);
        el->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    }

    el->setTrack(el->isTempoText() ? 0 : track);      // TempoText must be in track 0
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

    foreach (DurationElement* de, t->elements()) {
        if (de->type() == ElementType::CHORD || de->type() == ElementType::REST) {
            const auto cr = static_cast<ChordRest*>(de);
            const auto fraction = cr->ticks(); // TODO : take care of nested tuplets
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

    auto baseLen = tupletFullDuration * Fraction(1, t->ratio().denominator());
    /*
    LOGD("tupletFraction %s tupletFullDuration %s ratio %s baseLen %s",
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
    tuplet = new Tuplet(cr->measure());
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
    if (!tuplet) {
        return;
    }

    // set baselen
    TDuration td = determineTupletBaseLen(tuplet);
    tuplet->setBaseLen(td);
    Fraction f(normalNotes, td.fraction().denominator());
    f.reduce();
    tuplet->setTicks(f);
    // TODO determine usefulness of following check
    int totalDuration = 0;
    int ticksPerNote = f.ticks() / tuplet->ratio().numerator();
    bool ticksCorrect = true;
    for (DurationElement* de : tuplet->elements()) {
        if (de->type() == ElementType::CHORD || de->type() == ElementType::REST) {
            int globalTicks = de->globalTicks().ticks();
            if (globalTicks != ticksPerNote) {
                ticksCorrect = false;
            }
            totalDuration += globalTicks;
        }
    }
    if (totalDuration != f.ticks()) {
        LOGD("MusicXML::import: tuplet stop but bad duration");     // TODO
    }
    if (!ticksCorrect) {
        qDebug("MusicXML::import: tuplet stop but uneven note ticks"); // TODO
    }
    tuplet = 0;
}

//---------------------------------------------------------
//   setElementPropertyFlags
//---------------------------------------------------------

static void setElementPropertyFlags(EngravingObject* element, const Pid propertyId,
                                    const QString value1, const QString value2 = QString())
{
    if (value1.isEmpty()) { // Set as an implicit value
        element->setPropertyFlags(propertyId, PropertyFlags::STYLED);
    } else if (!value1.isNull() || !value2.isNull()) { // Set as an explicit value
        element->setPropertyFlags(propertyId, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   addArticulationToChord
//---------------------------------------------------------

static void addArticulationToChord(const Notation& notation, ChordRest* cr)
{
    const SymId articSym = notation.symId();
    const QString dir = notation.attribute("type");
    const QString place = notation.attribute("placement");
    const QColor color { notation.attribute("color") };
    Articulation* na = Factory::createArticulation(cr);
    na->setSymId(articSym);
    if (color.isValid()) {
        na->setColor(color);
    }

    if (dir == "up" || dir == "down") {
        na->setUp(dir == "up");
        na->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
    }

    // when setting anchor, assume type up/down without explicit placement
    // implies placement above/below
    if (place == "above" || (dir == "up" && place == "")) {
        na->setAnchor(ArticulationAnchor::TOP);
        na->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
    } else if (place == "below" || (dir == "down" && place == "")) {
        na->setAnchor(ArticulationAnchor::BOTTOM);
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
    const QColor color { notation.attribute("color") };
    Fermata* na = Factory::createFermata(cr);
    na->setSymIdAndTimeStretch(articSym);
    na->setTrack(cr->track());
    if (color.isValid()) {
        na->setColor(color);
    }
    if (!direction.isNull()) { // Only for case where XML attribute is present (isEmpty wouldn't work)
        na->setPlacement(direction == "inverted" ? PlacementV::BELOW : PlacementV::ABOVE);
    }
    setElementPropertyFlags(na, Pid::PLACEMENT, direction);
    if (cr->segment() == nullptr && cr->isGrace()) {
        cr->addFermata(na);           // store for later move to segment
    } else {
        cr->segment()->add(na);
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
    SymId articSym = SymId::noSym;   // legal but impossible ArticulationType value here indicating "not found"
    if (name == "inverted-mordent") {
        if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "") {
            articSym = SymId::ornamentShortTrill;
        } else if (attrLong == "yes" && attrAppr == "" && attrDep == "") {
            articSym = SymId::ornamentTremblement;
        } else if (attrLong == "yes" && attrAppr == "below" && attrDep == "") {
            articSym = SymId::ornamentUpPrall;
        } else if (attrLong == "yes" && attrAppr == "above" && attrDep == "") {
            articSym = SymId::ornamentPrecompMordentUpperPrefix;
        } else if (attrLong == "yes" && attrAppr == "" && attrDep == "below") {
            articSym = SymId::ornamentPrallDown;
        } else if (attrLong == "yes" && attrAppr == "" && attrDep == "above") {
            articSym = SymId::ornamentPrallUp;
        }
    } else if (name == "mordent") {
        if ((attrLong == "" || attrLong == "no") && attrAppr == "" && attrDep == "") {
            articSym = SymId::ornamentMordent;
        } else if (attrLong == "yes" && attrAppr == "" && attrDep == "") {
            articSym = SymId::ornamentPrallMordent;
        } else if (attrLong == "yes" && attrAppr == "below" && attrDep == "") {
            articSym = SymId::ornamentUpMordent;
        } else if (attrLong == "yes" && attrAppr == "above" && attrDep == "") {
            articSym = SymId::ornamentDownMordent;
        }
    }
    if (articSym != SymId::noSym) {
        const QColor color { notation.attribute("color") };
        Articulation* na = Factory::createArticulation(cr);
        na->setSymId(articSym);
        if (color.isValid()) {
            na->setColor(color);
        }
        cr->add(na);
    } else {
        LOGD("unknown ornament: name '%s' long '%s' approach '%s' departure '%s'",
             qPrintable(name), qPrintable(attrLong), qPrintable(attrAppr), qPrintable(attrDep));        // TODO
    }
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
    sym = SymNames::symIdByName(symname);

    if (sym != SymId::noSym) {
        const QColor color { notation.attribute("color") };
        Articulation* na = Factory::createArticulation(cr);
        na->setSymId(sym);
        if (color.isValid()) {
            na->setColor(color);
        }
        cr->add(na);
    } else {
        LOGD("unknown ornament: name '%s': '%s'.", qPrintable(name), qPrintable(symname));
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
    QMap<QString, SymId> map;         // map MusicXML articulation name to MuseScore symbol
    map["accent"]           = SymId::articAccentAbove;
    map["staccatissimo"]    = SymId::articStaccatissimoAbove;
    map["staccato"]         = SymId::articStaccatoAbove;
    map["tenuto"]           = SymId::articTenutoAbove;
    map["strong-accent"]    = SymId::articMarcatoAbove;
    map["delayed-turn"]     = SymId::ornamentTurn;
    map["turn"]             = SymId::ornamentTurn;
    map["inverted-turn"]    = SymId::ornamentTurnInverted;
    map["stopped"]          = SymId::brassMuteClosed;
    map["up-bow"]           = SymId::stringsUpBow;
    map["down-bow"]         = SymId::stringsDownBow;
    map["detached-legato"]  = SymId::articTenutoStaccatoAbove;
    map["spiccato"]         = SymId::articStaccatissimoAbove;
    map["snap-pizzicato"]   = SymId::pluckedSnapPizzicatoAbove;
    map["schleifer"]        = SymId::ornamentPrecompSlide;
    map["open"]             = SymId::brassMuteOpen;
    map["open-string"]      = SymId::brassMuteOpen;
    map["thumb-position"]   = SymId::stringsThumbPosition;
    map["soft-accent"]      = SymId::articSoftAccentAbove;
    map["stress"]           = SymId::articStressAbove;
    map["unstress"]         = SymId::articUnstressAbove;

    if (map.contains(mxmlName)) {
        id = map.value(mxmlName);
        return true;
    } else {
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

    if (map.contains(mxmlName)) {
        return map.value(mxmlName);
    } else {
        LOGD("unknown fermata %s", qPrintable(mxmlName));
    }
    return SymId::fermataAbove;
}

//---------------------------------------------------------
//   convertNotehead
//---------------------------------------------------------

/**
 Convert a MusicXML notehead name to a MuseScore headgroup.
 */

static NoteHeadGroup convertNotehead(QString mxmlName)
{
    QMap<QString, int> map;   // map MusicXML notehead name to a MuseScore headgroup
    map["slash"] = int(NoteHeadGroup::HEAD_SLASH);
    map["triangle"] = int(NoteHeadGroup::HEAD_TRIANGLE_UP);
    map["diamond"] = int(NoteHeadGroup::HEAD_DIAMOND);
    map["cross"] = int(NoteHeadGroup::HEAD_PLUS);
    map["x"] = int(NoteHeadGroup::HEAD_CROSS);
    map["circle-x"] = int(NoteHeadGroup::HEAD_XCIRCLE);
    map["inverted triangle"] = int(NoteHeadGroup::HEAD_TRIANGLE_DOWN);
    map["slashed"] = int(NoteHeadGroup::HEAD_SLASHED1);
    map["back slashed"] = int(NoteHeadGroup::HEAD_SLASHED2);
    map["normal"] = int(NoteHeadGroup::HEAD_NORMAL);
    map["do"] = int(NoteHeadGroup::HEAD_DO);
    map["re"] = int(NoteHeadGroup::HEAD_RE);
    map["mi"] = int(NoteHeadGroup::HEAD_MI);
    map["fa"] = int(NoteHeadGroup::HEAD_FA);
    map["fa up"] = int(NoteHeadGroup::HEAD_FA);
    map["so"] = int(NoteHeadGroup::HEAD_SOL);
    map["la"] = int(NoteHeadGroup::HEAD_LA);
    map["ti"] = int(NoteHeadGroup::HEAD_TI);

    if (map.contains(mxmlName)) {
        return NoteHeadGroup(map.value(mxmlName));
    } else {
        LOGD("unknown notehead %s", qPrintable(mxmlName));      // TODO
    }
    // default: return 0
    return NoteHeadGroup::HEAD_NORMAL;
}

//---------------------------------------------------------
//   addTextToNote
//---------------------------------------------------------

/**
 Add Text to Note.
 */

static void addTextToNote(int l, int c, QString txt, QString placement, QString fontWeight,
                          qreal fontSize, QString fontStyle, QString fontFamily, TextStyleType subType, Score*, Note* note)
{
    if (note) {
        if (!txt.isEmpty()) {
            TextBase* t = Factory::createFingering(note, subType);
            t->setPlainText(txt);
            bool needUseDefaultFont = configuration()->needUseDefaultFont();
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
                t->setPlacement(placement == "below" ? PlacementV::BELOW : PlacementV::ABOVE);
                t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
            }
            note->add(t);
        }
    } else {
        LOGD("%s", qPrintable(QString("Error at line %1 col %2: no note for text").arg(l).arg(c)));           // TODO
    }
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
    sli->setPlacement(placement == "above" ? PlacementV::ABOVE : PlacementV::BELOW);
    sli->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
}

//---------------------------------------------------------
//   handleSpannerStart
//---------------------------------------------------------

// note that in case of overlapping spanners, handleSpannerStart is called for every spanner
// as spanners QMap allows only one value per key, this does not hurt at all

static void handleSpannerStart(SLine* new_sp, track_idx_t track, QString& placement, const Fraction& tick, MusicXmlSpannerMap& spanners)
{
    new_sp->setTrack(track);
    setSLinePlacement(new_sp, placement);
    spanners[new_sp] = QPair<int, int>(tick.ticks(), -1);
}

//---------------------------------------------------------
//   handleSpannerStop
//---------------------------------------------------------

static void handleSpannerStop(SLine* cur_sp, track_idx_t track2, const Fraction& tick, MusicXmlSpannerMap& spanners)
{
    //LOGD("handleSpannerStop(sp %p, track2 %d, tick %s (%d))", cur_sp, track2, qPrintable(tick.print()), tick.ticks());
    if (!cur_sp) {
        return;
    }

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
    if (error != "") {
        _logger->logError(error, &_e);
        _errors += errorStringWithLocation(_e.lineNumber(), _e.columnNumber(), error) + '\n';
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
    if (duration.type() == DurationType::V_MEASURE) {
        cr->setDurationType(duration);
        cr->setTicks(dura);
    } else {
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

static Rest* addRest(Score*, Measure* m,
                     const Fraction& tick, const track_idx_t track, const int move,
                     const TDuration duration, const Fraction dura)
{
    Segment* s = m->getSegment(SegmentType::ChordRest, tick);
    // Sibelius might export two rests at the same place, ignore the 2nd one
    // <?DoletSibelius Two NoteRests in same voice at same position may be an error?>
    // Same issue may result from trying to import incomplete tuplets
    if (s->element(track)) {
        LOGD("cannot add rest at tick %s (%d) track %zu: element already present", qPrintable(tick.toString()), tick.ticks(), track); // TODO
        return nullptr;
    }

    Rest* cr = Factory::createRest(s);
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
        auto tuplet = pair.second;
        if (tuplet) {
            const auto actualDuration = tuplet->elementsDuration() / tuplet->ratio();
            const auto missingDuration = missingTupletDuration(actualDuration);
            LOGD("tuplet %p not stopped at end of measure, tick %s duration %s missing %s",
                 tuplet,
                 qPrintable(tuplet->tick().toString()),
                 qPrintable(actualDuration.toString()), qPrintable(missingDuration.toString()));
            if (actualDuration > Fraction(0, 1) && missingDuration > Fraction(0, 1)) {
                LOGD("add missing %s to previous tuplet", qPrintable(missingDuration.toString()));
                const auto& firstElement = tuplet->elements().at(0);
                // appended the rest to the current end of the tuplet (firstElement->tick() + actualDuration)
                const auto extraRest = addRest(firstElement->score(), firstElement->measure(),
                                               firstElement->tick() + actualDuration, firstElement->track(), 0,
                                               TDuration { missingDuration* tuplet->ratio() }, missingDuration);
                if (extraRest) {
                    extraRest->setTuplet(tuplet);
                    tuplet->add(extraRest);
                }
            }
            const auto normalNotes = tuplet->ratio().denominator();
            handleTupletStop(tuplet, normalNotes);
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
    _timeSigDura = Fraction(0, 0);               // invalid
    _tie    = 0;
    _lastVolta = 0;
    _hasDrumset = false;
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        _slurs[i] = SlurDesc();
    }
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        _trills[i] = 0;
    }
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        _glissandi[i][0] = _glissandi[i][1] = 0;
    }
    _pedalContinue = 0;
    _harmony = 0;
    _tremStart = 0;
    _figBass = 0;
    _multiMeasureRestCount = -1;
    _measureStyleSlash = MusicXmlSlash::NONE;
    _extendedLyrics.init();

    _nstaves = _pass1.getPart(partId)->nstaves();
    _measureRepeatNumMeasures.assign(_nstaves, 0);
    _measureRepeatCount.assign(_nstaves, 0);
}

//---------------------------------------------------------
//   findIncompleteSpannersInStack
//---------------------------------------------------------

static void findIncompleteSpannersInStack(const QString& spannerType, SpannerStack& stack, SpannerSet& res)
{
    for (auto& desc : stack) {
        if (desc._sp) {
            LOGD("%s not terminated at end of part", qPrintable(spannerType));
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
        LOGD("pedal not terminated at end of part");
        res.insert(_pedal._sp);
        _pedal = {};
    }
    return res;
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
    return partName.contains(QRegularExpression("^P[0-9]+$"));
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
    if (_multiMeasureRestCount >= 0) {
        _multiMeasureRestCount--;
    }
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

Err MusicXMLParserPass2::parse(QIODevice* device)
{
    //LOGD("MusicXMLParserPass2::parse()");
    _e.setDevice(device);
    Err res = parse();
    //LOGD("MusicXMLParserPass2::parse() res %d", int(res));
    return res;
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Start the parsing process, after verifying the top-level node is score-partwise
 */

Err MusicXMLParserPass2::parse()
{
    bool found = false;
    while (_e.readNextStartElement()) {
        if (_e.name() == "score-partwise") {
            found = true;
            scorePartwise();
        } else {
            _logger->logError("this is not a MusicXML score-partwise file", &_e);
            _e.skipCurrentElement();
            return Err::FileBadFormat;
        }
    }

    if (!found) {
        _logger->logError("this is not a MusicXML score-partwise file", &_e);
        return Err::FileBadFormat;
    }

    return Err::NoError;
}

//---------------------------------------------------------
//   createBarline
//---------------------------------------------------------

/*
 * Create a barline of the specified type.
 */

static std::unique_ptr<BarLine> createBarline(Score* score, const track_idx_t track, const BarLineType type, const bool visible,
                                              const QString& barStyle, int spanStaff)
{
    std::unique_ptr<BarLine> barline(Factory::createBarLine(score->dummy()->segment()));
    barline->setTrack(track);
    barline->setBarLineType(type);
    barline->setSpanStaff(spanStaff);
    barline->setVisible(visible);
    if (barStyle == "tick") {
        barline->setSpanFrom(BARLINE_SPAN_TICK1_FROM);
        barline->setSpanTo(BARLINE_SPAN_TICK1_TO);
    } else if (barStyle == "short") {
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
    auto st = SegmentType::BarLine;
    if (tick == measure->endTick()) {
        st = SegmentType::EndBarLine;
    } else if (tick == measure->tick()) {
        st = SegmentType::BeginBarLine;
    }
    const auto segment = measure->getSegment(st, tick);
    EngravingItem::renderer()->layoutItem(barline.get());
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
        } else if (_e.name() == "part-list") {
            partList();
        } else {
            skipLogCurrElem();
        }
    }
    // set last measure barline to normal or MuseScore will generate light-heavy EndBarline
    // this creates non-generated barlines spanning only the current instrument
    // BarLine::_spanStaff is set using the default in Staff::_barLineSpan
    auto lm = _score->lastMeasure();
    if (lm && lm->endBarLineType() == BarLineType::NORMAL) {
        for (staff_idx_t staffidx = 0; staffidx < _score->nstaves(); ++staffidx) {
            auto staff = _score->staff(staffidx);
            auto b = createBarline(_score, staffidx * VOICES, BarLineType::NORMAL, true, "", staff->barLineSpan());
            addBarlineToMeasure(lm, lm->endTick(), std::move(b));
        }
    }

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
        if (_e.name() == "score-part") {
            scorePart();
        } else {
            skipLogCurrElem();
        }
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
        if (_e.name() == "midi-instrument") {
            _e.skipCurrentElement();        // skip but don't log
        } else if (_e.name() == "score-instrument") {
            _e.skipCurrentElement();        // skip but don't log
        } else if (_e.name() == "part-name") {
            _e.skipCurrentElement();        // skip but don't log
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   createSegmentChordRest
//---------------------------------------------------------

static void createSegmentChordRest(Score* score, Fraction tick)
{
    // getSegment() creates the segment if it does not yet exist
    const auto measure = score->tick2measure(tick);
    measure->getSegment(SegmentType::ChordRest, tick);
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
        return;
    }

    initPartState(id);

    const auto& instruments = _pass1.getInstruments(id);
    _hasDrumset = hasDrumset(instruments);

    // set the parts first instrument
    Part* part = _pass1.getPart(id);
    setPartInstruments(_logger, &_e, part, id, _score, _pass1.getInstrList(id), _pass1.getIntervals(id), instruments);

    // set the part name
    auto mxmlPart = _pass1.getMusicXmlPart(id);
    part->setPartName(mxmlPart.getName());
    if (mxmlPart.getPrintName() && !isLikelyIncorrectPartName(mxmlPart.getName())) {
        part->setLongNameAll(mxmlPart.getName());
    } else {
        _pass1.getPart(id)->setLongNameAll(u"");
    }
    if (mxmlPart.getPrintAbbr()) {
        part->setPlainShortNameAll(mxmlPart.getAbbr());
    } else {
        _pass1.getPart(id)->setPlainShortNameAll(u"");
    }
    // try to prevent an empty track name
    if (part->partName() == "") {
        QString instrId = _pass1.getInstrList(id).instrument(Fraction(0, 1));
        part->setPartName(instruments[instrId].name);
    }

#ifdef DEBUG_VOICE_MAPPER
    VoiceList voicelist = _pass1.getVoiceList(id);
    // debug: print voice mapper contents
    LOGD("voiceMapperStats: part '%s'", qPrintable(id));
    for (QMap<QString, mu::engraving::VoiceDesc>::const_iterator i = voicelist.constBegin(); i != voicelist.constEnd(); ++i) {
        LOGD("voiceMapperStats: voice %s staff data %s",
             qPrintable(i.key()), qPrintable(i.value().toString()));
    }
#endif

    // read the measures
    int nr = 0; // current measure sequence number (always increments by one for each measure)
    _measureNumber = 0; // written measure number (doesn't always increment by 1)
    while (_e.readNextStartElement()) {
        if (_e.name() == "measure") {
            Fraction t = _pass1.getMeasureStart(nr);
            if (t.isValid()) {
                measure(id, t);
            } else {
                _logger->logError(QString("no valid start time for measure %1").arg(nr + 1), &_e);
                _e.skipCurrentElement();
            }
            ++nr;
        } else {
            skipLogCurrElem();
        }
    }

    // stop all remaining extends for this part
    Measure* lm = part->score()->lastMeasure();
    if (lm) {
        track_idx_t strack = _pass1.trackForPart(id);
        track_idx_t etrack = strack + part->nstaves() * VOICES;
        Fraction lastTick = lm->endTick();
        for (track_idx_t trk = strack; trk < etrack; trk++) {
            _extendedLyrics.setExtend(-1, trk, lastTick);
        }
    }

    const auto incompleteSpanners =  findIncompleteSpannersAtPartEnd();
    //LOGD("spanner list:");
    auto i = _spanners.constBegin();
    while (i != _spanners.constEnd()) {
        auto sp = i.key();
        Fraction tick1 = Fraction::fromTicks(i.value().first);
        Fraction tick2 = Fraction::fromTicks(i.value().second);
        if (sp->isPedal() && toPedal(sp)->endHookType() == HookType::HOOK_45) {
            // Handle pedal change end tick (slightly hacky)
            tick2 += _score->findCR(tick2, sp->track())->ticks();
        }
        //LOGD("spanner %p tp %d isHairpin %d tick1 %s tick2 %s track1 %d track2 %d start %p end %p",
        //       sp, sp->type(), sp->isHairpin(), qPrintable(tick1.toString()), qPrintable(tick2.toString()),
        //       sp->track(), sp->track2(), sp->startElement(), sp->endElement());
        if (incompleteSpanners.find(sp) == incompleteSpanners.end()) {
            // complete spanner found
            // PlaybackContext::handleSpanners() requires hairpins to have a valid start segment
            // always create start segment to prevent crash in case no note starts at this tick
            if (sp->isHairpin()) {
                createSegmentChordRest(_score, tick1);
            }
            // add to score
            sp->setTick(tick1);
            sp->setTick2(tick2);
            sp->score()->addElement(sp);
        } else {
            // incomplete spanner -> cleanup
            delete sp;
        }
        ++i;
    }
    _spanners.clear();

    if (_hasDrumset) {
        Drumset* drumset = new Drumset;
        const auto& instrumentsAfterPass2 = _pass1.getInstruments(id);
        initDrumset(drumset, instrumentsAfterPass2);
        // set staff type to percussion if incorrectly imported as pitched staff
        // Note: part has been read, staff type already set based on clef type and staff-details
        // but may be incorrect for a percussion staff that does not use a percussion clef
        setStaffTypePercussion(part, drumset);
    }

    bool showPart = false;
    for (Staff* staff : part->staves()) {
        if (staff->visible()) {
            showPart = true;
            break;
        }
    }

    part->setShow(showPart);

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
        if (m->tick() == tick) {
            return m;
        }
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
    for (size_t i = 0; i < beam->elements().size(); ++i) {
        beam->elements().at(i)->setBeamMode(BeamMode::NONE);
    }
    delete beam;
    beam = nullptr;
}

//---------------------------------------------------------
//   handleBeamAndStemDir
//---------------------------------------------------------

static void handleBeamAndStemDir(ChordRest* cr, const BeamMode bm, const DirectionV sd, Beam*& beam, bool hasBeamingInfo)
{
    if (!cr) {
        return;
    }
    // create a new beam
    if (bm == BeamMode::BEGIN) {
        // if currently in a beam, delete it
        if (beam) {
            LOGD("handleBeamAndStemDir() new beam, removing previous incomplete beam %p", beam);
            removeBeam(beam);
        }
        // create a new beam
        beam = Factory::createBeam(cr->score()->dummy()->system());
        beam->setTrack(cr->track());
        beam->setBeamDirection(sd);
    }
    // add ChordRest to beam
    if (beam) {
        // verify still in the same track (if still in the same voice)
        // and in a beam ...
        // (note no check is done on correct order of beam begin/continue/end)
        // TODO: Some BEGINs are being skipped
        if (cr->track() != beam->track()) {
            LOGD("handleBeamAndStemDir() from track %zu to track %zu -> abort beam",
                 beam->track(), cr->track());
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else if (bm == BeamMode::NONE) {
            LOGD("handleBeamAndStemDir() in beam, bm BeamMode::NONE -> abort beam");
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else if (!(bm == BeamMode::BEGIN || bm == BeamMode::MID || bm == BeamMode::END || bm == BeamMode::BEGIN16
                     || bm == BeamMode::BEGIN32)) {
            LOGD("handleBeamAndStemDir() in beam, bm %d -> abort beam", static_cast<int>(bm));
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else {
            // actually add cr to the beam
            beam->add(cr);
        }
    }
    // if no beam, set stem direction on chord itself
    if (!beam) {
        static_cast<Chord*>(cr)->setStemDirection(sd);
        // set beam to none if score has beaming information and note can get beam, otherwise
        // set to auto
        bool canGetBeam = (cr->durationType().type() >= DurationType::V_EIGHTH
                           && cr->durationType().type() <= DurationType::V_1024TH);
        if (hasBeamingInfo && canGetBeam) {
            cr->setBeamMode(BeamMode::NONE);
        } else {
            cr->setBeamMode(BeamMode::AUTO);
        }
    }
    // terminate the current beam and add to the score
    if (beam && bm == BeamMode::END) {
        beam = nullptr;
    }
}

//---------------------------------------------------------
//   markUserAccidentals
//---------------------------------------------------------

/**
 Check for "superfluous" accidentals to mark them as USER accidentals.
 The candidate map alterMap is ordered on note address. Check it here segment after segment.
 */

static void markUserAccidentals(const staff_idx_t firstStaff,
                                const size_t staves,
                                const Key key,
                                const Measure* measure,
                                const QMap<Note*, int>& alterMap)
{
    QMap<int, bool> accTmp;

    AccidentalState currAcc;
    currAcc.init(key);
    SegmentType st = SegmentType::ChordRest;
    for (mu::engraving::Segment* segment = measure->first(st); segment; segment = segment->next(st)) {
        for (track_idx_t track = 0; track < staves * VOICES; ++track) {
            EngravingItem* e = segment->element(firstStaff * VOICES + track);
            if (!e || e->type() != mu::engraving::ElementType::CHORD) {
                continue;
            }
            Chord* chord = static_cast<Chord*>(e);
            foreach (Note* nt, chord->notes()) {
                if (alterMap.contains(nt)) {
                    int alter = alterMap.value(nt);
                    int ln  = absStep(nt->tpc(), nt->pitch());
                    bool error = false;
                    AccidentalVal currAccVal = currAcc.accidentalVal(ln, error);
                    if (error) {
                        continue;
                    }
                    if ((alter == -1
                         && currAccVal == AccidentalVal::FLAT
                         && nt->accidental()->accidentalType() == AccidentalType::FLAT
                         && !accTmp.value(ln, false))
                        || (alter == 0
                            && currAccVal == AccidentalVal::NATURAL
                            && nt->accidental()->accidentalType() == AccidentalType::NATURAL
                            && !accTmp.value(ln, false))
                        || (alter == 1
                            && currAccVal == AccidentalVal::SHARP
                            && nt->accidental()->accidentalType() == AccidentalType::SHARP
                            && !accTmp.value(ln, false))) {
                        nt->accidental()->setRole(AccidentalRole::USER);
                    } else if (Accidental::isMicrotonal(nt->accidental()->accidentalType())
                               && nt->accidental()->accidentalType() < AccidentalType::END) {
                        // microtonal accidental
                        nt->accidental()->setRole(AccidentalRole::USER);
                        accTmp.insert(ln, false);
                    } else {
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
    if (mainChord->isSmall()) {
        graceChord->setSmall(true);
    }
    bool anyPlays = false;
    for (auto n : mainChord->notes()) {
        anyPlays |= n->play();
    }
    if (!anyPlays) {
        for (auto gn : graceChord->notes()) {
            gn->setPlay(false);
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
    if (!c) {
        return;
    }

    while (gac > 0) {
        if (gcl.size() > 0) {
            Chord* graceChord = gcl.first();
            gcl.removeFirst();
            graceChord->toGraceAfter();
            c->add(graceChord);              // TODO check if same voice ?
            coerceGraceCue(c, graceChord);
            LOGD("addGraceChordsAfter chord %p grace after chord %p", c, graceChord);
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
        for (EngravingItem* e : gc->el()) {
            if (e->isFermata()) {
                c->segment()->add(e);
                gc->removeFermata(toFermata(e));
                break;                          // out of the door, line on the left, one cross each
            }
        }
        c->add(gc);            // TODO check if same voice ?
        coerceGraceCue(c, gc);
    }
    gcl.clear();
}

static bool canAddTempoText(const TempoMap* const tempoMap, const int tick)
{
    if (!mu::contains(*tempoMap, tick)) {
        return true;
    }

    return tempoMap->tempo(tick) == Constants::DEFAULT_TEMPO;
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure node.
 */

void MusicXMLParserPass2::measure(const QString& partId, const Fraction time)
{
    bool isNumericMeasureNumber; // "measure numbers" don't have to be actual numbers in MusicXML
    int parsedMeasureNumber = _e.attributes().value("number").toInt(&isNumericMeasureNumber);

    //LOGD("measure %d start", parsedMeasureNumber);

    Measure* measure = findMeasure(_score, time);
    if (!measure) {
        _logger->logError(QString("measure at tick %1 not found!").arg(time.ticks()), &_e);
        skipLogCurrElem();
        return;
    }

    if (_e.attributes().value("implicit") == "yes") {
        // Implicit measure: expect measure number to be unchanged.
        measure->setIrregular(true);
    } else {
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
        if (!toMeasure(prevBase)->visible(staffIdx)) {
            measure->setStaffVisible(staffIdx, false);
        }
    } */

    Fraction mTime;   // current time stamp within measure
    Fraction prevTime;   // time stamp within measure previous chord
    Chord* prevChord = 0;         // previous chord
    Fraction mDura;   // current total measure duration
    GraceChordList gcl;   // grace chords collected sofar
    int gac = 0;         // grace after count in the grace chord list
    Beams beams; // Current beam for each voice in the current part
    QString cv = "1";         // current voice for chords, default is 1
    FiguredBassList fbl;                 // List of figured bass elements under a single note
    MxmlTupletStates tupletStates;         // Tuplet state for each voice in the current part
    Tuplets tuplets;         // Current tuplet for each voice in the current part
    DelayedDirectionsList delayedDirections; // Directions to be added to score *after* collecting all and sorting

    // collect candidates for courtesy accidentals to work out at measure end
    QMap<Note*, int> alterMap;

    while (_e.readNextStartElement()) {
        if (_e.name() == "attributes") {
            attributes(partId, measure, time + mTime);
        } else if (_e.name() == "direction") {
            MusicXMLParserDirection dir(_e, _score, _pass1, *this, _logger);
            dir.direction(partId, measure, time + mTime, _spanners, delayedDirections);
        } else if (_e.name() == "figured-bass") {
            FiguredBass* fb = figuredBass();
            if (fb) {
                fbl.append(fb);
            }
        } else if (_e.name() == "harmony") {
            harmony(partId, measure, time + mTime);
        } else if (_e.name() == "note") {
            Fraction missingPrev;
            Fraction dura;
            Fraction missingCurr;
            int alt = -10;                          // any number outside range of xml-tag "alter"
            // note: chord and grace note handling done in note()
            // dura > 0 iff valid rest or first note of chord found
            Note* n = note(partId, measure, time + mTime, time + prevTime, missingPrev, dura, missingCurr, cv, gcl, gac, beams, fbl, alt,
                           tupletStates, tuplets);
            if (n && !n->chord()->isGrace()) {
                prevChord = n->chord();          // remember last non-grace chord
            }
            if (n && n->accidental() && n->accidental()->accidentalType() != AccidentalType::NONE) {
                alterMap.insert(n, alt);
            }
            if (missingPrev.isValid()) {
                mTime += missingPrev;
            }
            if (dura.isValid() && dura > Fraction(0, 1)) {
                prevTime = mTime;         // save time stamp last chord created
                mTime += dura;
                if (mTime > mDura) {
                    mDura = mTime;
                }
            }
            if (missingCurr.isValid()) {
                mTime += missingCurr;
            }
            //LOGD("added note %p chord %p gac %d", n, n ? n->chord() : 0, gac);
        } else if (_e.name() == "forward") {
            Fraction dura;
            forward(dura);
            if (dura.isValid()) {
                mTime += dura;
                if (mTime > mDura) {
                    mDura = mTime;
                }
            }
        } else if (_e.name() == "backup") {
            Fraction dura;
            backup(dura);
            if (dura.isValid()) {
                if (dura <= mTime) {
                    mTime -= dura;
                } else {
                    _logger->logError("backup beyond measure start", &_e);
                    mTime.set(0, 1);
                }
                // check if the tick position is smaller than the minimum division resolution
                // (possibly caused by rounding errors) and in that case set position to 0
                if (mTime.isNotZero() && (_divs > 0) && (mTime < Fraction(1, 4 * _divs))) {
                    _logger->logError("backup to a fractional tick smaller than the minimum division", &_e);
                    mTime.set(0, 1);
                }
            }
        } else if (_e.name() == "sound") {
            QString tempo = _e.attributes().value("tempo").toString();

            if (!tempo.isEmpty()) {
                // sound tempo="..."
                // create an invisible default TempoText
                // to prevent duplicates, only if none is present yet
                Fraction tick = time + mTime;

                if (canAddTempoText(_score->tempomap(), tick.ticks())) {
                    double tpo = tempo.toDouble() / 60;
                    TempoText* t = Factory::createTempoText(_score->dummy()->segment());
                    t->setXmlText(QString("%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(DurationType::V_QUARTER)),
                                                         tempo));
                    t->setVisible(false);
                    t->setTempo(tpo);
                    t->setFollowText(true);

                    _score->setTempo(tick, tpo);

                    addElemOffset(t, _pass1.trackForPart(partId), "above", measure, tick);
                }
            }
            _e.skipCurrentElement();
        } else if (_e.name() == "barline") {
            barline(partId, measure, time + mTime);
        } else if (_e.name() == "print") {
            _e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }

        /*
         LOGD("mTime %s (%s) mDura %s (%s)",
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
    Part* part = _pass1.getPart(partId);   // should not fail, we only get here if the part exists
    fillGapsInFirstVoices(measure, part);

    // Prevent any beams from extending into the next measure
    for (Beam* beam : beams.values()) {
        if (beam) {
            removeBeam(beam);
        }
    }

    // Sort and add delayed directions
    std::sort(delayedDirections.begin(), delayedDirections.end(),
              // Lambda: sort by absolute value of totalY
              [](const MusicXMLDelayedDirectionElement* a, const MusicXMLDelayedDirectionElement* b) -> bool {
        return std::abs(a->totalY()) < std::abs(b->totalY());
    }
              );
    for (auto direction : delayedDirections) {
        direction->addElem();
        delete direction;
    }

    // TODO:
    // - how to handle _timeSigDura.isZero (shouldn't happen ?)
    // - how to handle unmetered music
    if (_timeSigDura.isValid() && !_timeSigDura.isZero()) {
        measure->setTimesig(_timeSigDura);
    }

    // mark superfluous accidentals as user accidentals
    const staff_idx_t scoreRelStaff = _score->staffIdx(part);
    const Key key = _score->staff(scoreRelStaff)->keySigEvent(time).key();
    markUserAccidentals(scoreRelStaff, part->nstaves(), key, measure, alterMap);

    // multi-measure rest handling
    if (getAndDecMultiMeasureRestCount() == 0) {
        // measure is first measure after a multi-measure rest
        measure->setBreakMultiMeasureRest(true);
    }

    setMeasureRepeats(scoreRelStaff, measure);

    addError(checkAtEndElement(_e, "measure"));
}

//---------------------------------------------------------
//   setMeasureRepeats
//---------------------------------------------------------

/**
 Measure repeat handling, based on values set in measureStyle().
 */

void MusicXMLParserPass2::setMeasureRepeats(const staff_idx_t scoreRelStaff, Measure* measure)
{
    for (staff_idx_t i = 0; i < _nstaves; ++i) {
        staff_idx_t staffIdx = scoreRelStaff + i;
        track_idx_t track = staff2track(staffIdx);
        if (_measureRepeatNumMeasures[i]) {
            // delete anything already added to measure
            _score->makeGap(measure->first(SegmentType::ChordRest), track, measure->stretchedLen(_score->staff(staffIdx)), 0);

            if (_measureRepeatCount[i] == _measureRepeatNumMeasures[i]) {
                // starting a new one, not continuing a multi-measure group
                _measureRepeatCount[i] = 1;
            } else {
                // continue building measure repeat group
                ++_measureRepeatCount[i];
            }

            if (((_measureRepeatNumMeasures[i] % 2) && (_measureRepeatCount[i] - 1 == _measureRepeatNumMeasures[i] / 2))
                || (!(_measureRepeatNumMeasures[i] % 2) && (_measureRepeatCount[i] == _measureRepeatNumMeasures[i] / 2))) {
                // MeasureRepeat element goes in center measure of group if odd-numbered,
                // or last measure of first half of group if even-numbered
                _score->addMeasureRepeat(measure->tick(), track, _measureRepeatNumMeasures[i]);
            } else {
                // measures that are part of group but do not contain the element have undisplayed whole rests
                _score->addRest(measure->tick(), track, TDuration(DurationType::V_MEASURE), 0);
            }
        } else {
            // measureStyle() hit a "stop" element most recently
            _measureRepeatCount[i] = 0;
        }
        measure->setMeasureRepeatCount(_measureRepeatCount[i], staffIdx);
    }
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
        if (_e.name() == "clef") {
            clef(partId, measure, tick);
        } else if (_e.name() == "divisions") {
            divisions();
        } else if (_e.name() == "key") {
            key(partId, measure, tick);
        } else if (_e.name() == "measure-style") {
            measureStyle(measure);
        } else if (_e.name() == "staff-details") {
            staffDetails(partId, measure);
        } else if (_e.name() == "time") {
            time(partId, measure, tick);
        } else if (_e.name() == "transpose") {
            _e.skipCurrentElement();        // skip but don't log
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   setStaffLines
//---------------------------------------------------------

/**
 Set stafflines and barline span for a single staff
 */

static void setStaffLines(Score* score, staff_idx_t staffIdx, int stafflines)
{
    score->staff(staffIdx)->setLines(Fraction(0, 1), stafflines);
    score->staff(staffIdx)->setBarLineTo(0);          // default
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
    IF_ASSERT_FAILED(part) {
        return;
    }
    size_t staves = part->nstaves();

    QString strNumber = _e.attributes().value("number").toString();
    int n = 0;  // default
    if (strNumber != "") {
        n = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strNumber.toInt());
        if (n < 0 || n >= int(staves)) {
            _logger->logError(QString("invalid staff-details number %1 (may be hidden)").arg(strNumber), &_e);
            n = 0;
        }
    }

    staff_idx_t staffIdx = _score->staffIdx(part) + n;

    StringData* t = new StringData;
    QString visible = _e.attributes().value("print-object").toString();
    QString spacing = _e.attributes().value("print-spacing").toString();
    if (visible == "no") {
        // EITHER:
        //  1) this indicates an empty staff that is hidden
        //  2) this indicates a cutaway measure. if it is a cutaway measure then print-spacing will be yes
        if (spacing == "yes") {
            measure->setStaffVisible(staffIdx, false);
        } else if (measure && !measure->hasVoices(staffIdx) && measure->isOnlyRests(staffIdx * VOICES)) {
            // measures with print-object="no" are generally exported by exporters such as dolet when empty staves are hidden.
            // for this reason, if we see print-object="no" (and no print-spacing), we can assume that this indicates we should set
            // the hide empty staves style.
            _score->style().set(Sid::hideEmptyStaves, true);
            _score->style().set(Sid::dontHideStavesInFirstSystem, false);
        } else {
            // this doesn't apply to a measure, so we'll assume the entire staff has to be hidden.
            _score->staff(staffIdx)->setVisible(false);
        }
    } else if (visible == "yes" || visible == "") {
        if (measure) {
            _score->staff(staffIdx)->setVisible(true);
            measure->setStaffVisible(staffIdx, true);
        }
    } else {
        _logger->logError(QString("print-object should be \"yes\" or \"no\""));
    }

    int staffLines = 0;
    while (_e.readNextStartElement()) {
        if (_e.name() == "staff-lines") {
            // save staff lines for later
            staffLines = _e.readElementText().toInt();
            // for a TAB staff also resize the string table and init with zeroes
            if (t) {
                if (0 < staffLines) {
                    t->stringList() = std::vector<instrString>(staffLines);
                } else {
                    _logger->logError(QString("illegal staff-lines %1").arg(staffLines), &_e);
                }
            }
        } else if (_e.name() == "staff-tuning") {
            staffTuning(t);
        } else {
            skipLogCurrElem();
        }
    }

    if (staffLines > 0) {
        setStaffLines(_score, staffIdx, staffLines);
    }

    if (t) {
        Instrument* i = part->instrument();
        if (_score->staff(staffIdx)->isTabStaff(Fraction(0, 1))) {
            if (i->stringData()->frets() == 0) {
                t->setFrets(25);
            } else {
                t->setFrets(i->stringData()->frets());
            }
        }
        if (t->strings() > 0) {
            i->setStringData(*t);
        } else {
            _logger->logError("trying to change string data (not supported)", &_e);
        }
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
        if (_e.name() == "tuning-alter") {
            alter = _e.readElementText().toInt();
        } else if (_e.name() == "tuning-octave") {
            octave = _e.readElementText().toInt();
        } else if (_e.name() == "tuning-step") {
            QString strStep = _e.readElementText();
            int pos = QString("CDEFGAB").indexOf(strStep);
            if (strStep.size() == 1 && pos >= 0 && pos < 7) {
                step = pos;
            } else {
                _logger->logError(QString("invalid step '%1'").arg(strStep), &_e);
            }
        } else {
            skipLogCurrElem();
        }
    }

    if (0 < line && line <= static_cast<int>(t->stringList().size())) {
        int pitch = MusicXMLStepAltOct2Pitch(step, alter, octave);
        if (pitch >= 0) {
            t->stringList()[line - 1].pitch = pitch;
        } else {
            _logger->logError(QString("invalid string %1 tuning step/alter/oct %2/%3/%4")
                              .arg(line).arg(step).arg(alter).arg(octave),
                              &_e);
        }
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
    QStringRef staffNumberString = _e.attributes().value("number");

    // by default, apply to all staves in part
    int startStaff = 0;
    int endStaff = static_cast<int>(_nstaves) - 1;

    // but if a staff number was specified in the measure-style tag, use that instead
    if (!staffNumberString.isEmpty()) {
        int staffNumber = staffNumberString.toInt();
        if (staffNumber < 1 || staffNumber > static_cast<int>(_nstaves)) {
            _logger->logError(QString("measure-style staff number can only be int from 1 to _nstaves."));
        }
        --staffNumber; // convert to 0-based
        endStaff = startStaff = staffNumber;
    }

    while (_e.readNextStartElement()) {
        if (_e.name() == "multiple-rest") {
            int multipleRest = _e.readElementText().toInt();
            if (multipleRest > 1) {
                _multiMeasureRestCount = multipleRest;
                _score->style().set(Sid::createMultiMeasureRests, true);
                measure->setBreakMultiMeasureRest(true);
            } else {
                _logger->logError(QString("multiple-rest %1 not supported").arg(multipleRest), &_e);
            }
        } else if (_e.name() == "measure-repeat") {
            QString startStop = _e.attributes().value("type").toString();
            // note: possible "slashes" attribute is either redundant with numMeasures or not supported by MuseScore, so ignored either way
            if (startStop == "start") {
                int numMeasures = _e.readElementText().toInt();
                for (int i = startStaff; i <= endStaff; i++) {
                    _measureRepeatNumMeasures[i] = numMeasures;
                    _measureRepeatCount[i] = numMeasures;   // measure repeat(s) haven't actually started yet in current measure, so this is a lie,
                                                            // but if we pretend it's true then everything is set for the next measure to restart
                }
            } else { // "stop"
                for (int i = startStaff; i <= endStaff; i++) {
                    _measureRepeatNumMeasures[i] = 0;
                }
                _e.skipCurrentElement(); // since not reading any text inside stop tag, we are done with this element
            }
        } else if (_e.name() == "slash") {
            QString type = _e.attributes().value("type").toString();
            QString stems = _e.attributes().value("use-stems").toString();
            _measureStyleSlash = type == "start" ? (stems == "yes" ? MusicXmlSlash::RHYTHM : MusicXmlSlash::SLASH) : MusicXmlSlash::NONE;
            _e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }
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

void MusicXMLDelayedDirectionElement::addElem()
{
    addElemOffset(_element, _track, _placement, _measure, _tick);
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
                                        DelayedDirectionsList& delayedDirections)
{
    //LOGD("direction tick %s", qPrintable(tick.print()));

    QString placement = _e.attributes().value("placement").toString();
    track_idx_t track = _pass1.trackForPart(partId);
    bool isVocalStaff = _pass1.isVocalStaff(partId);
    bool isExpressionText = false;
    //LOGD("direction track %d", track);
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
        if (_e.name() == "direction-type") {
            directionType(starts, stops);
        } else if (_e.name() == "offset") {
            _offset = _pass1.calcTicks(_e.readElementText().toInt(), _pass2.divs(), &_e);
            preventNegativeTick(tick, _offset, _logger);
        } else if (_e.name() == "sound") {
            sound();
        } else if (_e.name() == "staff") {
            QString strStaff = _e.readElementText();
            staff_idx_t staff = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt());
            track += staff * VOICES;
        } else {
            skipLogCurrElem();
        }
    }

    handleRepeats(measure, track, tick + _offset);
    handleNmiCmi(measure, track, tick + _offset, delayedDirections);

    // fix for Sibelius 7.1.3 (direct export) which creates metronomes without <sound tempo="..."/>:
    // if necessary, use the value calculated by metronome()
    // note: no floating point comparisons with 0 ...
    if (_tpoSound < 0.1 && _tpoMetro > 0.1) {
        _tpoSound = _tpoMetro;
    }

    //LOGD("words '%s' rehearsal '%s' metro '%s' tpo %g",
    //       qPrintable(_wordsText), qPrintable(_rehearsalText), qPrintable(_metroText), _tpoSound);

    // create text if any text was found
    if (isLyricBracket()) {
        return;
    } else if (isLikelyCredit(tick)) {
        Text* t = Factory::createText(_score->dummy(), TextStyleType::COMPOSER);
        t->setXmlText(_wordsText.trimmed());
        auto firstMeasure = _score->measures()->first();
        VBox* vbox = firstMeasure->isVBox() ? toVBox(firstMeasure) : MusicXMLParserPass1::createAndAddVBoxForCreditWords(_score);
        double spatium = _score->style().styleD(Sid::spatium);
        vbox->setBoxHeight(vbox->boxHeight() + Spatium(t->height() / spatium / 2)); // add some height
        vbox->add(t);
    } else if (_wordsText != "" || _rehearsalText != "" || _metroText != "") {
        TextBase* t = 0;
        if (_tpoSound > 0.1) {
            if (canAddTempoText(_score->tempomap(), tick.ticks())) {
                _tpoSound /= 60;
                t = Factory::createTempoText(_score->dummy()->segment());
                QString rawWordsText = _wordsText;
                rawWordsText.remove(QRegularExpression("(<.*?>)"));
                QString sep = _metroText != "" && _wordsText != "" && rawWordsText.back() != ' ' ? " " : "";
                t->setXmlText(_wordsText + sep + _metroText);
                ((TempoText*)t)->setTempo(_tpoSound);
                ((TempoText*)t)->setFollowText(true);
                _score->setTempo(tick, _tpoSound);
            }
        } else {
            if (_wordsText != "" || _metroText != "") {
                t = Factory::createStaffText(_score->dummy()->segment());
                t->setXmlText(_wordsText + _metroText);
                isExpressionText = _wordsText.contains("<i>") && _metroText.isEmpty();
            } else {
                t = Factory::createRehearsalMark(_score->dummy()->segment());
                if (!_rehearsalText.contains("<b>")) {
                    _rehearsalText = "<b></b>" + _rehearsalText;            // explicitly turn bold off
                }
                t->setXmlText(_rehearsalText);
                if (!_hasDefaultY) {
                    t->setPlacement(PlacementV::ABOVE);            // crude way to force placement TODO improve ?
                    t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                }
            }
        }

        if (t) {
            if (_enclosure == "circle") {
                t->setFrameType(FrameType::CIRCLE);
            } else if (_enclosure == "none") {
                t->setFrameType(FrameType::NO_FRAME);
            } else if (_enclosure == "rectangle") {
                t->setFrameType(FrameType::SQUARE);
                t->setFrameRound(0);
            }

            QString wordsPlacement = placement;
            // Case-based defaults
            if (wordsPlacement.isEmpty()) {
                if (isVocalStaff) {
                    wordsPlacement = "above";
                } else if (isExpressionText) {
                    wordsPlacement = "below";
                }
            }

            if (placement == "" && hasTotalY()) {
                placement = totalY() < 0 ? "above" : "below";
            }
            if (hasTotalY()) {
                // Add element to score later, after collecting all the others and sorting by default-y
                // This allows default-y to be at least respected by the order of elements
                MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(
                    totalY(), t, track, placement, measure, tick + _offset);
                delayedDirections.push_back(delayedDirection);
            } else {
                addElemOffset(t, track, placement, measure, tick + _offset);
            }
        }
    } else if (_tpoSound > 0) {
        // direction without text but with sound tempo="..."
        // create an invisible default TempoText

        if (canAddTempoText(_score->tempomap(), tick.ticks())) {
            double tpo = _tpoSound / 60;
            TempoText* t = Factory::createTempoText(_score->dummy()->segment());
            t->setXmlText(QString("%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(DurationType::V_QUARTER))).arg(
                              _tpoSound));
            t->setVisible(false);
            t->setTempo(tpo);
            t->setFollowText(true);

            // TBD may want ro use tick + _offset if sound is affected
            _score->setTempo(tick, tpo);

            addElemOffset(t, track, placement, measure, tick + _offset);
        }
    }

    // do dynamics
    // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
    for (QStringList::Iterator it = _dynamicsList.begin(); it != _dynamicsList.end(); ++it) {
        Dynamic* dyn = Factory::createDynamic(_score->dummy()->segment());
        dyn->setDynamicType(*it);
        if (!_dynaVelocity.isEmpty()) {
            int dynaValue = round(_dynaVelocity.toDouble() * 0.9);
            if (dynaValue > 127) {
                dynaValue = 127;
            } else if (dynaValue < 0) {
                dynaValue = 0;
            }
            dyn->setVelocity(dynaValue);
        }

        QString dynamicsPlacement = placement;
        // Case-based defaults
        if (dynamicsPlacement.isEmpty()) {
            dynamicsPlacement = isVocalStaff ? "above" : "below";
        }

        // Add element to score later, after collecting all the others and sorting by default-y
        // This allows default-y to be at least respected by the order of elements
        MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(
            hasTotalY() ? totalY() : 100, dyn, track, dynamicsPlacement, measure, tick + _offset);
        delayedDirections.push_back(delayedDirection);
    }

    // handle the elems
    foreach (auto elem, _elems) {
        // TODO (?) if (_hasDefaultY) elem->setYoff(_defaultY);
        addElemOffset(elem, track, placement, measure, tick + _offset);
    }

    // handle the spanner stops first
    foreach (auto desc, stops) {
        auto& spdesc = _pass2.getSpanner({ desc._tp, desc._nr });
        if (spdesc._isStopped) {
            _logger->logError("spanner already stopped", &_e);
            delete desc._sp;
        } else {
            if (spdesc._isStarted) {
                handleSpannerStop(spdesc._sp, track, tick + _offset, spanners);
                _pass2.clearSpanner(desc);
            } else {
                spdesc._sp = desc._sp;
                spdesc._tick2 = tick + _offset;
                spdesc._track2 = track;
                spdesc._isStopped = true;
            }
        }
    }

    // then handle the spanner starts
    // TBD handle offset ?
    foreach (auto desc, starts) {
        auto& spdesc = _pass2.getSpanner({ desc._tp, desc._nr });
        if (spdesc._isStarted) {
            _logger->logError("spanner already started", &_e);
            delete desc._sp;
        } else {
            if (spdesc._isStopped) {
                _pass2.addSpanner(desc);
                // handleSpannerStart and handleSpannerStop must be called in order
                // due to allocation of elements in the map
                handleSpannerStart(desc._sp, track, placement, tick + _offset, spanners);
                handleSpannerStop(spdesc._sp, spdesc._track2, spdesc._tick2, spanners);
                _pass2.clearSpanner(desc);
            } else {
                _pass2.addSpanner(desc);
                handleSpannerStart(desc._sp, track, placement, tick + _offset, spanners);
                spdesc._isStarted = true;
            }
        }
    }
}

//---------------------------------------------------------
//   isLikelyCredit
//---------------------------------------------------------

bool MusicXMLParserDirection::isLikelyCredit(const Fraction& tick) const
{
    return (tick + _offset < Fraction(5, 1)) // Only early in the piece
           && _rehearsalText == ""
           && _metroText == ""
           && _tpoSound < 0.1
           && _wordsText.contains(QRegularExpression("^\\s*((Words|Music|Lyrics).*)*by\\s+([A-Z][a-zA-Zö'’-]+\\s[A-Z][a-zA-Zös'’-]+.*)+"));
}

//---------------------------------------------------------
//   isLyricBracket
//    Dolet exports lyric brackets as staff text,
//    which we ought not render.
//---------------------------------------------------------

bool MusicXMLParserDirection::isLyricBracket() const
{
    return _wordsText.contains(QRegularExpression("^}|{$"))
           && _rehearsalText == ""
           && _metroText == ""
           && _dynamicsList.isEmpty()
           && _tpoSound < 0.1;
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
        _defaultY = _e.attributes().value("default-y").toDouble(&_hasDefaultY) * -0.1;
        QString number = _e.attributes().value("number").toString();
        int n = 0;
        if (number != "") {
            n = number.toInt();
            if (n <= 0) {
                _logger->logError(QString("invalid number %1").arg(number), &_e);
            } else {
                n--;          // make zero-based
            }
        }
        QString type = _e.attributes().value("type").toString();
        if (_e.name() == "metronome") {
            _metroText = metronome(_tpoMetro);
        } else if (_e.name() == "words") {
            _enclosure      = _e.attributes().value("enclosure").toString();
            _wordsText += xmlpass2::nextPartOfFormattedString(_e);
        } else if (_e.name() == "rehearsal") {
            _enclosure      = _e.attributes().value("enclosure").toString();
            if (_enclosure == "") {
                _enclosure = "square";          // note different default
            }
            _rehearsalText += xmlpass2::nextPartOfFormattedString(_e);
        } else if (_e.name() == "pedal") {
            pedal(type, n, starts, stops);
        } else if (_e.name() == "octave-shift") {
            octaveShift(type, n, starts, stops);
        } else if (_e.name() == "dynamics") {
            dynamics();
        } else if (_e.name() == "bracket") {
            bracket(type, n, starts, stops);
        } else if (_e.name() == "dashes") {
            dashes(type, n, starts, stops);
        } else if (_e.name() == "wedge") {
            wedge(type, n, starts, stops);
        } else if (_e.name() == "coda") {
            _wordsText += "<sym>coda</sym>";
            _e.skipCurrentElement();
        } else if (_e.name() == "segno") {
            _wordsText += "<sym>segno</sym>";
            _e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }
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
        if (_e.name() == "other-dynamics") {
            _dynamicsList.push_back(_e.readElementText());
        } else {
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

QString MusicXMLParserDirection::matchRepeat() const
{
    QString plainWords = MScoreTextToMXML::toPlainText(_wordsText.toLower().simplified());
    QRegularExpression daCapo("^(d\\.? ?|da )(c\\.?|capo)$");
    QRegularExpression daCapoAlFine("^(d\\.? ?|da )(c\\.? ?|capo )al fine$");
    QRegularExpression daCapoAlCoda("^(d\\.? ?|da )(c\\.? ?|capo )al coda$");
    QRegularExpression dalSegno("^(d\\.? ?|d[ae]l )(s\\.?|segno)$");
    QRegularExpression dalSegnoAlFine("^(d\\.? ?|d[ae]l )(s\\.?|segno\\.?) al fine$");
    QRegularExpression dalSegnoAlCoda("^(d\\.? ?|d[ae]l )(s\\.?|segno\\.?) al coda$");
    QRegularExpression fine("^fine$");
    QRegularExpression segno("^segno( segno)?$");
    QRegularExpression toCoda("^to coda( coda)?$");
    QRegularExpression coda("^coda( coda)?$");
    if (plainWords.contains(daCapo)) {
        return "daCapo";
    }
    if (plainWords.contains(daCapoAlFine)) {
        return "daCapoAlFine";
    }
    if (plainWords.contains(daCapoAlCoda)) {
        return "daCapoAlCoda";
    }
    if (plainWords.contains(dalSegno)) {
        return "dalSegno";
    }
    if (plainWords.contains(dalSegnoAlFine)) {
        return "dalSegnoAlFine";
    }
    if (plainWords.contains(dalSegnoAlCoda)) {
        return "dalSegnoAlCoda";
    }
    if (plainWords.contains(segno)) {
        return "segno";
    }
    if (plainWords.contains(fine)) {
        return "fine";
    }
    if (plainWords.contains(toCoda)) {
        return "toCoda";
    }
    if (plainWords.contains(coda)) {
        return "coda";
    }
    return "";
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
        jp = Factory::createJump(score->dummy()->measure());
        jp->setJumpType(JumpType::DC);
    } else if (repeat == "daCapoAlCoda") {
        jp = Factory::createJump(score->dummy()->measure());
        jp->setJumpType(JumpType::DC_AL_CODA);
    } else if (repeat == "daCapoAlFine") {
        jp = Factory::createJump(score->dummy()->measure());
        jp->setJumpType(JumpType::DC_AL_FINE);
    } else if (repeat == "dalSegno") {
        jp = Factory::createJump(score->dummy()->measure());
        jp->setJumpType(JumpType::DS);
    } else if (repeat == "dalSegnoAlCoda") {
        jp = Factory::createJump(score->dummy()->measure());
        jp->setJumpType(JumpType::DS_AL_CODA);
    } else if (repeat == "dalSegnoAlFine") {
        jp = Factory::createJump(score->dummy()->measure());
        jp->setJumpType(JumpType::DS_AL_FINE);
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
        m = Factory::createMarker(score->dummy());
        // note: Marker::read() also contains code to set text style based on type
        // avoid duplicated code
        // apparently this MUST be after setTextStyle
        m->setMarkerType(MarkerType::SEGNO);
    } else if (repeat == "coda") {
        m = Factory::createMarker(score->dummy());
        m->setMarkerType(MarkerType::CODA);
    } else if (repeat == "fine") {
        m = Factory::createMarker(score->dummy(), TextStyleType::REPEAT_RIGHT);
        m->setMarkerType(MarkerType::FINE);
    } else if (repeat == "toCoda") {
        m = Factory::createMarker(score->dummy(), TextStyleType::REPEAT_RIGHT);
        m->setMarkerType(MarkerType::TOCODA);
    }
    return m;
}

//---------------------------------------------------------
//   handleRepeats
//---------------------------------------------------------

void MusicXMLParserDirection::handleRepeats(Measure* measure, const track_idx_t track, const Fraction tick)
{
    // Try to recognize the various repeats
    QString repeat = "";
    if (_sndCoda != "") {
        repeat = "coda";
    } else if (_sndDacapo != "") {
        repeat = "daCapo";
    } else if (_sndDalsegno != "") {
        repeat = "dalSegno";
    } else if (_sndFine != "") {
        repeat = "fine";
    } else if (_sndSegno != "") {
        repeat = "segno";
    } else if (_sndToCoda != "") {
        repeat = "toCoda";
    } else {
        repeat = matchRepeat();
    }

    if (repeat != "") {
        TextBase* tb = nullptr;
        if ((tb = findJump(repeat, _score)) || (tb = findMarker(repeat, _score))) {
            tb->setTrack(track);
            if (!_wordsText.isEmpty()) {
                tb->setXmlText(_wordsText);
                _wordsText = "";
            } else {
                tb->setVisible(false);
            }
            // Sometimes Jumps and Markers are exported on the incorrect side
            // of the barline (i.e. end of mm. 29 vs. beginning of mm. 30).
            // This fixes that.
            bool closerToLeft = tick - measure->tick() < measure->endTick() - tick;
            if (tb->textStyleType() == TextStyleType::REPEAT_RIGHT
                && closerToLeft
                && measure->prevMeasure()) {
                measure = measure->prevMeasure();
            } else if (tb->textStyleType() == TextStyleType::REPEAT_LEFT
                       && !closerToLeft
                       && measure->nextMeasure()) {
                measure = measure->nextMeasure();
            }
            measure->add(tb);
        }
    }
}

//---------------------------------------------------------
//   handleNmiCmi
//    Dolet strangely exports N.C. chord symbols as the
//    text direction "NmiCmi".
//---------------------------------------------------------

void MusicXMLParserDirection::handleNmiCmi(Measure* measure, const track_idx_t track, const Fraction tick,
                                           DelayedDirectionsList& delayedDirections)
{
    if (!_wordsText.contains("NmiCmi")) {
        return;
    }
    Harmony* ha = new Harmony(_score->dummy()->segment());
    ha->setRootTpc(Tpc::TPC_INVALID);
    ha->setId(-1);
    ha->setTextName(u"N.C.");
    ha->setTrack(track);
    MusicXMLDelayedDirectionElement* delayedDirection = new MusicXMLDelayedDirectionElement(totalY(), ha, track, "above", measure, tick);
    delayedDirections.push_back(delayedDirection);
    _wordsText.replace("NmiCmi", "N.C.");
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
    const bool isWavy = lineType == "wavy";
    const ElementType elementType = isWavy ? ElementType::TRILL : ElementType::TEXTLINE;
    const auto& spdesc = _pass2.getSpanner({ elementType, number });
    if (type == "start") {
        SLine* sline = spdesc._isStopped ? spdesc._sp : 0;
        if ((sline && sline->isTrill()) || (!sline && isWavy)) {
            if (!sline) {
                sline = Factory::createTrill(_score->dummy());
            }
            auto trill = toTrill(sline);
            trill->setTrillType(TrillType::PRALLPRALL_LINE);

            if (!lineEnd.isEmpty() && lineEnd != "none") {
                _logger->logError(QString("line-end not supported for line-type \"wavy\""));
            }
        } else if ((sline && sline->isTextLine()) || (!sline && !isWavy)) {
            if (!sline) {
                sline = new TextLine(_score->dummy());
            }
            auto textLine = toTextLine(sline);
            // if (placement == "") placement = "above";  // TODO ? set default

            textLine->setBeginHookType(lineEnd != "none" ? HookType::HOOK_90 : HookType::NONE);
            if (lineEnd == "up") {
                textLine->setBeginHookHeight(-1 * textLine->beginHookHeight());
            }

            // hack: combine with a previous words element
            if (!_wordsText.isEmpty()) {
                // TextLine supports only limited formatting, remove all (compatible with 1.3)
                textLine->setBeginText(MScoreTextToMXML::toPlainText(_wordsText));
                _wordsText = "";
            }

            if (lineType == "solid") {
                textLine->setLineStyle(LineType::SOLID);
            } else if (lineType == "dashed") {
                textLine->setLineStyle(LineType::DASHED);
            } else if (lineType == "dotted") {
                textLine->setLineStyle(LineType::DOTTED);
            } else if (lineType != "wavy") {
                _logger->logError(QString("unsupported line-type: %1").arg(lineType.toString()), &_e);
            }
            const QColor color { _e.attributes().value("color").toString() };
            if (color.isValid()) {
                textLine->setLineColor(color);
            }
        }

        starts.append(MusicXmlSpannerDesc(sline, elementType, number));
    } else if (type == "stop") {
        SLine* sline = spdesc._isStarted ? spdesc._sp : 0;
        if ((sline && sline->isTrill()) || (!sline && isWavy)) {
            if (!sline) {
                sline = new Trill(_score->dummy());
            }
            if (!lineEnd.isEmpty() && lineEnd != "none") {
                _logger->logError(QString("line-end not supported for line-type \"wavy\""));
            }
        } else if ((sline && sline->isTextLine()) || (!sline && !isWavy)) {
            if (!sline) {
                sline = new TextLine(_score->dummy());
            }
            auto textLine = toTextLine(sline);
            textLine->setEndHookType(lineEnd != "none" ? HookType::HOOK_90 : HookType::NONE);
            if (lineEnd == "up") {
                textLine->setEndHookHeight(-1 * textLine->endHookHeight());
            }
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
    const auto& spdesc = _pass2.getSpanner({ ElementType::HAIRPIN, number });
    if (type == "start") {
        auto b = spdesc._isStopped ? toTextLine(spdesc._sp) : Factory::createTextLine(_score->dummy());
        // if (placement == "") placement = "above";  // TODO ? set default

        // hack: combine with a previous words element
        if (!_wordsText.isEmpty()) {
            // TextLine supports only limited formatting, remove all (compatible with 1.3)
            b->setBeginText(MScoreTextToMXML::toPlainText(_wordsText));
            _wordsText = "";
        }

        b->setBeginHookType(HookType::NONE);
        b->setEndHookType(HookType::NONE);
        b->setLineStyle(LineType::DASHED);
        // TODO brackets and dashes now share the same storage
        // because they both use ElementType::TEXTLINE
        // use MusicXML specific type instead
        starts.append(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
    } else if (type == "stop") {
        auto b = spdesc._isStarted ? toTextLine(spdesc._sp) : Factory::createTextLine(_score->dummy());
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
    const auto& spdesc = _pass2.getSpanner({ ElementType::OTTAVA, number });
    if (type == "up" || type == "down") {
        int ottavasize = _e.attributes().value("size").toInt();
        if (!(ottavasize == 8 || ottavasize == 15)) {
            _logger->logError(QString("unknown octave-shift size %1").arg(ottavasize), &_e);
        } else {
            auto o = spdesc._isStopped ? toOttava(spdesc._sp) : Factory::createOttava(_score->dummy());

            // if (placement == "") placement = "above";  // TODO ? set default

            if (type == "down" && ottavasize == 8) {
                o->setOttavaType(OttavaType::OTTAVA_8VA);
            }
            if (type == "down" && ottavasize == 15) {
                o->setOttavaType(OttavaType::OTTAVA_15MA);
            }
            if (type == "up" && ottavasize == 8) {
                o->setOttavaType(OttavaType::OTTAVA_8VB);
            }
            if (type == "up" && ottavasize == 15) {
                o->setOttavaType(OttavaType::OTTAVA_15MB);
            }

            const QColor color { _e.attributes().value("color").toString() };
            if (color.isValid()) {
                o->setLineColor(color);
            }

            starts.append(MusicXmlSpannerDesc(o, ElementType::OTTAVA, number));
        }
    } else if (type == "stop") {
        auto o = spdesc._isStarted ? toOttava(spdesc._sp) : Factory::createOttava(_score->dummy());
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
    const QColor color { _e.attributes().value("color").toString() };

    // We have found that many exporters omit "sign" even when one is originally present,
    // therefore we will default to "yes", even though this is technically against the spec.
    bool overrideDefaultSign = true; // TODO: set this flag based on the exporting software
    if (sign == "") {
        if (line != "yes" || (overrideDefaultSign && type == "start")) {
            sign = "yes";                           // MusicXML 2.0 compatibility
        } else if (line == "yes") {
            sign = "no";                            // MusicXML 2.0 compatibility
        }
    }
    auto& spdesc = _pass2.getSpanner({ ElementType::PEDAL, number });
    if (type == "start" || type == "resume" || type == "sostenuto") {
        if (spdesc._isStarted && !spdesc._isStopped) {
            // Previous pedal unterminated—likely an unrecorded "discontinue", so delete the line.
            // TODO: if "change", create 0-length spanner rather than delete
            _pass2.deleteHandledSpanner(spdesc._sp);
            spdesc._isStarted = false;
        }
        auto p = spdesc._isStopped ? toPedal(spdesc._sp) : new Pedal(_score->dummy());
        if (line == "yes") {
            p->setLineVisible(true);
        } else {
            p->setLineVisible(false);
        }
        if (!p->lineVisible() || sign == "yes") {
            p->setBeginText(u"<sym>keyboardPedalPed</sym>");
            p->setContinueText(u"(<sym>keyboardPedalPed</sym>)");
            if (type == "sostenuto") {
                p->setBeginText(u"<sym>keyboardPedalSost</sym>");
                p->setContinueText(u"(<sym>keyboardPedalSost</sym>)");
            }
        } else {
            p->setBeginText(u"");
            p->setContinueText(u"");
            p->setBeginHookType(type == "resume" ? HookType::NONE : HookType::HOOK_90);
        }
        p->setEndHookType(HookType::NONE);
        if (color.isValid()) {
            p->setLineColor(color);
        }
        starts.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
    } else if (type == "stop" || type == "discontinue") {
        auto p = spdesc._isStarted ? toPedal(spdesc._sp) : new Pedal(_score->dummy());
        if (line == "yes") {
            p->setLineVisible(true);
        } else if (line == "no") {
            p->setLineVisible(false);
        }
        if (!p->lineVisible() || sign == "yes") {
            p->setEndText(u"<sym>keyboardPedalUp</sym>");
        } else {
            p->setEndHookType(type == "discontinue" ? HookType::NONE : HookType::HOOK_90);
        }
        stops.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
    } else if (type == "change") {
        // pedal change is implemented as two separate pedals
        // first stop the first one
        if (spdesc._isStarted && !spdesc._isStopped) {
            auto p = toPedal(spdesc._sp);
            p->setEndHookType(HookType::HOOK_45);
            if (line == "yes") {
                p->setLineVisible(true);
            } else if (line == "no") {
                p->setLineVisible(false);
            }
            stops.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
        } else {
            _logger->logError(QString("\"change\" type pedal created without existing pedal"), &_e);
        }
        // then start a new one
        auto p = new Pedal(_score->dummy());
        p->setBeginHookType(HookType::HOOK_45);
        p->setEndHookType(HookType::HOOK_90);
        if (line == "yes") {
            p->setLineVisible(true);
        } else {
            p->setLineVisible(false);
        }
        if (sign == "no") {
            p->setBeginText(u"");
            p->setContinueText(u"");
        }
        if (color.isValid()) {
            p->setColor(color);
        }
        starts.append(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
    } else if (type == "continue") {
        // ignore
    } else {
        qDebug("unknown pedal type %s", qPrintable(type));
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
    const auto& spdesc = _pass2.getSpanner({ ElementType::HAIRPIN, number });
    if (type == "crescendo" || type == "diminuendo") {
        auto h = spdesc._isStopped ? toHairpin(spdesc._sp) : Factory::createHairpin(_score->dummy()->segment());
        h->setHairpinType(type == "crescendo"
                          ? HairpinType::CRESC_HAIRPIN : HairpinType::DECRESC_HAIRPIN);
        if (niente == "yes") {
            h->setHairpinCircledTip(true);
        }
        const QColor color { _e.attributes().value("color").toString() };
        if (color.isValid()) {
            h->setLineColor(color);
        }
        starts.append(MusicXmlSpannerDesc(h, ElementType::HAIRPIN, number));
    } else if (type == "stop") {
        auto h = spdesc._isStarted ? toHairpin(spdesc._sp) : Factory::createHairpin(_score->dummy()->segment());
        if (niente == "yes") {
            h->setHairpinCircledTip(true);
        }
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
           .arg(string, _tick2.toString())
           .arg(static_cast<int>(_track2))
           .arg(_isStarted ? "started" : "", _isStopped ? "stopped" : "")
    ;
}

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void MusicXMLParserPass2::addSpanner(const MusicXmlSpannerDesc& d)
{
    auto& spdesc = getSpanner(d);
    spdesc._sp = d._sp;
}

//---------------------------------------------------------
//   getSpanner
//---------------------------------------------------------

MusicXmlExtendedSpannerDesc& MusicXMLParserPass2::getSpanner(const MusicXmlSpannerDesc& d)
{
    if (d._tp == ElementType::HAIRPIN && 0 <= d._nr && d._nr < MAX_NUMBER_LEVEL) {
        return _hairpins[d._nr];
    } else if (d._tp == ElementType::OTTAVA && 0 <= d._nr && d._nr < MAX_NUMBER_LEVEL) {
        return _ottavas[d._nr];
    } else if (d._tp == ElementType::PEDAL && 0 == d._nr) {
        return _pedal;
    } else if ((d._tp == ElementType::TEXTLINE || d._tp == ElementType::TRILL) && 0 <= d._nr && d._nr < MAX_NUMBER_LEVEL) {
        return _brackets[d._nr];
    }
    _logger->logError(QString("invalid number %1").arg(d._nr + 1), &_e);
    return _dummyNewMusicXmlSpannerDesc;
}

//---------------------------------------------------------
//   clearSpanner
//---------------------------------------------------------

void MusicXMLParserPass2::clearSpanner(const MusicXmlSpannerDesc& d)
{
    auto& spdesc = getSpanner(d);
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

    if (parenth) {
        tempoText += "(";
    }

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
            QByteArray ba = txt.toLatin1();
            if (!dur1.isValid()) {
                dur1.setType(TConv::fromXml(ba.constData(), DurationType::V_INVALID));
            } else if (!dur2.isValid()) {
                dur2.setType(TConv::fromXml(ba.constData(), DurationType::V_INVALID));
            }
        } else if (_e.name() == "beat-unit-dot") {
            if (dur2.isValid()) {
                dur2.setDots(1);
            } else if (dur1.isValid()) {
                dur1.setDots(1);
            }
        } else if (_e.name() == "per-minute") {
            perMinute = txt;
        } else {
            skipLogCurrElem();
        }
    }

    if (dur1.isValid()) {
        tempoText += TempoText::duration2tempoTextString(dur1);
    }
    if (dur2.isValid()) {
        tempoText += " = ";
        tempoText += TempoText::duration2tempoTextString(dur2);
    } else if (perMinute != "") {
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

    if (parenth) {
        tempoText += ")";
    }

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

    if (barStyle == "light-heavy" && repeat == "backward") {
        type = BarLineType::END_REPEAT;
    } else if (barStyle == "heavy-light" && repeat == "forward") {
        type = BarLineType::START_REPEAT;
    } else if (barStyle == "light-heavy" && repeat.isEmpty()) {
        type = BarLineType::END;
    } else if (barStyle == "heavy-light" && repeat.isEmpty()) {
        type = BarLineType::REVERSE_END;
    } else if (barStyle == "regular") {
        type = BarLineType::NORMAL;
    } else if (barStyle == "dashed") {
        type = BarLineType::BROKEN;
    } else if (barStyle == "dotted") {
        type = BarLineType::DOTTED;
    } else if (barStyle == "light-light") {
        type = BarLineType::DOUBLE;
        /*
        } else if (barStyle == "heavy-light") {
         ;
         */
    } else if (barStyle == "heavy-heavy") {
        type = BarLineType::DOUBLE_HEAVY;
    } else if (barStyle == "heavy") {
        type = BarLineType::HEAVY;
    } else if (barStyle == "none") {
        visible = false;
    } else if (barStyle == "") {
        if (repeat == "backward") {
            type = BarLineType::END_REPEAT;
        } else if (repeat == "forward") {
            type = BarLineType::START_REPEAT;
        } else {
            LOGD("empty bar type");             // TODO
            return false;
        }
    } else if ((barStyle == "tick") || (barStyle == "short")) {
        // handled later (as normal barline with different parameters)
    } else {
        LOGD("unsupported bar type <%s>", qPrintable(barStyle));           // TODO
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
    if (loc == "") {
        loc = "right";
    }
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
            barStyle = _e.readElementText();
        } else if (_e.name() == "ending") {
            endingNumber = _e.attributes().value("number").toString();
            endingType   = _e.attributes().value("type").toString();
            endingColor = _e.attributes().value("color").toString();
            printEnding = _e.attributes().value("print-object").toString() != "no";
            endingText = _e.readElementText();
        } else if (_e.name() == "fermata") {
            const QColor fermataColor = _e.attributes().value("color").toString();
            const QString fermataType = _e.attributes().value("type").toString();
            const auto segment = measure->getSegment(SegmentType::EndBarLine, tick);
            const track_idx_t track = _pass1.trackForPart(partId);
            Fermata* fermata = Factory::createFermata(segment);
            fermata->setSymId(convertFermataToSymId(_e.readElementText()));
            fermata->setTrack(track);
            segment->add(fermata);
            if (fermataColor.isValid()) {
                fermata->setColor(fermataColor);
            }
            if (fermataType == "inverted") {
                fermata->setPlacement(PlacementV::BELOW);
            }
        } else if (_e.name() == "repeat") {
            repeat = _e.attributes().value("direction").toString();
            count = _e.attributes().value("times").toString();
            if (count.isEmpty()) {
                count = "2";
            }
            measure->setRepeatCount(count.toInt());
            _e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }
    }

    BarLineType type = BarLineType::NORMAL;
    bool visible = true;
    if (determineBarLineType(barStyle, repeat, type, visible)) {
        const track_idx_t track = _pass1.trackForPart(partId);
        if (type == BarLineType::START_REPEAT) {
            // combine start_repeat flag with current state initialized during measure parsing
            measure->setRepeatStart(true);
        } else if (type == BarLineType::END_REPEAT) {
            // combine end_repeat flag with current state initialized during measure parsing
            measure->setRepeatEnd(true);
        } else {
            if (barStyle != "regular" || barlineColor.isValid() || loc == "middle") {
                // Add barline to the first voice of every staff in the part,
                // and span every barline except the last
                staff_idx_t nstaves = _pass1.getPart(partId)->nstaves();
                for (staff_idx_t i = 0; i < nstaves; ++i) {
                    bool spanStaff = i < nstaves - 1;
                    track_idx_t currentTrack = track + (i * VOICES);
                    auto b = createBarline(measure->score(), currentTrack, type, visible, barStyle, spanStaff);
                    if (barlineColor.isValid()) {
                        b->setColor(barlineColor);
                    }
                    addBarlineToMeasure(measure, tick, std::move(b));
                }
            }
        }
    }

    doEnding(partId, measure, endingNumber, endingType, endingColor, endingText, printEnding);
}

//---------------------------------------------------------
//   findRedundantVolta
//---------------------------------------------------------
static Volta* findRedundantVolta(const track_idx_t track, const Measure* measure)
{
    auto spanners = measure->score()->spannerMap().findOverlapping(measure->tick().ticks(), measure->endTick().ticks());
    for (auto spanner : spanners) {
        if (spanner.value->isVolta() && track2staff(spanner.value->track()) != track2staff(track)) {
            return toVolta(spanner.value);
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   doEnding
//---------------------------------------------------------

void MusicXMLParserPass2::doEnding(const QString& partId, Measure* measure, const QString& number, const QString& type, const QColor color,
                                   const QString& text, const bool print)
{
    if (!(number.isEmpty() && type.isEmpty())) {
        if (number.isEmpty()) {
            _logger->logError("empty ending number", &_e);
        } else if (type.isEmpty()) {
            _logger->logError("empty ending type", &_e);
        } else {
            QStringList sl = number.split(",", Qt::SkipEmptyParts);
            std::vector<int> iEndingNumbers;
            bool unsupported = false;
            foreach (const QString& s, sl) {
                int iEndingNumber = s.toInt();
                if (iEndingNumber <= 0) {
                    unsupported = true;
                    break;
                }
                iEndingNumbers.push_back(iEndingNumber);
            }

            if (unsupported) {
                _logger->logError(QString("unsupported ending number '%1'").arg(number), &_e);
            } else {
                // Ignore if it is hidden and redundant
                Volta* redundantVolta = findRedundantVolta(_pass1.trackForPart(partId), measure);
                if (!print && redundantVolta) {
                    _logger->logDebugInfo("Ignoring redundant hidden Volta", &_e);
                } else if (type == "start") {
                    Volta* volta = Factory::createVolta(_score->dummy());
                    volta->setTrack(_pass1.trackForPart(partId));
                    volta->setText(text.isEmpty() ? number : text);
                    // LVIFIX TODO also support endings "1 - 3"
                    volta->endings().clear();
                    mu::join(volta->endings(), iEndingNumbers);
                    volta->setTick(measure->tick());
                    _score->addElement(volta);
                    _lastVolta = volta;
                    volta->setVisible(print);
                    if (color.isValid()) {
                        volta->setLineColor(color);
                    }
                } else if (type == "stop") {
                    if (_lastVolta) {
                        _lastVolta->setVoltaType(Volta::Type::CLOSED);
                        _lastVolta->setTick2(measure->tick() + measure->ticks());
                        // Assume print-object was handled at the start
                        _lastVolta = 0;
                    } else if (!redundantVolta) {
                        _logger->logError("ending stop without start", &_e);
                    }
                } else if (type == "discontinue") {
                    if (_lastVolta) {
                        _lastVolta->setVoltaType(Volta::Type::OPEN);
                        _lastVolta->setTick2(measure->tick() + measure->ticks());
                        // Assume print-object was handled at the start
                        _lastVolta = 0;
                    } else if (!redundantVolta) {
                        _logger->logError("ending discontinue without start", &_e);
                    }
                } else {
                    _logger->logError(QString("unsupported ending type '%1'").arg(type), &_e);
                }

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
    //LOGD("addSymToSig(step '%s' alt '%s' acc '%s')",
    //       qPrintable(step), qPrintable(alter), qPrintable(accid));

    SymId id = mxmlString2accSymId(accid, smufl);

    if (id == SymId::noSym) {
        bool ok;
        double d;
        d = alter.toDouble(&ok);
        AccidentalType accTpAlter = ok ? microtonalGuess(d) : AccidentalType::NONE;
        QString s = accidentalType2MxmlString(accTpAlter);
        if (s == "other") {
            s = accidentalType2SmuflMxmlString(accTpAlter);
        }
        id = mxmlString2accSymId(s);
    }

    if (step.size() == 1 && id != SymId::noSym) {
        const QString table = "CDEFGAB";
        if (table.contains(step)) {
            CustDef cd;
            cd.degree = table.indexOf(step);
            cd.sym = id;
            sig.customKeyDefs().push_back(cd);
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
                   Measure* measure, const staff_idx_t staffIdx, const Fraction& tick)
{
    Key oldkey = score->staff(staffIdx)->key(tick);
    // TODO only if different custom key ?
    if (oldkey != key.key() || key.custom() || key.isAtonal()) {
        // new key differs from key in effect at this tick
        Segment* s = measure->getSegment(SegmentType::KeySig, tick);
        KeySig* keysig = Factory::createKeySig(s);
        keysig->setTrack(staffIdx * VOICES);
        keysig->setKeySigEvent(key);
        keysig->setVisible(printObj);
        if (keyColor.isValid()) {
            keysig->setColor(keyColor);
        }
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
    //LOGD("flushAlteredTone(step '%s' alt '%s' acc '%s')",
    //       qPrintable(step), qPrintable(alt), qPrintable(acc));

    if (step == "" && alt == "" && acc == "") {
        return;      // nothing to do
    }
    // step and alt are required, but also accept step and acc
    if (step != "" && (alt != "" || acc != "")) {
        addSymToSig(kse, step, alt, acc, smufl);
    } else {
        LOGD("flushAlteredTone invalid combination of step '%s' alt '%s' acc '%s')",
             qPrintable(step), qPrintable(alt), qPrintable(acc));       // TODO
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

void MusicXMLParserPass2::key(const QString& partId, Measure* measure, const Fraction& tick)
{
    QString strKeyno = _e.attributes().value("number").toString();
    int keyno = -1;   // assume no number (see below)
    if (strKeyno != "") {
        keyno = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strKeyno.toInt());
        if (keyno < 0) {
            // conversion error (-1), assume staff 0
            _logger->logError(QString("invalid key number '%1'").arg(strKeyno), &_e);
            keyno = 0;
        }
    }
    const bool printObject = _e.attributes().value("print-object") != "no";
    const QColor keyColor { _e.attributes().value("color").toString() };

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
        if (_e.name() == "fifths") {
            Key tKey = Key(_e.readElementText().toInt());
            Key cKey = tKey;
            Interval v = _pass1.getPart(partId)->instrument()->transpose();
            if (!v.isZero() && !_score->style().styleB(Sid::concertPitch)) {
                cKey = transposeKey(tKey, v);
                // if there are more than 6 accidentals in transposing key, it cannot be PreferSharpFlat::AUTO
                Part* part = _pass1.getPart(partId);
                if ((tKey > 6 || tKey < -6) && part->preferSharpFlat() == PreferSharpFlat::AUTO) {
                    part->setPreferSharpFlat(PreferSharpFlat::NONE);
                }
            }
            key.setConcertKey(cKey);
            key.setKey(tKey);
        } else if (_e.name() == "mode") {
            QString m = _e.readElementText();
            if (m == "none") {
                key.setCustom(true);
                key.setMode(KeyMode::NONE);
            } else if (m == "major") {
                key.setMode(KeyMode::MAJOR);
            } else if (m == "minor") {
                key.setMode(KeyMode::MINOR);
            } else if (m == "dorian") {
                key.setMode(KeyMode::DORIAN);
            } else if (m == "phrygian") {
                key.setMode(KeyMode::PHRYGIAN);
            } else if (m == "lydian") {
                key.setMode(KeyMode::LYDIAN);
            } else if (m == "mixolydian") {
                key.setMode(KeyMode::MIXOLYDIAN);
            } else if (m == "aeolian") {
                key.setMode(KeyMode::AEOLIAN);
            } else if (m == "ionian") {
                key.setMode(KeyMode::IONIAN);
            } else if (m == "locrian") {
                key.setMode(KeyMode::LOCRIAN);
            } else {
                _logger->logError(QString("Unsupported mode '%1'").arg(m), &_e);
            }
        } else if (_e.name() == "cancel") {
            skipLogCurrElem();        // TODO ??
        } else if (_e.name() == "key-step") {
            flushAlteredTone(key, keyStep, keyAlter, keyAccidental, smufl);
            keyStep = _e.readElementText();
        } else if (_e.name() == "key-alter") {
            keyAlter = _e.readElementText();
        } else if (_e.name() == "key-accidental") {
            smufl = _e.attributes().value("smufl").toString();
            keyAccidental = _e.readElementText();
        } else {
            skipLogCurrElem();
        }
    }
    flushAlteredTone(key, keyStep, keyAlter, keyAccidental, smufl);

    size_t nstaves = _pass1.getPart(partId)->nstaves();
    staff_idx_t staffIdx = _pass1.trackForPart(partId) / VOICES;
    if (keyno == -1) {
        // apply key to all staves in the part
        for (staff_idx_t i = 0; i < nstaves; ++i) {
            addKey(key, keyColor, printObject, _score, measure, staffIdx + i, tick);
        }
    } else if (keyno < static_cast<int>(nstaves)) {
        addKey(key, keyColor, printObject, _score, measure, staffIdx + keyno, tick);
    }
}

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 */

void MusicXMLParserPass2::clef(const QString& partId, Measure* measure, const Fraction& tick)
{
    ClefType clef   = ClefType::G;
    StaffTypes st = StaffTypes::STANDARD;

    QString c;
    int i = 0;
    int line = -1;

    const QString strClefno = _e.attributes().value("number").toString();
    const bool afterBarline = _e.attributes().value("after-barline") == "yes";
    const bool printObject = _e.attributes().value("print-object") != "no";
    const QColor clefColor { _e.attributes().value("color").toString() };

    while (_e.readNextStartElement()) {
        if (_e.name() == "sign") {
            c = _e.readElementText();
        } else if (_e.name() == "line") {
            line = _e.readElementText().toInt();
        } else if (_e.name() == "clef-octave-change") {
            i = _e.readElementText().toInt();
            if (i && !(c == "F" || c == "G" || c == "C")) {
                LOGD("clef-octave-change only implemented for F and G key");          // TODO
            }
        } else {
            skipLogCurrElem();
        }
    }

    //some software (Primus) don't include line and assume some default
    // it's permitted by MusicXML 2.0 XSD
    if (line == -1) {
        if (c == "G") {
            line = 2;
        } else if (c == "F") {
            line = 4;
        } else if (c == "C") {
            line = 3;
        }
    }

    if (c == "G" && i == 0 && line == 2) {
        clef = ClefType::G;
    } else if (c == "G" && i == 1 && line == 2) {
        clef = ClefType::G8_VA;
    } else if (c == "G" && i == 2 && line == 2) {
        clef = ClefType::G15_MA;
    } else if (c == "G" && i == -1 && line == 2) {
        clef = ClefType::G8_VB;
    } else if (c == "G" && i == -2 && line == 2) {
        clef = ClefType::G15_MB;
    } else if (c == "G" && i == 0 && line == 1) {
        clef = ClefType::G_1;
    } else if (c == "F" && i == 0 && line == 3) {
        clef = ClefType::F_B;
    } else if (c == "F" && i == 0 && line == 4) {
        clef = ClefType::F;
    } else if (c == "F" && i == 1 && line == 4) {
        clef = ClefType::F_8VA;
    } else if (c == "F" && i == 2 && line == 4) {
        clef = ClefType::F_15MA;
    } else if (c == "F" && i == -1 && line == 4) {
        clef = ClefType::F8_VB;
    } else if (c == "F" && i == -2 && line == 4) {
        clef = ClefType::F15_MB;
    } else if (c == "F" && i == 0 && line == 5) {
        clef = ClefType::F_C;
    } else if (c == "C") {
        if (line == 5) {
            clef = ClefType::C5;
        } else if (line == 4) {
            if (i == -1) {
                clef = ClefType::C4_8VB;
            } else {
                clef = ClefType::C4;
            }
        } else if (line == 3) {
            clef = ClefType::C3;
        } else if (line == 2) {
            clef = ClefType::C2;
        } else if (line == 1) {
            clef = ClefType::C1;
        }
    } else if (c == "percussion") {
        clef = ClefType::PERC;
        if (_hasDrumset) {
            st = StaffTypes::PERC_DEFAULT;
        }
    } else if (c == "TAB") {
        clef = ClefType::TAB;
        st= StaffTypes::TAB_DEFAULT;
    } else {
        LOGD("clef: unknown clef <sign=%s line=%d oct ch=%d>", qPrintable(c), line, i);      // TODO
    }

    Part* part = _pass1.getPart(partId);
    IF_ASSERT_FAILED(part) {
        return;
    }

    // TODO: check error handling for
    // - single staff
    // - multi-staff with same clef
    size_t clefno = 0;   // default
    if (strClefno != "") {
        clefno = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strClefno.toInt());
    }
    if (clefno >= part->nstaves()) {
        // conversion error (0) or other issue, assume staff 1
        // Also for Cubase 6.5.5 which generates clef number="2" in a single staff part
        // Same fix is required in pass 1 and pass 2
        _logger->logError(QString("invalid clef number '%1'").arg(strClefno), &_e);
        clefno = 0;
    }

    Segment* s;
    // check if the clef change needs to be in the previous measure
    if (!afterBarline && (tick == measure->tick()) && measure->prevMeasure()) {
        s = measure->prevMeasure()->getSegment(SegmentType::Clef, tick);
    } else {
        s = measure->getSegment(tick.isNotZero() ? SegmentType::Clef : SegmentType::HeaderClef, tick);
    }

    Clef* clefs = Factory::createClef(s);
    clefs->setClefType(clef);
    clefs->setVisible(printObject);
    if (clefColor.isValid()) {
        clefs->setColor(clefColor);
    }
    track_idx_t track = _pass1.trackForPart(partId) + clefno * VOICES;
    clefs->setTrack(track);
    s->add(clefs);

    // set the correct staff type
    // note that clef handling should probably done in pass1
    staff_idx_t staffIdx = _score->staffIdx(part) + clefno;
    int lines = _score->staff(staffIdx)->lines(Fraction(0, 1));
    if (tick.isZero()) {   // changing staff type not supported (yet ?)
        _score->staff(staffIdx)->setStaffType(tick, *StaffType::preset(st));
        _score->staff(staffIdx)->setLines(tick, lines);     // preserve previously set staff lines
        _score->staff(staffIdx)->setBarLineTo(0);        // default
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
    bts = 0;         // the beats (max 4 separated by "+") as integer
    btp = 0;         // beat-type as integer
    // determine if timesig is valid
    if (timeSymbol == "cut") {
        st = TimeSigType::ALLA_BREVE;
    } else if (timeSymbol == "common") {
        st = TimeSigType::FOUR_FOUR;
    } else if (!timeSymbol.isEmpty() && timeSymbol != "normal") {
        LOGD("determineTimeSig: time symbol <%s> not recognized", qPrintable(timeSymbol)); // TODO
        return false;
    }

    btp = beatType.toInt();
    QStringList list = beats.split("+");
    for (int i = 0; i < list.size(); i++) {
        bts += list.at(i).toInt();
    }

    // determine if bts and btp are valid
    if (bts <= 0 || btp <= 0) {
        LOGD("determineTimeSig: beats=%s and/or beat-type=%s not recognized",
             qPrintable(beats), qPrintable(beatType));               // TODO
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
    const QColor timeColor { _e.attributes().value("color").toString() };

    while (_e.readNextStartElement()) {
        if (_e.name() == "beats") {
            beats = _e.readElementText();
        } else if (_e.name() == "beat-type") {
            beatType = _e.readElementText();
        } else {
            skipLogCurrElem();
        }
    }

    if (beats != "" && beatType != "") {
        // determine if timesig is valid
        TimeSigType st  = TimeSigType::NORMAL;
        int bts = 0;     // total beats as integer (beats may contain multiple numbers, separated by "+")
        int btp = 0;     // beat-type as integer
        if (determineTimeSig(beats, beatType, timeSymbol, st, bts, btp)) {
            _timeSigDura = Fraction(bts, btp);
            Fraction fractionTSig = Fraction(bts, btp);
            for (size_t i = 0; i < _pass1.getPart(partId)->nstaves(); ++i) {
                Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
                TimeSig* timesig = Factory::createTimeSig(s);
                timesig->setVisible(printObject);
                if (timeColor.isValid()) {
                    timesig->setColor(timeColor);
                }
                track_idx_t track = _pass1.trackForPart(partId) + i * VOICES;
                timesig->setTrack(track);
                timesig->setSig(fractionTSig, st);
                // handle simple compound time signature
                if (beats.contains(QChar('+'))) {
                    timesig->setNumeratorString(beats);
                    timesig->setDenominatorString(beatType);
                }
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
    if (!(_divs > 0)) {
        _logger->logError("illegal divisions", &_e);
    }
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
    if (!rest) {
        return false;
    }

    if (!dura.isValid()) {
        return false;
    }

    if (!mDura.isValid()) {
        return false;
    }

    return (type == "" && dura == mDura)
           || (type == "whole" && dura == mDura && dura != Fraction(1, 1));
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
    //LOGD("determineDuration rest %d type '%s' dots %d dura %s mDura %s",
    //       rest, qPrintable(type), dots, qPrintable(dura.print()), qPrintable(mDura.print()));

    TDuration res;
    if (rest) {
        if (isWholeMeasureRest(rest, type, dura, mDura)) {
            res.setType(DurationType::V_MEASURE);
        } else if (type == "") {
            // If no type, set duration type based on duration.
            // Note that sometimes unusual duration (e.g. 261/256) are found.
            res.setVal(dura.ticks());
        } else {
            QByteArray ba = type.toLatin1();
            res.setType(TConv::fromXml(ba.constData(), DurationType::V_INVALID));
            res.setDots(dots);
        }
    } else {
        QByteArray ba = type.toLatin1();
        res.setType(TConv::fromXml(ba.constData(), DurationType::V_INVALID));
        res.setDots(dots);
        if (res.type() == DurationType::V_INVALID) {
            res.setType(DurationType::V_QUARTER);        // default, TODO: use dura ?
        }
    }

    //LOGD("-> dur %hhd (%s) dots %d ticks %s",
    //       res.type(), qPrintable(res.name()), res.dots(), qPrintable(dura.print()));

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

static Chord* findOrCreateChord(Score*, Measure* m,
                                const Fraction& tick, const int track, const int move,
                                const TDuration duration, const Fraction dura,
                                BeamMode bm, bool small)
{
    //LOGD("findOrCreateChord tick %d track %d dur ticks %d ticks %s bm %hhd",
    //       tick, track, duration.ticks(), qPrintable(dura.print()), bm);
    Chord* c = m->findChord(tick, track);
    if (c == 0) {
        Segment* s = m->getSegment(SegmentType::ChordRest, tick);
        c = Factory::createChord(s);
        // better not to force beam end, as the beam palette does not support it
        if (bm == BeamMode::END) {
            c->setBeamMode(BeamMode::AUTO);
        } else {
            c->setBeamMode(bm);
        }
        c->setTrack(track);
        // Chord is initialized with the smallness of its first note.
        // If a non-small note is added later, this is handled in handleSmallness.
        c->setSmall(small);

        setChordRestDuration(c, duration, dura);
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
    } else {
        if (duration.type() == DurationType::V_QUARTER) {
            nt = NoteType::GRACE4;
        } else if (duration.type() == DurationType::V_16TH) {
            nt = NoteType::GRACE16;
        } else if (duration.type() == DurationType::V_32ND) {
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
                               const TDuration duration, const bool slash, const bool small)
{
    Chord* c = Factory::createChord(score->dummy()->segment());
    c->setNoteType(graceNoteType(duration, slash));
    c->setTrack(track);
    // Chord is initialized with the smallness of its first note.
    // If a non-small note is added later, this is handled in handleSmallness.
    c->setSmall(small);
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
        //LOGD("rest step=%d oct=%d", step, octave);
        ClefType clef = cr->staff()->clef(tick);
        int po = ClefInfo::pitchOffset(clef);
        //LOGD(" clef=%hhd po=%d step=%d", clef, po, step);
        int dp = 7 * (octave + 2) + step;
        //LOGD(" dp=%d po-dp=%d", dp, po-dp);
        cr->ryoffset() = (po - dp + 3) * spatium / 2;
    }
}

//---------------------------------------------------------
//   handleSmallness
//---------------------------------------------------------

static void handleSmallness(bool cueOrSmall, Note* note, Chord* c)
{
    if (cueOrSmall) {
        note->setSmall(!c->isSmall()); // Avoid redundant smallness
    } else {
        note->setSmall(false);
        if (c->isSmall()) {
            // What was a small chord becomes small notes in a non-small chord
            c->setSmall(false);
            for (Note* otherNote : c->notes()) {
                if (note != otherNote) {
                    otherNote->setSmall(true);
                }
            }
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
    const auto score = note->score();

    if (noteheadColor.isValid()) {
        note->setColor(noteheadColor);
    }
    if (noteheadParentheses) {
        auto s = new Symbol(note);
        s->setSym(SymId::noteheadParenthesisLeft);
        s->setParent(note);
        score->addElement(s);
        s = new Symbol(note);
        s->setSym(SymId::noteheadParenthesisRight);
        s->setParent(note);
        score->addElement(s);
    }

    if (noteheadFilled == "no") {
        note->setHeadType(NoteHeadType::HEAD_HALF);
    } else if (noteheadFilled == "yes") {
        note->setHeadType(NoteHeadType::HEAD_QUARTER);
    }
}

//---------------------------------------------------------
//   computeBeamMode
//---------------------------------------------------------

/**
 Calculate the beam mode based on the collected beamTypes.
 */

static BeamMode computeBeamMode(const QMap<int, QString>& beamTypes)
{
    // Start with uniquely-handled beam modes
    if (beamTypes.value(1) == "continue"
        && beamTypes.value(2) == "begin") {
        return BeamMode::BEGIN16;
    } else if (beamTypes.value(1) == "continue"
               && beamTypes.value(2) == "continue"
               && beamTypes.value(3) == "begin") {
        return BeamMode::BEGIN32;
    }
    // Generic beam modes are naive to all except the first beam
    else if (beamTypes.value(1) == "begin") {
        return BeamMode::BEGIN;
    } else if (beamTypes.value(1) == "continue") {
        return BeamMode::MID;
    } else if (beamTypes.value(1) == "end") {
        return BeamMode::END;
    } else {
        // backward-hook, forward-hook, and other unknown combinations
        return BeamMode::AUTO;
    }
}

//---------------------------------------------------------
//   addFiguredBassElements
//---------------------------------------------------------

/**
 Add the figured bass elements.
 */

static void addFiguredBassElements(FiguredBassList& fbl, const Fraction noteStartTime, const int msTrack,
                                   const Fraction dura, Measure* measure)
{
    if (!fbl.isEmpty()) {
        auto sTick = noteStartTime;                  // starting tick
        foreach (FiguredBass* fb, fbl) {
            fb->setTrack(msTrack);
            // No duration tag defaults ticks() to 0; set to note value
            if (fb->ticks().isZero()) {
                fb->setTicks(dura);
            }
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
                       const int tremoloNr, const QString& tremoloType,
                       Chord*& tremStart,
                       MxmlLogger* logger, const QXmlStreamReader* const xmlreader,
                       Fraction& timeMod)
{
    if (!cr->isChord()) {
        return;
    }
    if (tremoloNr) {
        //LOGD("tremolo %d type '%s' ticks %d tremStart %p", tremoloNr, qPrintable(tremoloType), ticks, _tremStart);
        if (tremoloNr == 1 || tremoloNr == 2 || tremoloNr == 3 || tremoloNr == 4) {
            if (tremoloType == "" || tremoloType == "single") {
                const auto tremolo = Factory::createTremolo(mu::engraving::toChord(cr));
                switch (tremoloNr) {
                case 1: tremolo->setTremoloType(TremoloType::R8);
                    break;
                case 2: tremolo->setTremoloType(TremoloType::R16);
                    break;
                case 3: tremolo->setTremoloType(TremoloType::R32);
                    break;
                case 4: tremolo->setTremoloType(TremoloType::R64);
                    break;
                }
                cr->add(tremolo);
            } else if (tremoloType == "start") {
                if (tremStart) {
                    logger->logError("MusicXML::import: double tremolo start", xmlreader);
                }
                tremStart = static_cast<Chord*>(cr);
                // timeMod takes into account also the factor 2 of a two-note tremolo
                if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                    timeMod.setDenominator(timeMod.denominator() / 2);
                }
            } else if (tremoloType == "stop") {
                if (tremStart) {
                    const auto tremolo = Factory::createTremolo(mu::engraving::toChord(cr));
                    switch (tremoloNr) {
                    case 1: tremolo->setTremoloType(TremoloType::C8);
                        break;
                    case 2: tremolo->setTremoloType(TremoloType::C16);
                        break;
                    case 3: tremolo->setTremoloType(TremoloType::C32);
                        break;
                    case 4: tremolo->setTremoloType(TremoloType::C64);
                        break;
                    }
                    tremolo->setChords(tremStart, static_cast<Chord*>(cr));
                    // fixup chord duration and type
                    const Fraction tremDur = cr->ticks() * Fraction(1, 2);
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
                } else {
                    logger->logError("MusicXML::import: double tremolo stop w/o start", xmlreader);
                }
                tremStart = nullptr;
            }
        } else {
            logger->logError(QString("unknown tremolo type %1").arg(tremoloNr), xmlreader);
        }
    }
}

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

// TODO: refactor: optimize parameters

static void setPitch(Note* note, MusicXMLParserPass1& pass1, const QString& partId, const QString& instrumentId, const mxmlNotePitch& mnp,
                     const int octaveShift, const Instrument* const instrument)
{
    const auto& instruments = pass1.getInstruments(partId);
    if (mnp.unpitched()) {
        if (hasDrumset(instruments)
            && instruments.contains(instrumentId)) {
            // step and oct are display-step and ...-oct
            // get pitch from instrument definition in drumset instead
            int unpitched = instruments[instrumentId].unpitched;
            note->setPitch(std::clamp(unpitched, 0, 127));
            // TODO - does this need to be key-aware?
            note->setTpc(pitch2tpc(unpitched, Key::C, Prefer::NEAREST));             // TODO: necessary ?
        } else {
            //LOGD("disp step %d oct %d", displayStep, displayOctave);
            xmlSetPitch(note, mnp.displayStep(), 0, mnp.displayOctave(), 0, instrument);
        }
    } else {
        xmlSetPitch(note, mnp.step(), mnp.alter(), mnp.octave(), octaveShift, instrument);
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
                       const Fraction noteStartTime, const mxmlNotePitch& mnp, const DirectionV stemDir, const NoteHeadGroup headGroup)
{
    // determine staff line based on display-step / -octave and clef type
    const auto clef = c->staff()->clef(noteStartTime);
    const auto po = ClefInfo::pitchOffset(clef);
    const auto pitch = MusicXMLStepAltOct2Pitch(mnp.displayStep(), 0, mnp.displayOctave());
    auto line = po - absStep(pitch);

    // correct for number of staff lines
    // see ExportMusicXml::unpitch2xml for explanation
    // TODO handle other # staff lines ?
    int staffLines = c->staff()->lines(Fraction(0, 1));
    if (staffLines == 1) {
        line -= 8;
    }
    if (staffLines == 3) {
        line -= 2;
    }

    // the drum palette cannot handle stem direction AUTO,
    // overrule if necessary
    DirectionV overruledStemDir = stemDir;
    if (stemDir == DirectionV::AUTO) {
        if (line > 4) {
            overruledStemDir = DirectionV::DOWN;
        } else {
            overruledStemDir = DirectionV::UP;
        }
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
                                Tuplets& tuplets)
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
    int staff = 0;
    QString type;
    QString voice;
    DirectionV stemDir = DirectionV::AUTO;
    bool noStem = false;
    NoteHeadGroup headGroup = NoteHeadGroup::HEAD_NORMAL;
    const QColor noteColor { _e.attributes().value("color").toString() };
    QColor noteheadColor = QColor::Invalid;
    bool noteheadParentheses = false;
    QString noteheadFilled;
    int velocity = round(_e.attributes().value("dynamics").toDouble() * 0.9);
    bool graceSlash = false;
    bool printObject = _e.attributes().value("print-object") != "no";
    BeamMode bm;
    QMap<int, QString> beamTypes;
    QString instrumentId;
    QString tieType;
    MusicXMLParserLyric lyric { _pass1.getMusicXmlPart(partId).lyricNumberHandler(), _e, _score, _logger };
    MusicXMLParserNotations notations { _e, _score, _logger };

    mxmlNoteDuration mnd { _divs, _logger, &_pass1 };
    mxmlNotePitch mnp { _logger };

    while (_e.readNextStartElement()) {
        if (mnp.readProperties(_e, _score)) {
            // element handled
        } else if (mnd.readProperties(_e)) {
            // element handled
        } else if (_e.name() == "beam") {
            beam(beamTypes);
        } else if (_e.name() == "chord") {
            chord = true;
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "cue") {
            cue = true;
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "grace") {
            grace = true;
            graceSlash = _e.attributes().value("slash") == "yes";
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "instrument") {
            instrumentId = _e.attributes().value("id").toString();
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "lyric") {
            // lyrics on grace notes not (yet) supported by MuseScore
            if (!grace) {
                lyric.parse();
            } else {
                _logger->logDebugInfo("ignoring lyrics on grace notes", &_e);
                skipLogCurrElem();
            }
        } else if (_e.name() == "notations") {
            notations.parse();
            addError(notations.errors());
        } else if (_e.name() == "notehead") {
            noteheadColor.setNamedColor(_e.attributes().value("color").toString());
            noteheadParentheses = _e.attributes().value("parentheses") == "yes";
            noteheadFilled = _e.attributes().value("filled").toString();
            auto noteheadValue = _e.readElementText();
            if (noteheadValue != "none") {
                headGroup = convertNotehead(noteheadValue);
            }
        } else if (_e.name() == "rest") {
            rest = true;
            mnp.displayStepOctave(_e);
        } else if (_e.name() == "staff") {
            auto ok = false;
            auto strStaff = _e.readElementText();
            staff = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt(&ok));
            if (!ok) {
                // error already reported in pass 1
                staff = -1;
            }
        } else if (_e.name() == "stem") {
            stem(stemDir, noStem);
        } else if (_e.name() == "tie") {
            tieType = _e.attributes().value("type").toString();
            _e.skipCurrentElement();
        } else if (_e.name() == "type") {
            isSmall = _e.attributes().value("size") == "cue" || _e.attributes().value("size") == "grace-cue";
            type = _e.readElementText();
        } else if (_e.name() == "voice") {
            voice = _e.readElementText();
        } else {
            skipLogCurrElem();
        }
    }

    // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
    if (!chord) {
        // remember voice
        currentVoice = voice;
    } else if (voice == "") {
        // use voice from last note w/o <chord>
        voice = currentVoice;
    }

    // Assume voice 1 if voice is empty (legal in a single voice part)
    if (voice == "") {
        voice = "1";
    }

    // Define currBeam based on currentVoice to handle multi-voice beaming (and instantiate if not already)
    if (!currBeams.contains(currentVoice)) {
        currBeams.insert(currentVoice, (Beam*)nullptr);
    }
    Beam*& currBeam = currBeams[currentVoice];

    bm = computeBeamMode(beamTypes);

    // check for timing error(s) and set dura
    // keep in this order as checkTiming() might change dura
    auto errorStr = mnd.checkTiming(type, rest, grace);
    dura = mnd.duration();
    if (errorStr != "") {
        _logger->logError(errorStr, &_e);
    }

    IF_ASSERT_FAILED(_pass1.getPart(partId)) {
        return nullptr;
    }

    // At this point all checks have been done, the note should be added
    // note: in case of error exit from here, the postponed <note> children
    // must still be skipped

    int msMove = 0;
    int msTrack = 0;
    int msVoice = 0;

    int voiceInt = _pass1.voiceToInt(voice);
    if (!_pass1.determineStaffMoveVoice(partId, staff, voiceInt, msMove, msTrack, msVoice)) {
        _logger->logDebugInfo(QString("could not map staff %1 voice '%2'").arg(staff + 1).arg(voice), &_e);
        addError(checkAtEndElement(_e, "note"));
        return 0;
    }

    // start time for note:
    // - sTime for non-chord / first chord note
    // - prevTime for others
    auto noteStartTime = chord ? prevSTime : sTime;
    auto timeMod = mnd.timeMod();

    // determine tuplet state, used twice (before and after note allocation)
    MxmlTupletFlags tupletAction;

    // handle tuplet state for the previous chord or rest
    if (!chord && !grace) {
        auto& tuplet = tuplets[voice];
        auto& tupletState = tupletStates[voice];
        tupletAction = tupletState.determineTupletAction(mnd.duration(), timeMod, notations.tupletDesc().type,
                                                         mnd.normalType(), missingPrev, missingCurr);
        if (tupletAction & MxmlTupletFlag::STOP_PREVIOUS) {
            // tuplet start while already in tuplet
            if (missingPrev.isValid() && missingPrev > Fraction(0, 1)) {
                const auto track = msTrack + msVoice;
                const auto extraRest = addRest(_score, measure, noteStartTime, track, msMove,
                                               TDuration { missingPrev* tuplet->ratio() }, missingPrev);
                if (extraRest) {
                    extraRest->setTuplet(tuplet);
                    tuplet->add(extraRest);
                    noteStartTime += missingPrev;
                }
            }
            // recover by simply stopping the current tuplet first
            const auto normalNotes = timeMod.numerator();
            handleTupletStop(tuplet, normalNotes);
        }
    }

    Chord* c { nullptr };
    ChordRest* cr { nullptr };
    Note* note { nullptr };

    TDuration duration = determineDuration(rest, type, mnd.dots(), dura, measure->ticks());

    // begin allocation
    if (rest) {
        const auto track = msTrack + msVoice;
        cr = addRest(_score, measure, noteStartTime, track, msMove,
                     duration, dura);
    } else {
        if (!grace) {
            // regular note
            // if there is already a chord just add to it
            // else create a new one
            // this basically ignores <chord/> errors
            c = findOrCreateChord(_score, measure,
                                  noteStartTime,
                                  msTrack + msVoice, msMove,
                                  duration, dura, bm, isSmall || cue);
        } else {
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
            } else {
                c = gcl.last();
            }
        }
        note = Factory::createNote(c);
        const staff_idx_t ottavaStaff = (msTrack - _pass1.trackForPart(partId)) / VOICES;
        const int octaveShift = _pass1.octaveShift(partId, ottavaStaff, noteStartTime);
        const auto part = _pass1.getPart(partId);
        const auto instrument = part->instrument(noteStartTime);
        setPitch(note, _pass1, partId, instrumentId, mnp, octaveShift, instrument);
        c->add(note);
        //c->setStemDirection(stemDir); // already done in handleBeamAndStemDir()
        //c->setNoStem(noStem);
        cr = c;
    }
    // end allocation

    if (rest) {
        const track_idx_t track = msTrack + msVoice;
        if (cr) {
            if (currBeam) {
                if (currBeam->track() == track) {
                    cr->setBeamMode(BeamMode::MID);
                    currBeam->add(cr);
                } else {
                    removeBeam(currBeam);
                }
            } else {
                cr->setBeamMode(BeamMode::NONE);
            }
            cr->setSmall(isSmall);
            if (noteColor.isValid()) {
                cr->setColor(noteColor);
            }
            cr->setVisible(printObject);
            handleDisplayStep(cr, mnp.displayStep(), mnp.displayOctave(), noteStartTime, _score->style().spatium());
        }
    } else {
        handleSmallness(cue || isSmall, note, c);
        note->setPlay(!cue);          // cue notes don't play
        note->setHeadGroup(headGroup);
        if (noteColor.isValid()) {
            note->setColor(noteColor);
        }
        setNoteHead(note, noteheadColor, noteheadParentheses, noteheadFilled);
        note->setVisible(printObject); // TODO also set the stem to invisible

        if (!grace) {
            // regular note
            // handle beam
            if (!chord) {
                handleBeamAndStemDir(c, bm, stemDir, currBeam, _pass1.hasBeamingInfo());
            }

            // append any grace chord after chord to the previous chord
            const auto prevChord = measure->findChord(prevSTime, msTrack + msVoice);
            if (prevChord && prevChord != c) {
                addGraceChordsAfter(prevChord, gcl, gac);
            }

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
            nel.push_back(ne);
            note->setPlayEvents(nel);
            if (c) {
                c->setPlayEventType(PlayEventType::User);
            }
        }

        if (velocity > 0) {
            note->setUserVelocity(velocity);
        }

        if (mnp.unpitched()) {
            setDrumset(c, _pass1, partId, instrumentId, noteStartTime, mnp, stemDir, headGroup);
        }

        // accidental handling
        //LOGD("note acc %p type %hhd acctype %hhd",
        //       acc, acc ? acc->accidentalType() : static_cast<mu::engraving::AccidentalType>(0), accType);
        Accidental* acc = mnp.acc();
        if (!acc && mnp.accType() != AccidentalType::NONE) {
            acc = Factory::createAccidental(_score->dummy());
            acc->setAccidentalType(mnp.accType());
        }

        if (acc) {
            note->add(acc);
            // save alter value for user accidental
            if (acc->accidentalType() != AccidentalType::NONE) {
                alt = mnp.alter();
            }
        }

        c->setNoStem(noStem);
    }

    // cr can be 0 here (if a rest cannot be added)
    // TODO: complete and cleanup handling this case
    if (cr) {
        cr->setVisible(printObject);
    }

    // handle notations
    if (cr) {
        notations.addToScore(cr, note, noteStartTime.ticks(), _slurs, _glissandi, _spanners, _trills, _tie);

        // if no tie added yet, convert the "tie" into "tied" and add it.
        if (note && !note->tieFor() && !tieType.isEmpty()) {
            Notation notation { "tied" };
            const QString type2 { "type" };
            notation.addAttribute(&type2, &tieType);
            addTie(notation, _score, note, cr->track(), _tie, _logger, &_e);
        }
    }

    // handle grace after state: remember current grace list size
    if (grace && notations.mustStopGraceAFter()) {
        gac = gcl.size();
    }

    // handle tremolo before handling tuplet (two note tremolos modify timeMod)
    if (cr) {
        addTremolo(cr, notations.tremoloNr(), notations.tremoloType(), _tremStart, _logger, &_e, timeMod);
    }

    // handle tuplet state for the current chord or rest
    if (cr) {
        if (!chord && !grace) {
            auto& tuplet = tuplets[voice];
            // do tuplet if valid time-modification is not 1/1 and is not 1/2 (tremolo)
            // TODO: check interaction tuplet and tremolo handling
            if (timeMod.isValid() && timeMod != Fraction(1, 1) && timeMod != Fraction(1, 2)) {
                const auto actualNotes = timeMod.denominator();
                const auto normalNotes = timeMod.numerator();
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
                        LOGD("add missing %s to current tuplet", qPrintable(missingCurr.toString()));
                        const auto track = msTrack + msVoice;
                        const auto extraRest = addRest(_score, measure, noteStartTime + dura, track, msMove,
                                                       TDuration { missingCurr* tuplet->ratio() }, missingCurr);
                        if (extraRest) {
                            extraRest->setTuplet(tuplet);
                            tuplet->add(extraRest);
                        }
                    }
                    handleTupletStop(tuplet, normalNotes);
                }
            } else if (tuplet) {
                // stop any still incomplete tuplet
                handleTupletStop(tuplet, 2);
            }
        }
    }

    // add lyrics found by lyric
    if (cr) {
        // add lyrics and stop corresponding extends
        addLyrics(_logger, &_e, cr, lyric.numberedLyrics(), lyric.extendedLyrics(), _extendedLyrics);
        if (rest) {
            // stop all extends
            _extendedLyrics.setExtend(-1, cr->track(), cr->tick());
        }
    }

    // add figured bass element
    addFiguredBassElements(fbl, noteStartTime, msTrack, dura, measure);

    // convert to slash or rhythmic notation if needed
    // TODO in the case of slash notation, we assume that given notes do in fact correspond to slash beats
    if (c && _measureStyleSlash != MusicXmlSlash::NONE) {
        c->setSlash(true, _measureStyleSlash == MusicXmlSlash::SLASH);
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        dura.set(0, 1);
    }

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
        } else if (_e.name() == "duration") {
            duration(dura);
        } else if (_e.name() == "grace") {
            grace = true;
            _e.skipCurrentElement();  // skip but don't log
        } else {
            _e.skipCurrentElement();              // skip but don't log
        }
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        dura.set(0, 1);
    }

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
    dura.set(0, 0);          // invalid unless set correctly
    const auto elementText = _e.readElementText();
    if (elementText.toInt() > 0) {
        dura = _pass1.calcTicks(elementText.toInt(), _divs, &_e);
    } else {
        _logger->logError(QString("illegal duration %1").arg(dura.toString()), &_e);
    }
    //LOGD("duration %s valid %d", qPrintable(dura.print()), dura.isValid());
}

static FiguredBassItem::Modifier MusicXML2Modifier(const String prefix)
{
    if (prefix == u"sharp") {
        return FiguredBassItem::Modifier::SHARP;
    } else if (prefix == u"flat") {
        return FiguredBassItem::Modifier::FLAT;
    } else if (prefix == u"natural") {
        return FiguredBassItem::Modifier::NATURAL;
    } else if (prefix == u"double-sharp") {
        return FiguredBassItem::Modifier::DOUBLESHARP;
    } else if (prefix == u"flat-flat") {
        return FiguredBassItem::Modifier::DOUBLEFLAT;
    } else if (prefix == u"sharp-sharp") {
        return FiguredBassItem::Modifier::DOUBLESHARP;
    } else if (prefix == u"cross") {
        return FiguredBassItem::Modifier::CROSS;
    } else if (prefix == u"backslash") {
        return FiguredBassItem::Modifier::BACKSLASH;
    } else if (prefix == u"slash") {
        return FiguredBassItem::Modifier::SLASH;
    } else {
        return FiguredBassItem::Modifier::NONE;
    }
}

//---------------------------------------------------------
//   figure
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/harmony/figured-bass/figure node.
 Return the result as a FiguredBassItem.
 */

FiguredBassItem* MusicXMLParserPass2::figure(const int idx, const bool paren, FiguredBass* parent)
{
    FiguredBassItem* fgi = parent->createItem(idx);

    // read the figure
    while (_e.readNextStartElement()) {
        if (_e.name() == "extend") {
            QStringRef type = _e.attributes().value("type");
            if (type == "start") {
                fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
            } else if (type == "continue") {
                fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
            } else if (type == "stop") {
                fgi->setContLine(FiguredBassItem::ContLine::SIMPLE);
            }
            _e.skipCurrentElement();
        } else if (_e.name() == "figure-number") {
            const QColor color { _e.attributes().value("color").toString() };
            QString val = _e.readElementText();
            int iVal = val.toInt();
            // MusicXML spec states figure-number is a number
            // MuseScore can only handle single digit
            if (1 <= iVal && iVal <= 9) {
                fgi->setDigit(iVal);
                fgi->setColor(color);
            } else {
                _logger->logError(QString("incorrect figure-number '%1'").arg(val), &_e);
            }
        } else if (_e.name() == "prefix") {
            fgi->setPrefix(MusicXML2Modifier(_e.readElementText()));
        } else if (_e.name() == "suffix") {
            fgi->setSuffix(MusicXML2Modifier(_e.readElementText()));
        } else {
            skipLogCurrElem();
        }
    }

    // set parentheses
    if (paren) {
        // parenthesis open
        if (fgi->prefix() != FiguredBassItem::Modifier::NONE) {
            fgi->setParenth1(FiguredBassItem::Parenthesis::ROUNDOPEN);              // before prefix
        } else if (fgi->digit() != FBIDigitNone) {
            fgi->setParenth2(FiguredBassItem::Parenthesis::ROUNDOPEN);              // before digit
        } else if (fgi->suffix() != FiguredBassItem::Modifier::NONE) {
            fgi->setParenth3(FiguredBassItem::Parenthesis::ROUNDOPEN);              // before suffix
        }
        // parenthesis close
        if (fgi->suffix() != FiguredBassItem::Modifier::NONE) {
            fgi->setParenth4(FiguredBassItem::Parenthesis::ROUNDCLOSED);              // after suffix
        } else if (fgi->digit() != FBIDigitNone) {
            fgi->setParenth3(FiguredBassItem::Parenthesis::ROUNDCLOSED);              // after digit
        } else if (fgi->prefix() != FiguredBassItem::Modifier::NONE) {
            fgi->setParenth2(FiguredBassItem::Parenthesis::ROUNDCLOSED);              // after prefix
        }
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
    FiguredBass* fb = Factory::createFiguredBass(_score->dummy()->segment());

    const bool parentheses = _e.attributes().value("parentheses") == "yes";
    const bool printObject = _e.attributes().value("print-object") != "no";
    const QString placement = _e.attributes().value("placement").toString();
    const QColor color { _e.attributes().value("color").toString() };

    fb->setVisible(printObject);
    if (color.isValid()) {
        fb->setColor(color);
    }

    QString normalizedText;
    int idx = 0;
    while (_e.readNextStartElement()) {
        if (_e.name() == "duration") {
            Fraction dura;
            duration(dura);
            if (dura.isValid() && dura > Fraction(0, 1)) {
                fb->setTicks(dura);
            }
        } else if (_e.name() == "figure") {
            FiguredBassItem* pItem = figure(idx++, parentheses, fb);
            pItem->setTrack(0 /* TODO fb->track() */);
            pItem->setParent(fb);
            fb->appendItem(pItem);
            // add item normalized text
            if (!normalizedText.isEmpty()) {
                normalizedText.append('\n');
            }
            normalizedText.append(pItem->normalizedText());
        } else {
            skipLogCurrElem();
            delete fb;
            return 0;
        }
    }

    fb->setXmlText(normalizedText);                          // this is the text to show while editing

    fb->setPlacement(placement == "above" ? PlacementV::ABOVE : PlacementV::BELOW);
    fb->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);

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

FretDiagram* MusicXMLParserPass2::frame()
{
    FretDiagram* fd = Factory::createFretDiagram(_score->dummy()->segment());

    // Format: fret: string
    std::map<int, int> bStarts;
    std::map<int, int> bEnds;

    while (_e.readNextStartElement()) {
        if (_e.name() == "first-fret") {
            bool ok {};
            int val = _e.readElementText().toInt(&ok);
            if (ok && val > 0) {
                fd->setFretOffset(val - 1);
            } else {
                _logger->logError(QString("FretDiagram::readMusicXML: illegal first-fret %1").arg(val), &_e);
            }
        } else if (_e.name() == "frame-frets") {
            int val = _e.readElementText().toInt();
            if (val > 0) {
                fd->setProperty(Pid::FRET_FRETS, val);
                fd->setPropertyFlags(Pid::FRET_FRETS, PropertyFlags::UNSTYLED);
            } else {
                _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-fret %1").arg(val), &_e);
            }
        } else if (_e.name() == "frame-note") {
            int fret   = -1;
            int string = -1;
            int actualString = -1;
            while (_e.readNextStartElement()) {
                if (_e.name() == "fret") {
                    fret = _e.readElementText().toInt();
                } else if (_e.name() == "string") {
                    string = _e.readElementText().toInt();
                    actualString = fd->strings() - string;
                } else if (_e.name() == "barre") {
                    // Keep barres to be added later
                    QString t = _e.attributes().value("type").toString();
                    if (t == "start") {
                        bStarts[fret] = actualString;
                    } else if (t == "stop") {
                        bEnds[fret] = actualString;
                    } else {
                        _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-note barre type %1").arg(t), &_e);
                    }
                    skipLogCurrElem();
                } else {
                    skipLogCurrElem();
                }
            }
            _logger->logDebugInfo(QString("FretDiagram::readMusicXML string %1 fret %2").arg(string).arg(fret), &_e);

            if (string > 0) {
                if (fret == 0) {
                    fd->setMarker(actualString, FretMarkerType::CIRCLE);
                } else if (fret > 0) {
                    if (fd->marker(actualString).mtype == FretMarkerType::CROSS) {
                        fd->setMarker(actualString, FretMarkerType::NONE);
                    }
                    fd->setDot(actualString, fret - fd->fretOffset(), true);
                }
            } else {
                _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-note string %1").arg(string), &_e);
            }
        } else if (_e.name() == "frame-strings") {
            int val = _e.readElementText().toInt();
            if (val > 0) {
                fd->setStrings(val);
                for (int i = 0; i < val; ++i) {
                    // MXML Spec: any string without a dot or other marker has a closed string
                    // cross marker above it.
                    fd->setMarker(i, FretMarkerType::CROSS);
                }
            } else {
                _logger->logError(QString("FretDiagram::readMusicXML: illegal frame-strings %1").arg(val), &_e);
            }
        } else {
            skipLogCurrElem();
        }
    }

    // Finally add barres
    for (auto const& i : bStarts) {
        int fret = i.first;
        int startString = i.second;

        if (bEnds.find(fret) == bEnds.end()) {
            continue;
        }

        int endString = bEnds[fret];
        fd->setBarre(startString, endString, fret - fd->fretOffset());
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
    track_idx_t track = _pass1.trackForPart(partId);

    bool printObject = _e.attributes().value("print-object") != "no";

    QString kind, kindText, functionText, symbols, parens;
    std::list<HDegree> degreeList;

    FretDiagram* fd = 0;
    Harmony* ha = Factory::createHarmony(_score->dummy()->segment());
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
                    if (_e.attributes().hasAttribute("text")) {
                        if (_e.attributes().value("text").toString() == "") {
                            invalidRoot = true;
                        }
                    }
                } else if (_e.name() == "root-alter") {
                    // attributes: print-object, print-style
                    //             location (left-right)
                    alter = _e.readElementText().toInt();
                } else {
                    skipLogCurrElem();
                }
            }
            if (invalidRoot) {
                ha->setRootTpc(Tpc::TPC_INVALID);
            } else {
                ha->setRootTpc(step2tpc(step, AccidentalVal(alter)));
            }
        } else if (_e.name() == "function") {
            // attributes: print-style
            ha->setRootTpc(Tpc::TPC_INVALID);
            ha->setBaseTpc(Tpc::TPC_INVALID);
            functionText = _e.readElementText();
            // TODO: parse to decide between ROMAN and NASHVILLE
            ha->setHarmonyType(HarmonyType::ROMAN);
        } else if (_e.name() == "kind") {
            // attributes: use-symbols  yes-no
            //             text, stack-degrees, parentheses-degree, bracket-degrees,
            //             print-style, halign, valign
            kindText = _e.attributes().value("text").toString();
            symbols = _e.attributes().value("use-symbols").toString();
            parens = _e.attributes().value("parentheses-degrees").toString();
            kind = _e.readElementText();
            if (kind == "none") {
                ha->setRootTpc(Tpc::TPC_INVALID);
            }
        } else if (_e.name() == "inversion") {
            // attributes: print-style
            skipLogCurrElem();
        } else if (_e.name() == "bass") {
            QString step;
            int alter = 0;
            while (_e.readNextStartElement()) {
                if (_e.name() == "bass-step") {
                    // attributes: print-style
                    step = _e.readElementText();
                } else if (_e.name() == "bass-alter") {
                    // attributes: print-object, print-style
                    //             location (left-right)
                    alter = _e.readElementText().toInt();
                } else {
                    skipLogCurrElem();
                }
            }
            ha->setBaseTpc(step2tpc(step, AccidentalVal(alter)));
        } else if (_e.name() == "degree") {
            int degreeValue = 0;
            int degreeAlter = 0;
            QString degreeType = "";
            while (_e.readNextStartElement()) {
                if (_e.name() == "degree-value") {
                    degreeValue = _e.readElementText().toInt();
                } else if (_e.name() == "degree-alter") {
                    degreeAlter = _e.readElementText().toInt();
                } else if (_e.name() == "degree-type") {
                    degreeType = _e.readElementText();
                } else {
                    skipLogCurrElem();
                }
            }
            if (degreeValue <= 0 || degreeValue > 13
                || degreeAlter < -2 || degreeAlter > 2
                || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                _logger->logError(QString("incorrect degree: degreeValue=%1 degreeAlter=%2 degreeType=%3")
                                  .arg(degreeValue).arg(degreeAlter).arg(degreeType), &_e);
            } else {
                if (degreeType == "add") {
                    degreeList.push_back(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                } else if (degreeType == "alter") {
                    degreeList.push_back(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                } else if (degreeType == "subtract") {
                    degreeList.push_back(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                }
            }
        } else if (_e.name() == "frame") {
            fd = frame();
        } else if (_e.name() == "level") {
            skipLogCurrElem();
        } else if (_e.name() == "offset") {
            offset = _pass1.calcTicks(_e.readElementText().toInt(), _divs, &_e);
            preventNegativeTick(sTime, offset, _logger);
        } else if (_e.name() == "staff") {
            size_t nstaves = _pass1.getPart(partId)->nstaves();
            QString strStaff = _e.readElementText();
            int staff = _pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt());
            if (staff >= 0 && staff < int(nstaves)) {
                track += staff * VOICES;
            } else {
                _logger->logError(QString("invalid staff %1").arg(strStaff), &_e);
            }
        } else {
            skipLogCurrElem();
        }
    }

    if (fd) {
        fd->setTrack(track);
        Segment* s = measure->getSegment(SegmentType::ChordRest, sTime + offset);
        s->add(fd);
    }

    const ChordDescription* d = 0;
    if (ha->rootTpc() != Tpc::TPC_INVALID) {
        d = ha->fromXml(kind, kindText, symbols, parens, degreeList);
    }
    if (d) {
        ha->setId(d->id);
        ha->setTextName(d->names.front());
    } else {
        ha->setId(-1);
        QString textName = functionText + kindText;
        ha->setTextName(textName);
    }
    ha->render();

    ha->setVisible(printObject);

    // TODO-LV: do this only if ha points to a valid harmony
    // harmony = ha;
    ha->setTrack(track);
    Segment* s = measure->getSegment(SegmentType::ChordRest, sTime + offset);
    s->add(ha);
}

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/beam node.
 Collects beamTypes, used in computeBeamMode.
 */

void MusicXMLParserPass2::beam(QMap<int, QString>& beamTypes)
{
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
        if (_e.name() == "duration") {
            duration(dura);
        } else if (_e.name() == "staff") {
            _e.skipCurrentElement();        // skip but don't log
        } else if (_e.name() == "voice") {
            _e.skipCurrentElement();        // skip but don't log
        } else {
            skipLogCurrElem();
        }
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
        if (_e.name() == "duration") {
            duration(dura);
        } else {
            skipLogCurrElem();
        }
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

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

void MusicXMLParserLyric::parse()
{
    std::unique_ptr<Lyrics> lyric { Factory::createLyrics(_score->dummy()->chord()) };
    // TODO in addlyrics: l->setTrack(trk);

    bool hasExtend = false;
    const auto lyricNumber = _e.attributes().value("number").toString();
    const QColor lyricColor { _e.attributes().value("color").toString() };
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
            formattedText += xmlpass2::nextPartOfFormattedString(_e);
        } else if (_e.name() == "extend") {
            hasExtend = true;
            extendType = _e.attributes().value("type").toString();
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "syllabic") {
            auto syll = _e.readElementText();
            if (syll == "single") {
                lyric->setSyllabic(LyricsSyllabic::SINGLE);
            } else if (syll == "begin") {
                lyric->setSyllabic(LyricsSyllabic::BEGIN);
            } else if (syll == "end") {
                lyric->setSyllabic(LyricsSyllabic::END);
            } else if (syll == "middle") {
                lyric->setSyllabic(LyricsSyllabic::MIDDLE);
            } else {
                LOGD("unknown syllabic %s", qPrintable(syll));                      // TODO
            }
        } else if (_e.name() == "text") {
            formattedText += xmlpass2::nextPartOfFormattedString(_e);
        } else {
            skipLogCurrElem();
        }
    }

    // if no lyric read (e.g. only 'extend "type=stop"'), no further action required
    if (formattedText == "") {
        return;
    }

    const auto lyricNo = _lyricNumberHandler.getLyricNo(lyricNumber);
    if (lyricNo < 0) {
        _logger->logError("invalid lyrics number (<0)", &_e);
        return;
    } else if (lyricNo > MAX_LYRICS) {
        _logger->logError(QString("too much lyrics (>%1)").arg(MAX_LYRICS), &_e);
        return;
    } else if (_numberedLyrics.contains(lyricNo)) {
        _logger->logError(QString("duplicate lyrics number (%1)").arg(lyricNumber), &_e);
        return;
    }

    //LOGD("formatted lyric '%s'", qPrintable(formattedText));
    lyric->setXmlText(formattedText);
    if (lyricColor.isValid()) {
        lyric->setProperty(Pid::COLOR, mu::draw::Color::fromQColor(lyricColor));
        lyric->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
    }

    const auto l = lyric.release();
    _numberedLyrics[lyricNo] = l;

    if (hasExtend
        && (extendType == "" || extendType == "start")
        && (l->syllabic() == LyricsSyllabic::SINGLE || l->syllabic() == LyricsSyllabic::END)) {
        _extendedLyrics.insert(l);
    }
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
    } else if (notation.attribute("type") == "start") {
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
    auto slurNo = notation.attribute("number").toInt();
    if (slurNo > 0) {
        slurNo--;
    }
    const auto slurType = notation.attribute("type");

    const auto track = cr->track();
    auto score = cr->score();

    // PriMus Music-Notation by Columbussoft (build 10093) generates overlapping
    // slurs that do not have a number attribute to distinguish them.
    // The duplicates must be ignored, to prevent memory allocation issues,
    // which caused a MuseScore crash
    // Similar issues happen with Sibelius 7.1.3 (direct export)

    if (slurType == "start") {
        if (slurs[slurNo].isStart()) {
            // slur start when slur already started: report error
            logger->logError(QString("ignoring duplicate slur start"), xmlreader);
        } else if (slurs[slurNo].isStop()) {
            // slur start when slur already stopped: wrap up
            auto newSlur = slurs[slurNo].slur();
            newSlur->setTick(Fraction::fromTicks(tick));
            newSlur->setStartElement(cr);
            slurs[slurNo] = SlurDesc();
        } else {
            // slur start for new slur: init
            auto newSlur = Factory::createSlur(score->dummy());
            if (cr->isGrace()) {
                newSlur->setAnchor(Spanner::Anchor::CHORD);
            }
            const auto lineType = notation.attribute("line-type");
            if (lineType == "dashed") {
                newSlur->setStyleType(SlurStyleType::Dashed);
            } else if (lineType == "dotted") {
                newSlur->setStyleType(SlurStyleType::Dotted);
            } else if (lineType == "solid" || lineType == "") {
                newSlur->setStyleType(SlurStyleType::Solid);
            }
            const QColor color { notation.attribute("color") };
            if (color.isValid()) {
                newSlur->setColor(color);
            }
            newSlur->setTick(Fraction::fromTicks(tick));
            newSlur->setStartElement(cr);
            if (configuration()->musicxmlImportLayout()) {
                const auto orientation = notation.attribute("orientation");
                const auto placement = notation.attribute("placement");
                if (orientation == "over" || placement == "above") {
                    newSlur->setSlurDirection(DirectionV::UP);
                } else if (orientation == "under" || placement == "below") {
                    newSlur->setSlurDirection(DirectionV::DOWN);
                } else if (orientation == "" || placement == "") {
                    // ignore
                } else {
                    logger->logError(QString("unknown slur orientation/placement: %1/%2").arg(orientation).arg(placement), xmlreader);
                }
            }
            newSlur->setTrack(track);
            newSlur->setTrack2(track);
            slurs[slurNo].start(newSlur);
            score->addElement(newSlur);
        }
    } else if (slurType == "stop") {
        if (slurs[slurNo].isStart()) {
            // slur stop when slur already started: wrap up
            auto newSlur = slurs[slurNo].slur();
            if (!(cr->isGrace())) {
                newSlur->setTick2(Fraction::fromTicks(tick));
                newSlur->setTrack2(track);
            }
            newSlur->setEndElement(cr);
            slurs[slurNo] = SlurDesc();
        } else if (slurs[slurNo].isStop()) {
            // slur stop when slur already stopped: report error
            logger->logError(QString("ignoring duplicate slur stop"), xmlreader);
        } else {
            // slur stop for new slur: init
            auto newSlur = Factory::createSlur(score->dummy());
            if (!(cr->isGrace())) {
                newSlur->setTick2(Fraction::fromTicks(tick));
                newSlur->setTrack2(track);
            }
            newSlur->setEndElement(cr);
            slurs[slurNo].stop(newSlur);
        }
    } else if (slurType == "continue") {
        // ignore
    } else {
        logger->logError(QString("unknown slur type %1").arg(slurType), xmlreader);
    }
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
    _dynamicsPlacement = _e.attributes().value("placement").toString();

    while (_e.readNextStartElement()) {
        if (_e.name() == "other-dynamics") {
            _dynamicsList.push_back(_e.readElementText());
        } else {
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
        SymId id { SymId::noSym };
        if (convertArticulationToSymId(_e.name().toString(), id)) {
            if (_e.name() == "detached-legato") {
                _notations.push_back(Notation::notationWithAttributes("tenuto",
                                                                      _e.attributes(), "articulations", SymId::articTenutoAbove));
                _notations.push_back(Notation::notationWithAttributes("staccato",
                                                                      _e.attributes(), "articulations", SymId::articStaccatoAbove));
            } else {
                Notation artic = Notation::notationWithAttributes(_e.name().toString(),
                                                                  _e.attributes(), "articulations", id);
                _notations.push_back(artic);
            }
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "breath-mark") {
            auto value = _e.readElementText();
            if (value == "tick") {
                _breath = SymId::breathMarkTick;
            } else if (value == "upbow") {
                _breath = SymId::breathMarkUpbow;
            } else if (value == "salzedo") {
                _breath = SymId::breathMarkSalzedo;
            } else {
                // Use comma as the default symbol
                _breath = SymId::breathMarkComma;
            }
        } else if (_e.name() == "caesura") {
            _breath = SymId::caesura;
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "doit"
                   || _e.name() == "falloff"
                   || _e.name() == "plop"
                   || _e.name() == "scoop") {
            Notation artic = Notation::notationWithAttributes("chord-line",
                                                              _e.attributes(), "articulations");
            artic.setSubType(_e.name().toString());
            _notations.push_back(artic);
            _e.skipCurrentElement();  // skip but don't log
        } else {
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
        SymId id { SymId::noSym };
        if (convertArticulationToSymId(_e.name().toString(), id)) {
            Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                 _e.attributes(), "articulations", id);
            _notations.push_back(notation);
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "trill-mark") {
            trillMark = true;
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "wavy-line") {
            auto wavyLineTypeWasStart = (_wavyLineType == "start");
            _wavyLineType = _e.attributes().value("type").toString();
            _wavyLineNo   = _e.attributes().value("number").toString().toInt();
            if (_wavyLineNo > 0) {
                _wavyLineNo--;
            }
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
        } else if (_e.name() == "tremolo") {
            _tremoloType = _e.attributes().value("type").toString();
            _tremoloNr = _e.readElementText().toInt();
        } else if (_e.name() == "inverted-mordent"
                   || _e.name() == "mordent") {
            mordentNormalOrInverted();
        } else if (_e.name() == "other-ornament") {
            Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                 _e.attributes(), "ornaments");
            _notations.push_back(notation);
            _e.skipCurrentElement();  // skip but don't log
        } else {
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
        SymId id { SymId::noSym };
        if (convertArticulationToSymId(_e.name().toString(), id)) {
            Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                 _e.attributes(), "technical", id);
            _notations.push_back(notation);
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "fingering" || _e.name() == "fret" || _e.name() == "pluck" || _e.name() == "string") {
            Notation notation = Notation::notationWithAttributes(_e.name().toString(),
                                                                 _e.attributes(), "technical");
            notation.setText(_e.readElementText());
            _notations.push_back(notation);
        } else if (_e.name() == "harmonic") {
            harmonic();
        } else {
            skipLogCurrElem();
        }
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
        } else {   // TODO: add artificial harmonic when supported by musescore
            _logger->logError(QString("unsupported harmonic type/pitch '%1'").arg(name), &_e);
            _e.skipCurrentElement();
        }
    }

    if (notation.subType() != "") {
        _notations.push_back(notation);
    }
}

//---------------------------------------------------------
//   addTechnical
//---------------------------------------------------------

void MusicXMLParserNotations::addTechnical(const Notation& notation, Note* note)
{
    QString placement = notation.attribute("placement");
    QString fontWeight = notation.attribute("font-weight");
    qreal fontSize = notation.attribute("font-size").toDouble();
    QString fontStyle = notation.attribute("font-style");
    QString fontFamily = notation.attribute("font-family");
    if (notation.name() == "fingering") {
        // TODO: distinguish between keyboards (style TextStyleName::FINGERING)
        // and (plucked) strings (style TextStyleName::LH_GUITAR_FINGERING)
        addTextToNote(_e.lineNumber(), _e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                      TextStyleType::FINGERING, _score, note);
    } else if (notation.name() == "fret") {
        auto fret = notation.text().toInt();
        if (note) {
            if (note->staff()->isTabStaff(Fraction(0, 1))) {
                note->setFret(fret);
            }
        } else {
            _logger->logError("no note for fret", &_e);
        }
    } else if (notation.name() == "pluck") {
        addTextToNote(_e.lineNumber(), _e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                      TextStyleType::RH_GUITAR_FINGERING, _score, note);
    } else if (notation.name() == "string") {
        if (note) {
            if (note->staff()->isTabStaff(Fraction(0, 1))) {
                note->setString(notation.text().toInt() - 1);
            } else {
                addTextToNote(_e.lineNumber(), _e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                              TextStyleType::STRING_NUMBER, _score, note);
            }
        } else {
            _logger->logError("no note for string", &_e);
        }
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
    auto glissandoNumber = notation.attribute("number").toInt();
    if (glissandoNumber > 0) {
        glissandoNumber--;
    }
    const auto glissandoType = notation.attribute("type");
    int glissandoTag = notation.name() == "slide" ? 0 : 1;
    //                  QString lineType  = ee.attribute(QString("line-type"), "solid");
    Glissando*& gliss = glissandi[glissandoNumber][glissandoTag];

    const auto tick = note->tick();
    const auto track = note->track();

    if (glissandoType == "start") {
        const QColor glissandoColor { notation.attribute("color") };
        const auto glissandoText = notation.text();
        if (gliss) {
            logger->logError(QString("overlapping glissando/slide number %1").arg(glissandoNumber + 1), xmlreader);
        } else if (!note) {
            logger->logError(QString("no note for glissando/slide number %1 start").arg(glissandoNumber + 1), xmlreader);
        } else {
            gliss = Factory::createGlissando(note);
            gliss->setAnchor(Spanner::Anchor::NOTE);
            gliss->setStartElement(note);
            gliss->setTick(tick);
            gliss->setTrack(track);
            gliss->setParent(note);
            if (glissandoColor.isValid()) {
                gliss->setLineColor(glissandoColor);
            }
            gliss->setText(glissandoText);
            gliss->setGlissandoType(glissandoTag == 0 ? GlissandoType::STRAIGHT : GlissandoType::WAVY);
            spanners[gliss] = QPair<int, int>(tick.ticks(), -1);
            // LOGD("glissando/slide=%p inserted at first tick %d", gliss, tick);
        }
    } else if (glissandoType == "stop") {
        if (!gliss) {
            logger->logError(QString("glissando/slide number %1 stop without start").arg(glissandoNumber + 1), xmlreader);
        } else if (!note) {
            logger->logError(QString("no note for glissando/slide number %1 stop").arg(glissandoNumber + 1), xmlreader);
        } else {
            spanners[gliss].second = tick.ticks() + note->chord()->ticks().ticks();
            gliss->setEndElement(note);
            gliss->setTick2(tick);
            gliss->setTrack2(track);
            // LOGD("glissando/slide=%p second tick %d", gliss, tick);
            gliss = nullptr;
        }
    } else {
        logger->logError(QString("unknown glissando/slide type %1").arg(glissandoType), xmlreader);
    }
}

//---------------------------------------------------------
//   addArpeggio
//---------------------------------------------------------

static void addArpeggio(ChordRest* cr, const QString& arpeggioType,
                        MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
{
    // no support for arpeggio on rest
    if (!arpeggioType.isEmpty() && cr->type() == ElementType::CHORD) {
        Arpeggio* arpeggio = Factory::createArpeggio(mu::engraving::toChord(cr));
        arpeggio->setArpeggioType(ArpeggioType::NORMAL);
        if (arpeggioType == "up") {
            arpeggio->setArpeggioType(ArpeggioType::UP);
        } else if (arpeggioType == "down") {
            arpeggio->setArpeggioType(ArpeggioType::DOWN);
        } else if (arpeggioType == "non-arpeggiate") {
            arpeggio->setArpeggioType(ArpeggioType::BRACKET);
        } else {
            logger->logError(QString("unknown arpeggio type %1").arg(arpeggioType), xmlreader);
        }
        // there can be only one
        if (!(static_cast<Chord*>(cr))->arpeggio()) {
            cr->add(arpeggio);
        }
    }
}

//---------------------------------------------------------
//   addArticLaissezVibrer
//---------------------------------------------------------

static void addArticLaissezVibrer(const Note* const note)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    auto chord = note->chord();
    if (!findLaissezVibrer(chord)) {
        Articulation* na = Factory::createArticulation(chord);
        na->setSymId(SymId::articLaissezVibrerBelow);
        chord->add(na);
    }
}

//---------------------------------------------------------
//   addTie
//---------------------------------------------------------

static void addTie(const Notation& notation, Score* score, Note* note, const track_idx_t track,
                   Tie*& tie, MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    const QString& type = notation.attribute("type");
    const QString& orientation = notation.attribute("orientation");
    const QString& placement = notation.attribute("placement");
    const QString& lineType = notation.attribute("line-type");

    if (type == "") {
        // ignore, nothing to do
    } else if (type == "start") {
        if (tie) {
            logger->logError(QString("Tie already active"), xmlreader);
        }
        tie = new Tie(score->dummy());
        note->setTieFor(tie);
        tie->setStartNote(note);
        tie->setTrack(track);

        const QColor color { notation.attribute("color") };
        if (color.isValid()) {
            tie->setColor(color);
        }

        if (configuration()->musicxmlImportLayout()) {
            if (orientation == "over" || placement == "above") {
                tie->setSlurDirection(DirectionV::UP);
            } else if (orientation == "under" || placement == "below") {
                tie->setSlurDirection(DirectionV::DOWN);
            } else if (orientation == "" || placement == "") {
                // ignore
            } else {
                logger->logError(QString("unknown tied orientation/placement: %1/%2").arg(orientation).arg(placement), xmlreader);
            }
        }

        if (lineType == "dashed") {
            tie->setStyleType(SlurStyleType::Dashed);
        } else if (lineType == "dotted") {
            tie->setStyleType(SlurStyleType::Dotted);
        } else if (lineType == "solid" || lineType == "") {
            tie->setStyleType(SlurStyleType::Solid);
        }
        tie = nullptr;
    } else if (type == "stop") {
        // ignore
    } else if (type == "let-ring") {
        addArticLaissezVibrer(note);
    } else {
        logger->logError(QString("unknown tied type %1").arg(type), xmlreader);
    }
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
        const auto ticks = cr->ticks();
        const auto track = cr->track();
        const auto trk = (track / VOICES) * VOICES;           // first track of staff
        Trill*& trill = trills[wavyLineNo];
        if (wavyLineType == "start" || wavyLineType == "startstop") {
            if (trill) {
                logger->logError(QString("overlapping wavy-line number %1").arg(wavyLineNo + 1), xmlreader);
            } else {
                trill = Factory::createTrill(cr->score()->dummy());
                trill->setTrack(trk);
                if (wavyLineType == "start") {
                    spanners[trill] = QPair<int, int>(tick.ticks(), -1);
                    // LOGD("trill=%p inserted at first tick %d", trill, tick);
                }
                if (wavyLineType == "startstop") {
                    spanners[trill] = QPair<int, int>(tick.ticks(), tick.ticks() + ticks.ticks());
                    trill = nullptr;
                    // LOGD("trill=%p inserted at first tick %d second tick %d", trill, tick, tick);
                }
            }
        } else if (wavyLineType == "stop") {
            if (!trill) {
                logger->logError(QString("wavy-line number %1 stop without start").arg(wavyLineNo + 1), xmlreader);
            } else {
                spanners[trill].second = tick.ticks() + ticks.ticks();
                // LOGD("trill=%p second tick %d", trill, tick);
                trill = nullptr;
            }
        } else {
            logger->logError(QString("unknown wavy-line type %1").arg(wavyLineType), xmlreader);
        }
    }
}

//---------------------------------------------------------
//   addBreath
//---------------------------------------------------------

static void addBreath(ChordRest* cr, const Fraction& tick, SymId breath)
{
    if (breath != SymId::noSym && !cr->isGrace()) {
        const Fraction& ticks = cr->ticks();
        const auto seg = cr->measure()->getSegment(SegmentType::Breath, tick + ticks);
        const auto b = Factory::createBreath(seg);
        // b->setTrack(trk + voice); TODO check next line
        b->setTrack(cr->track());
        b->setSymId(breath);
        seg->add(b);
    }
}

//---------------------------------------------------------
//   addChordLine
//---------------------------------------------------------

static void addChordLine(const Notation& notation, Note* note,
                         MxmlLogger* logger, const QXmlStreamReader* const xmlreader)
{
    const QString& chordLineType = notation.subType();
    if (chordLineType != "") {
        if (note) {
            const auto chordline = Factory::createChordLine(note->chord());
            if (chordLineType == "falloff") {
                chordline->setChordLineType(ChordLineType::FALL);
            }
            if (chordLineType == "doit") {
                chordline->setChordLineType(ChordLineType::DOIT);
            }
            if (chordLineType == "plop") {
                chordline->setChordLineType(ChordLineType::PLOP);
            }
            if (chordLineType == "scoop") {
                chordline->setChordLineType(ChordLineType::SCOOP);
            }
            note->chord()->add(chordline);
        } else {
            logger->logError(QString("no note for %1").arg(chordLineType), xmlreader);
        }
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

    if (_text != "") {
        res += " ";
        res += _text;
    }
    return res;
}

//---------------------------------------------------------
//   MusicXMLParserNotations
//---------------------------------------------------------

MusicXMLParserNotations::MusicXMLParserNotations(QXmlStreamReader& e, Score* score, MxmlLogger* logger)
    : _e(e), _score(score), _logger(logger)
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
    if (error != "") {
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
            if (_arpeggioType == "") {
                _arpeggioType = "none";
            }
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "articulations") {
            articulations();
        } else if (_e.name() == "dynamics") {
            dynamics();
        } else if (_e.name() == "fermata") {
            fermata();
        } else if (_e.name() == "glissando") {
            glissandoSlide();
        } else if (_e.name() == "non-arpeggiate") {
            _arpeggioType = "non-arpeggiate";
            _e.skipCurrentElement();  // skip but don't log
        } else if (_e.name() == "ornaments") {
            ornaments();
        } else if (_e.name() == "slur") {
            slur();
        } else if (_e.name() == "slide") {
            glissandoSlide();
        } else if (_e.name() == "technical") {
            technical();
        } else if (_e.name() == "tied") {
            tied();
        } else if (_e.name() == "tuplet") {
            tuplet();
        } else {
            skipLogCurrElem();
        }
    }

    /*
    for (const auto& notation : _notations) {
          LOGD("%s", qPrintable(notation.print()));
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
        QString notationType = notation.attribute("type");
        QString placement = notation.attribute("placement");
        if (notation.name() == "fermata") {
            if (notationType != "" && notationType != "upright" && notationType != "inverted") {
                notationType = (const char*)0;
                _logger->logError(QString("unknown fermata type %1").arg(notationType), &_e);
            }
            addFermataToChord(notation, cr);
        } else {
            if (notation.name() == "strong-accent") {
                if (notationType != "" && notationType != "up" && notationType != "down") {
                    notationType = (const char*)0;
                    _logger->logError(QString("unknown %1 type %2").arg(notation.name(), notationType), &_e);
                }
            } else if (notation.name() == "harmonic" || notation.name() == "delayed-turn"
                       || notation.name() == "turn" || notation.name() == "inverted-turn") {
                if (notation.name() == "delayed-turn") {
                    // TODO: actually this should be offset a bit to the right
                }
                if (placement != "above" && placement != "below") {
                    placement = (const char*)0;
                    _logger->logError(QString("unknown %1 placement %2").arg(notation.name(), placement), &_e);
                }
            } else {
                notationType = (const char*)0;           // TODO: Check for other symbols that have type
                placement = (const char*)0;           // TODO: Check for other symbols that have placement
            }
            addArticulationToChord(notation, cr);
        }
    } else if (notation.parent() == "ornaments") {
        if (notation.name() == "mordent" || notation.name() == "inverted-mordent") {
            addMordentToChord(notation, cr);
        } else if (notation.name() == "other-ornament") {
            addOtherOrnamentToChord(notation, cr);
        }
    } else if (notation.parent() == "articulations") {
        if (note && notation.name() == "chord-line") {
            addChordLine(notation, note, _logger, &_e);
        }
    } else {
        // LOGD("addNotation: notation has been skipped: %s %s", qPrintable(notation.name()), qPrintable(notation.parent()));
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
                                         TrillStack& trills, Tie*& tie)
{
    addArpeggio(cr, _arpeggioType, _logger, &_e);
    addBreath(cr, cr->tick(), _breath);
    addWavyLine(cr, Fraction::fromTicks(tick), _wavyLineNo, _wavyLineType, spanners, trills, _logger, &_e);

    for (const auto& notation : _notations) {
        if (notation.symId() != SymId::noSym) {
            addNotation(notation, cr, note);
        } else if (notation.name() == "slur") {
            addSlur(notation, slurs, cr, tick, _logger, &_e);
        } else if (note && (notation.name() == "glissando" || notation.name() == "slide")) {
            addGlissandoSlide(notation, note, glissandi, spanners, _logger, &_e);
        } else if (note && notation.name() == "tied") {
            addTie(notation, _score, note, cr->track(), tie, _logger, &_e);
        } else if (note && notation.parent() == "technical") {
            addTechnical(notation, note);
        } else {
            addNotation(notation, cr, note);
        }
    }

    // more than one dynamic ???
    // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
    // TODO remove duplicate code (see MusicXml::direction)
    for (const auto& d : qAsConst(_dynamicsList)) {
        auto dynamic = Factory::createDynamic(_score->dummy()->segment());
        dynamic->setDynamicType(d);
        addElemOffset(dynamic, cr->track(), _dynamicsPlacement, cr->measure(), Fraction::fromTicks(tick));
    }
}

//---------------------------------------------------------
//   stem
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/stem node.
 */

void MusicXMLParserPass2::stem(DirectionV& sd, bool& nost)
{
    // defaults
    sd = DirectionV::AUTO;
    nost = false;

    QString s = _e.readElementText();

    if (s == "up") {
        sd = DirectionV::UP;
    } else if (s == "down") {
        sd = DirectionV::DOWN;
    } else if (s == "none") {
        nost = true;
    } else if (s == "double") {
    } else {
        _logger->logError(QString("unknown stem direction %1").arg(s), &_e);
    }
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

    if (tupletType == "start") {
        _tupletDesc.type = MxmlStartStop::START;
    } else if (tupletType == "stop") {
        _tupletDesc.type = MxmlStartStop::STOP;
    } else if (tupletType != "" && tupletType != "start" && tupletType != "stop") {
        _logger->logError(QString("unknown tuplet type '%1'").arg(tupletType), &_e);
    }

    // set bracket, leave at default if unspecified
    if (tupletBracket == "yes") {
        _tupletDesc.bracket = TupletBracketType::SHOW_BRACKET;
    } else if (tupletBracket == "no") {
        _tupletDesc.bracket = TupletBracketType::SHOW_NO_BRACKET;
    }

    // set number, default is "actual" (=NumberType::SHOW_NUMBER)
    if (tupletShowNumber == "both") {
        _tupletDesc.shownumber = TupletNumberType::SHOW_RELATION;
    } else if (tupletShowNumber == "none") {
        _tupletDesc.shownumber = TupletNumberType::NO_TEXT;
    } else {
        _tupletDesc.shownumber = TupletNumberType::SHOW_NUMBER;
    }

    // set number and bracket placement
    if (tupletPlacement == "above") {
        _tupletDesc.direction = DirectionV::UP;
    } else if (tupletPlacement == "below") {
        _tupletDesc.direction = DirectionV::DOWN;
    } else if (tupletPlacement == "") {
        // ignore
    } else {
        _logger->logError(QString("unknown tuplet placement: %1").arg(tupletPlacement), &_e);
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
    _hasDefaultY(false), _defaultY(0.0), _hasRelativeY(false), _relativeY(0.0),
    _tpoMetro(0), _tpoSound(0), _offset(0, 1)
{
    // nothing
}
}
