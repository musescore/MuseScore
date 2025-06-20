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

#include "engraving/dom/box.h"
#include "engraving/dom/bracketItem.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"
#include "engraving/rendering/score/tlayout.h"

#include "engraving/style/style.h"
#include "engraving/style/textstyle.h"

#include "engraving/types/symnames.h"

#include "importmusicxmllogger.h"
#include "importmusicxmlnoteduration.h"
#include "importmusicxmlpass1.h"
#include "../shared/musicxmltypes.h"
#include "../shared/musicxmlsupport.h"

#include "modularity/ioc.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static std::shared_ptr<mu::iex::musicxml::IMusicXmlConfiguration> configuration()
{
    return muse::modularity::globalIoc()->resolve<mu::iex::musicxml::IMusicXmlConfiguration>("iex_musicxml");
}

static std::shared_ptr<mu::engraving::IEngravingFontsProvider> engravingFonts()
{
    return muse::modularity::globalIoc()->resolve<mu::engraving::IEngravingFontsProvider>("iex_musicxml");
}

static bool musicXmlImportBreaks()
{
    auto conf = configuration();
    return conf ? conf->importBreaks() : true;
}

static bool musicXmlImportLayout()
{
    auto conf = configuration();
    return conf ? conf->importLayout() : true;
}

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of note start/stop times in a voice in all staves.
*/

class NoteList
{
public:
    NoteList();
    void addNote(const int startTick, const int endTick, const size_t staff);
    void dump(const int& voice) const;
    bool stavesOverlap(const int staff1, const int staff2) const;
    bool anyStaffOverlaps() const;
private:
    std::vector<StartStopList> _staffNoteLists;   // The note start/stop times in all staves
    bool notesOverlap(const StartStop& n1, const StartStop& n2) const;
};

//---------------------------------------------------------
//   VoiceOverlapDetector
//---------------------------------------------------------

/**
 Detect overlap in a voice, which is when a voice has two or more notes
 active at the same time. In theory this should not happen, as voices
 only move forward in time, but Sibelius 7 reuses voice numbers in multi-
 staff parts, which leads to overlap.

 Current implementation does not detect voice overlap within a staff,
 but only between staves.
*/

class VoiceOverlapDetector
{
public:
    VoiceOverlapDetector();
    void addNote(const int startTick, const int endTick, const int& voice, const int staff);
    void dump() const;
    void newMeasure();
    bool stavesOverlap(const int& voice) const;
private:
    std::map<int, NoteList> _noteLists;   // The notelists for all the voices
};

NoteList::NoteList()
{
    _staffNoteLists.reserve(MAX_STAVES);
    for (int i = 0; i < MAX_STAVES; ++i) {
        _staffNoteLists.push_back(StartStopList());
    }
}

void NoteList::addNote(const int startTick, const int endTick, const size_t staff)
{
    if (staff < _staffNoteLists.size()) {
        _staffNoteLists[staff].push_back(StartStop(startTick, endTick));
    }
}

void NoteList::dump(const int& voice) const
{
    // dump contents
    for (int i = 0; i < MAX_STAVES; ++i) {
        printf("voice %d staff %d:", voice, i);
        for (size_t j = 0; j < _staffNoteLists.at(i).size(); ++j) {
            printf(" %d-%d", _staffNoteLists.at(i).at(j).first, _staffNoteLists.at(i).at(j).second);
        }
        printf("\n");
    }
    // show overlap
    printf("overlap voice %d:", voice);
    for (int i = 0; i < MAX_STAVES - 1; ++i) {
        for (int j = i + 1; j < MAX_STAVES; ++j) {
            stavesOverlap(i, j);
        }
    }
    printf("\n");
}

/**
 Determine if notes n1 and n2 overlap.
 This is NOT the case if
 - n1 starts when or after n2 stops
 - or n2 starts when or after n1 stops
 */

bool NoteList::notesOverlap(const StartStop& n1, const StartStop& n2) const
{
    return !(n1.first >= n2.second || n1.second <= n2.first);
}

/**
 Determine if any note in staff1 and staff2 overlaps.
 */

bool NoteList::stavesOverlap(const int staff1, const int staff2) const
{
    for (size_t i = 0; i < _staffNoteLists.at(staff1).size(); ++i) {
        for (size_t j = 0; j < _staffNoteLists.at(staff2).size(); ++j) {
            if (notesOverlap(_staffNoteLists.at(staff1).at(i), _staffNoteLists.at(staff2).at(j))) {
                //printf(" %d-%d", staff1, staff2);
                return true;
            }
        }
    }
    return false;
}

/**
 Determine if any note in any staff overlaps.
 */

bool NoteList::anyStaffOverlaps() const
{
    for (int i = 0; i < MAX_STAVES - 1; ++i) {
        for (int j = i + 1; j < MAX_STAVES; ++j) {
            if (stavesOverlap(i, j)) {
                return true;
            }
        }
    }
    return false;
}

VoiceOverlapDetector::VoiceOverlapDetector()
{
    // LOGD("VoiceOverlapDetector::VoiceOverlapDetector(staves %d)", MAX_STAVES);
}

void VoiceOverlapDetector::addNote(const int startTick, const int endTick, const int& voice, const int staff)
{
    // if necessary, create the note list for voice
    if (!muse::contains(_noteLists, voice)) {
        _noteLists.insert({ voice, NoteList() });
    }
    _noteLists[voice].addNote(startTick, endTick, staff);
}

void VoiceOverlapDetector::dump() const
{
    // LOGD("VoiceOverlapDetector::dump()");
    for (auto& p : _noteLists) {
        p.second.dump(p.first);
    }
}

void VoiceOverlapDetector::newMeasure()
{
    // LOGD("VoiceOverlapDetector::newMeasure()");
    _noteLists.clear();
}

bool VoiceOverlapDetector::stavesOverlap(const int& voice) const
{
    if (muse::contains(_noteLists, voice)) {
        return _noteLists.at(voice).anyStaffOverlaps();
    } else {
        return false;
    }
}

//---------------------------------------------------------
//   allocateStaves
//---------------------------------------------------------

/**
 Allocate MuseScore staff to MusicXML voices.
 For each staff, allocate at most VOICES voices to the staff.
 */

// for regular (non-overlapping) voices:
// 1) assign voice to a staff (allocateStaves)
// 2) assign voice numbers (allocateVoices)
// due to cross-staving, it is not a priori clear to which staff
// a voice has to be assigned
// allocate ordered by number of chordrests in the MusicXML voice
//
// for overlapping voices:
// 1) assign voice to staves it is found in (allocateStaves)
// 2) assign voice numbers (allocateVoices)

