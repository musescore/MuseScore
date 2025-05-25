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

#include "excerpt.h"

#include <list>

#include "containers.h"

#include "dom/partialtie.h"
#include "style/style.h"

#include "barline.h"
#include "beam.h"
#include "box.h"
#include "chord.h"
#include "factory.h"
#include "guitarbend.h"
#include "harmony.h"
#include "laissezvib.h"
#include "layoutbreak.h"
#include "linkedobjects.h"
#include "lyrics.h"
#include "masterscore.h"
#include "measure.h"
#include "note.h"
#include "ornament.h"
#include "page.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "staff.h"
#include "stafftype.h"
#include "system.h"
#include "text.h"
#include "textline.h"
#include "tie.h"
#include "tiemap.h"

#include "tremolotwochord.h"
#include "tuplet.h"
#include "tupletmap.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

Excerpt::Excerpt(const Excerpt& ex, bool copyContents)
    : m_masterScore(ex.m_masterScore), m_name(ex.m_name), m_parts(ex.m_parts), m_initialPartId(ex.m_initialPartId)
{
    if (copyContents) {
        m_tracksMapping = ex.m_tracksMapping;
        m_excerptScore = ex.m_excerptScore ? ex.m_excerptScore->clone() : nullptr;

        if (m_excerptScore) {
            m_excerptScore->setExcerpt(this);
        }
    }
}

Excerpt::~Excerpt()
{
    delete m_excerptScore;
}

bool Excerpt::inited() const
{
    return m_inited;
}

void Excerpt::setInited(bool inited)
{
    m_inited = inited;
}

bool Excerpt::custom() const
{
    return !m_initialPartId.isValid();
}

void Excerpt::markAsCustom()
{
    setInitialPartId(ID());
}

const ID& Excerpt::initialPartId() const
{
    return m_initialPartId;
}

void Excerpt::setInitialPartId(const ID& id)
{
    m_initialPartId = id;
}

void Excerpt::setExcerptScore(Score* s)
{
    m_excerptScore = s;

    if (s) {
        s->setExcerpt(this);
        writeNameToMetaTags();
    }
}

const String& Excerpt::name() const
{
    return m_name;
}

void Excerpt::setName(const String& name, bool saveAndNotify)
{
    if (m_name == name) {
        return;
    }

    m_name = name;

    if (saveAndNotify) {
        writeNameToMetaTags();
        m_nameChanged.notify();
    }
}

void Excerpt::writeNameToMetaTags()
{
    if (Score* score = excerptScore()) {
        if (Text* nameItem = score->getText(mu::engraving::TextStyleType::INSTRUMENT_EXCERPT)) {
            nameItem->setPlainText(m_name);
            score->setMetaTag(u"partName", m_name);
        }
    }
}

muse::async::Notification Excerpt::nameChanged() const
{
    return m_nameChanged;
}

bool Excerpt::hasFileName() const
{
    return !m_fileName.empty();
}

const String& Excerpt::fileName() const
{
    IF_ASSERT_FAILED(!m_fileName.empty()) {
        const_cast<Excerpt*>(this)->updateFileName();
    }

    return m_fileName;
}

void Excerpt::setFileName(const String& fileName)
{
    m_fileName = fileName;
}

void Excerpt::updateFileName(size_t index)
{
    if (index == muse::nidx && m_masterScore) {
        index = muse::indexOf(m_masterScore->excerpts(), this);
    }

    const String escapedName = muse::io::escapeFileName(m_name).toString();

    if (index == muse::nidx) {
        m_fileName = escapedName;
    } else {
        m_fileName = String(u"%1_%2").arg(String::number(index), escapedName);
    }
}

bool Excerpt::containsPart(const Part* part) const
{
    for (Part* _part : m_parts) {
        if (_part == part) {
            return true;
        }
    }

    return false;
}

void Excerpt::removePart(const ID& id)
{
    size_t index = 0;
    for (const Part* part: parts()) {
        if (part->id() == id) {
            break;
        }
        ++index;
    }
    if (index >= m_parts.size()) {
        return;
    }

    excerptScore()->undoRemovePart(excerptScore()->parts().at(index), index);
}

size_t Excerpt::nstaves() const
{
    size_t n = 0;
    for (Part* p : m_parts) {
        n += p->nstaves();
    }
    return n;
}

bool Excerpt::isEmpty() const
{
    return excerptScore() ? excerptScore()->parts().empty() : true;
}

const TracksMap& Excerpt::tracksMapping()
{
    updateTracksMapping();

    return m_tracksMapping;
}

void Excerpt::setTracksMapping(const TracksMap& tracksMapping)
{
    if (m_tracksMapping == tracksMapping) {
        return;
    }

    m_tracksMapping = tracksMapping;

    const Score* score = excerptScore();
    if (!score) {
        return;
    }

    for (Staff* staff : score->staves()) {
        const Staff* masterStaff = m_masterScore->staffById(staff->id());
        if (!masterStaff) {
            continue;
        }

        staff->updateVisibilityVoices(masterStaff, m_tracksMapping);
    }
}

void Excerpt::updateTracksMapping()
{
    const Score* score = excerptScore();
    if (!score) {
        return;
    }

    TracksMap tracks;

    TRACEFUNC;

    for (const Staff* staff : score->staves()) {
        const Staff* masterStaff = masterScore()->staffById(staff->id());
        if (!masterStaff) {
            continue;
        }

        staff_idx_t masterStaffIdx = masterStaff->idx();
        staff_idx_t staffIdx = staff->idx();

        voice_idx_t voice = 0;
        const std::array<bool, VOICES>& voicesVisibility = staff->visibilityVoices();

        for (voice_idx_t i = 0; i < VOICES; ++i) {
            if (!voicesVisibility[i]) {
                continue;
            }

            tracks.insert({ masterStaffIdx* VOICES + i % VOICES, staffIdx * VOICES + voice % VOICES });
            voice++;
        }
    }

    setTracksMapping(tracks);
}

void Excerpt::setVoiceVisible(Staff* staff, voice_idx_t voiceIndex, bool visible)
{
    TRACEFUNC;

    if (!staff) {
        return;
    }

    Staff* masterStaff = masterScore()->staffById(staff->id());
    if (!masterStaff) {
        return;
    }

    bool wasVisible = staff->isVoiceVisible(voiceIndex);
    if (visible == wasVisible) {
        return;
    }

    // update tracks (temporarily modify the old staff)
    staff->setVoiceVisible(voiceIndex, visible);
    updateTracksMapping();

    // create new staff
    Staff* newStaff = Factory::createStaff(staff->part());
    newStaff->setId(staff->id());
    newStaff->init(staff);

    // revert changes to old staff, to make undo work properly
    staff->setVoiceVisible(voiceIndex, wasVisible);

    // remove old staff, insert new
    staff_idx_t staffIndex = staff->idx();
    staff_idx_t partStaffIndex = staffIndex - excerptScore()->staffIdx(staff->part());
    excerptScore()->undoRemoveStaff(staff);
    excerptScore()->undoInsertStaff(newStaff, partStaffIndex);

    // clone master staff to new staff with mapped tracks
    Fraction startTick = staff->score()->firstMeasure()->tick();
    Fraction endTick = staff->score()->lastMeasure()->endTick();
    cloneStaff2(masterStaff, newStaff, startTick, endTick);

    // link master staff to cloned
    excerptScore()->undo(new Link(newStaff, masterStaff));
}

