/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "containers.h"

#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"
#include "engraving/iengravingfont.h"

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
#include "engraving/dom/gradualtempochange.h"
#include "engraving/dom/hammeronpulloff.h"
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
#include "engraving/dom/tapping.h"
#include "engraving/dom/tempo.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/volta.h"
#include "engraving/engravingerrors.h"

#include "importmusicxmllogger.h"
#include "importmusicxmlnoteduration.h"
#include "importmusicxmlnotepitch.h"
#include "importmusicxmlpass1.h"
#include "importmusicxmlpass2.h"
#include "../shared/musicxmlfonthandler.h"
#include "../shared/musicxmlsupport.h"
#include "../shared/musicxmltypes.h"

#include "modularity/ioc.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/rendering/score/tlayout.h"

#include "log.h"

using namespace muse;
using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;
using namespace mu::iex::musicxml;

namespace mu::iex::musicxml {
static std::shared_ptr<mu::iex::musicxml::IMusicXmlConfiguration> configuration()
{
    return muse::modularity::globalIoc()->resolve<mu::iex::musicxml::IMusicXmlConfiguration>("iex_musicxml");
}

static std::shared_ptr<mu::engraving::IEngravingFontsProvider> engravingFonts()
{
    return muse::modularity::globalIoc()->resolve<mu::engraving::IEngravingFontsProvider>("iex_musicxml");
}

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

//#define DEBUG_VOICE_MAPPER true

constexpr int MAX_LYRICS       = 16;

//---------------------------------------------------------
//   function declarations
//---------------------------------------------------------

static void addTie(const Notation& notation, Note* note, const track_idx_t track, MusicXmlTieMap& tie,
                   std::vector<Note*>& unstartedTieNotes, std::vector<Note*>& unendedTieNotes, MusicXmlLogger* logger,
                   const XmlStreamReader* const xmlreader);

//---------------------------------------------------------
//   support enums / structs / classes
//---------------------------------------------------------

//---------------------------------------------------------
//   MusicXmlTupletDesc
//---------------------------------------------------------

MusicXmlTupletDesc::MusicXmlTupletDesc()
    : type(MusicXmlStartStop::NONE), direction(DirectionV::AUTO),
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
    m_lyrics.clear();
}

//---------------------------------------------------------
//   addLyric
//---------------------------------------------------------

// add a single lyric to be extended later
// called when lyric with "extend" or "extend type=start" is found

void MusicXmlLyricsExtend::addLyric(Lyrics* const lyric)
{
    m_lyrics.insert(lyric);
}

//---------------------------------------------------------
//   lastChordTicks
//---------------------------------------------------------

// find the duration of the chord starting at or after s and ending at tick