static void allocateStaves(VoiceList& vcLst)
{
    // initialize
    int voicesAllocated[MAX_STAVES];   // number of voices allocated on each staff
    for (int i = 0; i < MAX_STAVES; ++i) {
        voicesAllocated[i] = 0;
    }

    // handle regular (non-overlapping) voices
    // note: outer loop executed vcLst.size() times, as each inner loop handles exactly one item
    for (size_t i = 0; i < vcLst.size(); ++i) {
        // find the regular voice containing the highest number of chords and rests that has not been handled yet
        int max = 0;
        int key = -1;
        for (VoiceList::const_iterator j = vcLst.cbegin(); j != vcLst.cend(); ++j) {
            if (!j->second.overlaps() && j->second.numberChordRests() > max && j->second.staff() == -1) {
                max = j->second.numberChordRests();
                key = j->first;
            }
        }
        if (key > 0) {
            int prefSt = muse::value(vcLst, key).preferredStaff();
            if (voicesAllocated[prefSt] < static_cast<int>(VOICES)) {
                vcLst[key].setStaff(prefSt);
                voicesAllocated[prefSt]++;
            } else {
                // out of voices: mark as used but not allocated
                vcLst[key].setStaff(-2);
            }
        }
    }

    // handle overlapping voices
    // for every staff allocate remaining voices (if space allows)
    // the ones with the highest number of chords and rests get allocated first
    for (int h = 0; h < MAX_STAVES; ++h) {
        // note: middle loop executed vcLst.size() times, as each inner loop handles exactly one item
        for (size_t i = 0; i < vcLst.size(); ++i) {
            // find the overlapping voice containing the highest number of chords and rests that has not been handled yet
            int max = 0;
            int key = -1;
            for (VoiceList::const_iterator j = vcLst.cbegin(); j != vcLst.cend(); ++j) {
                if (j->second.overlaps() && j->second.numberChordRests(h) > max && j->second.staffAlloc(h) == -1) {
                    max = j->second.numberChordRests(h);
                    key = j->first;
                }
            }
            if (key > 0) {
                int prefSt = h;
                if (voicesAllocated[prefSt] < static_cast<int>(VOICES)) {
                    vcLst[key].setStaffAlloc(prefSt, 1);
                    voicesAllocated[prefSt]++;
                } else {
                    // out of voices: mark as used but not allocated
                    vcLst[key].setStaffAlloc(prefSt, -2);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   allocateVoices
//---------------------------------------------------------

/**
 Allocate MuseScore voice to MusicXML voices.
 For each staff, the voices are number 1, 2, 3, 4
 in the same order they are numbered in the MusicXML file.
 */

static void allocateVoices(VoiceList& vcLst)
{
    int nextVoice[MAX_STAVES];   // number of voices allocated on each staff
    for (int i = 0; i < MAX_STAVES; ++i) {
        nextVoice[i] = 0;
    }
    // handle regular (non-overlapping) voices
    // a voice is allocated on one specific staff
    for (VoiceList::const_iterator i = vcLst.cbegin(); i != vcLst.cend(); ++i) {
        int staff = i->second.staff();
        int key   = i->first;
        if (staff >= 0) {
            vcLst[key].setVoice(nextVoice[staff]);
            nextVoice[staff]++;
        }
    }
    // handle overlapping voices
    // each voice may be in every staff
    for (VoiceList::const_iterator i = vcLst.cbegin(); i != vcLst.cend(); ++i) {
        for (int j = 0; j < MAX_STAVES; ++j) {
            int staffAlloc = i->second.staffAlloc(j);
            int key = i->first;
            if (staffAlloc >= 0) {
                vcLst[key].setVoice(j, nextVoice[j]);
                nextVoice[j]++;
            }
        }
    }
}

//---------------------------------------------------------
//   copyOverlapData
//---------------------------------------------------------

/**
 Copy the overlap data from the overlap detector to the voice list.
 */

static void copyOverlapData(VoiceOverlapDetector& vod, VoiceList& vcLst)
{
    for (VoiceList::const_iterator i = vcLst.cbegin(); i != vcLst.cend(); ++i) {
        int key = i->first;
        if (vod.stavesOverlap(key)) {
            vcLst[key].setOverlap(true);
        }
    }
}

//---------------------------------------------------------
//   MusicXmlParserPass1
//---------------------------------------------------------

MusicXmlParserPass1::MusicXmlParserPass1(Score* score, MusicXmlLogger* logger)
    : m_divs(0), m_score(score), m_logger(logger), m_hasBeamingInfo(false)
{
    // nothing
}

//---------------------------------------------------------
//   addError
//---------------------------------------------------------

void MusicXmlParserPass1::addError(const String& error)
{
    if (!error.empty()) {
        m_logger->logError(error, &m_e);
        m_errors += errorStringWithLocation(m_e.lineNumber(), m_e.columnNumber(), error) + '\n';
    }
}

void MusicXmlParserPass1::setExporterSoftware(String& exporter)
{
    if (exporter.contains(u"sibelius")) {
        if (exporter.contains(u"dolet 6")) {
            m_exporterSoftware = MusicXmlExporterSoftware::DOLET6;
        } else if (exporter.contains(u"dolet 8")) {
            m_exporterSoftware = MusicXmlExporterSoftware::DOLET8;
        } else {
            m_exporterSoftware = MusicXmlExporterSoftware::SIBELIUS;
        }
    } else if (exporter.contains(u"dorico")) {
        m_exporterSoftware = MusicXmlExporterSoftware::DORICO;
    } else if (exporter.contains(u"finale")) {
        m_exporterSoftware = MusicXmlExporterSoftware::FINALE;
    } else if (exporter.contains(u"noteflight")) {
        m_exporterSoftware = MusicXmlExporterSoftware::NOTEFLIGHT;
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

void MusicXmlParserPass1::initPartState(const String& /* partId */)
{
    m_timeSigDura = Fraction(0, 0);         // invalid
    m_octaveShifts.clear();
}

//---------------------------------------------------------
//   determineMeasureLength
//---------------------------------------------------------

/**
 Determine the length in ticks of each measure in all parts.
 Return false on error.
 */

bool MusicXmlParserPass1::determineMeasureLength(std::vector<Fraction>& ml) const
{
    ml.clear();

    // determine number of measures: max number of measures in any part
    size_t nMeasures = 0;
    for (const MusicXmlPart& part : muse::values(m_parts)) {
        if (part.nMeasures() > nMeasures) {
            nMeasures = part.nMeasures();
        }
    }

    // determine max length of a specific measure in all parts
    for (size_t i = 0; i < nMeasures; ++i) {
        Fraction maxMeasDur;
        for (const MusicXmlPart& part : muse::values(m_parts)) {
            if (i < part.nMeasures()) {
                Fraction measDurPartJ = part.measureDuration(i);
                if (measDurPartJ > maxMeasDur) {
                    maxMeasDur = measDurPartJ;
                }
            }
        }
        //LOGD("determineMeasureLength() measure %d %s (%d)", i, muPrintable(maxMeasDur.print()), maxMeasDur.ticks());
        ml.push_back(maxMeasDur);
    }
    return true;
}

//---------------------------------------------------------
//   getVoiceList
//---------------------------------------------------------

/**
 Get the VoiceList for part \a id.
 Return an empty VoiceList on error.
 */

VoiceList MusicXmlParserPass1::getVoiceList(const String& id) const
{
    if (muse::contains(m_parts, id)) {
        return m_parts.at(id).voicelist;
    }
    return VoiceList();
}

//---------------------------------------------------------
//   getInstrList
//---------------------------------------------------------

/**
 Get the MusicXmlInstrList for part \a id.
 Return an empty MusicXmlInstrList on error.
 */

MusicXmlInstrList MusicXmlParserPass1::getInstrList(const String& id) const
{
    if (muse::contains(m_parts, id)) {
        return m_parts.at(id)._instrList;
    }
    return MusicXmlInstrList();
}

//---------------------------------------------------------
//   getIntervals
//---------------------------------------------------------

/**
 Get the MusicXmlIntervalList for part \a id.
 Return an empty MusicXmlIntervalList on error.
 */

MusicXmlIntervalList MusicXmlParserPass1::getIntervals(const String& id) const
{
    if (muse::contains(m_parts, id)) {
        return m_parts.at(id)._intervals;
    }
    return MusicXmlIntervalList();
}

//---------------------------------------------------------
//   determineMeasureLength
//---------------------------------------------------------

/**
 Set default notehead, line and stem direction
 for instrument \a instrId in part \a id.
 Called from pass 2, notehead, line and stemDirection are not read in pass 1.
 */

void MusicXmlParserPass1::setDrumsetDefault(const String& id,
                                            const String& instrId,
                                            const NoteHeadGroup hg,
                                            const int line,
                                            const DirectionV sd)
{
    if (muse::contains(m_instruments, id) && muse::contains(m_instruments.at(id), instrId)) {
        m_instruments[id][instrId].notehead = hg;
        m_instruments[id][instrId].line = line;
        m_instruments[id][instrId].stemDirection = sd;
    }
}

//---------------------------------------------------------
//   determineStaffMoveVoice
//---------------------------------------------------------

/**
 For part \a id, determine MuseScore (ms) staffmove, track and voice from MusicXML (mx) staff and voice
 MusicXML staff is 0 for the first staff, 1 for the second.
 Note: track is the first track of the ms staff in the score, add ms voice for elements in a voice
 Return true if OK, false on error
 TODO: finalize
 */

bool MusicXmlParserPass1::determineStaffMoveVoice(const String& id, const int mxStaff, const int& mxVoice,
                                                  int& msMove, int& msTrack, int& msVoice) const
{
    VoiceList voicelist = getVoiceList(id);
    msMove = 0;   // TODO
    msTrack = 0;   // TODO
    msVoice = 0;   // TODO

    // MusicXML voices are counted for all staves of an
    // instrument. They are not limited. In mscore voices are associated
    // with a staff. Every staff can have at most VOICES voices.

    // The following lines map MusicXML voices to mscore voices.
    // If a voice crosses two staves, this is expressed with the
    // "move" parameter in mscore.

    // MusicXML voices are unique within a part, but not across parts.

    //LOGD("voice mapper before: voice='%s' staff=%d", muPrintable(mxVoice), mxStaff);
    int s;   // staff mapped by voice mapper
    int v;   // voice mapped by voice mapper
    if (muse::value(voicelist, mxVoice).overlaps()) {
        // for overlapping voices, the staff does not change
        // and the voice is mapped and staff-dependent
        s = mxStaff;
        v = muse::value(voicelist, mxVoice).voice(s);
    } else {
        // for non-overlapping voices, both staff and voice are
        // set by the voice mapper
        s = muse::value(voicelist, mxVoice).staff();
        v = muse::value(voicelist, mxVoice).voice();
    }

    //LOGD("voice mapper mapped: s=%d v=%d", s, v);
    if (s < 0 || v < 0) {
        LOGD("too many voices (staff=%d voice='%d' -> s=%d v=%d)",
             mxStaff + 1, mxVoice, s, v);
        return false;
    }

    msMove  = mxStaff - s;
    msVoice = v;

    // make score-relative instead on part-relative
    Part* part = muse::value(m_partMap, id);
    IF_ASSERT_FAILED(part) {
        return false;
    }
    staff_idx_t scoreRelStaff = m_score->staffIdx(part);   // zero-based number of parts first staff in the score
    msTrack = static_cast<int>((scoreRelStaff + s) * VOICES);

    //LOGD("voice mapper after: scoreRelStaff=%d partRelStaff=%d msMove=%d msTrack=%d msVoice=%d",
    //       scoreRelStaff, s, msMove, msTrack, msVoice);
    // note: relStaff is the staff number relative to the parts first staff
    //       voice is the voice number in the staff

    return true;
}

//---------------------------------------------------------
//   hasPart
//---------------------------------------------------------

/**
 Check if part \a id is found.
 */

bool MusicXmlParserPass1::hasPart(const String& id) const
{
    return muse::contains(m_parts, id);
}

//---------------------------------------------------------
//   trackForPart
//---------------------------------------------------------

/**
 Return the (score relative) track number for the first staff of part \a id.
 */

track_idx_t MusicXmlParserPass1::trackForPart(const String& id) const
{
    Part* part = muse::value(m_partMap, id);
    IF_ASSERT_FAILED(part) {
        return muse::nidx;
    }
    staff_idx_t scoreRelStaff = m_score->staffIdx(part);   // zero-based number of parts first staff in the score
    return scoreRelStaff * VOICES;
}

//---------------------------------------------------------
//   getMeasureStart
//---------------------------------------------------------

/**
 Return the measure start time for measure \a i.
 */

Fraction MusicXmlParserPass1::getMeasureStart(const size_t i) const
{
    if (i < m_measureStart.size()) {
        return m_measureStart.at(i);
    } else {
        return Fraction(0, 0);           // invalid
    }
}

//---------------------------------------------------------
//   octaveShift
//---------------------------------------------------------

/**
 Return the octave shift for part \a id in \a staff at \a f.
 */

int MusicXmlParserPass1::octaveShift(const String& id, const staff_idx_t staff, const Fraction& f) const
{
    if (muse::contains(m_parts, id)) {
        return m_parts.at(id).octaveShift(staff, f);
    }

    return 0;
}

//---------------------------------------------------------
//   skipLogCurrElem
//---------------------------------------------------------

/**
 Skip the current element, log debug as info.
 */

void MusicXmlParserPass1::skipLogCurrElem()
{
    m_logger->logDebugInfo(String(u"skipping '%1'").arg(String::fromAscii(m_e.name().ascii())), &m_e);
    m_e.skipCurrentElement();
}

//---------------------------------------------------------
//   addBreak
//---------------------------------------------------------

static void addBreak(Score* const, MeasureBase* const mb, const LayoutBreakType type)
{
    LayoutBreak* lb = Factory::createLayoutBreak(mb);
    lb->setLayoutBreakType(type);
    mb->add(lb);
}

//---------------------------------------------------------
//   addBreakToPreviousMeasureBase
//---------------------------------------------------------

static void addBreakToPreviousMeasureBase(Score* const score, MeasureBase* const mb, const LayoutBreakType type)
{
    MeasureBase* const pm = mb->prev();
    if (pm && musicXmlImportBreaks()) {
        addBreak(score, pm, type);
    }
}

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

/**
 Add text \a strTxt to VBox \a vbx using Tid \a stl.
 */

static void addText(VBox* vbx, Score*, const String& strTxt, const TextStyleType stl)
{
    if (!strTxt.isEmpty()) {
        Text* text = Factory::createText(vbx, stl);
        text->setXmlText(strTxt.trimmed());
        vbx->add(text);
    }
}

static bool overrideTextStyleForComposer(const String& creditString)
{
    // HACK: check if the string is likely to contain composer credit, so the proper text style can be applied.
    // TODO: introduce a flag to decide if we want to do this or not.
    static const std::wregex re(L"\\s*((Words|Music|Lyrics).*)*by\\s+([A-Z][a-zA-Zö'’-]+\\s[A-Z][a-zA-Zös'’-]+.*)+");
    return creditString.contains(re);
}

//---------------------------------------------------------
//   addText2
//---------------------------------------------------------

static void scaleTitle(Score* score, Text* text);

/**
 Add text \a strTxt to VBox \a vbx using Tid \a stl.
 Also sets Align and Yoff.
 */

static void addText2(VBox* vbx, Score* score, const String& strTxt, const TextStyleType stl, const Align align, const double yoffs)
{
    if (stl != TextStyleType::COMPOSER && overrideTextStyleForComposer(strTxt)) {
        // HACK: in some Dolet 8 files the composer is written as a subtitle, which leads to stupid formatting.
        // This overrides the formatting and introduces proper composer text
        Text* text = Factory::createText(vbx, TextStyleType::COMPOSER);
        text->setXmlText(strTxt.trimmed());
        text->setOffset(muse::PointF(0.0, yoffs));
        text->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        vbx->add(text);
    } else if (!strTxt.isEmpty()) {
        Text* text = Factory::createText(vbx, stl);
        text->setXmlText(strTxt.trimmed());
        text->setAlign(align);
        text->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
        text->setOffset(muse::PointF(0.0, yoffs));
        text->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        vbx->add(text);
        if (stl == TextStyleType::TITLE) {
            scaleTitle(score, text);
        }
    }
}

//---------------------------------------------------------
//   findYMinYMaxInWords
//---------------------------------------------------------

static void findYMinYMaxInWords(const std::vector<const CreditWords*>& words, int& miny, int& maxy)
{
    miny = 0;
    maxy = 0;

    if (words.empty()) {
        return;
    }

    miny = words.at(0)->defaultY;
    maxy = words.at(0)->defaultY;
    for (const CreditWords* w : words) {
        if (w->defaultY < miny) {
            miny = w->defaultY;
        }
        if (w->defaultY > maxy) {
            maxy = w->defaultY;
        }
    }
}

//---------------------------------------------------------
//   alignForCreditWords
//---------------------------------------------------------

static Align alignForCreditWords(const CreditWords* const w, const int pageWidth, const TextStyleType tid)
{
    Align align = AlignH::LEFT;
    if (w->defaultX > (pageWidth / 3)) {
        if (w->defaultX < (2 * pageWidth / 3)) {
            align = AlignH::HCENTER;
        } else {
            align = AlignH::RIGHT;
        }
    }
    if (tid == TextStyleType::COMPOSER) {
        align.vertical = AlignV::BOTTOM;
    }
    return align;
}

//---------------------------------------------------------
//   creditWordTypeToTid
//---------------------------------------------------------

static TextStyleType creditWordTypeToTid(const String& type)
{
    if (type == u"composer") {
        return TextStyleType::COMPOSER;
    } else if (type == u"lyricist") {
        return TextStyleType::LYRICIST;
    }
    /*
    else if (type == "page number")
          return TextStyleName::;
    else if (type == "rights")
          return TextStyleName::;
     */
    else if (type == u"subtitle") {
        return TextStyleType::SUBTITLE;
    } else if (type == u"title") {
        return TextStyleType::TITLE;
    } else {
        return TextStyleType::DEFAULT;
    }
}

//---------------------------------------------------------
//   creditWordTypeGuess
//---------------------------------------------------------

static TextStyleType creditWordTypeGuess(const CreditWords* const word, std::vector<const CreditWords*>& words, const int pageWidth)
{
    const double pw1 = pageWidth / 3;
    const double pw2 = pageWidth * 2 / 3;
    const double defx = word->defaultX;
    // composer is in the right column
    if (pw2 < defx) {
        // found composer
        return TextStyleType::COMPOSER;
    }
    // poet is in the left column
    else if (defx < pw1) {
        // found poet/lyricist
        return TextStyleType::LYRICIST;
    }
    // title is in the middle column
    else {
        // if another word in the middle column has a larger font size, this word is not the title
        for (const CreditWords* w : words) {
            if (w == word) {
                continue;                 // it's me
            }
            if (w->defaultX < pw1 || pw2 < w->defaultX) {
                continue;                 // it's not in the middle column
            }
            if (word->fontSize < w->fontSize) {
                return TextStyleType::SUBTITLE;                  // word does not have the largest font size, assume subtitle
            }
        }
        return TextStyleType::TITLE;                // no better title candidate found
    }
}

//---------------------------------------------------------
//   tidForCreditWords
//---------------------------------------------------------

static TextStyleType tidForCreditWords(const CreditWords* const word, std::vector<const CreditWords*>& words, const int pageWidth)
{
    const TextStyleType tid = creditWordTypeToTid(word->type);
    if (tid != TextStyleType::DEFAULT) {
        // type recognized, done
        return tid;
    } else {
        // type not recognized, guess
        return creditWordTypeGuess(word, words, pageWidth);
    }
}

//---------------------------------------------------------
//   createAndAddVBoxForCreditWords
//---------------------------------------------------------

VBox* MusicXmlParserPass1::createAndAddVBoxForCreditWords(Score* score, Fraction tick)
{
    VBox* vbox = Factory::createTitleVBox(score->dummy()->system());
    vbox->setTick(tick);
    score->measures()->append(vbox);
    return vbox;
}

//---------------------------------------------------------
//   mustAddWordToVbox
//---------------------------------------------------------

// determine if specific types of credit words must be added: do not add copyright and page number,
// as these typically conflict with MuseScore's style and/or layout

static bool mustAddWordToVbox(const String& creditType)
{
    return creditType != u"rights" && creditType != u"page number";
}

//---------------------------------------------------------
//   isLikelySubtitleText
//---------------------------------------------------------

bool isLikelySubtitleText(const String& text, const bool caseInsensitive = true)
{
    std::regex::flag_type caseOption = caseInsensitive ? std::regex::icase : std::regex::ECMAScript;
    return text.trimmed().contains(std::wregex(L"^[Ff]rom\\s+(?!$)", caseOption))
           || text.trimmed().contains(std::wregex(L"^Theme from\\s+(?!$)", caseOption))
           || text.trimmed().contains(std::wregex(L"(((Op\\.?\\s?\\d+)|(No\\.?\\s?\\d+))\\s?)+", caseOption))
           || text.trimmed().contains(std::wregex(L"\\(.*[Ff]rom\\s.*\\)", caseOption));
}

//---------------------------------------------------------
//   isLikelyCreditText
//---------------------------------------------------------

bool isLikelyCreditText(const String& text, const bool caseInsensitive = true)
{
    std::regex::flag_type caseOption = caseInsensitive ? std::regex::icase : std::regex::ECMAScript;
    return text.trimmed().contains(std::wregex(L"^((Words|Music|Lyrics|Composed),?(\\sand|\\s&amp;|\\s&)?\\s)*[Bb]y\\s+(?!$)", caseOption))
           || text.trimmed().contains(std::wregex(L"^(Traditional|Trad\\.)", caseOption));
}

static bool isLikelyRightsText(const String& text)
{
    return text.contains(u"all rights reserved", CaseSensitivity::CaseInsensitive) || text.contains(u"\u00A9");
}

//---------------------------------------------------------
//   inferSubTitleFromTitle
//---------------------------------------------------------

// Extracts a likely subtitle from the title string
// Returns the inferred subtitle

static void inferFromTitle(String& title, String& inferredSubtitle, String& inferredCredits)
{
    StringList subtitleLines;
    StringList creditLines;
    StringList titleLines = title.split(std::regex("\\n"));
    size_t nrOfTitleLines = titleLines.size();
    if (nrOfTitleLines == 1) {
        return;
    }
    for (size_t i = nrOfTitleLines; i > 0; --i) {
        String line = titleLines[i - 1];
        if (isLikelyCreditText(line, true)) {
            creditLines.insert(0, line);
            titleLines.erase(titleLines.begin() + i - 1);
        } else if (isLikelySubtitleText(line, true)) {
            subtitleLines.insert(0, line);
            titleLines.erase(titleLines.begin() + i - 1);
        }
    }
    title = titleLines.join(u"\n");
    inferredSubtitle = subtitleLines.join(u"\n");
    inferredCredits = creditLines.join(u"\n");
}

//---------------------------------------------------------
//   addCreditWords
//---------------------------------------------------------

static VBox* addCreditWords(Score* score, const CreditWordsList& crWords, const Size& pageSize,
                            Fraction tick, const bool isSibeliusScore)
{
    VBox* vbox = nullptr;
    const bool top = tick.isZero();

    std::vector<const CreditWords*> headerWords;
    std::vector<const CreditWords*> footerWords;
    for (const CreditWords* w : crWords) {
        if (w->page == 1) {
            if (w->defaultY > (pageSize.height() / 2)) {
                headerWords.push_back(w);
            } else {
                footerWords.push_back(w);
            }
        }
    }

    std::vector<const CreditWords*> words;
    // if there are more credit words in the footer than in header,
    // swap header and footer, assuming this will result in a vertical
    // frame with the title on top of the page.
    // Sibelius (direct export) typically exports no header
    // and puts the title etc. in the footer
    const bool doSwap = footerWords.size() > headerWords.size() && isSibeliusScore;
    if (top) {
        words = doSwap ? footerWords : headerWords;
    } else {
        words = doSwap ? headerWords : footerWords;
    }

    int miny = 0;
    int maxy = 0;
    findYMinYMaxInWords(words, miny, maxy);

    for (const CreditWords* w : words) {
        if (mustAddWordToVbox(w->type)) {
            const TextStyleType tid = top ? tidForCreditWords(w, words, pageSize.width()) : TextStyleType::DEFAULT;
            const Align align = alignForCreditWords(w, pageSize.width(), tid);
            double yoffs = tid == TextStyleType::COMPOSER ? 0.0 : (maxy - w->defaultY) * score->style().spatium() / 10;
            if (!vbox) {
                Fraction vBoxTick = top ? Fraction(0, 1) : tick;
                vbox = MusicXmlParserPass1::createAndAddVBoxForCreditWords(score, vBoxTick);
            }
            addText2(vbox, score, w->words, tid, align, yoffs);
        } else if (w->type == u"rights" && score->metaTag(u"copyright").empty()) {
            // Add rights to footer, not a vbox
            static const std::regex tagRe("(<.*?>)");
            String rights = w->words;
            rights.remove(tagRe);
            score->setMetaTag(u"copyright", rights);
        }
    }

    return vbox;
}

//---------------------------------------------------------
//   createMeasuresAndVboxes
//---------------------------------------------------------

void MusicXmlParserPass1::createDefaultHeader(Score* score)
{
    String strTitle;
    String strSubTitle;
    String inferredStrSubTitle;
    String inferredStrComposer;
    String strComposer;
    String strLyricist;
    String strTranslator;

    if (!(score->metaTag(u"movementTitle").isEmpty() && score->metaTag(u"workTitle").isEmpty())) {
        strTitle = score->metaTag(u"movementTitle");
        if (strTitle.isEmpty()) {
            strTitle = score->metaTag(u"workTitle");
        }
        inferFromTitle(strTitle, inferredStrSubTitle, inferredStrComposer);
    }
    if (!(score->metaTag(u"movementNumber").isEmpty() && score->metaTag(u"workNumber").isEmpty())) {
        strSubTitle = score->metaTag(u"movementNumber");
        if (strSubTitle.isEmpty()) {
            strSubTitle = score->metaTag(u"workNumber");
        }
    }
    if (!inferredStrSubTitle.isEmpty()) {
        strSubTitle = inferredStrSubTitle;
        m_hasInferredHeaderText = true;
    }
    String metaComposer = score->metaTag(u"composer");
    String metaLyricist = score->metaTag(u"lyricist");
    String metaTranslator = score->metaTag(u"translator");
    if (!metaComposer.isEmpty()) {
        strComposer = metaComposer;
    }
    if (!inferredStrComposer.isEmpty()) {
        strComposer = inferredStrComposer;
        m_hasInferredHeaderText = true;
    }
    if (metaLyricist.isEmpty()) {
        metaLyricist = score->metaTag(u"poet");
    }
    if (!metaLyricist.isEmpty()) {
        strLyricist = metaLyricist;
    }
    if (!metaTranslator.isEmpty()) {
        strTranslator = metaTranslator;
    }

    VBox* const vbox = MusicXmlParserPass1::createAndAddVBoxForCreditWords(score, Fraction(0, 1));
    vbox->setExcludeFromOtherParts(false);
    addText(vbox, score, strTitle.toXmlEscaped(),      TextStyleType::TITLE);
    addText(vbox, score, strSubTitle.toXmlEscaped(),   TextStyleType::SUBTITLE);
    addText(vbox, score, strComposer.toXmlEscaped(),   TextStyleType::COMPOSER);
    addText(vbox, score, strLyricist.toXmlEscaped(),   TextStyleType::LYRICIST);
    addText(vbox, score, strTranslator.toXmlEscaped(), TextStyleType::TRANSLATOR);
}

//---------------------------------------------------------
//   createMeasuresAndVboxes
//---------------------------------------------------------

/**
 Create required measures with correct number, start tick and length for Score \a score.
 */

void MusicXmlParserPass1::createMeasuresAndVboxes(Score* score,
                                                  const std::vector<Fraction>& ml,
                                                  const std::vector<Fraction>& ms,
                                                  const std::set<int>& systemStartMeasureNrs,
                                                  const std::set<int>& pageStartMeasureNrs,
                                                  const CreditWordsList& crWords,
                                                  const Size& pageSize)
{
    if (crWords.empty()) {
        createDefaultHeader(score);
    }

    int pageNr = 0;
    for (size_t i = 0; i < ml.size(); ++i) {
        VBox* vbox = nullptr;

        // add a header vbox if the this measure is the first in the score or the first on a new page
        if (pageStartMeasureNrs.count(int(i)) || i == 0) {
            ++pageNr;

            if (pageNr == 1) {
                vbox = addCreditWords(score, crWords, pageSize, Fraction(0, 0), sibOrDolet());
                if (i == 0 && vbox) {
                    vbox->setExcludeFromOtherParts(false);
                }
            }
        }

        // create and add the measure
        Measure* measure  = Factory::createMeasure(score->dummy()->system());
        measure->setTick(ms.at(i));
        measure->setTicks(ml.at(i));
        measure->setNo(int(i));
        score->measures()->append(measure);

        // add break to previous measure or vbox
        MeasureBase* mb = vbox;
        if (!mb) {
            mb = measure;
        }
        if (pageStartMeasureNrs.count(int(i))) {
            addBreakToPreviousMeasureBase(score, mb, LayoutBreakType::PAGE);
        } else if (systemStartMeasureNrs.count(int(i))) {
            addBreakToPreviousMeasureBase(score, mb, LayoutBreakType::LINE);
        }

        // add a footer vbox if the next measure is on a new page or end of score has been reached
        if ((pageStartMeasureNrs.count(int(i + 1)) || i == (ml.size() - 1)) && pageNr == 1) {
            addCreditWords(score, crWords, pageSize, measure->tick() + measure->ticks(), sibOrDolet());
        }
    }
}

bool MusicXmlParserPass1::sibOrDolet() const
{
    return m_exporterSoftware == MusicXmlExporterSoftware::SIBELIUS || dolet();
}

bool MusicXmlParserPass1::dolet() const
{
    return m_exporterSoftware == MusicXmlExporterSoftware::DOLET6 || m_exporterSoftware == MusicXmlExporterSoftware::DOLET8;
}

//---------------------------------------------------------
//   determineMeasureStart
//---------------------------------------------------------

/**
 Determine the start ticks of each measure
 i.e. the sum of all previous measures length
 or start tick measure equals start tick previous measure plus length previous measure
 */

static void determineMeasureStart(const std::vector<Fraction>& ml, std::vector<Fraction>& ms)
{
    ms.resize(ml.size());
    if (!(ms.size() > 0)) {
        return;      // no parts read
    }
    // first measure starts at t = 0
    ms[0] = Fraction(0, 1);
    // all others start at start time previous measure plus length previous measure
    for (size_t i = 1; i < ml.size(); i++) {
        ms[i] = ms.at(i - 1) + ml.at(i - 1);
    }
}

//---------------------------------------------------------
//   fixupSigmap
//---------------------------------------------------------

/**
 To enable error handling in pass2, ensure sigmap contains a valid entry at tick = 0.
 Required by TimeSigMap::tickValues(), called (indirectly) by Segment::add().
 */

static void fixupSigmap(MusicXmlLogger* logger, Score* score, const std::vector<Fraction>& measureLength)
{
    auto it = score->sigmap()->find(0);

    if (it == score->sigmap()->end()) {
        // no valid timesig at tick = 0
        logger->logDebugInfo(u"no valid time signature at tick = 0");
        // use length of first measure instead time signature.
        // if there is no first measure, we probably don't care,
        // but set a default anyway.
        Fraction tsig = measureLength.empty() ? Fraction(4, 4) : measureLength.at(0);
        score->sigmap()->add(0, tsig);
    }
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Parse MusicXML in \a device and extract pass 1 data.
 */

Err MusicXmlParserPass1::parse(const ByteArray& data)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::parse device");
    m_parts.clear();
    m_e.setData(data);
    Err res = parse();
    if (res != Err::NoError) {
        return res;
    }

    // Determine the start tick of each measure in the part
    determineMeasureLength(m_measureLength);
    determineMeasureStart(m_measureLength, m_measureStart);
    // Fixup timesig at tick = 0 if necessary
    fixupSigmap(m_logger, m_score, m_measureLength);
    // Create the measures
    createMeasuresAndVboxes(m_score, m_measureLength, m_measureStart, m_systemStartMeasureNrs, m_pageStartMeasureNrs, m_credits,
                            m_pageSize);

    return res;
}

//---------------------------------------------------------
//   parse
//---------------------------------------------------------

/**
 Start the parsing process, after verifying the top-level node is score-partwise
 */

Err MusicXmlParserPass1::parse()
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::parse");

    bool found = false;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "score-partwise") {
            found = true;
            scorePartwise();
        } else {
            m_logger->logError(String(u"this is not a MusicXML score-partwise file (top-level node '%1')")
                               .arg(String::fromAscii(m_e.name().ascii())), &m_e);
            m_e.skipCurrentElement();
            return Err::FileBadFormat;
        }
    }

    if (!found) {
        m_logger->logError(u"this is not a MusicXML score-partwise file, node <score-partwise> not found", &m_e);
        if (!m_e.errorString().isEmpty()) {
            m_errors += errorStringWithLocation(m_e.lineNumber(), m_e.columnNumber(), m_e.errorString()) + '\n';
        }
        return Err::FileBadFormat;
    }

    return Err::NoError;
}

//---------------------------------------------------------
//   allStaffGroupsIdentical
//---------------------------------------------------------

/**
 Return true if all staves in Part \a p have the same staff group
 */

static bool allStaffGroupsIdentical(Part const* const p)
{
    for (size_t i = 1; i < p->nstaves(); ++i) {
        if (p->staff(0)->constStaffType(Fraction(0, 1))->group() != p->staff(i)->constStaffType(Fraction(0, 1))->group()) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   isRedundantBracket
//---------------------------------------------------------

/**
 Return true if there is already an existing bracket
 with the same type and span.
 This prevents double brackets, which are sometimes exported
 by Dolet.
 */

static bool isRedundantBracket(Staff const* const staff, const BracketType bracketType, const size_t span)
{
    for (const BracketItem* bracket : staff->brackets()) {
        if (bracket->bracketType() == bracketType && bracket->bracketSpan() == span) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   scorePartwise
//---------------------------------------------------------

/**
 Parse the MusicXML top-level (XPath /score-partwise) node.
 */

void MusicXmlParserPass1::scorePartwise()
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::scorePartwise", &m_e);

    MusicXmlPartGroupList partGroupList;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "part") {
            part();
        } else if (m_e.name() == "part-list") {
            partList(partGroupList);
        } else if (m_e.name() == "work") {
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "work-number") {
                    m_score->setMetaTag(u"workNumber", m_e.readText());
                } else if (m_e.name() == "work-title") {
                    m_score->setMetaTag(u"workTitle", m_e.readText());
                } else {
                    skipLogCurrElem();
                }
            }
        } else if (m_e.name() == "identification") {
            identification();
        } else if (m_e.name() == "defaults") {
            defaults();
        } else if (m_e.name() == "movement-number") {
            m_score->setMetaTag(u"movementNumber", m_e.readText());
        } else if (m_e.name() == "movement-title") {
            m_score->setMetaTag(u"movementTitle", m_e.readText());
        } else if (m_e.name() == "credit") {
            credit(m_credits);
        } else {
            skipLogCurrElem();
        }
    }

    // add brackets where required

    /*
     LOGD("partGroupList");
     for (size_t i = 0; i < partGroupList.size(); i++) {
     MusicXmlPartGroup* pg = partGroupList[i];
     LOGD("part-group span %d start %d type %hhd barlinespan %d",
     pg->span, pg->start, pg->type, pg->barlineSpan);
     }
     */

    // set of (typically multi-staff) parts containing one or more explicit brackets
    // spanning only that part: these won't get an implicit brace later
    // e.g. a two-staff piano part with an explicit brace
    std::set<const Part*> partSet;

    // handle the explicit brackets
    const std::vector<Part*>& il = m_score->parts();
    for (size_t i = 0; i < partGroupList.size(); i++) {
        MusicXmlPartGroup* pg = partGroupList[i];
        // determine span in staves
        // and span all barlines except last if applicable
        size_t stavesSpan = 0;
        for (int j = 0; j < pg->span; j++) {
            Part* spannedPart = il.at(pg->start + j);
            stavesSpan += spannedPart->nstaves();

            if (!pg->barlineSpan) {
                continue;
            }

            for (Staff* spannedStaff : spannedPart->staves()) {
                if ((j == pg->span - 1) && (spannedStaff == spannedPart->staves().back())) {
                    // Very last staff of group,
                    continue;
                }

                spannedStaff->setBarLineSpan(true);
            }
        }
        // add bracket and set the span
        // TODO: use group-symbol default-x to determine horizontal order of brackets
        Staff* staff = il.at(pg->start)->staff(0);
        if (pg->type != BracketType::NO_BRACKET && !isRedundantBracket(staff, pg->type, stavesSpan)) {
            staff->setBracketType(pg->column, pg->type);
            staff->setBracketSpan(pg->column, stavesSpan);
            if ((staff->brackets().size() > pg->column) && pg->color.isValid()) {
                BracketItem* bracketItem = staff->brackets().at(pg->column);
                bracketItem->setColor(pg->color);
            }
            // add part to set (skip implicit bracket later)
            if (pg->span == 1) {
                partSet.insert(il.at(pg->start));
            }
        }

        Score* score = staff->score();
        if (!score->isSystemObjectStaff(staff) && exporterSoftware() == MusicXmlExporterSoftware::FINALE
            && configuration()->inferTextType()) {
            score->addSystemObjectStaff(staff);
        }
    }

    // handle the implicit brackets:
    // multi-staff parts w/o explicit brackets get a brace
    for (const Part* p : il) {
        if (p->nstaves() > 1 && !muse::contains(partSet, p)) {
            const size_t column = p->staff(0)->bracketLevels() + 1;
            p->staff(0)->setBracketType(column, BracketType::BRACE);
            p->staff(0)->setBracketSpan(column, p->nstaves());
            if (allStaffGroupsIdentical(p)) {
                // span only if the same types
                for (Staff* spannedStaff : p->staves()) {
                    if (spannedStaff != p->staves().back()) { // not last staff
                        spannedStaff->setBarLineSpan(true);
                    }
                }
            }
        }
    }
    addError(checkAtEndElement(m_e, u"score-partwise"));
}

//---------------------------------------------------------
//   identification
//---------------------------------------------------------

/**
 Parse the /score-partwise/identification node:
 read the metadata.
 */

void MusicXmlParserPass1::identification()
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::identification", &m_e);

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "creator") {
            // type is an arbitrary label
            String strType = m_e.attribute("type");
            m_score->setMetaTag(strType, m_e.readText());
        } else if (m_e.name() == "rights") {
            m_score->setMetaTag(u"copyright", m_e.readText());
        } else if (m_e.name() == "encoding") {
            // TODO
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "encoder") {
                    m_score->setMetaTag(u"encoder", m_e.readText());
                } else if (m_e.name() == "software") {
                    String exporterString = m_e.readText().toLower();
                    setExporterSoftware(exporterString);
                } else if (m_e.name() == "supports" && m_e.asciiAttribute("element") == "beam" && m_e.asciiAttribute("type") == "yes") {
                    m_hasBeamingInfo = true;
                    m_e.skipCurrentElement();
                } else {
                    m_e.skipCurrentElement();
                }
            }
            // _score->setMetaTag("encoding", _e.readText()); works with DOM but not with pull parser
            // temporarily fake the encoding tag (compliant with DOM parser) to help the autotester
            if (MScore::debugMode) {
                m_score->setMetaTag(u"encoding", u"MuseScore 0.7.02007-09-10");
            }
        } else if (m_e.name() == "source") {
            m_score->setMetaTag(u"source", m_e.readText());
        } else if (m_e.name() == "miscellaneous") {
            // store all miscellaneous information
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "miscellaneous-field") {
                    const String name = m_e.attribute("name");
                    m_score->setMetaTag(name, m_e.readText());
                } else {
                    m_e.skipCurrentElement();
                }
            }
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   text2syms
//---------------------------------------------------------