void Excerpt::createExcerpt(Excerpt* excerpt)
{
    MasterScore* masterScore = excerpt->masterScore();
    Score* score = excerpt->excerptScore();

    std::vector<Part*>& parts = excerpt->parts();
    std::vector<staff_idx_t> srcStaves;

    score->setPageNumberOffset(masterScore->pageNumberOffset());

    // Set instruments and create linked staves
    for (const Part* part : parts) {
        Part* p = new Part(score);
        p->setId(part->id());
        p->setInstrument(*part->instrument());
        p->setPartName(part->partName());
        p->setPreferSharpFlat(part->preferSharpFlat());

        for (Staff* staff : part->staves()) {
            Staff* s = Factory::createStaff(p);
            s->setId(staff->id());
            s->init(staff);
            s->setDefaultClefType(staff->defaultClefType());
            // the order of staff - s matters as staff should be the first entry in the
            // created link list to make primaryStaff() work
            // TODO: change implementation, maybe create an explicit "primary" flag
            score->undo(new Link(s, staff));
            score->appendStaff(s);
            srcStaves.push_back(staff->idx());
        }
        score->appendPart(p);
    }

    // Fill tracklist (map all tracks of a stave)
    if (excerpt->tracksMapping().empty()) {
        excerpt->updateTracksMapping();
    }

    cloneStaves(masterScore, score, srcStaves, excerpt->tracksMapping());

    MeasureBase* masterMeasure = masterScore->first();
    MeasureBase* scoreMeasure = score->first();

    if ((!scoreMeasure || !scoreMeasure->isVBox()) && (!masterMeasure || !masterMeasure->excludeFromOtherParts())) {
        score->insertBox(ElementType::VBOX, scoreMeasure);
        scoreMeasure = score->first();
    }

    VBox* titleFrameScore = scoreMeasure && scoreMeasure->isVBox() ? toVBox(scoreMeasure) : nullptr;

    if (titleFrameScore && masterMeasure && masterMeasure->isVBox()) {
        VBox* titleFrameMaster = toVBox(masterMeasure);

        titleFrameScore->copyValues(titleFrameMaster);
    }

    String partLabel = excerpt->name();
    if (!partLabel.empty()) {
        if (titleFrameScore) {
            Text* txt = Factory::createText(titleFrameScore, TextStyleType::INSTRUMENT_EXCERPT);
            txt->setPlainText(partLabel);
            titleFrameScore->add(txt);
        }

        score->setMetaTag(u"partName", partLabel);
    }

    // initial layout of score
    score->doLayout();

    // handle transposing instruments
    if (masterScore->style().styleB(Sid::concertPitch) != score->style().styleB(Sid::concertPitch)) {
        for (const Staff* staff : score->staves()) {
            if (staff->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {
                continue;
            }

            // if this staff has no transposition, and no instrument changes, we can skip it
            Interval interval = staff->part()->instrument()->transpose(); //tick?
            if (interval.isZero() && staff->part()->instruments().size() == 1) {
                continue;
            }
            bool flip = false;
            if (masterScore->style().styleB(Sid::concertPitch)) {
                interval.flip();          // flip the transposition for the original instrument
                flip = true;              // transposeKeys() will flip transposition for each instrument change
            }

            staff_idx_t staffIdx   = staff->idx();
            track_idx_t startTrack = staffIdx * VOICES;
            track_idx_t endTrack   = startTrack + VOICES;

            Fraction endTick = Fraction(0, 1);
            if (score->lastSegment()) {
                endTick = score->lastSegment()->tick();
            }
            score->transposeKeys(staffIdx, staffIdx + 1, Fraction(0, 1), endTick, flip);

            for (auto segment = score->firstSegmentMM(SegmentType::ChordRest); segment;
                 segment = segment->next1MM(SegmentType::ChordRest)) {
                interval = staff->transpose(segment->tick());
                if (interval.isZero()) {
                    continue;
                }
                if (masterScore->style().styleB(Sid::concertPitch)) {
                    interval.flip();
                }

                for (auto e : segment->annotations()) {
                    if (!e->isHarmony() || (e->track() < startTrack) || (e->track() >= endTrack)) {
                        continue;
                    }
                    Harmony* h  = toHarmony(e);
                    int rootTpc = mu::engraving::transposeTpc(h->rootTpc(), interval, true);
                    int baseTpc = mu::engraving::transposeTpc(h->bassTpc(), interval, true);
                    // mmrests are on by default in part
                    // if this harmony is attached to an mmrest,
                    // be sure to transpose harmony in underlying measure as well
                    for (EngravingObject* se : h->linkList()) {
                        Harmony* hh = static_cast<Harmony*>(se);
                        // skip links to other staves (including in other scores)
                        if (hh->staff() != h->staff()) {
                            continue;
                        }
                        score->undoTransposeHarmony(hh, rootTpc, baseTpc);
                    }
                }
            }
        }
    }

    // update style values if spatium different for part
    if (masterScore->style().spatium() != score->style().spatium()) {
        score->spatiumChanged(masterScore->style().spatium(), score->style().spatium());
        score->styleChanged();
    }

    // second layout of score
    score->setPlaylistDirty();
    masterScore->rebuildMidiMapping();
    masterScore->updateChannel();
    score->remapBracketsAndBarlines();

    score->setLayoutAll();
    score->doLayout();
}

void MasterScore::deleteExcerpt(Excerpt* excerpt)
{
    assert(excerpt->masterScore() == this);
    Score* partScore = excerpt->excerptScore();

    if (!partScore) {
        LOGD("deleteExcerpt: no partScore");
        return;
    }

    // unlink the staves in the excerpt
    for (Staff* st : partScore->staves()) {
        bool hasLinksInMaster = false;
        if (st->links()) {
            for (auto le : *st->links()) {
                if (le->score() == this) {
                    hasLinksInMaster = true;
                    break;
                }
            }
        }
        if (hasLinksInMaster) {
            staff_idx_t staffIdx = st->idx();
            // unlink the spanners
            for (auto i = partScore->spanner().begin(); i != partScore->spanner().cend(); ++i) {
                Spanner* sp = i->second;
                if (sp->staffIdx() == staffIdx) {
                    sp->undoUnlink();
                }
            }
            track_idx_t sTrack = staffIdx * VOICES;
            track_idx_t eTrack = sTrack + VOICES;
            // unlink elements and annotation
            for (Segment* s = partScore->firstSegmentMM(SegmentType::All); s; s = s->next1MM()) {
                for (int track = static_cast<int>(eTrack) - 1; track >= static_cast<int>(sTrack); --track) {
                    EngravingItem* el = s->element(track);
                    if (el) {
                        el->undoUnlink();
                    }
                }
                for (EngravingItem* e : s->annotations()) {
                    if (e->staffIdx() == staffIdx) {
                        e->undoUnlink();
                    }
                }
            }
            // unlink the staff
            undo(new Unlink(st));
        }
    }
    undo(new RemoveExcerpt(excerpt));
}

void MasterScore::initAndAddExcerpt(Excerpt* excerpt, bool fakeUndo)
{
    initExcerpt(excerpt);

    auto excerptCmd = new AddExcerpt(excerpt);
    if (fakeUndo) {
        excerptCmd->redo(nullptr);
    } else {
        excerpt->excerptScore()->undo(excerptCmd);
    }
}

void MasterScore::initExcerpt(Excerpt* excerpt)
{
    if (excerpt->inited()) {
        excerpt->excerptScore()->doLayout();
        return;
    }

    Score* score = new Score(masterScore());
    excerpt->setExcerptScore(score);
    score->style().set(Sid::createMultiMeasureRests, true);
    initParts(excerpt);

    Excerpt::createExcerpt(excerpt);
    excerpt->setInited(true);

    if (!score->style().isDefault(Sid::timeSigPlacement)) {
        score->resetStyleValue(Sid::timeSigPlacement);
    }
}

void MasterScore::initParts(Excerpt* excerpt)
{
    int nstaves { 1 }; // Initialise to 1 to force writing of the first part.
    std::set<ID> assignedStavesIds;
    for (Staff* excerptStaff : excerpt->excerptScore()->staves()) {
        const LinkedObjects* ls = excerptStaff->links();
        if (ls == 0) {
            continue;
        }

        for (auto le : *ls) {
            if (le->score() != this) {
                continue;
            }

            Staff* linkedMasterStaff = toStaff(le);
            if (muse::contains(assignedStavesIds, linkedMasterStaff->id())) {
                continue;
            }

            Part* excerptPart = excerptStaff->part();
            Part* masterPart = linkedMasterStaff->part();

            //! NOTE: parts/staves of excerpt must have the same ID as parts/staves of the master score
            //! In fact, excerpts are just viewers for the master score
            excerptStaff->setId(linkedMasterStaff->id());
            excerptPart->setId(masterPart->id());

            assignedStavesIds.insert(linkedMasterStaff->id());

            // For instruments with multiple staves, every staff will point to the
            // same part. To prevent adding the same part several times to the excerpt,
            // add only the part of the first staff pointing to the part.
            if (!(--nstaves)) {
                excerpt->parts().push_back(linkedMasterStaff->part());
                nstaves = static_cast<int>(linkedMasterStaff->part()->nstaves());
            }
            break;
        }
    }

    if (excerpt->tracksMapping().empty()) {   // SHOULDN'T HAPPEN, protected in the UI, but it happens during read-in!!!
        excerpt->updateTracksMapping();
    }
}

void MasterScore::initEmptyExcerpt(Excerpt* excerpt)
{
    if (excerpt->inited()) {
        return;
    }

    Excerpt::cloneMeasures(this, excerpt->excerptScore());
    excerpt->setInited(true);
}

static bool scoreContainsSpanner(const Score* score, Spanner* spanner)
{
    const std::multimap<int, Spanner*>& spanners = score->spanner();

    for (auto it = spanners.cbegin(); it != spanners.cend(); ++it) {
        const Spanner* curSpanner = it->second;
        if (curSpanner->links() && curSpanner->links()->contains(spanner)) {
            return true;
        }
    }

    return false;
}

void Excerpt::cloneSpanner(Spanner* s, Score* score, track_idx_t dstTrack, track_idx_t dstTrack2)
{
    // don’t clone system lines for track != 0
    if (isSystemTextLine(s) && s->track() != 0) {
        return;
    }

    Spanner* ns = toSpanner(s->linkedClone());
    ns->setScore(score);
    ns->moveToDummy();
    ns->setTrack(dstTrack);
    ns->setTrack2(dstTrack2);

    if (ns->isSlur()) {
        // set start/end element for slur
        ChordRest* cr1 = s->startCR();
        ChordRest* cr2 = s->endCR();

        ns->setStartElement(0);
        ns->setEndElement(0);
        if (cr1 && cr1->links()) {
            for (EngravingObject* e : *cr1->links()) {
                ChordRest* cr = toChordRest(e);
                if (cr == cr1) {
                    continue;
                }
                if ((cr->score() == score) && (cr->tick() == ns->tick()) && cr->track() == dstTrack) {
                    ns->setStartElement(cr);
                    break;
                }
            }
        }
        if (cr2 && cr2->links()) {
            for (EngravingObject* e : *cr2->links()) {
                ChordRest* cr = toChordRest(e);
                if (cr == cr2) {
                    continue;
                }
                if ((cr->score() == score) && (cr->tick() == ns->tick2()) && cr->track() == dstTrack2) {
                    ns->setEndElement(cr);
                    break;
                }
            }
        }
        if (!ns->startElement()) {
            LOGD("clone Slur: no start element");
        }
        if (!ns->endElement()) {
            LOGD("clone Slur: no end element");
        }
    } else if (ns->isTrill()) {
        ns->computeStartElement();
    } else {
        if (!ns->startElement()) {
            ns->computeStartElement();
        }
        if (!ns->endElement()) {
            ns->computeEndElement();
        }
    }

    if (!ns->startElement() || !ns->endElement()) {
        if (EngravingItem* startElement = ns->startElement()) {
            if (startElement->isChord()) {
                toChord(startElement)->removeStartingSpanner(ns);
            }
        }
        if (EngravingItem* endElement = ns->endElement()) {
            if (endElement->isChord()) {
                toChord(endElement)->removeEndingSpanner(ns);
            }
        }
        LOGD() << "No start or end element, can't add spanner: " << ns->tick().toString();
        delete ns;
        return;
    }

    score->doUndoAddElement(ns);
    ns->styleChanged();
}

static void updateSpatium(void* oldElement, EngravingItem* newElement)
{
    double oldSpatium = static_cast<EngravingItem*>(oldElement)->spatium();
    double newSpatium = newElement->spatium();
    if (!muse::RealIsEqual(oldSpatium, newSpatium)) {
        newElement->spatiumChanged(oldSpatium, newSpatium);
    }
}

static void cloneTuplets(ChordRest* ocr, ChordRest* ncr, Tuplet* ot, TupletMap& tupletMap, Measure* nm, track_idx_t track)
{
    const auto handleTuplet = [&](Tuplet* tuplet) {
        tuplet->clear();
        tuplet->setTrack(track);
        tuplet->setParent(nm);
        tuplet->styleChanged();
        tuplet->scanElements(ot, updateSpatium);
    };

    ot->setTrack(ocr->track());

    Tuplet* nt = tupletMap.findNew(ot);
    if (!nt) {
        nt = toTuplet(ot->linkedClone());
        handleTuplet(nt);
        tupletMap.add(ot, nt);

        Tuplet* nt1 = nt;
        while (ot->tuplet()) {
            Tuplet* nt2 = tupletMap.findNew(ot->tuplet());
            if (nt2 == 0) {
                nt2 = toTuplet(ot->tuplet()->linkedClone());
                handleTuplet(nt2);
                tupletMap.add(ot->tuplet(), nt2);
            }
            nt2->add(nt1);
            nt1->setTuplet(nt2);
            ot = ot->tuplet();
            nt1 = nt2;
        }
    }
    nt->add(ncr);
    ncr->setTuplet(nt);
}

static void processLinkedClone(EngravingItem* ne, Score* score, track_idx_t strack)
{
    // reset offset as most likely it will not fit
    PropertyFlags f = ne->propertyFlags(Pid::OFFSET);
    if (f == PropertyFlags::UNSTYLED) {
        ne->setPropertyFlags(Pid::OFFSET, PropertyFlags::STYLED);
        ne->resetProperty(Pid::OFFSET);
    }
    ne->setTrack(strack == muse::nidx ? 0 : strack);
    ne->setScore(score);
}

static void addTies(Note* originalNote, Note* newNote, TieMap& tieMap, Score* score)
{
    if (originalNote->tieFor() && originalNote->tieFor()->type() == ElementType::TIE) {
        Tie* tie = toTie(originalNote->tieFor()->linkedClone());
        tie->setScore(score);
        newNote->setTieFor(tie);
        tie->setStartNote(newNote);
        tie->setTrack(newNote->track());
        tieMap.add(originalNote->tieFor(), tie);
    }
    if (originalNote->tieBack() && originalNote->tieBack()->type() == ElementType::TIE) {
        Tie* tie = tieMap.findNew(originalNote->tieBack());
        if (tie) {
            newNote->setTieBack(tie);
            tie->setEndNote(newNote);
            tie->setTrack2(newNote->track());
        } else {
            LOGD("addTiesToMap: cannot find tie");
        }
    }

    if (originalNote->outgoingPartialTie()) {
        tieMap.add(originalNote->outgoingPartialTie(), newNote->outgoingPartialTie());
    }
}

static void addGraceNoteTies(GraceNotesGroup& originalGraceNotes, Chord* newChord, TieMap& tieMap, Score* score)
{
    for (Chord* oldGrace : originalGraceNotes) {
        Chord* newGrace = newChord->graceNoteAt(oldGrace->graceIndex());
        if (!newGrace) {
            continue;
        }

        size_t notes = oldGrace->notes().size();
        for (size_t i = 0; i < notes; ++i) {
            Note* originalNote = oldGrace->notes().at(i);
            Note* newNote = newGrace->notes().at(i);

            addTies(originalNote, newNote, tieMap, score);
        }
    }
}

static void addTremoloTwoChord(Chord* oldChord, Chord* newChord, TremoloTwoChord*& prevTremolo)
{
    if (!newChord || !oldChord || !newChord->tremoloTwoChord()) {
        return;
    }

    if (oldChord == oldChord->tremoloTwoChord()->chord1()) {
        prevTremolo = newChord->tremoloTwoChord();
    } else if (oldChord == oldChord->tremoloTwoChord()->chord2()) {
        if (!prevTremolo) {
            LOGD("first note for two note tremolo missing");
        } else {
            prevTremolo->setChords(prevTremolo->chord1(), newChord);
            newChord->setTremoloTwoChord(prevTremolo);
        }
    } else {
        LOGD("inconsistent two note tremolo");
    }
}

static void collectTieEndPoints(TieMap& tieMap)
{
    for (auto& tie : tieMap) {
        Tie* newTie = toTie(tie.second);
        if (newTie->type() == ElementType::TIE || (newTie->type() == ElementType::PARTIAL_TIE && toPartialTie(newTie)->isOutgoing())) {
            newTie->updatePossibleJumpPoints();
        }
    }
}

static MeasureBase* cloneMeasure(MeasureBase* mb, Score* score, const Score* oscore,
                                 const std::vector<staff_idx_t>& sourceStavesIndexes,
                                 const TracksMap& trackList, TieMap& tieMap)
{
    MeasureBase* nmb = nullptr;

    if (mb->isHBox()) {
        nmb = Factory::createHBox(score->dummy()->system());
        nmb->setTick(mb->tick());
        nmb->setTicks(mb->ticks());
    } else if (mb->isVBox()) {
        if (toBox(mb)->isTitleFrame()) {
            nmb = Factory::createTitleVBox(score->dummy()->system());
        } else {
            nmb = Factory::createVBox(score->dummy()->system());
        }
        nmb->setTick(mb->tick());
        nmb->setTicks(mb->ticks());
    } else if (mb->isTBox()) {
        nmb = Factory::createTBox(score->dummy()->system());
        nmb->setTick(mb->tick());
        nmb->setTicks(mb->ticks());
        Text* text = toTBox(mb)->text();
        EngravingItem* ne = text->linkedClone();
        ne->setScore(score);
        nmb->add(ne);
    } else if (mb->isMeasure()) {
        const Measure* m  = toMeasure(mb);
        Measure* nm = Factory::createMeasure(score->dummy()->system());
        nmb = nm;
        nm->setTick(m->tick());
        nm->setTicks(m->ticks());
        nm->setTimesig(m->timesig());

        nm->setRepeatCount(m->repeatCount());
        nm->setRepeatStart(m->repeatStart());
        nm->setRepeatEnd(m->repeatEnd());
        nm->setRepeatJump(m->repeatJump());

        nm->setIrregular(m->irregular());
        nm->setNo(m->no());
        nm->setNoOffset(m->noOffset());
        nm->setBreakMultiMeasureRest(m->breakMultiMeasureRest());

        for (staff_idx_t dstStaffIdx = 0; dstStaffIdx < sourceStavesIndexes.size(); ++dstStaffIdx) {
            nm->setStaffStemless(dstStaffIdx, m->stemless(sourceStavesIndexes[dstStaffIdx]));
        }
        size_t tracks = oscore->nstaves() * VOICES;

        if (sourceStavesIndexes.empty() && trackList.empty()) {
            tracks = 0;
        }

        for (track_idx_t srcTrack = 0; srcTrack < tracks; ++srcTrack) {
            TupletMap tupletMap;            // tuplets cannot cross measure boundaries

            track_idx_t strack = muse::value(trackList, srcTrack, muse::nidx);

            TremoloTwoChord* prevTremolo = nullptr;
            for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                Segment* ns = nullptr;           //create segment later, on demand
                for (EngravingItem* e : oseg->annotations()) {
                    if (e->generated() || e->excludeFromOtherParts()) {
                        continue;
                    }
                    if ((e->track() == srcTrack && strack != muse::nidx && !e->systemFlag())
                        || (e->systemFlag() && srcTrack == 0 && e->track() == srcTrack)) {
                        EngravingItem* ne = e->linkedClone();
                        processLinkedClone(ne, score, strack);
                        if (!ns) {
                            ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                        }
                        ns->add(ne);
                        // for chord symbols,
                        // re-render with new style settings
                        if (ne->isHarmony()) {
                            Harmony* h = toHarmony(ne);
                            h->render();
                        } else if (ne->isFretDiagram()) {
                            Harmony* h = toHarmony(toFretDiagram(ne)->harmony());
                            if (h) {
                                processLinkedClone(h, score, strack);
                                h->render();
                            }
                        }
                    }
                }

                //If track is not mapped skip the following
                if (muse::value(trackList, srcTrack, muse::nidx) == muse::nidx) {
                    continue;
                }

                // TimeSig and KeySig announce should never be cloned
                if (oseg->isTimeSigAnnounceType() || oseg->isKeySigAnnounceType()) {
                    continue;
                }

                //There are probably more destination tracks for the same source
                std::vector<track_idx_t> t = muse::values(trackList, srcTrack);

                for (track_idx_t track : t) {
                    //Clone KeySig TimeSig and Clefs if voice 1 of source staff is not mapped to a track
                    EngravingItem* oef = oseg->element(trackZeroVoice(srcTrack));
                    if (oef && !oef->generated() && (oef->isTimeSig() || oef->isKeySig())
                        && !(trackList.size() == (score->excerpt()->nstaves() * VOICES))) {
                        EngravingItem* ne = oef->linkedClone();
                        ne->setTrack(trackZeroVoice(track));
                        ne->setScore(score);
                        ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                        ns->add(ne);
                    }

                    EngravingItem* oe = oseg->element(srcTrack);
                    int adjustedBarlineSpan = 0;
                    if (srcTrack % VOICES == 0 && oseg->segmentType() == SegmentType::BarLine) {
                        // mid-measure barline segment
                        // may need to clone barline from a previous staff and/or adjust span
                        int oIdx = static_cast<int>(srcTrack / VOICES);
                        if (!oe) {
                            // no barline on this staff in original score,
                            // but check previous staves
                            for (int i = oIdx - 1; i >= 0; --i) {
                                oe = oseg->element(i * VOICES);
                                if (oe) {
                                    break;
                                }
                            }
                        }
                        if (oe) {
                            // barline found, now check span
                            BarLine* bl = toBarLine(oe);
                            int oSpan1 = static_cast<int>(bl->staff()->idx());
                            int oSpan2 = static_cast<int>(oSpan1 + bl->spanStaff());
                            if (oSpan1 <= oIdx && oIdx <= oSpan2) {
                                // this staff is within span
                                // calculate adjusted span for excerpt
                                int oSpan = oSpan2 - oIdx;
                                adjustedBarlineSpan = std::min(oSpan, static_cast<int>(score->nstaves()));
                            } else {
                                // this staff is not within span
                                oe = nullptr;
                            }
                        }
                    }

                    if (oe && !oe->generated() && !oe->excludeFromOtherParts()) {
                        EngravingItem* ne;
                        ne = oe->linkedClone();
                        ne->setTrack(track);

                        if (!(ne->track() % VOICES) && ne->isRest()) {
                            toRest(ne)->setGap(false);
                        }

                        ne->setScore(score);
                        if (oe->isBarLine() && adjustedBarlineSpan) {
                            BarLine* nbl = toBarLine(ne);
                            nbl->setSpanStaff(adjustedBarlineSpan);
                        } else if (oe->isChordRest()) {
                            ChordRest* ocr = toChordRest(oe);
                            ChordRest* ncr = toChordRest(ne);

                            if (ocr->beam() && !ocr->beam()->empty() && ocr->beam()->elements().front() == ocr) {
                                Beam* nb = ocr->beam()->clone();
                                nb->clear();
                                nb->setTrack(track);
                                nb->setParent(nm->system());
                                nb->setScore(score);
                                nb->add(ncr);
                                ncr->setBeam(nb);
                            }

                            Tuplet* ot = ocr->tuplet();

                            if (ot) {
                                cloneTuplets(ocr, ncr, ot, tupletMap, nm, track);
                            }

                            if (oe->isChord()) {
                                Chord* och = toChord(ocr);
                                Chord* nch = toChord(ncr);
                                addGraceNoteTies(och->graceNotesBefore(), nch, tieMap, score);
                                size_t n = och->notes().size();
                                for (size_t i = 0; i < n; ++i) {
                                    Note* on = och->notes().at(i);
                                    Note* nn = nch->notes().at(i);

                                    addTies(on, nn, tieMap, score);
                                    // add back spanners (going back from end to start spanner element
                                    // makes sure the 'other' spanner anchor element is already set up)
                                    // 'on' is the old spanner end note and 'nn' is the new spanner end note
                                    for (Spanner* oldSp : on->spannerBack()) {
                                        if (oldSp->startElement() && oldSp->endElement()
                                            && oldSp->startElement()->track() > oldSp->endElement()->track()) {
                                            continue;
                                        }
                                        Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                        if (newStart != nullptr) {
                                            Spanner* newSp = toSpanner(oldSp->linkedClone());
                                            newSp->setNoteSpan(newStart, nn);
                                            score->addElement(newSp);
                                        } else {
                                            LOGD("cloneStaves: cannot find spanner start note");
                                        }
                                    }
                                    for (Spanner* oldSp : on->spannerFor()) {
                                        if (oldSp->startElement() && oldSp->endElement()
                                            && oldSp->startElement()->track() <= oldSp->endElement()->track()) {
                                            continue;
                                        }
                                        Note* newEnd = Spanner::endElementFromSpanner(oldSp, nn);
                                        if (newEnd != nullptr) {
                                            Spanner* newSp = toSpanner(oldSp->linkedClone());
                                            newSp->setNoteSpan(nn, newEnd);
                                            score->addElement(newSp);
                                        } else {
                                            LOGD("cloneStaves: cannot find spanner end note");
                                        }
                                    }
                                }
                                addGraceNoteTies(och->graceNotesAfter(), nch, tieMap, score);
                                addTremoloTwoChord(och, nch, prevTremolo);
                            }
                        }
                        if (!ns) {
                            ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                        }
                        ns->add(ne);
                    }

                    Segment* tst = nm->segments().firstCRSegment();
                    if (srcTrack % VOICES && !(track % VOICES) && (!tst || (!tst->element(track)))) {
                        Segment* segment = nm->getSegment(SegmentType::ChordRest, nm->tick());
                        Rest* rest = Factory::createRest(segment);
                        rest->setTicks(nm->ticks());
                        rest->setDurationType(nm->ticks());
                        rest->setTrack(track);
                        segment->add(rest);
                    }
                }
            }
        }
    }

    score->undo(new Link(nmb, mb));
    nmb->setExcludeFromOtherParts(false);

    for (EngravingItem* e : mb->el()) {
        if (e->isLayoutBreak()) {
            LayoutBreakType st = toLayoutBreak(e)->layoutBreakType();
            if (st != LayoutBreakType::SECTION) {
                continue;
            }
        }
        track_idx_t track = muse::nidx;
        if (e->track() != muse::nidx) {
            // try to map track
            track = muse::value(trackList, e->track(), muse::nidx);
            if (e->systemFlag() && !e->isTopSystemObject()) {
                continue;
            }
            if (track == muse::nidx) {
                // even if track not in excerpt, we need to clone system elements
                if (e->systemFlag() && e->track() == 0) {
                    track = 0;
                } else {
                    continue;
                }
            }
        }

        EngravingItem* ne;
        // link text - title, subtitle, also repeats (eg, coda/segno)
        // measure numbers are not stored in this list, but they should not be cloned anyhow
        // layout breaks other than section were skipped above,
        // but section breaks do need to be cloned & linked
        // other measure-attached elements (?) are cloned but not linked
        if (e->isText() && toText(e)->textStyleType() == TextStyleType::INSTRUMENT_EXCERPT) {
            // skip part name in score
            continue;
        } else if (e->isTextBase() || e->isLayoutBreak()) {
            ne = e->linkedClone();
            ne->setAutoplace(true);
        } else {
            ne = e->clone();
        }
        ne->setScore(score);
        ne->setTrack(track);
        nmb->add(ne);
    }

    return nmb;
}