static Fraction lastChordTicks(const Segment* s, const Fraction& tick, const track_idx_t track)
{
    while (s && s->tick() < tick) {
        for (EngravingItem* el : s->elist()) {
            if (el && el->isChordRest() && el->track() == track) {
                ChordRest* cr = static_cast<ChordRest*>(el);
                if (cr->tick() + cr->actualTicks() == tick) {
                    return cr->actualTicks();
                }
            }
        }
        s = s->nextCR(muse::nidx, true);
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   setExtend
//---------------------------------------------------------

// set extend for lyric no in *staff* to end at tick
// called when lyric (with or without "extend") or note with "extend type=stop" is found
// // note that no == -1 means all lyrics in this *track*

void MusicXmlLyricsExtend::setExtend(const int no, const track_idx_t track, const Fraction& tick, const Lyrics* prevAddedLyrics = nullptr)
{
    std::vector<Lyrics*> list;
    for (Lyrics* l : m_lyrics) {
        const EngravingItem* el = l->parentItem();
        if (el->type() == ElementType::CHORD || el->type() == ElementType::REST) {
            const ChordRest* par = static_cast<const ChordRest*>(el);
            // no = -1: stop all extends on this track
            // otherwise, stop all extends in the stave with the same no and placement
            if ((no == -1 && par->track() == track)
                || (l->no() == no && track2staff(par->track()) == track2staff(track) && prevAddedLyrics
                    && prevAddedLyrics->placement() == l->placement())) {
                Fraction lct = lastChordTicks(l->segment(), tick, track);
                if (lct > Fraction(0, 1)) {
                    // set lyric tick to the total length from the lyric note
                    // plus all notes covered by the melisma minus the last note length
                    l->setTicks(tick - par->tick() - lct);
                }
                list.push_back(l);
            }
        }
    }
    // cleanup
    for (Lyrics* l : list) {
        muse::remove(m_lyrics, l);
    }
}

//---------------------------------------------------------
//   MusicXmlStepAltOct2Pitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step (0=C, 1=D, etc.) / \a alter / \a octave to midi pitch.
 Note: same code is in pass 1 and in pass 2.
 TODO: combine
 */

static int MusicXmlStepAltOct2Pitch(int step, int alter, int octave)
{
    //                       c  d  e  f  g  a   b
    static int table[7]  = { 0, 2, 4, 5, 7, 9, 11 };
    if (step < 0 || step > 6) {
        LOGD("MusicXmlStepAltOct2Pitch: illegal step %d", step);
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

static void xmlSetPitch(Note* n, int step, int alter, double tuning, int octave, const int octaveShift, const Instrument* const instr)
{
    //LOGD("xmlSetPitch(n=%p, step=%d, alter=%d, octave=%d, octaveShift=%d)",
    //       n, step, alter, octave, octaveShift);

    //const Staff* staff = n->score()->staff(track / VOICES);
    //const Instrument* instr = staff->part()->instr();

    const Interval intval = instr->transpose();

    //LOGD("  staff=%p instr=%p dia=%d chro=%d",
    //       staff, instr, static_cast<int>(intval.diatonic), static_cast<int>(intval.chromatic));

    int pitch = MusicXmlStepAltOct2Pitch(step, alter, octave);
    pitch += intval.chromatic;   // assume not in concert pitch
    pitch += 12 * octaveShift;   // correct for octave shift
    // ensure sane values
    pitch = std::clamp(pitch, 0, 127);

    int tpc2 = step2tpc(step, AccidentalVal(alter));
    int tpc1 = mu::engraving::transposeTpc(tpc2, intval, true);
    n->setPitch(pitch, tpc1, tpc2);
    n->setTuning(tuning);
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

static bool hasDrumset(const MusicXmlInstruments& instruments)
{
    bool res = false;
    for (const auto& p : instruments) {
        // debug: dump the instruments
        //LOGD("instrument: %s %s", muPrintable(ii.key()), muPrintable(ii.value().toString()));
        // find valid unpitched values
        int unpitched = p.second.unpitched;
        if (0 <= unpitched && unpitched <= 127) {
            res = true;
        }
    }

    /*
    for (const auto& instr : instruments) {
          // MusicXML elements instrument-name, midi-program, instrument-sound, virtual-library, virtual-name
          // in a shell script use "mscore ... 2>&1 | grep GREP_ME | cut -d' ' -f3-" to extract
          LOGD("GREP_ME '%s',%d,'%s','%s','%s'",
                 muPrintable(instr.name),
                 instr.midiProgram + 1,
                 muPrintable(instr.sound),
                 muPrintable(instr.virtLib),
                 muPrintable(instr.virtName)
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
// which stored in the MusicXmlInstrument pitch field)
// then if the part contains a drumset, Drumset drumset is initialized

static void initDrumset(Drumset* drumset, const MusicXmlInstruments& instruments)
{
    drumset->clear();

    for (const auto& ii : instruments) {
        // debug: also dump the drumset for this part
        //LOGD("initDrumset: instrument: %s %s", muPrintable(ii.key()), muPrintable(ii.value().toString()));
        int unpitched = ii.second.unpitched;
        if (0 <= unpitched && unpitched <= 127) {
            drumset->drum(ii.second.unpitched)
                = DrumInstrument(ii.second.name, ii.second.notehead, ii.second.line, ii.second.stemDirection);
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

static std::pair<String, String> processInstrName(const String& name)
{
    // Make corrections so instruments match our names, then separate transpositions and remove unneeded info
    String n = name;
    n.replace(u"♭", u"b").replace(u"♯", u"#");
    n.replace(u"Sax", u"Saxophone");
    n.replace(u"Bari", u"Baritone");
    std::pair<String, String> ret;
    // Find transposition information
    static const std::regex transpositionRegex("(^|(?:\\s|\\u00A0))([ABCDEF][b#]?)((?:\\s|\\u00A0)|$)");
    // Remove: ' in', part numbers '1, 2' which may be bracketed, any text after a slash
    static const std::regex removeRegex("(?:\\s|\u00A0)in|(\\(?[0-9]([,](?:\\s|\u00A0)?)?)\\)?|\\.|(\\/.*)");

    const StringList results = n.search(transpositionRegex, { 2 });
    if (!results.empty()) {
        ret.second = convertPitchStringFlatsAndSharpsToUnicode(results.front());
        n.remove(transpositionRegex);
    }
    n.remove(removeRegex);
    ret.first = n.simplified();

    return ret;
}

//---------------------------------------------------------
//   createInstrument
//---------------------------------------------------------

/**
 Create an Instrument based on the information in \a mxmlInstr.
 */

static Instrument createInstrument(const MusicXmlInstrument& mxmlInstr, const Interval interval)
{
    Instrument instr;

    const InstrumentTemplate* it = nullptr;
    const std::pair<String, String> nameSplit = processInstrName(mxmlInstr.name);
    const String name = nameSplit.first;
    const int transposition = nameSplit.second.isEmpty() ? 0 : string2pitch(nameSplit.second + u"5") % 12;

    it = combinedTemplateSearch(mxmlInstr.sound, name, transposition, 0, mxmlInstr.midiProgram);

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

static void updatePartWithInstrument(Part* const part, const MusicXmlInstrument& mxmlInstr, const Interval interval,
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

static InstrumentChange* createInstrumentChange(Score* score, const MusicXmlInstrument& mxmlInstr, const Interval interval,
                                                const track_idx_t track, const Instrument* curInstr)
{
    const Instrument instr = createInstrument(mxmlInstr, interval);

    if (curInstr->id() == instr.id() && instr.longNames() == curInstr->longNames() && instr.shortNames() == curInstr->shortNames()) {
        return nullptr;
    }

    InstrumentChange* instrChange = Factory::createInstrumentChange(score->dummy()->segment(), instr);
    instrChange->setTrack(track);

    // for text use instrument name (if known) else use "Instrument change"
    const String text = mxmlInstr.name;
    instrChange->setXmlText(text.empty() ? "Instrument change" : text.toUtf8().constChar());
    instrChange->setVisible(false);

    return instrChange;
}

//---------------------------------------------------------
//   updatePartWithInstrumentChange
//---------------------------------------------------------

static void updatePartWithInstrumentChange(Part* const part, const MusicXmlInstrument& mxmlInstr, const Interval interval,
                                           Segment* const segment, const track_idx_t track, const Fraction tick)
{
    const Instrument* curInstr = part->instrument(tick);
    InstrumentChange* const ic = createInstrumentChange(part->score(), mxmlInstr, interval, track, curInstr);

    if (!ic) {
        return;
    }

    segment->add(ic);               // note: includes part::setInstrument(instr);

    // setMidiChannel() depends on setInstrument() already been done
    if (mxmlInstr.midiChannel >= 0) {
        part->setMidiChannel(mxmlInstr.midiChannel, mxmlInstr.midiPort, tick);
    }
}

static void setPercussionInstrument(MusicXmlInstrument& mxmlInstr, const String& partName)
{
    // If there are multiple unpitched instruments we should set the part's instrument to drumkit or percussion
    if (partName.contains(u"drumset",
                          CaseSensitivity::CaseInsensitive) || partName.contains(u"drumkit", CaseSensitivity::CaseInsensitive)) {
        mxmlInstr = MusicXmlInstrument();
        mxmlInstr.name = u"Drum Kit";
    } else if (partName.contains(u"percussion", CaseSensitivity::CaseInsensitive)) {
        mxmlInstr = MusicXmlInstrument();
        mxmlInstr.name = u"Percussion";
        mxmlInstr.sound = u"drum.group.set";
    }
}

//---------------------------------------------------------
//   setPartInstruments
//---------------------------------------------------------

/**
 Set instruments for Part \a part
 Note:
 - MusicXmlInstrList: which instrument plays when
 - MusicXmlInstruments: instrument details from score-part and part
 */

static void setPartInstruments(MusicXmlLogger* logger, const XmlStreamReader* xmlreader,
                               Part* part, const String& partId,
                               const Score* score,
                               const MusicXmlInstrList& instrList,
                               const MusicXmlIntervalList& intervList,
                               const MusicXmlInstruments& instruments,
                               const String& partName)
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
        MusicXmlInstrument mxmlInstr = instruments.begin()->second;
        if (instruments.size() > 1) {
            setPercussionInstrument(mxmlInstr, partName);
        }
        updatePartWithInstrument(part, mxmlInstr, {}, true);
        return;
    }

    if (instrList.empty()) {
        // instrument details found, but no instrument ids found
        // -> only a single instrument is playing in the part
        //LOGD("single instrument");
        MusicXmlInstrument mxmlInstr = instruments.begin()->second;
        updatePartWithInstrument(part, mxmlInstr, intervList.interval({ 0, 1 }));
        return;
    }

    // either a single instrument is playing, or forwards / rests resulted in gaps in the instrument map
    // (and thus multiple entries)
    //LOGD("possibly multiple instruments");
    String prevInstrId;
    for (auto it = instrList.cbegin(); it != instrList.cend(); ++it) {
        Fraction tick = (*it).first;
        if (it == instrList.cbegin()) {
            prevInstrId = (*it).second;              // first instrument id
            MusicXmlInstrument mxmlInstr = muse::value(instruments, prevInstrId);
            updatePartWithInstrument(part, mxmlInstr, intervList.interval(tick));
        } else {
            String instrId = (*it).second;
            bool mustInsert = instrId != prevInstrId;
            /*
             LOGD("tick %s previd %s id %s mustInsert %d",
             muPrintable(tick.print()),
             muPrintable(prevInstrId),
             muPrintable(instrId),
             mustInsert);
             */
            if (mustInsert) {
                const staff_idx_t staff = score->staffIdx(part);
                const track_idx_t track = staff * VOICES;
                //LOGD("instrument change: tick %s (%d) track %d instr '%s'",
                //       muPrintable(tick.print()), tick.ticks(), track, muPrintable(instrId));

                Measure* const m = score->tick2measure(tick);
                Segment* const segment = m->getSegment(SegmentType::ChordRest, tick);

                if (!segment) {
                    logger->logError(String(u"segment for instrument change at tick %1 not found")
                                     .arg(tick.ticks()), xmlreader);
                } else if (!muse::contains(instruments, instrId)) {
                    logger->logError(String(u"changed instrument '%1' at tick %2 not found in part '%3'")
                                     .arg(instrId).arg(tick.ticks()).arg(partId), xmlreader);
                } else {
                    MusicXmlInstrument mxmlInstr = muse::value(instruments, instrId);
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
static String text2syms(const String& t)
{
    //QTime time;
    //time.start();

    // first create a map from symbol (Unicode) text to symId
    // note that this takes about 1 msec on a Core i5,
    // caching does not gain much

    IEngravingFontPtr sf = engravingFonts()->fallbackFont();
    std::map<String, SymId> map;
    size_t maxStringSize = 0;          // maximum string size found

    for (int i = int(SymId::noSym); i < int(SymId::lastSym); ++i) {
        SymId id((SymId(i)));
        String string = sf->toString(id);
        // insert all syms except space to prevent matching all regular spaces
        if (id != SymId::space) {
            map.insert({ string, id });
        }
        if (string.size() > maxStringSize) {
            maxStringSize = string.size();
        }
    }

    // Special case Dolet inference (TODO: put behind a setting or export type flag)
    map.insert({ u"$", SymId::segno });
    map.insert({ u"Ø", SymId::coda });

    //LOGD("text2syms map count %d maxsz %d filling time elapsed: %d ms",
    //       map.size(), maxStringSize, time.elapsed());

    // then look for matches
    String in = t;
    String res;

    while (!in.empty()) {
        // try to find the largest match possible
        int maxMatch = int(std::min(in.size(), maxStringSize));
        AsciiStringView sym;
        while (maxMatch > 0) {
            String toBeMatched = in.left(maxMatch);
            if (muse::contains(map, toBeMatched)) {
                sym = SymNames::nameForSymId(map.at(toBeMatched));
                break;
            }
            maxMatch--;
        }
        if (maxMatch > 0) {
            // found a match, add sym to res and remove match from string in
            res += u"<sym>";
            res += String::fromAscii(sym.ascii());
            res += u"</sym>";
            in.remove(0, maxMatch);
        } else {
            // not found, move one char from res to in
            res += in.left(1);
            in.remove(0, 1);
        }
    }

    //LOGD("text2syms total time elapsed: %d ms, res '%s'", time.elapsed(), muPrintable(res));
    return res;
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
static String nextPartOfFormattedString(XmlStreamReader& e)
{
    //String lang       = e.attribute(String("xml:lang"), "it");
    String fontWeight = e.attribute("font-weight");
    String fontSize   = e.attribute("font-size");
    String fontStyle  = e.attribute("font-style");
    String underline  = e.attribute("underline");
    String strike     = e.attribute("line-through");
    String fontFamily = e.attribute("font-family");
    // TODO: color, enclosure, yoffset in only part of the text, ...

    String txt = e.readText();
    // replace HTML entities
    txt = String::decodeXmlEntities(txt);
    String syms = xmlpass2::text2syms(txt);

    String importedtext;

    if (!fontSize.empty()) {
        bool ok = true;
        float size = fontSize.toFloat(&ok);
        if (ok) {
            importedtext += String(u"<font size=\"%1\"/>").arg(size);
        }
    }

    bool needUseDefaultFont = configuration()->needUseDefaultFont();
    if (!fontFamily.empty() && txt == syms && !needUseDefaultFont) {
        // add font family only if no <sym> replacement made
        importedtext += String(u"<font face=\"%1\"/>").arg(fontFamily);
    }
    if (fontWeight == u"bold") {
        importedtext += u"<b>";
    }
    if (fontStyle == u"italic") {
        importedtext += u"<i>";
    }
    if (!underline.empty()) {
        bool ok = true;
        int lines = underline.toInt(&ok);
        if (ok && (lines > 0)) {    // 1, 2, or 3 underlines are imported as single underline
            importedtext += u"<u>";
        } else {
            underline.clear();
        }
    }
    if (!strike.empty()) {
        bool ok = true;
        int lines = strike.toInt(&ok);
        if (ok && (lines > 0)) {    // 1, 2, or 3 strikes are imported as single strike
            importedtext += u"<s>";
        } else {
            strike.clear();
        }
    }
    if (txt == syms) {
        txt.replace(String(u"\r"), String());     // convert Windows line break \r\n -> \n
        importedtext += txt.toXmlEscaped();
    } else {
        // <sym> replacement made, should be no need for line break or other conversions
        importedtext += syms;
    }
    if (!strike.empty()) {
        importedtext += u"</s>";
    }
    if (!underline.empty()) {
        importedtext += u"</u>";
    }
    if (fontStyle == u"italic") {
        importedtext += u"</i>";
    }
    if (fontWeight == u"bold") {
        importedtext += u"</b>";
    }
    //LOGD("importedtext '%s'", muPrintable(importedtext));
    return importedtext;
}
}

//---------------------------------------------------------
//   addLyric
//---------------------------------------------------------

/**
 Add a single lyric to the score or delete it (if number too high)
 */

static void addLyric(MusicXmlLogger* logger, const XmlStreamReader* const xmlreader,
                     ChordRest* cr, Lyrics* l, int lyricNo, MusicXmlLyricsExtend& extendedLyrics)
{
    if (lyricNo > MAX_LYRICS) {
        logger->logError(String(u"too much lyrics (>%1)")
                         .arg(MAX_LYRICS), xmlreader);
        delete l;
    } else {
        l->setNo(lyricNo);
        l->initTextStyleType(l->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD, /*preserveDifferent*/ true);
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

static void addLyrics(MusicXmlLogger* logger, const XmlStreamReader* const xmlreader,
                      ChordRest* cr,
                      const std::map<int, Lyrics*>& numbrdLyrics,
                      const std::set<Lyrics*>& extLyrics,
                      MusicXmlLyricsExtend& extendedLyrics)
{
    for (const int lyricNo : muse::keys(numbrdLyrics)) {
        Lyrics* const lyric = numbrdLyrics.at(lyricNo);
        addLyric(logger, xmlreader, cr, lyric, lyricNo, extendedLyrics);
        if (muse::contains(extLyrics, lyric)) {
            extendedLyrics.addLyric(lyric);
        }
    }
}

static void addGraceNoteLyrics(const std::map<int, Lyrics*>& numberedLyrics, std::set<Lyrics*> extendedLyrics,
                               std::vector<GraceNoteLyrics>& gnLyrics)
{
    for (const int lyricNo : muse::keys(numberedLyrics)) {
        Lyrics* const lyric = numberedLyrics.at(lyricNo);
        if (lyric) {
            bool extend = muse::contains(extendedLyrics, lyric);
            const GraceNoteLyrics gnl = GraceNoteLyrics(lyric, extend, lyricNo);
            gnLyrics.push_back(gnl);
        }
    }
}

static void addInferredStickings(ChordRest* cr, const std::vector<Sticking*>& numberedStickings)
{
    for (Sticking* sticking : numberedStickings) {
        sticking->setParent(cr->segment());
        sticking->setTrack(cr->track());
        cr->score()->addElement(sticking);
    }
}

//---------------------------------------------------------
//   addElemOffset
//---------------------------------------------------------

void MusicXmlParserPass2::addElemOffset(engraving::EngravingItem* el, engraving::track_idx_t track, const muse::String& placement,
                                        engraving::Measure* measure, const engraving::Fraction& tick)
{
    if (!measure) {
        return;
    }
    Fraction elTick = tick;

    if (!placement.empty()) {
        if (el->hasVoiceAssignmentProperties()) {
            el->setProperty(Pid::DIRECTION, placement == u"above" ? DirectionV::UP : DirectionV::DOWN);
            el->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
        } else {
            el->setPlacement(placement == u"above" ? PlacementV::ABOVE : PlacementV::BELOW);
            el->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
            if (!el->isSticking()) {
                el->resetProperty(Pid::OFFSET);
            }
        }
    }
    const Fraction& endTick = measure->score()->endTick();
    if (elTick > endTick) {
        elTick = endTick;
    }

    el->setTrack(el->isTempoText() ? 0 : track);      // TempoText must be in track 0
    Segment* s = measure->getSegment(SegmentType::ChordRest, elTick);

    if (el->isSticking()) {
        if (el->propertyFlags(Pid::OFFSET) == PropertyFlags::UNSTYLED) {
            const EngravingItem* item = s->element(el->track());
            const Chord* chord = item && item->isChord() ? toChord(item) : nullptr;
            const bool hasGraceNotes = chord && !chord->graceNotes().empty();

            if (!hasGraceNotes) {
                el->resetProperty(Pid::OFFSET);
            }
        }
    }

    if (el->systemFlag()) {
        const bool finale = m_pass1.exporterSoftware() == MusicXmlExporterSoftware::FINALE;
        Score* score = measure->score();
        Staff* st = score->staff(track2staff(track));

        if (!st->isSystemObjectStaff() && st->idx() != 0) {
            if (finale) {
                delete el;
                return;
            }
            score->addSystemObjectStaff(st);
        }

        bool found = false;
        for (EngravingItem* existingEl : muse::values(systemElements(), elTick.ticks())) {
            if (el->type() == existingEl->type()) {
                if (el->isTextBase()) {
                    TextBase* elText = toTextBase(el);
                    TextBase* existingText = toTextBase(existingEl);
                    if (elText->xmlText() == existingText->xmlText()) {
                        found = true;
                        break;
                    } else {
                        continue;
                    }
                }
                found = true;
                break;
            }
        }
        if (!found) {
            el->setParent(s);
            addSystemElement(el, elTick);
        }
    } else {
        s->add(el);
    }
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
    MusicXmlTupletState::determineTupletFractionAndFullDuration(calculateTupletDuration(t), tupletFraction, tupletFullDuration);

    Fraction baseLen = tupletFullDuration * Fraction(1, t->ratio().denominator());
    /*
    LOGD("tupletFraction %s tupletFullDuration %s ratio %s baseLen %s",
           muPrintable(tupletFraction.toString()),
           muPrintable(tupletFullDuration.toString()),
           muPrintable(t->ratio().toString()),
           muPrintable(baseLen.toString())
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
    if (!f.isValid()) {
        LOGD("MusicXml::import: tuplet stop but note values too small");
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
            if (globalTicks != ticksPerNote) {
                ticksCorrect = false;
            }
            totalDuration += globalTicks;
        }
    }
    if (totalDuration != f.ticks()) {
        LOGD("MusicXml::import: tuplet stop but bad duration");     // TODO
    }
    if (!ticksCorrect) {
        LOGD("MusicXml::import: tuplet stop but uneven note ticks"); // TODO
    }
    tuplet = 0;
}

//---------------------------------------------------------
//   setElementPropertyFlags
//---------------------------------------------------------

static void setElementPropertyFlags(EngravingObject* element, const Pid propertyId,
                                    const String value1, const String value2 = String())
{
    if (value1.empty()) { // Set as an implicit value
        element->setPropertyFlags(propertyId, PropertyFlags::STYLED);
    } else if (!value1.empty() || !value2.empty()) { // Set as an explicit value
        element->setPropertyFlags(propertyId, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   addArticulationToChord
//---------------------------------------------------------

static void addArticulationToChord(const Notation& notation, ChordRest* cr)
{
    const SymId articSym = notation.symId();
    const String dir = notation.attribute(u"type");
    const String place = notation.attribute(u"placement");
    const Color color = Color::fromString(notation.attribute(u"color"));
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
    if (place == "above" || (dir == "up" && place.empty())) {
        na->setAnchor(ArticulationAnchor::TOP);
        na->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
    } else if (place == "below" || (dir == "down" && place.empty())) {
        na->setAnchor(ArticulationAnchor::BOTTOM);
        na->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
    }

    if (cr->isChord() && toChord(cr)->hasArticulation(na)) {
        return;
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
    const String direction = notation.attribute(u"type");
    const Color color = Color::fromString(notation.attribute(u"color"));
    Segment* seg = cr->segment();
    Fermata* fermata = Factory::createFermata(seg ? seg : cr->score()->dummy()->segment());
    fermata->setSymIdAndTimeStretch(articSym);
    fermata->setTrack(cr->track());
    if (color.isValid()) {
        fermata->setColor(color);
    }
    if (!direction.empty()) {
        fermata->setPlacement(direction == "inverted" ? PlacementV::BELOW : PlacementV::ABOVE);
        fermata->resetProperty(Pid::OFFSET);
    } else {
        fermata->setPlacement(fermata->propertyDefault(Pid::PLACEMENT).value<PlacementV>());
    }
    setElementPropertyFlags(fermata, Pid::PLACEMENT, direction);
    if (!seg) {
        assert(cr->isGrace());
        cr->addFermata(fermata); // store for later move to segment
    } else {
        cr->segment()->add(fermata);
    }
}

//---------------------------------------------------------
//   addTapToChord
//---------------------------------------------------------

/**
 Add Tap to Chord.
 */

static void addTapToChord(const Notation& notation, ChordRest* cr)
{
    const Color color = Color::fromString(notation.attribute(u"color"));
    const String place = notation.attribute(u"placement");
    const TappingHand hand = notation.attribute(u"hand") == u"right" ? TappingHand::RIGHT : TappingHand::LEFT;

    Tapping* tap = Factory::createTapping(cr);
    tap->setTrack(cr->track());
    tap->setHand(hand);
    if (place == u"above") {
        tap->setAnchor(ArticulationAnchor::TOP);
        tap->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
    } else if (place == u"below") {
        tap->setAnchor(ArticulationAnchor::BOTTOM);
        tap->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
    } else {
        tap->setAnchor(ArticulationAnchor::AUTO);
    }
    if (color.isValid()) {
        tap->setColor(color);
    }
    cr->add(tap);
}

//---------------------------------------------------------
//   addMordentToChord
//---------------------------------------------------------

/**
 Add Mordent to Chord.
 */

static void addMordentToChord(const Notation& notation, ChordRest* cr)
{
    const String name = notation.name();
    const String attrLong = notation.attribute(u"long");
    const String attrAppr = notation.attribute(u"approach");
    const String attrDep = notation.attribute(u"departure");
    SymId articSym = SymId::noSym;   // legal but impossible ArticulationType value here indicating "not found"
    if (name == "inverted-mordent") {
        if ((attrLong.empty() || attrLong == "no") && attrAppr.empty() && attrDep.empty()) {
            articSym = SymId::ornamentShortTrill;
        } else if (attrLong == "yes" && attrAppr.empty() && attrDep.empty()) {
            articSym = SymId::ornamentTremblement;
        } else if (attrLong == "yes" && attrAppr == "below" && attrDep.empty()) {
            articSym = SymId::ornamentUpPrall;
        } else if (attrLong == "yes" && attrAppr == "above" && attrDep.empty()) {
            articSym = SymId::ornamentPrecompMordentUpperPrefix;
        } else if (attrLong == "yes" && attrAppr.empty() && attrDep == "below") {
            articSym = SymId::ornamentPrallDown;
        } else if (attrLong == "yes" && attrAppr.empty() && attrDep == "above") {
            articSym = SymId::ornamentPrallUp;
        }
    } else if (name == "mordent") {
        if ((attrLong.empty() || attrLong == "no") && attrAppr.empty() && attrDep.empty()) {
            articSym = SymId::ornamentMordent;
        } else if (attrLong == "yes" && attrAppr.empty() && attrDep.empty()) {
            articSym = SymId::ornamentPrallMordent;
        } else if (attrLong == "yes" && attrAppr == "below" && attrDep.empty()) {
            articSym = SymId::ornamentUpMordent;
        } else if (attrLong == "yes" && attrAppr == "above" && attrDep.empty()) {
            articSym = SymId::ornamentDownMordent;
        }
    }
    if (articSym != SymId::noSym) {
        const Color color = Color::fromString(notation.attribute(u"color"));
        const String place = notation.attribute(u"placement");
        Ornament* mordent = Factory::createOrnament(cr);
        mordent->setSymId(articSym);
        if (place == u"above") {
            mordent->setAnchor(ArticulationAnchor::TOP);
        } else if (place == u"below") {
            mordent->setAnchor(ArticulationAnchor::BOTTOM);
        } else {
            mordent->setAnchor(ArticulationAnchor::AUTO);
        }
        if (color.isValid()) {
            mordent->setColor(color);
        }
        cr->add(mordent);
    } else {
        LOGD("unknown ornament: name '%s' long '%s' approach '%s' departure '%s'",
             muPrintable(name), muPrintable(attrLong), muPrintable(attrAppr), muPrintable(attrDep));        // TODO
    }
}

//---------------------------------------------------------
//   addTurnToChord
//---------------------------------------------------------

/**
 Add Turn to Chord.
 */

static void addTurnToChord(const Notation& notation, ChordRest* cr)
{
    SymId turnSym = notation.symId();
    const Color color = Color::fromString(notation.attribute(u"color"));
    const String place = notation.attribute(u"placement");
    if (notation.attribute(u"slash") == "yes") {
        // TODO: currently this is the only available SMuFL turn with a slash
        turnSym = SymId::ornamentTurnSlash;
    }
    Ornament* turn = Factory::createOrnament(cr);
    turn->setSymId(turnSym);
    if (place == u"above") {
        turn->setAnchor(ArticulationAnchor::TOP);
    } else if (place == u"below") {
        turn->setAnchor(ArticulationAnchor::BOTTOM);
    } else {
        turn->setAnchor(ArticulationAnchor::AUTO);
    }
    if (color.isValid()) {
        turn->setColor(color);
    }
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
    const String name = notation.name();
    const String symname = notation.attribute(u"smufl");
    SymId sym = SymId::noSym;   // legal but impossible ArticulationType value here indicating "not found"
    sym = SymNames::symIdByName(symname);

    if (sym != SymId::noSym) {
        const Color color = Color::fromString(notation.attribute(u"color"));
        Ornament* ornam = Factory::createOrnament(cr);
        ornam->setSymId(sym);
        if (color.isValid()) {
            ornam->setColor(color);
        }
        cr->add(ornam);
    } else {
        LOGD("unknown ornament: name '%s': '%s'.", muPrintable(name), muPrintable(symname));
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

static bool convertArticulationToSymId(const String& mxmlName, SymId& id)
{
    // map MusicXML notations name to MuseScore symbol
    static const std::map<String, SymId> map {
        // ornaments
        { u"delayed-turn",           SymId::ornamentTurn },
        { u"inverted-turn",          SymId::ornamentTurnInverted },
        { u"vertical-turn",          SymId::ornamentTurnUp },
        { u"inverted-vertical-turn", SymId::ornamentTurnUpS },
        { u"turn",                   SymId::ornamentTurn },
        { u"shake",                  SymId::ornamentTremblementCouperin },
        { u"schleifer",              SymId::ornamentPrecompSlide },
        { u"haydn",                  SymId::ornamentHaydn },
        // articulations
        { u"accent",                 SymId::articAccentAbove },
        { u"strong-accent",          SymId::articMarcatoAbove },
        { u"staccato",               SymId::articStaccatoAbove },
        { u"tenuto",                 SymId::articTenutoAbove },
        { u"detached-legato",        SymId::articTenutoStaccatoAbove },
        { u"staccatissimo",          SymId::articStaccatissimoAbove },
        { u"spiccato",               SymId::articStaccatissimoStrokeAbove },
        { u"stress",                 SymId::articStressAbove },
        { u"unstress",               SymId::articUnstressAbove },
        { u"soft-accent",            SymId::articSoftAccentAbove },
        // technical
        { u"up-bow",                 SymId::stringsUpBow },
        { u"down-bow",               SymId::stringsDownBow },
        { u"open-string",            SymId::brassMuteOpen },
        { u"thumb-position",         SymId::stringsThumbPosition },
        { u"double-tongue",          SymId::doubleTongueAbove },
        { u"triple-tongue",          SymId::tripleTongueAbove },
        { u"stopped",                SymId::brassMuteClosed },
        { u"snap-pizzicato",         SymId::pluckedSnapPizzicatoAbove },
        { u"heel",                   SymId::keyboardPedalHeel1 },
        { u"toe",                    SymId::keyboardPedalToe2 },
        { u"fingernails",            SymId::pluckedWithFingernails },
        { u"brass-bend",             SymId::brassBend },
        { u"flip",                   SymId::brassFlip },
        { u"smear",                  SymId::brassSmear },
        { u"open",                   SymId::brassMuteOpen },
        { u"half-muted",             SymId::brassMuteHalfClosed }, // ignoring the smufl attribute
        { u"golpe",                  SymId::guitarGolpe },

        { u"belltree", SymId::handbellsBelltree },
        { u"damp", SymId::handbellsDamp3 },
        { u"echo", SymId::handbellsEcho1 },
        { u"gyro", SymId::handbellsGyro },
        { u"hand martellato", SymId::handbellsHandMartellato },
        { u"mallet lift", SymId::handbellsMalletLft },
        { u"mallet table", SymId::handbellsMalletBellOnTable },
        { u"martellato", SymId::handbellsMartellato },
        { u"martellato lift", SymId::handbellsMartellatoLift },
        { u"muted martellato", SymId::handbellsMutedMartellato },
        { u"pluck lift", SymId::handbellsPluckLift },
        { u"swing", SymId::handbellsSwing }
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        id = it->second;
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

static SymId convertFermataToSymId(const String& mxmlName)
{
    // map MusicXML fermata name to MuseScore symbol
    static const std::map<String, SymId> map {
        { u"normal",        SymId::fermataAbove },
        { u"angled",        SymId::fermataShortAbove },
        { u"square",        SymId::fermataLongAbove },
        { u"double-angled", SymId::fermataVeryShortAbove },
        { u"double-square", SymId::fermataVeryLongAbove },
        { u"double-dot",    SymId::fermataLongHenzeAbove },
        { u"half-curve",    SymId::fermataShortHenzeAbove },
        { u"curlew",        SymId::curlewSign },
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        return it->second;
    }

    return SymId::fermataAbove;
}

//---------------------------------------------------------
//   convertNotehead
//---------------------------------------------------------

/**
 Convert a MusicXML notehead name to a MuseScore headgroup.
 */

static NoteHeadGroup convertNotehead(String mxmlName)
{
    // map MusicXML notehead name to a MuseScore headgroup
    static const std::map<String, NoteHeadGroup> map {
        { u"slash", NoteHeadGroup::HEAD_SLASH },
        { u"triangle", NoteHeadGroup::HEAD_TRIANGLE_UP },
        { u"diamond", NoteHeadGroup::HEAD_DIAMOND },
        { u"cross", NoteHeadGroup::HEAD_PLUS },
        { u"x", NoteHeadGroup::HEAD_CROSS },
        { u"circle-x", NoteHeadGroup::HEAD_XCIRCLE },
        { u"inverted triangle", NoteHeadGroup::HEAD_TRIANGLE_DOWN },
        { u"slashed", NoteHeadGroup::HEAD_SLASHED1 },
        { u"back slashed", NoteHeadGroup::HEAD_SLASHED2 },
        { u"normal", NoteHeadGroup::HEAD_NORMAL },
        { u"do", NoteHeadGroup::HEAD_DO },
        { u"re", NoteHeadGroup::HEAD_RE },
        { u"mi", NoteHeadGroup::HEAD_MI },
        { u"fa", NoteHeadGroup::HEAD_FA },
        { u"fa up", NoteHeadGroup::HEAD_FA },
        { u"so", NoteHeadGroup::HEAD_SOL },
        { u"la", NoteHeadGroup::HEAD_LA },
        { u"ti", NoteHeadGroup::HEAD_TI }
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        return it->second;
    } else {
        LOGD("unknown notehead %s", muPrintable(mxmlName));      // TODO
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

static void addTextToNote(int l, int c, String txt, String placement, String fontWeight,
                          double fontSize, String fontStyle, String fontFamily, Color color,
                          TextStyleType subType, const Score*, Note* note)
{
    if (note) {
        if (!txt.empty()) {
            TextBase* t = Factory::createFingering(note, subType);
            t->setPlainText(txt);
            bool needUseDefaultFont = configuration()->needUseDefaultFont();
            if (!fontFamily.empty() && !needUseDefaultFont) {
                t->setFamily(fontFamily);
                t->setPropertyFlags(Pid::FONT_FACE, PropertyFlags::UNSTYLED);
            }
            if (std::isnormal(fontSize) && fontSize > 0.0) {
                t->setSize(fontSize);
                t->setPropertyFlags(Pid::FONT_SIZE, PropertyFlags::UNSTYLED);
            }
            if (!fontWeight.empty()) {
                t->setBold(fontWeight == u"bold");
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
            if (!fontStyle.empty()) {
                t->setItalic(fontStyle == u"italic");
                t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
            }
            if (!placement.empty()) {
                t->setPlacement(placement == u"below" ? PlacementV::BELOW : PlacementV::ABOVE);
                t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                t->resetProperty(Pid::OFFSET);
            }
            if (color.isValid()) {
                t->setColor(color);
                t->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
            }
            note->add(t);
        }
    } else {
        LOGD("%s", muPrintable(String(u"Error at line %1 col %2: no note for text").arg(l).arg(c)));           // TODO
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

static void setSLinePlacement(SLine* sli, const String& placement)
{
    if (placement == u"above" || placement == u"below") {
        if (sli->hasVoiceAssignmentProperties()) {
            sli->setProperty(Pid::DIRECTION, placement == u"above" ? DirectionV::UP : DirectionV::DOWN);
            sli->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
        } else {
            sli->setPlacement(placement == u"above" ? PlacementV::ABOVE : PlacementV::BELOW);
            sli->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
            sli->resetProperty(Pid::OFFSET);
        }
    }
}

//---------------------------------------------------------
//   handleSpannerStart
//---------------------------------------------------------

// note that in case of overlapping spanners, handleSpannerStart is called for every spanner
// as spanners std::map allows only one value per key, this does not hurt at all

static void handleSpannerStart(SLine* new_sp, track_idx_t track, String& placement, const Fraction& tick, MusicXmlSpannerMap& spanners)
{
    new_sp->setTrack(track);
    setSLinePlacement(new_sp, placement);
    spanners[new_sp] = std::pair<int, int>(tick.ticks(), -1);
}

//---------------------------------------------------------
//   handleSpannerStop
//---------------------------------------------------------

static void handleSpannerStop(SLine* cur_sp, track_idx_t track2, const Fraction& tick, MusicXmlSpannerMap& spanners)
{
    //LOGD("handleSpannerStop(sp %p, track2 %d, tick %s (%d))", cur_sp, track2, muPrintable(tick.print()), tick.ticks());
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
//   MusicXmlParserPass2
//---------------------------------------------------------

MusicXmlParserPass2::MusicXmlParserPass2(Score* score, MusicXmlParserPass1& pass1, MusicXmlLogger* logger)
    : m_divs(0), m_score(score), m_pass1(pass1), m_logger(logger)
{
    // nothing
}

//---------------------------------------------------------
//   addError
//---------------------------------------------------------

void MusicXmlParserPass2::addError(const String& error)
{
    if (!error.empty()) {
        m_logger->logError(error, &m_e);
        m_errors += errorStringWithLocation(m_e.lineNumber(), m_e.columnNumber(), error) + u'\n';
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
 * TODO: display step handling
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
        LOGD("cannot add rest at tick %s (%d) track %zu: element already present", muPrintable(tick.toString()), tick.ticks(), track); // TODO
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
        Tuplet* tuplet = pair.second;
        if (tuplet) {
            const Fraction actualDuration = tuplet->elementsDuration() / tuplet->ratio();
            const Fraction missingDuration = MusicXmlTupletState::missingTupletDuration(actualDuration);
            LOGD("tuplet %p not stopped at end of measure, tick %s duration %s missing %s",
                 tuplet,
                 muPrintable(tuplet->tick().toString()),
                 muPrintable(actualDuration.toString()), muPrintable(missingDuration.toString()));
            if (actualDuration > Fraction(0, 1) && missingDuration > Fraction(0, 1)) {
                LOGD("add missing %s to previous tuplet", muPrintable(missingDuration.toString()));
                const DurationElement* firstElement = tuplet->elements().at(0);
                // appended the rest to the current end of the tuplet (firstElement->tick() + actualDuration)
                Rest* const extraRest = addRest(firstElement->score(), firstElement->measure(),
                                                firstElement->tick() + actualDuration, firstElement->track(), 0,
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
//   initPartState
//---------------------------------------------------------

/**
 Initialize members as required for reading the MusicXML part element.
 TODO: factor out part reading into a separate class
 TODO: preferably use automatically initialized variables
 */

void MusicXmlParserPass2::initPartState(const String& partId)
{
    UNUSED(partId);
    m_timeSigDura = Fraction(0, 0);               // invalid
    m_ties.clear();
    m_unstartedTieNotes.clear();
    m_unendedTieNotes.clear();
    m_lastVolta = 0;
    m_hasDrumset = false;
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        m_slurs[i] = SlurDesc();
    }
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        m_trills[i] = 0;
    }
    for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
        m_glissandi[i][0] = m_glissandi[i][1] = 0;
    }
    m_pedalContinue = 0;
    m_harmony = 0;
    m_tremStart = 0;
    m_figBass = 0;
    m_delayedOttava = 0;
    m_multiMeasureRestCount = -1;
    m_measureStyleSlash = MusicXmlSlash::NONE;
    m_extendedLyrics.init();
    m_graceNoteLyrics.clear();
    m_inferredHairpins.clear();
    m_inferredTempoLines.clear();
    m_inferredPerc.clear();

    m_nstaves = m_pass1.getPart(partId)->nstaves();
    m_measureRepeatNumMeasures.assign(m_nstaves, 0);
    m_measureRepeatCount.assign(m_nstaves, 0);
}

//---------------------------------------------------------
//   findIncompleteSpannersInStack
//---------------------------------------------------------

static void findIncompleteSpannersInStack(const String& spannerType, SpannerStack& stack, SpannerSet& res)
{
    for (MusicXmlExtendedSpannerDesc& desc : stack) {
        if (desc.sp) {
            LOGD("%s not terminated at end of part", muPrintable(spannerType));
            res.insert(desc.sp);
            desc = {};
        }
    }
}

//---------------------------------------------------------
//   findIncompleteSpannersAtPartEnd
//---------------------------------------------------------

SpannerSet MusicXmlParserPass2::findIncompleteSpannersAtPartEnd()
{
    SpannerSet res;
    findIncompleteSpannersInStack(u"bracket", m_brackets, res);
    findIncompleteSpannersInStack(u"wedge", m_hairpins, res);
    findIncompleteSpannersInStack(u"octave-shift", m_ottavas, res);
    if (m_pedal.sp) {
        LOGD("pedal not terminated at end of part");
        res.insert(m_pedal.sp);
        m_pedal = {};
    }
    return res;
}

//---------------------------------------------------------
//   cleanupUnterminatedTie
//---------------------------------------------------------
/**
 Delete tie and add Laissez Vibrer where it was
 */

static void cleanupUnterminatedTie(Tie* tie, const Score* score, bool fixForCrossStaff = false)
{
    Note* unterminatedTieNote = tie->startNote();
    const Chord* unterminatedChord = unterminatedTieNote->chord();

    // Dolet 6 doesn't export cross staff information
    // If a tie is unterminated, try to find a candidate to tie it to on a different track/stave
    if (fixForCrossStaff) {
        const Segment* nextSeg = score->tick2leftSegment(unterminatedChord->tick() + unterminatedChord->ticks());
        if (nextSeg) {
            const Part* part = unterminatedTieNote->part();
            for (track_idx_t track = part->startTrack(); track <= part->endTrack(); ++track) {
                const EngravingItem* el = nextSeg->element(track);
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

static bool isLikelyIncorrectPartName(const String& partName)
{
    static const std::wregex re(L"^P[0-9]+$");
    return partName.contains(re);
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

void MusicXmlParserPass2::setMultiMeasureRestCount(int count)
{
    m_multiMeasureRestCount = count;
}

//---------------------------------------------------------
//   getAndDecMultiMeasureRestCount
//---------------------------------------------------------

/**
 Return current multi-measure rest counter.
 Decrement counter if possible (not beyond -1).
 */

int MusicXmlParserPass2::getAndDecMultiMeasureRestCount()
{
    int res = m_multiMeasureRestCount;
    if (m_multiMeasureRestCount >= 0) {
        m_multiMeasureRestCount--;
    }
    return res;
}

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXmlParserDirection::skipLogCurrElem()
{
    //_logger->logDebugInfo(String("skipping '%1'").arg(_e.name().toString()), &_e);
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXmlParserPass2::skipLogCurrElem()
{
    //_logger->logDebugInfo(String("skipping '%1'").arg(_e.name().toString()), &_e);
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Parse MusicXML in \a device and extract pass 2 data.
 */

Err MusicXmlParserPass2::parse(const ByteArray& data)
{
    //LOGD("MusicXmlParserPass2::parse()");
    m_e.setData(data);
    Err res = parse();
    //LOGD("MusicXmlParserPass2::parse() res %d", int(res));
    return res;
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Start the parsing process, after verifying the top-level node is score-partwise
 */

Err MusicXmlParserPass2::parse()
{
    bool found = false;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "score-partwise") {
            found = true;
            scorePartwise();
        } else {
            m_logger->logError(u"this is not a MusicXML score-partwise file", &m_e);
            m_e.skipCurrentElement();
            return Err::FileBadFormat;
        }
    }

    if (!found) {
        m_logger->logError(u"this is not a MusicXML score-partwise file", &m_e);
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

static std::unique_ptr<BarLine> createBarline(const Score* score, const track_idx_t track, const BarLineType type, const bool visible,
                                              const String& barStyle, int spanStaff)
{
    std::unique_ptr<BarLine> barline(Factory::createBarLine(score->dummy()->segment()));
    barline->setTrack(track);
    barline->setBarLineType(type);
    barline->setSpanStaff(spanStaff);
    barline->setVisible(visible);
    if (barStyle == u"tick") {
        barline->setSpanFrom(BARLINE_SPAN_TICK1_FROM);
        barline->setSpanTo(BARLINE_SPAN_TICK1_TO);
    } else if (barStyle == u"short") {
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
    if (tick == measure->endTick()) {
        st = SegmentType::EndBarLine;
    } else if (tick == measure->tick()) {
        st = SegmentType::BeginBarLine;
    }
    Segment* const segment = measure->getSegment(st, tick);
    barline->renderer()->layoutItem(barline.get());
    segment->add(barline.release());
}

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Parse the MusicXML top-level (XPath /score-partwise) node.
 */

void MusicXmlParserPass2::scorePartwise()
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "part") {
            part();
        } else if (m_e.name() == "part-list") {
            partList();
        } else {
            skipLogCurrElem();
        }
    }
    // set last measure barline to normal or MuseScore will generate light-heavy EndBarline
    // this creates non-generated barlines spanning only the current instrument
    // BarLine::_spanStaff is set using the default in Staff::_barLineSpan
    Measure* const lm = m_score->lastMeasure();
    if (lm && lm->endBarLineType() & BarLineType::NORMAL) {
        for (staff_idx_t staffidx = 0; staffidx < m_score->nstaves(); ++staffidx) {
            const Staff* staff = m_score->staff(staffidx);
            auto b = createBarline(m_score, staffidx * VOICES, BarLineType::NORMAL, true, u"", staff->barLineSpan());
            addBarlineToMeasure(lm, lm->endTick(), std::move(b));
        }
    }
    addError(checkAtEndElement(m_e, u"score-partwise"));

    for (EngravingItem* sysEl : muse::values(m_sysElements)) {
        m_score->undoAddElement(sysEl);

        // Remove potential duplicated text for text and text lines
        const bool elIsText = (sysEl->isTextBase() || sysEl->isTextLineBase());
        if (!elIsText) {
            continue;
        }
        const Score* score = sysEl->score();
        const Segment* seg = score ? score->tick2segment(sysEl->tick(), false, SegmentType::ChordRest) : nullptr;
        if (!seg) {
            continue;
        }
        const String sysElText = sysEl->isTextBase() ? toTextBase(sysEl)->plainText() : toTextLineBase(sysEl)->beginText();
        std::vector<EngravingItem*> annotations = seg->annotations();
        for (EngravingItem* existingEl : annotations) {
            const bool bothText = (existingEl->isTextBase() || existingEl->isTextLineBase()) && elIsText;
            if (existingEl && existingEl != sysEl && bothText) {
                const String existingText
                    = existingEl->isTextBase() ? toTextBase(existingEl)->plainText() : toTextLineBase(existingEl)->beginText();
                const bool textMatches = existingText == sysElText;
                const bool placementMatches = existingEl->placement() == sysEl->placement();
                const bool staffMatches = existingEl->staffIdx() == sysEl->staffIdx();
                if (textMatches && (!existingEl->systemFlag() || staffMatches) && placementMatches) {
                    m_score->removeElement(existingEl);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   partList
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list node.
 */

void MusicXmlParserPass2::partList()
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "score-part") {
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

void MusicXmlParserPass2::scorePart()
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "midi-instrument") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "score-instrument") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "part-name") {
            m_e.skipCurrentElement();        // skip but don't log
        } else {
            skipLogCurrElem();
        }
    }
}

static void createTimeTick(const Score* score, const Fraction& tick, const staff_idx_t staffIdx)
{
    Measure* const measure = score->tick2measure(tick);
    if (!measure->findSegment(SegmentType::TimeTick | SegmentType::ChordRest, tick)) {
        EditTimeTickAnchors::createTimeTickAnchor(measure, tick - measure->tick(), staffIdx);
    }
}

static String replacePartNameAccidentals(const String& partName)
{
    String name = partName;
    static const std::wregex re(L"((^|\\s|\\u00A0)[ABCDEF][b#\\u00AF]($|\\s|\\u00A0))");
    StringList res = name.search(re, { 1 }, SplitBehavior::SkipEmptyParts);
    if (!res.empty()) {
        String transp = res.at(0).replace(u"b", u"♭").replace(u"¯", u"♭").replace(u"#", u"♯");
        name.replace(re, transp);
        return name;
    }
    return partName;
}

//---------------------------------------------------------
//   part
//---------------------------------------------------------

/**
 Parse the /score-partwise/part node.
 */

void MusicXmlParserPass2::part()
{
    const String id = m_e.attribute("id");

    if (!m_pass1.hasPart(id)) {
        m_logger->logError(String(u"MusicXmlParserPass2::part cannot find part '%1'").arg(id), &m_e);
        skipLogCurrElem();
        return;
    }

    initPartState(id);

    const MusicXmlInstruments& instruments = m_pass1.getInstruments(id);
    m_hasDrumset = hasDrumset(instruments);

    // set the part name
    Part* part = m_pass1.getPart(id);
    MusicXmlPart mxmlPart = m_pass1.getMusicXmlPart(id);
    String partName = mxmlPart.getName();
    setPartInstruments(m_logger, &m_e, part, id, m_score, m_pass1.getInstrList(id), m_pass1.getIntervals(id), instruments, partName);
    partName = replacePartNameAccidentals(partName);
    part->setPartName(partName);
    if (mxmlPart.getPrintName() && !isLikelyIncorrectPartName(partName)) {
        part->setLongNameAll(partName);
    } else {
        m_pass1.getPart(id)->setLongNameAll(u"");
    }
    if (mxmlPart.getPrintAbbr()) {
        part->setPlainShortNameAll(mxmlPart.getAbbr());
    } else {
        m_pass1.getPart(id)->setPlainShortNameAll(u"");
    }
    // set the parts first instrument
    // try to prevent an empty track name
    if (part->partName() == "") {
        String instrId = m_pass1.getInstrList(id).instrument(Fraction(0, 1));
        part->setPartName(muse::value(instruments, instrId).name);
    }

#ifdef DEBUG_VOICE_MAPPER
    VoiceList voicelist = _pass1.getVoiceList(id);
    // debug: print voice mapper contents
    LOGD("voiceMapperStats: part '%s'", muPrintable(id));
    for (std::map<String, mu::engraving::VoiceDesc>::const_iterator i = voicelist.cbegin(); i != voicelist.cend(); ++i) {
        LOGD("voiceMapperStats: voice %s staff data %s",
             muPrintable(i.key()), muPrintable(i.value().toString()));
    }
#endif

    // read the measures
    size_t nr = 0; // current measure sequence number (always increments by one for each measure)
    m_measureNumber = 0; // written measure number (doesn't always increment by 1)
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "measure") {
            Fraction t = m_pass1.getMeasureStart(nr);
            if (t.isValid()) {
                measure(id, t);
            } else {
                m_logger->logError(String(u"no valid start time for measure %1").arg(nr + 1), &m_e);
                m_e.skipCurrentElement();
            }
            ++nr;
        } else {
            skipLogCurrElem();
        }
    }

    // stop all remaining extends for this part and add remaining ottava if present
    Measure* lm = part->score()->lastMeasure();
    if (lm) {
        track_idx_t strack = m_pass1.trackForPart(id);
        track_idx_t etrack = strack + part->nstaves() * VOICES;
        Fraction lastTick = lm->endTick();
        for (track_idx_t trk = strack; trk < etrack; trk++) {
            m_extendedLyrics.setExtend(-1, trk, lastTick);
        }
        if (m_delayedOttava && m_delayedOttava->tick2() < lastTick) {
            handleSpannerStop(m_delayedOttava, m_delayedOttava->track2(), lastTick, m_spanners);
            m_delayedOttava = nullptr;
        }
    }

    if (configuration()->inferTextType()) {
        for (Hairpin* hp : m_inferredHairpins) {
            hp->score()->addElement(hp);
        }
        for (GradualTempoChange* line : m_inferredTempoLines) {
            addSystemElement(line, line->tick());
        }
    }

    const SpannerSet incompleteSpanners =  findIncompleteSpannersAtPartEnd();
    //LOGD("spanner list:");
    auto i = m_spanners.cbegin();
    while (i != m_spanners.cend()) {
        SLine* const sp = i->first;
        Fraction tick1 = Fraction::fromTicks(i->second.first);
        Fraction tick2 = Fraction::fromTicks(i->second.second);
        //LOGD("spanner %p tp %d isHairpin %d tick1 %s tick2 %s track1 %d track2 %d start %p end %p",
        //       sp, sp->type(), sp->isHairpin(), muPrintable(tick1.toString()), muPrintable(tick2.toString()),
        //       sp->track(), sp->track2(), sp->startElement(), sp->endElement());
        if (incompleteSpanners.find(sp) == incompleteSpanners.end()) {
            // complete spanner found
            // Create time tick for hairpin
            if (sp->isHairpin()) {
                createTimeTick(m_score, tick1, track2staff(sp->track()));
            }
            sp->setTick(tick1);
            sp->setTick2(tick2);
            sp->score()->addElement(sp);
        } else {
            // incomplete spanner -> cleanup
            delete sp;
        }
        ++i;
    }
    m_spanners.clear();

    // Clean up any remaining ties
    MusicXmlTieMap tieMap = m_ties;
    for (auto& tie : tieMap) {
        if (tie.second) {
            m_unendedTieNotes.push_back(tie.second->startNote());
            m_ties.erase(tie.first);
        }
    }
    // Find ties between different voices which may have been missed
    for (Note* startNote : m_unendedTieNotes) {
        Tie* unendedTie = startNote->tieFor();
        if (!unendedTie) {
            continue;
        }
        const Chord* startChord = startNote->chord();
        const Measure* startMeasure = startChord ? startChord->measure() : nullptr;
        for (Note* endNote : m_unstartedTieNotes) {
            if (endNote->tieBack()) {
                continue;
            }
            const Chord* endChord = endNote->chord();
            if (startNote->pitch() == endNote->pitch()
                && (startMeasure == endChord->measure() || startChord->tick() + startChord->actualTicks() == endChord->tick())) {
                unendedTie->setEndNote(endNote);
                endNote->setTieBack(unendedTie);
            }
        }

        if (!unendedTie->endNote()) {
            // Tie started with no matching end tags
            // Find a note of the same pitch in the same voice immediately following the start chord
            Segment* nextSeg = startChord ? startChord->segment() : nullptr;
            while (nextSeg && nextSeg->tick() < startChord->tick() + startChord->ticks()) {
                nextSeg = nextSeg->nextCR(startChord->track(), true);
            }
            EngravingItem* nextEl = nextSeg ? nextSeg->element(startNote->track()) : nullptr;
            Chord* nextChord = nextEl && nextEl->isChord() ? toChord(nextEl) : nullptr;
            Note* matchingNote = nextChord ? nextChord->findNote(startNote->pitch()) : nullptr;

            if (matchingNote && matchingNote->tpc() == startNote->tpc() && matchingNote != startNote) {
                unendedTie->setEndNote(matchingNote);
                matchingNote->setTieBack(unendedTie);
            } else if (startChord) {
                // try other voices in the stave
                const Part* p = startChord->part();
                for (track_idx_t track = p->startTrack(); track < p->endTrack() + VOICES; track++) {
                    nextEl = nextSeg ? nextSeg->element(track) : nullptr;
                    nextChord = nextEl && nextEl->isChord() ? toChord(nextEl) : nullptr;
                    if (nextChord && nextChord->vStaffIdx() != startChord->vStaffIdx()) {
                        continue;
                    }
                    matchingNote = nextChord ? nextChord->findNote(startNote->pitch()) : nullptr;
                    if (matchingNote && matchingNote->tpc() == startNote->tpc() && matchingNote != startNote) {
                        unendedTie->setEndNote(matchingNote);
                        matchingNote->setTieBack(unendedTie);
                    }
                }
            }
        }

        if (!unendedTie->endNote()) {
            cleanupUnterminatedTie(unendedTie, m_score, m_pass1.exporterSoftware() == MusicXmlExporterSoftware::DOLET6);
        }
    }

    m_ties.clear();
    m_unstartedTieNotes.clear();
    m_unendedTieNotes.clear();

    if (m_hasDrumset) {
        Drumset* drumset = new Drumset;
        const MusicXmlInstruments& instrumentsAfterPass2 = m_pass1.getInstruments(id);
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

    addError(checkAtEndElement(m_e, u"part"));
}

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

/**
 In Score \a score find the measure starting at \a tick.
 */

static Measure* findMeasure(const Score* score, const Fraction& tick)
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

static void handleBeamAndStemDir(ChordRest* cr, const BeamMode bm, const DirectionV sd, Beam*& beam,
                                 bool hasBeamingInfo, Color beamColor, const String fan)
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
        beam->setDirection(sd);
        if (beamColor.isValid()) {
            beam->setColor(beamColor);
        }
        if (!fan.empty() && fan != u"none") {
            beam->setAsFeathered(fan == u"rit");
        }
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
                                const std::map<Note*, int>& alterMap)
{
    std::map<int, bool> accTmp;

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
            for (Note* nt : chord->notes()) {
                if (muse::contains(alterMap, nt)) {
                    int alter = alterMap.at(nt);
                    int ln  = absStep(nt->tpc(), nt->pitch());
                    bool error = false;
                    AccidentalVal currAccVal = currAcc.accidentalVal(ln, error);
                    if (error) {
                        continue;
                    }
                    if ((alter == -1
                         && currAccVal == AccidentalVal::FLAT
                         && nt->accidental()->accidentalType() == AccidentalType::FLAT
                         && !muse::value(accTmp, ln, false))
                        || (alter == 0
                            && currAccVal == AccidentalVal::NATURAL
                            && nt->accidental()->accidentalType() == AccidentalType::NATURAL
                            && !muse::value(accTmp, ln, false))
                        || (alter == 1
                            && currAccVal == AccidentalVal::SHARP
                            && nt->accidental()->accidentalType() == AccidentalType::SHARP
                            && !muse::value(accTmp, ln, false))) {
                        nt->accidental()->setRole(AccidentalRole::USER);
                    } else if (Accidental::isMicrotonal(nt->accidental()->accidentalType())
                               && nt->accidental()->accidentalType() < AccidentalType::END) {
                        // microtonal accidental
                        nt->accidental()->setRole(AccidentalRole::USER);
                        accTmp.insert({ ln, false });
                    } else {
                        accTmp.insert({ ln, true });
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
    for (const Note* n : mainChord->notes()) {
        anyPlays |= n->play();
    }
    if (!anyPlays) {
        for (Note* gn : graceChord->notes()) {
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

static void addGraceChordsAfter(Chord* c, GraceChordList& gcl, size_t& gac)
{
    if (!c) {
        return;
    }

    while (gac > 0) {
        if (gcl.size() > 0) {
            Chord* graceChord = muse::takeFirst(gcl);
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
    for (int i = static_cast<int>(gcl.size()) - 1; i >= 0; i--) {
        Chord* gc = gcl.at(i);
        std::vector<EngravingItem*> el = gc->el(); // copy, because modified during loop
        for (EngravingItem* e : el) {
            if (e->isFermata()) {
                c->segment()->add(e);
                gc->removeFermata(toFermata(e));
            }
        }
        c->add(gc); // TODO check if same voice ?
        coerceGraceCue(c, gc);
    }
    gcl.clear();
}

static bool canAddTempoText(const TempoMap* const tempoMap, const int tick)
{
    if (!muse::contains(*tempoMap, tick)) {
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

void MusicXmlParserPass2::measure(const String& partId, const Fraction time)
{
    // "measure numbers" don't have to be actual numbers in MusicXML
    bool isNumericMeasureNumber = false;
    int parsedMeasureNumber = 0;

    AsciiStringView numberA = m_e.asciiAttribute("number");
    if (!numberA.empty()) {
        if (numberA.at(0).ascii() == 'X') {
            isNumericMeasureNumber = false;
        } else {
            parsedMeasureNumber = numberA.toInt(&isNumericMeasureNumber);
        }
    }

    //LOGD("measure %d start", parsedMeasureNumber);

    Measure* measure = findMeasure(m_score, time);
    if (!measure) {
        m_logger->logError(String(u"measure at tick %1 not found!").arg(time.ticks()), &m_e);
        skipLogCurrElem();
        return;
    }

    if (m_e.asciiAttribute("implicit") == "yes") {
        // Implicit measure: expect measure number to be unchanged.
        measure->setIrregular(true);
    } else {
        // Normal measure: expect number to have increased by one.
        ++m_measureNumber;
    }

    if (isNumericMeasureNumber) {
        // Actual measure number may differ from expected value.
        measure->setNoOffset(parsedMeasureNumber - m_measureNumber);
        m_measureNumber = parsedMeasureNumber;
    }

    // set measure's RepeatFlag to none because musicXML is allowing single measure repeat and no ordering in repeat start and end barlines
    measure->setRepeatStart(false);
    measure->setRepeatEnd(false);

    /* TODO: for cutaway measures, i believe we can expect the staff to continue to be cutaway until another
    * print-object="yes" attribute is found. Here is the code that does that, though I don't want to actually commit this until
    * we have the exporter dealing with this sort of stuff as well.
    *
    * When print-object="yes" is encountered, the measure will explicitly be set to visible (see MusicXmlParserPass2::staffDetails)

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
    size_t gac = 0;         // grace after count in the grace chord list
    Beams beams; // Current beam for each voice in the current part
    String cv = u"1";         // current voice for chords, default is 1
    FiguredBassList fbl;                 // List of figured bass elements under a single note
    MusicXmlTupletStates tupletStates;         // Tuplet state for each voice in the current part
    Tuplets tuplets;         // Current tuplet for each voice in the current part
    DelayedDirectionsList delayedDirections; // Directions to be added to score *after* collecting all and sorting
    InferredFingeringsList inferredFingerings; // Directions to be reinterpreted as Fingerings
    ArpeggioMap arpMap;
    DelayedArpMap delayedArps;
    HarmonyMap delayedHarmony;
    bool measureHasCoda = false;
    String tempoString;  // helper for Dorico imports

    // collect candidates for courtesy accidentals to work out at measure end
    std::map<Note*, int> alterMap;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "attributes") {
            attributes(partId, measure, time + mTime);
        } else if (m_e.name() == "direction") {
            MusicXmlParserDirection dir(m_e, m_score, m_pass1, *this, m_logger);
            if (!tempoString.empty() && m_pass1.exporterSoftware() == MusicXmlExporterSoftware::DORICO) {
                dir.setBpm(tempoString.toDouble());
                tempoString.clear();
            }
            dir.direction(partId, measure, time + mTime, m_spanners, delayedDirections,
                          inferredFingerings, delayedHarmony, measureHasCoda, m_segnos);
        } else if (m_e.name() == "figured-bass") {
            FiguredBass* fb = figuredBass();
            if (fb) {
                fbl.push_back(fb);
            }
        } else if (m_e.name() == "harmony") {
            harmony(partId, measure, time + mTime, delayedHarmony);
        } else if (m_e.name() == "note") {
            if (!tempoString.empty()) {
                // sound tempo="..."
                // create an invisible default TempoText
                // to prevent duplicates, only if none is present yet
                Fraction tick = time + mTime;

                if (canAddTempoText(m_score->tempomap(), tick.ticks())) {
                    double tpo = tempoString.toDouble() / 60;
                    TempoText* t = Factory::createTempoText(m_score->dummy()->segment());
                    t->setXmlText(String(u"%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(DurationType::V_QUARTER)),
                                                         tempoString));
                    t->setVisible(false);
                    t->setTempo(tpo);
                    t->setFollowText(true);

                    m_score->setTempo(tick, tpo);

                    addElemOffset(t, m_pass1.trackForPart(partId), u"above", measure, tick);
                }
                tempoString.clear();
            }

            // Correct delayed ottava tick
            if (m_delayedOttava && m_delayedOttava->tick2() < time + mTime) {
                handleSpannerStop(m_delayedOttava, m_delayedOttava->track2(), time + mTime, m_spanners);
                m_delayedOttava = nullptr;
            }
            Fraction missingPrev;
            Fraction dura;
            Fraction missingCurr;
            int alt = -10;                          // any number outside range of xml-tag "alter"
            // note: chord and grace note handling done in note()
            // dura > 0 iff valid rest or first note of chord found
            Note* n = note(partId, measure, time + mTime, time + prevTime, missingPrev, dura, missingCurr, cv, gcl, gac, beams, fbl, alt,
                           tupletStates, tuplets, arpMap, delayedArps);
            if (n && !n->chord()->isGrace()) {
                prevChord = n->chord();          // remember last non-grace chord
            }
            if (n && n->accidental() && n->accidental()->accidentalType() != AccidentalType::NONE) {
                alterMap.insert({ n, alt });
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
        } else if (m_e.name() == "forward") {
            Fraction dura;
            forward(dura);
            if (dura.isValid()) {
                mTime += dura;
                if (mTime > mDura) {
                    mDura = mTime;
                }
            }
        } else if (m_e.name() == "backup") {
            Fraction dura;
            backup(dura);
            if (dura.isValid()) {
                if (dura <= mTime) {
                    mTime -= dura;
                } else {
                    m_logger->logError(u"backup beyond measure start", &m_e);
                    mTime.set(0, 1);
                }
                // check if the tick position is smaller than the minimum division resolution
                // (possibly caused by rounding errors) and in that case set position to 0
                if (mTime.isNotZero() && (m_divs > 0) && (mTime < Fraction(1, 4 * m_divs))) {
                    m_logger->logError(u"backup to a fractional tick smaller than the minimum division", &m_e);
                    mTime.set(0, 1);
                }
            }
        } else if (m_e.name() == "sound") {
            tempoString = m_e.attribute("tempo");
            m_e.skipCurrentElement();
        } else if (m_e.name() == "barline") {
            barline(partId, measure, time + mTime);
        } else if (m_e.name() == "print") {
            if (m_score->parts()[0] == m_pass1.getPart(partId)) {
                // only process for first part
                while (m_e.readNextStartElement()) {
                    if (m_e.name() == "page-layout") {
                        m_e.skipCurrentElement();            // skip but don't log
                    } else if (m_e.name() == "system-layout") {
                        m_e.skipCurrentElement();            // skip but don't log
                    } else if (m_e.name() == "staff-layout") {
                        m_e.skipCurrentElement();            // skip but don't log
                    } else if (m_e.name() == "measure-layout") {
                        measureLayout(measure);
                    } else {
                        skipLogCurrElem();
                    }
                }
            } else {
                m_e.skipCurrentElement();
            }
        } else {
            skipLogCurrElem();
        }

        /*
         LOGD("mTime %s (%s) mDura %s (%s)",
         muPrintable(mTime.print()),
         muPrintable(mTime.reduced().print()),
         muPrintable(mDura.print()),
         muPrintable(mDura.reduced().print()));
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
    Part* part = m_pass1.getPart(partId);   // should not fail, we only get here if the part exists
    fillGapsInFirstVoices(measure, part);

    // Prevent any beams from extending into the next measure
    for (Beam* beam : muse::values(beams)) {
        if (beam) {
            removeBeam(beam);
        }
    }

    // Sort and add inferred fingerings
    std::sort(inferredFingerings.begin(), inferredFingerings.end(),
              // Lambda: sort by absolute value of totalY
              [](const MusicXmlInferredFingering* a, const MusicXmlInferredFingering* b) -> bool {
        return std::abs(a->totalY()) < std::abs(b->totalY());
    }
              );
    for (MusicXmlInferredFingering* inferredFingering : inferredFingerings) {
        if (!inferredFingering->findAndAddToNotes(measure)) {
            // Could not find notes to add to; print as direction
            delayedDirections.push_back(inferredFingering->toDelayedDirection());
        }
        delete inferredFingering;
    }

    for (auto& harmony : delayedHarmony) {
        HarmonyDesc harmonyDesc = harmony.second;
        Fraction tick = Fraction::fromTicks(harmony.first);
        if (harmonyDesc.m_fretDiagram) {
            harmonyDesc.m_fretDiagram->setTrack(harmonyDesc.m_track);
            Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
            harmonyDesc.m_harmony->setProperty(Pid::ALIGN, Align(AlignH::HCENTER, AlignV::TOP));
            s->add(harmonyDesc.m_fretDiagram);
        }

        if (harmonyDesc.m_harmony) {
            harmonyDesc.m_harmony->setTrack(harmonyDesc.m_track);
            Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
            s->add(harmonyDesc.m_harmony);
        }
    }

    // Sort and add delayed directions
    std::sort(delayedDirections.begin(), delayedDirections.end(),
              // Lambda: sort by absolute value of totalY
              [](const MusicXmlDelayedDirectionElement* a, const MusicXmlDelayedDirectionElement* b) -> bool {
        return std::abs(a->totalY()) < std::abs(b->totalY());
    }
              );
    for (MusicXmlDelayedDirectionElement* direction : delayedDirections) {
        addElemOffset(direction->element(), direction->track(), direction->placement(), direction->measure(), direction->tick());
    }

    // TODO:
    // - how to handle _timeSigDura.isZero (shouldn't happen ?)
    // - how to handle unmetered music
    if (m_timeSigDura.isValid() && !m_timeSigDura.isZero()) {
        measure->setTimesig(m_timeSigDura);
    }

    // mark superfluous accidentals as user accidentals
    const staff_idx_t scoreRelStaff = m_score->staffIdx(part);
    const Key key = m_score->staff(scoreRelStaff)->keySigEvent(time).key();
    markUserAccidentals(scoreRelStaff, part->nstaves(), key, measure, alterMap);

    // multi-measure rest handling
    if (getAndDecMultiMeasureRestCount() == 0) {
        // measure is first measure after a multi-measure rest
        measure->setBreakMultiMeasureRest(true);
    }

    setMeasureRepeats(scoreRelStaff, measure);

    addError(checkAtEndElement(m_e, u"measure"));
}

//---------------------------------------------------------
//   setMeasureRepeats
//---------------------------------------------------------

/**
 Measure repeat handling, based on values set in measureStyle().
 */

void MusicXmlParserPass2::setMeasureRepeats(const staff_idx_t scoreRelStaff, Measure* measure)
{
    for (staff_idx_t i = 0; i < m_nstaves; ++i) {
        staff_idx_t staffIdx = scoreRelStaff + i;
        track_idx_t track = staff2track(staffIdx);
        if (m_measureRepeatNumMeasures[i]) {
            // delete anything already added to measure
            m_score->makeGap(measure->first(SegmentType::ChordRest), track, measure->stretchedLen(m_score->staff(staffIdx)), 0);

            if (m_measureRepeatCount[i] == m_measureRepeatNumMeasures[i]) {
                // starting a new one, not continuing a multi-measure group
                m_measureRepeatCount[i] = 1;
            } else {
                // continue building measure repeat group
                ++m_measureRepeatCount[i];
            }

            if (((m_measureRepeatNumMeasures[i] % 2) && (m_measureRepeatCount[i] - 1 == m_measureRepeatNumMeasures[i] / 2))
                || (!(m_measureRepeatNumMeasures[i] % 2) && (m_measureRepeatCount[i] == m_measureRepeatNumMeasures[i] / 2))) {
                // MeasureRepeat element goes in center measure of group if odd-numbered,
                // or last measure of first half of group if even-numbered
                m_score->addMeasureRepeat(measure->tick(), track, m_measureRepeatNumMeasures[i]);
            } else {
                // measures that are part of group but do not contain the element have undisplayed whole rests
                m_score->addRest(measure->tick(), track, TDuration(DurationType::V_MEASURE), 0);
            }
        } else {
            // measureStyle() hit a "stop" element most recently
            m_measureRepeatCount[i] = 0;
        }
        measure->setMeasureRepeatCount(m_measureRepeatCount[i], staffIdx);
    }
}

//---------------------------------------------------------
//   measureLayout
//---------------------------------------------------------

void MusicXmlParserPass2::measureLayout(Measure* measure)
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "measure-distance") {
            const Spatium val(m_e.readDouble() / 10.0);
            if (!measure->prev() || !measure->prev()->isHBox()) {
                MeasureBase* gap = m_score->insertBox(ElementType::HBOX, measure);
                toHBox(gap)->setBoxWidth(val);
            }
        } else {
            skipLogCurrElem();
        }
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

void MusicXmlParserPass2::attributes(const String& partId, Measure* measure, const Fraction& tick)
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "clef") {
            clef(partId, measure, tick);
        } else if (m_e.name() == "divisions") {
            divisions();
        } else if (m_e.name() == "key") {
            key(partId, measure, tick);
        } else if (m_e.name() == "measure-style") {
            measureStyle(measure);
        } else if (m_e.name() == "staff-details") {
            staffDetails(partId, measure);
        } else if (m_e.name() == "time") {
            time(partId, measure, tick);
        } else if (m_e.name() == "transpose") {
            m_e.skipCurrentElement();        // skip but don't log
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

void MusicXmlParserPass2::staffDetails(const String& partId, Measure* measure)
{
    //logDebugTrace("MusicXmlParserPass2::staffDetails");

    Part* part = m_pass1.getPart(partId);
    IF_ASSERT_FAILED(part) {
        return;
    }
    size_t staves = part->nstaves();

    String strNumber = m_e.attribute("number");
    int n = 0;  // default
    if (!strNumber.empty()) {
        n = m_pass1.getMusicXmlPart(partId).staffNumberToIndex(strNumber.toInt());
        if (n < 0 || n >= int(staves)) {
            m_logger->logError(String(u"invalid staff-details number %1 (may be hidden)").arg(strNumber), &m_e);
            n = 0;
        }
    }

    staff_idx_t staffIdx = m_score->staffIdx(part) + n;

    StringData stringData;
    AsciiStringView visible = m_e.asciiAttribute("print-object");
    AsciiStringView spacing = m_e.asciiAttribute("print-spacing");
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
            m_score->style().set(Sid::hideEmptyStaves, true);
            m_score->style().set(Sid::dontHideStavesInFirstSystem, false);
        } else {
            // this doesn't apply to a measure, so we'll assume the entire staff has to be hidden.
            m_score->staff(staffIdx)->setVisible(false);
        }
    } else if (visible == "yes" || visible.empty()) {
        if (measure) {
            m_score->staff(staffIdx)->setVisible(true);
            measure->setStaffVisible(staffIdx, true);
        }
    } else {
        m_logger->logError(String(u"print-object should be \"yes\" or \"no\""));
    }

    int staffLines = 0;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "staff-lines") {
            // save staff lines for later
            staffLines = m_e.readInt();
            // for a TAB staff also resize the string table and init with zeroes
            if (0 < staffLines) {
                stringData.stringList() = std::vector<instrString>(staffLines);
            } else {
                m_logger->logError(String(u"illegal staff-lines %1").arg(staffLines), &m_e);
            }
        } else if (m_e.name() == "line-detail") {
            const Color color = Color::fromString(m_e.attribute("color"));
            if (color.isValid()) {
                m_score->staff(staffIdx)->staffType(Fraction(0, 1))->setColor(color);
            }
            if (m_e.attribute("print-object") == u"no") {
                m_score->staff(staffIdx)->staffType(Fraction(0, 1))->setInvisible(true);
            }
            m_e.skipCurrentElement();
        } else if (m_e.name() == "staff-tuning") {
            staffTuning(&stringData);
        } else if (m_e.name() == "staff-size") {
            const double val = m_e.readDouble() / 100;
            m_score->staff(staffIdx)->setProperty(Pid::MAG, val);
        } else {
            skipLogCurrElem();
        }
    }

    if (staffLines > 0) {
        setStaffLines(m_score, staffIdx, staffLines);
    }

    Instrument* i = part->instrument();
    if (m_score->staff(staffIdx)->isTabStaff(Fraction(0, 1))) {
        if (i->stringData()->frets() == 0) {
            stringData.setFrets(25);
        } else {
            stringData.setFrets(i->stringData()->frets());
        }
        if (stringData.strings() > 0) {
            i->setStringData(stringData);
        }
    } else if (stringData.strings() > 0) {
        m_logger->logError(u"trying to change string data for non-TAB staff (not supported)", &m_e);
    }
}

//---------------------------------------------------------
//   staffTuning
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/staff-details/staff-tuning node.
 */

void MusicXmlParserPass2::staffTuning(StringData* t)
{
    //logDebugTrace("MusicXmlParserPass2::staffTuning");

    // ignore <staff-tuning> if not a TAB staff
    if (!t) {
        m_logger->logError(u"<staff-tuning> on non-TAB staff", &m_e);
        skipLogCurrElem();
        return;
    }

    int line   = m_e.intAttribute("line");
    int step   = 0;
    int alter  = 0;
    int octave = 0;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "tuning-alter") {
            alter = m_e.readText().trimmed().toInt();
        } else if (m_e.name() == "tuning-octave") {
            octave = m_e.readInt();
        } else if (m_e.name() == "tuning-step") {
            String strStep = m_e.readText();
            int pos = static_cast<int>(String(u"CDEFGAB").indexOf(strStep));
            if (strStep.size() == 1 && pos >= 0 && pos < 7) {
                step = pos;
            } else {
                m_logger->logError(String(u"invalid step '%1'").arg(strStep), &m_e);
            }
        } else {
            skipLogCurrElem();
        }
    }

    if (0 < line && line <= static_cast<int>(t->stringList().size())) {
        int pitch = MusicXmlStepAltOct2Pitch(step, alter, octave);
        if (pitch >= 0) {
            t->stringList()[line - 1].pitch = pitch;
        } else {
            m_logger->logError(String(u"invalid string %1 tuning step/alter/oct %2/%3/%4")
                               .arg(line).arg(step).arg(alter).arg(octave),
                               &m_e);
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

void MusicXmlParserPass2::measureStyle(Measure* measure)
{
    AsciiStringView staffNumberString = m_e.asciiAttribute("number");

    // by default, apply to all staves in part
    int startStaff = 0;
    int endStaff = static_cast<int>(m_nstaves) - 1;

    // but if a staff number was specified in the measure-style tag, use that instead
    if (!staffNumberString.empty()) {
        int staffNumber = staffNumberString.toInt();
        if (staffNumber < 1 || staffNumber > static_cast<int>(m_nstaves)) {
            m_logger->logError(String(u"measure-style staff number can only be int from 1 to _nstaves."));
        }
        --staffNumber; // convert to 0-based
        endStaff = startStaff = staffNumber;
    }

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "multiple-rest") {
            int multipleRest = m_e.readInt();
            if (multipleRest > 1) {
                m_multiMeasureRestCount = multipleRest;
                m_score->style().set(Sid::createMultiMeasureRests, true);
                measure->setBreakMultiMeasureRest(true);
            } else {
                m_logger->logError(String(u"multiple-rest %1 not supported").arg(multipleRest), &m_e);
            }
        } else if (m_e.name() == "measure-repeat") {
            String startStop = m_e.attribute("type");
            // note: possible "slashes" attribute is either redundant with numMeasures or not supported by MuseScore, so ignored either way
            if (startStop == u"start") {
                int numMeasures = m_e.readInt();
                for (int i = startStaff; i <= endStaff; i++) {
                    m_measureRepeatNumMeasures[i] = numMeasures;
                    m_measureRepeatCount[i] = numMeasures;   // measure repeat(s) haven't actually started yet in current measure, so this is a lie,
                                                             // but if we pretend it's true then everything is set for the next measure to restart
                }
            } else { // "stop"
                for (int i = startStaff; i <= endStaff; i++) {
                    m_measureRepeatNumMeasures[i] = 0;
                }
                m_e.skipCurrentElement(); // since not reading any text inside stop tag, we are done with this element
            }
        } else if (m_e.name() == "slash") {
            String type = m_e.attribute("type");
            String stems = m_e.attribute("use-stems");
            m_measureStyleSlash = type == u"start" ? (stems == u"yes" ? MusicXmlSlash::RHYTHM : MusicXmlSlash::SLASH) : MusicXmlSlash::NONE;
            m_e.skipCurrentElement();
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

static void preventNegativeTick(const Fraction& tick, Fraction& offset, MusicXmlLogger* logger)
{
    if (tick + offset < Fraction(0, 1)) {
        logger->logError(String(u"illegal offset %1 at tick %2").arg(offset.ticks()).arg(tick.ticks()));
        offset = -tick;
    }
}

String MusicXmlParserDirection::placement() const
{
    if (m_placement.empty() && hasTotalY()) {
        return totalY() < 0 ? u"above" : u"below";
    } else {
        return m_placement;
    }
}

static GradualTempoChangeType getTempoChangeTypeFromString(String txt)
{
    txt = txt.toLower();
    if (txt.contains(u"accel")) {
        return GradualTempoChangeType::Accelerando;
    } else if (txt.contains(u"allarg")) {
        return GradualTempoChangeType::Allargando;
    } else if (txt == u"calando") {
        return GradualTempoChangeType::Calando;
    } else if (txt.contains(u"lent")) {
        return GradualTempoChangeType::Lentando;
    } else if (txt == u"morendo") {
        return GradualTempoChangeType::Morendo;
    } else if (txt == u"precipitando") {
        return GradualTempoChangeType::Precipitando;
    } else if (txt.contains(u"rall")) {
        return GradualTempoChangeType::Rallentando;
    } else if (txt.contains(u"rit")) {
        return GradualTempoChangeType::Ritardando;
    } else if (txt.contains(u"smorz")) {
        return GradualTempoChangeType::Smorzando;
    } else if (txt == u"stringendo") {
        return GradualTempoChangeType::Stringendo;
    } else {
        return GradualTempoChangeType::Undefined;
    }
}

static void terminateInferredLine(const std::vector<TextLineBase*> lines, const Fraction& tick, const track_idx_t track)
{
    // Check staff and end any lines which are waiting
    if (configuration()->inferTextType()) {
        // To avoid extending lines which aren't intended to be terminated by their end markings,
        // only extend lines to markings within 24 quarter notes
        static const Fraction MAX_INFERRED_LINE_LEN = Fraction(24, 4);
        for (TextLineBase* line : lines) {
            Fraction diff = tick - line->tick();
            if (line && line->staffIdx() == track2staff(track)
                && line->ticks() == Fraction(0, 1) && diff <= MAX_INFERRED_LINE_LEN) {
                line->setTrack2(track);
                line->setTick2(tick);
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

void MusicXmlParserDirection::direction(const String& partId,
                                        Measure* measure,
                                        const Fraction& tick,
                                        MusicXmlSpannerMap& spanners,
                                        DelayedDirectionsList& delayedDirections,
                                        InferredFingeringsList& inferredFingerings,
                                        HarmonyMap& harmonyMap,
                                        bool& measureHasCoda,
                                        SegnoStack& segnos)
{
    //LOGD("direction tick %s", muPrintable(tick.print()));

    m_placement = m_e.attribute("placement");
    m_track = m_pass1.trackForPart(partId);
    bool isVocalStaff = m_pass1.isVocalStaff(partId);
    bool isPercussionStaff = m_pass1.isPercussionStaff(partId);
    bool isExpressionText = false;
    bool delayOttava = m_pass1.exporterSoftware() == MusicXmlExporterSoftware::SIBELIUS;
    m_systemDirection = m_e.attribute("system") == "only-top";
    //LOGD("direction track %d", track);
    std::vector<MusicXmlSpannerDesc> starts;
    std::vector<MusicXmlSpannerDesc> stops;
    bool isDynamicRange = false;
    bool tempoTextAdded = false;

    // note: file order is direction-type first, then staff
    // this means staff is still unknown when direction-type is handled
    // easiest solution is to put spanners on a stop and start list
    // and handle these after the while loop

    // note that placement is a <direction> attribute (which is currently supported by MS)
    // but the <direction-type> children also have formatting attributes
    // (currently NOT supported by MS, at least not for spanners when <direction> children)

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "direction-type") {
            directionType(starts, stops);
        } else if (m_e.name() == "offset") {
            m_offset = m_pass1.calcTicks(m_e.readInt(), m_pass2.divs(), &m_e);
            preventNegativeTick(tick, m_offset, m_logger);
        } else if (m_e.name() == "sound") {
            sound();
        } else if (m_e.name() == "staff") {
            String strStaff = m_e.readText();
            int staff = m_pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt());
            if (staff >= 0) {
                m_track += staff * VOICES;
            }
        } else {
            skipLogCurrElem();
        }
    }

    handleRepeats(measure, tick + m_offset, measureHasCoda, segnos, delayedDirections);
    handleNmiCmi(measure, tick + m_offset, delayedDirections);
    handleFraction();
    handleChordSym(tick + m_offset, harmonyMap);
    handleDrumInstrument(isPercussionStaff, tick + m_offset);

    // fix for Sibelius 7.1.3 (direct export) which creates metronomes without <sound tempo="..."/>:
    // if necessary, use the value calculated by metronome()
    // note: no floating point comparisons with 0 ...
    if (m_tpoSound < 0.1 && m_tpoMetro > 0.1) {
        m_tpoSound = m_tpoMetro;
    }

    //LOGD("words '%s' rehearsal '%s' metro '%s' tpo %g",
    //       muPrintable(_wordsText), muPrintable(_rehearsalText), muPrintable(_metroText), _tpoSound);

    // create text if any text was found
    if (isLyricBracket()) {
        return;
    } else if (isLikelyCredit(tick)) {
        Text* inferredText = addTextToHeader(TextStyleType::COMPOSER);
        if (inferredText) {
            m_pass1.setHasInferredHeaderText(true);
            hideRedundantHeaderText(inferredText, { u"lyricist", u"composer", u"poet" });
        }
    } else if (isLikelySubtitle(tick)) {
        Text* inferredText = addTextToHeader(TextStyleType::SUBTITLE);
        if (inferredText) {
            m_pass1.setHasInferredHeaderText(true);
            if (m_score->metaTag(u"source").isEmpty()) {
                m_score->setMetaTag(u"source", inferredText->plainText());
            }
            hideRedundantHeaderText(inferredText, { u"source" });
        }
    } else if (isLikelyLegallyDownloaded(tick)) {
        // Ignore (TBD: print to footer?)
        return;
    } else if (isLikelyTempoText(m_track)) {
        TempoText* tt = Factory::createTempoText(m_score->dummy()->segment());
        tt->setXmlText(m_wordsText + m_metroText);
        if (m_tpoSound > 0 && canAddTempoText(m_score->tempomap(), tick.ticks())) {
            double tpo = m_tpoSound / 60;
            tt->setTempo(tpo);
            if (tt->plainText().contains('=')) {
                tt->setFollowText(true);
            }
        }
        tt->setVisible(m_visible);

        m_pass2.addElemOffset(tt, m_track, placement(), measure, tick + m_offset);
        tempoTextAdded = true;
    } else if (isLikelyTempoLine(m_track)) {
        String simplifiedText = MScoreTextToMusicXml::toPlainText(m_wordsText).simplified();
        m_wordsText.clear();

        GradualTempoChangeType gtc = getTempoChangeTypeFromString(simplifiedText);

        GradualTempoChange* tempoLine = Factory::createGradualTempoChange(m_score->dummy()->segment());
        tempoLine->setTempoChangeType(gtc);
        tempoLine->setBeginText(simplifiedText);
        tempoLine->setContinueText(u"");
        m_inferredTempoLineStart = tempoLine;
    } else if (isLikelySticking()) {
        Sticking* sticking = Factory::createSticking(m_score->dummy()->segment());
        sticking->setXmlText(m_wordsText);
        if (!RealIsNull(m_relativeX)) {
            PointF offset = sticking->offset();
            offset.setX(m_relativeX);
            sticking->setOffset(offset);
            sticking->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        }

        if (hasTotalY()) {
            // Add element to score later, after collecting all the others and sorting by default-y
            // This allows default-y to be at least respected by the order of elements
            MusicXmlDelayedDirectionElement* delayedDirection = new MusicXmlDelayedDirectionElement(
                totalY(), sticking, m_track, placement(), measure, tick + m_offset);
            delayedDirections.push_back(delayedDirection);
        } else {
            m_pass2.addElemOffset(sticking, m_track, placement(), measure, tick + m_offset);
        }
    } else if (isLikelyDynamicRange()) {
        isDynamicRange = true;
    } else if (!m_wordsText.empty() || !m_rehearsalText.empty() || !m_metroText.empty()) {
        TextBase* t = 0;
        if (m_tpoSound > 0.1) {
            if (canAddTempoText(m_score->tempomap(), tick.ticks())) {
                m_tpoSound /= 60;
                t = Factory::createTempoText(m_score->dummy()->segment());
                String rawWordsText = m_wordsText;
                static const std::regex re("(<.*?>)");
                rawWordsText.remove(re);
                String sep = !m_metroText.empty() && !rawWordsText.empty() && rawWordsText.back() != ' ' ? u" " : String();
                t->setXmlText(m_wordsText + sep + m_metroText);
                ((TempoText*)t)->setTempo(m_tpoSound);
                if (t->plainText().contains('=')) {
                    ((TempoText*)t)->setFollowText(true);
                }
                m_score->setTempo(tick, m_tpoSound);
                tempoTextAdded = true;
            }
        } else {
            if (!m_wordsText.empty() || !m_metroText.empty()) {
                PlayingTechniqueType technique = PlayingTechniqueType::Undefined;
                if (!m_play.empty()) {
                    technique = TConv::fromXml(m_play.toAscii().constChar(), PlayingTechniqueType::Undefined);
                    m_play.clear();
                } else {
                    technique = getPlayingTechnique();
                }
                isExpressionText = m_wordsText.contains(u"<i>") && m_metroText.empty() && placement() == u"below";
                if (isExpressionText) {
                    t = Factory::createExpression(m_score->dummy()->segment());
                } else if (m_systemDirection) {
                    t = Factory::createSystemText(m_score->dummy()->segment());
                } else if (technique != PlayingTechniqueType::Undefined) {
                    t = Factory::createPlayTechAnnotation(m_score->dummy()->segment(), technique, TextStyleType::STAFF);
                } else {
                    t = Factory::createStaffText(m_score->dummy()->segment());
                }
                t->setXmlText(m_wordsText + m_metroText);
            } else {
                t = Factory::createRehearsalMark(m_score->dummy()->segment());
                if (!m_rehearsalText.contains(u"<b>")) {
                    m_rehearsalText = u"<b></b>" + m_rehearsalText;            // explicitly turn bold off
                }
                t->setXmlText(m_rehearsalText);
                if (!m_hasDefaultY) {
                    t->setPlacement(PlacementV::ABOVE);            // crude way to force placement TODO improve ?
                    t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                    t->resetProperty(Pid::OFFSET);
                }
            }
        }

        if (t) {
            if (m_enclosure == "circle") {
                t->setFrameType(FrameType::CIRCLE);
            } else if (m_enclosure == "none") {
                t->setFrameType(FrameType::NO_FRAME);
            } else if (m_enclosure == "rectangle") {
                t->setFrameType(FrameType::SQUARE);
                t->setFrameRound(0);
            }

            if (m_color.isValid()) {
                t->setColor(m_color);
            }

            if (configuration()->importLayout()) {
                if (m_justify == u"right") {
                    t->setAlign(AlignH::RIGHT);
                } else if (m_justify == u"center") {
                    t->setAlign(AlignH::HCENTER);
                } else {
                    t->setAlign(AlignH::LEFT);
                }
            }

            t->setVisible(m_visible);

            if (m_swing.second != 0) {
                toStaffTextBase(t)->setSwing(true);
                toStaffTextBase(t)->setSwingParameters(m_swing.first,
                                                       m_swing.first ? m_swing.second
                                                       : toStaffTextBase(t)->style().styleI(Sid::swingRatio));
                m_swing.second = 0;
            }

            String wordsPlacement = m_placement;
            // Case-based defaults
            if (wordsPlacement.empty()) {
                if (isVocalStaff) {
                    wordsPlacement = u"above";
                } else if (isExpressionText) {
                    wordsPlacement = u"below";
                }
            }

            String fingeringStr = m_wordsText;
            static const std::regex xmlFormating("(<.*?>)");
            fingeringStr.remove(xmlFormating).remove(u'\u00A0');
            if (isLikelyFingering(fingeringStr)) {
                m_logger->logDebugInfo(String(u"Inferring fingering: %1").arg(fingeringStr));
                t->setXmlText(fingeringStr);
                MusicXmlInferredFingering* inferredFingering = new MusicXmlInferredFingering(totalY(), t, fingeringStr, m_track,
                                                                                             placement(), measure, tick + m_offset);
                inferredFingerings.push_back(inferredFingering);
            } else {
                if (hasTotalY()) {
                    // Add element to score later, after collecting all the others and sorting by default-y
                    // This allows default-y to be at least respected by the order of elements
                    MusicXmlDelayedDirectionElement* delayedDirection = new MusicXmlDelayedDirectionElement(
                        totalY(), t, m_track, placement(), measure, tick + m_offset);
                    delayedDirections.push_back(delayedDirection);
                } else {
                    m_pass2.addElemOffset(t, m_track, placement(), measure, tick + m_offset);
                }
            }
        }
    } else if (m_tpoSound > 0) {
        // direction without text but with sound tempo="..."
        // create an invisible default TempoText

        if (canAddTempoText(m_score->tempomap(), tick.ticks())) {
            double tpo = m_tpoSound / 60;
            TempoText* t = Factory::createTempoText(m_score->dummy()->segment());
            t->setXmlText(String(u"%1 = %2").arg(TempoText::duration2tempoTextString(TDuration(DurationType::V_QUARTER))).arg(
                              m_tpoSound));
            t->setVisible(false);
            t->setTempo(tpo);
            t->setFollowText(true);

            // TBD may want ro use tick + _offset if sound is affected
            m_score->setTempo(tick, tpo);

            m_pass2.addElemOffset(t, m_track, placement(), measure, tick + m_offset);
            tempoTextAdded = true;
        }
    }

    if (tempoTextAdded) {
        const InferredTempoLineStack& lines = m_pass2.getInferredTempoLine();
        terminateInferredLine(std::vector<TextLineBase*>(lines.begin(), lines.end()), tick + m_offset, m_track);
    }
    addInferredTempoLine(tick + m_offset);

    Dynamic* firstDyn = nullptr;

    // do dynamics
    // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
    for (StringList::iterator it = m_dynamicsList.begin(); it != m_dynamicsList.end(); ++it) {
        Dynamic* dyn = Factory::createDynamic(m_score->dummy()->segment());
        dyn->setDynamicType(*it);

        if (isDynamicRange) {
            if (it == m_dynamicsList.begin()) {
                firstDyn = dyn;
            } else if (it == m_dynamicsList.end() - 1 && firstDyn) {
                // append hyphen and this dynamic to first
                firstDyn->setXmlText(firstDyn->xmlText() + u"<sym>dynamicCombinedSeparatorHyphen</sym>" + dyn->xmlText());
                delete dyn;
                continue;
            }
        }

        if (!m_dynaVelocity.empty()) {
            int dynaValue = round(m_dynaVelocity.toDouble() * 0.9);
            if (dynaValue > 127) {
                dynaValue = 127;
            } else if (dynaValue < 0) {
                dynaValue = 0;
            }
            dyn->setVelocity(dynaValue);
        }

        dyn->setVisible(m_visible);

        String dynamicsPlacement = placement();
        // Case-based defaults
        if (dynamicsPlacement.empty()) {
            dynamicsPlacement = isVocalStaff ? u"above" : u"below";
        }

        const InferredHairpinsStack& hairpins = m_pass2.getInferredHairpins();
        terminateInferredLine(std::vector<TextLineBase*>(hairpins.begin(), hairpins.end()), tick + m_offset, m_track);

        // Add element to score later, after collecting all the others and sorting by default-y
        // This allows default-y to be at least respected by the order of elements
        MusicXmlDelayedDirectionElement* delayedDirection = new MusicXmlDelayedDirectionElement(
            hasTotalY() ? totalY() : 100, dyn, m_track, dynamicsPlacement, measure, tick + m_offset);
        delayedDirections.push_back(delayedDirection);
    }

    addInferredHairpin(tick + m_offset, isVocalStaff);

    // handle the elems
    for (EngravingItem* elem : m_elems) {
        // TODO (?) if (_hasDefaultY) elem->setYoff(_defaultY);
        if (hasTotalY()) {
            MusicXmlDelayedDirectionElement* delayedDirection = new MusicXmlDelayedDirectionElement(
                totalY(), elem, m_track, placement(), measure, tick + m_offset);
            delayedDirections.push_back(delayedDirection);
        } else {
            m_pass2.addElemOffset(elem, m_track, placement(), measure, tick + m_offset);
        }
    }

    // handle the spanner stops first
    for (MusicXmlSpannerDesc desc : stops) {
        MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ desc.tp, desc.nr });
        if (spdesc.isStopped) {
            m_logger->logError(u"spanner already stopped", &m_e);
            delete desc.sp;
        } else {
            if (spdesc.isStarted) {
                // Adjustments to ottavas by the offset value are unwanted
                const Fraction spTick = spdesc.sp && spdesc.sp->isOttava() ? tick : tick + m_offset;
                if (spdesc.sp && spdesc.sp->isOttava() && delayOttava) {
                    // Sibelius writes ottava ends 1 note too early
                    m_pass2.setDelayedOttava(spdesc.sp);
                    m_pass2.delayedOttava()->setTrack2(m_track);
                    m_pass2.delayedOttava()->setTick2(spTick);
                    // need to set tick again later
                    m_pass2.clearSpanner(desc);
                } else {
                    handleSpannerStop(spdesc.sp, m_track, spTick, spanners);
                    m_pass2.clearSpanner(desc);
                }
            } else {
                spdesc.sp = desc.sp;
                const Fraction spTick = spdesc.sp && spdesc.sp->isOttava() ? tick : tick + m_offset;
                spdesc.tick2 = spTick;
                spdesc.track2 = m_track;
                spdesc.isStopped = true;
            }
        }
    }

    // then handle the spanner starts
    // TBD handle offset ?
    for (MusicXmlSpannerDesc desc : starts) {
        MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ desc.tp, desc.nr });
        if (spdesc.isStarted) {
            m_logger->logError(u"spanner already started", &m_e);
            delete desc.sp;
        } else {
            String spannerPlacement = placement();
            // Case-based defaults
            if (spannerPlacement.empty()) {
                if (desc.sp->isHairpin()) {
                    spannerPlacement = isVocalStaff ? u"above" : u"below";
                } else {
                    spannerPlacement = totalY() < 0 ? u"above" : u"below";
                }
            }
            desc.sp->setVisible(m_visible);
            if (spdesc.isStopped) {
                m_pass2.addSpanner(desc);
                // handleSpannerStart and handleSpannerStop must be called in order
                // due to allocation of elements in the map
                handleSpannerStart(desc.sp, m_track, spannerPlacement, tick + m_offset, spanners);
                handleSpannerStop(spdesc.sp, spdesc.track2, spdesc.tick2, spanners);
                m_pass2.clearSpanner(desc);
            } else {
                m_pass2.addSpanner(desc);
                handleSpannerStart(desc.sp, m_track, spannerPlacement, tick + m_offset, spanners);
                spdesc.isStarted = true;
            }
        }
    }
}

//---------------------------------------------------------
//   isLikelyCredit
//---------------------------------------------------------

bool MusicXmlParserDirection::isLikelyCredit(const Fraction& tick) const
{
    if (!configuration()->inferTextType()) {
        return false;
    }
    return (tick + m_offset < Fraction(5, 1)) // Only early in the piece
           && m_rehearsalText.empty()
           && m_metroText.empty()
           && m_tpoSound < 0.1
           && isLikelyCreditText(m_wordsText, false);
}

//---------------------------------------------------------
//   isLyricBracket
//    Dolet exports lyric brackets as staff text,
//    which we ought not render.
//---------------------------------------------------------

bool MusicXmlParserDirection::isLyricBracket() const
{
    if (m_wordsText.empty()) {
        return false;
    }

    if (!(m_wordsText.front() == u'}' || m_wordsText.back() == u'{')) {
        return false;
    }

    return m_rehearsalText.empty()
           && m_metroText.empty()
           && m_dynamicsList.empty()
           && m_tpoSound < 0.1;
}

bool MusicXmlParserDirection::isLikelySubtitle(const Fraction& tick) const
{
    if (!configuration()->inferTextType()) {
        return false;
    }
    return (tick + m_offset < Fraction(5, 1)) // Only early in the piece
           && m_rehearsalText.empty()
           && m_metroText.empty()
           && m_tpoSound < 0.1
           && isLikelySubtitleText(m_wordsText, false);
}

bool MusicXmlParserDirection::isLikelyLegallyDownloaded(const Fraction& tick) const
{
    if (!configuration()->inferTextType()) {
        return false;
    }
    return (tick + m_offset < Fraction(5, 1))   // Only early in the piece
           && m_rehearsalText.empty()
           && m_metroText.empty()
           && m_tpoSound < 0.1
           && m_wordsText.contains(std::wregex(L"This music has been legally downloaded\\.(?:\\s|\\u00A0)Do not photocopy\\."));
}

bool MusicXmlParserDirection::isLikelyTempoText(const track_idx_t track) const
{
    if (!configuration()->inferTextType() || m_wordsText.contains(u"<i>") || m_wordsText.contains(u"“")
        || m_wordsText.contains(u"”") || placement() == u"below"
        || track2staff(track) || m_wordsText.empty() || m_swing.second) {
        return false;
    }

    const String plainText = MScoreTextToMusicXml::toPlainText(m_wordsText.simplified());
    static const std::array<String,
                            29> tempoStrs
        = { u"adag", u"alleg", u"andant", u"a tempo", u"ballad", u"brisk", u"determination", u"dolce", u"expressive",
            u"fast", u"free", u"gently", u"grave", u"larg", u"lento", u"stesso tempo", u"lively", u"maestoso", u"moderat", u"mosso",
            u"prest", u"rubato", u"slow", u"straight", u"tango", u"tempo i", u"tenderly", u"triumphant", u"vivace" };

    for (const String& str : tempoStrs) {
        if (plainText.contains(str, CaseSensitivity::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

void MusicXmlParserDirection::handleFraction()
{
    if (!configuration()->inferTextType()) {
        return;
    }

    String rawWordsText = m_wordsText;
    static const std::regex re("(<.*?>)");
    rawWordsText.remove(re);
    rawWordsText = rawWordsText.simplified();
    //                         UTF-16 encoding: 0x00BC, 0x00BD, 0x00BE, 0x2150, 0x2151, 0x2152,  etc...
    static const std::array<String, 18> fracs { u"1/4", u"1/2", u"3/4", u"1/7", u"1/9", u"1/10", u"1/3", u"2/3", u"1/5", u"2/5", u"3/5",
                                                u"4/5", u"1/6", u"5/6", u"1/8", u"3/8", u"5/8", u"7/8" };

    for (size_t n = 0; n < fracs.size(); n++) {
        if (rawWordsText.contains(fracs.at(n))) {
            size_t p = n <= 2 ? 0x00BC + n : 0x2150 + n - 3;
            rawWordsText.replace(fracs.at(n), String(char16_t(p)));
            m_wordsText = rawWordsText;
            return;
        }
    }
}

bool MusicXmlParserDirection::isLikelyTempoLine(const track_idx_t track) const
{
    if (!configuration()->inferTextType() || m_wordsText.contains(u"“")
        || m_wordsText.contains(u"”") || placement() == u"below"
        || track2staff(track) != 0 || m_wordsText.empty()) {
        return false;
    }

    const String plainText = MScoreTextToMusicXml::toPlainText(m_wordsText.simplified());
    static const std::wregex re(L"(^|(?:\\s|\u00A0))(accel|faster|mosso|rit|rubato|slower|stretto)", std::regex_constants::icase);

    return plainText.contains(re);
}

Text* MusicXmlParserDirection::addTextToHeader(const TextStyleType textStyleType)
{
    Text* t = Factory::createText(m_score->dummy(), textStyleType);
    t->setXmlText(m_wordsText.trimmed());
    MeasureBase* const firstMeasure = m_score->measures()->first();
    VBox* vbox
        = firstMeasure->isVBox() ? toVBox(firstMeasure) : MusicXmlParserPass1::createAndAddVBoxForCreditWords(m_score, Fraction(0, 1));
    double spatium = m_score->style().styleD(Sid::spatium);
    vbox->setBoxHeight(vbox->boxHeight() + Spatium(t->height() / spatium / 2)); // add some height
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

void MusicXmlParserDirection::hideRedundantHeaderText(const Text* inferredText, const std::vector<String> metaTags)
{
    if (!inferredText->parent()->isVBox()) {
        return;
    }

    for (EngravingItem* e : toVBox(inferredText->parent())->el()) {
        if (e == inferredText || !e->isText()) {
            continue;
        }

        Text* t = toText(e);
        for (const String& metaTag : metaTags) {
            if (t->plainText() == m_score->metaTag(metaTag)) {
                t->setVisible(false);
                continue;
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

void MusicXmlParserDirection::directionType(std::vector<MusicXmlSpannerDesc>& starts,
                                            std::vector<MusicXmlSpannerDesc>& stops)
{
    while (m_e.readNextStartElement()) {
        m_defaultY = m_e.asciiAttribute("default-y").toDouble(&m_hasDefaultY) * -0.1;
        m_relativeX = m_e.doubleAttribute("relative-x") / 10 * m_score->style().spatium();
        m_visible = m_e.asciiAttribute("print-object") != "no";
        String number = m_e.attribute("number");
        int n = 0;
        if (!number.empty()) {
            n = number.toInt();
            if (n <= 0) {
                m_logger->logError(String(u"invalid number %1").arg(number), &m_e);
            } else {
                n--;          // make zero-based
            }
        }
        String type = m_e.attribute("type");
        m_color = Color::fromString(m_e.asciiAttribute("color").ascii());
        m_justify = m_e.attribute("justify");
        if (m_e.name() == "metronome") {
            m_metroText = metronome(m_tpoMetro);
        } else if (m_e.name() == "words") {
            m_enclosure = m_e.attribute("enclosure");
            m_fontFamily = m_e.attribute("font-family");
            String nextPart = xmlpass2::nextPartOfFormattedString(m_e);

            textToDynamic(nextPart);
            textToCrescLine(nextPart);
            handleTempo(nextPart);
            m_wordsText += nextPart;
        } else if (m_e.name() == "rehearsal") {
            m_enclosure = m_e.attribute("enclosure");
            if (m_enclosure.empty()) {
                m_enclosure = u"square";          // note different default
            }
            m_rehearsalText += xmlpass2::nextPartOfFormattedString(m_e);
        } else if (m_e.name() == "harp-pedals") {
            harpPedal();
        } else if (m_e.name() == "pedal") {
            pedal(type, n, starts, stops);
        } else if (m_e.name() == "octave-shift") {
            octaveShift(type, n, starts, stops);
        } else if (m_e.name() == "dynamics") {
            dynamics();
        } else if (m_e.name() == "bracket") {
            bracket(type, n, starts, stops);
        } else if (m_e.name() == "dashes") {
            dashes(type, n, starts, stops);
        } else if (m_e.name() == "wedge") {
            wedge(type, n, starts, stops);
        } else if (m_e.name() == "coda") {
            m_wordsText += u"<sym>coda</sym>";
            m_e.skipCurrentElement();
        } else if (m_e.name() == "segno") {
            m_wordsText += u"<sym>segno</sym>";
            m_e.skipCurrentElement();
        } else if (m_e.name() == "symbol") {
            const String smufl = m_e.readText();
            if (!smufl.empty()) {
                m_wordsText += u"<sym>" + smufl + u"</sym>";
            }
        } else if (m_e.name() == "other-direction") {
            otherDirection();
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

void MusicXmlParserDirection::sound()
{
    m_sndCoda = m_e.attribute("coda");
    m_sndDacapo = m_e.attribute("dacapo");
    m_sndDalsegno = m_e.attribute("dalsegno");
    m_sndFine = m_e.attribute("fine");
    m_sndSegno = m_e.attribute("segno");
    m_sndToCoda = m_e.attribute("tocoda");
    m_tpoSound = m_e.doubleAttribute("tempo");
    m_dynaVelocity = m_e.attribute("dynamics");

    const String pizz = m_e.attribute("pizzicato");
    if (pizz == u"yes") {
        m_play = u"pizzicato";
    } else if (pizz == u"no") {
        m_play = u"natural";
    }

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "play") {
            play();
        } else if (m_e.name() == "swing") {
            swing();
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   play
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/sound/play node.
 */

void MusicXmlParserDirection::play()
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "mute") {
            const String muted = m_e.readText();
            m_play = (muted == u"off") ? u"open" : u"mute";
        } else if (m_e.name() == "other-play") {
            m_play = m_e.attribute("type");
            m_e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   swing
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/sound/swing node.
 */

void MusicXmlParserDirection::swing()
{
    int swingNumerator = 1;
    int swingDenominator = 1;
    int swingUnit = 0;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "straight") {
            // unused
            m_e.skipCurrentElement();
        } else if (m_e.name() == "first") {
            swingDenominator = m_e.readText().toInt();
        } else if (m_e.name() == "second") {
            swingNumerator = m_e.readText().toInt();
        } else if (m_e.name() == "swing-type") {
            const String swingType = m_e.readText();
            if (swingType == u"eighth") {
                swingUnit = Constants::DIVISION / 2;
            } else if (swingType == u"16th") {
                swingUnit = Constants::DIVISION / 4;
            }
        } else if (m_e.name() == "swing-style") {
            // unused
            m_e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }
    }
    m_swing.first = swingUnit;
    m_swing.second = (swingNumerator * 100) / swingDenominator;
}

//---------------------------------------------------------
//   dynamics
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/dynamics node.
 */

void MusicXmlParserDirection::dynamics()
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "other-dynamics") {
            m_dynamicsList.push_back(m_e.readText());
        } else {
            m_dynamicsList.push_back(String::fromAscii(m_e.name().ascii()));
            m_e.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   harpPedal
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/hair-pedal node.
 */

void MusicXmlParserDirection::harpPedal()
{
    const std::vector <String> pedalSteps = { u"D", u"C", u"B", u"E", u"F", u"G", u"A" };
    const Color color = Color::fromString(m_e.attribute("color"));

    HarpPedalDiagram* hpd = Factory::createHarpPedalDiagram(m_score->dummy()->segment());
    while (m_e.readNextStartElement()) {
        int stepIndex = 0;
        PedalPosition pedpos = PedalPosition::UNSET;
        while (m_e.readNextStartElement()) {
            if (m_e.name() == "pedal-step") {
                stepIndex = std::distance(std::begin(pedalSteps), std::find(std::begin(pedalSteps), std::end(pedalSteps), m_e.readText()));
            } else if (m_e.name() == "pedal-alter") {
                pedpos = static_cast<PedalPosition>(m_e.readText().trimmed().toInt() + 1);
            } else {
                skipLogCurrElem();
            }
        }
        hpd->setPedal(static_cast<HarpStringType>(stepIndex), pedpos);
        if (color.isValid()) {
            hpd->setColor(color);
        }
    }
    m_elems.push_back(hpd);
}

void MusicXmlParserDirection::otherDirection()
{
    // <other-direction> element is used to define any <direction> symbols not yet in the MusicXML format
    const String smufl = m_e.attribute("smufl");
    const Color color = Color::fromString(m_e.attribute("color"));

    // Read smufl symbol
    if (!smufl.empty()) {
        SymId id = SymNames::symIdByName(smufl, SymId::noSym);
        if (id != SymId::noSym) {
            Symbol* smuflSym = Factory::createSymbol(m_score->dummy());
            smuflSym->setSym(id);
            if (color.isValid()) {
                smuflSym->setColor(color);
            }
            m_elems.push_back(smuflSym);
        }
        m_e.skipCurrentElement();
    } else {
        // TODO: Multiple sets of maps for exporters other than Dolet 6/Sibelius
        // TODO: Add more symbols from Sibelius
        std::map<String, String> otherDirectionStrings;
        if (m_pass1.dolet()) {
            otherDirectionStrings = {
                { String(u"To Coda"), String(u"To Coda") },
                { String(u"Segno"), String(u"<sym>segno</sym>") },
                { String(u"CODA"), String(u"CODA") },
            };
        }
        static const std::map<String, SymId> otherDirectionSyms = { { String(u"Rhythm dot"), SymId::augmentationDot },
            { String(u"Whole rest"), SymId::restWhole },
            { String(u"l.v. down"), SymId::articLaissezVibrerBelow },
            { String(u"8vb"), SymId::ottavaBassaVb },
            { String(u"Treble clef"), SymId::gClef },
            { String(u"Bass clef"), SymId::fClef },
            { String(u"Caesura"), SymId::caesura },
            { String(u"Thick caesura"), SymId::caesuraThick }
        };
        String t = m_e.readText();
        String val = muse::value(otherDirectionStrings, t);
        if (!val.empty()) {
            m_wordsText += val;
        } else {
            SymId sym = muse::value(otherDirectionSyms, t);
            Symbol* smuflSym = Factory::createSymbol(m_score->dummy());
            smuflSym->setSym(sym);
            if (color.isValid()) {
                smuflSym->setColor(color);
            }
            m_elems.push_back(smuflSym);
        }
    }
}

static const String getMarkerId(const String& words)
{
    // Helper function to set the marker ID string
    // eg. extract 'II' from 'To Coda II'
    static const std::regex number("(i{0,3})");
    const StringList nums = words.search(number, { 1 },  SplitBehavior::SkipEmptyParts);
    if (!nums.empty()) {
        return nums.at(0);
    }
    return String();
}

String MusicXmlParserDirection::matchRepeat(const String& plainWords) const
{
    static const std::wregex daCapo(L"^(d\\.? ?|da )(c\\.?|capo)$");
    static const std::wregex daCapoAlFine(L"^(d\\.? ?|da )(c\\.? ?|capo )al fine$");
    static const std::wregex daCapoAlCoda(L"^(d\\.? ?|da )(c\\.? ?|capo )al coda ?(i{0,3})?$");
    static const std::wregex dalSegno(L"^(d\\.? ?|d[ae]l )(s\\.?s?\\.?|segno) ?(i{0,3})?$");
    static const std::wregex dalSegnoAlFine(L"^(d\\.? ?|d[ae]l )(s\\.?s?\\.?|segno\\.?) ?(i{0,3})? ?(\\(.*\\))? al fine$");
    static const std::wregex dalSegnoAlCoda(L"^(d\\.? ?|d[ae]l )(s\\.?s?\\.?|segno\\.?) ?(i{0,3})? ?(\\(.*\\))? al coda ?(i{0,3})?$");
    static const std::wregex fine(L"^fine$");
    static const std::wregex segno(L"^segno( segno)? ?(i{0,3})?$");
    static const std::wregex toCoda(L"^to coda( coda)? ?(i{0,3})?$");
    static const std::wregex coda(L"^coda( coda)? ?(i{0,3})?$");

    if (plainWords.contains(daCapo)) {
        return u"daCapo";
    }
    if (plainWords.contains(daCapoAlFine)) {
        return u"daCapoAlFine";
    }
    if (plainWords.contains(daCapoAlCoda)) {
        return u"daCapoAlCoda";
    }
    if (plainWords.contains(dalSegno)) {
        return u"dalSegno";
    }
    if (plainWords.contains(dalSegnoAlFine)) {
        return u"dalSegnoAlFine";
    }
    if (plainWords.contains(dalSegnoAlCoda)) {
        return u"dalSegnoAlCoda";
    }
    if (plainWords.contains(segno)) {
        return u"segno";
    }
    if (plainWords.contains(fine)) {
        return u"fine";
    }
    if (plainWords.contains(toCoda)) {
        return u"toCoda";
    }
    if (plainWords.contains(coda)) {
        return u"coda";
    }
    return String();
}

static void addSymbolsToCoda(String& wordsString, const String& codaId)
{
    // Append correct number of symbols to coda text
    if (!wordsString.contains(u"<sym>coda</sym>")) {
        String end = u"\n";
        if (codaId.size() > 0) {
            end.append(codaId);
            end.replace(u"i", u"<sym>coda</sym>");
        } else {
            end.append(u"<sym>coda</sym>");
        }
        wordsString.append(end);
    }
}

//---------------------------------------------------------
//   isLikelyFingering
//---------------------------------------------------------

bool MusicXmlParserDirection::isLikelyFingering(const String& fingeringStr) const
{
    if (!configuration()->inferTextType()) {
        return false;
    }

    // One or more newline-separated digits, possibly lead or trailed by whitespace
    static const std::wregex re(
        L"^(?:\\s|\\u00A0)*[-–]?(?:\\s|\\u00A0)?[0-5pimac](?:[-–][0-5pimac])?(?:\n[-–]?(?:\\s|\\u00A0)?[0-5pimac](?:[-–][0-5pimac])?)*(?:\\s|\\u00A0)*$");
    return fingeringStr.contains(re)
           && m_rehearsalText.empty()
           && m_metroText.empty()
           && m_tpoSound < 0.1;
}

//---------------------------------------------------------
//   MusicXmlInferredFingering
//---------------------------------------------------------

MusicXmlInferredFingering::MusicXmlInferredFingering(double totalY,
                                                     EngravingItem* element,
                                                     String& text,
                                                     track_idx_t track,
                                                     String placement,
                                                     Measure* measure,
                                                     Fraction tick)
    : m_totalY(totalY), m_element(element),  m_text(text), m_track(track), m_placement(placement), m_measure(measure), m_tick(tick)
{
    m_fingerings = m_text.simplified().split(u" ");
}

//---------------------------------------------------------
//   roundTick
//---------------------------------------------------------
/**
 Round tick to multiple of gcd of measure
 */

void MusicXmlInferredFingering::roundTick(Measure* measure)
{
    measure->computeTicks();
    int gcdTicks = Fraction(1, 1).ticks();
    for (auto s = measure->segments().begin(); s != measure->segments().end(); ++s) {
        if ((*s).isChordRestType()) {
            gcdTicks = std::gcd(gcdTicks, (*s).ticks().ticks());
        }
    }
    if (!gcdTicks || gcdTicks == Fraction(1, 1).ticks() || !(m_tick.ticks() % gcdTicks)) {
        return;
    }
    int roundedTick = std::round(static_cast<double>(m_tick.ticks()) / static_cast<double>(gcdTicks)) * (gcdTicks);
    m_tick = Fraction::fromTicks(roundedTick);
}

//---------------------------------------------------------
//   findAndAddToNotes
//---------------------------------------------------------
/**
 Attempts to find an eligible collection of notes to add inferred
 fingerings to. Adds notes and returns true if successful, else no-op
 and returns false.
 */
bool MusicXmlInferredFingering::findAndAddToNotes(Measure* measure)
{
    roundTick(measure);
    std::vector<Note*> collectedNotes;
    for (track_idx_t track = m_track; track < m_track + 4; ++track) {
        Chord* candidateChord = measure->findChord(tick(), track);
        if (candidateChord) {
            if (candidateChord->notes().size() >= fingerings().size()) {
                addToNotes(candidateChord->notes());
                return true;
            } else {
                collectedNotes.insert(collectedNotes.begin(),
                                      candidateChord->notes().begin(),
                                      candidateChord->notes().end());
                if (collectedNotes.size() >= fingerings().size()) {
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
void MusicXmlInferredFingering::addToNotes(std::vector<Note*>& notes) const
{
    assert(notes.size() >= m_fingerings.size());
    for (size_t i = 0; i < m_fingerings.size(); ++i) {
        // Fingerings in reverse order
        addTextToNote(-1, -1,
                      m_fingerings[m_fingerings.size() - 1 - i], m_placement, u"", -1, u"", u"",
                      Color::BLACK, TextStyleType::FINGERING,
                      notes[i]->score(),
                      notes[i]);
    }
}

//---------------------------------------------------------
//   toDelayedDirection
//---------------------------------------------------------

MusicXmlDelayedDirectionElement* MusicXmlInferredFingering::toDelayedDirection()
{
    MusicXmlDelayedDirectionElement* dd = new MusicXmlDelayedDirectionElement(m_totalY, m_element, m_track, m_placement, m_measure, m_tick);
    return dd;
}

//---------------------------------------------------------
//   textToDynamic
//---------------------------------------------------------
/**
 Attempts to convert text to dynamic text. No-op if unable.
 */
void MusicXmlParserDirection::textToDynamic(String& text)
{
    if (!configuration()->inferTextType()) {
        return;
    }
    String simplifiedText = MScoreTextToMusicXml::toPlainText(text).simplified();
    // Correct finale's incorrect dynamic export
    if (m_pass1.exporterSoftware() == MusicXmlExporterSoftware::FINALE) {
        static const std::map<String,
                              String> finaleDynamicSubs
            = { { u"π", u"pp" }, { u"P", u"mp" }, { u"F", u"mf" }, { u"ƒ", u"ff" }, { u"Ï", u"fff" }, { u"S", u"sf" }, { u"ß", u"sfz" },
            { u"Z", u"fz" },  { u"Í", u"fp" } };
        for (const auto& sub : finaleDynamicSubs) {
            if (simplifiedText == sub.first) {
                simplifiedText = sub.second;
            }
        }
    }
    // We don't want to count a single 'm', 'r', 's' or 'z' as a whole dynamic
    static const std::wregex singleCharDynamic = std::wregex(L"^[mrsz]$");
    // try to find a dynamic - xml representation or
    // if found add to dynamics list and set text to blank string
    if (!simplifiedText.contains(singleCharDynamic) && TConv::dynamicValid(simplifiedText.toStdString())) {
        DynamicType dt = TConv::fromXml(simplifiedText.toStdString(), DynamicType::OTHER);
        if (dt != DynamicType::OTHER) {
            m_dynaVelocity = String::number(round(Dynamic::dynamicVelocity(dt) / 0.9));
            m_dynamicsList.push_back(Dynamic::dynamicText(dt));
            text.clear();
        }
    }
}

void MusicXmlParserDirection::textToCrescLine(String& text)
{
    if (!configuration()->inferTextType()) {
        return;
    }
    String simplifiedText = MScoreTextToMusicXml::toPlainText(text).simplified();
    bool cresc = simplifiedText.contains(u"cresc");
    bool dim = simplifiedText.contains(u"dim");
    if (!cresc && !dim) {
        return;
    }

    // Create line
    text.clear();
    Hairpin* line = Factory::createHairpin(m_score->dummy()->segment());

    line->setHairpinType(cresc ? HairpinType::CRESC_LINE : HairpinType::DIM_LINE);
    line->setBeginText(simplifiedText);
    line->setContinueText(u"");
    line->setProperty(Pid::LINE_VISIBLE, false);
    m_inferredHairpinStart = line;
}

void MusicXmlParserDirection::addInferredHairpin(const Fraction& tick, const bool isVocalStaff)
{
    if (!configuration()->inferTextType() || !m_inferredHairpinStart) {
        return;
    }

    m_inferredHairpinStart->setTrack(m_track);
    m_inferredHairpinStart->setTick(tick + m_offset);

    String spannerPlacement = m_placement;
    if (m_placement.empty()) {
        spannerPlacement = isVocalStaff ? u"above" : u"below";
    }
    setSLinePlacement(m_inferredHairpinStart, spannerPlacement);

    m_pass2.addInferredHairpin(m_inferredHairpinStart);
}

void MusicXmlParserDirection::addInferredTempoLine(const Fraction& tick)
{
    if (!configuration()->inferTextType() || !m_inferredTempoLineStart) {
        return;
    }

    m_inferredTempoLineStart->setTrack(m_track);
    m_inferredTempoLineStart->setTick(tick + m_offset);

    String spannerPlacement = placement();

    setSLinePlacement(m_inferredTempoLineStart, spannerPlacement);

    m_pass2.addInferredTempoLine(m_inferredTempoLineStart);
}

static String findDetachedRepeatNumber(const Measure* measure, const track_idx_t track, const Fraction tick, const String& placement,
                                       DelayedDirectionsList& directions)
{
    static const std::wregex number(L"^(i{1,3})$");
    DelayedDirectionsList::iterator it;

    for (it = directions.begin(); it != directions.end(); ++it) {
        MusicXmlDelayedDirectionElement d = **it;
        const EngravingItem* el = d.element();
        if (d.track() == track && d.tick() == tick && d.placement() == placement && el->isTextBase()) {
            const String text = toTextBase(el)->plainText();
            if (text.toLower().contains(number)) {
                it = directions.erase(it);
                return u" " + text;
            }
        }
    }

    Segment* s = measure->findSegment(SegmentType::ChordRest, tick);
    if (s) {
        for (EngravingItem* el : s->annotations()) {
            if (el->tick() == tick && el->track() == track && el->placeAbove() == (placement == u"above") && el->isTextBase()) {
                const String text = toTextBase(el)->plainText();
                if (text.toLower().contains(number)) {
                    s->removeAnnotation(el);
                    return u" " + text;
                }
            }
        }
    }

    return String();
}

static String countSegno(const String& plainWords)
{
    // Count how many 's' in D.S. or D.S.S...
    std::wsmatch matches;
    const std::wstring str = plainWords.toStdWString();
    static const std::wregex dalSegnoAlCoda(L"^(d\\.? ?|d[ae]l )(s\\.?s?\\.?|segno\\.?) ?(i{0,3})?(\\(.*\\))? al coda ?(i{0,3})?$");
    if (std::regex_search(str, matches, dalSegnoAlCoda)) {
        const String segnoGroup = String::fromStdWString(matches.str(2)).toLower();
        const int count = segnoGroup.count('s');
        return String::fromStdString(std::string(count, 'i'));
    }
    return String();
}

//---------------------------------------------------------
//   handleRepeats
//---------------------------------------------------------

void MusicXmlParserDirection::handleRepeats(Measure* measure, const Fraction tick, bool& measureHasCoda,
                                            SegnoStack& segnos, DelayedDirectionsList& delayedDirections)
{
    if (!configuration()->inferTextType()) {
        return;
    }
    // Try to recognize the various repeats
    String repeat;
    const String plainWords = MScoreTextToMusicXml::toPlainText(m_wordsText.toLower().simplified());
    if (!m_sndCoda.empty()) {
        repeat = u"coda";
    } else if (!m_sndDacapo.empty()) {
        repeat = u"daCapo";
    } else if (!m_sndDalsegno.empty()) {
        repeat = u"dalSegno";
    } else if (!m_sndFine.empty()) {
        repeat = u"fine";
    } else if (!m_sndSegno.empty()) {
        repeat = u"segno";
    } else if (!m_sndToCoda.empty()) {
        repeat = u"toCoda";
    } else {
        repeat = matchRepeat(plainWords);
    }
    // Check if repeat number has become detached
    if (repeat == u"coda" || repeat == u"segno") {
        m_wordsText += findDetachedRepeatNumber(measure, m_track, tick, placement(), delayedDirections);
    }

    // Check if coda is numbered by checking numerals after 'coda' or 'segno'
    if (repeat == u"daCapoAlCoda" || repeat == u"toCoda" || repeat == u"coda") {
        m_codaId = getMarkerId(plainWords);
    } else if (repeat == u"dalSegnoAlFine" || repeat == u"segno" || repeat == u"dalSegno") {
        m_segnoId = getMarkerId(plainWords);
    } else if (repeat == u"dalSegnoAlCoda") {
        // Find numerals eg D.S I al Coda II
        std::wsmatch matches;
        std::wstring str = plainWords.toStdWString();
        static const std::wregex dalSegnoAlCoda(L"^(d\\.? ?|d[ae]l )(s\\.?s?\\.?|segno\\.?) ?(i{0,3})?(\\(.*\\))? al coda ?(i{0,3})?$");
        if (std::regex_search(str, matches, dalSegnoAlCoda)) {
            m_segnoId = String::fromStdWString(matches.str(3));
            m_codaId = String::fromStdWString(matches.str(5));
        }
    }

    if ((repeat == u"dalSegno" || repeat == u"dalSegnoAlCoda") && m_segnoId.empty()) {
        // If no numeral, count how many 's' in D.S. or D.S.S...
        m_segnoId = countSegno(plainWords);
    }

    if (!repeat.empty()) {
        TextBase* tb = nullptr;
        if (m_codaId == u"i") {
            m_codaId.clear();
        }
        if (m_segnoId == u"i") {
            m_segnoId.clear();
        }
        m_wordsText = TextBase::unEscape(m_wordsText);
        if ((repeat == u"coda" || repeat == u"toCoda") && measureHasCoda) {
            // If there is already a coda in the bar, discard this marker
            m_wordsText.clear();
            return;
        } else if ((tb = findJump(repeat)) || (tb = findMarker(repeat))) {
            if (tb->isMarker()) {
                if (repeat == u"segno") {
                    if (Marker* prevSegno = muse::value(segnos, tick.ticks(), nullptr)) {
                        // If there is another segno at this tick, combine their text and increase the label
                        // eg. two segno symbols will have the label 'segnoii'
                        if (prevSegno->xmlText().contains(u"<sym>segno</sym>") && m_wordsText.contains(u"<sym>segno</sym>")) {
                            prevSegno->setXmlText(prevSegno->xmlText() + u" " + m_wordsText);
                            m_wordsText.clear();
                            const int segnoCount = prevSegno->xmlText().count(u"<sym>segno</sym>");
                            const String label = String::fromStdString(std::string(segnoCount, 'i'));
                            prevSegno->setLabel(prevSegno->label() + label);

                            return;
                        }
                    } else {
                        segnos.insert(std::pair<int, Marker*>(tick.ticks(), toMarker(tb)));
                    }
                } else {
                    if (repeat == u"coda" || repeat == u"toCoda") {
                        measureHasCoda = true;
                        addSymbolsToCoda(m_wordsText, m_codaId);
                    }
                }
            }

            tb->setTrack(m_track);
            if (!m_wordsText.empty()) {
                tb->setXmlText(m_wordsText);
                m_wordsText.clear();
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
            // Temporary solution to indent codas - add a horizontal frame at start of system or midway through
            if (tb->isMarker() && toMarker(tb)->markerType() == MarkerType::CODA) {
                MeasureBase* gap = m_score->insertBox(ElementType::HBOX, measure);
                toHBox(gap)->setBoxWidth(Spatium(10));
            }
            tb->setVisible(m_visible);
            measure->add(tb);
        }
    }
}

Marker* MusicXmlParserDirection::findMarker(const String& repeat) const
{
    Marker* m = nullptr;
    if (repeat == u"segno") {
        m = Factory::createMarker(m_score->dummy());
        // note: Marker::read() also contains code to set text style based on type
        // avoid duplicated code
        // apparently this MUST be after setTextStyle
        m->setMarkerType(MarkerType::SEGNO);
        m->setLabel(m->label() + m_segnoId);
    } else if (repeat == u"coda") {
        m = Factory::createMarker(m_score->dummy());
        m->setMarkerType(MarkerType::CODA);
        m->setLabel(m->label() + m_codaId);
    } else if (repeat == u"fine") {
        m = Factory::createMarker(m_score->dummy(), TextStyleType::REPEAT_RIGHT);
        m->setMarkerType(MarkerType::FINE);
    } else if (repeat == u"toCoda") {
        m = Factory::createMarker(m_score->dummy(), TextStyleType::REPEAT_RIGHT);
        m->setMarkerType(MarkerType::TOCODA);
        m->setLabel(m->label() + m_codaId);
    }
    return m;
}

Jump* MusicXmlParserDirection::findJump(const String& repeat) const
{
    Jump* jp = nullptr;
    if (repeat == u"daCapo") {
        jp = Factory::createJump(m_score->dummy()->measure());
        jp->setJumpType(JumpType::DC);
    } else if (repeat == u"daCapoAlCoda") {
        jp = Factory::createJump(m_score->dummy()->measure());
        jp->setJumpType(JumpType::DC_AL_CODA);
        jp->setPlayUntil(jp->playUntil() + m_codaId);
        jp->setContinueAt(jp->continueAt() + m_codaId);
    } else if (repeat == u"daCapoAlFine") {
        jp = Factory::createJump(m_score->dummy()->measure());
        jp->setJumpType(JumpType::DC_AL_FINE);
    } else if (repeat == u"dalSegno") {
        jp = Factory::createJump(m_score->dummy()->measure());
        jp->setJumpType(JumpType::DS);
        jp->setJumpTo(jp->jumpTo() + m_segnoId);
    } else if (repeat == u"dalSegnoAlCoda") {
        jp = Factory::createJump(m_score->dummy()->measure());
        jp->setJumpType(JumpType::DS_AL_CODA);
        jp->setJumpTo(jp->jumpTo() + m_segnoId);
        jp->setPlayUntil(jp->playUntil() + m_codaId);
        jp->setContinueAt(jp->continueAt() + m_codaId);
    } else if (repeat == u"dalSegnoAlFine") {
        jp = Factory::createJump(m_score->dummy()->measure());
        jp->setJumpType(JumpType::DS_AL_FINE);
        jp->setJumpTo(jp->jumpTo() + m_segnoId);
    }
    return jp;
}

//---------------------------------------------------------
//   handleNmiCmi
//    Dolet strangely exports N.C. chord symbols as the
//    text direction "NmiCmi".
//---------------------------------------------------------

void MusicXmlParserDirection::handleNmiCmi(Measure* measure, const Fraction& tick,
                                           DelayedDirectionsList& delayedDirections)
{
    if (!configuration()->inferTextType()) {
        return;
    }
    if (!m_wordsText.contains(u"NmiCmi")) {
        return;
    }
    Harmony* ha = new Harmony(m_score->dummy()->segment());
    HarmonyInfo* info = new HarmonyInfo(m_score);
    info->setRootTpc(Tpc::TPC_INVALID);
    info->setId(-1);
    info->setTextName(u"N.C.");
    ha->setTrack(m_track);
    ha->addChord(info);
    MusicXmlDelayedDirectionElement* delayedDirection = new MusicXmlDelayedDirectionElement(totalY(), ha, m_track, u"above", measure, tick);
    delayedDirections.push_back(delayedDirection);
    m_wordsText.replace(u"NmiCmi", u"N.C.");
}

void MusicXmlParserDirection::handleChordSym(const Fraction& tick, HarmonyMap& harmonyMap)
{
    if (!configuration()->inferTextType() || placement() == "below") {
        return;
    }

    static const std::wregex re(L"^([abcdefg])(([#b♯♭])\3?)?(maj|min|m)?[769]?((add[#b♯♭]?(9|11|))|(sus[24]?))?(\\(.*\\))?$");
    String plainWords = m_wordsText.simplified().toLower();
    if (!plainWords.contains(re)) {
        return;
    }

    Harmony* ha = Factory::createHarmony(m_score->dummy()->segment());
    ha->setHarmony(m_wordsText);
    ha->setTrack(m_track);
    ha->setPlacement(placement() == u"above" ? PlacementV::ABOVE : PlacementV::BELOW);
    ha->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    ha->resetProperty(Pid::OFFSET);
    ha->setVisible(m_visible);
    const HarmonyInfo* info = ha->chords().empty() ? nullptr : ha->chords().front();
    const ChordDescription* desc = info ? info->descr() : nullptr;
    HarmonyDesc newHarmonyDesc(m_track, ha, nullptr);

    const int ticks = tick.ticks();
    bool insert = true;
    for (auto itr = harmonyMap.begin(); itr != harmonyMap.end(); itr++) {
        if (itr->first != ticks) {
            continue;
        }
        HarmonyDesc& foundHarmonyDesc = itr->second;

        // Don't insert if there is a matching chord symbol
        // This symbol doesn't have a fret diagram, so no need to check that here
        if (track2staff(foundHarmonyDesc.m_track) == track2staff(m_track) && foundHarmonyDesc.descr() == desc) {
            insert = false;
        }
    }

    if (insert) {
        harmonyMap.insert(std::pair<int, HarmonyDesc>(ticks, newHarmonyDesc));
    }
    m_wordsText.clear();
}

void MusicXmlParserDirection::handleTempo(String& wordsString)
{
    if (!configuration()->inferTextType()) {
        return;
    }

    // Map Sibelius' representation of note types to their SMuFL counterparts and duration types
    static const MetronomeTextMap sibeliusSyms = {
        { u"y", { u"<sym>metNote32ndUp</sym>", DurationType::V_32ND } },
        { u"x", { u"<sym>metNote16thUp</sym>", DurationType::V_16TH } },
        { u"e", { u"<sym>metNote8thUp</sym>", DurationType::V_EIGHTH } },
        { u"q", { u"<sym>metNoteQuarterUp</sym>", DurationType::V_QUARTER } },
        { u"h", { u"<sym>metNoteHalfUp</sym>", DurationType::V_HALF } },
        { u"w", { u"<sym>metNoteWhole</sym>", DurationType::V_WHOLE } },
        { u"V", { u"<sym>metNoteDoubleWholeSquare</sym>", DurationType::V_BREVE } },
        { u"W", { u"<sym>metNoteDoubleWhole</sym>", DurationType::V_BREVE } }
    };

    static const MetronomeTextMap metTimesSyms = {
        { u"Œ", { u"<sym>metNoteQuarterUp</sym>", DurationType::V_QUARTER } },
        { u"Ó", { u"<sym>metNoteHalfUp</sym>", DurationType::V_HALF } },
    };

    MetronomeTextMap textMap;
    if (m_pass1.sibOrDolet()) {
        textMap = sibeliusSyms;
    } else if (m_fontFamily == u"MetTimes Plain") {
        textMap = metTimesSyms;
    }
    if (textMap.empty()) {
        return;
    }
    String substitutions;
    for (auto& entry : textMap) {
        substitutions.append(entry.first);
    }

    // Pick up any tempo markings which may have been exported as <words>
    // eg. andante (q = c. 90)
    // Sibelius uses a symbol font with the characters 'yxeqhVwW' each drawn as a different duration
    // which we need to map to SMuFL syms
    String plainWords = MScoreTextToMusicXml::toPlainText(wordsString.simplified());

    static const std::wregex tempo(L".*([" + substitutions.toStdWString() + L"])(\\.?)(?:\\s|\\u00A0)*=[^0-9]*([0-9]+).*");
    StringList tempoMatches = plainWords.search(tempo, { 1, 2, 3 }, SplitBehavior::SkipEmptyParts);

    // Not a tempo
    if (tempoMatches.size() < 2) {
        return;
    }

    const String dur = tempoMatches.at(0);
    const bool dot = tempoMatches.size() == 3;
    const String val = tempoMatches.at(dot ? 2 : 1);

    const String dotStr = dot ? u"<sym>space</sym><sym>metAugmentationDot</sym>" : u"";

    static const std::wregex replace(L"(.*)[" + substitutions.toStdWString() + L"]\\.?((?:\\s|\\u00A0)*=[^0-9]*[0-9]+.*)");
    const String newStr = u"$1" + textMap.at(dur).first + dotStr + u"$2";
    plainWords.replace(replace, newStr);
    wordsString = plainWords;

    if (!val.empty() && !dur.empty()) {
        bool ok;
        double d = val.toDouble(&ok);
        TDuration duration = TDuration(textMap.at(dur).second);
        duration.setDots(dot);

        if (ok && duration.isValid()) {
            // convert fraction to beats per minute
            m_tpoMetro = 4 * duration.fraction().numerator() * d / duration.fraction().denominator();
        }
    }
}

bool MusicXmlParserDirection::isLikelySticking()
{
    if (!configuration()->inferTextType()) {
        return false;
    }

    String plainWords = MScoreTextToMusicXml::toPlainText(m_wordsText.simplified());
    static const std::wregex sticking(L"^[lrbLRB]+$");
    return plainWords.contains(sticking)
           && m_rehearsalText.empty()
           && m_metroText.empty()
           && m_tpoSound < 0.1;
}

bool MusicXmlParserDirection::isLikelyDynamicRange() const
{
    if (!configuration()->inferTextType()) {
        return false;
    }

    String rawWordsText = m_wordsText.simplified();
    static const std::regex re("(<.*?>)");
    rawWordsText.remove(re);

    // If there are two dynamics and a hyphen in a single direction node, this is most likely something like "mp-f"
    return (rawWordsText.simplified() == u"-" || rawWordsText.simplified() == u"—") && m_dynamicsList.size() == 2;
}

PlayingTechniqueType MusicXmlParserDirection::getPlayingTechnique() const
{
    if (!configuration()->inferTextType()) {
        return PlayingTechniqueType::Undefined;
    }

    const String plainWords = MScoreTextToMusicXml::toPlainText(m_wordsText.simplified());
    static const std::unordered_map<muse::String, PlayingTechniqueType> textToPlayTechniqueType {
        { u"natural", PlayingTechniqueType::Natural },
        { u"normal", PlayingTechniqueType::Natural },
        { u"ord", PlayingTechniqueType::Natural },
        { u"arco", PlayingTechniqueType::Natural },
        { u"pizz", PlayingTechniqueType::Pizzicato },
        { u"open", PlayingTechniqueType::Open },
        { u"senza sord", PlayingTechniqueType::Open },
        { u"unmuted", PlayingTechniqueType::Open },
        { u"without mute", PlayingTechniqueType::Open },
        { u"no mute", PlayingTechniqueType::Open },
        { u"mute", PlayingTechniqueType::Mute },
        { u"con sord", PlayingTechniqueType::Mute },
        { u"sord", PlayingTechniqueType::Mute },
        { u"trem", PlayingTechniqueType::Tremolo },
        { u"detache", PlayingTechniqueType::Detache },
        { u"martele", PlayingTechniqueType::Martele },
        { u"legno", PlayingTechniqueType::ColLegno },
        { u"sul pont", PlayingTechniqueType::SulPonticello },
        { u"sul tast", PlayingTechniqueType::SulTasto },
        { u"vibrato", PlayingTechniqueType::Vibrato },
        { u"legato", PlayingTechniqueType::Legato },
        { u"distortion", PlayingTechniqueType::Distortion },
        { u"overdrive", PlayingTechniqueType::Overdrive },
        { u"harmonics", PlayingTechniqueType::Harmonics },
        { u"jazz", PlayingTechniqueType::JazzTone },
    };

    for (auto& technique : textToPlayTechniqueType) {
        if (plainWords.contains(technique.first)) {
            return technique.second;
        }
    }

    return PlayingTechniqueType::Undefined;
}

void MusicXmlParserDirection::handleDrumInstrument(bool isPerc, Fraction tick) const
{
    if (!configuration()->inferTextType() || m_wordsText.empty() || !m_rehearsalText.empty() || !m_metroText.empty() || m_tpoSound > 0.1
        || !isPerc) {
        return;
    }

    String plainWords = m_wordsText;

    static const std::regex to("to ", std::regex_constants::icase);
    static const std::regex brackets("\\(.*\\)");
    plainWords.remove(to);
    plainWords.remove(brackets);

    plainWords = MScoreTextToMusicXml::toPlainText(plainWords.simplified());

    const InstrumentTemplate* it = searchTemplateForInstrNameList({ plainWords }, true, false);

    // Ignore marching percussion, as these won't map correctly to the standard drumkit
    if (it && it->familyId() != u"batterie") {
        int pitch = it->useDrumset && it->drumset ? it->drumset->nextPitch(0) : 0;

        // Text under the staff will be for voice 2 and 4 even if it's in voice 1
        track_idx_t track = placement() == u"below" && !(m_track & 1) ? m_track + 1 : m_track;

        InferredPercInstr instr = InferredPercInstr(pitch, track, it->id, tick);
        m_pass2.addInferredPercInstr(instr);
    }
}

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/bracket node.
 */

void MusicXmlParserDirection::bracket(const String& type, const int number,
                                      std::vector<MusicXmlSpannerDesc>& starts,
                                      std::vector<MusicXmlSpannerDesc>& stops)
{
    const AsciiStringView lineEnd = m_e.asciiAttribute("line-end");
    const AsciiStringView lineType = m_e.asciiAttribute("line-type");
    const AsciiStringView endLength = m_e.asciiAttribute("end-length");
    const bool isWavy = lineType == "wavy";
    const ElementType elementType = isWavy ? ElementType::TRILL : ElementType::TEXTLINE;
    const MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ elementType, number });
    if (type == "start") {
        SLine* sline = spdesc.isStopped ? spdesc.sp : 0;
        if ((sline && sline->isTrill()) || (!sline && isWavy)) {
            if (!sline) {
                sline = Factory::createTrill(m_score->dummy());
            }
            Trill* trill = toTrill(sline);
            trill->setTrillType(TrillType::PRALLPRALL_LINE);

            if (!lineEnd.empty() && lineEnd != "none") {
                m_logger->logError(String(u"line-end not supported for line-type \"wavy\""));
            }
        } else if ((sline && sline->isTextLine()) || (!sline && !isWavy)) {
            if (!sline) {
                sline = new TextLine(m_score->dummy());
                sline->setSystemFlag(m_systemDirection);
            }
            TextLine* textLine = toTextLine(sline);
            // if (placement.empty()) placement = "above";  // TODO ? set default

            if (!endLength.empty()) {
                double length = endLength.toDouble();
                textLine->setBeginHookHeight(Spatium(lineEnd == "both" ? length / 20 : length / 10));
            }
            if (lineEnd == "up") {
                textLine->setBeginHookType(HookType::HOOK_90);
                textLine->setBeginHookHeight(-1 * textLine->beginHookHeight());
            } else if (lineEnd == "down") {
                textLine->setBeginHookType(HookType::HOOK_90);
            } else if (lineEnd == "both") {
                textLine->setBeginHookType(HookType::HOOK_90T);
            } else if (lineEnd == "arrow") {
                m_logger->logError(String(u"line-end \"arrow\" not supported"));
            } else if (lineEnd == "none") {
                textLine->setBeginHookType(HookType::NONE);
            }

            // hack: combine with a previous words element
            if (!m_wordsText.empty()) {
                // TextLine supports only limited formatting, remove all (compatible with 1.3)
                textLine->setBeginText(MScoreTextToMusicXml::toPlainText(m_wordsText));
                m_wordsText.clear();
            }

            if (lineType == "solid") {
                textLine->setLineStyle(LineType::SOLID);
                textLine->setPropertyFlags(Pid::LINE_STYLE, PropertyFlags::UNSTYLED);
            } else if (lineType == "dashed") {
                textLine->setLineStyle(LineType::DASHED);
                textLine->setPropertyFlags(Pid::LINE_STYLE, PropertyFlags::UNSTYLED);
            } else if (lineType == "dotted") {
                textLine->setLineStyle(LineType::DOTTED);
                textLine->setPropertyFlags(Pid::LINE_STYLE, PropertyFlags::UNSTYLED);
            } else if (lineType != "wavy") {
                m_logger->logError(String(u"unsupported line-type: %1").arg(String::fromAscii(lineType.ascii())), &m_e);
            }
            const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());
            if (color.isValid()) {
                textLine->setLineColor(color);
            }
        }

        starts.push_back(MusicXmlSpannerDesc(sline, elementType, number));
    } else if (type == "stop") {
        SLine* sline = spdesc.isStarted ? spdesc.sp : 0;
        if ((sline && sline->isTrill()) || (!sline && isWavy)) {
            if (!sline) {
                sline = new Trill(m_score->dummy());
            }
            if (!lineEnd.empty() && lineEnd != "none") {
                m_logger->logError(String(u"line-end not supported for line-type \"wavy\""));
            }
        } else if ((sline && sline->isTextLine()) || (!sline && !isWavy)) {
            if (!sline) {
                sline = new TextLine(m_score->dummy());
            }
            TextLine* textLine = toTextLine(sline);

            if (!endLength.empty()) {
                double length = endLength.toDouble();
                textLine->setEndHookHeight(Spatium(lineEnd == "both" ? length / 20 : length / 10));
            }
            if (lineEnd == "up") {
                textLine->setEndHookType(HookType::HOOK_90);
                textLine->setEndHookHeight(-1 * textLine->endHookHeight());
            } else if (lineEnd == "down") {
                textLine->setEndHookType(HookType::HOOK_90);
            } else if (lineEnd == "both") {
                textLine->setEndHookType(HookType::HOOK_90T);
            } else if (lineEnd == "arrow") {
                m_logger->logError(String(u"line-end \"arrow\" not supported"));
            } else if (lineEnd == "none") {
                textLine->setEndHookType(HookType::NONE);
            }
        }

        stops.push_back(MusicXmlSpannerDesc(sline, elementType, number));
    }
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   dashes
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/dashes node.
 */

void MusicXmlParserDirection::dashes(const String& type, const int number,
                                     std::vector<MusicXmlSpannerDesc>& starts,
                                     std::vector<MusicXmlSpannerDesc>& stops)
{
    const MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ ElementType::HAIRPIN, number });
    if (type == u"start") {
        TextLineBase* b = spdesc.isStopped ? toTextLine(spdesc.sp) : Factory::createTextLine(m_score->dummy());
        // if (placement.empty()) placement = "above";  // TODO ? set default

        // hack: combine with a previous words element
        if (!m_wordsText.empty()) {
            // TextLine supports only limited formatting, remove all (compatible with 1.3)
            String simplifiedText = MScoreTextToMusicXml::toPlainText(m_wordsText).simplified();
            if (isLikelyTempoLine(m_track)) {
                b = Factory::createGradualTempoChange(m_score->dummy()->segment());
                GradualTempoChangeType gtc = getTempoChangeTypeFromString(simplifiedText);
                toGradualTempoChange(b)->setTempoChangeType(gtc);
            }
            b->setBeginText(simplifiedText);
            m_wordsText.clear();
        }

        b->setBeginHookType(HookType::NONE);
        b->setEndHookType(HookType::NONE);
        b->setLineStyle(LineType::DASHED);
        // TODO brackets and dashes now share the same storage
        // because they both use ElementType::TEXTLINE
        // use MusicXML specific type instead
        starts.push_back(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
    } else if (type == u"stop") {
        TextLine* b = spdesc.isStarted ? toTextLine(spdesc.sp) : Factory::createTextLine(m_score->dummy());
        stops.push_back(MusicXmlSpannerDesc(b, ElementType::TEXTLINE, number));
    }
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   octaveShift
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/octave-shift node.
 */

void MusicXmlParserDirection::octaveShift(const String& type, const int number,
                                          std::vector<MusicXmlSpannerDesc>& starts,
                                          std::vector<MusicXmlSpannerDesc>& stops)
{
    const MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ ElementType::OTTAVA, number });
    if (type == u"up" || type == u"down") {
        int ottavasize = m_e.intAttribute("size");
        if (!(ottavasize == 8 || ottavasize == 15)) {
            m_logger->logError(String(u"unknown octave-shift size %1").arg(ottavasize), &m_e);
        } else {
            Ottava* o = spdesc.isStopped ? toOttava(spdesc.sp) : Factory::createOttava(m_score->dummy());

            if (type == u"down") {
                m_placement = m_placement.empty() ? u"above" : m_placement;
                if (ottavasize == 8) {
                    o->setOttavaType(OttavaType::OTTAVA_8VA);
                } else if (ottavasize == 15) {
                    o->setOttavaType(OttavaType::OTTAVA_15MA);
                }
            } else if (type == u"up") {
                m_placement = m_placement.empty() ? u"below" : m_placement;
                if (ottavasize == 8) {
                    o->setOttavaType(OttavaType::OTTAVA_8VB);
                } else if (ottavasize == 15) {
                    o->setOttavaType(OttavaType::OTTAVA_15MB);
                }
            }

            const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());
            if (color.isValid()) {
                o->setLineColor(color);
            }

            starts.push_back(MusicXmlSpannerDesc(o, ElementType::OTTAVA, number));
        }
    } else if (type == u"stop") {
        Ottava* o = spdesc.isStarted ? toOttava(spdesc.sp) : Factory::createOttava(m_score->dummy());
        stops.push_back(MusicXmlSpannerDesc(o, ElementType::OTTAVA, number));
    }
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/pedal node.
 */

void MusicXmlParserDirection::pedal(const String& type, const int /* number */,
                                    std::vector<MusicXmlSpannerDesc>& starts,
                                    std::vector<MusicXmlSpannerDesc>& stops)
{
    const int number { 0 };
    AsciiStringView line = m_e.asciiAttribute("line");
    String sign = m_e.attribute("sign");
    const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());

    // We have found that many exporters omit "sign" even when one is originally present,
    // therefore we will default to "yes", even though this is technically against the spec.
    bool overrideDefaultSign = true; // TODO: set this flag based on the exporting software
    if (sign.empty()) {
        if (line != "yes" || (overrideDefaultSign && type == "start")) {
            sign = u"yes";                           // MusicXML 2.0 compatibility
        } else if (line == "yes") {
            sign = u"no";                            // MusicXML 2.0 compatibility
        }
    }
    MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ ElementType::PEDAL, number });
    if (type == u"start" || type == u"resume" || type == u"sostenuto") {
        if (spdesc.isStarted && !spdesc.isStopped) {
            // Previous pedal unterminated
            // if previous pedal was a change, create a new change instead of a new pedal start
            if (toPedal(spdesc.sp)->beginHookType() == HookType::HOOK_45) {
                Pedal* p = Factory::createPedal(m_score->dummy());
                p->setBeginHookType(HookType::HOOK_45);
                p->setEndHookType(HookType::HOOK_90);
                if (line == "yes") {
                    p->setLineVisible(true);
                } else {
                    p->setLineVisible(false);
                }
                if (sign == u"no") {
                    p->setBeginText(u"");
                    p->setContinueText(u"");
                    p->setEndText(u"");
                }
                if (color.isValid()) {
                    p->setColor(color);
                }
                starts.push_back(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
                m_e.skipCurrentElement();

                return;
            } else {
                // likely an unrecorded "discontinue", so delete the line.
                m_pass2.deleteHandledSpanner(spdesc.sp);
                spdesc.isStarted = false;
            }
        }
        Pedal* p = spdesc.isStopped ? toPedal(spdesc.sp) : Factory::createPedal(m_score->dummy());
        if (line == "yes") {
            p->setLineVisible(true);
        } else {
            p->setLineVisible(false);
        }
        if (!p->lineVisible() || sign == u"yes") {
            p->setBeginText(m_score->style().styleSt(Sid::pedalText));
            p->setContinueText(m_score->style().styleSt(Sid::pedalContinueText));
            if (type == "sostenuto") {
                p->setBeginText(u"<sym>keyboardPedalSost</sym>");
                p->setContinueText(u"<sym>keyboardPedalParensLeft</sym><sym>keyboardPedalSost</sym><sym>keyboardPedalParensRight</sym>");
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
        starts.push_back(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
    } else if (type == u"stop" || type == u"discontinue") {
        Pedal* p = spdesc.isStarted ? toPedal(spdesc.sp) : Factory::createPedal(m_score->dummy());
        if (line == "yes") {
            p->setLineVisible(true);
        } else if (line == "no") {
            p->setLineVisible(false);
        }
        if ((!p->lineVisible() || sign == u"yes") && p->endHookType() == HookType::NONE) {
            p->setEndText(u"<sym>keyboardPedalUp</sym>");
        } else {
            p->setEndHookType(type == "discontinue" ? HookType::NONE : HookType::HOOK_90);
        }
        stops.push_back(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
    } else if (type == "change") {
        // pedal change is implemented as two separate pedals
        // first stop the first one
        if (spdesc.isStarted && !spdesc.isStopped) {
            Pedal* p = toPedal(spdesc.sp);
            p->setEndHookType(HookType::HOOK_45);
            if (line == "yes") {
                p->setLineVisible(true);
            } else if (line == "no") {
                p->setLineVisible(false);
            }
            stops.push_back(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
        } else {
            m_logger->logError(String(u"\"change\" type pedal created without existing pedal"), &m_e);
        }
        // then start a new one
        Pedal* p = Factory::createPedal(m_score->dummy());
        p->setBeginHookType(HookType::HOOK_45);
        p->setEndHookType(HookType::HOOK_90);
        if (line == "yes") {
            p->setLineVisible(true);
        } else {
            p->setLineVisible(false);
        }
        if (sign == u"no") {
            p->setBeginText(u"");
            p->setContinueText(u"");
        }
        if (color.isValid()) {
            p->setColor(color);
        }
        starts.push_back(MusicXmlSpannerDesc(p, ElementType::PEDAL, number));
    } else if (type == "continue") {
        // ignore
    } else {
        LOGD("unknown pedal type %s", muPrintable(type));
    }

    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   wedge
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/wedge node.
 */

void MusicXmlParserDirection::wedge(const String& type, const int number,
                                    std::vector<MusicXmlSpannerDesc>& starts,
                                    std::vector<MusicXmlSpannerDesc>& stops)
{
    AsciiStringView niente = m_e.asciiAttribute("niente");
    const MusicXmlExtendedSpannerDesc& spdesc = m_pass2.getSpanner({ ElementType::HAIRPIN, number });
    if (type == "crescendo" || type == "diminuendo") {
        Hairpin* h = spdesc.isStopped ? toHairpin(spdesc.sp) : Factory::createHairpin(m_score->dummy()->segment());
        h->setHairpinType(type == "crescendo"
                          ? HairpinType::CRESC_HAIRPIN : HairpinType::DIM_HAIRPIN);
        if (niente == "yes") {
            h->setHairpinCircledTip(true);
        }
        const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());
        if (color.isValid()) {
            h->setLineColor(color);
        }
        AsciiStringView lineType = m_e.asciiAttribute("line-type");
        if (lineType == "dashed") {
            h->setLineStyle(LineType::DASHED);
        } else if (lineType == "dotted") {
            h->setLineStyle(LineType::DOTTED);
        }
        const String spread = m_e.attribute("spread");
        if (!spread.empty() && configuration()->importLayout()) {
            const Spatium val(spread.toDouble() / 10.0);
            h->setHairpinHeight(val);
        }
        starts.push_back(MusicXmlSpannerDesc(h, ElementType::HAIRPIN, number));
    } else if (type == "stop") {
        Hairpin* h = spdesc.isStarted ? toHairpin(spdesc.sp) : Factory::createHairpin(m_score->dummy()->segment());
        if (niente == "yes") {
            h->setHairpinCircledTip(true);
        }
        stops.push_back(MusicXmlSpannerDesc(h, ElementType::HAIRPIN, number));
    }
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   toString
//---------------------------------------------------------

String MusicXmlExtendedSpannerDesc::toString() const
{
    String spStr = sp ? String::fromStdString(sp->eid().toStdString()) : u"null";
    return String(u"sp %1 tp %2 tick2 %3 track2 %4 %5 %6")
           .arg(spStr, tick2.toString())
           .arg(static_cast<int>(track2))
           .arg(isStarted ? u"started" : u"", isStopped ? u"stopped" : u"")
    ;
}

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void MusicXmlParserPass2::addSpanner(const MusicXmlSpannerDesc& d)
{
    MusicXmlExtendedSpannerDesc& spdesc = getSpanner(d);
    spdesc.sp = d.sp;
}

//---------------------------------------------------------
//   getSpanner
//---------------------------------------------------------

MusicXmlExtendedSpannerDesc& MusicXmlParserPass2::getSpanner(const MusicXmlSpannerDesc& d)
{
    if (d.tp == ElementType::HAIRPIN && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL) {
        return m_hairpins[d.nr];
    } else if (d.tp == ElementType::OTTAVA && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL) {
        return m_ottavas[d.nr];
    } else if (d.tp == ElementType::PEDAL && 0 == d.nr) {
        return m_pedal;
    } else if ((d.tp == ElementType::TEXTLINE || d.tp == ElementType::TRILL) && 0 <= d.nr && d.nr < MAX_NUMBER_LEVEL) {
        return m_brackets[d.nr];
    }
    m_logger->logError(String(u"invalid number %1").arg(d.nr + 1), &m_e);
    return m_dummyNewMusicXmlSpannerDesc;
}

//---------------------------------------------------------
//   clearSpanner
//---------------------------------------------------------

void MusicXmlParserPass2::clearSpanner(const MusicXmlSpannerDesc& d)
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

void MusicXmlParserPass2::deleteHandledSpanner(SLine* const& spanner)
{
    muse::remove(m_spanners, spanner);
    delete spanner;
}

InferredPercInstr MusicXmlParserPass2::inferredPercInstr(const Fraction& tick, const track_idx_t trackIdx)
{
    InferredPercInstr instr = InferredPercInstr();

    for (InferredPercList::iterator iter = m_inferredPerc.begin(); iter != m_inferredPerc.end();) {
        if (iter->tick == tick && iter->track == trackIdx) {
            instr = *iter;
            iter = m_inferredPerc.erase(iter);
            break;
        } else {
            ++iter;
        }
    }

    return instr;
}

//---------------------------------------------------------
//   metronome
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type/metronome node.
 Convert to text and set r to calculated tempo.
 */

String MusicXmlParserDirection::metronome(double& r)
{
    r = 0;
    String tempoText;
    String perMinute;
    bool parenth = m_e.asciiAttribute("parentheses") == "yes";

    if (parenth) {
        tempoText += u"(";
    }

    TDuration dur1;
    TDuration dur2;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "metronome-note" || m_e.name() == "metronome-relation") {
            skipLogCurrElem();
            continue;
        }
        String txt = m_e.readText();
        if (m_e.name() == "beat-unit") {
            // set first dur that is still invalid
            ByteArray ba = txt.toAscii();
            if (!dur1.isValid()) {
                dur1.setType(TConv::fromXml(ba.constChar(), DurationType::V_INVALID));
            } else if (!dur2.isValid()) {
                dur2.setType(TConv::fromXml(ba.constChar(), DurationType::V_INVALID));
            }
        } else if (m_e.name() == "beat-unit-dot") {
            if (dur2.isValid()) {
                dur2.setDots(1);
            } else if (dur1.isValid()) {
                dur1.setDots(1);
            }
        } else if (m_e.name() == "per-minute") {
            perMinute = txt;
        } else {
            skipLogCurrElem();
        }
    }

    if (dur1.isValid()) {
        tempoText += TempoText::duration2tempoTextString(dur1);
    }
    if (dur2.isValid()) {
        tempoText += u" = ";
        tempoText += TempoText::duration2tempoTextString(dur2);
    } else if (!perMinute.empty()) {
        tempoText += u" = ";
        tempoText += perMinute;
    }
    if (dur1.isValid() && !dur2.isValid() && !perMinute.empty()) {
        bool ok;
        double d = perMinute.toDouble(&ok);
        if (ok) {
            // convert fraction to beats per minute
            r = 4 * dur1.fraction().numerator() * d / dur1.fraction().denominator();
        }
    }

    if (parenth) {
        tempoText += u")";
    }

    return tempoText;
}

//---------------------------------------------------------
//   determineBarLineType
//---------------------------------------------------------

static bool determineBarLineType(const String& barStyle, const String& repeat,
                                 BarLineType& type, bool& visible)
{
    // set defaults
    type = BarLineType::NORMAL;
    visible = true;

    if (barStyle == u"light-heavy" && repeat == u"backward") {
        type = BarLineType::END_REPEAT;
    } else if (barStyle == u"heavy-light" && repeat == u"forward") {
        type = BarLineType::START_REPEAT;
    } else if (barStyle == u"light-heavy" && repeat.empty()) {
        type = BarLineType::END;
    } else if (barStyle == u"heavy-light" && repeat.empty()) {
        type = BarLineType::REVERSE_END;
    } else if (barStyle == u"regular") {
        type = BarLineType::NORMAL;
    } else if (barStyle == u"dashed") {
        type = BarLineType::BROKEN;
    } else if (barStyle == u"dotted") {
        type = BarLineType::DOTTED;
    } else if (barStyle == u"light-light") {
        type = BarLineType::DOUBLE;
        /*
        } else if (barStyle == "heavy-light") {
         ;
         */
    } else if (barStyle == u"heavy-heavy") {
        type = BarLineType::DOUBLE_HEAVY;
    } else if (barStyle == u"heavy") {
        type = BarLineType::HEAVY;
    } else if (barStyle == u"none") {
        visible = false;
    } else if (barStyle.empty()) {
        if (repeat == u"backward") {
            type = BarLineType::END_REPEAT;
        } else if (repeat == u"forward") {
            type = BarLineType::START_REPEAT;
        } else {
            return false;
        }
    } else if ((barStyle == u"tick") || (barStyle == u"short")) {
        // handled later (as normal barline with different parameters)
    } else {
        LOGD("unsupported bar type <%s>", muPrintable(barStyle));           // TODO
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

void MusicXmlParserPass2::barline(const String& partId, Measure* measure, const Fraction& tick)
{
    String loc = m_e.attribute("location");
    if (loc.empty()) {
        loc = u"right";
    }
    // Place barline in correct place
    Fraction locTick = tick;
    if (loc == u"left") {
        locTick = measure->tick();
    } else if (loc == u"right") {
        locTick = measure->endTick();
    }

    String barStyle;
    Color barlineColor;
    String endingNumber;
    String endingType;
    Color endingColor;
    String endingText;
    String repeat;
    String count;
    bool printEnding = true;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "bar-style") {
            barlineColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            barStyle = m_e.readText();
        } else if (m_e.name() == "ending") {
            endingNumber = m_e.attribute("number");
            endingType   = m_e.attribute("type");
            endingColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            printEnding = m_e.asciiAttribute("print-object") != "no";
            endingText = m_e.readText();
        } else if (m_e.name() == "fermata") {
            const Color fermataColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            const String fermataType = m_e.attribute("type");
            Segment* const segment = measure->getSegment(SegmentType::EndBarLine, locTick);
            const track_idx_t track = m_pass1.trackForPart(partId);
            Fermata* fermata = Factory::createFermata(segment);
            fermata->setSymId(convertFermataToSymId(m_e.readText()));
            fermata->setTrack(track);
            segment->add(fermata);
            if (fermataColor.isValid()) {
                fermata->setColor(fermataColor);
            }
            if (fermataType == u"inverted") {
                fermata->setPlacement(PlacementV::BELOW);
                fermata->resetProperty(Pid::OFFSET);
            } else if (fermataType.empty()) {
                fermata->setPlacement(fermata->propertyDefault(Pid::PLACEMENT).value<PlacementV>());
            }

            // Terminate tempo lines
            const InferredTempoLineStack& lines = getInferredTempoLine();
            terminateInferredLine(std::vector<TextLineBase*>(lines.begin(), lines.end()), locTick, track);
        } else if (m_e.name() == "repeat") {
            repeat = m_e.attribute("direction");
            count = m_e.attribute("times");
            if (count.empty()) {
                count = u"2";
            }
            measure->setRepeatCount(count.toInt());
            m_e.skipCurrentElement();
        } else {
            skipLogCurrElem();
        }
    }

    BarLineType type = BarLineType::NORMAL;
    bool visible = true;
    if (determineBarLineType(barStyle, repeat, type, visible)) {
        const track_idx_t track = m_pass1.trackForPart(partId);
        if (type & BarLineType::START_REPEAT) {
            // combine start_repeat flag with current state initialized during measure parsing
            measure->setRepeatStart(true);
        } else if (type & BarLineType::END_REPEAT) {
            // combine end_repeat flag with current state initialized during measure parsing
            measure->setRepeatEnd(true);
        } else {
            if (barStyle != "regular" || barlineColor.isValid() || loc == "middle") {
                // Add barline to the first voice of every staff in the part,
                // and span every barline except the last
                const Part* part = m_pass1.getPart(partId);
                staff_idx_t nstaves = part->nstaves();
                for (staff_idx_t i = 0; i < nstaves; ++i) {
                    const Staff* staff = part->staff(i);
                    bool spanStaff = nstaves > 1 ? i < nstaves - 1 : staff->barLineSpan();
                    track_idx_t currentTrack = track + (i * VOICES);
                    auto b = createBarline(measure->score(), currentTrack, type, visible, barStyle, spanStaff);
                    if (barlineColor.isValid()) {
                        b->setColor(barlineColor);
                    }
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

void MusicXmlParserPass2::doEnding(const String& partId, Measure* measure, const String& number,
                                   const String& type, const Color color,
                                   const String& text, const bool print)
{
    if (!(number.empty() && type.empty())) {
        if (number.empty()) {
            m_logger->logError(u"empty ending number", &m_e);
        } else if (type.empty()) {
            m_logger->logError(u"empty ending type", &m_e);
        } else {
            StringList sl = number.split(u',', SkipEmptyParts);
            std::vector<int> iEndingNumbers;
            bool unsupported = false;
            for (const String& s : sl) {
                int iEndingNumber = s.toInt();
                if (iEndingNumber <= 0) {
                    unsupported = true;
                    break;
                }
                iEndingNumbers.push_back(iEndingNumber);
            }

            if (unsupported) {
                m_logger->logError(String(u"unsupported ending number '%1'").arg(number), &m_e);
            } else {
                // Ignore if it is hidden and redundant
                Volta* redundantVolta = findRedundantVolta(m_pass1.trackForPart(partId), measure);
                if (!print && redundantVolta) {
                    m_logger->logDebugInfo(u"Ignoring redundant hidden Volta", &m_e);
                } else if (type == u"start") {
                    Volta* volta = Factory::createVolta(m_score->dummy());
                    volta->setTrack(m_pass1.trackForPart(partId));
                    volta->setText(text.empty() ? number : text);
                    // LVIFIX TODO also support endings "1 - 3"
                    volta->endings().clear();
                    muse::join(volta->endings(), iEndingNumbers);
                    volta->setTick(measure->tick());
                    m_score->addElement(volta);
                    m_lastVolta = volta;
                    volta->setVisible(print);
                    if (color.isValid()) {
                        volta->setLineColor(color);
                    }
                } else if (type == u"stop") {
                    if (m_lastVolta) {
                        m_lastVolta->setVoltaType(Volta::Type::CLOSED);
                        m_lastVolta->setTick2(measure->tick() + measure->ticks());
                        // Assume print-object was handled at the start
                        m_lastVolta = 0;
                    } else if (!redundantVolta) {
                        m_logger->logError(u"ending stop without start", &m_e);
                    }
                } else if (type == u"discontinue") {
                    if (m_lastVolta) {
                        m_lastVolta->setVoltaType(Volta::Type::OPEN);
                        m_lastVolta->setTick2(measure->tick() + measure->ticks());
                        // Assume print-object was handled at the start
                        m_lastVolta = 0;
                    } else if (!redundantVolta) {
                        m_logger->logError(u"ending discontinue without start", &m_e);
                    }
                } else {
                    m_logger->logError(String(u"unsupported ending type '%1'").arg(type), &m_e);
                }

                // Delete any hidden redundant voltas before
                while (redundantVolta && !redundantVolta->visible()) {
                    m_score->removeElement(redundantVolta);
                    delete redundantVolta;
                    redundantVolta = findRedundantVolta(m_pass1.trackForPart(partId), measure);
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

static void addSymToSig(KeySigEvent& sig, const String& step, const String& alter,
                        const String& accid, const String& smufl)
{
    //LOGD("addSymToSig(step '%s' alt '%s' acc '%s')",
    //       muPrintable(step), muPrintable(alter), muPrintable(accid));

    SymId id = musicXmlString2accSymId(accid, smufl);

    if (id == SymId::noSym) {
        bool ok;
        double d;
        d = alter.toDouble(&ok);
        AccidentalType accTpAlter = ok ? microtonalGuess(d) : AccidentalType::NONE;
        String s = accidentalType2MusicXmlString(accTpAlter);
        if (s == u"other") {
            s = accidentalType2SmuflMusicXmlString(accTpAlter);
        }
        id = musicXmlString2accSymId(s);
    }

    if (step.size() == 1 && id != SymId::noSym) {
        const String table = u"CDEFGAB";
        if (table.contains(step)) {
            CustDef cd;
            cd.degree = int(table.indexOf(step));
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

static void addKey(const KeySigEvent key, const Color keyColor, const bool printObj, Score* score,
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

static void flushAlteredTone(KeySigEvent& kse, String& step, String& alt, String& acc,
                             String& smufl)
{
    //LOGD("flushAlteredTone(step '%s' alt '%s' acc '%s')",
    //       muPrintable(step), muPrintable(alt), muPrintable(acc));

    if (step.empty() && alt.empty() && acc.empty()) {
        return;      // nothing to do
    }
    // step and alt are required, but also accept step and acc
    if (!step.empty() && (!alt.empty() || !acc.empty())) {
        addSymToSig(kse, step, alt, acc, smufl);
    } else {
        LOGD("flushAlteredTone invalid combination of step '%s' alt '%s' acc '%s')",
             muPrintable(step), muPrintable(alt), muPrintable(acc));       // TODO
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

void MusicXmlParserPass2::key(const String& partId, Measure* measure, const Fraction& tick)
{
    String strKeyno = m_e.attribute("number");
    int keyno = -1;   // assume no number (see below)
    if (!strKeyno.empty()) {
        keyno = m_pass1.getMusicXmlPart(partId).staffNumberToIndex(strKeyno.toInt());
        if (keyno < 0) {
            // conversion error (-1), assume staff 0
            m_logger->logError(String(u"invalid key number '%1'").arg(strKeyno), &m_e);
            keyno = 0;
        }
    }
    const bool printObject = m_e.asciiAttribute("print-object") != "no";
    const Color keyColor = Color::fromString(m_e.asciiAttribute("color").ascii());

    // for custom keys, a single altered tone is described by
    // key-step (required),  key-alter (required) and key-accidental (optional)
    // none, one or more altered tone may be present
    // a simple state machine is required to detect them
    KeySigEvent key;
    String keyStep;
    String keyAlter;
    String keyAccidental;
    String smufl;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "fifths") {
            Key tKey = Key(m_e.readInt());
            Key cKey = tKey;
            Interval v = m_pass1.getPart(partId)->instrument()->transpose();
            if (!v.isZero() && !m_score->style().styleB(Sid::concertPitch)) {
                cKey = transposeKey(tKey, v);
                // if there are more than 6 accidentals in transposing key, it cannot be PreferSharpFlat::AUTO
                Part* part = m_pass1.getPart(partId);
                if ((tKey > 6 || tKey < -6) && part->preferSharpFlat() == PreferSharpFlat::AUTO) {
                    part->setPreferSharpFlat(PreferSharpFlat::NONE);
                }
            }
            key.setConcertKey(cKey);
            key.setKey(tKey);
        } else if (m_e.name() == "mode") {
            String m = m_e.readText();
            if (m == u"none") {
                key.setCustom(true);
                key.setMode(KeyMode::NONE);
            } else if (m == u"major") {
                key.setMode(KeyMode::MAJOR);
            } else if (m == u"minor") {
                key.setMode(KeyMode::MINOR);
            } else if (m == u"dorian") {
                key.setMode(KeyMode::DORIAN);
            } else if (m == u"phrygian") {
                key.setMode(KeyMode::PHRYGIAN);
            } else if (m == u"lydian") {
                key.setMode(KeyMode::LYDIAN);
            } else if (m == u"mixolydian") {
                key.setMode(KeyMode::MIXOLYDIAN);
            } else if (m == u"aeolian") {
                key.setMode(KeyMode::AEOLIAN);
            } else if (m == u"ionian") {
                key.setMode(KeyMode::IONIAN);
            } else if (m == u"locrian") {
                key.setMode(KeyMode::LOCRIAN);
            } else {
                m_logger->logError(String(u"Unsupported mode '%1'").arg(m), &m_e);
            }
        } else if (m_e.name() == "cancel") {
            skipLogCurrElem();        // TODO ??
        } else if (m_e.name() == "key-step") {
            flushAlteredTone(key, keyStep, keyAlter, keyAccidental, smufl);
            keyStep = m_e.readText();
        } else if (m_e.name() == "key-alter") {
            keyAlter = m_e.readText().trimmed();
        } else if (m_e.name() == "key-accidental") {
            smufl = m_e.attribute("smufl");
            keyAccidental = m_e.readText();
        } else {
            skipLogCurrElem();
        }
    }
    flushAlteredTone(key, keyStep, keyAlter, keyAccidental, smufl);

    size_t nstaves = m_pass1.getPart(partId)->nstaves();
    staff_idx_t staffIdx = m_pass1.trackForPart(partId) / VOICES;
    if (keyno == -1) {
        // apply key to all staves in the part
        for (staff_idx_t i = 0; i < nstaves; ++i) {
            addKey(key, keyColor, printObject, m_score, measure, staffIdx + i, tick);
        }
    } else if (keyno < static_cast<int>(nstaves)) {
        addKey(key, keyColor, printObject, m_score, measure, staffIdx + keyno, tick);
    }
}

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 */

void MusicXmlParserPass2::clef(const String& partId, Measure* measure, const Fraction& tick)
{
    ClefType clef   = ClefType::G;
    StaffTypes st = StaffTypes::STANDARD;

    String c;
    int i = 0;
    int line = -1;

    const String strClefno = m_e.attribute("number");
    const bool afterBarline = m_e.asciiAttribute("after-barline") == "yes";
    const bool printObject = m_e.asciiAttribute("print-object") != "no";
    const Color clefColor = Color::fromString(m_e.asciiAttribute("color").ascii());

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "sign") {
            c = m_e.readText();
        } else if (m_e.name() == "line") {
            line = m_e.readInt();
        } else if (m_e.name() == "clef-octave-change") {
            i = m_e.readInt();
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
        if (c == u"G") {
            line = 2;
        } else if (c == u"F") {
            line = 4;
        } else if (c == u"C") {
            line = 3;
        }
    }

    if (c == u"G" && i == 0 && line == 2) {
        clef = ClefType::G;
    } else if (c == u"G" && i == 1 && line == 2) {
        clef = ClefType::G8_VA;
    } else if (c == u"G" && i == 2 && line == 2) {
        clef = ClefType::G15_MA;
    } else if (c == u"G" && i == -1 && line == 2) {
        clef = ClefType::G8_VB;
    } else if (c == u"G" && i == -2 && line == 2) {
        clef = ClefType::G15_MB;
    } else if (c == u"G" && i == 0 && line == 1) {
        clef = ClefType::G_1;
    } else if (c == u"F" && i == 0 && line == 3) {
        clef = ClefType::F_B;
    } else if (c == u"F" && i == 0 && line == 4) {
        clef = ClefType::F;
    } else if (c == u"F" && i == 1 && line == 4) {
        clef = ClefType::F_8VA;
    } else if (c == u"F" && i == 2 && line == 4) {
        clef = ClefType::F_15MA;
    } else if (c == u"F" && i == -1 && line == 4) {
        clef = ClefType::F8_VB;
    } else if (c == u"F" && i == -2 && line == 4) {
        clef = ClefType::F15_MB;
    } else if (c == u"F" && i == 0 && line == 5) {
        clef = ClefType::F_C;
    } else if (c == u"C") {
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
    } else if (c == u"percussion") {
        clef = ClefType::PERC;
        if (m_hasDrumset) {
            st = StaffTypes::PERC_DEFAULT;
        }
    } else if (c == u"TAB") {
        clef = ClefType::TAB;
        st= StaffTypes::TAB_DEFAULT;
    } else {
        LOGD("clef: unknown clef <sign=%s line=%d oct ch=%d>", muPrintable(c), line, i);      // TODO
    }

    Part* part = m_pass1.getPart(partId);
    IF_ASSERT_FAILED(part) {
        return;
    }

    // TODO: check error handling for
    // - single staff
    // - multi-staff with same clef
    int clefno = 0;   // default
    if (!strClefno.empty()) {
        clefno = m_pass1.getMusicXmlPart(partId).staffNumberToIndex(strClefno.toInt());
    }
    if (clefno < 0 || clefno >= int(part->nstaves())) {
        // conversion error (0) or other issue, assume staff 1
        // Also for Cubase 6.5.5 which generates clef number="2" in a single staff part
        // Same fix is required in pass 1 and pass 2
        m_logger->logError(String(u"invalid clef number '%1'").arg(strClefno), &m_e);
        clefno = 0;
    }

    Segment* s;
    ClefToBarlinePosition position = ClefToBarlinePosition::AUTO;
    // check if the clef change needs to be in the previous measure
    if (!afterBarline && (tick == measure->tick()) && measure->prevMeasure()) {
        s = measure->prevMeasure()->getSegment(SegmentType::Clef, tick);
        if (measure->prevMeasure()->repeatEnd()) {
            position = ClefToBarlinePosition::BEFORE;
        }
    } else {
        s = measure->getSegment(tick.isNotZero() ? SegmentType::Clef : SegmentType::HeaderClef, tick);
        if (measure->prevMeasure() && !measure->prevMeasure()->repeatEnd()) {
            position = ClefToBarlinePosition::AFTER;
        }
    }

    Clef* clefs = Factory::createClef(s);
    clefs->setClefType(clef);
    clefs->setVisible(printObject);
    clefs->setClefToBarlinePosition(position);
    if (clefColor.isValid()) {
        clefs->setColor(clefColor);
    }
    track_idx_t track = m_pass1.trackForPart(partId) + clefno * VOICES;
    clefs->setTrack(track);
    s->add(clefs);

    // set the correct staff type
    // note that clef handling should probably done in pass1
    staff_idx_t staffIdx = m_score->staffIdx(part) + clefno;
    int lines = m_score->staff(staffIdx)->lines(Fraction(0, 1));
    if (tick.isZero()) {   // changing staff type not supported (yet ?)
        m_score->staff(staffIdx)->setStaffType(tick, *StaffType::preset(st));
        m_score->staff(staffIdx)->setLines(tick, lines);     // preserve previously set staff lines
        m_score->staff(staffIdx)->setBarLineTo(0);        // default
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

static bool determineTimeSig(const String& beats, const String& beatType, const String& timeSymbol,
                             TimeSigType& st, int& bts, int& btp)
{
    // initialize
    st  = TimeSigType::NORMAL;
    bts = 0;         // the beats (max 4 separated by "+") as integer
    btp = 0;         // beat-type as integer
    // determine if timesig is valid
    if (timeSymbol == u"cut") {
        st = TimeSigType::ALLA_BREVE;
    } else if (timeSymbol == u"common") {
        st = TimeSigType::FOUR_FOUR;
    } else if (timeSymbol == u"single-number") {
        // let pass
    } else if (!timeSymbol.empty() && timeSymbol != u"normal") {
        LOGD("determineTimeSig: time symbol <%s> not recognized", muPrintable(timeSymbol)); // TODO
        return false;
    }

    btp = beatType.toInt();
    StringList list = beats.split(u'+');
    for (size_t i = 0; i < list.size(); i++) {
        bts += list.at(i).toInt();
    }

    // determine if bts and btp are valid
    if (bts <= 0 || btp <= 0) {
        LOGD("determineTimeSig: beats=%s and/or beat-type=%s not recognized",
             muPrintable(beats), muPrintable(beatType));               // TODO
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

void MusicXmlParserPass2::time(const String& partId, Measure* measure, const Fraction& tick)
{
    String beats;
    String beatType;
    String timeSymbol = m_e.attribute("symbol");
    bool printObject = m_e.asciiAttribute("print-object") != "no";
    const Color timeColor = Color::fromString(m_e.asciiAttribute("color").ascii());

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "beats") {
            beats = m_e.readText();
        } else if (m_e.name() == "beat-type") {
            beatType = m_e.readText();
        } else {
            skipLogCurrElem();
        }
    }

    if (!beats.empty() && !beatType.empty()) {
        // determine if timesig is valid
        TimeSigType st  = TimeSigType::NORMAL;
        int bts = 0;     // total beats as integer (beats may contain multiple numbers, separated by "+")
        int btp = 0;     // beat-type as integer
        if (determineTimeSig(beats, beatType, timeSymbol, st, bts, btp)) {
            m_timeSigDura = Fraction(bts, btp);
            Fraction fractionTSig = Fraction(bts, btp);
            for (size_t i = 0; i < m_pass1.getPart(partId)->nstaves(); ++i) {
                Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
                TimeSig* timesig = Factory::createTimeSig(s);
                timesig->setVisible(printObject);
                if (timeColor.isValid()) {
                    timesig->setColor(timeColor);
                }
                track_idx_t track = m_pass1.trackForPart(partId) + i * VOICES;
                timesig->setTrack(track);
                timesig->setSig(fractionTSig, st);
                // handle simple compound and single time signatures
                if (beats.contains(Char(u'+'))) {
                    timesig->setNumeratorString(beats);
                    timesig->setDenominatorString(beatType);
                } else if (timeSymbol == u"single-number") {
                    timesig->setNumeratorString(beats);
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

void MusicXmlParserPass2::divisions()
{
    m_divs = m_e.readInt();
    if (!(m_divs > 0)) {
        m_logger->logError(u"illegal divisions", &m_e);
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

static bool isWholeMeasureRest(const String& type, const Fraction restDuration, const Fraction measureDuration)
{
    if (!restDuration.isValid() || !measureDuration.isValid()) {
        return false;
    }

    return (type.empty() && restDuration == measureDuration)
           || (type == u"whole" && restDuration == measureDuration);
}

//---------------------------------------------------------
//   determineDuration
//---------------------------------------------------------

/**
 * Determine duration for a note or rest.
 * This includes whole measure rest detection.
 */

static TDuration determineDuration(const bool isRest, const bool measureRest, const String& type, const int dots,
                                   const Fraction chordRestDuration, const Fraction measureDuration)
{
    //LOGD("determineDuration %s, type <%s>, dots %d, duration %s, measure duration %s",
    //      isRest ? "rest" : "chord", muPrintable(durationType), dots, muPrintable(chordRestDuration.toString()), muPrintable(measureDuration.toString()));

    TDuration res;
    if (isRest && (measureRest || isWholeMeasureRest(type, chordRestDuration, measureDuration))) {
        res.setType(DurationType::V_MEASURE);
    } else if (type.empty()) {
        // If no type, set duration type based on duration.
        // Note that sometimes unusual duration (e.g. 261/256) are found.
        res.setVal(chordRestDuration.ticks());
    } else {
        ByteArray ba = type.toAscii();
        res.setType(TConv::fromXml(ba.constChar(), DurationType::V_INVALID));
        res.setDots(dots);
        if (res.type() == DurationType::V_INVALID) {
            res.setType(DurationType::V_QUARTER);        // default, TODO: use measureDuration ?
        }
    }

    //LOGD("-> duration %hhd, dots %d, ticks %s",
    //      res.type(), res.dots(), muPrintable(chordRestDuration.toString()));

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
    //       tick, track, duration.ticks(), muPrintable(dura.print()), bm);
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

static void handleDisplayStep(ChordRest* cr, int step, int octave, const Fraction& tick, double spatium)
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

static void setNoteHead(Note* note, const Color noteheadColor, const bool noteheadParentheses, const String& noteheadFilled)
{
    Score* const score = note->score();

    if (noteheadColor.isValid()) {
        note->setColor(noteheadColor);
    }
    if (noteheadParentheses) {
        Symbol* s = new Symbol(note);
        s->setSym(SymId::noteheadParenthesisLeft);
        s->setParent(note);
        score->addElement(s);
        s = new Symbol(note);
        s->setSym(SymId::noteheadParenthesisRight);
        s->setParent(note);
        score->addElement(s);
    }

    if (noteheadFilled == u"no") {
        note->setHeadType(NoteHeadType::HEAD_HALF);
    } else if (noteheadFilled == u"yes") {
        note->setHeadType(NoteHeadType::HEAD_QUARTER);
    }
}

//---------------------------------------------------------
//   computeBeamMode
//---------------------------------------------------------

/**
 Calculate the beam mode based on the collected beamTypes.
 */

static BeamMode computeBeamMode(const std::map<int, String>& beamTypes)
{
    // Start with uniquely-handled beam modes
    if (muse::value(beamTypes, 1) == u"continue"
        && muse::value(beamTypes, 2) == u"begin") {
        return BeamMode::BEGIN16;
    } else if (muse::value(beamTypes, 1) == u"continue"
               && muse::value(beamTypes, 2) == u"continue"
               && muse::value(beamTypes, 3) == u"begin") {
        return BeamMode::BEGIN32;
    }
    // Generic beam modes are naive to all except the first beam
    else if (muse::value(beamTypes, 1) == u"begin") {
        return BeamMode::BEGIN;
    } else if (muse::value(beamTypes, 1) == u"continue") {
        return BeamMode::MID;
    } else if (muse::value(beamTypes, 1) == u"end") {
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
    if (!fbl.empty()) {
        Fraction sTick = noteStartTime;                  // starting tick
        for (FiguredBass* fb : fbl) {
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

static void addTremolo(ChordRest* cr, const int tremoloNr, const String& tremoloType, const String& smufl, const Color& color,
                       Chord*& tremStart, MusicXmlLogger* logger, const XmlStreamReader* const xmlreader, Fraction& timeMod)
{
    if (!cr->isChord()) {
        return;
    }
    if (tremoloNr) {
        //LOGD("tremolo %d type '%s' ticks %d tremStart %p", tremoloNr, muPrintable(tremoloType), ticks, _tremStart);
        if (tremoloNr == 1 || tremoloNr == 2 || tremoloNr == 3 || tremoloNr == 4) {
            if (tremoloType.empty() || tremoloType == u"single") {
                TremoloType type = TremoloType::INVALID_TREMOLO;
                switch (tremoloNr) {
                case 1: type = TremoloType::R8;
                    break;
                case 2: type = TremoloType::R16;
                    break;
                case 3: type = TremoloType::R32;
                    break;
                case 4: type = TremoloType::R64;
                    break;
                }

                if (type != TremoloType::INVALID_TREMOLO) {
                    TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(mu::engraving::toChord(cr));
                    tremolo->setTremoloType(type);
                    if (color.isValid()) {
                        tremolo->setColor(color);
                    }
                    cr->add(tremolo);
                }
            } else if (tremoloType == u"start") {
                if (tremStart) {
                    logger->logError(u"MusicXml::import: double tremolo start", xmlreader);
                }
                tremStart = static_cast<Chord*>(cr);
                // timeMod takes into account also the factor 2 of a two-note tremolo
                if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                    timeMod.setDenominator(timeMod.denominator() / 2);
                }
            } else if (tremoloType == u"stop") {
                if (tremStart) {
                    TremoloType type = TremoloType::INVALID_TREMOLO;
                    switch (tremoloNr) {
                    case 1: type = TremoloType::C8;
                        break;
                    case 2: type = TremoloType::C16;
                        break;
                    case 3: type = TremoloType::C32;
                        break;
                    case 4: type = TremoloType::C64;
                        break;
                    }

                    if (type != TremoloType::INVALID_TREMOLO) {
                        TremoloTwoChord* tremolo = Factory::createTremoloTwoChord(mu::engraving::toChord(cr));
                        tremolo->setTremoloType(type);
                        if (color.isValid()) {
                            tremolo->setColor(color);
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
                    }
                    // timeMod takes into account also the factor 2 of a two-note tremolo
                    if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                        timeMod.setDenominator(timeMod.denominator() / 2);
                    }
                } else {
                    logger->logError(u"MusicXml::import: double tremolo stop w/o start", xmlreader);
                }
                tremStart = nullptr;
            }
        } else {
            logger->logError(String(u"unknown tremolo type %1").arg(tremoloNr), xmlreader);
        }
    } else if (tremoloNr == 0 && (tremoloType == u"unmeasured" || tremoloType.empty() || smufl == u"buzzRoll")) {
        // Out of all the SMuFL unmeasured tremolos, we only support 'buzzRoll'
        TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(mu::engraving::toChord(cr));
        tremolo->setTremoloType(TremoloType::BUZZ_ROLL);
        if (color.isValid()) {
            tremolo->setColor(color);
        }
        cr->add(tremolo);
    } else if (!smufl.empty() && smufl != u"buzzRoll") {
        logger->logError(String(u"MusicXml::import: only buzzRoll glyph is supported for unmeasured tremolos"), xmlreader);
    }
}

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

// TODO: refactor: optimize parameters

static void setPitch(Note* note, const MusicXmlInstruments& instruments, const String& instrumentId, const MusicXmlNotePitch& mnp,
                     const int octaveShift, const Instrument* const instrument)
{
    if (mnp.unpitched()) {
        if (hasDrumset(instruments) && muse::contains(instruments, instrumentId)) {
            // step and oct are display-step and ...-oct
            // get pitch from instrument definition in drumset instead
            int unpitched = instruments.at(instrumentId).unpitched;
            note->setPitch(std::clamp(unpitched, 0, 127));
            note->setTpcFromPitch();
        } else {
            //LOGD("disp step %d oct %d", displayStep, displayOctave);
            xmlSetPitch(note, mnp.displayStep(), 0, 0.0, mnp.displayOctave(), 0, instrument);
        }
    } else {
        xmlSetPitch(note, mnp.step(), mnp.alter(), mnp.tuning(), mnp.octave(), octaveShift, instrument);
    }
}

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

// set drumset information
// note that in MuseScore, the drumset contains defaults for notehead,
// line and stem direction, while a MusicXML file contains actuals.
// the MusicXML values for each note are simply copied to the defaults

static void setDrumset(Chord* c, MusicXmlParserPass1& pass1, const String& partId, const String& instrumentId,
                       const Fraction& noteStartTime, const MusicXmlNotePitch& mnp, const DirectionV stemDir, const NoteHeadGroup headGroup)
{
    // determine staff line based on display-step / -octave and clef type
    const ClefType clef = c->staff()->clef(noteStartTime);
    const int po = ClefInfo::pitchOffset(clef);
    const int pitch = MusicXmlStepAltOct2Pitch(mnp.displayStep(), 0, mnp.displayOctave());
    int line = po - absStep(pitch);

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

void MusicXmlParserPass2::xmlSetDrumsetPitch(Note* note, const Chord* chord, const Staff* staff, int step, int octave,
                                             NoteHeadGroup headGroup, DirectionV& stemDir, Instrument* instrument)
{
    Drumset* ds = instrument->drumset();
    // get line
    // determine staff line based on display-step / -octave and clef type
    const ClefType clef = staff->clef(chord->tick());
    const int po = ClefInfo::pitchOffset(clef);
    const int pitch = MusicXmlStepAltOct2Pitch(step, 0, octave);
    int line = po - absStep(pitch);
    const int staffLines = staff->lines(chord->tick());
    if (staffLines == 1) {
        line -= 8;
    }
    if (staffLines == 3) {
        line -= 2;
    }

    const int firstDrum = ds->nextPitch(0);
    int curDrum = firstDrum;
    int newPitch = pitch;
    bool matchFound = false;
    do {
        if (ds->line(curDrum) == line) {
            if (ds->noteHead(curDrum) == headGroup) {
                newPitch = curDrum;
                matchFound = true;
                break;
            }
        }
        curDrum = ds->nextPitch(curDrum);
    } while (curDrum != firstDrum);

    // Find inferred instruments at this tick
    if (configuration()->inferTextType()) {
        InferredPercInstr instr = inferredPercInstr(chord->tick(), chord->track());
        if (instr.track != muse::nidx) {
            // Clear old instrument
            ds->drum(newPitch) = DrumInstrument();

            newPitch = instr.pitch;
            ds->drum(newPitch) = ds->drum(newPitch) = DrumInstrument(
                instr.name, headGroup, line, stemDir, static_cast<int>(chord->voice()));
        }
    }

    // If there is no exact match add an entry to the drumkit with the XML line and notehead
    if (!matchFound) {
        // Create new instrument in drumkit
        if (stemDir == DirectionV::AUTO) {
            if (line > 4) {
                stemDir = DirectionV::DOWN;
            } else {
                stemDir = DirectionV::UP;
            }
        }

        ds->drum(newPitch) = DrumInstrument(u"drum", headGroup, line, stemDir, static_cast<int>(chord->voice()));
    } else if (stemDir == DirectionV::AUTO) {
        stemDir = ds->stemDirection(newPitch);
    }

    note->setPitch(newPitch);
    note->setTpcFromPitch();
}

//---------------------------------------------------------
//   note
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node.
 */

Note* MusicXmlParserPass2::note(const String& partId,
                                Measure* measure,
                                const Fraction sTime,
                                const Fraction prevSTime,
                                Fraction& missingPrev,
                                Fraction& dura,
                                Fraction& missingCurr,
                                String& currentVoice,
                                GraceChordList& gcl,
                                size_t& gac,
                                Beams& currBeams,
                                FiguredBassList& fbl,
                                int& alt,
                                MusicXmlTupletStates& tupletStates,
                                Tuplets& tuplets, ArpeggioMap& arpMap, DelayedArpMap& delayedArps)
{
    if (m_e.asciiAttribute("print-spacing") == "no") {
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
    String type;
    String voice;
    DirectionV stemDir = DirectionV::AUTO;
    bool noStem = false;
    bool hasHead = true;
    NoteHeadGroup headGroup = NoteHeadGroup::HEAD_NORMAL;
    NoteHeadScheme headScheme = NoteHeadScheme::HEAD_AUTO;
    const Color noteColor = Color::fromString(m_e.asciiAttribute("color").ascii());
    Color noteheadColor;
    Color stemColor;
    Color beamColor;
    bool noteheadParentheses = false;
    String noteheadFilled;
    int velocity = round(m_e.doubleAttribute("dynamics") * 0.9);
    bool graceSlash = false;
    bool printObject = m_e.asciiAttribute("print-object") != "no";
    bool isSingleDrumset = false;
    BeamMode bm;
    std::map<int, String> beamTypes;
    String beamFan;
    String instrumentId;
    String tieType;
    MusicXmlParserLyric lyric { m_pass1.getMusicXmlPart(partId).lyricNumberHandler(), m_e, m_score, m_logger,
                                m_pass1.isVocalStaff(partId) };
    MusicXmlParserNotations notations { m_e, m_score, m_logger, *this };

    MusicXmlNoteDuration mnd { m_divs, m_logger, &m_pass1 };
    MusicXmlNotePitch mnp { m_logger };

    while (m_e.readNextStartElement()) {
        if (mnp.readProperties(m_e, m_score)) {
            // element handled
        } else if (mnd.readProperties(m_e)) {
            // element handled
        } else if (m_e.name() == "accidental") {
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "beam") {
            beamColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            beamFan = m_e.attribute("fan");
            beam(beamTypes);
        } else if (m_e.name() == "chord") {
            chord = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "cue") {
            cue = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "grace") {
            grace = true;
            graceSlash = m_e.asciiAttribute("slash") == "yes";
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "instrument") {
            instrumentId = m_e.attribute("id");
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "lyric") {
            // lyrics on grace notes not (yet) supported by MuseScore
            // add to main note instead
            lyric.parse();
        } else if (m_e.name() == "notations") {
            notations.parse();
            addError(notations.errors());
        } else if (m_e.name() == "notehead") {
            noteheadColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            noteheadParentheses = m_e.asciiAttribute("parentheses") == "yes";
            noteheadFilled = m_e.attribute("filled");
            String noteheadValue = m_e.readText();
            if (noteheadValue == "none") {
                hasHead = false;
            } else if (noteheadValue == "named" && m_pass1.exporterSoftware() == MusicXmlExporterSoftware::NOTEFLIGHT) {
                headScheme = NoteHeadScheme::HEAD_PITCHNAME;
            } else {
                headGroup = convertNotehead(noteheadValue);
            }
        } else if (m_e.name() == "notehead-text") {
            String noteheadText;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "display-text") {
                    noteheadText = m_e.readText();
                } else if (m_e.name() == "accidental-text") {
                    m_e.skipCurrentElement();
                } else {
                    skipLogCurrElem();
                }
            }
            if (noteheadText.size() == 1) {
                const bool isGerman = noteheadText == u"H" || (noteheadText == u"B" && mnp.alter());
                headScheme = isGerman ? NoteHeadScheme::HEAD_PITCHNAME_GERMAN : NoteHeadScheme::HEAD_PITCHNAME;
            } else {
                const std::vector<String> names = { u"Do", u"Re", u"Mi", u"Fa", u"Sol", u"La", u"Si" };
                const bool isFixed = names.at(mnp.step()) == noteheadText;
                headScheme = isFixed ? NoteHeadScheme::HEAD_SOLFEGE_FIXED : NoteHeadScheme::HEAD_SOLFEGE;
            }
        } else if (m_e.name() == "rest") {
            rest = true;
            measureRest = m_e.asciiAttribute("measure") == "yes";
            mnp.displayStepOctave(m_e);
        } else if (m_e.name() == "staff") {
            bool ok = false;
            String strStaff = m_e.readText();
            staff = m_pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt(&ok));
            if (!ok) {
                // error already reported in pass 1
                staff = -1;
            }
        } else if (m_e.name() == "stem") {
            stemColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            stem(stemDir, noStem);
        } else if (m_e.name() == "tie") {
            tieType = m_e.attribute("type");
            m_e.skipCurrentElement();
        } else if (m_e.name() == "type") {
            isSmall = m_e.asciiAttribute("size") == "cue" || m_e.asciiAttribute("size") == "grace-cue";
            type = m_e.readText();
        } else if (m_e.name() == "voice") {
            voice = m_e.readText();
        } else {
            skipLogCurrElem();
        }
    }

    // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
    if (!chord) {
        // remember voice
        currentVoice = voice;
    } else if (voice.empty()) {
        // use voice from last note w/o <chord>
        voice = currentVoice;
    }

    // Assume voice 1 if voice is empty (legal in a single voice part)
    if (voice.empty()) {
        voice = u"1";
    }

    // Define currBeam based on currentVoice to handle multi-voice beaming (and instantiate if not already)
    if (!muse::contains(currBeams, currentVoice)) {
        currBeams.insert({ currentVoice, (Beam*)nullptr });
    }
    Beam*& currBeam = currBeams[currentVoice];

    bm = computeBeamMode(beamTypes);

    // check for timing error(s) and set dura
    // keep in this order as checkTiming() might change dura
    String errorStr = mnd.checkTiming(type, rest, grace);
    dura = mnd.duration();
    if (!errorStr.empty()) {
        m_logger->logError(errorStr, &m_e);
    }

    IF_ASSERT_FAILED(m_pass1.getPart(partId)) {
        return nullptr;
    }

    // At this point all checks have been done, the note should be added
    // note: in case of error exit from here, the postponed <note> children
    // must still be skipped

    int msMove = 0;
    int msTrack = 0;
    int msVoice = 0;

    int voiceInt = m_pass1.voiceToInt(voice);
    if (!m_pass1.determineStaffMoveVoice(partId, staff, voiceInt, msMove, msTrack, msVoice)) {
        m_logger->logDebugInfo(String(u"could not map staff %1 voice '%2'").arg(staff + 1).arg(voice), &m_e);
        addError(checkAtEndElement(m_e, u"note"));
        return 0;
    }

    // start time for note:
    // - sTime for non-chord / first chord note
    // - prevTime for others
    Fraction noteStartTime = chord ? prevSTime : sTime;
    Fraction timeMod = mnd.timeMod();

    // determine tuplet state, used twice (before and after note allocation)
    MusicXmlTupletFlags tupletAction;

    // handle tuplet state for the previous chord or rest
    if (!chord && !grace) {
        Tuplet* tuplet = tuplets[voice];
        MusicXmlTupletState& tupletState = tupletStates[voice];
        tupletAction = tupletState.determineTupletAction(mnd.duration(), timeMod, notations.tupletDesc().type,
                                                         mnd.normalType(), missingPrev, missingCurr);
        if (tupletAction & MusicXmlTupletFlag::STOP_PREVIOUS) {
            // tuplet start while already in tuplet
            if (missingPrev.isValid() && missingPrev > Fraction(0, 1)) {
                const int track = msTrack + msVoice;
                Rest* const extraRest = addRest(m_score, measure, noteStartTime, track, msMove,
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

    Part* part = m_pass1.getPart(partId);
    Instrument* instrument = part->instrument(noteStartTime);
    const MusicXmlInstruments& instruments = m_pass1.getInstruments(partId);
    isSingleDrumset = instrument->drumset() && instruments.size() == 1;
    // begin allocation
    if (rest) {
        if (!grace) {
            const int track = msTrack + msVoice;
            cr = addRest(m_score, measure, noteStartTime, track, msMove,
                         duration, dura);
        } else {
            LOGD("ignoring grace rest");
        }
    } else {
        if (!grace) {
            // regular note
            // if there is already a chord just add to it
            // else create a new one
            // this basically ignores <chord/> errors
            c = findOrCreateChord(m_score, measure,
                                  noteStartTime,
                                  msTrack + msVoice, msMove,
                                  duration, dura, bm, isSmall || cue);
        } else {
            // grace note
            // TODO: check if explicit stem direction should also be set for grace notes
            // (the DOM parser does that, but seems to have no effect on the autotester)
            if (!chord || gcl.empty()) {
                c = createGraceChord(m_score, msTrack + msVoice, duration, graceSlash, isSmall || cue);
                // TODO FIX
                // the setStaffMove() below results in identical behaviour as 2.0:
                // grace note will be at the wrong staff with the wrong pitch,
                // seems to use the line value calculated for the right staff
                // leaving it places the note at the wrong staff with the right pitch
                // this affects only grace notes where staff move differs from
                // the main note, e.g. DebuMandSample.xml first grace in part 2
                // c->setStaffMove(msMove);
                // END TODO
                gcl.push_back(c);
            } else {
                c = gcl.back();
            }
        }
        note = Factory::createNote(c);
        const staff_idx_t ottavaStaff = (msTrack - m_pass1.trackForPart(partId)) / VOICES;
        const int octaveShift = m_pass1.octaveShift(partId, ottavaStaff, noteStartTime);
        const Staff* st = c->staff();
        if (isSingleDrumset && mnp.unpitched() && instrumentId.empty()) {
            xmlSetDrumsetPitch(note, c, st, mnp.displayStep(), mnp.displayOctave(), headGroup, stemDir, instrument);
        } else {
            setPitch(note, instruments, instrumentId, mnp, octaveShift, instrument);
        }
        c->add(note);
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
            handleDisplayStep(cr, mnp.displayStep(), mnp.displayOctave(), noteStartTime, m_score->style().spatium());
        }
    } else {
        handleSmallness(cue || isSmall, note, c);
        note->setPlay(!cue);          // cue notes don't play
        note->setHeadGroup(headGroup);
        if (headScheme != NoteHeadScheme::HEAD_AUTO) {
            note->setHeadScheme(headScheme);
        }
        if (noteColor.isValid()) {
            note->setColor(noteColor);
        }
        Stem* stem = c->stem();
        if (!stem) {
            stem = Factory::createStem(c);
            if (stemColor.isValid()) {
                stem->setColor(stemColor);
            } else if (noteColor.isValid()) {
                stem->setColor(noteColor);
            }
            c->add(stem);
        }
        setNoteHead(note, noteheadColor, noteheadParentheses, noteheadFilled);
        note->setVisible(hasHead && printObject);
        stem->setVisible(printObject);

        if (!grace) {
            // regular note
            // handle beam
            if (!chord) {
                handleBeamAndStemDir(c, bm, stemDir, currBeam, m_pass1.hasBeamingInfo(), beamColor, beamFan);
            }

            // append any grace chord after chord to the previous chord
            Chord* const prevChord = measure->findChord(prevSTime, msTrack + msVoice);
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

        if (mnp.unpitched() && !isSingleDrumset) {
            setDrumset(c, m_pass1, partId, instrumentId, noteStartTime, mnp, stemDir, headGroup);
        }

        // accidental handling
        //LOGD("note acc %p type %hhd acctype %hhd",
        //       acc, acc ? acc->accidentalType() : static_cast<mu::engraving::AccidentalType>(0), accType);
        Accidental* acc = mnp.acc();
        if (!acc && mnp.accType() != AccidentalType::NONE) {
            acc = Factory::createAccidental(m_score->dummy());
            acc->setAccidentalType(mnp.accType());
        }

        if (acc) {
            acc->setVisible(printObject);
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
        notations.addToScore(cr, note,
                             noteStartTime.ticks(), m_slurs, m_glissandi, m_spanners, m_trills, m_ties, m_unstartedTieNotes,
                             m_unendedTieNotes, arpMap, delayedArps);

        // if no tie added yet, convert the "tie" into "tied" and add it.
        if (note && !note->tieFor() && !note->tieBack() && !tieType.empty()) {
            Notation notation = Notation(u"tied");
            const String type2 = u"type";
            notation.addAttribute(type2, tieType);
            addTie(notation, note, cr->track(), m_ties, m_unstartedTieNotes, m_unendedTieNotes, m_logger, &m_e);
        }
    }

    // handle grace after state: remember current grace list size
    if (grace && notations.mustStopGraceAFter()) {
        gac = gcl.size();
    }

    // handle tremolo before handling tuplet (two note tremolos modify timeMod)
    if (cr && notations.hasTremolo()) {
        addTremolo(cr, notations.tremoloNr(), notations.tremoloType(), notations.tremoloSmufl(), notations.tremoloColor(),
                   m_tremStart, m_logger, &m_e, timeMod);
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
                if (tupletAction & MusicXmlTupletFlag::START_NEW) {
                    // create a new tuplet
                    handleTupletStart(cr, tuplet, actualNotes, normalNotes, notations.tupletDesc());
                }
                if (tupletAction & MusicXmlTupletFlag::ADD_CHORD) {
                    cr->setTuplet(tuplet);
                    tuplet->add(cr);
                }
                if (tupletAction & MusicXmlTupletFlag::STOP_CURRENT) {
                    if (missingCurr.isValid() && missingCurr > Fraction(0, 1)) {
                        LOGD("add missing %s to current tuplet", muPrintable(missingCurr.toString()));
                        const int track = msTrack + msVoice;
                        Rest* const extraRest = addRest(m_score, measure, noteStartTime + dura, track, msMove,
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

    // Add all lyrics from grace notes attached to this chord
    if (c && !c->graceNotes().empty() && !m_graceNoteLyrics.empty()) {
        for (GraceNoteLyrics gnl : m_graceNoteLyrics) {
            if (gnl.lyric) {
                addLyric(m_logger, &m_e, cr, gnl.lyric, gnl.no, m_extendedLyrics);
                if (gnl.extend) {
                    m_extendedLyrics.addLyric(gnl.lyric);
                }
            }
        }
        m_graceNoteLyrics.clear();
    }

    if (cr) {
        addInferredStickings(cr, lyric.inferredStickings());
    }

    // add lyrics found by lyric
    if (cr && !grace) {
        // add lyrics and stop corresponding extends
        addLyrics(m_logger, &m_e, cr, lyric.numberedLyrics(), lyric.extendedLyrics(), m_extendedLyrics);
        if (rest) {
            // stop all extends
            m_extendedLyrics.setExtend(-1, cr->track(), cr->tick());
        }
    } else if (c && grace) {
        // Add grace note lyrics to main chord later
        addGraceNoteLyrics(lyric.numberedLyrics(), lyric.extendedLyrics(), m_graceNoteLyrics);
    }

    // add figured bass element
    addFiguredBassElements(fbl, noteStartTime, msTrack, dura, measure);

    // convert to slash or rhythmic notation if needed
    // TODO in the case of slash notation, we assume that given notes do in fact correspond to slash beats
    if (c && m_measureStyleSlash != MusicXmlSlash::NONE) {
        c->setSlash(true, m_measureStyleSlash == MusicXmlSlash::SLASH);
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        dura.set(0, 1);
    }

    addError(checkAtEndElement(m_e, u"note"));

    return note;
}

//---------------------------------------------------------
//   notePrintSpacingNo
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node for a note with print-spacing="no".
 These are handled like a forward: only moving the time forward.
 */

void MusicXmlParserPass2::notePrintSpacingNo(Fraction& dura)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::notePrintSpacingNo", &_e);

    bool chord = false;
    bool grace = false;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "chord") {
            chord = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "duration") {
            duration(dura);
        } else if (m_e.name() == "grace") {
            grace = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else {
            m_e.skipCurrentElement();              // skip but don't log
        }
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        dura.set(0, 1);
    }

    addError(checkAtEndElement(m_e, u"note"));
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXmlParserPass2::duration(Fraction& dura)
{
    dura.set(0, 0);          // invalid unless set correctly
    const String elementText = m_e.readText();
    if (elementText.toInt() > 0) {
        dura = m_pass1.calcTicks(elementText.toInt(), m_divs, &m_e);
    } else {
        m_logger->logError(String(u"illegal duration %1").arg(dura.toString()), &m_e);
    }
    //LOGD("duration %s valid %d", muPrintable(dura.print()), dura.isValid());
}

static FiguredBassItem::Modifier MusicXml2Modifier(const String prefix)
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

FiguredBassItem* MusicXmlParserPass2::figure(const int idx, const bool paren, FiguredBass* parent)
{
    FiguredBassItem* fgi = parent->createItem(idx);

    // read the figure
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "extend") {
            AsciiStringView type = m_e.asciiAttribute("type");
            if (type == "start") {
                fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
            } else if (type == "continue") {
                fgi->setContLine(FiguredBassItem::ContLine::EXTENDED);
            } else if (type == "stop") {
                fgi->setContLine(FiguredBassItem::ContLine::SIMPLE);
            }
            m_e.skipCurrentElement();
        } else if (m_e.name() == "figure-number") {
            const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());
            String val = m_e.readText();
            int iVal = val.toInt();
            // MusicXML spec states figure-number is a number
            // MuseScore can only handle single digit
            if (1 <= iVal && iVal <= 9) {
                fgi->setDigit(iVal);
                fgi->setColor(color);
            } else {
                m_logger->logError(String(u"incorrect figure-number '%1'").arg(val), &m_e);
            }
        } else if (m_e.name() == "prefix") {
            fgi->setPrefix(MusicXml2Modifier(m_e.readText()));
        } else if (m_e.name() == "suffix") {
            fgi->setSuffix(MusicXml2Modifier(m_e.readText()));
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

FiguredBass* MusicXmlParserPass2::figuredBass()
{
    FiguredBass* fb = Factory::createFiguredBass(m_score->dummy()->segment());

    const bool parentheses = m_e.asciiAttribute("parentheses") == "yes";
    const bool printObject = m_e.asciiAttribute("print-object") != "no";
    const String placement = m_e.attribute("placement");
    const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());

    fb->setVisible(printObject);
    if (color.isValid()) {
        fb->setColor(color);
    }

    String normalizedText;
    int idx = 0;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "duration") {
            Fraction dura;
            duration(dura);
            if (dura.isValid() && dura > Fraction(0, 1)) {
                fb->setTicks(dura);
            }
        } else if (m_e.name() == "figure") {
            FiguredBassItem* pItem = figure(idx++, parentheses, fb);
            pItem->setTrack(0 /* TODO fb->track() */);
            pItem->setParent(fb);
            fb->appendItem(pItem);
            // add item normalized text
            if (!normalizedText.empty()) {
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
    fb->resetProperty(Pid::OFFSET);

    if (normalizedText.empty()) {
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

FretDiagram* MusicXmlParserPass2::frame()
{
    FretDiagram* fd = Factory::createFretDiagram(m_score->dummy()->segment());

    const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());
    if (color.isValid()) {
        fd->setColor(color);
        fd->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
    }

    // Format: fret: string
    std::map<int, int> bStarts;
    std::map<int, int> bEnds;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "first-fret") {
            bool ok {};
            int val = m_e.readInt(&ok);
            if (ok && val > 0) {
                fd->setFretOffset(val - 1);
            } else {
                m_logger->logError(String(u"FretDiagram::readMusicXml: illegal first-fret %1").arg(val), &m_e);
            }
        } else if (m_e.name() == "frame-frets") {
            int val = m_e.readInt();
            if (val > 0) {
                fd->setProperty(Pid::FRET_FRETS, val);
                fd->setPropertyFlags(Pid::FRET_FRETS, PropertyFlags::UNSTYLED);
            } else {
                m_logger->logError(String(u"FretDiagram::readMusicXml: illegal frame-fret %1").arg(val), &m_e);
            }
        } else if (m_e.name() == "frame-note") {
            int fret   = -1;
            int string = -1;
            int actualString = -1;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "fret") {
                    fret = m_e.readInt();
                } else if (m_e.name() == "string") {
                    string = m_e.readInt();
                    actualString = fd->strings() - string;
                } else if (m_e.name() == "barre") {
                    // Keep barres to be added later
                    String t = m_e.attribute("type");
                    if (t == "start") {
                        bStarts[fret] = actualString;
                    } else if (t == "stop") {
                        bEnds[fret] = actualString;
                    } else {
                        m_logger->logError(String(u"FretDiagram::readMusicXml: illegal frame-note barre type %1").arg(t), &m_e);
                    }
                    skipLogCurrElem();
                } else {
                    skipLogCurrElem();
                }
            }
            m_logger->logDebugInfo(String(u"FretDiagram::readMusicXml string %1 fret %2").arg(string).arg(fret), &m_e);

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
                m_logger->logError(String(u"FretDiagram::readMusicXml: illegal frame-note string %1").arg(string), &m_e);
            }
        } else if (m_e.name() == "frame-strings") {
            int val = m_e.readInt();
            if (val > 0) {
                fd->setStrings(val);
                for (int i = 0; i < val; ++i) {
                    // MXML Spec: any string without a dot or other marker has a closed string
                    // cross marker above it.
                    fd->setMarker(i, FretMarkerType::CROSS);
                }
            } else {
                m_logger->logError(String(u"FretDiagram::readMusicXml: illegal frame-strings %1").arg(val), &m_e);
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

void MusicXmlParserPass2::harmony(const String& partId, Measure* measure, const Fraction& sTime, HarmonyMap& harmonyMap)
{
    UNUSED(measure);
    track_idx_t track = m_pass1.trackForPart(partId);

    Staff* staff = m_score->staff(track2staff(track));
    Key key = staff ? staff->key(sTime) : Key::INVALID;

    const Color color = Color::fromString(m_e.asciiAttribute("color").ascii());
    const String placement = m_e.attribute("placement");
    const bool printObject = m_e.asciiAttribute("print-object") != "no";

    String kind, kindText, functionText, inversionText, symbols, parens;
    std::list<HDegree> degreeList;

    FretDiagram* fd = nullptr;
    Harmony* ha = Factory::createHarmony(m_score->dummy()->segment());
    HarmonyInfo* info = new HarmonyInfo(m_score);
    Fraction offset;
    if (!placement.empty()) {
        ha->setPlacement(placement == "below" ? PlacementV::BELOW : PlacementV::ABOVE);
        ha->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
        ha->resetProperty(Pid::OFFSET);
    }
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "root") {
            String step;
            int alter = 0;
            bool invalidRoot = false;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "root-step") {
                    // attributes: print-style
                    step = m_e.readText();
                    if (m_e.hasAttribute("text")) {
                        if (m_e.attribute("text").empty()) {
                            invalidRoot = true;
                        }
                    }
                } else if (m_e.name() == "root-alter") {
                    // attributes: print-object, print-style
                    //             location (left-right)
                    alter = m_e.readText().trimmed().toInt();
                } else {
                    skipLogCurrElem();
                }
            }
            if (invalidRoot) {
                info->setRootTpc(Tpc::TPC_INVALID);
            } else {
                info->setRootTpc(step2tpc(step, AccidentalVal(alter)));
            }
        } else if (m_e.name() == "function") {
            // deprecated in MusicXML 4.0
            // attributes: print-style
            info->setRootTpc(Tpc::TPC_INVALID);
            info->setBassTpc(Tpc::TPC_INVALID);
            functionText = m_e.readText();
            ha->setHarmonyType(HarmonyType::ROMAN);
        } else if (m_e.name() == "numeral") {
            info->setRootTpc(Tpc::TPC_INVALID);
            info->setBassTpc(Tpc::TPC_INVALID);
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "numeral-root") {
                    functionText = m_e.attribute("text");
                    const String numeralRoot = m_e.readText();
                    if (functionText.isEmpty() || functionText.at(0).isDigit()) {
                        ha->setHarmonyType(HarmonyType::NASHVILLE);
                        setHarmonyRootTpcFromFunction(info, ha, numeralRoot, key);
                    } else {
                        ha->setHarmonyType(HarmonyType::ROMAN);
                    }
                } else if (m_e.name() == "numeral-alter") {
                    const int alter = m_e.readText().toInt();
                    switch (alter) {
                    case -1:
                        setHarmonyRootTpcFromFunction(info, ha, u"b" + harmonyXmlFunction(info, ha, key), key);
                        break;
                    case 1:
                        setHarmonyRootTpcFromFunction(info, ha, u"#" + harmonyXmlFunction(info, ha, key), key);
                        break;
                    default:
                        break;
                    }
                } else {
                    skipLogCurrElem();
                }
            }
        } else if (m_e.name() == "kind") {
            // attributes: use-symbols  yes-no
            //             text, stack-degrees, parentheses-degree, bracket-degrees,
            //             print-style, halign, valign
            kindText = m_e.attribute("text");
            symbols = m_e.attribute("use-symbols");
            parens = m_e.attribute("parentheses-degrees");
            kind = m_e.readText();
            if (kind == "none") {
                info->setRootTpc(Tpc::TPC_INVALID);
            }
        } else if (m_e.name() == "inversion") {
            const int inversion = m_e.readText().toInt();
            switch (inversion) {
            case 1: inversionText = u"6";
                break;
            case 2: inversionText = u"64";
                break;
            default:
                inversionText = u"";
                break;
            }
        } else if (m_e.name() == "bass") {
            String step;
            int alter = 0;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "bass-step") {
                    // attributes: print-style
                    step = m_e.readText();
                } else if (m_e.name() == "bass-alter") {
                    // attributes: print-object, print-style
                    //             location (left-right)
                    alter = m_e.readText().trimmed().toInt();
                } else {
                    skipLogCurrElem();
                }
            }
            info->setBassTpc(step2tpc(step, AccidentalVal(alter)));
        } else if (m_e.name() == "degree") {
            int degreeValue = 0;
            int degreeAlter = 0;
            String degreeType;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "degree-value") {
                    degreeValue = m_e.readInt();
                } else if (m_e.name() == "degree-alter") {
                    degreeAlter = m_e.readText().trimmed().toInt();
                } else if (m_e.name() == "degree-type") {
                    degreeType = m_e.readText();
                } else {
                    skipLogCurrElem();
                }
            }
            if (degreeValue <= 0 || degreeValue > 13
                || degreeAlter < -2 || degreeAlter > 2
                || (degreeType != u"add" && degreeType != u"alter" && degreeType != u"subtract")) {
                m_logger->logError(String(u"incorrect degree: degreeValue=%1 degreeAlter=%2 degreeType=%3")
                                   .arg(degreeValue).arg(degreeAlter).arg(degreeType), &m_e);
            } else {
                if (degreeType == "add") {
                    degreeList.push_back(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                } else if (degreeType == "alter") {
                    degreeList.push_back(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                } else if (degreeType == "subtract") {
                    degreeList.push_back(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                }
            }
        } else if (m_e.name() == "frame") {
            fd = frame();
        } else if (m_e.name() == "level") {
            skipLogCurrElem();
        } else if (m_e.name() == "offset") {
            offset = m_pass1.calcTicks(m_e.readInt(), m_divs, &m_e);
            preventNegativeTick(sTime, offset, m_logger);
        } else if (m_e.name() == "staff") {
            size_t nstaves = m_pass1.getPart(partId)->nstaves();
            String strStaff = m_e.readText();
            int staffIdx = m_pass1.getMusicXmlPart(partId).staffNumberToIndex(strStaff.toInt());
            if (staffIdx >= 0 && staffIdx < int(nstaves)) {
                track += staffIdx * VOICES;
            } else {
                m_logger->logError(String(u"invalid staff %1").arg(strStaff), &m_e);
            }
        } else {
            skipLogCurrElem();
        }
    }

    const ChordDescription* d = nullptr;
    if (info->rootTpc() != Tpc::TPC_INVALID || ha->harmonyType() == HarmonyType::NASHVILLE) {
        d = harmonyFromXml(info, m_score, kind, kindText, symbols, parens, degreeList);
    }
    if (d) {
        info->setId(d->id);
        info->setTextName(d->names.front());
    } else {
        info->setId(-1);
        String textName = functionText + kindText + inversionText;
        info->setTextName(textName);
    }
    ha->addChord(info);

    ha->setVisible(printObject);
    if (placement == u"below") {
        ha->setPlacement(PlacementV::BELOW);
        ha->resetProperty(Pid::OFFSET);
    }
    if (color.isValid()) {
        ha->setColor(color);
        ha->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
    }

    const HarmonyDesc newHarmonyDesc(track, ha, fd);
    bool insert = true;
    if (m_pass1.sibOrDolet()) {
        const int ticks = (sTime + offset).ticks();
        for (auto itr = harmonyMap.begin(); itr != harmonyMap.end(); itr++) {
            if (itr->first != ticks) {
                continue;
            }
            HarmonyDesc& foundHarmonyDesc = itr->second;
            if (track2staff(foundHarmonyDesc.m_track) == track2staff(track) && foundHarmonyDesc.m_harmony
                && foundHarmonyDesc.descr() == info->descr()) {
                if (foundHarmonyDesc.m_harmony && foundHarmonyDesc.fretDiagramVisible() == newHarmonyDesc.fretDiagramVisible()) {
                    // Matching harmony with matching visibility of fret diagram.  No need to add
                    insert = false;
                } else if (fd && fd->visible() && !foundHarmonyDesc.fretDiagramVisible()) {
                    // Matching harmony without a fret diagram found at this tick, replace with this harmony and its fret diagram
                    foundHarmonyDesc.m_harmony = ha;
                    foundHarmonyDesc.m_fretDiagram = fd;
                    foundHarmonyDesc.m_track = track;
                    insert = false;
                }
            }
        }
    }

    if (insert) {
        // No harmony at this tick, add to the map
        harmonyMap.insert(std::pair<int, HarmonyDesc>((sTime + offset).ticks(), newHarmonyDesc));
    }
}

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/beam node.
 Collects beamTypes, used in computeBeamMode.
 */

void MusicXmlParserPass2::beam(std::map<int, String>& beamTypes)
{
    bool hasBeamNo;
    int beamNo = m_e.asciiAttribute("number").toInt(&hasBeamNo);
    String s = m_e.readText();

    beamTypes.insert({ hasBeamNo ? beamNo : 1, s });
}

//---------------------------------------------------------
//   forward
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/forward node.
 */

void MusicXmlParserPass2::forward(Fraction& dura)
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "duration") {
            duration(dura);
        } else if (m_e.name() == "staff") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "voice") {
            m_e.skipCurrentElement();        // skip but don't log
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

void MusicXmlParserPass2::backup(Fraction& dura)
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "duration") {
            duration(dura);
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   MusicXmlParserLyric
//---------------------------------------------------------

MusicXmlParserLyric::MusicXmlParserLyric(const LyricNumberHandler lyricNumberHandler,
                                         XmlStreamReader& e, Score* score, MusicXmlLogger* logger, bool isVoiceStaff)
    : m_lyricNumberHandler(lyricNumberHandler), m_e(e), m_score(score), m_logger(logger), m_isVoiceStaff(isVoiceStaff)
{
    // nothing
}

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXmlParserLyric::skipLogCurrElem()
{
    //_logger->logDebugInfo(String("skipping '%1'").arg(_e.name().toString()), &_e);
    m_e.skipCurrentElement();
}

void MusicXmlParserLyric::readElision(String& formattedText)
{
    const String text = m_e.readText();
    const String smufl = m_e.attribute("smufl");
    if (!text.empty()) {
        formattedText += text;
    } else if (!smufl.empty()) {
        formattedText += u"<sym>" + smufl + u"</sym>";
    } else {
        formattedText += u"<sym>lyricsElision</sym>";
    }
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

void MusicXmlParserLyric::parse()
{
    bool hasExtend = false;
    const String lyricNumber = m_e.attribute("number");
    const Color lyricColor = Color::fromString(m_e.asciiAttribute("color").ascii());
    const bool printLyric = m_e.asciiAttribute("print-object") != "no";
    m_placement = m_e.attribute("placement");
    double relX = m_e.doubleAttribute("relative-x") * 0.1 * DPMM;
    m_relativeY = m_e.doubleAttribute("relative-y") * -0.1 * DPMM;
    m_defaultY = m_e.doubleAttribute("default-y") * -0.1 * DPMM;
    LyricsSyllabic syllabic = LyricsSyllabic::SINGLE;

    String extendType;
    String formattedText;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "elision") {
            readElision(formattedText);
        } else if (m_e.name() == "extend") {
            hasExtend = true;
            extendType = m_e.attribute("type");
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "syllabic") {
            String syll = m_e.readText();
            if (syll == "single") {
                syllabic = LyricsSyllabic::SINGLE;
            } else if (syll == "begin") {
                syllabic = LyricsSyllabic::BEGIN;
            } else if (syll == "end") {
                syllabic = LyricsSyllabic::END;
            } else if (syll == "middle") {
                syllabic = LyricsSyllabic::MIDDLE;
            } else {
                LOGD("unknown syllabic %s", muPrintable(syll));                      // TODO
            }
        } else if (m_e.name() == "text") {
            formattedText += xmlpass2::nextPartOfFormattedString(m_e);
        } else {
            skipLogCurrElem();
        }
    }

    // if no lyric read (e.g. only 'extend "type=stop"'), no further action required
    if (formattedText.empty()) {
        return;
    }

    const int lyricNo = m_lyricNumberHandler.getLyricNo(lyricNumber);
    if (lyricNo < 0) {
        m_logger->logError(u"invalid lyrics number (<0)", &m_e);
        return;
    } else if (lyricNo > MAX_LYRICS) {
        m_logger->logError(String(u"too much lyrics (>%1)").arg(MAX_LYRICS), &m_e);
        return;
    } else if (muse::contains(m_numberedLyrics, lyricNo)) {
        m_logger->logError(String(u"duplicate lyrics number (%1)").arg(lyricNumber), &m_e);
        return;
    }

    TextBase* item = nullptr;
    if (isLikelySticking(formattedText, syllabic, hasExtend)) {
        item = Factory::createSticking(m_score->dummy()->segment());
    } else {
        item = Factory::createLyrics(m_score->dummy()->chord());
    }

    //LOGD("formatted lyric '%s'", muPrintable(formattedText));
    item->setXmlText(formattedText);
    if (lyricColor.isValid()) {
        item->setProperty(Pid::COLOR, lyricColor);
        item->setPropertyFlags(Pid::COLOR, PropertyFlags::UNSTYLED);
    }
    item->setVisible(printLyric);

    item->setPlacement(placement() == "above" ? PlacementV::ABOVE : PlacementV::BELOW);
    item->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    item->resetProperty(Pid::OFFSET);

    if (!RealIsNull(relX)) {
        PointF offset = item->offset();
        offset.setX(relX);
        item->setOffset(offset);
        item->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
    }

    if (item->isLyrics()) {
        // Add lyric
        Lyrics* l = toLyrics(item);
        l->setSyllabic(syllabic);
        m_numberedLyrics[lyricNo] = l;

        if (hasExtend
            && (extendType.empty() || extendType == "start")
            && (l->syllabic() == LyricsSyllabic::SINGLE || l->syllabic() == LyricsSyllabic::END)) {
            m_extendedLyrics.insert(l);
        }
    } else if (item->isSticking()) {
        // Add sticking
        Sticking* s = toSticking(item);
        m_inferredStickings.push_back(s);
    }
}

String MusicXmlParserLyric::placement() const
{
    if (m_placement == "" && hasTotalY()) {
        return totalY() < 0 ? u"above" : u"below";
    } else {
        return m_placement;
    }
}

bool MusicXmlParserLyric::isLikelySticking(const String& text, const LyricsSyllabic syllabic, const bool hasExtend)
{
    if (!configuration()->inferTextType()) {
        return false;
    }
    String plainWords = MScoreTextToMusicXml::toPlainText(text.simplified());
    static const std::wregex sticking(L"^[lrbLRB]+$");
    return plainWords.contains(sticking) && syllabic == LyricsSyllabic::SINGLE && !hasExtend && !m_isVoiceStaff;
}

//---------------------------------------------------------
//   slur
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/slur node.
 */

void MusicXmlParserNotations::slur()
{
    Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                         m_e.attributes(), u"notations");
    m_notations.push_back(notation);

    // any grace note containing a slur stop means
    // last note of a grace after set has been found
    // -> remember slur stop
    if (notation.attribute(u"type") == u"stop") {
        m_slurStop = true;
    } else if (notation.attribute(u"type") == u"start") {
        m_slurStart = true;
    }

    m_e.skipCurrentElement();  // skip but don't log
}

//---------------------------------------------------------
//   addSlur
//---------------------------------------------------------

static void addSlur(const Notation& notation, SlurStack& slurs, ChordRest* cr, const int tick,
                    MusicXmlLogger* logger, const XmlStreamReader* const xmlreader)
{
    int slurNo = notation.attribute(u"number").toInt();
    if (slurNo > 0) {
        slurNo--;
    }
    const String slurType = notation.attribute(u"type");

    const track_idx_t track = cr->track();
    Score* score = cr->score();

    // PriMus Music-Notation by Columbussoft (build 10093) generates overlapping
    // slurs that do not have a number attribute to distinguish them.
    // The duplicates must be ignored, to prevent memory allocation issues,
    // which caused a MuseScore crash
    // Similar issues happen with Sibelius 7.1.3 (direct export)

    if (slurType == u"start") {
        if (slurs[slurNo].isStart()) {
            // slur start when slur already started: report error
            logger->logError(String(u"ignoring duplicate slur start"), xmlreader);
        } else if (slurs[slurNo].isStop()) {
            // slur start when slur already stopped: wrap up
            Slur* newSlur = slurs[slurNo].slur();
            newSlur->setTrack(track);
            newSlur->setTick(Fraction::fromTicks(tick));
            newSlur->setStartElement(cr);
            newSlur->setTick2(newSlur->endElement()->tick());
            slurs[slurNo] = SlurDesc();
            if (newSlur->ticks().negative()) {
                logger->logError(String(u"slur end is before slur start"), xmlreader);
                delete newSlur;
                return;
            }
            score->addElement(newSlur);
        } else {
            // slur start for new slur: init
            Slur* newSlur = notation.name() == "slur"
                            ? Factory::createSlur(score->dummy())
                            : static_cast<Slur*>(Factory::createHammerOnPullOff(score->dummy()));
            if (cr->isGrace()) {
                newSlur->setAnchor(Spanner::Anchor::CHORD);
            }
            const String lineType = notation.attribute(u"line-type");
            if (lineType == u"dashed") {
                newSlur->setStyleType(SlurStyleType::Dashed);
            } else if (lineType == u"dotted") {
                newSlur->setStyleType(SlurStyleType::Dotted);
            } else if (lineType == u"solid" || lineType.empty()) {
                newSlur->setStyleType(SlurStyleType::Solid);
            }
            const Color color = Color::fromString(notation.attribute(u"color"));
            if (color.isValid()) {
                newSlur->setColor(color);
            }
            newSlur->setTick(Fraction::fromTicks(tick));
            newSlur->setStartElement(cr);
            if (configuration()->importLayout()) {
                const String orientation = notation.attribute(u"orientation");
                const String placement = notation.attribute(u"placement");
                if (orientation == u"over" || placement == u"above") {
                    newSlur->setSlurDirection(DirectionV::UP);
                } else if (orientation == u"under" || placement == u"below") {
                    newSlur->setSlurDirection(DirectionV::DOWN);
                } else if (orientation.empty() || placement.empty()) {
                    // ignore
                } else {
                    logger->logError(String(u"unknown slur orientation/placement: %1/%2").arg(orientation).arg(placement), xmlreader);
                }
            }
            newSlur->setTrack(track);
            newSlur->setTrack2(track);
            slurs[slurNo].start(newSlur);
        }
    } else if (slurType == u"stop") {
        if (slurs[slurNo].isStart()) {
            // slur stop when slur already started: wrap up
            Slur* newSlur = slurs[slurNo].slur();
            if (!(cr->isGrace())) {
                newSlur->setTick2(Fraction::fromTicks(tick));
                newSlur->setTrack2(track);
            }
            newSlur->setEndElement(cr);
            slurs[slurNo] = SlurDesc();
            score->addElement(newSlur);
        } else if (slurs[slurNo].isStop()) {
            // slur stop when slur already stopped: report error
            logger->logError(String(u"ignoring duplicate slur stop"), xmlreader);
        } else {
            // slur stop for new slur: init
            Slur* newSlur = Factory::createSlur(score->dummy());
            if (!(cr->isGrace())) {
                newSlur->setTrack2(track);
            }
            newSlur->setEndElement(cr);
            slurs[slurNo].stop(newSlur);
        }
    } else if (slurType == "continue") {
        // ignore
    } else {
        logger->logError(String(u"unknown slur type %1").arg(slurType), xmlreader);
    }
}

//---------------------------------------------------------
//   tied
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/tied node.
 */

void MusicXmlParserNotations::tied()
{
    Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()), m_e.attributes(), u"notations");
    // Make sure "stops" get processed before "starts"
    if (notation.attribute(u"type") == u"stop") {
        m_notations.insert(m_notations.begin(), notation);
    } else {
        m_notations.push_back(notation);
    }
    String tiedType = notation.attribute(u"type");
    if (tiedType != u"start" && tiedType != u"stop" && tiedType != u"let-ring") {
        m_logger->logError(String(u"unknown tied type %1").arg(tiedType), &m_e);
    }

    m_e.skipCurrentElement();  // skip but don't log
}

//---------------------------------------------------------
//   dynamics
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/dynamics node.
 */

void MusicXmlParserNotations::dynamics()
{
    m_dynamicsPlacement = m_e.attribute("placement");

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "other-dynamics") {
            m_dynamicsList.push_back(m_e.readText());
        } else {
            m_dynamicsList.push_back(String::fromAscii(m_e.name().ascii()));
            m_e.skipCurrentElement();  // skip but don't log
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

void MusicXmlParserNotations::articulations()
{
    while (m_e.readNextStartElement()) {
        SymId id { SymId::noSym };
        if (convertArticulationToSymId(String::fromAscii(m_e.name().ascii()), id)) {
            if (m_e.name() == "detached-legato") {
                m_notations.push_back(Notation::notationWithAttributes(u"tenuto",
                                                                       m_e.attributes(), u"articulations", SymId::articTenutoAbove));
                m_notations.push_back(Notation::notationWithAttributes(u"staccato",
                                                                       m_e.attributes(), u"articulations", SymId::articStaccatoAbove));
            } else {
                Notation artic = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                  m_e.attributes(), u"articulations", id);
                m_notations.push_back(artic);
            }
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "breath-mark") {
            std::vector<XmlStreamReader::Attribute> attributes = m_e.attributes();
            String value = m_e.readText();
            engraving::SymId breath = SymId::noSym;
            if (value == "tick") {
                breath = SymId::breathMarkTick;
            } else if (value == "upbow") {
                breath = SymId::breathMarkUpbow;
            } else if (value == "salzedo") {
                breath = SymId::breathMarkSalzedo;
            } else {
                // Use comma as the default symbol
                breath = SymId::breathMarkComma;
            }
            m_notations.push_back(Notation::notationWithAttributes(u"breath",
                                                                   attributes, u"articulations", breath));
        } else if (m_e.name() == "caesura") {
            std::vector<XmlStreamReader::Attribute> attributes = m_e.attributes();
            String value = m_e.readText();
            engraving::SymId caesura = SymId::noSym;
            if (value == "curved") {
                caesura = SymId::caesuraCurved;
            } else if (value == "short") {
                caesura = SymId::caesuraShort;
            } else if (value == "thick") {
                caesura = SymId::caesuraThick;
            } else if (value == "single") {
                caesura = SymId::caesuraSingleStroke;
            } else { // Use as the default symbol
                caesura = SymId::caesura;
            }
            m_notations.push_back(Notation::notationWithAttributes(u"breath",
                                                                   attributes, u"articulations", caesura));
        } else if (m_e.name() == "doit"
                   || m_e.name() == "falloff"
                   || m_e.name() == "plop"
                   || m_e.name() == "scoop") {
            Notation artic = Notation::notationWithAttributes(u"chord-line",
                                                              m_e.attributes(), u"articulations");
            artic.setSubType(String::fromAscii(m_e.name().ascii()));
            m_notations.push_back(artic);
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "other-articulation") {
            const String smufl = m_e.attribute("smufl");

            if (!smufl.empty()) {
                SymId sid = SymNames::symIdByName(smufl, SymId::noSym);
                Notation artic = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                  m_e.attributes(), u"articulations", sid);
                m_notations.push_back(artic);
            }
            m_e.skipCurrentElement();  // skip but don't log
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

void MusicXmlParserNotations::ornaments()
{
    bool trillMark = false;
    // <trill-mark placement="above"/>
    while (m_e.readNextStartElement()) {
        SymId id { SymId::noSym };
        if (convertArticulationToSymId(String::fromAscii(m_e.name().ascii()), id)) {
            Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                 m_e.attributes(), u"ornaments", id);
            m_notations.push_back(notation);
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "trill-mark") {
            trillMark = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "wavy-line") {
            bool wavyLineTypeWasStart = (m_wavyLineType == "start");
            m_wavyLineType = m_e.attribute("type");
            m_wavyLineNo   = m_e.intAttribute("number");
            if (m_wavyLineNo > 0) {
                m_wavyLineNo--;
            }
            // any grace note containing a wavy-line stop means
            // last note of a grace after set has been found
            // remember wavy-line stop
            if (m_wavyLineType == u"stop") {
                m_wavyLineStop = true;
            }
            // check for start and stop on same note
            if (wavyLineTypeWasStart && m_wavyLineType == u"stop") {
                m_wavyLineType = u"startstop";
            }
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "tremolo") {
            m_hasTremolo = true;
            m_tremoloColor = Color::fromString(m_e.attribute("color"));
            m_tremoloType = m_e.attribute("type");
            m_tremoloSmufl = m_e.attribute("smufl");
            m_tremoloNr = m_e.readInt();
        } else if (m_e.name() == "inverted-mordent"
                   || m_e.name() == "mordent") {
            mordentNormalOrInverted();
        } else if (m_e.name() == "other-ornament") {
            Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                 m_e.attributes(), u"ornaments");
            m_notations.push_back(notation);
            m_e.skipCurrentElement();  // skip but don't log
        } else {
            skipLogCurrElem();
        }
    }

    // note that mscore wavy line already implicitly includes a trillsym
    // so don't add an additional one
    if (trillMark && m_wavyLineType != "start" && m_wavyLineType != "startstop") {
        Notation ornament = Notation::notationWithAttributes(u"trill-mark", m_e.attributes(), u"ornaments", SymId::ornamentTrill);
        m_notations.push_back(ornament);
    }
}

//---------------------------------------------------------
//   technical
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical node.
 */

void MusicXmlParserNotations::technical()
{
    while (m_e.readNextStartElement()) {
        SymId id { SymId::noSym };
        const String smufl = m_e.attribute("smufl");
        if (!smufl.empty()) {
            id = SymNames::symIdByName(smufl, SymId::noSym);
            Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                 m_e.attributes(), u"technical", id);
            m_notations.push_back(notation);
            m_e.skipCurrentElement();
        } else if (convertArticulationToSymId(String::fromAscii(m_e.name().ascii()), id)) {
            Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                 m_e.attributes(), u"technical", id);
            m_notations.push_back(notation);
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "fingering" || m_e.name() == "fret" || m_e.name() == "pluck"
                   || m_e.name() == "string" || m_e.name() == "tap") {
            Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                 m_e.attributes(), u"technical");
            notation.setText(m_e.readText());
            m_notations.push_back(notation);
        } else if (m_e.name() == "harmonic") {
            harmonic();
        } else if (m_e.name() == "handbell") {
            const std::vector<XmlStreamReader::Attribute> attributes = m_e.attributes();
            convertArticulationToSymId(m_e.readText(), id);
            m_notations.push_back(Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                                   attributes, u"technical", id));
        } else if (m_e.name() == "harmon-mute") {
            harmonMute();
        } else if (m_e.name() == "hole") {
            hole();
        } else if (m_e.name() == "other-technical") {
            otherTechnical();
        } else {
            skipLogCurrElem();
        }
    }
}

void MusicXmlParserNotations::otherTechnical()
{
    const Color color = Color::fromString(m_e.attribute("color"));
    const String text = m_e.readText();

    if (text == u"z") {
        // Buzz roll
        m_hasTremolo = true;
        m_tremoloNr = 0;
        m_tremoloType = u"unmeasured";
        m_tremoloColor = color;
    }
}

//---------------------------------------------------------
//   harmonic
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical/harmonic node.
 */

void MusicXmlParserNotations::harmonic()
{
    Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                         m_e.attributes(), u"technical", SymId::stringsHarmonic);

    while (m_e.readNextStartElement()) {
        String name = String::fromAscii(m_e.name().ascii());
        if (name == "natural") {
            notation.setSubType(name);
            m_e.skipCurrentElement();  // skip but don't log
        } else if (name == "artificial") {   // TODO: add artificial harmonic when supported by MuseScore
            m_logger->logError(String(u"unsupported harmonic type/pitch '%1'").arg(name), &m_e);
            notation.setSymId(SymId::noSym);
            m_e.skipCurrentElement();
        } else {
            m_e.skipCurrentElement();
        }
    }

    m_notations.push_back(notation);
}

//---------------------------------------------------------
//   harmonMute
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical/harmon-mute node.
 */

void MusicXmlParserNotations::harmonMute()
{
    engraving::SymId mute = SymId::brassHarmonMuteClosed;
    const std::vector<XmlStreamReader::Attribute> attributes = m_e.attributes();
    while (m_e.readNextStartElement()) {
        String name = String::fromAscii(m_e.name().ascii());
        if (name == "harmon-closed") {
            const String location = m_e.attribute("location");
            String value = m_e.readText();
            if (value == "yes") {
                mute = SymId::brassHarmonMuteClosed;
            } else if (value == "no") {
                mute = SymId::brassHarmonMuteStemOpen;
            } else if (value == "half") {
                if (location == "left") {
                    mute = SymId::brassHarmonMuteStemHalfLeft;
                } else if (location == "right") {
                    mute = SymId::brassHarmonMuteStemHalfRight;
                } else {
                    m_logger->logError(String(u"unsupported harmon-closed location '%1'").arg(location), &m_e);
                    mute = SymId::brassHarmonMuteStemHalfLeft;
                }
            }
        } else {
            m_e.skipCurrentElement();
        }
    }
    m_notations.push_back(Notation::notationWithAttributes(u"harmon-closed", attributes, u"technical", mute));
}

//---------------------------------------------------------
//   hole
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/technical/hole node.
 */

void MusicXmlParserNotations::hole()
{
    engraving::SymId hole = SymId::noSym;
    const std::vector<XmlStreamReader::Attribute> attributes = m_e.attributes();
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "hole-closed") {
            const String location = m_e.attribute("location");
            const String value = m_e.readText();
            if (value == "yes") {
                hole = SymId::windClosedHole;
            } else if (value == "no") {
                hole = SymId::windOpenHole;
            } else if (value == "half") {
                if (location == "bottom") {
                    hole = SymId::windHalfClosedHole2;
                } else if (location == "right") {
                    hole = SymId::windHalfClosedHole1;
                } else {
                    m_logger->logError(String(u"unsupported hole-closed location '%1'").arg(location), &m_e);
                    hole = SymId::windHalfClosedHole3;
                }
            }
        } else {
            m_e.skipCurrentElement();
        }
    }
    m_notations.push_back(Notation::notationWithAttributes(u"hole-closed", attributes, u"technical", hole));
}

//---------------------------------------------------------
//   addTechnical
//---------------------------------------------------------

void MusicXmlParserNotations::addTechnical(const Notation& notation, Note* note)
{
    const String placement = notation.attribute(u"placement");
    const String fontWeight = notation.attribute(u"font-weight");
    const double fontSize = notation.attribute(u"font-size").toDouble();
    const String fontStyle = notation.attribute(u"font-style");
    const String fontFamily = notation.attribute(u"font-family");
    const Color color = Color::fromString(notation.attribute(u"color"));
    if (notation.name() == u"fingering") {
        // TODO: distinguish between keyboards (style TextStyleName::FINGERING)
        // and (plucked) strings (style TextStyleName::LH_GUITAR_FINGERING)
        addTextToNote(m_e.lineNumber(), m_e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                      color, TextStyleType::FINGERING, m_score, note);
    } else if (notation.name() == u"fret") {
        int fret = notation.text().toInt();
        if (note) {
            if (note->staff()->isTabStaff(Fraction(0, 1))) {
                note->setFret(fret);
            }
        } else {
            m_logger->logError(u"no note for fret", &m_e);
        }
    } else if (notation.name() == "pluck") {
        addTextToNote(m_e.lineNumber(), m_e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                      color, TextStyleType::RH_GUITAR_FINGERING, m_score, note);
    } else if (notation.name() == "string") {
        if (note) {
            if (note->staff()->isTabStaff(Fraction(0, 1))) {
                note->setString(notation.text().toInt() - 1);
            } else {
                addTextToNote(m_e.lineNumber(), m_e.columnNumber(), notation.text(), placement, fontWeight, fontSize, fontStyle, fontFamily,
                              color, TextStyleType::STRING_NUMBER, m_score, note);
            }
        } else {
            m_logger->logError(u"no note for string", &m_e);
        }
    } else if (notation.name() == u"tap") {
        if (note) {
            addTapToChord(notation, note->chord());
        } else {
            m_logger->logError(u"no note for tap", &m_e);
        }
    }
}

void MusicXmlParserNotations::arpeggio()
{
    m_arpeggioType = m_e.attribute("direction");
    if (m_arpeggioType.empty()) {
        m_arpeggioType = u"none";
    }
    m_arpeggioNo = m_e.intAttribute("number");
    if (m_arpeggioNo == 0) {
        m_arpeggioNo = 1;
    }
    m_arpeggioColor = Color::fromString(m_e.attribute("color"));
    m_e.skipCurrentElement();  // skip but don't log
}

//---------------------------------------------------------
//   mordentNormalOrInverted
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/ornaments/mordent
 and /score-partwise/part/measure/note/notations/ornaments/inverted-mordent nodes.
 */

void MusicXmlParserNotations::mordentNormalOrInverted()
{
    Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()), m_e.attributes(), u"ornaments");
    notation.setText(m_e.readText());
    m_notations.push_back(notation);
}

//---------------------------------------------------------
//   glissandoSlide
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/glissando
 and /score-partwise/part/measure/note/notations/slide nodes.
 */

void MusicXmlParserNotations::glissandoSlide()
{
    Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()), m_e.attributes(), u"notations");
    notation.setText(m_e.readText());
    m_notations.push_back(notation);
}

//---------------------------------------------------------
//   addGlissandoSlide
//---------------------------------------------------------

static void addGlissandoSlide(const Notation& notation, Note* note,
                              Glissando* glissandi[MAX_NUMBER_LEVEL][2], MusicXmlSpannerMap& spanners,
                              MusicXmlLogger* logger, const XmlStreamReader* const xmlreader)
{
    int glissandoNumber = notation.attribute(u"number").toInt();
    if (glissandoNumber > 0) {
        glissandoNumber--;
    }
    const String glissandoType = notation.attribute(u"type");
    const bool glissandoTag = notation.name() != u"slide";
    const String lineType  = notation.attribute(u"line-type");
    Glissando*& gliss = glissandi[glissandoNumber][glissandoTag];

    const Fraction tick = note->tick();
    const track_idx_t track = note->track();

    if (glissandoType == u"start") {
        const Color glissandoColor = Color::fromString(notation.attribute(u"color"));
        const String glissandoText = notation.text();
        if (gliss) {
            logger->logError(String(u"overlapping glissando/slide number %1").arg(glissandoNumber + 1), xmlreader);
        } else if (!note) {
            logger->logError(String(u"no note for glissando/slide number %1 start").arg(glissandoNumber + 1), xmlreader);
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
            if (lineType == u"dashed") {
                gliss->setLineStyle(LineType::DASHED);
            } else if (lineType == u"dotted") {
                gliss->setLineStyle(LineType::DOTTED);
            } else if (lineType == u"solid" || lineType.empty()) {
                gliss->setLineStyle(LineType::SOLID);
            }
            gliss->setText(glissandoText);
            gliss->setGlissandoType(glissandoTag || (lineType == u"wavy") ? GlissandoType::WAVY : GlissandoType::STRAIGHT);
            spanners[gliss] = std::pair<int, int>(tick.ticks(), -1);
            // LOGD("glissando/slide=%p inserted at first tick %d", gliss, tick);
        }
    } else if (glissandoType == u"stop") {
        if (!gliss) {
            logger->logError(String(u"glissando/slide number %1 stop without start").arg(glissandoNumber + 1), xmlreader);
        } else if (!note) {
            logger->logError(String(u"no note for glissando/slide number %1 stop").arg(glissandoNumber + 1), xmlreader);
        } else if (gliss->tick() == tick) {
            // A gliss starting and ending on the same tick is a fall
            ChordLine* cl = Factory::createChordLine(note->chord());
            cl->setChordLineType(ChordLineType::FALL);
            cl->setWavy(gliss->glissandoType() == GlissandoType::WAVY ? true : false);
            cl->setStraight(true);
            cl->setParent(note);

            note->chord()->add(cl);
            spanners.erase(gliss);
            delete gliss;
            gliss = nullptr;
        } else {
            // Remove glissandos longer than 16 beats
            if ((tick + note->chord()->ticks()) - gliss->tick() >= Fraction(4, 1)) {
                logger->logError(String(u"glissando/slide number %1 too long (%2)").arg(glissandoNumber + 1,
                                                                                        ((tick + note->chord()->ticks())
                                                                                         - gliss->tick()).ticks()), xmlreader);
                spanners.erase(gliss);
            } else {
                spanners[gliss].second = tick.ticks() + note->chord()->ticks().ticks();
                gliss->setEndElement(note);
                gliss->setTick2(tick);
                gliss->setTrack2(track);
                // LOGD("glissando/slide=%p second tick %d", gliss, tick);
            }
            gliss = nullptr;
        }
    } else {
        logger->logError(String(u"unknown glissando/slide type %1").arg(glissandoType), xmlreader);
    }
}

//---------------------------------------------------------
//   addArpeggio
//---------------------------------------------------------

static void addArpeggio(ChordRest* cr, String& arpeggioType, int arpeggioNo, Color arpeggioColor,
                        ArpeggioMap& arpMap, DelayedArpMap& delayedArps)
{
    if (cr->isRest() && !arpeggioType.empty()) {
        // If the arpeggio is attached to a rest, store to add to the next available chord
        DelayedArpeggio delayedArp(arpeggioType, arpeggioNo);
        delayedArps.insert(std::pair<int, DelayedArpeggio>(cr->tick().ticks(), delayedArp));
    } else {
        // Retrieve stored arpeggio to add to this chord
        DelayedArpeggio delayedArp = muse::value(delayedArps, cr->tick().ticks(), DelayedArpeggio(u"", 0));
        if (!delayedArp.m_arpeggioType.empty()) {
            arpeggioType = delayedArp.m_arpeggioType;
            arpeggioNo = delayedArp.m_arpeggioNo;
            delayedArps.erase(cr->tick().ticks());
        }
    }

    // If no current arpeggio with same number add new
    // If not, expand span
    // no support for arpeggio on rest
    const std::vector<MusicXmlArpeggioDesc> arps = muse::values(arpMap, cr->tick().ticks());
    Arpeggio* curArp = nullptr;
    for (const MusicXmlArpeggioDesc arp : arps) {
        if (arp.no == arpeggioNo) {
            curArp = arp.arp;
        }
    }

    if (curArp) {
        track_idx_t chordTrack = cr->track();
        track_idx_t arpTrack = curArp->track();
        track_idx_t span = cr->track() - curArp->track();
        if (chordTrack > arpTrack && span != 0) {
            curArp->setSpan(static_cast<int>(span + 1));
        }
    } else {
        if (!arpeggioType.empty() && cr->type() == ElementType::CHORD) {
            Arpeggio* arpeggio = Factory::createArpeggio(mu::engraving::toChord(cr));
            arpeggio->setArpeggioType(ArpeggioType::NORMAL);
            if (arpeggioType == "up") {
                arpeggio->setArpeggioType(ArpeggioType::UP);
            } else if (arpeggioType == "down") {
                arpeggio->setArpeggioType(ArpeggioType::DOWN);
            } else if (arpeggioType == "non-arpeggiate") {
                arpeggio->setArpeggioType(ArpeggioType::BRACKET);
            }
            if (arpeggioColor.isValid()) {
                arpeggio->setColor(arpeggioColor);
            }
            // there can be only one
            if (!(static_cast<Chord*>(cr))->arpeggio()) {
                cr->add(arpeggio);

                MusicXmlArpeggioDesc arpDesc(arpeggio, arpeggioNo);
                arpMap.insert(std::pair<int, MusicXmlArpeggioDesc>(cr->tick().ticks(), arpDesc));
            }
        }
    }
}

//---------------------------------------------------------
//   addTie
//---------------------------------------------------------

static void addTie(const Notation& notation, Note* note, const track_idx_t track, MusicXmlTieMap& ties,
                   std::vector<Note*>& unstartedTieNotes, std::vector<Note*>& unendedTieNotes, MusicXmlLogger* logger,
                   const XmlStreamReader* const xmlreader)
{
    IF_ASSERT_FAILED(note) {
        return;
    }

    const String type = notation.attribute(u"type");
    const String orientation = notation.attribute(u"orientation");
    const String placement = notation.attribute(u"placement");
    const String lineType = notation.attribute(u"line-type");

    TieLocation loc = TieLocation(note->pitch(), note->track());

    if (type.empty()) {
        // ignore, nothing to do
    } else if (type == u"start") {
        if (Tie* activeTie = muse::value(ties, loc, nullptr)) {
            logger->logError(String(u"Tie already active"), xmlreader);
            unendedTieNotes.push_back(activeTie->startNote());
            ties.erase(loc);
        }
        ties[loc] = Factory::createTie(note);
        Tie* currTie = ties[loc];
        note->setTieFor(currTie);
        currTie->setStartNote(note);
        currTie->setTrack(track);

        const Color color = Color::fromString(notation.attribute(u"color"));
        if (color.isValid()) {
            currTie->setColor(color);
        }

        if (configuration()->importLayout()) {
            if (orientation == u"over" || placement == u"above") {
                currTie->setSlurDirection(DirectionV::UP);
            } else if (orientation == u"under" || placement == u"below") {
                currTie->setSlurDirection(DirectionV::DOWN);
            } else if (orientation.empty() || placement.empty()) {
                // ignore
            } else {
                logger->logError(String(u"unknown tied orientation/placement: %1/%2").arg(orientation).arg(placement), xmlreader);
            }
        }

        if (lineType == u"dashed") {
            currTie->setStyleType(SlurStyleType::Dashed);
        } else if (lineType == u"dotted") {
            currTie->setStyleType(SlurStyleType::Dotted);
        } else if (lineType == u"solid" || lineType.empty()) {
            currTie->setStyleType(SlurStyleType::Solid);
        }
        currTie = nullptr;
    } else if (type == "stop") {
        if (Tie* currTie = muse::value(ties, loc, nullptr)) {
            const Note* startNote = currTie->startNote();
            const Chord* startChord = startNote ? startNote->chord() : nullptr;
            const Chord* endChord = note->chord();
            const Measure* startMeasure = startChord ? startChord->measure() : nullptr;
            if (startMeasure == endChord->measure()
                || (startChord && startChord->tick() + startChord->measure()->ticks() >= endChord->tick())) {
                // only connect if they're in the same bar or no further than a full measure apart
                currTie->setEndNote(note);
                note->setTieBack(currTie);
            } else {
                logger->logError(String(u"Intervening note in voice"), xmlreader);
                unstartedTieNotes.push_back(note);
                unendedTieNotes.push_back(currTie->startNote());
            }
            ties.erase(loc);
        } else {
            unstartedTieNotes.push_back(note);
            logger->logError(String(u"Non-started tie terminated. No-op."), xmlreader);
        }
    } else if (type == "let-ring") {
        LaissezVib* lvTie = Factory::createLaissezVib(note);
        lvTie->setParent(note);
        note->score()->undoAddElement(lvTie);
    } else {
        logger->logError(String(u"unknown tied type %1").arg(type), xmlreader);
    }
}

//---------------------------------------------------------
//   addWavyLine
//---------------------------------------------------------

static void addWavyLine(ChordRest* cr, const Fraction& tick,
                        const int wavyLineNo, const String& wavyLineType,
                        MusicXmlSpannerMap& spanners, TrillStack& trills,
                        MusicXmlLogger* logger, const XmlStreamReader* const xmlreader)
{
    if (!wavyLineType.empty()) {
        const Fraction ticks = cr->ticks();
        const track_idx_t track = cr->track();
        Trill*& trill = trills[wavyLineNo];
        if (wavyLineType == u"start" || wavyLineType == u"startstop") {
            if (trill) {
                logger->logError(String(u"overlapping wavy-line number %1").arg(wavyLineNo + 1), xmlreader);
            } else {
                trill = Factory::createTrill(cr->score()->dummy());
                trill->setTrack(track);
                trill->setTrack2(track);

                trill->setOrnament(Factory::createOrnament(cr));
                trill->ornament()->setAnchor(ArticulationAnchor::AUTO);

                if (wavyLineType == u"start") {
                    spanners[trill] = std::pair<int, int>(tick.ticks(), -1);
                    // LOGD("trill=%p inserted at first tick %d", trill, tick);
                }
                if (wavyLineType == u"startstop") {
                    spanners[trill] = std::pair<int, int>(tick.ticks(), tick.ticks() + ticks.ticks());
                    trill = nullptr;
                    // LOGD("trill=%p inserted at first tick %d second tick %d", trill, tick, tick);
                }
            }
        } else if (wavyLineType == u"stop") {
            if (!trill) {
                logger->logError(String(u"wavy-line number %1 stop without start").arg(wavyLineNo + 1), xmlreader);
            } else {
                spanners[trill].second = tick.ticks() + ticks.ticks();
                // LOGD("trill=%p second tick %d", trill, tick);
                trill = nullptr;
            }
        } else {
            logger->logError(String(u"unknown wavy-line type %1").arg(wavyLineType), xmlreader);
        }
    }
}

//---------------------------------------------------------
//   addBreath
//---------------------------------------------------------

static void addBreath(const Notation& notation, ChordRest* cr)
{
    const SymId breath = notation.symId();
    const Color color = Color::fromString(notation.attribute(u"color"));
    const String placement = notation.attribute(u"placement");

    Segment* const seg = cr->measure()->getSegment(SegmentType::Breath, cr->tick() + cr->ticks());
    Breath* const b = Factory::createBreath(seg);
    // b->setTrack(trk + voice); TODO check next line
    b->setTrack(cr->track());
    b->setSymId(breath);
    if (color.isValid()) {
        b->setColor(color);
    }
    b->setPlacement(placement == u"below" ? PlacementV::BELOW : PlacementV::ABOVE);
    b->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    seg->add(b);
}

//---------------------------------------------------------
//   addChordLine
//---------------------------------------------------------

static void addChordLine(const Notation& notation, Note* note,
                         MusicXmlLogger* logger, const XmlStreamReader* const xmlreader)
{
    const String chordLineType = notation.subType();
    const Color color = Color::fromString(notation.attribute(u"color"));
    if (!chordLineType.empty()) {
        if (note) {
            ChordLine* const chordline = Factory::createChordLine(note->chord());
            if (chordLineType == u"falloff") {
                chordline->setChordLineType(ChordLineType::FALL);
            } else if (chordLineType == u"doit") {
                chordline->setChordLineType(ChordLineType::DOIT);
            } else if (chordLineType == u"plop") {
                chordline->setChordLineType(ChordLineType::PLOP);
            } else if (chordLineType == u"scoop") {
                chordline->setChordLineType(ChordLineType::SCOOP);
            }
            if (color.isValid()) {
                chordline->setColor(color);
            }
            note->chord()->add(chordline);
        } else {
            logger->logError(String(u"no note for %1").arg(chordLineType), xmlreader);
        }
    }
}

//---------------------------------------------------------
//   notationWithAttributes
//---------------------------------------------------------

/**
 Helper function to create Notation with initial attributes.
 */

Notation Notation::notationWithAttributes(const String& name, const std::vector<XmlStreamReader::Attribute>& attributes,
                                          const String& parent, const SymId& symId)
{
    Notation notation = Notation(name, parent, symId);
    for (const XmlStreamReader::Attribute& attr : attributes) {
        notation.addAttribute(String::fromAscii(attr.name.ascii()), attr.value);
    }
    return notation;
}

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

void Notation::addAttribute(const String& name, const String& value)
{
    m_attributes.emplace(name, value);
}

//---------------------------------------------------------
//   attribute
//---------------------------------------------------------

String Notation::attribute(const String& name) const
{
    const auto it = m_attributes.find(name);
    return (it != m_attributes.end()) ? it->second : String();
}

//---------------------------------------------------------
//   print
//---------------------------------------------------------

String Notation::print() const
{
    String res = m_name;

    for (auto const& pair : m_attributes) {
        res += u" ";
        res += pair.first;
        res += u" ";
        res += pair.second;
    }

    if (!m_text.empty()) {
        res += u" ";
        res += m_text;
    }
    return res;
}

//---------------------------------------------------------
//   MusicXmlParserNotations
//---------------------------------------------------------

MusicXmlParserNotations::MusicXmlParserNotations(XmlStreamReader& e, Score* score, MusicXmlLogger* logger, MusicXmlParserPass2& pass2)
    : m_e(e), m_pass2(pass2), m_score(score), m_logger(logger)
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

void MusicXmlParserNotations::addError(const String& error)
{
    if (!error.empty()) {
        m_logger->logError(error, &m_e);
        m_errors += error;
    }
}

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXmlParserNotations::skipLogCurrElem()
{
    //_logger->logDebugInfo(String("skipping '%1'").arg(_e.name().toString()), &_e);
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

void MusicXmlParserNotations::parse()
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "arpeggiate") {
            arpeggio();
        } else if (m_e.name() == "articulations") {
            articulations();
        } else if (m_e.name() == "dynamics") {
            dynamics();
        } else if (m_e.name() == "fermata") {
            fermata();
        } else if (m_e.name() == "glissando") {
            glissandoSlide();
        } else if (m_e.name() == "hammer-on") {
            slur();
        } else if (m_e.name() == "non-arpeggiate") {
            m_arpeggioColor = Color::fromString(m_e.attribute("color"));
            m_arpeggioType = u"non-arpeggiate";
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "ornaments") {
            ornaments();
        } else if (m_e.name() == "pull-off") {
            slur();
        } else if (m_e.name() == "slur") {
            slur();
        } else if (m_e.name() == "slide") {
            glissandoSlide();
        } else if (m_e.name() == "technical") {
            technical();
        } else if (m_e.name() == "tied") {
            tied();
        } else if (m_e.name() == "tuplet") {
            tuplet();
        } else if (m_e.name() == "other-notation") {
            otherNotation();
        } else {
            skipLogCurrElem();
        }
    }

    /*
    for (const auto& notation : _notations) {
          LOGD("%s", muPrintable(notation.print()));
          }
     */

    addError(checkAtEndElement(m_e, u"notations"));
}

//---------------------------------------------------------
//   addNotation
//---------------------------------------------------------

void MusicXmlParserNotations::addNotation(const Notation& notation, ChordRest* const cr, Note* const note)
{
    if (notation.symId() != SymId::noSym) {
        if (notation.name() == u"breath") {
            addBreath(notation, cr);
        } else if (notation.name() == u"fermata") {
            addFermataToChord(notation, cr);

            // Terminate tempo line
            const InferredTempoLineStack& lines = m_pass2.getInferredTempoLine();
            terminateInferredLine(std::vector<TextLineBase*>(lines.begin(), lines.end()), cr->tick(), cr->track());
        } else if (notation.parent() == u"ornaments") {
            addTurnToChord(notation, cr);
        } else {
            addArticulationToChord(notation, cr);
        }
    } else if (notation.parent() == u"ornaments") {
        if (notation.name() == u"mordent" || notation.name() == u"inverted-mordent") {
            addMordentToChord(notation, cr);
        } else if (notation.name() == u"other-ornament") {
            addOtherOrnamentToChord(notation, cr);
        }
    } else if (notation.parent() == u"articulations") {
        if (note && notation.name() == u"chord-line") {
            addChordLine(notation, note, m_logger, &m_e);
        }
    } else {
        // LOGD("addNotation: notation has been skipped: %s %s", muPrintable(notation.name()), muPrintable(notation.parent()));
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

void MusicXmlParserNotations::addToScore(ChordRest* const cr, Note* const note, const int tick, SlurStack& slurs,
                                         Glissando* glissandi[MAX_NUMBER_LEVEL][2], MusicXmlSpannerMap& spanners,
                                         TrillStack& trills, MusicXmlTieMap& ties, std::vector<Note*>& unstartedTieNotes,
                                         std::vector<Note*>& unendedTieNotes, ArpeggioMap& arpMap,
                                         DelayedArpMap& delayedArps)
{
    addArpeggio(cr, m_arpeggioType, m_arpeggioNo, m_arpeggioColor, arpMap, delayedArps);
    addWavyLine(cr, Fraction::fromTicks(tick), m_wavyLineNo, m_wavyLineType, spanners, trills, m_logger, &m_e);

    for (const Notation& notation : m_notations) {
        if (notation.symId() != SymId::noSym) {
            addNotation(notation, cr, note);
        } else if (notation.name() == "slur" || notation.name() == "hammer-on" || notation.name() == "pull-off") {
            addSlur(notation, slurs, cr, tick, m_logger, &m_e);
        } else if (note && (notation.name() == "glissando" || notation.name() == "slide")) {
            addGlissandoSlide(notation, note, glissandi, spanners, m_logger, &m_e);
        } else if (note && notation.name() == "tied") {
            addTie(notation, note, cr->track(), ties, unstartedTieNotes, unendedTieNotes, m_logger, &m_e);
        } else if (note && notation.parent() == "technical") {
            addTechnical(notation, note);
        } else {
            addNotation(notation, cr, note);
        }
    }

    // more than one dynamic ???
    // LVIFIX: check import/export of <other-dynamics>unknown_text</...>
    // TODO remove duplicate code (see MusicXml::direction)
    for (const String& d : std::as_const(m_dynamicsList)) {
        Dynamic* dynamic = Factory::createDynamic(m_score->dummy()->segment());
        dynamic->setDynamicType(d);
        m_pass2.addElemOffset(dynamic, cr->track(), m_dynamicsPlacement, cr->measure(), Fraction::fromTicks(tick));
    }
}

//---------------------------------------------------------
//   stem
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/stem node.
 */

void MusicXmlParserPass2::stem(DirectionV& sd, bool& nost)
{
    // defaults
    sd = DirectionV::AUTO;
    nost = false;

    String s = m_e.readText();

    if (s == u"up") {
        sd = DirectionV::UP;
    } else if (s == u"down") {
        sd = DirectionV::DOWN;
    } else if (s == u"none") {
        nost = true;
    } else if (s == u"double") {
    } else {
        m_logger->logError(String(u"unknown stem direction %1").arg(s), &m_e);
    }
}

//---------------------------------------------------------
//   fermata
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/fermata node.
 Note: MusicXML common.mod: "An empty fermata element represents a normal fermata."
 */

void MusicXmlParserNotations::fermata()
{
    Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()), m_e.attributes(), u"notations");
    const String fermataText = m_e.readText();

    notation.setSymId(convertFermataToSymId(fermataText));
    notation.setText(fermataText);
    m_notations.push_back(notation);
}

//---------------------------------------------------------
//   tuplet
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations/tuplet node.
 */

void MusicXmlParserNotations::tuplet()
{
    const String tupletType       = m_e.attribute("type");
    const String tupletPlacement  = m_e.attribute("placement");
    const String tupletBracket    = m_e.attribute("bracket");
    const String tupletShowNumber = m_e.attribute("show-number");

    // ignore possible children (currently not supported)
    m_e.skipCurrentElement();

    if (tupletType == u"start") {
        m_tupletDesc.type = MusicXmlStartStop::START;
    } else if (tupletType == u"stop") {
        m_tupletDesc.type = MusicXmlStartStop::STOP;
    } else if (!tupletType.empty() && tupletType != u"start" && tupletType != u"stop") {
        m_logger->logError(String(u"unknown tuplet type '%1'").arg(tupletType), &m_e);
    }

    // set bracket, leave at default if unspecified
    if (tupletBracket == u"yes") {
        m_tupletDesc.bracket = TupletBracketType::SHOW_BRACKET;
    } else if (tupletBracket == u"no") {
        m_tupletDesc.bracket = TupletBracketType::SHOW_NO_BRACKET;
    }

    // set number, default is "actual" (=NumberType::SHOW_NUMBER)
    if (tupletShowNumber == u"both") {
        m_tupletDesc.shownumber = TupletNumberType::SHOW_RELATION;
    } else if (tupletShowNumber == u"none") {
        m_tupletDesc.shownumber = TupletNumberType::NO_TEXT;
    } else {
        m_tupletDesc.shownumber = TupletNumberType::SHOW_NUMBER;
    }

    // set number and bracket placement
    if (tupletPlacement == u"above") {
        m_tupletDesc.direction = DirectionV::UP;
    } else if (tupletPlacement == u"below") {
        m_tupletDesc.direction = DirectionV::DOWN;
    } else if (tupletPlacement.empty()) {
        // ignore
    } else {
        m_logger->logError(String(u"unknown tuplet placement: %1").arg(tupletPlacement), &m_e);
    }
}

void MusicXmlParserNotations::otherNotation()
{
    const String type = m_e.attribute("type");
    const String smufl = m_e.attribute("smufl");

    if (!smufl.empty()) {
        SymId id = SymNames::symIdByName(smufl, SymId::noSym);
        Notation notation = Notation::notationWithAttributes(String::fromAscii(m_e.name().ascii()),
                                                             m_e.attributes(), u"notations", id);
        m_notations.push_back(notation);
        m_e.skipCurrentElement();
    }
}

//---------------------------------------------------------
//   MusicXmlParserDirection
//---------------------------------------------------------

/**
 MusicXmlParserDirection constructor.
 */

MusicXmlParserDirection::MusicXmlParserDirection(XmlStreamReader& e,
                                                 Score* score,
                                                 MusicXmlParserPass1& pass1,
                                                 MusicXmlParserPass2& pass2,
                                                 MusicXmlLogger* logger)
    : m_e(e), m_score(score), m_pass1(pass1), m_pass2(pass2), m_logger(logger),
    m_hasDefaultY(false), m_defaultY(0.0), m_hasRelativeY(false), m_relativeY(0.0),
    m_tpoMetro(0), m_tpoSound(0), m_offset(0, 1)
{
    // nothing
}

bool HarmonyDesc::fretDiagramVisible() const
{
    return m_fretDiagram ? m_fretDiagram->visible() : false;
}

const ChordDescription* HarmonyDesc::descr() const
{
    return m_harmony && !m_harmony->chords().empty() ? m_harmony->chords().front()->descr() : nullptr;
}
}