/**
 Convert SMuFL code points to MuseScore <sym>...</sym>
 */

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
        SymId id = SymId(i);
        String string = sf->toString(id);
        // insert all syms except space to prevent matching all regular spaces
        if (id != SymId::space) {
            map.insert({ string, id });
        }
        if (string.size() > maxStringSize) {
            maxStringSize = string.size();
        }
    }
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

//---------------------------------------------------------
//   nextPartOfFormattedString
//---------------------------------------------------------

// TODO: probably should be shared between pass 1 and 2

/**
 Read the next part of a MusicXML formatted string and convert to MuseScore internal encoding.
 */

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
    String syms       = text2syms(txt);
    if (overrideTextStyleForComposer(syms)) {
        return syms;
    }

    String importedtext;

    if (!fontSize.isEmpty()) {
        bool ok = true;
        float size = fontSize.toFloat(&ok);
        if (ok) {
            importedtext += String(u"<font size=\"%1\"/>").arg(size);
        }
    }

    bool needUseDefaultFont = configuration()->needUseDefaultFont();
    if (!fontFamily.isEmpty() && txt == syms && !needUseDefaultFont) {
        // add font family only if no <sym> replacement made
        importedtext += String(u"<font face=\"%1\"/>").arg(fontFamily);
    }
    if (fontWeight == u"bold") {
        importedtext += u"<b>";
    }
    if (fontStyle == u"italic") {
        importedtext += u"<i>";
    }
    if (!underline.isEmpty()) {
        bool ok = true;
        int lines = underline.toInt(&ok);
        if (ok && (lines > 0)) {    // 1, 2, or 3 underlines are imported as single underline
            importedtext += u"<u>";
        } else {
            underline.clear();
        }
    }
    if (!strike.isEmpty()) {
        bool ok = true;
        int lines = strike.toInt(&ok);
        if (ok && (lines > 0)) {    // 1, 2, or 3 strikes are imported as single strike
            importedtext += u"<s>";
        } else {
            underline.clear();
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

//---------------------------------------------------------
//   credit
//---------------------------------------------------------

/**
 Parse the /score-partwise/credit node:
 read the credits for later handling by doCredits().
 */

void MusicXmlParserPass1::credit(CreditWordsList& credits)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::credit", &m_e);

    const int page = m_e.intAttribute("page");         // ignoring errors implies incorrect conversion defaults to the first page
    // multiple credit-words elements may be present,
    // which are appended
    // use the position info from the first one
    // font information is ignored, credits will be styled
    bool creditWordsRead = false;
    double defaultx = 0;
    double defaulty = 0;
    double fontSize = 0;
    String justify;
    String halign;
    String valign;
    StringList crtypes;
    String crwords;
    bool hasRights = false;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "credit-words") {
            // IMPORT_LAYOUT
            if (!creditWordsRead) {
                defaultx = m_e.doubleAttribute("default-x");
                defaulty = m_e.doubleAttribute("default-y");
                fontSize = m_e.doubleAttribute("font-size");
                justify  = m_e.attribute("justify");
                halign   = m_e.attribute("halign");
                valign   = m_e.attribute("valign");
                creditWordsRead = true;
            }
            crwords += nextPartOfFormattedString(m_e);
        } else if (m_e.name() == "credit-type") {
            // multiple credit-type elements may be present, supported by
            // e.g. Finale v26.3 for Mac.
            String type = m_e.readText();
            crtypes.push_back(type);
            hasRights = hasRights || type == u"rights";
        } else {
            skipLogCurrElem();
        }
    }
    if (!hasRights && isLikelyRightsText(crwords)) {
        crtypes.push_back(u"rights");
    }
    if (!crwords.empty()) {
        // as the meaning of multiple credit-types is undocumented,
        // use credit-type only if exactly one was found
        String crtype = (crtypes.size() == 1) ? crtypes.at(0) : String();
        CreditWords* cw = new CreditWords(page, crtype, defaultx, defaulty, fontSize, justify, halign, valign, crwords);
        credits.push_back(cw);
    }
}