void Excerpt::cloneStaves(Score* sourceScore, Score* dstScore, const std::vector<staff_idx_t>& sourceStavesIndexes,
                          const TracksMap& trackList)
{
    MeasureBaseList* measures = dstScore->measures();
    TieMap tieMap;

    for (MeasureBase* mb = sourceScore->measures()->first(); mb; mb = mb->next()) {
        if (mb->excludeFromOtherParts()) {
            // if excluded measure contains section break, add it to precedent measure in part
            if (LayoutBreak* sectionBreak = mb->sectionBreakElement()) {
                if (MeasureBase* prevMB = measures->last()) {
                    EngravingItem* newSectionBreak = sectionBreak->linkedClone();
                    newSectionBreak->setScore(dstScore);
                    newSectionBreak->setParent(prevMB);
                    dstScore->doUndoAddElement(newSectionBreak);
                }
            }
            continue;
        }
        MeasureBase* newMeasure = cloneMeasure(mb, dstScore, sourceScore, sourceStavesIndexes, trackList, tieMap);
        measures->append(newMeasure);
    }

    size_t n = sourceStavesIndexes.size();
    for (staff_idx_t dstStaffIdx = 0; dstStaffIdx < n; ++dstStaffIdx) {
        Staff* srcStaff = sourceScore->staff(sourceStavesIndexes[dstStaffIdx]);

        Measure* m = sourceScore->firstMeasure();
        Measure* nm = dstScore->firstMeasure();

        while (m && nm) {
            nm->setMeasureRepeatCount(m->measureRepeatCount(srcStaff->idx()), dstStaffIdx);
            m = m->nextMeasure();
            nm = nm->nextMeasure();
        }
    }

    if (dstScore->noStaves()) {
        return;
    }

    for (auto i : sourceScore->spanner()) {
        Spanner* s    = i.second;
        if (s->excludeFromOtherParts()) {
            continue;
        }

        track_idx_t dstTrack  = muse::nidx;
        track_idx_t dstTrack2 = muse::nidx;

        if (isSystemTextLine(s)) {
            //always export voltas to first staff in part
            dstTrack  = 0;
            dstTrack2 = 0;
            cloneSpanner(s, dstScore, dstTrack, dstTrack2);
        } else {
            if (muse::value(trackList, s->track(), muse::nidx) == muse::nidx
                || muse::value(trackList, s->track2(), muse::nidx) == muse::nidx) {
                continue;
            }
            std::vector<track_idx_t> track1 = muse::values(trackList, s->track());
            std::vector<track_idx_t> track2 = muse::values(trackList, s->track2());

            if (track1.size() != track2.size()) {
                continue;
            }

            //export other spanner if staffidx matches
            for (auto it1 = track1.begin(), it2 = track2.begin(); it1 != track1.end(); ++it1, ++it2) {
                dstTrack = *it1;
                dstTrack2 = *it2;
                cloneSpanner(s, dstScore, dstTrack, dstTrack2);
            }
        }
    }
    collectTieEndPoints(tieMap);
}