//---------------------------------------------------------
//   isTitleFrameStyle
//---------------------------------------------------------

/**
 Determine if tid is a style type used in a title frame
 */

static bool isTitleFrameStyle(const TextStyleType tid)
{
    return tid == TextStyleType::TITLE
           || tid == TextStyleType::SUBTITLE
           || tid == TextStyleType::COMPOSER
           || tid == TextStyleType::LYRICIST;
}

//---------------------------------------------------------
//   isHarpDiagramStyle
//---------------------------------------------------------

/**
 Determine if tid is a style type used in a harp pedal diagram
 */

static bool isHarpPedalStyle(const TextStyleType tid)
{
    return tid == TextStyleType::HARP_PEDAL_DIAGRAM || tid == TextStyleType::HARP_PEDAL_TEXT_DIAGRAM;
}

//---------------------------------------------------------
//   updateStyles
//---------------------------------------------------------

/**
 Update the style definitions to match the MusicXML word-font and lyric-font.
 */

static void updateStyles(Score* score,
                         const String& wordFamily, const String& wordSize,
                         const String& lyricFamily, const String& lyricSize)
{
    const double dblWordSize = wordSize.toDouble();     // note conversion error results in value 0.0
    const double dblLyricSize = lyricSize.toDouble();   // but avoid comparing (double) floating point number with exact value later
    const double epsilon = 0.001;                       // use epsilon instead

    bool needUseDefaultFont = configuration()->needUseDefaultFont();

    // loop over all text styles (except the empty, always hidden, first one)
    // set all text styles to the MusicXML defaults
    for (const TextStyleType tid : allTextStyles()) {
        // The MusicXML specification does not specify to which kinds of text
        // the word-font setting applies. Setting all sizes to the size specified
        // gives bad results, so a selection is made:
        // exclude lyrics odd and even lines (handled separately),
        // Roman numeral analysis and harp pedal diagrams (special case, leave untouched)
        // and text types used in the title frame
        // Some further tweaking may still be required.

        if (tid == TextStyleType::LYRICS_ODD || tid == TextStyleType::LYRICS_EVEN
            || tid == TextStyleType::FRET_DIAGRAM_FINGERING || tid == TextStyleType::FRET_DIAGRAM_FRET_NUMBER
            || tid == TextStyleType::TAB_FRET_NUMBER) {
            continue;
        }

        bool needUseDefaultSize = tid == TextStyleType::HARMONY_ROMAN
                                  || tid == TextStyleType::HAMMER_ON_PULL_OFF
                                  || isTitleFrameStyle(tid)
                                  || isHarpPedalStyle(tid);

        const TextStyle* ts = textStyle(tid);
        for (const TextStyleProperty& a :*ts) {
            if (a.pid == Pid::FONT_FACE && !needUseDefaultFont) {
                score->style().set(a.sid, wordFamily);
            } else if (a.pid == Pid::FONT_SIZE && dblWordSize > epsilon && !needUseDefaultSize) {
                score->style().set(a.sid, dblWordSize);
            }
        }
    }

    // handle lyrics odd and even lines separately
    if (!needUseDefaultFont) {
        score->style().set(Sid::lyricsOddFontFace, lyricFamily);
        score->style().set(Sid::lyricsEvenFontFace, lyricFamily);
    }
    if (dblLyricSize > epsilon) {
        score->style().set(Sid::lyricsOddFontSize, dblLyricSize);
        score->style().set(Sid::lyricsEvenFontSize, dblLyricSize);
    }
}

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

static void setPageFormat(Score* score, const PageFormat& pf)
{
    score->style().set(Sid::pageWidth, pf.size.width());
    score->style().set(Sid::pageHeight, pf.size.height());
    score->style().set(Sid::pagePrintableWidth, pf.printableWidth);
    score->style().set(Sid::pageEvenLeftMargin, pf.evenLeftMargin);
    score->style().set(Sid::pageOddLeftMargin, pf.oddLeftMargin);
    score->style().set(Sid::pageEvenTopMargin, pf.evenTopMargin);
    score->style().set(Sid::pageEvenBottomMargin, pf.evenBottomMargin);
    score->style().set(Sid::pageOddTopMargin, pf.oddTopMargin);
    score->style().set(Sid::pageOddBottomMargin, pf.oddBottomMargin);
    score->style().set(Sid::pageTwosided, pf.twosided);
}

static double scaleText(const String& str, const Sid fontFaceSid, const double fontSize, const Score* score)
{
    // Scale text to fit within margins
    const MStyle style = score->style();
    const String fontFace = style.styleV(fontFaceSid).value<String>();
    Font font(fontFace, Font::Type::Unknown);
    font.setPointSizeF(fontSize);
    const FontMetrics fm(font);

    const double pagePrintableWidth = style.styleV(Sid::pagePrintableWidth).value<double>() * DPI;
    const double pageWidth = style.styleV(Sid::pageWidth).value<double>() * DPI;
    const double pageHeight = style.styleV(Sid::pageHeight).value<double>() * DPI;
    const double textWidth = fm.boundingRect(RectF(0, 0, pageWidth, pageHeight), TextShowMnemonic, str).width();

    return pagePrintableWidth / textWidth;
}

static void scaleCopyrightText(Score* score)
{
    String copyright = score->metaTag(u"copyright");
    if (copyright.empty()) {
        return;
    }

    const double fontSize = score->style().styleV(Sid::footerFontSize).value<double>();
    const double sizeRatio = scaleText(copyright, Sid::footerFontFace, fontSize, score);

    if (sizeRatio < 1) {
        const double newSize = floor(fontSize * sizeRatio * 10) / 10;
        score->style().set(Sid::footerFontSize, newSize);
    }
}

static void scaleTitle(Score* score, Text* t)
{
    String title = score->metaTag(u"workTitle");
    if (title.empty()) {
        return;
    }

    const double fontSize = score->style().styleV(Sid::titleFontSize).value<double>();
    const double sizeRatio = scaleText(title, Sid::titleFontFace, fontSize, score);

    if (sizeRatio < 1) {
        const double newSize = floor(fontSize * sizeRatio * 10) / 10;
        // Need to layout text to generate fragments before changing its size
        TLayout::layoutText(t, t->mutldata());
        t->setProperty(Pid::FONT_SIZE, newSize);
        t->setPropertyFlags(Pid::FONT_SIZE, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

/**
 Parse the /score-partwise/defaults node:
 read the general score layout settings.
 */

void MusicXmlParserPass1::defaults()
{
    //_logger->logDebugTrace("MusicXmlParserPass1::defaults", &_e);

    double millimeter = m_score->style().spatium() / 10.0;
    double tenths = 1.0;
    String lyricFontFamily;
    String lyricFontSize;
    String wordFontFamily;
    String wordFontSize;

    bool isImportLayout = musicXmlImportLayout();

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "scaling") {
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "millimeters") {
                    millimeter = m_e.readDouble();
                } else if (m_e.name() == "tenths") {
                    tenths = m_e.readDouble();
                } else {
                    skipLogCurrElem();
                }
            }
            double _spatium = DPMM * (millimeter * 10.0 / tenths);
            if (isImportLayout) {
                m_score->style().setSpatium(_spatium);
            }
        } else if (m_e.name() == "concert-score") {
            m_score->style().set(Sid::concertPitch, true);
            m_e.skipCurrentElement();            // skip but don't log
        } else if (m_e.name() == "page-layout") {
            PageFormat pf;
            pageLayout(pf, millimeter / (tenths * INCH));
            if (isImportLayout) {
                setPageFormat(m_score, pf);
            }
        } else if (m_e.name() == "system-layout") {
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "system-margins") {
                    m_e.skipCurrentElement();            // skip but don't log
                } else if (m_e.name() == "system-distance") {
                    const Spatium val(m_e.readDouble() / 10.0);
                    if (isImportLayout) {
                        m_score->style().set(Sid::minSystemDistance, val);
                        //LOGD("system distance %f", val.val());
                    }
                } else if (m_e.name() == "top-system-distance") {
                    m_e.skipCurrentElement();            // skip but don't log
                } else if (m_e.name() == "system-dividers") {
                    while (m_e.readNextStartElement()) {
                        if (m_e.name() == "left-divider") {
                            m_score->style().set(Sid::dividerLeft, (m_e.asciiAttribute("print-object") != "no"));
                            if (isImportLayout) {
                                m_score->style().set(Sid::dividerLeftX, m_e.doubleAttribute("relative-x") / 10.0);
                                m_score->style().set(Sid::dividerLeftY, m_e.doubleAttribute("relative-y") / 10.0);
                            }
                            m_e.skipCurrentElement();
                        } else if (m_e.name() == "right-divider") {
                            m_score->style().set(Sid::dividerRight, (m_e.asciiAttribute("print-object") != "no"));
                            if (isImportLayout) {
                                m_score->style().set(Sid::dividerRightX, m_e.doubleAttribute("relative-x") / 10.0);
                                m_score->style().set(Sid::dividerRightY, m_e.doubleAttribute("relative-y") / 10.0);
                            }
                            m_e.skipCurrentElement();
                        } else {
                            skipLogCurrElem();
                        }
                    }
                } else {
                    skipLogCurrElem();
                }
            }
        } else if (m_e.name() == "staff-layout") {
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "staff-distance") {
                    const Spatium val(m_e.readDouble() / 10.0);
                    if (isImportLayout) {
                        m_score->style().set(Sid::staffDistance, val);
                    }
                } else {
                    skipLogCurrElem();
                }
            }
        } else if (m_e.name() == "appearance") {
            while (m_e.readNextStartElement()) {
                const String type = m_e.attribute("type");
                if (m_e.name() == "line-width") {
                    const double val = m_e.readDouble();
                    if (isImportLayout) {
                        setStyle(type, val);
                    }
                } else if (m_e.name() == "note-size") {
                    const double val = m_e.readDouble();
                    if (isImportLayout) {
                        setStyle(type, val);
                    }
                } else if (m_e.name() == "distance") {
                    m_e.skipCurrentElement();        // skip but don't log
                } else if (m_e.name() == "glyph") {
                    m_e.skipCurrentElement();        // skip but don't log
                } else if (m_e.name() == "other-appearance") {
                    m_e.skipCurrentElement();        // skip but don't log
                } else {
                    skipLogCurrElem();
                }
            }
        } else if (m_e.name() == "music-font") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "word-font") {
            wordFontFamily = m_e.attribute("font-family");
            wordFontSize = m_e.attribute("font-size");
            m_e.skipCurrentElement();
        } else if (m_e.name() == "lyric-font") {
            lyricFontFamily = m_e.attribute("font-family");
            lyricFontSize = m_e.attribute("font-size");
            m_e.skipCurrentElement();
        } else if (m_e.name() == "lyric-language") {
            m_e.skipCurrentElement();        // skip but don't log
        } else {
            skipLogCurrElem();
        }
    }

    /*
    LOGD("word font family '%s' size '%s' lyric font family '%s' size '%s'",
           muPrintable(wordFontFamily), muPrintable(wordFontSize),
           muPrintable(lyricFontFamily), muPrintable(lyricFontSize));
    */
    wordFontFamily = wordFontFamily.empty() ? u"Edwin" : wordFontFamily;
    lyricFontFamily = lyricFontFamily.empty() ? wordFontFamily : lyricFontFamily;
    updateStyles(m_score, wordFontFamily, wordFontSize, lyricFontFamily, lyricFontSize);

    scaleCopyrightText(m_score);
}

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void MusicXmlParserPass1::setStyle(const String& type, const double val)
{
    if (type == u"light barline") {
        m_score->style().set(Sid::barWidth, Spatium(val / 10));
    } else if (type == u"heavy barline") {
        m_score->style().set(Sid::endBarWidth, Spatium(val / 10));
    } else if (type == u"beam") {
        m_score->style().set(Sid::beamWidth, Spatium(val / 10));
    } else if (type == u"bracket") {
        m_score->style().set(Sid::bracketWidth, Spatium(val / 10));
    } else if (type == u"dashes") {
        m_score->style().set(Sid::lyricsDashLineThickness, Spatium(val / 10));
    } else if (type == u"enclosure") {
        m_score->style().set(Sid::staffTextFrameWidth, Spatium(val / 10));
    } else if (type == u"ending") {
        m_score->style().set(Sid::voltaLineWidth, Spatium(val / 10));
    } else if (type == u"extend") {
        m_score->style().set(Sid::lyricsLineThickness, Spatium(val / 10));
    } else if (type == u"leger") {
        m_score->style().set(Sid::ledgerLineWidth, Spatium(val / 10));
    } else if (type == u"pedal") {
        m_score->style().set(Sid::pedalLineWidth, Spatium(val / 10));
    } else if (type == "octave shift") {
        m_score->style().set(Sid::ottavaLineWidth, Spatium(val / 10));
    } else if (type == u"staff") {
        m_score->style().set(Sid::staffLineWidth, Spatium(val / 10));
    } else if (type == u"stem") {
        m_score->style().set(Sid::stemWidth, Spatium(val / 10));
    } else if (type == u"tuplet bracket") {
        m_score->style().set(Sid::tupletBracketWidth, Spatium(val / 10));
    } else if (type == u"wedge") {
        m_score->style().set(Sid::hairpinLineWidth, Spatium(val / 10));
    } else if (type == u"slur middle") {
        m_score->style().set(Sid::slurMidWidth, Spatium(val / 10));
    } else if (type == u"slur tip") {
        m_score->style().set(Sid::slurEndWidth, Spatium(val / 10));
    } else if (type == u"tie middle") {
        m_score->style().set(Sid::tieMidWidth, Spatium(val / 10));
    } else if (type == u"tie tip") {
        m_score->style().set(Sid::tieEndWidth, Spatium(val / 10));
    } else if ((type == u"cue")) {
        m_score->style().set(Sid::smallNoteMag, val / 100);
    } else if ((type == u"grace")) {
        m_score->style().set(Sid::graceNoteMag, val / 100);
    } else if ((type == u"grace-cue")) {
        // not supported
    }
}

//---------------------------------------------------------
//   pageLayout
//---------------------------------------------------------

/**
 Parse the /score-partwise/defaults/page-layout node: read the page layout.
 Note that MuseScore does not support a separate value for left and right margins
 for odd and even pages. Only odd and even left margins are used, together  with
 the printable width, which is calculated from the left and right margins in the
 MusicXML file.
 */

void MusicXmlParserPass1::pageLayout(PageFormat& pf, const double conversion)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::pageLayout", &m_e);

    double _oddRightMargin  = 0.0;
    double _evenRightMargin = 0.0;
    SizeF size;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "page-margins") {
            String type = m_e.attribute("type");
            if (type.empty()) {
                type = u"both";
            }
            double lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "left-margin") {
                    lm = m_e.readDouble() * conversion;
                } else if (m_e.name() == "right-margin") {
                    rm = m_e.readDouble() * conversion;
                } else if (m_e.name() == "top-margin") {
                    tm = m_e.readDouble() * conversion;
                } else if (m_e.name() == "bottom-margin") {
                    bm = m_e.readDouble() * conversion;
                } else {
                    skipLogCurrElem();
                }
            }
            pf.twosided = type == "odd" || type == "even";
            if (type == "odd" || type == "both") {
                pf.oddLeftMargin = lm;
                _oddRightMargin = rm;
                pf.oddTopMargin = tm;
                pf.oddBottomMargin = bm;
            }
            if (type == "even" || type == "both") {
                pf.evenLeftMargin = lm;
                _evenRightMargin = rm;
                pf.evenTopMargin = tm;
                pf.evenBottomMargin = bm;
            }
        } else if (m_e.name() == "page-height") {
            const double val = m_e.readDouble();
            size.setHeight(val * conversion);
            // set pageHeight and pageWidth for use by doCredits()
            m_pageSize.setHeight(static_cast<int>(val + 0.5));
        } else if (m_e.name() == "page-width") {
            const double val = m_e.readDouble();
            size.setWidth(val * conversion);
            // set pageHeight and pageWidth for use by doCredits()
            m_pageSize.setWidth(static_cast<int>(val + 0.5));
        } else {
            skipLogCurrElem();
        }
    }
    pf.size = size;
    double w1 = size.width() - pf.oddLeftMargin - _oddRightMargin;
    double w2 = size.width() - pf.evenLeftMargin - _evenRightMargin;
    pf.printableWidth = std::max(w1, w2);     // silently adjust right margins
}

//---------------------------------------------------------
//   partList
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list:
 create the parts and for each part set id and name.
 Also handle the part-groups.
 */

void MusicXmlParserPass1::partList(MusicXmlPartGroupList& partGroupList)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::partList", &m_e);

    int scoreParts = 0;   // number of score-parts read sofar
    MusicXmlPartGroupMap partGroups;
    String curPartGroupName;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "part-group") {
            partGroup(scoreParts, partGroupList, partGroups, curPartGroupName);
        } else if (m_e.name() == "score-part") {
            scorePart(curPartGroupName);
            scoreParts++;
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   createPart
//---------------------------------------------------------

/**
 Create the part, set its \a id and insert it in PartMap \a pm.
 Part name (if any) will be set later.
 */

static void createPart(Score* score, const String& id, PartMap& pm)
{
    Part* part = new Part(score);
    pm.insert({ id, part });
    score->appendPart(part);
    Staff* staff = Factory::createStaff(part);
    staff->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);
    score->appendStaff(staff);
}