void Excerpt::cloneMeasures(Score* oscore, Score* score)
{
    MeasureBaseList* measures = score->measures();
    TieMap tieMap;

    for (MeasureBase* mb = oscore->firstMeasure(); mb; mb = mb->next()) {
        MeasureBase* newMeasure = cloneMeasure(mb, score, oscore, {}, {}, tieMap);
        measures->append(newMeasure);
    }

    collectTieEndPoints(tieMap);
}

//! NOTE For staves in the same score
void Excerpt::cloneStaff(Staff* srcStaff, Staff* dstStaff, bool cloneSpanners)
{
    Score* score = srcStaff->score();
    TieMap tieMap;

    score->undo(new Link(dstStaff, srcStaff));

    staff_idx_t srcStaffIdx = srcStaff->idx();
    staff_idx_t dstStaffIdx = dstStaff->idx();

    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        m->setMeasureRepeatCount(m->measureRepeatCount(srcStaffIdx), dstStaffIdx);

        track_idx_t sTrack = srcStaffIdx * VOICES;
        track_idx_t eTrack = sTrack + VOICES;
        for (track_idx_t srcTrack = sTrack; srcTrack < eTrack; ++srcTrack) {
            TupletMap tupletMap;          // tuplets cannot cross measure boundaries
            track_idx_t dstTrack = dstStaffIdx * VOICES + (srcTrack - sTrack);
            TremoloTwoChord* prevTremolo = nullptr;
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                EngravingItem* oe = seg->element(srcTrack);
                if (oe == 0 || oe->generated()) {
                    continue;
                }
                if (oe->isTimeSig()) {
                    continue;
                }
                EngravingItem* ne = 0;
                if (oe->isClef()) {
                    // only clone clef if it matches staff group and does not exists yet
                    Clef* clef = toClef(oe);
                    Fraction tick = seg->tick();
                    if (ClefInfo::staffGroup(clef->concertClef()) == dstStaff->constStaffType(Fraction(0, 1))->group()
                        && dstStaff->clefType(tick) != clef->clefTypeList()) {
                        ne = oe->clone();
                    }
                } else {
                    ne = oe->linkedClone();
                }
                if (ne) {
                    ne->setTrack(dstTrack);
                    ne->setParent(seg);
                    ne->setScore(score);
                    if (ne->isChordRest()) {
                        ChordRest* ncr = toChordRest(ne);
                        if (ncr->tuplet()) {
                            ncr->setTuplet(0);               //TODO nested tuplets
                        }
                    }
                    score->undoAddElement(ne);
                }
                if (oe->isChordRest()) {
                    ChordRest* ocr = toChordRest(oe);
                    ChordRest* ncr = toChordRest(ne);
                    Tuplet* ot     = ocr->tuplet();
                    if (ot) {
                        cloneTuplets(ocr, ncr, ot, tupletMap, m, dstTrack);
                    }

                    // remove lyrics from chord
                    // since only one set of lyrics is used with linked staves
                    for (Lyrics* l : ncr->lyrics()) {
                        if (l) {
                            l->unlink();
                        }
                    }
                    muse::DeleteAll(ncr->lyrics());
                    ncr->lyrics().clear();

                    ncr->checkStaffMoveValidity();
                    // creating copy for iteration, cause seg->annotations() may change during loop
                    const std::vector<EngravingItem*> iterableAnnotations = seg->annotations();

                    for (EngravingItem* e : iterableAnnotations) {
                        if (!e) {
                            LOGD("cloneStaff: corrupted annotation found.");
                            continue;
                        }
                        if (e->generated() || e->systemFlag()) {
                            continue;
                        }
                        if (e->track() != srcTrack) {
                            continue;
                        }
                        switch (e->type()) {
                        // exclude certain element types
                        // this should be same list excluded in Score::undoAddElement()
                        case ElementType::STAFF_TEXT:
                        case ElementType::SYSTEM_TEXT:
                        case ElementType::TRIPLET_FEEL:
                        case ElementType::PLAYTECH_ANNOTATION:
                        case ElementType::CAPO:
                        case ElementType::STRING_TUNINGS:
                        case ElementType::FRET_DIAGRAM:
                        case ElementType::HARMONY:
                        case ElementType::FIGURED_BASS:
                        case ElementType::DYNAMIC:
                        case ElementType::INSTRUMENT_CHANGE:
                        case ElementType::LYRICS:                     // not normally segment-attached
                            continue;
                        case ElementType::FERMATA:
                        {
                            // Fermatas are special since the belong to a segment but should
                            // be created and linked on each staff.
                            EngravingItem* ne1 = e->linkedClone();
                            ne1->setTrack(dstTrack);
                            ne1->setParent(seg);
                            ne1->setScore(score);
                            score->doUndoAddElement(ne1);
                            continue;
                        }
                        default:
                            if (toTextLine(e)->systemFlag()) {
                                continue;
                            }
                            EngravingItem* ne1 = e->clone();
                            ne1->setTrack(dstTrack);
                            ne1->setParent(seg);
                            ne1->setScore(score);
                            score->undoAddElement(ne1);
                        }
                    }
                    if (oe->isChord()) {
                        Chord* och = toChord(ocr);
                        Chord* nch = toChord(ncr);
                        addGraceNoteTies(och->graceNotesBefore(), nch, tieMap, score);
                        size_t n = och->notes().size();
                        for (size_t i = 0; i < n; ++i) {
                            Note* on = och->notes().at(i);
                            Note* nn = nch->notes().at(i);
                            addTies(on, nn, tieMap, score);
                            // add back spanners (going back from end to start spanner element
                            // makes sure the 'other' spanner anchor element is already set up)
                            // 'on' is the old spanner end note and 'nn' is the new spanner end note
                            for (Spanner* oldSp : on->spannerBack()) {
                                Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                if (newStart != nullptr) {
                                    Spanner* newSp = toSpanner(oldSp->linkedClone());
                                    newSp->setNoteSpan(newStart, nn);
                                    score->addElement(newSp);
                                } else {
                                    LOGD("cloneStaff: cannot find spanner start note");
                                }
                            }
                        }
                        addGraceNoteTies(och->graceNotesAfter(), nch, tieMap, score);
                        addTremoloTwoChord(och, nch, prevTremolo);

                        // Check grace note staff move validity
                        for (Chord* gn : nch->graceNotes()) {
                            gn->checkStaffMoveValidity();
                        }
                        if (Ornament* o = nch->findOrnament()) {
                            Chord* cueChord = o->cueNoteChord();
                            if (cueChord) {
                                cueChord->checkStaffMoveValidity();
                            }
                        }
                    }
                }
            }
        }
    }

    if (cloneSpanners) {
        for (auto i : score->spanner()) {
            Spanner* s = i.second;
            staff_idx_t staffIdx = s->staffIdx();
            track_idx_t dstTrack = muse::nidx;
            track_idx_t dstTrack2 = muse::nidx;
            if (!isSystemTextLine(s)) {
                //export other spanner if staffidx matches
                if (srcStaffIdx == staffIdx) {
                    dstTrack = dstStaffIdx * VOICES + s->voice();
                    dstTrack2 = dstStaffIdx * VOICES + (s->track2() % VOICES);
                }
            }
            if (dstTrack == muse::nidx) {
                continue;
            }
            cloneSpanner(s, score, dstTrack, dstTrack2);
        }
    }

    collectTieEndPoints(tieMap);
}