//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

typedef std::map<int, MusicXmlPartGroup*> MusicXmlPartGroupMap;

/**
 Store part-group start with number \a n, first part \a p and symbol / \a s in the partGroups
 map \a pgs for later reference, as at this time insufficient information is available to be able
 to generate the brackets.
 */

static void partGroupStart(MusicXmlPartGroupMap& pgs, int n, int p, const String& s, bool barlineSpan, Color color)
{
    //LOGD("partGroupStart number=%d part=%d symbol=%s", n, p, muPrintable(s));

    if (pgs.count(n) > 0) {
        LOGD("part-group number=%d already active", n);
        return;
    }

    BracketType bracketType = BracketType::NO_BRACKET;
    if (s.empty()) {
        // ignore (handle as NO_BRACKET)
    } else if (s == u"none") {
        // already set to NO_BRACKET
    } else if (s == u"brace") {
        bracketType = BracketType::BRACE;
    } else if (s == u"bracket") {
        bracketType = BracketType::NORMAL;
    } else if (s == u"line") {
        bracketType = BracketType::LINE;
    } else if (s == u"square") {
        bracketType = BracketType::SQUARE;
    } else {
        LOGD("part-group symbol=%s not supported", muPrintable(s));
        return;
    }

    MusicXmlPartGroup* pg = new MusicXmlPartGroup;
    pg->span = 0;
    pg->start = p;
    pg->barlineSpan = barlineSpan,
    pg->type = bracketType;
    if (color.isValid()) {
        pg->color = color;
    }
    pg->column = static_cast<size_t>(n);
    pgs[n] = pg;
}

//---------------------------------------------------------
//   partGroupStop
//---------------------------------------------------------

/**
 Handle part-group stop with number \a n and part \a p.

 For part group n, the start part, span (in parts) and type are now known.
 To generate brackets, the span in staves must also be known.
 */

static void partGroupStop(MusicXmlPartGroupMap& pgs, int n, int p,
                          MusicXmlPartGroupList& pgl)
{
    if (pgs.count(n) == 0) {
        LOGD("part-group number=%d not active", n);
        return;
    }

    pgs[n]->span = p - pgs[n]->start;
    //LOGD("partgroupstop number=%d start=%d span=%d type=%hhd",
    //       n, pgs[n]->start, pgs[n]->span, pgs[n]->type);
    pgl.push_back(pgs[n]);
    pgs.erase(n);
}

//---------------------------------------------------------
//   partGroup
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/part-group node.
 */

void MusicXmlParserPass1::partGroup(const int scoreParts,
                                    MusicXmlPartGroupList& partGroupList,
                                    MusicXmlPartGroupMap& partGroups, String& curPartGroupName)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::partGroup", &m_e);
    bool barlineSpan = true;
    int number = m_e.intAttribute("number");
    if (number > 0) {
        number--;
    }
    String symbol;
    String type = m_e.attribute("type");
    Color symbolColor;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "group-name") {
            curPartGroupName = m_e.readText();
        } else if (m_e.name() == "group-abbreviation") {
            symbol = m_e.readText();
        } else if (m_e.name() == "group-symbol") {
            symbolColor = Color::fromString(m_e.attribute("color"));
            symbol = m_e.readText();
        } else if (m_e.name() == "group-barline") {
            if (m_e.readText() == "no") {
                barlineSpan = false;
            }
        } else {
            skipLogCurrElem();
        }
    }

    if (type == "start") {
        partGroupStart(partGroups, number, scoreParts, symbol, barlineSpan, symbolColor);
    } else if (type == "stop") {
        partGroupStop(partGroups, number, scoreParts, partGroupList);
    } else {
        m_logger->logError(String(u"part-group type '%1' not supported").arg(type), &m_e);
    }
}

//---------------------------------------------------------
//   scorePart
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part node:
 create the part and sets id and name.
 Note that a part is created even if no part-name is present
 which is invalid MusicXML but is (sometimes ?) generated by NWC2MusicXml.
 */

void MusicXmlParserPass1::scorePart(const String& curPartGroupName)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::scorePart", &m_e);
    String id = m_e.attribute("id").trimmed();

    if (muse::contains(m_parts, id)) {
        m_logger->logError(String(u"duplicate part id '%1'").arg(id), &m_e);
        skipLogCurrElem();
        return;
    } else {
        m_parts.insert({ id, MusicXmlPart(id) });
        m_instruments.insert({ id, MusicXmlInstruments() });
        createPart(m_score, id, m_partMap);
    }

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "part-name") {
            // EngravingItem part-name contains the displayed (full) part name
            // It is displayed by default, but can be suppressed (print-object=”no”)
            // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
            // and the formatted version in the part-name-display element
            m_parts[id].setPrintName(m_e.asciiAttribute("print-object") != "no");
            String name = m_e.readText();
            m_parts[id].setName(name);
        } else if (m_e.name() == "part-name-display") {
            String name;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "display-text") {
                    name += m_e.readText();
                } else if (m_e.name() == "accidental-text") {
                    name += musicXmlAccidentalTextToChar(m_e.readText());
                } else {
                    skipLogCurrElem();
                }
            }
            if (!name.empty()) {
                m_parts[id].setName(name);
            }
        } else if (m_e.name() == "part-abbreviation") {
            // EngravingItem part-name contains the displayed (abbreviated) part name
            // It is displayed by default, but can be suppressed (print-object=”no”)
            // As of MusicXML 3.0, formatting is deprecated, with part-name in plain text
            // and the formatted version in the part-abbreviation-display element
            m_parts[id].setPrintAbbr(m_e.asciiAttribute("print-object") != "no");
            String name = m_e.readText();
            m_parts[id].setAbbr(name);
        } else if (m_e.name() == "part-abbreviation-display") {
            String name;
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "display-text") {
                    name += m_e.readText();
                } else if (m_e.name() == "accidental-text") {
                    name += musicXmlAccidentalTextToChar(m_e.readText());
                } else {
                    skipLogCurrElem();
                }
            }
            if (!name.empty()) {
                m_parts[id].setAbbr(name);
            }
        } else if (m_e.name() == "group") {
            // TODO
            m_e.skipCurrentElement();          // skip but don't log
        } else if (m_e.name() == "score-instrument") {
            scoreInstrument(id, curPartGroupName);
        } else if (m_e.name() == "player") {
            // unsupported
            m_e.skipCurrentElement();          // skip but don't log
        } else if (m_e.name() == "midi-device") {
            if (!m_e.hasAttribute("port")) {
                m_e.readText();         // empty string
                continue;
            }
            String instrId = m_e.attribute("id");
            String port = m_e.attribute("port");
            // If instrId is missing, the device assignment affects all
            // score-instrument elements in the score-part
            if (instrId.isEmpty()) {
                for (auto it = m_instruments[id].cbegin(); it != m_instruments[id].cend(); ++it) {
                    m_instruments[id][it->first].midiPort = port.toInt() - 1;
                }
            } else if (muse::contains(m_instruments.at(id), instrId)) {
                m_instruments[id][instrId].midiPort = port.toInt() - 1;
            }

            m_e.readText();       // empty string
        } else if (m_e.name() == "midi-instrument") {
            midiInstrument(id);
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   scoreInstrument
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part/score-instrument node.
 */

void MusicXmlParserPass1::scoreInstrument(const String& partId, const String& curPartGroupName)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::scoreInstrument", &m_e);
    String instrId = m_e.attribute("id");

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "ensemble") {
            skipLogCurrElem();
        } else if (m_e.name() == "instrument-name") {
            String instrName = m_e.readText();
            /*
            LOGD("partId '%s' instrId '%s' instrName '%s'",
                   muPrintable(partId),
                   muPrintable(instrId),
                   muPrintable(instrName)
                   );
             */

            // Finale exports all instrument names as 'Grand Piano' - use part name
            if (exporterSoftware() == MusicXmlExporterSoftware::FINALE) {
                instrName = m_parts[partId].getName();
                if (instrName.size() <= 1) {
                    instrName = curPartGroupName;
                }
            }
            m_instruments[partId].insert({ instrId, MusicXmlInstrument(instrName) });
            // EngravingItem instrument-name is typically not displayed in the score,
            // but used only internally
            if (muse::contains(m_instruments.at(partId), instrId)) {
                m_instruments[partId][instrId].name = instrName;
            }
        } else if (m_e.name() == "instrument-sound") {
            String instrSound = m_e.readText();
            if (muse::contains(m_instruments.at(partId), instrId)) {
                m_instruments[partId][instrId].sound = instrSound;
            }
        } else if (m_e.name() == "virtual-instrument") {
            while (m_e.readNextStartElement()) {
                if (m_e.name() == "virtual-library") {
                    String virtualLibrary = m_e.readText();
                    if (muse::contains(m_instruments.at(partId), instrId)) {
                        m_instruments[partId][instrId].virtLib = virtualLibrary;
                    }
                } else if (m_e.name() == "virtual-name") {
                    String virtualName = m_e.readText();
                    if (muse::contains(m_instruments.at(partId), instrId)) {
                        m_instruments[partId][instrId].virtName = virtualName;
                    }
                } else {
                    skipLogCurrElem();
                }
            }
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   midiInstrument
//---------------------------------------------------------

/**
 Parse the /score-partwise/part-list/score-part/midi-instrument node.
 */

void MusicXmlParserPass1::midiInstrument(const String& partId)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::midiInstrument", &m_e);
    String instrId = m_e.attribute("id");

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "midi-bank") {
            skipLogCurrElem();
        } else if (m_e.name() == "midi-channel") {
            int channel = m_e.readInt();
            if (channel < 1) {
                m_logger->logError(String(u"incorrect midi-channel: %1").arg(channel), &m_e);
                channel = 1;
            } else if (channel > 16) {
                m_logger->logError(String(u"incorrect midi-channel: %1").arg(channel), &m_e);
                channel = 16;
            }
            if (muse::contains(m_instruments.at(partId), instrId)) {
                m_instruments[partId][instrId].midiChannel = channel - 1;
            }
        } else if (m_e.name() == "midi-program") {
            int program = m_e.readInt();
            // Bug fix for Cubase 6.5.5 which generates <midi-program>0</midi-program>
            // Check program number range
            if (program < 1) {
                m_logger->logError(String(u"incorrect midi-program: %1").arg(program), &m_e);
                program = 1;
            } else if (program > 128) {
                m_logger->logError(String(u"incorrect midi-program: %1").arg(program), &m_e);
                program = 128;
            }
            if (muse::contains(m_instruments.at(partId), instrId)) {
                m_instruments[partId][instrId].midiProgram = program - 1;
            }
        } else if (m_e.name() == "midi-unpitched") {
            if (muse::contains(m_instruments.at(partId), instrId)) {
                m_instruments[partId][instrId].unpitched = m_e.readInt() - 1;
            }
        } else if (m_e.name() == "volume") {
            double vol = m_e.readDouble();
            if (vol >= 0 && vol <= 100) {
                if (muse::contains(m_instruments.at(partId), instrId)) {
                    m_instruments[partId][instrId].midiVolume = static_cast<int>((vol / 100) * 127);
                }
            } else {
                m_logger->logError(String(u"incorrect midi-volume: %1").arg(vol), &m_e);
            }
        } else if (m_e.name() == "pan") {
            double pan = m_e.readDouble();
            if (pan >= -90 && pan <= 90) {
                if (muse::contains(m_instruments.at(partId), instrId)) {
                    m_instruments[partId][instrId].midiPan = static_cast<int>(((pan + 90) / 180) * 127);
                }
            } else {
                m_logger->logError(String(u"incorrect midi-volume: %g1").arg(pan), &m_e);
            }
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   setNumberOfStavesForPart
//---------------------------------------------------------

/**
 Set number of staves for part \a partId to the max value
 of the current value \a staves.
 Also handle HideMode.
 */

static void setNumberOfStavesForPart(Part* const part, const size_t staves)
{
    IF_ASSERT_FAILED(part) {
        return;
    }

    size_t prevnstaves = part->nstaves();
    if (staves > part->nstaves()) {
        part->setStaves(static_cast<int>(staves));
        // New staves default to INSTRUMENT hide mode
        for (size_t i = prevnstaves; i < staves; ++i) {
            part->staff(i)->setHideWhenEmpty(Staff::HideMode::INSTRUMENT);
        }
    }
    if (staves != 0 && prevnstaves != 1 && prevnstaves != staves) {
        for (size_t i = 0; i < part->nstaves(); ++i) {
            // A "staves" value different from the existing nstaves means
            // staves in a part will sometimes be hidden.
            // We can approximate this with the AUTO hide mode.
            part->staff(i)->setHideWhenEmpty(Staff::HideMode::AUTO);
        }
    }
}

//---------------------------------------------------------
//   part
//---------------------------------------------------------

/**
 Parse the /score-partwise/part node:
 read the parts data to determine measure timing and octave shifts.
 Assign voices and staves.
 */

void MusicXmlParserPass1::part()
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::part", &m_e);
    const String id = m_e.attribute("id").trimmed();

    if (!muse::contains(m_parts, id)) {
        m_logger->logError(String(u"cannot find part '%1'").arg(id), &m_e);
        skipLogCurrElem();
        return;
    }

    initPartState(id);

    VoiceOverlapDetector vod;
    Fraction time;    // current time within part
    Fraction mdur;    // measure duration

    int measureNr = 0;
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "measure") {
            measure(id, time, mdur, vod, measureNr);
            time += mdur;
            ++measureNr;
        } else {
            skipLogCurrElem();
        }
    }

    // Bug fix for Cubase 6.5.5..9.5.10 which generate <staff>2</staff> in a single staff part
    setNumberOfStavesForPart(muse::value(m_partMap, id), m_parts[id].maxStaff() + 1);
    // allocate MuseScore staff to MusicXML voices
    allocateStaves(m_parts[id].voicelist);
    // allocate MuseScore voice to MusicXML voices
    allocateVoices(m_parts[id].voicelist);
    // calculate the octave shifts
    m_parts[id].calcOctaveShifts();
    // determine the lyric numbers for this part
    m_parts[id].lyricNumberHandler().determineLyricNos();
}