//! NOTE For staves potentially in different scores
void Excerpt::cloneStaff2(Staff* srcStaff, Staff* dstStaff, const Fraction& startTick, const Fraction& endTick)
{
    Score* oscore = srcStaff->score();
    Score* score  = dstStaff->score();

    Excerpt* oex = oscore->excerpt();
    Excerpt* ex  = score->excerpt();
    TracksMap otracks, tracks;
    if (oex) {
        otracks = oex->tracksMapping();
    }
    if (ex) {
        tracks  = ex->tracksMapping();
    }

    Measure* m1   = oscore->tick2measure(startTick);
    Measure* m2   = oscore->tick2measure(endTick);

    if (m2->tick() < endTick) { // end of score
        m2 = 0;
    }

    TieMap tieMap;

    staff_idx_t srcStaffIdx = srcStaff->idx();
    staff_idx_t dstStaffIdx = dstStaff->idx();

    track_idx_t sTrack = srcStaffIdx * VOICES;
    track_idx_t eTrack = sTrack + VOICES;

    std::map<track_idx_t, track_idx_t> map;
    for (track_idx_t i = sTrack; i < eTrack; i++) {
        if (!oex && !ex) {
            map.insert({ i, dstStaffIdx * VOICES + i % VOICES });
        } else if (oex && !ex) {
            track_idx_t k = muse::key(otracks, i, muse::nidx);
            if (k != muse::nidx) {
                map.insert({ i, k });
            }
        } else if (!oex && ex) {
            for (track_idx_t j : muse::values(tracks, i)) {
                if (dstStaffIdx * VOICES <= j && j < (dstStaffIdx + 1) * VOICES) {
                    map.insert({ i, j });
                    break;
                }
            }
        } else if (oex && ex) {
            track_idx_t k = muse::key(otracks, i, muse::nidx);
            if (k != muse::nidx) {
                for (track_idx_t j : muse::values(tracks, k)) {
                    if (dstStaffIdx * VOICES <= j && j < (dstStaffIdx + 1) * VOICES) {
                        map.insert({ i, j });
                        break;
                    }
                }
            }
        }
    }

    if (map.empty()) {
        for (track_idx_t i = sTrack; i < eTrack; i++) {
            map.insert({ i, dstStaffIdx * VOICES + i % VOICES });
        }
    }

    bool firstVoiceVisible = dstStaff->isVoiceVisible(0);

    auto addElement = [score](EngravingItem* element) {
        score->undoAddElement(element, false /*addToLinkedStaves*/);
    };

    for (Measure* m = m1; m && (m != m2); m = m->nextMeasure()) {
        Measure* nm = score->tick2measure(m->tick());
        nm->setMeasureRepeatCount(m->measureRepeatCount(srcStaffIdx), dstStaffIdx);

        for (EngravingItem* oldEl : m->el()) {
            if (oldEl->isLayoutBreak()) {
                continue;
            }
            if (oldEl->systemFlag() && dstStaffIdx != 0) {
                continue;
            }
            bool alreadyCloned = oldEl->systemFlag() && oldEl->findLinkedInScore(score);
            if (alreadyCloned) {
                continue;
            }
            EngravingItem* newEl = oldEl->linkedClone();
            newEl->setParent(nm);
            newEl->setTrack(0);
            newEl->setScore(score);
            newEl->styleChanged();
            addElement(newEl);
        }

        TremoloTwoChord* prevTremolo = nullptr;
        for (track_idx_t srcTrack : muse::keys(map)) {
            TupletMap tupletMap;          // tuplets cannot cross measure boundaries
            track_idx_t dstTrack = map.at(srcTrack);
            for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                if (oseg->header()) {
                    // Generated at layout time, should not be cloned
                    continue;
                }
                Segment* ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                EngravingItem* oef = oseg->element(trackZeroVoice(srcTrack));
                if (oef && !oef->generated() && (oef->isTimeSig() || oef->isKeySig())) {
                    if (!firstVoiceVisible) {
                        EngravingItem* ne = oef->linkedClone();
                        ne->setTrack(trackZeroVoice(dstTrack));
                        ne->setParent(ns);
                        ne->setScore(score);
                        ne->styleChanged();
                        ne->scanElements(oef, updateSpatium);
                        addElement(ne);
                    }
                }

                for (EngravingItem* e : oseg->annotations()) {
                    if (e->generated() || e->excludeFromOtherParts()) {
                        continue;
                    }
                    bool systemObject = e->systemFlag() && e->track() == 0;
                    EngravingItem* linkedElement = e->findLinkedInScore(score);
                    Segment* linkedParent = linkedElement ? toSegment(linkedElement->parent()) : nullptr;
                    bool alreadyCloned = linkedParent && (linkedParent == ns
                                                          || (linkedParent->isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE)
                                                              && ns->isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE)
                                                              && linkedParent->tick() == ns->tick()));
                    bool cloneAnnotation = !alreadyCloned && (e->elementAppliesToTrack(srcTrack) || systemObject);

                    if (!cloneAnnotation) {
                        continue;
                    }
                    EngravingItem* ne1 = e->linkedClone();
                    ne1->setTrack(dstTrack);
                    ne1->setParent(ns);
                    ne1->setScore(score);
                    ne1->styleChanged();
                    addElement(ne1);
                }

                EngravingItem* oe = oseg->element(srcTrack);
                if (oe == 0 || oe->generated() || oe->excludeFromOtherParts()) {
                    continue;
                }

                EngravingItem* ne = oe->linkedClone();
                ne->setTrack(dstTrack);
                ne->setParent(ns);
                ne->setScore(score);
                ne->styleChanged();
                ne->scanElements(oe, updateSpatium);
                addElement(ne);
                if (oe->isChordRest()) {
                    ChordRest* ocr = toChordRest(oe);
                    ChordRest* ncr = toChordRest(ne);
                    Tuplet* ot = ocr->tuplet();
                    if (ot) {
                        cloneTuplets(ocr, ncr, ot, tupletMap, nm, dstTrack);
                    }
                    if (oe->isChord()) {
                        Chord* och = toChord(ocr);
                        Chord* nch = toChord(ncr);
                        addGraceNoteTies(och->graceNotesBefore(), nch, tieMap, score);
                        size_t n = och->notes().size();
                        for (size_t i = 0; i < n; ++i) {
                            Note* on = och->notes().at(i);
                            Note* nn = nch->notes().at(i);
                            addTies(on, nn, tieMap, score);
                            // add back spanners (going back from end to start spanner element
                            // makes sure the 'other' spanner anchor element is already set up)
                            // 'on' is the old spanner end note and 'nn' is the new spanner end note
                            for (Spanner* oldSp : on->spannerBack()) {
                                Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                if (newStart != nullptr) {
                                    Spanner* newSp = toSpanner(oldSp->linkedClone());
                                    newSp->setNoteSpan(newStart, nn);
                                    score->addElement(newSp);
                                } else {
                                    LOGD("cloneStaff2: cannot find spanner start note");
                                }
                            }
                            GuitarBend* bendBack = on->bendBack();
                            Note* newStartNote = bendBack ? toNote(bendBack->startNote()->findLinkedInStaff(dstStaff)) : nullptr;
                            if (bendBack && newStartNote) {
                                GuitarBend* newBend = toGuitarBend(bendBack->linkedClone());
                                newBend->setScore(score);
                                newBend->setParent(newStartNote);
                                newBend->setTrack(newStartNote->track());
                                newBend->setTrack2(nn->track());
                                newBend->setStartElement(newStartNote);
                                newBend->setEndElement(nn);
                                newStartNote->addSpannerFor(newBend);
                                nn->addSpannerBack(newBend);
                            }
                            GuitarBend* bendFor = on->bendFor();
                            if (bendFor && bendFor->type() == GuitarBendType::SLIGHT_BEND) {
                                // Because slight bends aren't detected as "bendBack"
                                GuitarBend* newBend = toGuitarBend(bendFor->linkedClone());
                                newBend->setScore(score);
                                newBend->setParent(nn);
                                newBend->setTrack(nn->track());
                                newBend->setTrack2(nn->track());
                                newBend->setStartElement(nn);
                                newBend->setEndElement(nn);
                                nn->addSpannerFor(newBend);
                            }
                        }
                        addGraceNoteTies(och->graceNotesAfter(), nch, tieMap, score);
                        addTremoloTwoChord(och, nch, prevTremolo);
                    }
                }
            }
        }
        for (Segment& seg : nm->segments()) {
            seg.checkEmpty();
            if (seg.empty()) {
                score->doUndoRemoveElement(&seg);
            }
        }
        if (!nm->hasVoices(dstStaffIdx, nm->tick(), nm->ticks())) {
            promoteGapRestsToRealRests(nm, dstStaffIdx);
        }
    }

    for (auto i : oscore->spanner()) {
        Spanner* s = i.second;
        if (!(s->tick() >= startTick && s->tick2() < endTick) || s->excludeFromOtherParts()) {
            continue;
        }

        staff_idx_t staffIdx = s->staffIdx();
        track_idx_t dstTrack = muse::nidx;
        track_idx_t dstTrack2 = muse::nidx;

        if (isSystemTextLine(s)) {
            if (!scoreContainsSpanner(score, s)) {
                dstTrack = s->track();
                dstTrack2 = s->track2();
            }
        } else {
            // export other spanner if staffidx matches
            if (srcStaffIdx == staffIdx) {
                if (muse::contains(map, s->track())) {
                    dstTrack  = map.at(s->track());
                }
                if (muse::contains(map, s->track2())) {
                    dstTrack2 = map.at(s->track2());
                }
            }
        }

        if (dstTrack == muse::nidx) {
            continue;
        }

        cloneSpanner(s, score, dstTrack, dstTrack2);
    }

    bool oscoreConcertPitch = oscore->style().styleB(Sid::concertPitch);
    bool scoreConcertPitch = score->style().styleB(Sid::concertPitch);

    if ((oscoreConcertPitch && !scoreConcertPitch)
        || (!oscoreConcertPitch && scoreConcertPitch)) {
        Interval interval = srcStaff->part()->instrument()->transpose();
        if (interval.isZero() && srcStaff->part()->instruments().size() == 1) {
            return;
        }

        if (!scoreConcertPitch) {
            interval.flip();
        }

        score->transposeKeys(dstStaffIdx, dstStaffIdx + 1, startTick, endTick, !scoreConcertPitch);
    }

    collectTieEndPoints(tieMap);
}