//---------------------------------------------------------
//   measureDurationAsFraction
//---------------------------------------------------------

/**
 Determine a suitable measure duration value given the time signature
 by setting the duration denominator to be greater than or equal
 to the time signature denominator
 */

static Fraction measureDurationAsFraction(const Fraction length, const int tsigtype)
{
    if (tsigtype <= 0) {
        // invalid tsigtype
        return length;
    }

    Fraction res = length;
    while (res.denominator() < tsigtype) {
        res.setNumerator(res.numerator() * 2);
        res.setDenominator(res.denominator() * 2);
    }
    return res;
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure node:
 read the measures data as required to determine measure timing, octave shifts
 and assign voices and staves.
 */

void MusicXmlParserPass1::measure(const String& partId,
                                  const Fraction cTime,
                                  Fraction& mdur,
                                  VoiceOverlapDetector& vod,
                                  const int measureNr)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::measure", &m_e);
    String number = m_e.attribute("number");

    Fraction mTime;   // current time stamp within measure
    Fraction mDura;   // current total measure duration
    vod.newMeasure();
    MusicXmlTupletStates tupletStates;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "attributes") {
            attributes(partId, cTime + mTime);
        } else if (m_e.name() == "barline") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "note") {
            Fraction missingPrev;
            Fraction dura;
            Fraction missingCurr;
            // note: chord and grace note handling done in note()
            note(partId, cTime + mTime, missingPrev, dura, missingCurr, vod, tupletStates);
            if (missingPrev.isValid()) {
                mTime += missingPrev;
            }
            if (dura.isValid()) {
                mTime += dura;
            }
            if (missingCurr.isValid()) {
                mTime += missingCurr;
            }
            if (mTime > mDura) {
                mDura = mTime;
            }
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
            }
        } else if (m_e.name() == "direction") {
            direction(partId, cTime + mTime);
        } else if (m_e.name() == "harmony") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "print") {
            print(measureNr);
        } else if (m_e.name() == "sound") {
            m_e.skipCurrentElement();        // skip but don't log
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
    }

    // debug vod
    // vod.dump();
    // copy overlap data from vod to voicelist
    copyOverlapData(vod, m_parts[partId].voicelist);

    // measure duration fixups
    mDura.reduce();

    // fix for PDFtoMusic Pro v1.3.0d Build BF4E and PlayScore / ReadScoreLib Version 3.11
    // which sometimes generate empty measures
    // if no valid length found and length according to time signature is known,
    // use length according to time signature
    if (mDura.isZero() && m_timeSigDura.isValid() && m_timeSigDura > Fraction(0, 1)) {
        mDura = m_timeSigDura;
    }
    // if no valid length found and time signature is unknown, use default
    if (mDura.isZero() && !m_timeSigDura.isValid()) {
        mDura = Fraction(4, 4);
    }

    // if necessary, round up to an integral number of 1/128th,
    // to comply with MuseScores actual measure length constraints
    Fraction length = mDura * Fraction(128, 1);
    Fraction correctedLength = mDura;
    length.reduce();
    if (length.denominator() != 1) {
        Fraction roundDown = Fraction(length.numerator() / length.denominator(), 128);
        Fraction roundUp = Fraction(length.numerator() / length.denominator() + 1, 128);
        // mDura is not an integer multiple of 1/128;
        // first check if the duration is larger than an integer multiple of 1/128
        // by an amount smaller than the minimum division resolution
        // in that case, round down (rounding errors have possibly occurred),
        // otherwise, round up
        if ((m_divs > 0) && ((mDura - roundDown) < Fraction(1, 4 * m_divs))) {
            m_logger->logError(String(u"rounding down measure duration %1 to %2")
                               .arg(mDura.toString(), roundDown.toString()),
                               &m_e);
            correctedLength = roundDown;
        } else {
            m_logger->logError(String(u"rounding up measure duration %1 to %2")
                               .arg(mDura.toString(), roundUp.toString()),
                               &m_e);
            correctedLength = roundUp;
        }
        mDura = correctedLength;
    }

    // set measure duration to a suitable value given the time signature
    if (m_timeSigDura.isValid() && m_timeSigDura > Fraction(0, 1)) {
        int btp = m_timeSigDura.denominator();
        if (btp > 0) {
            mDura = measureDurationAsFraction(mDura, btp);
        }
    }

    // set return value(s)
    mdur = mDura;

    // set measure number and duration
    /*
    LOGD("part %s measure %s dura %s (%d)",
           muPrintable(partId), muPrintable(number), muPrintable(mdur.print()), mdur.ticks());
     */
    m_parts[partId].addMeasureNumberAndDuration(number, mdur);

    addError(checkAtEndElement(m_e, u"measure"));
}

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void MusicXmlParserPass1::print(const int measureNr)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::print", &m_e);

    const String newPage = m_e.attribute("new-page");
    const String newSystem = m_e.attribute("new-system");
    if (newPage == u"yes") {
        m_pageStartMeasureNrs.insert(measureNr);
    }
    if (newSystem == u"yes") {
        m_systemStartMeasureNrs.insert(measureNr);
    }

    m_e.skipCurrentElement();          // skip but don't log
}

//---------------------------------------------------------
//   attributes
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes node.
 */

void MusicXmlParserPass1::attributes(const String& partId, const Fraction cTime)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::attributes", &m_e);

    int staves = 0;
    std::set<int> hiddenStaves = {};

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "clef") {
            clef(partId);
        } else if (m_e.name() == "divisions") {
            divisions();
        } else if (m_e.name() == "key") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "instruments") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "staff-details") {
            if (m_e.asciiAttribute("print-object") == "no") {
                hiddenStaves.emplace(m_e.intAttribute("number"));
            }
            m_e.skipCurrentElement();
        } else if (m_e.name() == "staves") {
            staves = m_e.readInt();
        } else if (m_e.name() == "time") {
            time(cTime);
        } else if (m_e.name() == "transpose") {
            transpose(partId, cTime);
        } else {
            skipLogCurrElem();
        }
    }

    if (staves - static_cast<int>(hiddenStaves.size()) > MAX_STAVES) {
        m_logger->logError(u"staves exceed MAX_STAVES, even when discarding hidden staves", &m_e);
        return;
    } else if (staves > MAX_STAVES
               && static_cast<int>(hiddenStaves.size()) > 0
               && m_parts[partId].staffNumberToIndex().size() == 0) {
        m_logger->logError(u"staves exceed MAX_STAVES, but hidden staves can be discarded", &m_e);
        // Some scores have parts with many staves (~10), but most are hidden
        // When this occurs, we can discard hidden staves
        // and store a std::map between staffNumber and staffIndex.
        int staffNumber = 1;
        size_t staffIndex = 0;
        for (; staffNumber <= staves; ++staffNumber) {
            if (hiddenStaves.find(staffNumber) != hiddenStaves.end()) {
                m_logger->logError(String(u"removing hidden staff %1").arg(staffNumber), &m_e);
                continue;
            }
            m_parts[partId].insertStaffNumberToIndex(staffNumber, static_cast<int>(staffIndex));
            ++staffIndex;
        }
        DO_ASSERT(staffIndex == m_parts[partId].staffNumberToIndex().size());

        setNumberOfStavesForPart(muse::value(m_partMap, partId), staves - static_cast<int>(hiddenStaves.size()));
    } else {
        // Otherwise, don't discard any staves
        // And set hidden staves to HideMode::AUTO
        // (MuseScore doesn't currently have a mechanism
        // for hiding non-empty staves, so this is an approximation
        // of the correct implementation)
        setNumberOfStavesForPart(muse::value(m_partMap, partId), staves);
        for (int hiddenStaff : hiddenStaves) {
            int hiddenStaffIndex = muse::value(m_parts, partId).staffNumberToIndex(hiddenStaff);
            if (hiddenStaffIndex >= 0) {
                muse::value(m_partMap, partId)->staff(hiddenStaffIndex)->setHideWhenEmpty(Staff::HideMode::AUTO);
            }
        }
    }
}

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/clef node.
 TODO: Store the clef type, to simplify staff type setting in pass 2.
 */

void MusicXmlParserPass1::clef(const String& /* partId */)
{
    m_logger->logDebugTrace(u"MusicXmlParserPass1::clef", &m_e);

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "line") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "sign") {
            String sign = m_e.readText();
        } else {
            skipLogCurrElem();
        }
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

static bool determineTimeSig(MusicXmlLogger* logger, const XmlStreamReader* const xmlreader,
                             const String& beats, const String& beatType, const String& timeSymbol,
                             TimeSigType& st, int& bts, int& btp)
{
    // initialize
    st  = TimeSigType::NORMAL;
    bts = 0;               // the beats (max 4 separated by "+") as integer
    btp = 0;               // beat-type as integer
    // determine if timesig is valid
    if (timeSymbol == "cut") {
        st = TimeSigType::ALLA_BREVE;
    } else if (timeSymbol == "common") {
        st = TimeSigType::FOUR_FOUR;
    } else if (!timeSymbol.isEmpty() && timeSymbol != "normal") {
        logger->logError(String(u"time symbol '%1' not recognized")
                         .arg(timeSymbol), xmlreader);
        return false;
    }

    btp = beatType.toInt();
    StringList list = beats.split(u'+');
    for (size_t i = 0; i < list.size(); i++) {
        bts += list.at(i).toInt();
    }

    // determine if bts and btp are valid
    if (bts <= 0 || btp <= 0) {
        logger->logError(String(u"beats=%1 and/or beat-type=%2 not recognized")
                         .arg(beats, beatType), xmlreader);
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

void MusicXmlParserPass1::time(const Fraction cTime)
{
    String beats;
    String beatType;
    String timeSymbol = m_e.attribute("symbol");

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
        int bts = 0;           // total beats as integer (beats may contain multiple numbers, separated by "+")
        int btp = 0;           // beat-type as integer
        if (determineTimeSig(m_logger, &m_e, beats, beatType, timeSymbol, st, bts, btp)) {
            m_timeSigDura = Fraction(bts, btp);
            m_score->sigmap()->add(cTime.ticks(), m_timeSigDura);
        }
    }
}

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/transpose node.
 */

void MusicXmlParserPass1::transpose(const String& partId, const Fraction& tick)
{
    Interval interval;
    while (m_e.readNextStartElement()) {
        int i = m_e.readInt();
        if (m_e.name() == "diatonic") {
            interval.diatonic = i;
        } else if (m_e.name() == "chromatic") {
            interval.chromatic = i;
        } else if (m_e.name() == "octave-change") {
            interval.diatonic += i * 7;
            interval.chromatic += i * 12;
        } else {
            skipLogCurrElem();
        }
    }

    if (m_parts[partId]._intervals.count(tick) == 0) {
        if (!interval.diatonic && interval.chromatic) {
            interval.diatonic = chromatic2diatonic(interval.chromatic);
        }
        m_parts[partId]._intervals[tick] = interval;
    } else {
        LOGD("duplicate transpose at tick %s", muPrintable(tick.toString()));
    }
}

//---------------------------------------------------------
//   divisions
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/attributes/divisions node.
 */

void MusicXmlParserPass1::divisions()
{
    m_divs = m_e.readInt();
    if (!(m_divs > 0)) {
        m_logger->logError(u"illegal divisions", &m_e);
    }
}

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction node
 to be able to handle octave-shifts, as these must be interpreted
 in musical order instead of in MusicXML file order.
 */

void MusicXmlParserPass1::direction(const String& partId, const Fraction& cTime)
{
    // note: file order is direction-type first, then staff
    // this means staff is still unknown when direction-type is handled

    std::vector<MusicXmlOctaveShiftDesc> starts;
    std::vector<MusicXmlOctaveShiftDesc> stops;
    int staff = 0;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "direction-type") {
            directionType(cTime, starts, stops);
        } else if (m_e.name() == "staff") {
            int nstaves = static_cast<int>(getPart(partId)->nstaves());
            String strStaff = m_e.readText();
            staff = m_parts[partId].staffNumberToIndex(strStaff.toInt());
            if (0 <= staff && staff < nstaves) {
                //LOGD("direction staff %d", staff + 1);
            } else {
                m_logger->logError(String(u"invalid staff %1").arg(strStaff), &m_e);
                staff = 0;
            }
        } else {
            m_e.skipCurrentElement();
        }
    }

    // handle the stops first
    for (const MusicXmlOctaveShiftDesc& desc : stops) {
        if (muse::contains(m_octaveShifts, static_cast<int>(desc.num))) {
            MusicXmlOctaveShiftDesc prevDesc = m_octaveShifts.at(desc.num);
            if (prevDesc.tp == MusicXmlOctaveShiftDesc::Type::UP
                || prevDesc.tp == MusicXmlOctaveShiftDesc::Type::DOWN) {
                // a complete pair
                m_parts[partId].addOctaveShift(staff, prevDesc.size, prevDesc.time);
                m_parts[partId].addOctaveShift(staff, -prevDesc.size, desc.time);
            } else {
                m_logger->logError(u"double octave-shift stop", &m_e);
            }
            muse::remove(m_octaveShifts, desc.num);
        } else {
            m_octaveShifts.insert({ desc.num, desc });
        }
    }

    // then handle the starts
    for (const MusicXmlOctaveShiftDesc& desc : starts) {
        if (muse::contains(m_octaveShifts, static_cast<int>(desc.num))) {
            MusicXmlOctaveShiftDesc prevDesc = m_octaveShifts.at(desc.num);
            if (prevDesc.tp == MusicXmlOctaveShiftDesc::Type::STOP) {
                // a complete pair
                m_parts[partId].addOctaveShift(staff, desc.size, desc.time);
                m_parts[partId].addOctaveShift(staff, -desc.size, prevDesc.time);
            } else {
                m_logger->logError(u"double octave-shift start", &m_e);
            }
            muse::remove(m_octaveShifts, desc.num);
        } else {
            m_octaveShifts.insert({ desc.num, desc });
        }
    }
}