void Excerpt::promoteGapRestsToRealRests(const Measure* measure, staff_idx_t staffIdx)
{
    track_idx_t startTrack = staff2track(staffIdx);
    track_idx_t endTrack = startTrack + VOICES;
    for (const Segment& seg : measure->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* item = seg.element(track);
            if (item && item->isRest() && toRest(item)->isGap()) {
                toRest(item)->setGap(false);
            }
        }
    }
}

std::vector<Excerpt*> Excerpt::createExcerptsFromParts(const std::vector<Part*>& parts, MasterScore* score)
{
    StringList allExcerptLowerNames;
    for (const Excerpt* e : score->excerpts()) {
        allExcerptLowerNames.push_back(e->name().toLower());
    }

    std::vector<Excerpt*> result;

    for (Part* part : parts) {
        Excerpt* excerpt = new Excerpt(score);
        excerpt->parts().push_back(part);

        track_idx_t startTrack = part->startTrack();
        track_idx_t endTrack = part->endTrack();

        for (track_idx_t i = startTrack, j = 0; i < endTrack; ++i, ++j) {
            excerpt->m_tracksMapping.insert({ i, j });
        }

        String name = formatUniqueExcerptName(part->partName(), allExcerptLowerNames);
        excerpt->setName(name);
        excerpt->setInitialPartId(part->id());

        allExcerptLowerNames.push_back(name.toLower());
        result.push_back(excerpt);
    }

    return result;
}