//---------------------------------------------------------
//   directionType
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/direction/direction-type node.
 */

void MusicXmlParserPass1::directionType(const Fraction cTime,
                                        std::vector<MusicXmlOctaveShiftDesc>& starts,
                                        std::vector<MusicXmlOctaveShiftDesc>& stops)
{
    while (m_e.readNextStartElement()) {
        if (m_e.name() == "octave-shift") {
            String number = m_e.attribute("number");
            int n = 0;
            if (!number.empty()) {
                n = number.toInt();
                if (n <= 0) {
                    m_logger->logError(String(u"invalid number %1").arg(number), &m_e);
                } else {
                    n--;            // make zero-based
                }
            }

            if (0 <= n && n < MAX_NUMBER_LEVEL) {
                short size = static_cast<short>(m_e.intAttribute("size"));
                String type = m_e.attribute("type");
                //LOGD("octave-shift type '%s' size %d number %d", muPrintable(type), size, n);
                MusicXmlOctaveShiftDesc osDesc;
                handleOctaveShift(cTime, type, size, osDesc);
                osDesc.num = n;
                if (osDesc.tp == MusicXmlOctaveShiftDesc::Type::UP
                    || osDesc.tp == MusicXmlOctaveShiftDesc::Type::DOWN) {
                    starts.push_back(osDesc);
                } else if (osDesc.tp == MusicXmlOctaveShiftDesc::Type::STOP) {
                    stops.push_back(osDesc);
                }
            } else {
                m_logger->logError(String(u"invalid octave-shift number %1").arg(number), &m_e);
            }
            m_e.skipCurrentElement();
        } else {
            m_e.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   handleOctaveShift
//---------------------------------------------------------

void MusicXmlParserPass1::handleOctaveShift(const Fraction& cTime,
                                            const String& type, short size,
                                            MusicXmlOctaveShiftDesc& desc)
{
    MusicXmlOctaveShiftDesc::Type tp = MusicXmlOctaveShiftDesc::Type::NONE;
    short sz = 0;

    switch (size) {
    case   8: sz =  1;
        break;
    case  15: sz =  2;
        break;
    default:
        m_logger->logError(String(u"invalid octave-shift size %1").arg(size), &m_e);
        return;
    }

    if (!cTime.isValid() || cTime < Fraction(0, 1)) {
        m_logger->logError(u"invalid current time", &m_e);
    }

    if (type == u"up") {
        tp = MusicXmlOctaveShiftDesc::Type::UP;
    } else if (type == u"down") {
        tp = MusicXmlOctaveShiftDesc::Type::DOWN;
        sz *= -1;
    } else if (type == u"stop") {
        tp = MusicXmlOctaveShiftDesc::Type::STOP;
    } else {
        m_logger->logError(String(u"invalid octave-shift type '%1'").arg(type), &m_e);
        return;
    }

    desc = MusicXmlOctaveShiftDesc(tp, sz, cTime);
}

//---------------------------------------------------------
//   notations
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/notations node.
 */

void MusicXmlParserPass1::notations(MusicXmlStartStop& tupletStartStop)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::note", &_e);

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "tuplet") {
            String tupletType = m_e.attribute("type");

            // ignore possible children (currently not supported)
            m_e.skipCurrentElement();

            if (tupletType == u"start") {
                tupletStartStop = MusicXmlStartStop::START;
            } else if (tupletType == u"stop") {
                tupletStartStop = MusicXmlStartStop::STOP;
            } else if (!tupletType.empty() && tupletType != u"start" && tupletType != u"stop") {
                m_logger->logError(String(u"unknown tuplet type '%1'").arg(tupletType), &m_e);
            }
        } else {
            m_e.skipCurrentElement();              // skip but don't log
        }
    }
}

//---------------------------------------------------------
//   voiceToInt
//---------------------------------------------------------
static int voiceStrIndex(const String& v)
{
    if (v.empty()) {
        return 0;
    }
    return v.at(0).unicode();
}

int MusicXmlParserPass1::voiceToInt(const String& voice)
{
    bool ok;
    int voiceInt = voice.toInt(&ok);
    if (voice.empty()) {
        voiceInt = 1;
    } else if (!ok) {
        voiceInt = voiceStrIndex(voice); // Handle the rare but techincally in-spec case of a non-int voice
    }
    return voiceInt;
}

//---------------------------------------------------------
//   note
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node.
 */

void MusicXmlParserPass1::note(const String& partId,
                               const Fraction& sTime,
                               Fraction& missingPrev,
                               Fraction& dura,
                               Fraction& missingCurr,
                               VoiceOverlapDetector& vod,
                               MusicXmlTupletStates& tupletStates)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::note", &_e);

    if (m_e.asciiAttribute("print-spacing") == "no") {
        notePrintSpacingNo(dura);
        return;
    }

    //float alter = 0;
    bool chord = false;
    bool grace = false;
    //int octave = -1;
    bool bRest = false;
    int staff = 0;
    //int step = 0;
    String type;
    String voice = u"1";
    String instrId;
    MusicXmlStartStop tupletStartStop { MusicXmlStartStop::NONE };

    MusicXmlNoteDuration mnd(m_divs, m_logger, this);

    while (m_e.readNextStartElement()) {
        if (mnd.readProperties(m_e)) {
            // element handled
        } else if (m_e.name() == "accidental") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "beam") {
            m_hasBeamingInfo = true;
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "chord") {
            chord = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "cue") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "grace") {
            grace = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "instrument") {
            instrId = m_e.attribute("id");
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "lyric") {
            const String number = m_e.attribute("number");
            m_parts[partId].lyricNumberHandler().addNumber(number);
            m_parts[partId].hasLyrics(true);
            m_e.skipCurrentElement();
        } else if (m_e.name() == "notations") {
            notations(tupletStartStop);
        } else if (m_e.name() == "notehead") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "pitch") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "rest") {
            bRest = true;
            rest();
        } else if (m_e.name() == "staff") {
            bool ok = false;
            String strStaff = m_e.readText();
            staff = m_parts[partId].staffNumberToIndex(strStaff.toInt(&ok));
            m_parts[partId].setMaxStaff(staff);
            Part* part = muse::value(m_partMap, partId);
            IF_ASSERT_FAILED(part) {
                continue;
            }
            if (!ok || staff < 0 || staff >= int(part->nstaves())) {
                m_logger->logError(String(u"illegal or hidden staff '%1'").arg(strStaff), &m_e);
            }
        } else if (m_e.name() == "stem") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "tie") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "type") {
            type = m_e.readText();
        } else if (m_e.name() == "unpitched") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "voice") {
            voice = m_e.readText();
        } else {
            skipLogCurrElem();
        }
    }

    // multi-instrument handling
    String prevInstrId = m_parts[partId]._instrList.instrument(sTime);
    bool mustInsert = instrId != prevInstrId;
    /*
    LOGD("tick %s (%d) staff %d voice '%s' previnst='%s' instrument '%s' mustInsert %d",
           muPrintable(sTime.print()),
           sTime.ticks(),
           staff + 1,
           muPrintable(voice),
           muPrintable(prevInstrId),
           muPrintable(instrId),
           mustInsert
           );
    */
    if (mustInsert) {
        m_parts[partId]._instrList.setInstrument(instrId, sTime);
    }

    // check for timing error(s) and set dura
    // keep in this order as checkTiming() might change dura
    String errorStr = mnd.checkTiming(type, bRest, grace);
    dura = mnd.duration();
    if (!errorStr.empty()) {
        m_logger->logError(errorStr, &m_e);
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    missingPrev.set(0, 1);
    if (chord || grace) {
        dura.set(0, 1);
    }

    if (!chord && !grace) {
        // do tuplet
        Fraction timeMod = mnd.timeMod();
        MusicXmlTupletState& tupletState = tupletStates[voice];
        tupletState.determineTupletAction(mnd.duration(), timeMod, tupletStartStop, mnd.normalType(), missingPrev, missingCurr);
    }

    // store result
    if (dura.isValid() && dura > Fraction(0, 1)) {
        // count the chords
        int voiceInt = voiceToInt(voice);
        if (!muse::contains(muse::value(m_parts, partId).voicelist, voiceInt)) {
            VoiceDesc vs;
            m_parts[partId].voicelist.insert({ voiceInt, vs });
        }
        m_parts[partId].voicelist[voiceInt].incrChordRests(staff);
        // determine note length for voiceInt overlap detection
        vod.addNote((sTime + missingPrev).ticks(), (sTime + missingPrev + dura).ticks(), voiceInt, staff);
    }

    addError(checkAtEndElement(m_e, u"note"));
}

//---------------------------------------------------------
//   notePrintSpacingNo
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node for a note with print-spacing="no".
 These are handled like a forward: only moving the time forward.
 */

void MusicXmlParserPass1::notePrintSpacingNo(Fraction& dura)
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
}

//---------------------------------------------------------
//   calcTicks
//---------------------------------------------------------

Fraction MusicXmlParserPass1::calcTicks(const int& intTicks, const int& _divisions, const XmlStreamReader* xmlReader)
{
    Fraction dura(0, 1);              // invalid unless set correctly

    if (_divisions > 0) {
        dura.set(intTicks, 4 * _divisions);
        dura.reduce(); // prevent overflow in later Fraction operations

        // Correct for previously adjusted durations
        // This is necessary when certain tuplets are
        // followed by a <backup> element.
        // There are two strategies:
        // 1. Use a lookup table of previous adjustments
        // 2. Check if within maxDiff of a seenDenominator
        if (muse::contains(m_adjustedDurations, dura)) {
            dura = m_adjustedDurations.at(dura);
        } else if (dura.reduced().denominator() > 64) {
            for (int seenDenominator : m_seenDenominators) {
                int seenDenominatorTicks = Fraction(1, seenDenominator).ticks();
                if (std::abs(dura.ticks() % seenDenominatorTicks) <= m_maxDiff) {
                    Fraction roundedDura = Fraction(std::round(dura.ticks() / double(seenDenominatorTicks)), seenDenominator);
                    roundedDura.reduce();
                    m_logger->logError(String(u"calculated duration (%1) assumed to be a rounding error by proximity to (%2)")
                                       .arg(dura.toString(), roundedDura.toString()));
                    insertAdjustedDuration(dura, roundedDura);
                    dura = roundedDura;
                    break;
                }
            }
        }
    } else {
        m_logger->logError(u"illegal or uninitialized divisions", xmlReader);
    }
    //qDebug("duration %s valid %d", muPrintable(dura.print()), dura.isValid());

    return dura;
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/duration node.
 */

void MusicXmlParserPass1::duration(Fraction& dura, muse::XmlStreamReader& e)
{
    DO_ASSERT(e.isStartElement() && e.name() == "duration");
    m_logger->logDebugTrace(u"MusicXmlParserPass1::duration", &e);

    dura.set(0, 0);    // invalid unless set correctly
    int intDura = e.readInt();
    dura = calcTicks(intDura);
}

//---------------------------------------------------------
//   forward
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/forward node.
 */

void MusicXmlParserPass1::forward(Fraction& dura)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::forward", &_e);

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

void MusicXmlParserPass1::backup(Fraction& dura)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::backup", &_e);

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "duration") {
            duration(dura);
        } else {
            skipLogCurrElem();
        }
    }
}

//---------------------------------------------------------
//   timeModification
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/time-modification node.
 */

void MusicXmlParserPass1::timeModification(Fraction& timeMod)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::timeModification", &_e);

    int intActual = 0;
    int intNormal = 0;
    String strActual;
    String strNormal;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "actual-notes") {
            strActual = m_e.readText();
        } else if (m_e.name() == "normal-notes") {
            strNormal = m_e.readText();
        } else {
            skipLogCurrElem();
        }
    }

    intActual = strActual.toInt();
    intNormal = strNormal.toInt();
    if (intActual > 0 && intNormal > 0) {
        timeMod.set(intNormal, intActual);
    } else {
        timeMod.set(1, 1);
        m_logger->logError(String(u"illegal time-modification: actual-notes %1 normal-notes %2")
                           .arg(strActual, strNormal), &m_e);
    }
}

//---------------------------------------------------------
//   rest
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note/rest node.
 */

void MusicXmlParserPass1::rest()
{
    //_logger->logDebugTrace("MusicXmlParserPass1::rest", &_e);

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "display-octave") {
            m_e.skipCurrentElement();        // skip but don't log
        } else if (m_e.name() == "display-step") {
            m_e.skipCurrentElement();        // skip but don't log
        } else {
            skipLogCurrElem();
        }
    }
}
} // namespace Ms
