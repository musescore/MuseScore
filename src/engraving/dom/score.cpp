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

/**
 \file
 Implementation of class Score (partial).
*/

#include "score.h"

#include <cmath>
#include <map>

#include "containers.h"

#include "style/style.h"
#include "style/defaultstyle.h"
#include "compat/dummyelement.h"

#include "iengravingfont.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "anchors.h"
#include "audio.h"
#include "beam.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "capo.h"
#include "chord.h"
#include "clef.h"
#include "excerpt.h"
#include "dynamic.h"
#include "factory.h"
#include "fermata.h"
#include "glissando.h"
#include "gradualtempochange.h"
#include "guitarbend.h"
#include "harmony.h"
#include "imageStore.h"
#include "instrchange.h"
#include "instrtemplate.h"
#include "key.h"
#include "keysig.h"
#include "linkedobjects.h"
#include "lyrics.h"
#include "masterscore.h"
#include "measure.h"
#include "mscore.h"
#include "mscoreview.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "palmmute.h"
#include "part.h"
#include "partialtie.h"
#include "pitchspelling.h"
#include "rehearsalmark.h"
#include "repeatlist.h"
#include "rest.h"
#include "scoreorder.h"
#include "segment.h"
#include "stafftextbase.h"
#include "select.h"
#include "shadownote.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftype.h"
#include "synthesizerstate.h"
#include "system.h"
#include "tempo.h"
#include "tempotext.h"
#include "text.h"
#include "tie.h"
#include "tiemap.h"
#include "timesig.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#include "accessibility/accessibleroot.h"
#endif

#include "log.h"

using namespace mu;
using namespace muse;
using namespace mu::engraving;

namespace mu::engraving {
MasterScore* gpaletteScore;                 ///< system score, used for palettes etc.
std::set<Score*> Score::validScores;

bool noSeq           = false;
bool noMidi          = false;
bool midiInputTrace  = false;
bool midiOutputTrace = false;

static void markInstrumentsAsPrimary(std::vector<Part*>& parts)
{
    TRACEFUNC;

    std::unordered_map<String /*instrumentId*/, int /*count*/> instrumentCount;

    for (Part* part : parts) {
        Instrument* instrument = part->instrument();
        if (!instrument) {
            continue;
        }

        auto it = instrumentCount.find(instrument->id());
        if (it == instrumentCount.cend()) {
            it = instrumentCount.insert(instrumentCount.begin(), { instrument->id(), 0 });
        }

        it->second++;

        bool isPrimary = (it->second % 2 != 0);
        instrument->setIsPrimary(isPrimary);
    }
}

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score(const modularity::ContextPtr& iocCtx)
    : EngravingObject(ElementType::SCORE, nullptr), muse::Injectable(iocCtx)
    , m_headersText(MAX_HEADERS, nullptr), m_footersText(MAX_FOOTERS, nullptr), m_selection(this)
{
    if (elementsProvider()) {
        elementsProvider()->reg(this);
    }

    Score::validScores.insert(this);
    m_masterScore = nullptr;

    m_engravingFont = engravingFonts()->fontByName("Leland");

    m_fileDivision = Constants::DIVISION;
    m_style = DefaultStyle::defaultStyle();

    m_rootItem = new RootItem(this);
    m_rootItem->init();

    //! NOTE Looks like a bug, `minimumPaddingUnit` is set using the default style's spatium value
    //! and does not change if the style or the spatium of this score is changed
    m_paddingTable.setMinimumPaddingUnit(0.1 * style().spatium());
    createPaddingTable();

    m_shadowNote = new ShadowNote(this);
    m_shadowNote->setVisible(false);
}

Score::Score(MasterScore* parent, bool forcePartStyle /* = true */)
    : Score{parent->iocContext()}
{
    Score::validScores.insert(this);
    m_masterScore = parent;

    if (DefaultStyle::defaultStyleForParts()) {
        m_style = *DefaultStyle::defaultStyleForParts();
    } else {
        // inherit most style settings from parent
        m_style = parent->style();

        static const Sid styles[] = {
            Sid::pageWidth,
            Sid::pageHeight,
            Sid::pagePrintableWidth,
            Sid::pageEvenLeftMargin,
            Sid::pageOddLeftMargin,
            Sid::pageEvenTopMargin,
            Sid::pageEvenBottomMargin,
            Sid::pageOddTopMargin,
            Sid::pageOddBottomMargin,
            Sid::pageTwosided,
            Sid::spatium
        };
        // but borrow defaultStyle page layout settings
        for (auto i : styles) {
            m_style.set(i, DefaultStyle::defaultStyle().value(i));
        }
        // and force some style settings that just make sense for parts
        if (forcePartStyle) {
            style().set(Sid::concertPitch, false);
            style().set(Sid::createMultiMeasureRests, true);
            style().set(Sid::dividerLeft, false);
            style().set(Sid::dividerRight, false);
        }
    }
    // update style values
    m_style.precomputeValues();
    checkChordList();
    m_synthesizerState = parent->m_synthesizerState;
    m_mscVersion = parent->m_mscVersion;
    createPaddingTable();
}

Score::Score(MasterScore* parent, const MStyle& s)
    : Score{parent}
{
    Score::validScores.insert(this);
    m_style  = s;
    createPaddingTable();
}

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
{
    Score::validScores.erase(this);

    for (MuseScoreView* v : m_viewer) {
        v->removeScore();
    }
    // deselectAll();
    muse::DeleteAll(m_systems);   // systems are layout-only objects so we delete
    // them prior to measures.
    for (MeasureBase* m = m_measures.first(); m;) {
        MeasureBase* nm = m->next();
        if (m->isMeasure() && toMeasure(m)->mmRest()) {
            delete toMeasure(m)->mmRest();
        }
        delete m;
        m = nm;
    }

    m_spanner.clear();

    muse::DeleteAll(m_systemLocks.allLocks());
    m_systemLocks.clear();

    muse::DeleteAll(m_parts);
    m_parts.clear();

    muse::DeleteAll(m_staves);
    m_staves.clear();

    muse::DeleteAll(m_pages);
    m_pages.clear();

    m_masterScore = nullptr;

    imageStore.clearUnused();

    delete m_shadowNote;
    m_shadowNote = nullptr;

    delete m_rootItem;
    m_rootItem = nullptr;
}

muse::async::Channel<LoopBoundaryType, unsigned> Score::loopBoundaryTickChanged() const
{
    return m_loopBoundaryTickChanged;
}

void Score::notifyLoopBoundaryTickChanged(LoopBoundaryType type, unsigned ticks)
{
    m_loopBoundaryTickChanged.send(type, ticks);
}

muse::async::Channel<EngravingItem*> Score::elementDestroyed()
{
    return m_elementDestroyed;
}

muse::async::Channel<float> Score::layoutProgressChannel() const
{
    return m_layoutProgressChannel;
}

//---------------------------------------------------------
//   Score::clone
//         To create excerpt clone to show when changing PageSettings
//         Use MasterScore::clone() instead
//---------------------------------------------------------

Score* Score::clone()
{
    // TODO: see comments regarding setting version in corresponding code in 3.x branch
    // and also compare to MasterScore::clone()
    Excerpt* excerpt = new Excerpt(masterScore());
    excerpt->setName(name());

    TracksMap tracks;

    for (Part* part : m_parts) {
        excerpt->parts().push_back(part);

        for (track_idx_t track = part->startTrack(); track < part->endTrack(); ++track) {
            tracks.insert({ track, track });
        }
    }

    excerpt->setTracksMapping(tracks);

    masterScore()->initAndAddExcerpt(excerpt, true);
    masterScore()->removeExcerpt(excerpt);

    return excerpt->excerptScore();
}

Score* Score::paletteScore()
{
    return gpaletteScore;
}

bool Score::isPaletteScore() const
{
    return this == gpaletteScore;
}

static void onBracketItemDestruction(const Score* score, const BracketItem* item)
{
    BracketItem* dummy = score->dummy()->bracketItem();

    for (const System* system : score->systems()) {
        for (Bracket* bracket : system->brackets()) {
            if (bracket && bracket->bracketItem() == item) {
                bracket->setBracketItem(dummy);
            }
        }
    }
}

//---------------------------------------------------------
//   Score::onElementDestruction
//    Ensure correct state of the score after destruction
//    of the element (e.g. remove invalid pointers etc.).
//---------------------------------------------------------

void Score::onElementDestruction(EngravingItem* e)
{
    Score* score = e->EngravingObject::score();

    if (!score || Score::validScores.find(score) == Score::validScores.end()) {
        // No score or the score is already deleted
        return;
    }

    if (e->isBracketItem()) {
        onBracketItemDestruction(score, toBracketItem(e));
    }

    score->selection().remove(e);
    score->cmdState().unsetElement(e);
    score->elementDestroyed().send(e);
}

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

void Score::addMeasure(MeasureBase* m, MeasureBase* pos)
{
    m->setNext(pos);
    m_measures.add(m);
}

void Score::setUpTempoMapLater()
{
    m_needSetUpTempoMap = true;
}

//---------------------------------------------------------
//    setUpTempoMap
//    update:
//      - measure ticks
//      - tempo map
//      - time signature map
//---------------------------------------------------------

/**
 This is needed after
      - inserting or removing a measure
      - changing the sigmap
      - after inserting/deleting time (changes the sigmap)
*/

void Score::setUpTempoMap()
{
    TRACEFUNC;

    Fraction tick = Fraction(0, 1);
    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }

    for (Staff* staff : m_staves) {
        staff->clearTimeSig();
    }

    if (isMaster()) {
        tempomap()->clear();
        sigmap()->clear();
        sigmap()->add(0, SigEvent(fm->ticks(),  fm->timesig(), 0));
    }
    std::vector<Measure*> anacrusisMeasures;

    auto tempoPrimo = std::optional<BeatsPerSecond> {};

    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            mb->setTick(tick);
            continue;
        }
        Measure* m            = toMeasure(mb);
        Fraction mtick        = m->tick();
        Fraction diff         = tick - mtick;
        Fraction measureTicks = m->ticks();
        m->moveTicks(diff);
        if (m->mmRest()) {
            m->mmRest()->moveTicks(diff);
        }
        if (m->isAnacrusis()) {
            anacrusisMeasures.push_back(m);
        }

        rebuildTempoAndTimeSigMaps(m, tempoPrimo);

        tick += measureTicks;
    }

    m_measures.updateTickIndex();

    if (isMaster()) {
        for (const auto& pair : spanner()) {
            const Spanner* spannerItem = pair.second;
            if (!spannerItem || !spannerItem->isGradualTempoChange() || !spannerItem->playSpanner()) {
                continue;
            }

            const GradualTempoChange* tempoChange = toGradualTempoChange(spannerItem);
            if (!tempoChange) {
                continue;
            }

            int tickPositionFrom = tempoChange->tick().ticks();
            BeatsPerSecond currentBps = tempomap()->tempo(tickPositionFrom);
            BeatsPerSecond newBps = currentBps * tempoChange->tempoChangeFactor();

            int totalTicks = tempoChange->ticks().ticks();
            int stepsCount = std::max(8, totalTicks / Constants::DIVISION);
            std::map<int, double> tempoCurve = TConv::easingValueCurve(totalTicks,
                                                                       stepsCount,
                                                                       newBps.val - currentBps.val,
                                                                       tempoChange->easingMethod());

            for (const auto& pair2 : tempoCurve) {
                int tick2 = tickPositionFrom + pair2.first;

                if (tempomap()->find(tick2) == tempomap()->end()) {
                    tempomap()->setTempo(tick2, BeatsPerSecond(currentBps.val + pair2.second));
                }
            }
        }

        if (tempomap()->empty()) {
            tempomap()->setTempo(0, Constants::DEFAULT_TEMPO);
        }
    }

    masterScore()->updateRepeatListTempo();
    if (!anacrusisMeasures.empty()) {
        fixAnacrusisTempo(anacrusisMeasures);
    }
    m_needSetUpTempoMap = false;
}

void Score::setNeedLayoutFretBox(bool layout)
{
    m_needLayoutFretBox = layout;
}

//---------------------------------------------------------
//    fixTicks
///    updates tempomap and time sig map for a measure
//---------------------------------------------------------

void Score::rebuildTempoAndTimeSigMaps(Measure* measure, std::optional<BeatsPerSecond>& tempoPrimo)
{
    if (isMaster()) {
        // Reset tempo to set correct time stretch for fermata.
        const Fraction& startTick = measure->tick();
        resetTempoRange(startTick, measure->endTick());

        // Implement section break rest
        for (MeasureBase* mb = measure->prev(); mb && mb->endTick() == startTick; mb = mb->prev()) {
            if (mb->pause()) {
                tempomap()->setPause(startTick.ticks(), mb->pause());
            }
        }

        // Add pauses from the end of the previous measure (at measure->tick()):
        for (Segment* s = measure->first(); s && s->tick() == startTick; s = s->prev1()) {
            if (!s->isBreathType()) {
                continue;
            }
            double length = 0.0;
            for (EngravingItem* e : s->elist()) {
                if (e && e->isBreath()) {
                    length = std::max(length, toBreath(e)->pause());
                }
            }
            if (!RealIsNull(length)) {
                tempomap()->setPause(startTick.ticks(), length);
            }
        }
    }

    for (Segment& segment : measure->segments()) {
        if (segment.isBreathType()) {
            if (!isMaster()) {
                continue;
            }
            double length = 0.0;
            Fraction tick = segment.tick();
            // find longest pause
            for (track_idx_t i = 0, n = ntracks(); i < n; ++i) {
                EngravingItem* e = segment.element(i);
                if (e && e->isBreath()) {
                    Breath* b = toBreath(e);
                    length = std::max(length, b->pause());
                }
            }
            if (!RealIsNull(length)) {
                tempomap()->setPause(tick.ticks(), length);
            }
        } else if (segment.isTimeSigType()) {
            for (size_t staffIdx = 0; staffIdx < m_staves.size(); ++staffIdx) {
                TimeSig* ts = toTimeSig(segment.element(staffIdx * VOICES));
                if (ts) {
                    staff(staffIdx)->addTimeSig(ts);
                }
            }
        } else if (segment.isChordRestType() || segment.isTimeTickType()) {
            if (!isMaster()) {
                continue;
            }
            double stretch = 0.0;
            for (EngravingItem* e : segment.annotations()) {
                if (e->isFermata() && toFermata(e)->play()) {
                    stretch = std::max(stretch, toFermata(e)->timeStretch());
                } else if (e->isTempoText()) {
                    TempoText* tt = toTempoText(e);

                    if (!tt->playTempoText()) {
                        continue;
                    }

                    if (tt->isNormal() && !tt->isRelative() && !tempoPrimo) {
                        tempoPrimo = tt->tempo();
                    } else if (tt->isRelative()) {
                        tt->updateRelative();
                    }

                    int ticks = tt->segment()->tick().ticks();
                    if (tt->isATempo() && tt->followText()) {
                        // this will effectively reset the tempo to the previous one
                        // when a progressive change was active
                        tempomap()->setTempo(ticks, tempomap()->tempo(ticks));
                    } else if (tt->isTempoPrimo() && tt->followText()) {
                        tempomap()->setTempo(ticks, tempoPrimo ? *tempoPrimo : Constants::DEFAULT_TEMPO);
                    } else {
                        tempomap()->setTempo(ticks, tt->tempo());
                    }
                }
            }

            if (!RealIsNull(stretch) && !RealIsEqual(stretch, 1.0)) {
                BeatsPerSecond otempo = tempomap()->tempo(segment.tick().ticks());
                BeatsPerSecond ntempo = otempo.val / stretch;
                tempomap()->setTempo(segment.tick().ticks(), ntempo);

                Fraction tempoEndTick;

                const Segment* nextActiveSegment = segment.next1();
                while (nextActiveSegment && !nextActiveSegment->isActive()) {
                    nextActiveSegment = nextActiveSegment->next1();
                }

                if (nextActiveSegment) {
                    tempoEndTick = nextActiveSegment->tick();
                } else {
                    tempoEndTick = segment.tick() + segment.ticks();
                }

                Fraction etick = tempoEndTick - Fraction(1, 480 * 4);
                auto e = tempomap()->find(etick.ticks());
                if (e == tempomap()->end()) {
                    tempomap()->setTempo(etick.ticks(), otempo);
                }
            }
        }
    }

    // update time signature map
    // create event if measure len and time signature are different
    // even if they are equivalent 4/4 vs 2/2
    // also check if nominal time signature has changed

    if (isMaster()) {
        const Measure* m = measure;
        const Fraction mTicks = m->isMMRest() ? m->mmRestFirst()->ticks() : m->ticks();     // for time signature the underlying measure length matters for MM rests

        const Measure* pm = measure->prevMeasure();
        // prevMeasure() doesn't return MM rest so we don't handle it here

        if (pm && (!mTicks.identical(pm->ticks()) || !m->timesig().identical(pm->timesig()))) {
            sigmap()->add(m->tick().ticks(), SigEvent(mTicks, m->timesig(), m->no()));
        }
    }
}

void Score::fixAnacrusisTempo(const std::vector<Measure*>& measures) const
{
    auto getTempoTextIfExist = [](const Measure* m) -> TempoText* {
        for (const Segment& s : m->segments()) {
            if (s.isChordRestType()) {
                for (EngravingItem* e : s.annotations()) {
                    if (e->isTempoText() && toTempoText(e)->playTempoText()) {
                        return toTempoText(e);
                    }
                }
            }
        }
        return nullptr;
    };

    for (Measure* measure : measures) {
        if (getTempoTextIfExist(measure)) {
            continue;
        }
        Measure* nextMeasure = measure->nextMeasure();
        if (nextMeasure) {
            if (TempoText* tt = getTempoTextIfExist(nextMeasure); tt) {
                tempomap()->setTempo(measure->tick().ticks(), tt->tempo());
            }
        }
    }
}

//---------------------------------------------------------
//   pos2measure
//     Return measure for canvas relative position \a p.
//---------------------------------------------------------

Measure* Score::pos2measure(const PointF& p, staff_idx_t* rst, int* pitch, Segment** seg, PointF* offset) const
{
    Measure* m = searchMeasure(p);
    if (m == 0) {
        return 0;
    }

    System* s = m->system();
    double y   = p.y() - s->canvasPos().y();

    const staff_idx_t i = s->searchStaff(y);

    // search for segment + offset
    PointF pppp = p - m->canvasPos();
    staff_idx_t strack = i * VOICES;
    if (!staff(i)) {
        return nullptr;
    }
//      int etrack = staff(i)->part()->nstaves() * VOICES + strack;
    track_idx_t etrack = VOICES + strack;

    constexpr SegmentType st = SegmentType::ChordRest;
    Segment* segment = m->searchSegment(pppp.x(), st, strack, etrack);
    if (segment) {
        SysStaff* sstaff = m->system()->staff(i);
        *rst = i;
        if (pitch) {
            Staff* s1 = m_staves[i];
            Fraction tick  = segment->tick();
            ClefType clef = s1->clef(tick);
            *pitch = y2pitch(pppp.y() - sstaff->bbox().y(), clef, s1->spatium(tick));
        }
        if (offset) {
            *offset = pppp - PointF(segment->x(), sstaff->bbox().y());
        }
        if (seg) {
            *seg = segment;
        }
        return m;
    }

    return 0;
}

//---------------------------------------------------------
//   dragPosition
///   \param p   drag position in canvas coordinates
///   \param rst \b input: current staff index \n
///              \b output: new staff index for drag position
///   \param seg \b input: current segment \n
///              \b output: new segment for drag position
//---------------------------------------------------------

void Score::dragPosition(const PointF& p, staff_idx_t* rst, Segment** seg, double spacingFactor, bool allowTimeAnchor) const
{
    const System* preferredSystem = (*seg) ? (*seg)->system() : nullptr;
    Measure* m = searchMeasure(p, preferredSystem, spacingFactor);
    if (m == 0 || m->isMMRest()) {
        return;
    }

    System* s = m->system();
    double y   = p.y() - s->canvasPos().y();

    const staff_idx_t i = s->searchStaff(y, *rst, spacingFactor);

    // search for segment + offset
    PointF pppp = p - m->canvasPos();
    track_idx_t strack = staff2track(i);
    if (!staff(i)) {
        return;
    }
    track_idx_t etrack = staff2track(i + 1);

    SegmentType st = allowTimeAnchor ? Segment::CHORD_REST_OR_TIME_TICK_TYPE : SegmentType::ChordRest;
    Segment* segment = m->searchSegment(pppp.x(), st, strack, etrack, *seg, spacingFactor);
    if (segment) {
        if (segment->isTimeTickType()) {
            if (Segment* crAtSamePos = m->findSegmentR(SegmentType::ChordRest, segment->rtick())) {
                // If TimeTick and ChordRest at same position, prefer ChordRest
                segment = crAtSamePos;
            }
        }
        *rst = i;
        *seg = segment;
        return;
    }

    return;
}

//---------------------------------------------------------
//   setShowInvisible
//---------------------------------------------------------

void Score::setShowInvisible(bool v)
{
    m_showInvisible = v;
    // BSP tree does not include elements which are not
    // displayed, so we need to refresh it to get
    // invisible elements displayed or properly hidden.
    rebuildBspTree();
}

//---------------------------------------------------------
//   setShowUnprintable
//---------------------------------------------------------

void Score::setShowUnprintable(bool v)
{
    m_showUnprintable = v;
}

//---------------------------------------------------------
//   setShowFrames
//---------------------------------------------------------

void Score::setShowFrames(bool v)
{
    m_showFrames = v;
}

//---------------------------------------------------------
//   setShowPageborders
//---------------------------------------------------------

void Score::setShowPageborders(bool v)
{
    m_showPageborders = v;
}

void Score::setShowSoundFlags(bool v)
{
    if (m_showSoundFlags == v) {
        return;
    }

    m_showSoundFlags = v;
    setLayoutAll();
}

//---------------------------------------------------------
//   setMarkIrregularMeasures
//---------------------------------------------------------

void Score::setMarkIrregularMeasures(bool v)
{
    m_markIrregularMeasures = v;
}

void Score::setShowAnchors(const ShowAnchors& showAnchors)
{
    m_showAnchors = showAnchors;
}

//---------------------------------------------------------
//   readOnly
//---------------------------------------------------------

bool Score::readOnly() const
{
    return m_masterScore->readOnly();
}

//---------------------------------------------------------
//   dirty
//---------------------------------------------------------

bool Score::dirty() const
{
    return !undoStack()->isClean();
}

//---------------------------------------------------------
//   playlistDirty
//---------------------------------------------------------

bool Score::playlistDirty() const
{
    return masterScore()->playlistDirty();
}

//---------------------------------------------------------
//   setPlaylistDirty
//---------------------------------------------------------

void Score::setPlaylistDirty()
{
    masterScore()->setPlaylistDirty();
}

bool Score::isOpen() const
{
    return m_isOpen;
}

void Score::setIsOpen(bool open)
{
    m_isOpen = open;
}

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell()
{
    for (staff_idx_t i = 0; i < nstaves(); ++i) {
        std::vector<Note*> notes;
        for (Segment* s = firstSegment(SegmentType::All); s; s = s->next1()) {
            track_idx_t strack = i * VOICES;
            track_idx_t etrack = strack + VOICES;
            for (track_idx_t track = strack; track < etrack; ++track) {
                EngravingItem* e = s->element(track);
                if (e && e->type() == ElementType::CHORD) {
                    std::copy_if(toChord(e)->notes().begin(), toChord(e)->notes().end(),
                                 std::back_inserter(notes), [this](EngravingItem* ce) { return selection().isNone() || ce->selected(); });
                }
            }
        }
        spellNotelist(notes);
    }
}

void Score::spell(staff_idx_t startStaff, staff_idx_t endStaff, Segment* startSegment, Segment* endSegment)
{
    for (staff_idx_t i = startStaff; i < endStaff; ++i) {
        std::vector<Note*> notes;
        for (Segment* s = startSegment; s && s != endSegment; s = s->next()) {
            track_idx_t strack = i * VOICES;
            track_idx_t etrack = strack + VOICES;
            for (track_idx_t track = strack; track < etrack; ++track) {
                EngravingItem* e = s->element(track);
                if (e && e->type() == ElementType::CHORD) {
                    notes.insert(notes.end(),
                                 toChord(e)->notes().begin(),
                                 toChord(e)->notes().end());
                }
            }
        }
        spellNotelist(notes);
    }
}

void Score::changeEnharmonicSpelling(bool both)
{
    std::list<Note*> notes = selection().uniqueNotes();
    for (Note* n : notes) {
        Staff* staff = n->staff();
        if (staff->part()->instrument(n->tick())->useDrumset()) {
            continue;
        }
        if (staff->isTabStaff(n->tick())) {
            int string = n->line() + (both ? 1 : -1);
            int fret = staff->part()->stringData(n->tick(), staff->idx())->fret(n->pitch(), string, staff);
            if (fret != -1) {
                n->undoChangeProperty(Pid::FRET, fret);
                n->undoChangeProperty(Pid::STRING, string);
            }
        } else {
            static const int tab[36] = {
                26, 14,  2,    // 60  B#   C   Dbb
                21, 21,  9,    // 61  C#   C#  Db
                28, 16,  4,    // 62  C##  D   Ebb
                23, 23, 11,    // 63  D#   D#  Eb
                30, 18,  6,    // 64  D##  E   Fb
                25, 13,  1,    // 65  E#   F   Gbb
                20, 20,  8,    // 66  F#   F#  Gb
                27, 15,  3,    // 67  F##  G   Abb
                22, 22, 10,    // 68  G#   G#  Ab
                29, 17,  5,    // 69  G##  A   Bbb
                24, 24, 12,    // 70  A#   A#  Bb
                31, 19,  7,    // 71  A##  B   Cb
            };
            int tpc = n->tpc();
            for (int i = 0; i < 36; ++i) {
                if (tab[i] == tpc) {
                    if ((i % 3) < 2) {
                        if (tab[i] == tab[i + 1]) {
                            tpc = tab[i + 2];
                        } else {
                            tpc = tab[i + 1];
                        }
                    } else {
                        tpc = tab[i - 2];
                    }
                    break;
                }
            }
            n->undoSetTpc(tpc);
            if (both || staff->part()->instrument(n->chord()->tick())->transpose().isZero()) {
                // change both spellings
                int t = n->transposeTpc(tpc);
                if (n->concertPitch()) {
                    n->undoChangeProperty(Pid::TPC2, t);
                } else {
                    n->undoChangeProperty(Pid::TPC1, t);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   prevNote
//---------------------------------------------------------

Note* prevNote(Note* n)
{
    Chord* chord = n->chord();
    Segment* seg = chord->segment();
    const std::vector<Note*> nl = chord->notes();
    auto i = std::find(nl.begin(), nl.end(), n);
    if (i != nl.begin()) {
        return *(i - 1);
    }
    staff_idx_t staff      = n->staffIdx();
    track_idx_t startTrack = staff * VOICES + n->voice() - 1;
    track_idx_t endTrack   = 0;
    while (seg) {
        if (seg->segmentType() == SegmentType::ChordRest) {
            for (track_idx_t track = startTrack; track >= endTrack; --track) {
                EngravingItem* e = seg->element(track);
                if (e && e->type() == ElementType::CHORD) {
                    return toChord(e)->upNote();
                }
            }
        }
        seg = seg->prev1();
        startTrack = staff * VOICES + VOICES - 1;
    }
    return n;
}

//---------------------------------------------------------
//   nextNote
//---------------------------------------------------------

static Note* nextNote(Note* n)
{
    Chord* chord = n->chord();
    const std::vector<Note*> nl = chord->notes();
    auto i = std::find(nl.begin(), nl.end(), n);
    if (i != nl.end()) {
        ++i;
        if (i != nl.end()) {
            return *i;
        }
    }
    Segment* seg   = chord->segment();
    staff_idx_t staff      = n->staffIdx();
    track_idx_t startTrack = staff * VOICES + n->voice() + 1;
    track_idx_t endTrack   = staff * VOICES + VOICES;
    while (seg) {
        if (seg->segmentType() == SegmentType::ChordRest) {
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                EngravingItem* e = seg->element(track);
                if (e && e->type() == ElementType::CHORD) {
                    return ((Chord*)e)->downNote();
                }
            }
        }
        seg = seg->next1();
        startTrack = staff * VOICES;
    }
    return n;
}

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell(Note* note)
{
    std::vector<Note*> notes;

    notes.push_back(note);
    Note* nn = nextNote(note);
    notes.push_back(nn);
    nn = nextNote(nn);
    notes.push_back(nn);
    nn = nextNote(nn);
    notes.push_back(nn);

    nn = prevNote(note);
    notes.insert(notes.begin(), nn);
    nn = prevNote(nn);
    notes.insert(notes.begin(), nn);
    nn = prevNote(nn);
    notes.insert(notes.begin(), nn);

    int opt = computeWindow(notes, 0, 7);
    note->setTpc(tpc(3, note->pitch(), opt));
}

//---------------------------------------------------------
//   searchPage
//    p is in canvas coordinates
//---------------------------------------------------------

Page* Score::searchPage(const PointF& p) const
{
    if (layoutMode() == LayoutMode::LINE) {
        return m_pages.empty() ? nullptr : m_pages.front();
    }

    for (Page* page : m_pages) {
        RectF r = page->ldata()->bbox().translated(page->pos());
        if (r.contains(p)) {
            return page;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   searchSystem
///   Returns list of systems as there may be more than
///   one system in a row
///   \param pos Position in canvas coordinates
///   \param preferredSystem If not nullptr, will give more
///   space to the given system when searching it by its
///   coordinate.
///   \returns List of found systems.
//---------------------------------------------------------

std::vector<System*> Score::searchSystem(const PointF& pos, const System* preferredSystem, double spacingFactor,
                                         double preferredSpacingFactor) const
{
    std::vector<System*> systems;
    Page* page = searchPage(pos);
    if (!page) {
        return systems;
    }
    double y = pos.y() - page->pos().y();    // transform to page relative
    const std::vector<System*>& sl = page->systems();
    double y2;
    size_t n = sl.size();
    for (size_t i = 0; i < n; ++i) {
        System* s = sl.at(i);
        if (!s->firstMeasure()) {
            continue;
        }
        System* ns = 0;                   // next system row
        size_t ii = i + 1;
        for (; ii < n; ++ii) {
            ns = sl.at(ii);
            if (ns->y() != s->y()) {
                break;
            }
        }
        if ((ii == n) || (ns == 0)) {
            y2 = page->height();
        } else {
            double currentSpacingFactor;
            double sy2 = s->y() + s->ldata()->bbox().height();
            if (s == preferredSystem) {
                currentSpacingFactor = preferredSpacingFactor; //y2 = ns->y();
            } else if (ns == preferredSystem) {
                currentSpacingFactor = 1.0 - preferredSpacingFactor; //y2 = sy2;
            } else {
                currentSpacingFactor = spacingFactor;
            }
            y2 = sy2 + (ns->y() - sy2) * currentSpacingFactor;
        }
        if (y < y2) {
            systems.push_back(s);
            for (size_t iii = i + 1; iii < n; ++iii) {
                if (sl.at(iii)->y() != s->y()) {
                    break;
                }
                systems.push_back(sl.at(iii));
            }
            return systems;
        }
    }
    return systems;
}

//---------------------------------------------------------
//   searchMeasure
///   \param p Position in canvas coordinates
///   \param preferredSystem If not nullptr, will give more
///   space to measures in this system when searching.
//---------------------------------------------------------

Measure* Score::searchMeasure(const PointF& p, const System* preferredSystem, double spacingFactor, double preferredSpacingFactor) const
{
    std::vector<System*> systems = searchSystem(p, preferredSystem, spacingFactor, preferredSpacingFactor);
    for (System* system : systems) {
        double x = p.x() - system->canvasPos().x();
        for (MeasureBase* mb : system->measures()) {
            if (mb->isMeasure() && (x < (mb->x() + mb->ldata()->bbox().width()))) {
                return toMeasure(mb);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//    getNextValidInputSegment
//    - segment is of type SegmentType::ChordRest
//---------------------------------------------------------

static Segment* getNextValidInputSegment(Segment* segment, track_idx_t track, voice_idx_t voice)
{
    if (segment == nullptr) {
        return nullptr;
    }

    assert(segment->segmentType() == SegmentType::ChordRest);

    ChordRest* chordRest = nullptr;
    for (Segment* s1 = segment; s1; s1 = s1->prev(SegmentType::ChordRest)) {
        EngravingItem* element = s1->element(track + voice);
        chordRest = toChordRest(element);

        if (chordRest) {
            break;
        }
    }

    Fraction nextTick = (chordRest == nullptr) ? segment->measure()->tick()
                        : chordRest->tick() + chordRest->actualTicks();

    static const SegmentType st { SegmentType::ChordRest };
    while (segment) {
        if (segment->element(track + voice)) {
            break;
        }

        if (voice && segment->tick() == nextTick) {
            return segment;
        }

        segment = segment->next(st);
    }

    return segment;
}

//---------------------------------------------------------
//   getPosition
//    return true if valid position found
//---------------------------------------------------------

bool Score::getPosition(Position* pos, const PointF& p, voice_idx_t voice) const
{
    System* preferredSystem = nullptr;
    staff_idx_t preferredStaffIdx = muse::nidx;
    const double spacingFactor = 0.5;
    const double preferredSpacingFactor = 0.75;
    if (noteEntryMode() && inputState().staffGroup() != StaffGroup::TAB) {
        // for non-tab staves, prefer the current system & staff
        // this makes it easier to add notes far above or below the staff
        // not helpful for tab since notes are not entered above or below
        Segment* seg = inputState().segment();
        if (seg) {
            preferredSystem = seg->system();
        }
        track_idx_t track = inputState().track();
        if (track != muse::nidx) {
            preferredStaffIdx = track >> 2;
        }
    }
    Measure* measure = searchMeasure(p, preferredSystem, spacingFactor, preferredSpacingFactor);
    if (measure == 0) {
        return false;
    }

    pos->fret = INVALID_FRET_INDEX;
    //
    //    search staff
    //
    pos->staffIdx      = 0;
    SysStaff* sstaff   = 0;
    System* system     = measure->system();
    double y           = p.y() - system->pagePos().y();
    for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
        Staff* st = staff(pos->staffIdx);
        if (!st->part()->show()) {
            continue;
        }
        double sy2;
        SysStaff* ss = system->staff(pos->staffIdx);
        if (!ss->show()) {
            continue;
        }
        staff_idx_t idx = muse::nidx;
        SysStaff* nstaff = 0;

        // find next visible staff
        for (staff_idx_t i = pos->staffIdx + 1; i < nstaves(); ++i) {
            Staff* sti = staff(i);
            if (!sti->part()->show()) {
                continue;
            }
            nstaff = system->staff(i);
            if (!nstaff->show()) {
                nstaff = 0;
                continue;
            }
            if (i == preferredStaffIdx) {
                idx = i;
            }
            break;
        }

        if (nstaff) {
            double currentSpacingFactor;
            if (pos->staffIdx == preferredStaffIdx) {
                currentSpacingFactor = preferredSpacingFactor;
            } else if (idx == preferredStaffIdx) {
                currentSpacingFactor = 1.0 - preferredSpacingFactor;
            } else {
                currentSpacingFactor = spacingFactor;
            }
            double s1y2 = ss->bbox().bottom();
            sy2        = system->page()->canvasPos().y() + s1y2 + (nstaff->bbox().y() - s1y2) * currentSpacingFactor;
        } else {
            sy2 = system->page()->canvasPos().y() + system->page()->height() - system->pagePos().y();         // system->height();
        }
        if (y < sy2) {
            sstaff = ss;
            break;
        }
    }
    if (sstaff == 0) {
        return false;
    }

    //
    //    search segment
    //
    PointF pppp(p - measure->canvasPos());
    double x         = pppp.x();
    Segment* segment = 0;
    pos->segment     = 0;

    // int track = pos->staffIdx * VOICES + voice;
    track_idx_t track = pos->staffIdx * VOICES;

    for (segment = measure->first(SegmentType::ChordRest); segment;) {
        segment = getNextValidInputSegment(segment, track, voice);
        if (segment == 0) {
            break;
        }
        Segment* ns = getNextValidInputSegment(segment->next(SegmentType::ChordRest), track, voice);

        double x1 = segment->x();
        double x2;
        double d;
        if (ns) {
            x2    = ns->x();
            d     = x2 - x1;
        } else {
            x2    = measure->ldata()->bbox().width();
            d     = (x2 - x1) * 2.0;
            x     = x1;
            pos->segment = segment;
            break;
        }

        if (x < (x1 + d * .5)) {
            x = x1;
            pos->segment = segment;
            break;
        }
        segment = ns;
    }
    if (segment == 0) {
        return false;
    }
    //
    // TODO: restrict to reasonable values (pitch 0-127)
    //
    const Staff* s      = staff(pos->staffIdx);
    const Fraction tick = segment->tick();
    const double mag     = s->staffMag(tick);
    // in TABs, step from one string to another; in other staves, step on and between lines
    double lineDist = s->staffType(tick)->lineDistance().val()
                      * (s->isTabStaff(measure->tick()) ? 1 : .5)
                      * mag
                      * style().spatium();

    const double yOff = sstaff->yOffset();  // Get system staff vertical offset (usually for 1-line staves)
    pos->line = lrint((pppp.y() - sstaff->bbox().y() - yOff) / lineDist);
    if (s->isTabStaff(measure->tick())) {
        if (pos->line < -1 || pos->line > s->lines(tick) + 1) {
            return false;
        }
        if (pos->line < 0) {
            pos->line = 0;
        } else if (pos->line >= s->lines(tick)) {
            pos->line = s->lines(tick) - 1;
        }
    } else {
        int minLine   = absStep(0);
        ClefType clef = s->clef(pos->segment->tick());
        minLine       = relStep(minLine, clef);
        int maxLine   = absStep(127);
        maxLine       = relStep(maxLine, clef);

        if (pos->line > minLine || pos->line < maxLine) {
            return false;
        }
    }

    y         = sstaff->y() + pos->line * lineDist;
    pos->pos  = PointF(x, y) + measure->canvasPos();
    return true;
}

//---------------------------------------------------------
//   checkHasMeasures
//---------------------------------------------------------

bool Score::checkHasMeasures() const
{
    Page* page = pages().empty() ? 0 : pages().front();
    const std::vector<System*>* sl = page ? &page->systems() : 0;
    if (sl == 0 || sl->empty() || sl->front()->measures().empty()) {
        LOGD("first create measure, then repeat operation");
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   spatiumHasChanged
//---------------------------------------------------------

static void spatiumHasChanged(void* data, EngravingItem* e)
{
    double* val = (double*)data;
    e->spatiumChanged(val[0], val[1]);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Score::spatiumChanged(double oldValue, double newValue)
{
    double data[2];
    data[0] = oldValue;
    data[1] = newValue;
    scanElements(data, spatiumHasChanged, true);
    for (Staff* staff : m_staves) {
        staff->spatiumChanged(oldValue, newValue);
    }
    m_layoutOptions.noteHeadWidth = m_engravingFont->width(SymId::noteheadBlack, newValue / SPATIUM20);
    createPaddingTable();
}

//---------------------------------------------------------
//   updateStyle
//---------------------------------------------------------

static void updateStyle(void*, EngravingItem* e)
{
    bool v = e->generated();
    e->styleChanged();
    e->setGenerated(v);
}

//---------------------------------------------------------
//   styleChanged
//    must be called after every style change
//---------------------------------------------------------

void Score::styleChanged()
{
    scanElements(0, updateStyle);
    for (int i = 0; i < MAX_HEADERS; i++) {
        if (headerText(i)) {
            headerText(i)->styleChanged();
        }
    }
    for (int i = 0; i < MAX_FOOTERS; i++) {
        if (footerText(i)) {
            footerText(i)->styleChanged();
        }
    }
    for (Staff* staff : staves()) {
        for (int tick = 0; tick != -1; tick = staff->staffTypeRange(Fraction::fromTicks(tick + 1)).second) {
            StaffType* st = staff->staffType(Fraction::fromTicks(tick));
            if (!st) {
                continue;
            }
            st->styleChanged();
        }
    }
    createPaddingTable();
    setLayoutAll();
}

//---------------------------------------------------------
//   getCreateMeasure
//    - return Measure for tick
//    - create Factory::createMeasure(s) if there is no measure for
//      this tick
//---------------------------------------------------------

Measure* Score::getCreateMeasure(const Fraction& tick)
{
    Measure* last = lastMeasure();
    if (last == 0 || ((last->tick() + last->ticks()) <= tick)) {
        Fraction lastTick  = last ? (last->tick() + last->ticks()) : Fraction(0, 1);
        while (tick >= lastTick) {
            Measure* m = Factory::createMeasure(this->dummy()->system());
            Fraction ts = sigmap()->timesig(lastTick).timesig();
            m->setTick(lastTick);
            m->setTimesig(ts);
            m->setTicks(ts);
            measures()->append(toMeasureBase(m));
            lastTick += Fraction::fromTicks(ts.ticks());
        }
    }
    return tick2measure(tick);
}

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 Add \a element to its parent.

 Several elements (clef, keysig, timesig) need special handling, as they may cause
 changes throughout the score.
*/

void Score::addElement(EngravingItem* element)
{
    EngravingItem* parent = element->parentItem();
    element->triggerLayout();

//      LOGD("Score(%p) EngravingItem(%p)(%s) parent %p(%s)",
//         this, element, element->typeName(), parent, parent ? parent->typeName() : "");

    ElementType et = element->type();
    if (et == ElementType::MEASURE
        || (et == ElementType::HBOX && !(parent && parent->isVBox()))
        || et == ElementType::VBOX
        || et == ElementType::TBOX
        || et == ElementType::FBOX
        ) {
        measures()->add(toMeasureBase(element));
        element->triggerLayout();
        return;
    }

    if (parent) {
        parent->add(element);
    }

    switch (et) {
    case ElementType::BEAM:
    {
        Beam* b = toBeam(element);
        size_t n = b->elements().size();
        for (size_t i = 0; i < n; ++i) {
            b->elements().at(i)->setBeam(b);
        }
    }
    break;

    case ElementType::SLUR:
    case ElementType::HAMMER_ON_PULL_OFF:
        addLayoutFlags(LayoutFlag::PLAY_EVENTS);
    // fall through

    case ElementType::VOLTA:
    case ElementType::TRILL:
    case ElementType::VIBRATO:
    case ElementType::PEDAL:
    case ElementType::TEXTLINE:
    case ElementType::HAIRPIN:
    case ElementType::LET_RING:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::PALM_MUTE:
    case ElementType::WHAMMY_BAR:
    case ElementType::RASGUEADO:
    case ElementType::HARMONIC_MARK:
    case ElementType::PICK_SCRAPE:
    case ElementType::PARTIAL_LYRICSLINE:
    {
        Spanner* spanner = toSpanner(element);
        if (et == ElementType::TEXTLINE && spanner->anchor() == Spanner::Anchor::NOTE) {
            break;
        }
        addSpanner(spanner);
        for (SpannerSegment* ss : spanner->spannerSegments()) {
            if (ss->system()) {
                ss->system()->add(ss);
            }
        }
    }
    break;

    case ElementType::OTTAVA:
    {
        Ottava* o = toOttava(element);
        addSpanner(o);
        for (SpannerSegment* ss : o->spannerSegments()) {
            if (ss->system()) {
                ss->system()->add(ss);
            }
        }
        o->staff()->updateOttava();
        setPlaylistDirty();
    }
    break;

    case ElementType::DYNAMIC:
        setPlaylistDirty();
        break;

    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* ic = toInstrumentChange(element);
        ic->part()->setInstrument(ic->instrument(), ic->segment()->tick());
        addLayoutFlags(LayoutFlag::REBUILD_MIDI_MAPPING);
        cmdState().instrumentsChanged = true;
    }
    break;

    case ElementType::CHORD:
    {
        setPlaylistDirty();
        // May need to reconnect slur when inserting new chord
        SpannerMap& smap = spannerMap();
        Fraction tick = element->tick();
        auto spanners = smap.findOverlapping(tick.ticks(), tick.ticks());
        for (auto interval : spanners) {
            Spanner* s = interval.value;
            if (s->track() != element->track() || !s->isSlur()) {
                continue;
            }
            if (!s->startElement() && s->tick() == tick) {
                s->setStartElement(element);
            }
            if (!s->endElement() && s->tick2() == tick) {
                s->setEndElement(element);
            }
        }
        break;
    }
    case ElementType::HARMONY:
    case ElementType::FRET_DIAGRAM:
        element->part()->updateHarmonyChannels(true);
        break;
    case ElementType::GUITAR_BEND:
    {
        GuitarBend* bend = toGuitarBend(element);
        if (bend->type() == GuitarBendType::GRACE_NOTE_BEND || bend->type() == GuitarBendType::PRE_BEND) {
            Note* startNote = bend->startNote();
            Chord* startChord = startNote ? startNote->chord() : nullptr;
            if (startChord) {
                startChord->setNoStem(true);
                startChord->setBeamMode(BeamMode::NONE);
            }
        }
    }

    default:
        break;
    }

    if (element->isTextBase() && toTextBase(element)->hasParentSegment()) {
        MoveElementAnchors::checkMeasureBoundariesAndMoveIfNeed(element);
    }

    element->triggerLayout();
}

void Score::doUndoAddElement(EngravingItem* element)
{
    if (element->generated()) {
        addElement(element);
    } else {
        undo(new AddElement(element));
    }
}

//---------------------------------------------------------
//   removeElement
///   Remove \a element from its parent.
///   Several elements (clef, keysig, timesig) need special handling, as they may cause
///   changes throughout the score.
//---------------------------------------------------------

void Score::removeElement(EngravingItem* element)
{
    EngravingItem* parent = element->parentItem();
    element->triggerLayout();

    // special for MEASURE, HBOX, VBOX
    // their parent is not static

    ElementType et = element->type();
    bool parentIsVBox = parent && parent->isVBox();

    if (et == ElementType::MEASURE
        || (et == ElementType::HBOX && !parentIsVBox)
        || et == ElementType::VBOX
        || et == ElementType::TBOX
        || et == ElementType::FBOX
        ) {
        MeasureBase* mb = toMeasureBase(element);
        measures()->remove(mb);
        System* system = mb->system();

        if (!system) {
            // vertical boxes are not shown in continuous view so no system
#ifndef NDEBUG
            bool noSystemMode = lineMode() && (element->isVBox() || element->isTBox());
#endif
            assert(noSystemMode || !isOpen());
            return;
        }

        Page* page = system->page();
        if (element->isBox() && system->measures().size() == 1) {
            auto i = std::find(page->systems().begin(), page->systems().end(), system);
            page->systems().erase(i);
            mb->resetExplicitParent();
            if (page->systems().empty()) {
                // Remove this page, since it is now empty.
                // This involves renumbering and repositioning all subsequent pages.
                PointF pos = page->pos();
                auto ii = std::find(pages().begin(), pages().end(), page);
                pages().erase(ii);
                while (ii != pages().end()) {
                    page = *ii;
                    page->setNo(page->no() - 1);
                    PointF p = page->pos();
                    page->setPos(pos);
                    pos = p;
                    ii++;
                }
            }
        }
//            setLayout(mb->tick());
        return;
    }

    if (et == ElementType::BEAM) {            // beam parent does not survive layout
        element->resetExplicitParent();
        parent = 0;
    }

    if (parent) {
        parent->remove(element);
    }

    switch (et) {
    case ElementType::BEAM:
        for (ChordRest* cr : toBeam(element)->elements()) {
            cr->setBeam(0);
        }
        break;

    case ElementType::SLUR:
    case ElementType::HAMMER_ON_PULL_OFF:
        addLayoutFlags(LayoutFlag::PLAY_EVENTS);
    // fall through

    case ElementType::VOLTA:
    case ElementType::TRILL:
    case ElementType::VIBRATO:
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::PALM_MUTE:
    case ElementType::PARTIAL_LYRICSLINE:
    case ElementType::WHAMMY_BAR:
    case ElementType::RASGUEADO:
    case ElementType::HARMONIC_MARK:
    case ElementType::PICK_SCRAPE:
    case ElementType::TEXTLINE:
    case ElementType::HAIRPIN:
    {
        Spanner* spanner = toSpanner(element);
        if (et == ElementType::TEXTLINE && spanner->anchor() == Spanner::Anchor::NOTE) {
            break;
        }
        spanner->triggerLayout();
        removeSpanner(spanner);
    }
    break;

    case ElementType::GUITAR_BEND:
    {
        Note* startNote = toGuitarBend(element)->startNote();
        if (startNote) {
            startNote->setParenthesesMode(ParenthesesMode::NONE);
            startNote->chord()->setNoStem(false);
            startNote->chord()->setBeamMode(BeamMode::AUTO);
        }
        Note* endNote = toGuitarBend(element)->endNote();
        if (endNote) {
            endNote->setGhost(false);
            endNote->setVisible(true);
            const StringData* stringData = endNote->part()->stringData(endNote->tick(), endNote->staffIdx());
            int endFret = stringData->fret(endNote->pitch(), endNote->string(), endNote->staff());
            endNote->setFret(endFret);
        }
        break;
    }

    case ElementType::OTTAVA:
    {
        Ottava* o = toOttava(element);
        o->triggerLayout();
        removeSpanner(o);
        o->staff()->updateOttava();
        setPlaylistDirty();
    }
    break;

    case ElementType::DYNAMIC:
        setPlaylistDirty();
        break;

    case ElementType::CHORD:
    case ElementType::REST:
    case ElementType::MMREST:
    {
        ChordRest* cr = toChordRest(element);
        if (cr->beam()) {
            cr->beam()->remove(cr);
        }
        for (Lyrics* lyr : cr->lyrics()) {
            lyr->removeFromScore();
        }
        // TODO: check for tuplet?
    }
    break;
    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* ic = toInstrumentChange(element);
        ic->part()->removeInstrument(ic->segment()->tick());
        addLayoutFlags(LayoutFlag::REBUILD_MIDI_MAPPING);
        cmdState().instrumentsChanged = true;
    }
    break;

    case ElementType::HARMONY:
    case ElementType::FRET_DIAGRAM:
        element->part()->updateHarmonyChannels(true, true);
        break;

    default:
        break;
    }
}

void Score::doUndoRemoveElement(EngravingItem* element)
{
    if (element->generated()) {
        removeElement(element);
        //! HACK: don't delete as it may still be used in Inspector
        // element->deleteLater();
    } else {
        undo(new RemoveElement(element));
    }
}

bool Score::canReselectItem(const EngravingItem* item) const
{
    if (!item || item->selected()) {
        return false;
    }

    EngravingItem* seg = const_cast<EngravingItem*>(item->findAncestor(ElementType::SEGMENT));
    if (seg) {
        std::vector<EngravingItem*> elements;
        seg->scanElements(&elements, collectElements, false /*all*/);
        return muse::contains(elements, const_cast<EngravingItem*>(item));
    }

    return true;
}

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* Score::firstMeasure() const
{
    MeasureBase* mb = m_measures.first();
    while (mb && mb->type() != ElementType::MEASURE) {
        mb = mb->next();
    }

    return toMeasure(mb);
}

//---------------------------------------------------------
//   firstMeasureMM
//---------------------------------------------------------

Measure* Score::firstMeasureMM() const
{
    Measure* m = firstMeasure();
    if (m && style().styleB(Sid::createMultiMeasureRests) && m->hasMMRest()) {
        return m->mmRest();
    }
    return m;
}

//---------------------------------------------------------
//   firstMM
//---------------------------------------------------------

MeasureBase* Score::firstMM() const
{
    MeasureBase* m = m_measures.first();
    if (m
        && m->type() == ElementType::MEASURE
        && style().styleB(Sid::createMultiMeasureRests)
        && toMeasure(m)->hasMMRest()) {
        return toMeasure(m)->mmRest();
    }
    return m;
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

MeasureBase* Score::measure(int idx) const
{
    MeasureBase* mb = m_measures.first();
    for (int i = 0; i < idx; ++i) {
        mb = mb->next();
        if (mb == 0) {
            return 0;
        }
    }
    return mb;
}

//---------------------------------------------------------
//   crMeasure
//    Returns a measure containing chords and/or rests
//    by its index, skipping other MeasureBase descendants
//---------------------------------------------------------

Measure* Score::crMeasure(int idx) const
{
    int i = -1;
    for (MeasureBase* mb = m_measures.first(); mb; mb = mb->next()) {
        if (mb->isMeasure()) {
            ++i;
        }
        if (i == idx) {
            return toMeasure(mb);
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* Score::lastMeasure() const
{
    MeasureBase* mb = m_measures.last();

    if (!mb) {
        return nullptr;
    }

    while (mb && mb->type() != ElementType::MEASURE) {
        mb = mb->prev();
    }
    return toMeasure(mb);
}

//---------------------------------------------------------
//   lastMeasureMM
//---------------------------------------------------------

Measure* Score::lastMeasureMM() const
{
    Measure* m = lastMeasure();
    return m ? const_cast<Measure*>(m->coveringMMRestOrThis()) : nullptr;
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction Score::endTick() const
{
    Measure* m = lastMeasure();
    return m ? m->endTick() : Fraction(0, 1);
}

//---------------------------------------------------------
//   firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(SegmentType segType) const
{
    Segment* seg;
    Measure* m = firstMeasure();
    if (!m) {
        seg = 0;
    } else {
        seg = m->first();
        if (seg && !(seg->segmentType() & segType)) {
            seg = seg->next1(segType);
        }
    }
    return seg;
}

//---------------------------------------------------------
//   firstSegmentMM
//---------------------------------------------------------

Segment* Score::firstSegmentMM(SegmentType segType) const
{
    Measure* m = firstMeasureMM();
    return m ? m->first(segType) : 0;
}

//---------------------------------------------------------
//   lastSegment
//---------------------------------------------------------

Segment* Score::lastSegment() const
{
    Measure* m = lastMeasure();
    return m ? m->last() : 0;
}

//---------------------------------------------------------
//   lastSegmentMM
//---------------------------------------------------------

Segment* Score::lastSegmentMM() const
{
    Measure* m = lastMeasureMM();
    return m ? m->last() : 0;
}

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

double Score::utick2utime(int tick) const
{
    return repeatList().utick2utime(tick);
}

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int Score::utime2utick(double utime) const
{
    return repeatList().utime2utick(utime);
}

//---------------------------------------------------------
//   scanElementsInRange
//---------------------------------------------------------

void Score::scanElementsInRange(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    Segment* startSeg = m_selection.startSegment();
    for (Segment* s = startSeg; s && s != m_selection.endSegment(); s = s->next1()) {
        s->scanElements(data, func, all);
        Measure* m = s->measure();
        if (m && s == m->first()) {
            Measure* mmr = m->mmRest();
            if (mmr) {
                mmr->scanElements(data, func, all);
            }
        }
    }

    std::set<Spanner*> handledSpanners;
    for (EngravingItem* e : m_selection.elements()) {
        if (!e->isSpannerSegment()) {
            continue;
        }
        Spanner* spanner = toSpannerSegment(e)->spanner();
        if (handledSpanners.insert(spanner).second) {
            for (SpannerSegment* ss : spanner->spannerSegments()) {
                ss->scanElements(data, func, all);
            }
        }
    }
}

//---------------------------------------------------------
//   setSelection
//---------------------------------------------------------

void Score::setSelection(const Selection& s)
{
    deselectAll();
    m_selection = s;

    for (EngravingItem* e : m_selection.elements()) {
        e->setSelected(true);
    }
}

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

Text* Score::getText(TextStyleType tid) const
{
    MeasureBase* m = first();
    if (m && m->isVBox()) {
        for (EngravingItem* e : m->el()) {
            if (e->isText() && toText(e)->textStyleType() == tid) {
                return toText(e);
            }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   metaTag
//---------------------------------------------------------

String Score::metaTag(const String& s) const
{
    if (muse::contains(m_metaTags, s)) {
        return m_metaTags.at(s);
    }

    return muse::value(m_masterScore->m_metaTags, s);
}

//---------------------------------------------------------
//   setMetaTag
//---------------------------------------------------------

void Score::setMetaTag(const String& tag, const String& val)
{
    m_metaTags.insert_or_assign(tag, val);
}

//---------------------------------------------------------
//   setSynthesizerState
//---------------------------------------------------------

void Score::setSynthesizerState(const SynthesizerState& s)
{
    // TODO: make undoable
    m_synthesizerState = s;
}

//---------------------------------------------------------
//   removeAudio
//---------------------------------------------------------

void Score::removeAudio()
{
    delete m_audio;
    m_audio = 0;
}

//---------------------------------------------------------
//   appendScore
//---------------------------------------------------------

bool Score::appendScore(Score* score, bool addPageBreak, bool addSectionBreak)
{
    if (parts().size() < score->parts().size() || staves().size() < score->staves().size()) {
        LOGD("Score to append has %zu parts and %zu staves, but this score only has %zu parts and %zu staves.",
             score->parts().size(), score->staves().size(), parts().size(), staves().size());
        return false;
    }

    if (!last()) {
        LOGD("This score doesn't have any MeasureBase objects.");
        return false;
    }

    // apply Page/Section Breaks if desired
    if (addPageBreak) {
        if (!last()->pageBreak()) {
            last()->undoSetBreak(false, LayoutBreakType::LINE);       // remove line break if exists
            last()->undoSetBreak(true, LayoutBreakType::PAGE);        // apply page break
        }
    } else if (!last()->lineBreak() && !last()->pageBreak()) {
        last()->undoSetBreak(true, LayoutBreakType::LINE);
    }

    if (addSectionBreak && !last()->sectionBreak()) {
        last()->undoSetBreak(true, LayoutBreakType::SECTION);
    }

    // match concert pitch states
    if (style().styleB(Sid::concertPitch) != score->style().styleB(Sid::concertPitch)) {
        score->cmdConcertPitchChanged(style().styleB(Sid::concertPitch));
    }

    // clone the measures
    appendMeasuresFromScore(score, Fraction(0, 1), score->last()->endTick());

    setLayoutAll();
    return true;
}

//---------------------------------------------------------
//   appendMeasuresFromScore
//     clone measures from another score to the end of this
//---------------------------------------------------------

bool Score::appendMeasuresFromScore(Score* score, const Fraction& startTick, const Fraction& endTick)
{
    Fraction tickOfAppend = last()->endTick();
    TieMap tieMap;

    MeasureBase* fmb = score->tick2measureBase(startTick);
    MeasureBase* emb = score->tick2measureBase(endTick);
    Fraction curTick = tickOfAppend;
    for (MeasureBase* cmb = fmb; cmb != emb; cmb = cmb->next()) {
        MeasureBase* nmb;
        if (cmb->isMeasure()) {
            Measure* nm = toMeasure(cmb)->cloneMeasure(this, curTick, &tieMap);
            curTick += nm->ticks();
            nmb = toMeasureBase(nm);
        } else {
            nmb = static_cast<MeasureBase*>(cmb->clone());
        }

        nmb->setScore(this);
        measures()->append(nmb);
    }

    Measure* firstAppendedMeasure = tick2measure(tickOfAppend);

    // if the appended score has less staves,
    // make sure the measures have full measure rest
    for (Measure* m = firstAppendedMeasure; m; m = m->nextMeasure()) {
        for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Fraction f;
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (voice_idx_t v = 0; v < VOICES; ++v) {
                    ChordRest* cr = toChordRest(s->element(staffIdx * VOICES + v));
                    if (cr == 0) {
                        continue;
                    }
                    f += cr->actualTicks();
                }
            }
            if (f.isZero()) {
                addRest(m->tick(), staffIdx * VOICES, TDuration(DurationType::V_MEASURE), 0);
            }
        }
    }

    // at first added measure, check if we need to add Clef/Key/TimeSig
    //  this is needed if it was changed and needs to be changed back
    size_t n = nstaves();
    Fraction otick = fmb->tick(), ctick = tickOfAppend;
    for (staff_idx_t staffIdx = 0; staffIdx < n; ++staffIdx) {   // iterate over all staves
        track_idx_t trackIdx = staff2track(staffIdx);     // idx of irst track on the staff
        Staff* staff = this->staff(staffIdx);
        Staff* ostaff = score->staff(staffIdx);

        // check if key signature needs to be changed
        if (ostaff->key(otick) != staff->key(ctick)) {
            Segment* ns = firstAppendedMeasure->undoGetSegment(SegmentType::KeySig, ctick);
            KeySigEvent nkse = KeySigEvent(ostaff->keySigEvent(otick));
            KeySig* nks = Factory::createKeySig(ns);
            nks->setScore(this);
            nks->setTrack(trackIdx);

            nks->setKeySigEvent(nkse);
            staff->setKey(ctick, nkse);
            ns->add(nks);
        }
        // check if a key signature is present but is spurious (i.e. no actual change)
        else if (staff->currentKeyTick(ctick) == ctick
                 && staff->key(ctick - Fraction::fromTicks(1)) == ostaff->key(otick)) {
            Segment* ns = firstAppendedMeasure->first(SegmentType::KeySig);
            if (ns) {
                ns->remove(ns->element(trackIdx));
            }
        }

        // check if time signature needs to be changed
        TimeSig* ots = ostaff->timeSig(otick), * cts = staff->timeSig(ctick);
        TimeSig* pts = staff->timeSig(ctick - Fraction::fromTicks(1));
        if (ots && cts && *ots != *cts) {
            Segment* ns = firstAppendedMeasure->undoGetSegment(SegmentType::TimeSig, ctick);
            TimeSig* nsig = new TimeSig(*ots);

            nsig->setScore(this);
            nsig->setTrack(trackIdx);
            ns->add(nsig);
        }
        // check if a time signature is present but is spurious (i.e. no actual change)
        else if (staff->currentTimeSigTick(ctick) == ctick
                 && ots && pts && *pts == *ots) {
            Segment* ns = firstAppendedMeasure->first(SegmentType::TimeSig);
            if (ns) {
                ns->remove(ns->element(trackIdx));
            }
        }

        // check if clef signature needs to be changed
        if (ostaff->clef(otick) != staff->clef(ctick)) {
            undoChangeClef(staff, firstAppendedMeasure, ostaff->clef(otick));
        }
        // check if a clef change is present but is spurious (i.e. no actual change)
        else if (staff->currentClefTick(ctick) == ctick
                 && staff->clef(ctick - Fraction::fromTicks(1)) == ostaff->clef(otick)) {
            Segment* ns = firstAppendedMeasure->first(SegmentType::Clef);
            if (!ns) {
                ns = firstAppendedMeasure->first(SegmentType::HeaderClef);
            }
            if (ns) {
                ns->remove(ns->element(trackIdx));
            }
        }
    }

    // check if section starts with a pick-up measure to be merged with end of previous section
    Measure* cm = firstAppendedMeasure, * pm = cm->prevMeasure();
    if (pm->timesig() == cm->timesig() && pm->ticks() + cm->ticks() == cm->timesig()) {
        cmdJoinMeasure(pm, cm);
    }

    // clone the spanners (only in the range currently copied)
    auto ospans = score->spanner();
    auto lb = ospans.lower_bound(startTick.ticks()), ub = ospans.upper_bound(endTick.ticks());
    for (auto sp = lb; sp != ub; sp++) {
        Spanner* spanner = sp->second;

        if (spanner->tick2() > endTick) {
            continue;                                 // map is by tick() so this can still happen in theory...
        }
        Spanner* ns = toSpanner(spanner->clone());
        ns->setScore(this);
        ns->resetExplicitParent();
        ns->setTick(spanner->tick() - startTick + tickOfAppend);
        ns->setTick2(spanner->tick2() - startTick + tickOfAppend);
        ns->computeStartElement();
        ns->computeEndElement();
        addElement(ns);
    }

    return true;
}

//---------------------------------------------------------
//   splitStaff
//---------------------------------------------------------

void Score::splitStaff(staff_idx_t staffIdx, int splitPoint)
{
//      LOGD("split staff %d point %d", staffIdx, splitPoint);

    //
    // create second staff
    //
    Staff* st = staff(staffIdx);
    Part* p  = st->part();
    Staff* ns = Factory::createStaff(p);
    ns->init(st);

    // convert staffIdx from score-relative to part-relative
    staff_idx_t staffIdxPart = staffIdx - p->staff(0)->idx();
    undoInsertStaff(ns, staffIdxPart + 1, false);

    Segment* seg = firstMeasure()->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
    Clef* clef = Factory::createClef(seg);
    clef->setClefType(ClefType::F);
    clef->setTrack((staffIdx + 1) * VOICES);
    clef->setParent(seg);
    clef->setIsHeader(true);
    undoAddElement(clef);

    undoChangeKeySig(ns, Fraction(0, 1), st->keySigEvent(Fraction(0, 1)));

    masterScore()->rebuildMidiMapping();
    cmdState().instrumentsChanged = true;
    doLayout();

    //
    // move notes
    //
    select(0, SelectType::SINGLE, 0);
    track_idx_t strack = staffIdx * VOICES;
    track_idx_t dtrack = (staffIdx + 1) * VOICES;

    // Keep track of ties to be reconnected.
    struct OldTie {
        Tie* tie;
        Note* nnote;
    };
    std::map<Note*, OldTie> oldTies;

    // Notes under the split point can be part of a tuplet, so keep track
    // of the tuplet mapping too!
    std::map<Tuplet*, Tuplet*> tupletMapping;
    Tuplet* tupletSrc[VOICES] = { };
    Tuplet* tupletDst[VOICES] = { };

    for (Segment* s = firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            EngravingItem* e = s->element(strack + voice);

            if (!e) {
                continue;
            }
            if (toDurationElement(e)->tuplet()) {
                tupletSrc[voice] = toDurationElement(e)->tuplet();
                if (muse::contains(tupletMapping, tupletSrc[voice])) {
                    tupletDst[voice] = tupletMapping[tupletSrc[voice]];
                } else {
                    tupletDst[voice] = Factory::copyTuplet(*tupletSrc[voice]);
                    tupletDst[voice]->setTrack(dtrack);
                    tupletMapping.insert({ tupletSrc[voice], tupletDst[voice] });
                }
            } else {
                tupletSrc[voice] = nullptr;
                tupletDst[voice] = nullptr;
            }

            bool createRestDst = true;
            bool createRestSrc = false;
            TDuration lengthDst = toChordRest(e)->actualDurationType();
            TDuration lengthSrc = TDuration();

            if (e->isChord()) {
                Chord* c = toChord(e);
                std::list<Note*> removeNotes;
                for (Note* note : c->notes()) {
                    if (note->pitch() >= splitPoint) {
                        continue;
                    } else {
                        Chord* chord = toChord(s->element(dtrack + voice));
                        assert(!chord || (chord->isChord()));
                        if (!chord) {
                            chord = Factory::copyChord(*c);
                            muse::DeleteAll(chord->notes());
                            chord->notes().clear();
                            chord->setTuplet(tupletDst[voice]);
                            chord->setTrack(dtrack + voice);
                            undoAddElement(chord);
                        }
                        Note* nnote = Factory::copyNote(*note);
                        if (note->tieFor()) {
                            // Save the note and the tie for processing later.
                            // Use the end note as index in the map so, when this is found
                            // we know the tie has to be recreated.
                            oldTies.insert({ note->tieFor()->endNote(), OldTie { note->tieFor(), nnote } });
                        }
                        nnote->setTrack(dtrack + voice);
                        chord->add(nnote);
                        nnote->updateLine();
                        removeNotes.push_back(note);
                        createRestDst = false;
                        lengthDst = chord->actualDurationType();

                        // Is the note the last note of a tie?
                        if (muse::contains(oldTies, note)) {
                            // Yes! Create a tie between the new notes and remove the
                            // old tie.
                            Tie* tie = oldTies[note].tie->clone();
                            tie->setStartNote(oldTies[note].nnote);
                            tie->setEndNote(nnote);
                            tie->setTrack(nnote->track());
                            undoAddElement(tie);
                            undoRemoveElement(oldTies[note].tie);
                            muse::remove(oldTies, note);
                        }
                    }
                }
                createRestSrc = false;
                for (Note* note : removeNotes) {
                    undoRemoveElement(note);
                    Chord* chord = note->chord();
                    if (chord->notes().empty()) {
                        for (auto sp : spanner()) {
                            Slur* slur = toSlur(sp.second);
                            if (slur->type() != ElementType::SLUR) {
                                continue;
                            }
                            if (slur->startCR() == chord) {
                                slur->undoChangeProperty(Pid::TRACK, slur->track() + VOICES);
                                for (EngravingObject* ee : slur->linkList()) {
                                    Slur* lslur = toSlur(ee);
                                    lslur->setStartElement(0);
                                }
                            }
                            if (slur->endCR() == chord) {
                                slur->undoChangeProperty(Pid::SPANNER_TRACK2, slur->track2() + VOICES);
                                for (EngravingObject* ee : slur->linkList()) {
                                    Slur* lslur = toSlur(ee);
                                    lslur->setEndElement(0);
                                }
                            }
                        }
                        createRestSrc = true;
                        lengthSrc = chord->actualDurationType();
                        undoRemoveElement(chord);
                    }
                }
            }

            if (createRestSrc) {
                addRest(s, strack + voice, lengthSrc, tupletSrc[voice]);
            }
            if (createRestDst) {
                addRest(s, dtrack + voice, lengthDst, tupletDst[voice]);
            }
        }
    }
}

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Score::cmdRemovePart(Part* part)
{
    if (!part) {
        return;
    }

    staff_idx_t sidx = staffIdx(part);
    size_t n = part->nstaves();

    for (staff_idx_t i = 0; i < n; ++i) {
        cmdRemoveStaff(sidx);
    }

    undoRemovePart(part, muse::indexOf(m_parts, part));
}

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, size_t targetPartIdx)
{
    if (!part) {
        return;
    }

    part->setScore(this);
    assignIdIfNeed(*part);

    if (targetPartIdx < m_parts.size()) {
        m_parts.insert(m_parts.begin() + targetPartIdx, part);
    } else {
        m_parts.push_back(part);
    }

    masterScore()->rebuildMidiMapping();
    setInstrumentsChanged(true);
    markInstrumentsAsPrimary(m_parts);
}

void Score::appendPart(Part* part)
{
    if (!part) {
        return;
    }

    part->setScore(this);
    assignIdIfNeed(*part);
    m_parts.push_back(part);
    markInstrumentsAsPrimary(m_parts);
}

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
{
    part_idx_t index = muse::indexOf(m_parts, part);

    if (index == muse::nidx) {
        for (size_t i = 0; i < m_parts.size(); ++i) {
            if (m_parts[i]->id() == part->id()) {
                index = i;
                break;
            }
        }
    }

    m_parts.erase(m_parts.begin() + index);
    markInstrumentsAsPrimary(m_parts);

    if (m_excerpt) {
        for (Part* excerptPart : m_excerpt->parts()) {
            if (excerptPart->id() != part->id()) {
                continue;
            }

            muse::remove(m_excerpt->parts(), excerptPart);
            markInstrumentsAsPrimary(m_excerpt->parts());
            break;
        }
    }

    masterScore()->rebuildMidiMapping();
    setInstrumentsChanged(true);
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Score::insertStaff(Staff* staff, staff_idx_t ridx)
{
    if (!staff || !staff->part()) {
        return;
    }

    assignIdIfNeed(*staff);
    staff->part()->insertStaff(staff, ridx);

    staff_idx_t idx = staffIdx(staff->part()) + ridx;
    m_staves.insert(m_staves.begin() + idx, staff);

    for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
        Spanner* s = i->second;
        if (!moveDownWhenAddingStaves(s, idx)) {
            continue;
        }
        if (s->staffIdx() >= idx) {
            track_idx_t t = s->track() + VOICES;
            if (t >= ntracks()) {
                t = ntracks() - 1;
            }
            s->setTrack(t);
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setTrack(t);
            }
            if (s->track2() != muse::nidx) {
                t = s->track2() + VOICES;
                s->setTrack2(t < ntracks() ? t : s->track());
            }
        }
    }

    updateStavesNumberForSystems();
}

void Score::appendStaff(Staff* staff)
{
    if (!staff || !staff->part()) {
        return;
    }

    assignIdIfNeed(*staff);
    staff->part()->appendStaff(staff);
    m_staves.push_back(staff);

    updateStavesNumberForSystems();
}

void Score::assignIdIfNeed(Staff& staff) const
{
    if (staff.id() == INVALID_ID) {
        staff.setId(newStaffId());
    }
}

void Score::assignIdIfNeed(Part& part) const
{
    if (part.id() == INVALID_ID) {
        part.setId(newPartId());
    }
}

void Score::updateStavesNumberForSystems()
{
    for (System* system : m_systems) {
        if (!system->firstMeasure()) {
            continue;
        }

        system->adjustStavesNumber(nstaves());
    }
}

ID Score::newStaffId() const
{
    ID maxId = 0;

    for (const Staff* staff : score()->staves()) {
        maxId = std::max(maxId, staff->id());
    }

    return maxId + 1;
}

ID Score::newPartId() const
{
    ID maxId = 0;

    for (const Part* part : score()->parts()) {
        maxId = std::max(maxId, part->id());
    }

    return maxId + 1;
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
{
    staff_idx_t idx = staff->idx();
    for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
        Spanner* s = i->second;
        if (s->staffIdx() > idx) {
            track_idx_t t =  s->track() >= VOICES ? s->track() - VOICES : 0;
            s->setTrack(t);
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setTrack(t);
            }
            if (s->track2() != muse::nidx) {
                t = s->track2() - VOICES;
                s->setTrack2((t != muse::nidx) ? t : s->track());
            }
        }
    }

    muse::remove(m_staves, staff);
    staff->part()->removeStaff(staff);

    if (isSystemObjectStaff(staff)) {
        muse::remove(m_systemObjectStaves, staff);
    }

    updateStavesNumberForSystems();
}

//---------------------------------------------------------
//   adjustBracketsDel
//---------------------------------------------------------

void Score::adjustBracketsDel(size_t sidx, size_t eidx)
{
    IF_ASSERT_FAILED(sidx < eidx && eidx <= m_staves.size()) {
        return;
    }

    for (size_t staffIdx = 0; staffIdx < eidx; ++staffIdx) {
        Staff* staff = m_staves[staffIdx];
        for (BracketItem* bi : staff->brackets()) {
            size_t span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) <= sidx)) {
                continue;
            }
            const bool startsOutsideDeletedRange = (staffIdx < sidx);
            const bool endsOutsideDeletedRange = ((staffIdx + span) >= eidx);
            if (startsOutsideDeletedRange && endsOutsideDeletedRange) {
                // Shorten the bracket by the number of staves deleted
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(span - (eidx - sidx)));
            } else if (startsOutsideDeletedRange) {
                // Shorten the bracket by the number of staves deleted that were spanned by it
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(sidx - staffIdx));
            } else if (endsOutsideDeletedRange) {
                if (eidx < m_staves.size()) {
                    // Move the bracket past the end of the deleted range,
                    // and shorten it by the number of staves deleted that were spanned by it.
                    // That is, add a new bracket; the old one will be removed when removing the staves.

                    undoAddBracket(m_staves.at(eidx), bi->column(), bi->bracketType(), int(span - (eidx - staffIdx)));
                }
            }
        }
    }
}

//---------------------------------------------------------
//   adjustBracketsIns
//---------------------------------------------------------

void Score::adjustBracketsIns(size_t sidx, size_t eidx)
{
    for (size_t staffIdx = 0; staffIdx < m_staves.size(); ++staffIdx) {
        Staff* staff = m_staves[staffIdx];
        for (BracketItem* bi : staff->brackets()) {
            size_t span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx)) {
                continue;
            }
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(span + (eidx - sidx)));
            }
        }
    }
}

//---------------------------------------------------------
//   adjustKeySigs
//---------------------------------------------------------

void Score::adjustKeySigs(track_idx_t sidx, track_idx_t eidx, KeyList km)
{
    for (track_idx_t staffIdx = sidx; staffIdx < eidx; ++staffIdx) {
        Staff* staff = m_staves[staffIdx];
        for (auto i = km.begin(); i != km.end(); ++i) {
            Fraction tick = Fraction::fromTicks(i->first);
            Measure* measure = tick2measure(tick);
            if (!measure) {
                continue;
            }
            if (staff->isDrumStaff(tick)) {
                continue;
            }
            KeySigEvent key = i->second;
            Interval v = staff->part()->instrument(tick)->transpose();
            if (!v.isZero() && !style().styleB(Sid::concertPitch) && !key.isAtonal()) {
                v.flip();
                key.setKey(transposeKey(key.concertKey(), v, staff->part()->preferSharpFlat()));
            }
            staff->setKey(tick, key);

            Segment* s = measure->getSegment(SegmentType::KeySig, tick);
            KeySig* keysig = Factory::createKeySig(s);
            keysig->setParent(s);
            keysig->setTrack(staffIdx * VOICES);
            keysig->setKeySigEvent(key);
            doUndoAddElement(keysig);
        }
    }
}

//---------------------------------------------------------
//   getKeyList
//      This is taken from MuseScore::editInstrList()
//---------------------------------------------------------

KeyList Score::keyList() const
{
    // find the keylist of the first pitched staff
    KeyList tmpKeymap;
    Staff* firstStaff = nullptr;
    bool isFirstStaff = true;
    for (Staff* s : masterScore()->staves()) {
        if (!s->isDrumStaff(Fraction(0, 1))) {
            KeyList* km = s->keyList();
            if (isFirstStaff) {
                isFirstStaff = false;
                tmpKeymap.insert(km->begin(), km->end());
                firstStaff = s;
                continue;
            }

            // in terms of ticks, tmpKeymap = intersection(tmpKeymap, km)
            // note that if every single staff has a custom local keysig on this tick,
            // we will just copy the one from the top staff.
            auto currKey = km->begin();
            KeyList kl;
            for (auto key : tmpKeymap) {
                if (key.first < (*currKey).first) {
                    continue;
                }
                while ((*currKey).first < key.first && currKey != km->end()) {
                    currKey++;
                }
                if (currKey == km->end()) {
                    break;
                }
                if (key.first == (*currKey).first) {
                    // there is a matching key sig on this staff
                    ++currKey;
                    kl.insert(key);
                    if (currKey == km->end()) {
                        break;
                    }
                }
            }
            tmpKeymap.clear();
            tmpKeymap.insert(kl.begin(), kl.end());
        }
    }
    Key normalizedC = Key::C;
    bool needNormalize = firstStaff && !masterScore()->style().styleB(Sid::concertPitch)
                         && (firstStaff->part()->instrument()->transpose().chromatic || firstStaff->part()->instruments().size() > 1);
    // normalize the keyevents to concert pitch if necessary
    KeyList nkl;
    for (auto key : tmpKeymap) {
        if (firstStaff && !key.second.forInstrumentChange()) {
            if (needNormalize) {
                Key cKey = key.second.concertKey();
                key.second.setKey(cKey);
            }
            nkl.insert(key);
        }
    }
    if (firstStaff && !masterScore()->style().styleB(Sid::concertPitch)
        && (firstStaff->part()->instrument()->transpose().chromatic || firstStaff->part()->instruments().size() > 1)) {
        int interval = firstStaff->part()->instrument()->transpose().chromatic;
        normalizedC = transposeKey(normalizedC, interval);
    }

    // create initial keyevent for transposing instrument if necessary
    auto i = nkl.begin();
    if (i == nkl.end() || i->first != 0) {
        nkl[0].setConcertKey(normalizedC);
    }

    return nkl;
}

//---------------------------------------------------------
//   cmdRemoveStaff
//---------------------------------------------------------

void Score::cmdRemoveStaff(staff_idx_t staffIdx)
{
    Staff* s = staff(staffIdx);
    adjustBracketsDel(staffIdx, staffIdx + 1);

    undoRemoveStaff(s);
}

void Score::sortSystemObjects(std::vector<staff_idx_t>& dst)
{
    std::vector<staff_idx_t> moveTo;
    for (const Staff* staff : m_systemObjectStaves) {
        staff_idx_t oldStaffIdx = staff->idx();
        staff_idx_t newStaffIfx = oldStaffIdx < dst.size() ? dst[oldStaffIdx] : muse::nidx;

        if (muse::contains(moveTo, newStaffIfx)) {
            newStaffIfx = muse::nidx;
        }

        moveTo.push_back(newStaffIfx);
    }

    for (staff_idx_t i = 0; i < m_systemObjectStaves.size(); i++) {
        if (moveTo[i] == m_systemObjectStaves[i]->idx()) {
            // this sysobj staff doesn't move
            continue;
        } else {
            // move all system objects from systemObjectStaves[i] to _staves[moveTo[i]]
            for (MeasureBase* mb = measures()->first(); mb; mb = mb->next()) {
                if (!mb->isMeasure()) {
                    continue;
                }

                Measure* m = toMeasure(mb);
                for (EngravingItem* e : m->el()) {
                    if ((e->isJump() || e->isMarker()) && e->isLinked() && e->track() == staff2track(m_systemObjectStaves[i]->idx())) {
                        if (moveTo[i] == muse::nidx) {
                            // delete this clone
                            m->remove(e);
                            e->unlink();
                            delete e;
                        } else {
                            e->setTrack(staff2track(m_staves[moveTo[i]]->idx()));
                        }
                    }
                }

                for (Segment* s = m->first(); s; s = s->next()) {
                    if (s->isChordRest() || !s->annotations().empty()) {
                        const auto annotations = s->annotations(); // make a copy since we alter the list
                        for (EngravingItem* e : annotations) {
                            if (e->isRehearsalMark()
                                || e->isSystemText()
                                || e->isTripletFeel()
                                || e->isTempoText()
                                || isSystemTextLine(e)) {
                                if (e->track() == staff2track(m_systemObjectStaves[i]->idx()) && e->isLinked()) {
                                    if (moveTo[i] == muse::nidx) {
                                        s->removeAnnotation(e);
                                        e->unlink();
                                        delete e;
                                    } else {
                                        e->setTrack(staff2track(m_staves[moveTo[i]]->idx()));
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // update systemObjectStaves with the correct staff
            if (moveTo[i] == muse::nidx) {
                m_systemObjectStaves.erase(m_systemObjectStaves.begin() + i);
                moveTo.erase(moveTo.begin() + i);
            } else {
                m_systemObjectStaves[i] = m_staves[moveTo[i]];
            }
        }
    }
}

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Score::sortStaves(std::vector<staff_idx_t>& dst)
{
    sortSystemObjects(dst);
    muse::DeleteAll(systems());
    systems().clear();    //??
    m_parts.clear();
    Part* curPart = nullptr;
    std::vector<Staff*> dl;
    std::map<size_t, size_t> trackMap;
    track_idx_t track = 0;
    for (staff_idx_t idx : dst) {
        Staff* staff = m_staves[idx];
        if (staff->part() != curPart) {
            curPart = staff->part();
            curPart->clearStaves();
            m_parts.push_back(curPart);
        }
        curPart->appendStaff(staff);
        dl.push_back(staff);
        for (size_t itrack = 0; itrack < VOICES; ++itrack) {
            trackMap.insert({ idx* VOICES + itrack, track++ });
        }
    }
    m_staves = dl;

    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        m->sortStaves(dst);
        if (m->hasMMRest()) {
            m->mmRest()->sortStaves(dst);
        }
    }
    for (auto i : m_spanner.map()) {
        Spanner* sp = i.second;
        voice_idx_t voice    = sp->voice();
        staff_idx_t staffIdx = sp->staffIdx();
        staff_idx_t idx = muse::indexOf(dst, staffIdx);
        if (idx != muse::nidx && !sp->isTopSystemObject()) {
            sp->setTrack(idx * VOICES + voice);
            if (sp->track2() != muse::nidx) {
                sp->setTrack2(idx * VOICES + (sp->track2() % VOICES));        // at least keep the voice...
            }
        }
    }
    setLayoutAll();
    markInstrumentsAsPrimary(m_parts);
}

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag)
{
    if (flag == style().styleB(Sid::concertPitch)) {
        return;
    }

    undoChangeStyleVal(Sid::concertPitch, flag);         // change style flag

    for (Staff* staff : m_staves) {
        if (staff->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {         // TODO
            continue;
        }
        // if this staff has no transposition, and no instrument changes, we can skip it
        Interval interval = staff->part()->instrument()->transpose(); //tick?
        if (interval.isZero() && staff->part()->instruments().size() == 1) {
            continue;
        }
        if (!flag) {
            interval.flip();
        }

        staff_idx_t staffIdx   = staff->idx();
        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack   = startTrack + VOICES;

        transposeKeys(staffIdx, staffIdx + 1, Fraction(0, 1), lastSegment()->tick(), !flag);

        for (Segment* segment = firstSegment(SegmentType::ChordRest); segment; segment = segment->next1(SegmentType::ChordRest)) {
            interval = staff->transpose(segment->tick());
            if (!flag) {
                interval.flip();
            }
            for (EngravingItem* e : segment->annotations()) {
                if (!e->isHarmony() || (e->track() < startTrack) || (e->track() >= endTrack)) {
                    continue;
                }
                Harmony* h  = toHarmony(e);
                for (EngravingObject* se : h->linkList()) {
                    // don't transpose all links
                    // just ones resulting from mmrests
                    Harmony* he = toHarmony(se);              // toHarmony() does not work as e is an ScoreElement
                    if (he->staff() == h->staff()) {
                        undoTransposeHarmony(he, interval);
                    }
                }
                //realized harmony should be invalid after a transpose command
                assert(!h->realizedHarmony().valid());
            }
        }
    }
}

void Score::padToggle(Pad p, bool toggleForSelectionOnly)
{
    if (!noteEntryMode()) {
        for (ChordRest* cr : getSelectedChordRests()) {
            if (cr->isChord() && toChord(cr)->isTrillCueNote()) {
                return;
            }
        }
    }

    const TDuration oldDuration = m_is.duration();
    const bool oldRest = m_is.rest();
    const AccidentalType oldAccidentalType = m_is.accidentalType();

    switch (p) {
    case Pad::NOTE00:
        m_is.setDuration(DurationType::V_LONG);
        break;
    case Pad::NOTE0:
        m_is.setDuration(DurationType::V_BREVE);
        break;
    case Pad::NOTE1:
        m_is.setDuration(DurationType::V_WHOLE);
        break;
    case Pad::NOTE2:
        m_is.setDuration(DurationType::V_HALF);
        break;
    case Pad::NOTE4:
        m_is.setDuration(DurationType::V_QUARTER);
        break;
    case Pad::NOTE8:
        m_is.setDuration(DurationType::V_EIGHTH);
        break;
    case Pad::NOTE16:
        m_is.setDuration(DurationType::V_16TH);
        break;
    case Pad::NOTE32:
        m_is.setDuration(DurationType::V_32ND);
        break;
    case Pad::NOTE64:
        m_is.setDuration(DurationType::V_64TH);
        break;
    case Pad::NOTE128:
        m_is.setDuration(DurationType::V_128TH);
        break;
    case Pad::NOTE256:
        m_is.setDuration(DurationType::V_256TH);
        break;
    case Pad::NOTE512:
        m_is.setDuration(DurationType::V_512TH);
        break;
    case Pad::NOTE1024:
        m_is.setDuration(DurationType::V_1024TH);
        break;
    case Pad::REST:
        if (noteEntryMode()) {
            m_is.setRest(!m_is.rest());
            m_is.setAccidentalType(AccidentalType::NONE);
        } else if (selection().isNone()) {
            m_is.setDuration(DurationType::V_QUARTER);
            m_is.setRest(true);
        } else {
            for (ChordRest* cr : getSelectedChordRests()) {
                if (!cr->isRest()) {
                    setNoteRest(cr->segment(), cr->track(), NoteVal(), cr->durationTypeTicks(), DirectionV::AUTO, false,
                                m_is.articulationIds());
                }
            }
        }
        break;
    case Pad::DOT:
        if ((m_is.duration().dots() == 1) || (m_is.duration() == DurationType::V_1024TH)) {
            m_is.setDots(0);
        } else {
            m_is.setDots(1);
        }
        break;
    case Pad::DOT2:
        if ((m_is.duration().dots() == 2)
            || (m_is.duration() == DurationType::V_512TH)
            || (m_is.duration() == DurationType::V_1024TH)) {
            m_is.setDots(0);
        } else {
            m_is.setDots(2);
        }
        break;
    case Pad::DOT3:
        if ((m_is.duration().dots() == 3)
            || (m_is.duration() == DurationType::V_256TH)
            || (m_is.duration() == DurationType::V_512TH)
            || (m_is.duration() == DurationType::V_1024TH)) {
            m_is.setDots(0);
        } else {
            m_is.setDots(3);
        }
        break;
    case Pad::DOT4:
        if ((m_is.duration().dots() == 4)
            || (m_is.duration() == DurationType::V_128TH)
            || (m_is.duration() == DurationType::V_256TH)
            || (m_is.duration() == DurationType::V_512TH)
            || (m_is.duration() == DurationType::V_1024TH)) {
            m_is.setDots(0);
        } else {
            m_is.setDots(4);
        }
        break;
    }
    if (p >= Pad::NOTE00 && p <= Pad::NOTE1024) {
        m_is.setDots(0);
        //
        // if in "note enter" mode, reset
        // rest flag
        //
        if (noteEntryMode() && !toggleForSelectionOnly) {
            if (usingNoteEntryMethod(NoteEntryMethod::BY_DURATION) || usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
                switch (oldDuration.dots()) {
                case 1:
                    padToggle(Pad::DOT);
                    break;
                case 2:
                    padToggle(Pad::DOT2);
                    break;
                case 3:
                    padToggle(Pad::DOT3);
                    break;
                case 4:
                    padToggle(Pad::DOT4);
                    break;
                }

                if (m_is.rest()) {
                    // Enter a rest
                    setNoteRest(m_is.segment(), m_is.track(), NoteVal(), m_is.duration().fraction());
                    m_is.moveToNextInputPos();
                } else {
                    if (usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
                        const EngravingItem* selectedItem = score()->selection().element();
                        if (selectedItem && selectedItem->isNote()) {
                            m_is.setNotes({ toNote(selectedItem)->noteVal() });
                        }
                    }

                    ChordRest* cr = m_is.cr();
                    Chord* chord = nullptr;

                    if (cr && cr->isChord() && cr->durationType() == m_is.duration()) {
                        chord = toChord(cr);
                    }

                    for (const NoteVal& nval : m_is.notes()) {
                        if (chord && chord->findNote(nval.pitch)) {
                            continue;
                        }

                        NoteVal copy(nval);
                        const Note* note = nullptr;

                        if (chord) {
                            note = addPitchToChord(copy, chord);
                        } else {
                            note = addPitch(copy, false /*addFlag*/);
                        }

                        if (note) {
                            chord = note->chord();
                        }
                    }
                }
            } else {
                m_is.setRest(false);
            }
        }
    }

    if (noteEntryMode()) {
        if (!toggleForSelectionOnly || selection().isNone()) {
            return;
        }
    }

    std::vector<ChordRest*> crs;
    std::list<EngravingItem*> elementsToSelect;

    if (selection().isSingle()) {
        EngravingItem* e = selection().element();
        ChordRest* cr = InputState::chordRest(e);

        // do not allow to add a dot on a full measure rest
        if (cr && cr->isRest()) {
            Rest* r = toRest(cr);
            if (r->isFullMeasureRest()) {
                m_is.setDots(0);
            }
        }

        // on measure rest, select the first actual rest
        if (cr && cr->isMMRest()) {
            Measure* m = cr->measure()->mmRestFirst();
            if (m) {
                cr = m->findChordRest(m->tick(), 0);
            }
        }

        if (cr) {
            crs.push_back(cr);
        } else {
            deselect(e);
        }
    } else if (selection().isNone() && p != Pad::REST) {
        TDuration td = m_is.duration();
        m_is.setDuration(td);
        m_is.setAccidentalType(AccidentalType::NONE);
    } else {
        const auto elements = selection().uniqueElements();
        for (EngravingItem* e : elements) {
            if (selection().isList() || e->isRest() || e->isNote()) {
                elementsToSelect.push_back(e);
                deselect(e);
            }
        }
        bool canAdjustLength = true;
        for (EngravingItem* e : elements) {
            ChordRest* cr = InputState::chordRest(e);
            if (!cr) {
                continue;
            }
            if (cr->isMeasureRepeat() || cr->isMMRest()) {
                canAdjustLength = false;
                break;
            }
            crs.push_back(cr);
        }

        if (canAdjustLength) {
            // Change length from last to first chord/rest
            std::sort(crs.begin(), crs.end(), [](const ChordRest* cr1, const ChordRest* cr2) {
                if (cr2->track() == cr1->track()) {
                    return cr2->isBefore(cr1);
                }
                return cr2->track() < cr1->track();
            });
            // Remove duplicates from the list
            crs.erase(std::unique(crs.begin(), crs.end()), crs.end());
        } else {
            crs.clear();
        }
    }

    const int dots = m_is.duration().dots();

    if (toggleForSelectionOnly && dots > 0 && !crs.empty()) {
        bool shouldRemoveDots = true;
        for (const ChordRest* cr : crs) {
            if (dots != cr->dots()) {
                shouldRemoveDots = false;
                break;
            }
        }

        if (shouldRemoveDots) {
            m_is.setDots(0);
        }
    }

    for (ChordRest* cr : crs) {
        if (cr->isChord() && (toChord(cr)->isGrace())) {
            //
            // handle appoggiatura and acciaccatura
            //
            undoChangeChordRestLen(cr, m_is.duration());
        } else {
            changeCRlen(cr, m_is.duration());
        }
    }

    if (!elementsToSelect.empty()) {
        std::vector<EngravingItem*> selectList;
        for (EngravingItem* e : elementsToSelect) {
            if (canReselectItem(e)) {
                selectList.push_back(e);
            }
        }

        select(selectList, SelectType::ADD, 0);
        selection().updateSelectedElements();
    }

    if (toggleForSelectionOnly) {
        m_is.setDuration(oldDuration);
        m_is.setRest(oldRest);
        m_is.setAccidentalType(oldAccidentalType);

        if (noteEntryMode()) {
            if (m_is.lastSegment() == m_is.segment()) {
                m_is.moveToNextInputPos();
            }
        }
    }
}

static void onFocusedItemChanged(EngravingItem* item)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    if (!item || !item->selected()) {
        return;
    }

    if (item->isSpannerSegment()) {
        item = toSpannerSegment(item)->spanner();
        if (!item) {
            return;
        }
    }

    item->initAccessibleIfNeed();

    AccessibleItemPtr accessible = item->accessible();
    if (!accessible) {
        return;
    }

    const Score* score = item->score();
    if (!score) {
        return;
    }

    AccessibleRoot* currAccRoot = accessible->accessibleRoot();
    AccessibleRoot* accRoot = score->rootItem()->accessible()->accessibleRoot();
    AccessibleRoot* dummyAccRoot = score->dummy()->rootItem()->accessible()->accessibleRoot();

    if (accRoot && currAccRoot == accRoot && accRoot->registered()) {
        accRoot->setFocusedElement(accessible);

        if (AccessibleItemPtr focusedElement = dummyAccRoot->focusedElement().lock()) {
            accRoot->updateStaffInfo(accessible, focusedElement);
        }

        dummyAccRoot->setFocusedElement(nullptr);
    }

    if (dummyAccRoot && currAccRoot == dummyAccRoot && dummyAccRoot->registered()) {
        dummyAccRoot->setFocusedElement(accessible);

        if (AccessibleItemPtr focusedElement = accRoot->focusedElement().lock()) {
            dummyAccRoot->updateStaffInfo(accessible, focusedElement);
        }

        accRoot->setFocusedElement(nullptr);
    }
#else
    UNUSED(item);
#endif
}

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(EngravingItem* el)
{
    addRefresh(el->pageBoundingRect());
    m_selection.remove(el);
    setSelectionChanged(true);
    m_selection.update();
}

//---------------------------------------------------------
//   select
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::select(EngravingItem* item, SelectType type, staff_idx_t staffIdx)
{
    select(std::vector<EngravingItem*> { item }, type, staffIdx);
}

void Score::select(const std::vector<EngravingItem*>& items, SelectType type, staff_idx_t staffIdx)
{
    for (EngravingItem* item : items) {
        doSelect(item, type, staffIdx);
    }

    if (!m_selection.elements().empty()) {
        onFocusedItemChanged(m_selection.elements().back());
    }
}

void Score::doSelect(EngravingItem* e, SelectType type, staff_idx_t staffIdx)
{
    if (MScore::debugMode) {
        LOGD("select element <%s> type %d(state %d) staff %zu",
             e ? e->typeName() : "", int(type), int(selection().state()), e ? e->staffIdx() : -1);
    }

    switch (type) {
    case SelectType::SINGLE:
        selectSingle(e, staffIdx);
        break;
    case SelectType::ADD:
        selectAdd(e);
        break;
    case SelectType::RANGE:
        selectRange(e, staffIdx);
        break;
    case SelectType::REPLACE:
        break;
    }
    setSelectionChanged(true);
}

//---------------------------------------------------------
//   selectSingle
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::selectSingle(EngravingItem* e, staff_idx_t staffIdx)
{
    SelState selState = m_selection.state();
    deselectAll();
    if (e == 0) {
        selState = SelState::NONE;
        setUpdateAll();
    } else {
        if (e->isMeasure()) {
            doSelect(toMeasure(e)->coveringMMRestOrThis(), SelectType::RANGE, staffIdx);
            return;
        }
        addRefresh(e->pageBoundingRect());
        m_selection.add(e);
        m_is.setTrack(e->track());
        selState = SelState::LIST;
        if (e->type() == ElementType::NOTE) {
            e = e->parentItem();
        }
        if (e->isChordRest()) {
            m_is.setLastSegment(m_is.segment());
            m_is.setSegment(toChordRest(e)->segment());
        }
    }
    m_selection.setActiveSegment(0);
    m_selection.setActiveTrack(0);

    m_selection.setState(selState);
}

//---------------------------------------------------------
//   switchToPageMode
//---------------------------------------------------------

void Score::switchToPageMode()
{
    if (layoutMode() != LayoutMode::PAGE) {
        setLayoutMode(LayoutMode::PAGE);
        doLayout();
    }
}

//---------------------------------------------------------
//   selectAdd
//---------------------------------------------------------

void Score::selectAdd(EngravingItem* e)
{
    SelState selState = m_selection.state();

    if (m_selection.isRange()) {
        doSelect(e, SelectType::SINGLE, 0);
        return;
    }

    if (e->isMeasure()) {
        Measure* m = toMeasure(e)->coveringMMRestOrThis();
        Fraction tick  = m->tick();
        if (m_selection.isNone()) {
            m_selection.setRange(m->tick2segment(tick),
                                 m == lastMeasureMM() ? 0 : m->last(),
                                 0,
                                 nstaves());
            setUpdateAll();
            selState = SelState::RANGE;
            m_selection.updateSelectedElements();
        }
    } else if (!muse::contains(m_selection.elements(), e)) {
        addRefresh(e->pageBoundingRect());
        selState = SelState::LIST;
        m_selection.add(e);
    }

    m_selection.setState(selState);
}

//---------------------------------------------------------
//   selectRange
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

static Segment* findElementStartSegment(Score* score, EngravingItem* e)
{
    if (Segment* ancestor = toSegment(e->findAncestor(ElementType::SEGMENT))) {
        if (ancestor->isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
            return ancestor;
        }
    }

    return score->tick2segmentMM(e->tick(), true, Segment::CHORD_REST_OR_TIME_TICK_TYPE);
}

/// Returns `nullptr` when the end segment is the end of the score;
/// returns `def` if no end segment is found.
static Segment* findElementEndSegment(Score* score, EngravingItem* e, Segment* def)
{
    ChordRest* cr = nullptr;

    if (e->isNote()) {
        cr = toChordRest(e->parentItem());
    } else if (e->isChordRest()) {
        cr = toChordRest(e);
    } else if (EngravingItem* c = e->findAncestor(ElementType::CHORD)) {
        cr = toChordRest(c);
    } else if (EngravingItem* r = e->findAncestor(ElementType::REST)) {
        cr = toChordRest(r);
    }

    if (cr) {
        return cr->nextSegmentAfterCR(SegmentType::ChordRest | SegmentType::EndBarLine | SegmentType::Clef);
    }

    if (e->isSpanner() || e->isSpannerSegment()) {
        Spanner* sp = e->isSpanner() ? toSpanner(e) : toSpannerSegment(e)->spanner();
        return score->tick2segmentMM(sp->tick2(), true, Segment::CHORD_REST_OR_TIME_TICK_TYPE);
    }

    if (Segment* ancestor = toSegment(e->findAncestor(ElementType::SEGMENT))) {
        if (ancestor->isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
            // https://github.com/musescore/MuseScore/pull/25821#issuecomment-2617369881
            if (Segment* next = ancestor->nextCR(e->track(), true)) {
                return next;
            }
        }
    }

    if (Segment* seg = score->tick2segmentMM(e->tick(), true, Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
        return seg;
    }

    return def;
}

void Score::selectRange(EngravingItem* e, staff_idx_t staffIdx)
{
    if (m_selection.isSingle()) {
        if (!e->isMeasureBase()
            && !e->isNote()
            && !e->isChordRest()) {
            // Try to select similar in range
            bool success = trySelectSimilarInRange(e);
            if (success) {
                return;
            }
        }

        // Try to extend single selection into range selection
        bool success = tryExtendSingleSelectionToRange(e, staffIdx);
        if (success) {
            return;
        }
    }

    if (e->isMeasure()) {
        Measure* m = toMeasure(e)->coveringMMRestOrThis();
        Segment* startSegment = m->first(SegmentType::ChordRest);
        Segment* endSegment = m == lastMeasureMM() ? nullptr : m->last();
        Fraction tick = m->tick();
        Fraction etick = tick + m->ticks();

        if (m_selection.isRange()) {
            // Extend existing range selection
            m_selection.extendRangeSelection(startSegment, endSegment, staffIdx, tick, etick);
        } else {
            // Create new range selection
            if (!m_selection.isNone()) {
                deselectAll();
            }
            m_selection.setRange(startSegment, endSegment, staffIdx, staffIdx + 1);
        }

        m_selection.updateSelectedElements();
        m_selection.setActiveTrack(staffIdx * VOICES);
        return;
    }

    if (m_selection.isRange()) {
        // Extend existing range selection

        Segment* startSegment = findElementStartSegment(this, e);
        if (startSegment) {
            Segment* endSegment = findElementEndSegment(this, e, m_selection.endSegment());
            staff_idx_t elementStaffIdx = e->staffIdx();
            if (endSegment && elementStaffIdx != muse::nidx) {
                Fraction tick = startSegment->tick();
                Fraction etick = endSegment->tick();

                m_selection.extendRangeSelection(startSegment, endSegment, elementStaffIdx, tick, etick);
                m_selection.updateSelectedElements();

                m_selection.setActiveTrack(e->track());
                return;
            }
        }
    }

    if (e->isNote() || e->isChordRest()) {
        // Create new range selection
        if (!m_selection.isNone()) {
            deselectAll();
        }

        ChordRest* cr = e->isNote() ? toChordRest(e->parentItem()) : toChordRest(e);

        SegmentType st = SegmentType::ChordRest | SegmentType::EndBarLine | SegmentType::Clef;
        m_selection.setRange(cr->segment(), cr->nextSegmentAfterCR(st), cr->staffIdx(), cr->staffIdx() + 1);
        m_selection.updateSelectedElements();

        m_selection.setActiveTrack(cr->track());
        return;
    }

    // Nothing worked; just select the new element
    doSelect(e, SelectType::SINGLE, staffIdx);
}

bool Score::trySelectSimilarInRange(EngravingItem* e)
{
    EngravingItem* selectedElement = m_selection.element();
    if (!selectedElement || e->type() != selectedElement->type()) {
        return false;
    }

    staff_idx_t idx1 = selectedElement->staffIdx();
    staff_idx_t idx2 = e->staffIdx();

    if (idx1 == muse::nidx || idx2 == muse::nidx) {
        return false;
    }

    if (idx2 < idx1) {
        std::swap(idx1, idx2);
    }

    Fraction t1 = selectedElement->tick();
    Fraction t2 = e->tick();
    if (t1 > t2) {
        std::swap(t1, t2);
    }

    Segment* s1 = tick2segmentMM(t1, true, SegmentType::ChordRest);
    Segment* s2 = tick2segmentMM(t2, true, SegmentType::ChordRest);
    if (s2) {
        s2 = s2->next1MM(SegmentType::ChordRest);
    }

    if (!s1) {
        return false;
    }

    m_selection.setRange(s1, s2, idx1, idx2 + 1);
    selectSimilarInRange(e);
    if (selectedElement->track() == e->track()) {
        // limit to this voice only
        const std::vector<EngravingItem*>& list = m_selection.elements();
        for (EngravingItem* el : list) {
            if (el->track() != e->track()) {
                m_selection.remove(el);
            }
        }
    }

    return true;
}

bool Score::tryExtendSingleSelectionToRange(EngravingItem* newElement, staff_idx_t staffIdx)
{
    EngravingItem* selectedElement = m_selection.element();
    if (!selectedElement) {
        return false;
    }

    Segment* startSegment = findElementStartSegment(this, selectedElement);
    if (!startSegment) {
        return false;
    }

    Segment* endSegment = findElementEndSegment(this, selectedElement, startSegment);

    staff_idx_t startStaffIdx = selectedElement->staffIdx();
    if (startStaffIdx == muse::nidx) {
        return false;
    }

    staff_idx_t endStaffIdx = startStaffIdx + 1;

    track_idx_t activeTrack = newElement->track();
    bool activeSegmentIsStart = false;

    if (newElement->isMeasure()) {
        Measure* m = toMeasure(newElement)->coveringMMRestOrThis();
        const Fraction tick = m->tick();

        if (tick < startSegment->tick()) {
            startSegment = m->first(SegmentType::ChordRest);
            activeSegmentIsStart = true;
        }
        if (m == lastMeasureMM()) {
            endSegment = nullptr;
        } else if (endSegment && tick + m->ticks() > endSegment->tick()) {
            endSegment = m->last();
        }

        startStaffIdx = std::min(startStaffIdx, staffIdx);
        endStaffIdx = std::max(endStaffIdx, staffIdx + 1);

        activeTrack = staffIdx * VOICES;
    } else {
        Segment* newStartSegment = findElementStartSegment(this, newElement);
        if (newStartSegment && newStartSegment->tick() < startSegment->tick()) {
            startSegment = newStartSegment;
            activeSegmentIsStart = true;
        }

        Segment* newEndSegment = findElementEndSegment(this, newElement, newStartSegment);
        if (endSegment && newEndSegment->tick() > endSegment->tick()) {
            endSegment = newEndSegment;
        }

        staff_idx_t newStaffIdx = newElement->staffIdx();
        if (newStaffIdx != muse::nidx) {
            startStaffIdx = std::min(startStaffIdx, newStaffIdx);
            endStaffIdx = std::max(endStaffIdx, newStaffIdx + 1);
        }
    }

    m_selection.setRange(startSegment, endSegment, startStaffIdx, endStaffIdx);
    m_selection.updateSelectedElements();

    m_selection.setActiveTrack(activeTrack);
    m_selection.setActiveSegment(activeSegmentIsStart ? startSegment : endSegment);

    return true;
}

//---------------------------------------------------------
//   collectMatch
//---------------------------------------------------------

void Score::collectMatch(void* data, EngravingItem* e)
{
    ElementPattern* p = static_cast<ElementPattern*>(data);

    if (p->type != int(ElementType::INVALID) && p->type != int(e->type())) {
        return;
    }

    if (p->type == int(ElementType::NOTE)) {
        if (p->subtype < 0) {
            if (!(toNote(e)->chord()->isGrace())) {
                return;
            }
        } else if ((toNote(e)->chord()->isGrace()) || (p->subtype != e->subtype())) {
            return;
        }
    } else if (p->subtypeValid && p->subtype != e->subtype()) {
        return;
    }

    if ((p->staffStart != muse::nidx)
        && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx()))) {
        return;
    }

    if (p->voice != muse::nidx && p->voice != e->voice()) {
        return;
    }

    if (p->system) {
        EngravingItem* ee = e;
        do {
            if (ee->type() == ElementType::SYSTEM) {
                if (p->system != ee) {
                    return;
                }
                break;
            }
            ee = ee->parentItem();
        } while (ee);
    }

    if (e->isRest() && p->durationTicks != Fraction(-1, 1)) {
        const Rest* r = toRest(e);
        if (p->durationTicks != r->actualTicks()) {
            return;
        }
    }

    if (p->measure) {
        auto eMeasure = e->findMeasure();
        if (!eMeasure && e->isSpannerSegment()) {
            if (auto s = toSpannerSegment(e)->spanner()) {
                if (auto se = s->startElement()) {
                    if (auto mse = se->findMeasure()) {
                        eMeasure = mse;
                    }
                }
            }
        }
        if (p->measure != eMeasure) {
            return;
        }
    }

    if ((p->beat.isValid()) && (p->beat != e->beat())) {
        return;
    }

    p->el.push_back(e);
}

//---------------------------------------------------------
//   collectNoteMatch
//---------------------------------------------------------

void Score::collectNoteMatch(void* data, EngravingItem* e)
{
    NotePattern* p = static_cast<NotePattern*>(data);
    if (!e->isNote()) {
        return;
    }
    Note* n = toNote(e);
    if (p->type != NoteType::INVALID && p->type != n->noteType()) {
        return;
    }
    if (p->pitch != -1 && p->pitch != n->pitch()) {
        return;
    }
    if (p->string != INVALID_STRING_INDEX && p->string != n->string()) {
        return;
    }
    if (p->tpc != Tpc::TPC_INVALID && p->tpc != n->tpc()) {
        return;
    }
    if (p->notehead != NoteHeadGroup::HEAD_INVALID && p->notehead != n->headGroup()) {
        return;
    }
    if (p->durationType.type() != DurationType::V_INVALID && p->durationType != n->chord()->actualDurationType()) {
        return;
    }
    if (p->durationTicks != Fraction(-1, 1) && p->durationTicks != n->chord()->actualTicks()) {
        return;
    }
    if ((p->staffStart != muse::nidx)
        && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx()))) {
        return;
    }
    if (p->voice != muse::nidx && p->voice != e->voice()) {
        return;
    }
    if (p->system && (p->system != n->chord()->segment()->system())) {
        return;
    }
    if (p->measure && (p->measure != n->findMeasure())) {
        return;
    }
    if ((p->beat.isValid()) && (p->beat != n->beat())) {
        return;
    }
    p->el.push_back(n);
}

//---------------------------------------------------------
//   selectSimilar
//---------------------------------------------------------

void Score::selectSimilar(EngravingItem* e, bool sameStaff)
{
    ElementType type = e->type();
    Score* score = e->score();

    ElementPattern pattern;
    pattern.type = int(type);
    pattern.subtype = 0;
    pattern.subtypeValid = false;
    if (type == ElementType::NOTE) {
        if (toNote(e)->chord()->isGrace()) {
            pattern.subtype = -1;       // hack
        } else {
            pattern.subtype = e->subtype();
        }
    } else if (e->type() == ElementType::HAIRPIN_SEGMENT) {
        pattern.subtype = e->subtype();
        pattern.subtypeValid = true;
    }
    pattern.staffStart = sameStaff ? e->staffIdx() : muse::nidx;
    pattern.staffEnd = sameStaff ? e->staffIdx() + 1 : muse::nidx;
    pattern.voice = muse::nidx;

    score->scanElements(&pattern, collectMatch);

    score->select(0, SelectType::SINGLE, 0);
    score->select(pattern.el, SelectType::ADD, 0);
}

//---------------------------------------------------------
//   selectSimilarInRange
//---------------------------------------------------------

void Score::selectSimilarInRange(EngravingItem* e)
{
    ElementType type = e->type();
    Score* score = e->score();

    ElementPattern pattern;
    pattern.type    = int(type);
    pattern.subtype = 0;
    pattern.subtypeValid = false;
    if (type == ElementType::NOTE) {
        if (toNote(e)->chord()->isGrace()) {
            pattern.subtype = -1;       //hack
        } else {
            pattern.subtype = e->subtype();
        }
        pattern.subtypeValid = true;
    } else if (e->type() == ElementType::HAIRPIN_SEGMENT) {
        pattern.subtype = e->subtype();
        pattern.subtypeValid = true;
    }
    pattern.staffStart = selection().staffStart();
    pattern.staffEnd = selection().staffEnd();
    pattern.voice = muse::nidx;

    score->scanElementsInRange(&pattern, collectMatch);

    score->select(0, SelectType::SINGLE, 0);
    score->select(pattern.el, SelectType::ADD, 0);
}

//---------------------------------------------------------
//   scoreOrder
//---------------------------------------------------------

ScoreOrder Score::scoreOrder() const
{
    ScoreOrder order = m_scoreOrder;
    order.customized = !order.isScoreOrder(this);
    return order;
}

//---------------------------------------------------------
//   setScoreOrder
//---------------------------------------------------------

void Score::setScoreOrder(ScoreOrder order)
{
    m_scoreOrder = order;
}

//---------------------------------------------------------
//   updateBracesAndBarlines
//---------------------------------------------------------

void Score::updateBracesAndBarlines(Part* part, size_t newIndex)
{
    bool noBracesFound = true;
    for (size_t indexInPart = 0; indexInPart < part->nstaves(); ++indexInPart) {
        size_t indexInScore = part->staves()[indexInPart]->idx();
        for (BracketItem* bi : staff(indexInScore)->brackets()) {
            noBracesFound = false;
            if (bi->bracketType() == BracketType::BRACE) {
                if ((indexInPart <= newIndex) && (newIndex <= (bi->bracketSpan()))) {
                    bi->undoChangeProperty(Pid::BRACKET_SPAN, bi->bracketSpan() + 1);
                }
            }
        }
    }

    auto updateBracketSpan = [this, part](size_t modIndex, size_t refIndex) {
        Staff* modStaff = staff(part->staves()[modIndex]->idx());
        Staff* refStaff = staff(part->staves()[refIndex]->idx());
        if (modStaff->getProperty(Pid::STAFF_BARLINE_SPAN) != refStaff->getProperty(Pid::STAFF_BARLINE_SPAN)) {
            modStaff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, refStaff->getProperty(Pid::STAFF_BARLINE_SPAN));
        }
    };

    if (newIndex >= 2) {
        updateBracketSpan(newIndex - 1, newIndex - 2);
    }

    if (part->nstaves() == 2) {
        const InstrumentTemplate* tp = searchTemplate(part->instrumentId());
        if (tp) {
            if (tp->barlineSpan[0] > 0) {
                staff(part->staves()[0]->idx())->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, tp->barlineSpan[0]);
            }
            if (noBracesFound && (tp->bracket[0] != BracketType::NO_BRACKET)) {
                undoAddBracket(part->staves()[0], 0, tp->bracket[0], part->nstaves());
            }
        }
    } else {
        updateBracketSpan(newIndex, newIndex - 1);
    }
}

//---------------------------------------------------------
//   setBracketsAndBarlines
//---------------------------------------------------------

void Score::setBracketsAndBarlines()
{
    scoreOrder().setBracketsAndBarlines(this);
}

//---------------------------------------------------------
//   remapBracketsAndBarlines
///  Reconstructs brackets and barlines based on how they
///  are in the masterScore. Must be called after creating
///  an excerpt or adding new parts to it.
//---------------------------------------------------------

void Score::remapBracketsAndBarlines()
{
    if (isMaster()) {
        return;
    }

    // Remove all brackets
    for (Staff* staff : staves()) {
        for (BracketItem* bracket : staff->brackets()) {
            bracket->setBracketType(BracketType::NO_BRACKET);
        }
    }
    // Remap all brackets from masterScore
    Score* master = masterScore();
    for (staff_idx_t masterStaffIdx = 0; masterStaffIdx < master->nstaves(); ++masterStaffIdx) {
        Staff* masterStaff = master->staff(masterStaffIdx);
        auto brackets = masterStaff->brackets();

        for (size_t bracketIdx = 0; bracketIdx < brackets.size(); ++bracketIdx) {
            BracketItem* bracket = brackets.at(bracketIdx);
            Staff* firstBracketed = nullptr;
            int span = 0;

            // There is no guarantee that bracket->bracketSpan() is correctly bounded,
            // so masterStaffIdx + bracket->bracketSpan() might be > master->nstaves(),
            // in which case masterStaff below will become null.
            size_t endSpannedMasterStaffIdx = std::min(masterStaffIdx + bracket->bracketSpan(), master->nstaves());

            for (staff_idx_t spannedMasterStIdx = masterStaffIdx; spannedMasterStIdx < endSpannedMasterStaffIdx; ++spannedMasterStIdx) {
                masterStaff = master->staff(spannedMasterStIdx);
                IF_ASSERT_FAILED(masterStaff) {
                    continue;
                }

                Staff* linkedStaff = toStaff(masterStaff->findLinkedInScore(this));
                if (!linkedStaff) {
                    continue;
                }

                if (!firstBracketed) {
                    firstBracketed = linkedStaff;
                }

                ++span;
            }

            if (firstBracketed && span > 1) {
                firstBracketed->setBracketType(bracketIdx, bracket->bracketType());
                firstBracketed->setBracketSpan(bracketIdx, span);
            }
        }
    }

    // Remap all barline spans from masterScore
    for (Staff* staff : staves()) {
        Staff* masterStaff = toStaff(staff->findLinkedInScore(masterScore()));
        if (!masterStaff || !masterStaff->barLineSpan()) {
            continue;
        }
        // Look in the masterScore for all the staves spanned by a common barline.
        // If at least one of them is also in this score, then connect it through.
        bool extendBarline = false;
        int span = masterStaff->barLineSpan();
        while (!extendBarline && span > 0 && masterStaff->idx() + 1 < master->nstaves()) {
            masterStaff = masterScore()->staff(masterStaff->idx() + 1);
            span = masterStaff->barLineSpan();
            if (masterStaff->findLinkedInScore(this)) {
                extendBarline = true;
                break;
            }
        }
        if (extendBarline) {
            staff->setBarLineSpan(1);
        }
    }
}

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const RectF& bbox)
{
    select(0, SelectType::SINGLE, 0);
    RectF fr(bbox.normalized());
    for (Page* page : pages()) {
        RectF pr(page->ldata()->bbox());
        RectF frr(fr.translated(-page->pos()));
        if (pr.right() < frr.left()) {
            continue;
        }
        if (pr.left() > frr.right()) {
            break;
        }

        std::vector<EngravingItem*> items = page->items(frr);
        std::vector<EngravingItem*> itemsToSelect;

        for (EngravingItem* item : items) {
            if (frr.contains(item->pageBoundingRect())) {
                if (item->type() != ElementType::MEASURE && item->selectable()) {
                    itemsToSelect.push_back(item);
                }
            }
        }

        select(itemsToSelect, SelectType::ADD, 0);
    }
}

//---------------------------------------------------------
//   lassoSelectEnd
//---------------------------------------------------------

void Score::lassoSelectEnd()
{
    int noteRestCount     = 0;
    Segment* startSegment = 0;
    Segment* endSegment   = 0;
    staff_idx_t startStaff = 0x7fffffff;
    staff_idx_t endStaff = 0;
    const ChordRest* endCR = 0;

    if (m_selection.elements().empty()) {
        m_selection.setState(SelState::NONE);
        setUpdateAll();
        return;
    }
    m_selection.setState(SelState::LIST);

    for (const EngravingItem* e : m_selection.elements()) {
        if (e->type() != ElementType::NOTE && e->type() != ElementType::REST) {
            continue;
        }
        ++noteRestCount;
        if (e->type() == ElementType::NOTE) {
            e = e->parentItem();
        }
        Segment* seg = static_cast<const ChordRest*>(e)->segment();
        if ((startSegment == 0) || (*seg < *startSegment)) {
            startSegment = seg;
        }
        if ((endSegment == 0) || (*seg > *endSegment)) {
            endSegment = seg;
            endCR = static_cast<const ChordRest*>(e);
        }
        staff_idx_t idx = e->staffIdx();
        if (idx < startStaff) {
            startStaff = idx;
        }
        if (idx > endStaff) {
            endStaff = idx;
        }
    }
    if (noteRestCount > 0) {
        endSegment = endCR->nextSegmentAfterCR(SegmentType::ChordRest
                                               | SegmentType::EndBarLine
                                               | SegmentType::Clef);
        m_selection.setRange(startSegment, endSegment, startStaff, endStaff + 1);
        if (!m_selection.isRange()) {
            m_selection.setState(SelState::RANGE);
        }
        m_selection.updateSelectedElements();
    }
    setUpdateAll();
}

//---------------------------------------------------------
//   addLyrics
//---------------------------------------------------------

void Score::addLyrics(const Fraction& tick, staff_idx_t staffIdx, const String& txt)
{
    if (txt.trimmed().isEmpty()) {
        return;
    }
    Measure* measure = tick2measure(tick);
    Segment* seg     = measure->findSegment(SegmentType::ChordRest, tick);
    if (seg == 0) {
        LOGD("no segment found for lyrics<%s> at tick %d",
             muPrintable(txt), tick.ticks());
        return;
    }

    bool lyricsAdded = false;
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        track_idx_t track = staffIdx * VOICES + voice;
        ChordRest* cr = toChordRest(seg->element(track));
        if (cr) {
            Lyrics* l = Factory::createLyrics(cr);
            l->setXmlText(txt);
            l->setTrack(track);
            cr->add(l);
            lyricsAdded = true;
            break;
        }
    }
    if (!lyricsAdded) {
        LOGD("no chord/rest for lyrics<%s> at tick %d, staff %zu",
             muPrintable(txt), tick.ticks(), staffIdx);
    }
}

//---------------------------------------------------------
//   setTempo
//    convenience function to access TempoMap
//---------------------------------------------------------

void Score::setTempo(Segment* segment, BeatsPerSecond tempo)
{
    setTempo(segment->tick(), tempo);
}

void Score::setTempo(const Fraction& tick, BeatsPerSecond tempo)
{
    tempomap()->setTempo(tick.ticks(), tempo);
    setPlaylistDirty();
}

//---------------------------------------------------------
//   removeTempo
//---------------------------------------------------------

void Score::removeTempo(const Fraction& tick)
{
    tempomap()->delTempo(tick.ticks());
    setPlaylistDirty();
}

//---------------------------------------------------------
//   resetTempo
//---------------------------------------------------------

void Score::resetTempo()
{
    resetTempoRange(Fraction(0, 1), Fraction(std::numeric_limits<int>::max(), 1));
}

//---------------------------------------------------------
//   resetTempoRange
//    Reset tempo and timesig maps in the given range.
//    Start tick included, end tick excluded.
//---------------------------------------------------------

void Score::resetTempoRange(const Fraction& tick1, const Fraction& tick2)
{
    const bool zeroInRange = (tick1 <= Fraction(0, 1) && tick2 > Fraction(0, 1));
    tempomap()->clearRange(tick1.ticks(), tick2.ticks());
    if (zeroInRange) {
        tempomap()->setTempo(0, Constants::DEFAULT_TEMPO);
    }
    sigmap()->clearRange(tick1.ticks(), tick2.ticks());
    if (zeroInRange) {
        Measure* m = firstMeasure();
        if (m) {
            sigmap()->add(0, SigEvent(m->ticks(),  m->timesig(), 0));
        }
    }
}

//---------------------------------------------------------
//   setPause
//---------------------------------------------------------

void Score::setPause(const Fraction& tick, double seconds)
{
    tempomap()->setPause(tick.ticks(), seconds);
    setPlaylistDirty();
}

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

BeatsPerSecond Score::tempo(const Fraction& tick) const
{
    return tempomap()->tempo(tick.ticks());
}

BeatsPerSecond Score::multipliedTempo(const Fraction& tick) const
{
    return tempomap()->multipliedTempo(tick.ticks());
}

//---------------------------------------------------------
//   cmdSelectAll
//---------------------------------------------------------

void Score::cmdSelectAll()
{
    if (m_measures.size() == 0) {
        return;
    }
    deselectAll();
    Measure* first = firstMeasureMM();
    if (!first) {
        return;
    }
    Measure* last = lastMeasureMM();
    selectRange(first, 0);
    selectRange(last, nstaves() - 1);
    setUpdateAll();
    update();
}

//---------------------------------------------------------
//   cmdSelectSection
//---------------------------------------------------------

void Score::cmdSelectSection()
{
    Segment* s = m_selection.startSegment();
    if (s == 0) {
        return;
    }
    MeasureBase* sm = s->measure();
    MeasureBase* em = sm;
    while (sm->prev()) {
        if (sm->prev()->sectionBreak()) {
            break;
        }
        sm = sm->prev();
    }
    while (em->next()) {
        if (em->sectionBreak()) {
            break;
        }
        em = em->next();
    }
    while (sm && sm->type() != ElementType::MEASURE) {
        sm = sm->next();
    }
    while (em && em->type() != ElementType::MEASURE) {
        em = em->next();
    }
    if (sm == 0 || em == 0) {
        return;
    }

    m_selection.setRange(toMeasure(sm)->first(), toMeasure(em)->last(), 0, nstaves());
    setUpdateAll();
    update();
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void Score::undo(UndoCommand* cmd, EditData* ed) const
{
    undoStack()->pushAndPerform(cmd, ed);
}

//---------------------------------------------------------
//   scoreList
//    return a list of scores containing the root score
//    and all part scores (if there are any)
//---------------------------------------------------------

std::list<Score*> Score::scoreList()
{
    std::list<Score*> scores;
    MasterScore* root = masterScore();
    scores.push_back(root);
    for (const Excerpt* ex : root->excerpts()) {
        if (ex->excerptScore()) {
            scores.push_back(ex->excerptScore());
        }
    }
    return scores;
}

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void Score::appendPart(const InstrumentTemplate* t)
{
    Part* part = new Part(this);
    part->initFromInstrTemplate(t);
    for (staff_idx_t i = 0; i < t->staffCount; ++i) {
        Staff* staff = Factory::createStaff(part);
        StaffType* stt = staff->staffType(Fraction(0, 1));
        stt->setLines(t->staffLines[i]);
        stt->setSmall(t->smallStaff[i]);
        if (i == 0) {
            staff->setBracketType(0, t->bracket[0]);
            staff->setBracketSpan(0, t->staffCount);
        }
        undoInsertStaff(staff, i);
    }
    part->staves().front()->setBarLineSpan(static_cast<int>(part->nstaves()));
    undoInsertPart(part, m_parts.size());
    setUpTempoMapLater();
    masterScore()->rebuildMidiMapping();
}

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n)
{
    InsertMeasureOptions options;
    options.createEmptyMeasures = false;

    for (int i = 0; i < n; ++i) {
        insertMeasure(ElementType::MEASURE, nullptr, options);
    }
}

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Score::addSpanner(Spanner* s, bool computeStartEnd)
{
    m_spanner.addSpanner(s);
    s->added();
    if (computeStartEnd) {
        s->computeStartElement();
        s->computeEndElement();
    }
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeSpanner(Spanner* s)
{
    m_spanner.removeSpanner(s);
    s->removed();

    EngravingItem* startElement = s->startElement();
    Chord* startChord = startElement && startElement->isChord() ? toChord(startElement) : nullptr;
    if (startChord) {
        startChord->removeStartingSpanner(s);
    }

    EngravingItem* endElement = s->endElement();
    Chord* endChord = endElement && endElement->isChord() ? toChord(endElement) : nullptr;
    if (endChord) {
        endChord->removeEndingSpanner(s);
    }
}

//---------------------------------------------------------
//   isSpannerStartEnd
//    does is spanner start or end at tick position tick
//    for track ?
//---------------------------------------------------------

bool Score::isSpannerStartEnd(const Fraction& tick, track_idx_t track) const
{
    for (auto i : m_spanner.map()) {
        if (i.second->track() != track) {
            continue;
        }
        if (i.second->tick() == tick || i.second->tick2() == tick) {
            return true;
        }
    }
    return false;
}

void Score::insertTime(const Fraction& tick, const Fraction& len)
{
    for (Staff* staff : staves()) {
        staff->insertTime(tick, len);
    }
    for (Part* part : parts()) {
        part->insertTime(tick, len);
    }
}

//---------------------------------------------------------
//   addUnmanagedSpanner
//---------------------------------------------------------

void Score::addUnmanagedSpanner(Spanner* s)
{
    m_unmanagedSpanner.insert(s);
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeUnmanagedSpanner(Spanner* s)
{
    m_unmanagedSpanner.erase(s);
}

//---------------------------------------------------------
//   uniqueStaves
//---------------------------------------------------------

std::list<staff_idx_t> Score::uniqueStaves() const
{
    std::list<staff_idx_t> sl;

    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        Staff* s = staff(staffIdx);
        if (s->links()) {
            bool alreadyInList = false;
            for (staff_idx_t idx : sl) {
                if (s->links()->contains(staff(idx))) {
                    alreadyInList = true;
                    break;
                }
            }
            if (alreadyInList) {
                continue;
            }
        }
        sl.push_back(staffIdx);
    }
    return sl;
}

//---------------------------------------------------------
//   findCR
//    find chord/rest <= tick in track
//---------------------------------------------------------

ChordRest* Score::findCR(Fraction tick, track_idx_t track) const
{
    Measure* m = tick2measureMM(tick);
    if (!m) {
        //LOGD("findCR: no measure for tick %d", tick.ticks());
        return nullptr;
    }
    // attach to first rest all spanner when mmRest
    if (m->isMMRest()) {
        tick = m->tick();
    }
    Segment* s = m->first(SegmentType::ChordRest);
    for (Segment* ns = s;; ns = ns->next(SegmentType::ChordRest)) {
        if (!ns || ns->tick() > tick) {
            break;
        }
        EngravingItem* el = ns->element(track);
        if (el && el->isRest() && toRest(el)->isGap()) {
            continue;
        } else if (el) {
            s = ns;
        }
    }

    if (!s) {
        return nullptr;
    }
    EngravingItem* el = s->element(track);
    if (el && el->isRest() && toRest(el)->isGap()) {
        return nullptr;
    }

    return toChordRest(el);
}

//---------------------------------------------------------
//   findChordRestEndingBeforeTickInStaff
//    find last chord/rest on staff that ends before tick
//---------------------------------------------------------

ChordRest* Score::findChordRestEndingBeforeTickInStaff(const Fraction& tick, staff_idx_t staffIdx) const
{
    return findChordRestEndingBeforeTickInStaffAndVoice(tick, staffIdx, false, 0);
}

//-----------------------------------------------------------------
//   findChordRestEndingBeforeTickInStaffAndVoice
//    find last chord/rest on staff and voice that ends before tick
//-----------------------------------------------------------------
ChordRest* Score::findChordRestEndingBeforeTickInStaffAndVoice(const Fraction& tick, staff_idx_t staffIdx, voice_idx_t voice) const
{
    return findChordRestEndingBeforeTickInStaffAndVoice(tick, staffIdx, true, voice);
}

//-----------------------------------------------------------------
//   findChordRestEndingBeforeTickInStaffAndVoice
//    find last chord/rest on staff and voice that ends before tick
//    allow no specific voice
//-----------------------------------------------------------------
ChordRest* Score::findChordRestEndingBeforeTickInStaffAndVoice(const Fraction& tick, staff_idx_t staffIdx, bool forceVoice,
                                                               voice_idx_t voice) const
{
    Fraction ptick = tick - Fraction::fromTicks(1);
    Measure* m = tick2measureMM(ptick);
    if (!m) {
        LOGD("findCRinStaff: no measure for tick %d", ptick.ticks());
        return 0;
    }
    // attach to first rest all spanner when mmRest
    if (m->isMMRest()) {
        ptick = m->tick();
    }

    Segment* s = m->first(SegmentType::ChordRest);
    track_idx_t strack = forceVoice ? staffIdx * VOICES + voice : staffIdx * VOICES;
    track_idx_t etrack = forceVoice ? strack + 1 : strack + VOICES;
    track_idx_t actualTrack = strack;

    Fraction lastTick = Fraction(-1, 1);
    for (Segment* ns = s;; ns = ns->next(SegmentType::ChordRest)) {
        if (ns == 0 || ns->tick() > ptick) {
            break;
        }
        // found a segment; now find longest cr on this staff that does not overlap tick
        for (track_idx_t t = strack; t < etrack; ++t) {
            ChordRest* cr = toChordRest(ns->element(t));
            if ((cr) && ((!forceVoice) || voice == cr->voice())) {
                Fraction endTick = cr->tick() + cr->actualTicks();
                if (endTick >= lastTick && endTick <= tick) {
                    s = ns;
                    actualTrack = t;
                    lastTick = endTick;
                }
            }
        }
    }
    if (s) {
        return toChordRest(s->element(actualTrack));
    }
    return 0;
}

ChordRest* Score::findChordRestEndingBeforeTickInTrack(const Fraction& tick, track_idx_t trackIdx) const
{
    Measure* measure = tick2measureMM(tick - Fraction::eps());
    if (!measure) {
        LOGD("findCRinStaff: no measure for tick %d", tick.ticks());
        return nullptr;
    }

    for (const Segment* segment = measure->last(); segment; segment = segment->prev()) {
        EngravingItem* item = segment->elementAt(trackIdx);
        if (!segment->isChordRestType() || !item) {
            continue;
        }
        ChordRest* chordRest = toChordRest(item);
        if (segment->tick() + chordRest->actualTicks() <= tick) {
            return chordRest;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   cmdNextPrevSystem
//---------------------------------------------------------

ChordRest* Score::cmdNextPrevSystem(ChordRest* cr, bool next)
{
    IF_ASSERT_FAILED(cr) {
        return nullptr;
    }

    auto newCR = cr;
    auto currentMeasure = cr->measure();
    auto currentSystem = currentMeasure->system() ? currentMeasure->system() : currentMeasure->coveringMMRestOrThis()->system();
    if (!currentSystem) {
        return cr;
    }
    auto destinationMeasure = currentSystem->firstMeasure();
    if (!destinationMeasure) {
        return cr;
    }

    auto firstSegment = destinationMeasure->first(SegmentType::ChordRest);

    // Case: Go to next system
    if (next) {
        if ((destinationMeasure = currentSystem->lastMeasure()->nextMeasure())) {
            // There is a next system present: get it and accommodate for MMRest
            currentSystem = destinationMeasure->system()
                            ? destinationMeasure->system()
                            : destinationMeasure->coveringMMRestOrThis()->system();
            if (!currentSystem) {
                return cr;
            }
            if ((destinationMeasure = currentSystem->firstMeasure())) {
                if ((newCR = destinationMeasure->first()->nextChordRest(trackZeroVoice(cr->track()), false))) {
                    cr = newCR;
                }
            }
        } else if (currentMeasure != lastMeasure()) {
            // There is no next system present: go to last measure of current system
            if ((destinationMeasure = lastMeasure())) {
                if ((newCR = destinationMeasure->first()->nextChordRest(trackZeroVoice(cr->track()), false))) {
                    if (!destinationMeasure->isMMRest()) {
                        cr = newCR;
                    }
                    // Last visual measure is a MMRest: go to very last measure within that MMRest
                    else if ((destinationMeasure = lastMeasureMM())
                             && (newCR = destinationMeasure->first()->nextChordRest(trackZeroVoice(cr->track()), false))) {
                        cr = newCR;
                    }
                }
            }
        }
    }
    // Case: Go to previous system
    else {
        auto currentSegment = cr->segment();
        // Only go to previous system's beginning if user is already at the absolute beginning of current system
        // and not in first measure of entire score
        if ((destinationMeasure != firstMeasure() && destinationMeasure != firstMeasureMM())
            && (currentSegment == firstSegment || (currentMeasure->mmRest() && currentMeasure->mmRest()->isFirstInSystem()))) {
            if (!(destinationMeasure = destinationMeasure->prevMeasureMM())) {
                return cr;
            }
            if (!(currentSystem = destinationMeasure->system()
                                  ? destinationMeasure->system()
                                  : destinationMeasure->coveringMMRestOrThis()->system())) {
                return cr;
            }
            destinationMeasure = currentSystem->firstMeasure();
        }
        if (destinationMeasure) {
            if ((newCR = destinationMeasure->first()->nextChordRest(trackZeroVoice(cr->track()), false))) {
                cr = newCR;
            }
        }
    }
    return cr;
}

//---------------------------------------------------------
//   cmdNextPrevFrame
//   Return next/previous [Vertical/Horizontal/Text] frame
//   to be used as a navigation command
//---------------------------------------------------------

Box* Score::cmdNextPrevFrame(MeasureBase* currentMeasureBase, bool next) const
{
    Box* selectedBox { nullptr };
    while (!selectedBox && (currentMeasureBase = (next ? currentMeasureBase->next() : currentMeasureBase->prev()))) {
        if (currentMeasureBase->isBox()) {
            selectedBox = toBox(currentMeasureBase);
        }
    }
    return selectedBox;
}

//---------------------------------------------------------
//   cmdNextPrevSection
//    Return [Box* or ChordRest*] of next/previous section
//---------------------------------------------------------

EngravingItem* Score::cmdNextPrevSection(EngravingItem* el, bool dir) const
{
    auto currentMeasureBase = el->findMeasureBase();
    auto destination = currentMeasureBase;
    if (currentMeasureBase) {
        // -----------------------
        // Next Section of Score
        // -----------------------
        if (dir) {
            if ((destination = getNextPrevSectionBreak(currentMeasureBase, true))) {
                el = getScoreElementOfMeasureBase(destination->next());
            }
        }
        // -------------------------
        // Previous Section of Score
        // -------------------------
        else {
            auto currentSegment = el->isChordRest() ? toChordRest(el)->segment() : nullptr;
            if ((destination = getNextPrevSectionBreak(currentMeasureBase, false))) {
                if (currentSegment) {
                    if ((el = getScoreElementOfMeasureBase((score()->first() == destination) ? destination : destination->next()))) {
                        if (el->isChordRest() && (toChordRest(el)->segment() == currentSegment)) {
                            if ((destination = getNextPrevSectionBreak(destination, false))) {
                                el = !(destination->sectionBreak()) ? destination : getScoreElementOfMeasureBase(destination->next());
                            }
                        }
                    }
                } else if ((score()->first() != currentMeasureBase) && (el = getScoreElementOfMeasureBase(destination->next()))) {
                    if (el->findMeasureBase() == currentMeasureBase) {
                        if ((destination = getNextPrevSectionBreak(destination, false))) {
                            el = !(destination->sectionBreak()) ? el : getScoreElementOfMeasureBase(destination->next());
                        }
                    }
                }
            }
        }
    }
    return el;
}

//---------------------------------------------------------
//   getNextPrevSectionBreak
//    Condition: MeasureBase* must be valid before call
//    If no previous section break exists selects first
//    MeasureBase within score
//---------------------------------------------------------

MeasureBase* Score::getNextPrevSectionBreak(MeasureBase* mb, bool dir) const
{
    auto destination = mb;
    if (destination) {
        if (dir) {
            // Find next section break
            auto endOfSection { false };
            while (!endOfSection) {
                if ((destination = destination->next())) {
                    endOfSection = destination->sectionBreak();
                } else {
                    break;
                }
            }
        } else {
            // Find previous section break
            auto inCurrentSection { true };
            while (inCurrentSection && destination) {
                if (destination->index()) {
                    // Safety: SegFaults if invoking prev() when index=0
                    //         even when MeasureBase* is valid!
                    destination = destination->prev();
                    inCurrentSection = !(destination->sectionBreak());
                } else {
                    destination = nullptr;
                }
            }
            if (inCurrentSection || !destination) {
                destination = score()->first();
            }
        }
    }
    return destination;
}

//---------------------------------------------------------
//   getScoreElementOfMeasureBase
//    Helper function
//    Get an EngravingItem* as Box or ChordRest depending on
//    MeasureBase
//---------------------------------------------------------

EngravingItem* Score::getScoreElementOfMeasureBase(MeasureBase* mb) const
{
    EngravingItem* el { nullptr };
    ChordRest* cr { nullptr };
    const Measure* currentMeasure { nullptr };
    if (mb) {
        if (mb->isBox()) {
            el = toBox(mb);
        } else if ((currentMeasure = mb->findMeasure())) {
            // Accommodate for MMRest
            if (style().styleB(Sid::createMultiMeasureRests) && currentMeasure->hasMMRest()) {
                currentMeasure = currentMeasure->coveringMMRestOrThis();
            }
            if ((cr = currentMeasure->first()->nextChordRest(0, false))) {
                el = cr;
            }
        }
    }
    return el;
}

//---------------------------------------------------------
//   nmeasure
//---------------------------------------------------------

size_t Score::nmeasures() const
{
    size_t n = 0;
    for (const Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        n++;
    }
    return n;
}

//---------------------------------------------------------
//   firstTrailingMeasure
//---------------------------------------------------------

Measure* Score::firstTrailingMeasure(ChordRest** cr)
{
    Measure* firstMeasure = nullptr;
    auto m = lastMeasure();

    if (!cr) {
        // No active selection: prepare first empty trailing measure of entire score
        while (m && m->isEmpty(muse::nidx)) {
            firstMeasure = m;
            m = m->prevMeasure();
        }
    } else {
        // Active selection: select full measure rest of active staff's empty trailing measure
        ChordRest* tempCR = *cr;
        while (m && (tempCR = m->first()->nextChordRest(trackZeroVoice((*cr)->track()), false))->isFullMeasureRest()) {
            *cr = tempCR;
            firstMeasure = m;
            m = m->prevMeasure();
        }
    }

    return firstMeasure;
}

//---------------------------------------------------------
//   cmdTopStaff
//---------------------------------------------------------

ChordRest* Score::cmdTopStaff(ChordRest* cr)
{
    // Go to top-most staff of current or first measure depending upon active selection
    const auto* destinationMeasure = cr ? cr->measure() : firstMeasure();
    if (destinationMeasure) {
        // Accommodate for MMRest
        if (style().styleB(Sid::createMultiMeasureRests) && destinationMeasure->hasMMRest()) {
            destinationMeasure = destinationMeasure->coveringMMRestOrThis();
        }
        // Get first ChordRest of top staff
        cr = destinationMeasure->first()->nextChordRest(0, false);
    }
    return cr;
}

//---------------------------------------------------------
//   hasLyrics
//---------------------------------------------------------

bool Score::hasLyrics()
{
    if (!firstMeasure()) {
        return false;
    }

    SegmentType st = SegmentType::ChordRest;
    for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
        for (size_t i = 0; i < ntracks(); ++i) {
            ChordRest* cr = toChordRest(seg->element(static_cast<int>(i)));
            if (cr && !cr->lyrics().empty()) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   hasHarmonies
//---------------------------------------------------------

bool Score::hasHarmonies()
{
    if (!firstMeasure()) {
        return false;
    }

    SegmentType st = SegmentType::ChordRest;
    for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
        for (EngravingItem* e : seg->annotations()) {
            if (e->type() == ElementType::HARMONY) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   lyricCount
//---------------------------------------------------------

int Score::lyricCount()
{
    size_t count = 0;
    SegmentType st = SegmentType::ChordRest;
    for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
        for (size_t i = 0; i < ntracks(); ++i) {
            ChordRest* cr = toChordRest(seg->element(static_cast<int>(i)));
            if (cr) {
                count += cr->lyrics().size();
            }
        }
    }
    return int(count);
}

//---------------------------------------------------------
//   harmonyCount
//---------------------------------------------------------

int Score::harmonyCount()
{
    int count = 0;
    SegmentType st = SegmentType::ChordRest;
    for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
        for (EngravingItem* e : seg->annotations()) {
            if (e->type() == ElementType::HARMONY) {
                count++;
            }
        }
    }
    return count;
}

//---------------------------------------------------------
//   extractLyrics
//---------------------------------------------------------

String Score::extractLyrics()
{
    String result;
    masterScore()->setExpandRepeats(true);
    SegmentType st = SegmentType::ChordRest;
    for (size_t track = 0; track < ntracks(); track++) {
        size_t maxLyrics = 1;
        for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->setPlaybackCount(0);
        }
        // follow the repeat segments
        const RepeatList& rlist = repeatList();
        for (const RepeatSegment* rs : rlist) {
            Fraction startTick  = Fraction::fromTicks(rs->tick);
            Fraction endTick    = startTick + Fraction::fromTicks(rs->len());
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                size_t playCount = m->playbackCount();
                for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                    ChordRest* cr = toChordRest(seg->element(track));
                    if (!cr || cr->lyrics().empty()) {
                        continue;
                    }
                    if (cr->lyrics().size() > maxLyrics) {
                        maxLyrics = cr->lyrics().size();
                    }
                    if (playCount > cr->lyrics().size()) {
                        continue;
                    }
                    Lyrics* l = cr->lyrics(static_cast<int>(playCount));
                    if (!l) {
                        continue;
                    }
                    String lyric = l->plainText().trimmed();
                    LyricsSyllabic ls = l->syllabic();
                    if (ls == LyricsSyllabic::SINGLE || ls == LyricsSyllabic::END) {
                        result += lyric + u" ";
                    } else if (ls == LyricsSyllabic::BEGIN || ls == LyricsSyllabic::MIDDLE) {
                        result += lyric;
                    }
                }
                m->setPlaybackCount(m->playbackCount() + 1);
                if (m->endTick() >= endTick) {
                    break;
                }
            }
        }
        // consider remaining lyrics
        for (size_t lyricsNumber = 0; lyricsNumber < maxLyrics; lyricsNumber++) {
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                size_t playCount = m->playbackCount();
                if (lyricsNumber < playCount) {
                    continue;
                }
                for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                    ChordRest* cr = toChordRest(seg->element(track));
                    if (!cr || cr->lyrics().empty()) {
                        continue;
                    }
                    if (cr->lyrics().size() > maxLyrics) {
                        maxLyrics = cr->lyrics().size();
                    }
                    if (lyricsNumber > cr->lyrics().size()) {
                        continue;
                    }
                    Lyrics* l = cr->lyrics(static_cast<int>(lyricsNumber));
                    if (!l) {
                        continue;
                    }
                    String lyric = l->plainText().trimmed();
                    LyricsSyllabic ls = l->syllabic();
                    if (ls == LyricsSyllabic::SINGLE || ls == LyricsSyllabic::END) {
                        result += lyric + u" ";
                    } else if (ls == LyricsSyllabic::BEGIN || ls == LyricsSyllabic::MIDDLE) {
                        result += lyric;
                    }
                }
            }
        }
    }
    return result.trimmed();
}

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

int Score::keysig()
{
    Key result = Key::C;
    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        Staff* st = staff(staffIdx);
        constexpr Fraction t(0, 1);
        Key key = st->concertKey(t);
        if (st->staffType(t)->group() == StaffGroup::PERCUSSION || st->keySigEvent(t).isAtonal()) {         // ignore percussion and custom / atonal key
            continue;
        }
        result = key;
        //TODO keySigs and pitched to unpitched instr changes (Igor Korsukov 2021-02-05) ??? why here?
        break;
    }
    return int(result);
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

int Score::duration()
{
    const RepeatList& rl = masterScore()->repeatList(true);
    if (rl.empty()) {
        return 0;
    }
    const RepeatSegment* rs = rl.back();
    return lrint(utick2utime(rs->utick + rs->len()));
}

//---------------------------------------------------------
//   durationWithoutRepeats
//---------------------------------------------------------

int Score::durationWithoutRepeats()
{
    const RepeatList& rl = masterScore()->repeatList(false);
    if (rl.empty()) {
        return 0;
    }
    const RepeatSegment* rs = rl.back();
    return lrint(utick2utime(rs->utick + rs->len()));
}

//---------------------------------------------------------
//   createRehearsalMarkText
//---------------------------------------------------------

String Score::createRehearsalMarkText(RehearsalMark* current) const
{
    Fraction tick = current->segment()->tick();
    RehearsalMark* before = 0;
    RehearsalMark* after = 0;
    for (Segment* s = firstSegment(SegmentType::All); s; s = s->next1()) {
        for (EngravingItem* e : s->annotations()) {
            if (e && e->type() == ElementType::REHEARSAL_MARK) {
                if (s->tick() < tick) {
                    before = toRehearsalMark(e);
                } else if (s->tick() > tick) {
                    after = toRehearsalMark(e);
                    break;
                }
            }
        }
        if (after) {
            break;
        }
    }
    String s = u"A";
    String s1 = before ? before->xmlText() : u"";
    String s2 = after ? after->xmlText() : u"";
    if (s1.isEmpty()) {
        return s;
    }
    s = nextRehearsalMarkText(before, current);       // try to sequence
    if (s == current->xmlText()) {
        // no sequence detected (or current happens to be correct)
        return s;
    } else if (s == s2) {
        // next in sequence already present
        if (s1.at(0).isLetter()) {
            if (s1.size() == 2) {
                s = String(s1.at(0)) + Char::fromAscii(s1.at(1).toAscii() + 1);          // BB, BC, CC
            } else {
                s = s1 + u'1';                              // B, B1, C
            }
        } else {
            s = s1 + u'A';                                  // 2, 2A, 3
        }
    }
    return s;
}

//---------------------------------------------------------
//   nextRehearsalMarkText
//    finds next rehearsal in sequence established by previous
//     Alphabetic sequences:
//      A, B, ..., Y, Z, AA, BB, ..., YY, ZZ
//      a, b, ..., y, z, aa, bb, ..., yy, zz
//     Numeric sequences:
//      1, 2, 3, ...
//      If number of previous rehearsal mark matches measure number, assume use of measure numbers throughout
//---------------------------------------------------------

String Score::nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const
{
    String previousText = previous->xmlText();
    String fallback = current ? current->xmlText() : previousText + u"'";

    if (previousText.size() == 1 && previousText.at(0).isLetter()) {
        // single letter sequence
        if (previousText == "Z") {
            return u"AA";
        } else if (previousText == "z") {
            return u"aa";
        } else {
            return String(Char::fromAscii(previousText.at(0).toAscii() + 1));
        }
    } else if (previousText.size() == 2 && previousText.at(0).isLetter() && previousText.at(1).isLetter()) {
        // double letter sequence
        if (previousText.at(0) == previousText.at(1)) {
            // repeated letter sequence
            if (previousText.toUpper() != "ZZ") {
                String c = Char::fromAscii(previousText.at(0).toAscii() + 1);
                return c + c;
            } else {
                return fallback;
            }
        } else {
            return fallback;
        }
    } else {
        // try to interpret as number
        bool ok;
        int n = previousText.toInt(&ok);
        if (!ok) {
            return fallback;
        } else if (current && n == previous->segment()->measure()->no() + 1) {
            // use measure number
            n = current->segment()->measure()->no() + 1;
            return String::number(n);
        } else {
            // use number sequence
            n = previousText.toInt() + 1;
            return String::number(n);
        }
    }
}

//---------------------------------------------------------
//   changeSelectedNotesVoice
//    moves selected notes into specified voice if possible
//---------------------------------------------------------

void Score::changeSelectedElementsVoice(voice_idx_t voice)
{
    auto isOperationValid = [](Score* score, track_idx_t origTrack, track_idx_t dstTrack) {
        Excerpt* excerpt = score->excerpt();
        if (!excerpt) {
            return true;
        }

        const track_idx_t dstTrackMapped = muse::key(excerpt->tracksMapping(), dstTrack, muse::nidx);
        if (dstTrackMapped == muse::nidx) {
            return false;
        }

        const track_idx_t origTrackMapped = muse::key(excerpt->tracksMapping(), origTrack, muse::nidx);
        if (origTrackMapped == dstTrack) {
            return false;
        }

        return true;
    };

    std::vector<EngravingItem*> newElements;
    std::vector<EngravingItem*> oel = selection().elements();       // make copy
    for (EngravingItem* e : oel) {
        if (e->isNote()) {
            Note* note   = toNote(e);
            Chord* chord = note->chord();

            // move grace notes with main chord only
            if (chord->isGrace() || chord->isTrillCueNote()) {
                continue;
            }

            if (chord->voice() == voice) {
                continue;
            }

            Score* score = this;
            if (excerpt() && !chord->staff()->isVoiceVisible(voice)) {
                // We are on a linked stave with the desired voice not visible
                // Get note in main score to continue
                note = toNote(note->findLinkedInScore(m_masterScore));
                chord = note->chord();
                score = m_masterScore;
            }
            Segment* s       = chord->segment();
            Measure* m       = s->measure();
            size_t notes     = chord->notes().size();
            track_idx_t dstTrack = chord->staffIdx() * VOICES + voice;
            ChordRest* dstCR = toChordRest(s->element(dstTrack));
            Chord* dstChord  = nullptr;

            if (!isOperationValid(score, chord->track(), dstTrack)) {
                continue;
            }

            // set up destination chord

            if (dstCR && dstCR->type() == ElementType::CHORD && dstCR->globalTicks() == chord->globalTicks()) {
                // existing chord in destination with correct duration;
                //   can simply move note in
                dstChord = toChord(dstCR);
            } else if (dstCR && dstCR->type() == ElementType::REST
                       && dstCR->globalTicks() == chord->globalTicks()) {
                // existing rest in destination with correct duration;
                //   replace with chord, then move note in
                //   this case allows for tuplets, unlike the more general case below
                dstChord = Factory::createChord(s);
                dstChord->setTrack(dstTrack);
                dstChord->setDurationType(chord->durationType());
                dstChord->setTicks(chord->ticks());
                dstChord->setTuplet(dstCR->tuplet());
                dstChord->setParent(s);
                score->undoRemoveElement(dstCR);
            } else if (!chord->tuplet()) {
                // rests or gap in destination
                //   insert new chord if the rests / gap are long enough
                //   then move note in
                ChordRest* pcr = nullptr;
                ChordRest* ncr = nullptr;
                for (Segment* s2 = m->first(SegmentType::ChordRest); s2; s2 = s2->next()) {
                    if (s2->segmentType() != SegmentType::ChordRest) {
                        continue;
                    }
                    ChordRest* cr2 = toChordRest(s2->element(dstTrack));
                    if (!cr2 || cr2->type() == ElementType::REST) {
                        continue;
                    }
                    if (s2->tick() < s->tick()) {
                        pcr = cr2;
                        continue;
                    } else if (s2->tick() >= s->tick()) {
                        ncr = cr2;
                        break;
                    }
                }
                Fraction gapStart = pcr ? pcr->tick() + pcr->actualTicks() : m->tick();
                Fraction gapEnd   = ncr ? ncr->tick() : m->tick() + m->ticks();
                if (gapStart <= s->tick() && gapEnd >= s->tick() + chord->actualTicks()) {
                    // big enough gap found
                    dstChord = Factory::createChord(s);
                    dstChord->setTrack(dstTrack);
                    dstChord->setDurationType(chord->durationType());
                    dstChord->setTicks(chord->ticks());
                    dstChord->setParent(s);
                    // makeGapVoice will not back-fill an empty voice
                    if (voice && !dstCR) {
                        score->expandVoice(s, /*m->first(SegmentType::ChordRest,*/ dstTrack);
                    }
                    score->makeGapVoice(s, dstTrack, chord->actualTicks(), s->tick());
                }
            }

            if (!dstChord) {
                continue;
            }

            // move note to destination chord

            // create & add new note
            Note* newNote = Factory::copyNote(*note);
            newNote->setSelected(false);
            newNote->setParent(dstChord);
            score->undoAddElement(newNote);
            newElements.push_back(newNote);
            // add new chord if one was created
            if (dstChord != dstCR) {
                score->undoAddCR(dstChord, m, s->tick());
            }
            for (EngravingObject* linked : note->linkList()) {
                Note* linkedNote = toNote(linked);
                Note* linkedNewNote = linked == note ? newNote : toNote(newNote->findLinkedInStaff(linkedNote->staff()));
                // reconnect the tie to this note, if any
                Tie* tie = linkedNote->tieBack();
                if (tie) {
                    Note* startNote = tie->isPartialTie() && !toPartialTie(tie)->isOutgoing() ? nullptr : tie->startNote();
                    score->undoChangeSpannerElements(tie, startNote, linkedNewNote);
                }
                // reconnect the tie from this note, if any
                tie = linkedNote->tieFor();
                if (tie) {
                    score->undoChangeSpannerElements(tie, linkedNewNote, tie->endNote());
                }

                // Reconnect note anchored spanners
                for (EngravingItem* item : linkedNote->spannerBack()) {
                    if (!item || !item->isSpanner()) {
                        continue;
                    }
                    Spanner* spanner = toSpanner(item);
                    score->undoChangeSpannerElements(spanner, spanner->startElement(), linkedNewNote);
                }

                for (EngravingItem* item : linkedNote->spannerFor()) {
                    if (!item || !item->isSpanner()) {
                        continue;
                    }
                    Spanner* spanner = toSpanner(item);
                    score->undoChangeSpannerElements(spanner, linkedNewNote, spanner->endElement());
                }
            }

            // remove original note
            if (notes > 1) {
                score->undoRemoveElement(note);
            } else if (notes == 1) {
                // take care of slurs
                int currentTick = chord->tick().ticks();
                for (auto it : score->spannerMap().findOverlapping(currentTick, currentTick + 1)) {
                    Spanner* spanner = it.value;
                    if (!spanner->isSlur()) {
                        continue;
                    }
                    Slur* slur = toSlur(spanner);
                    if (slur->startElement() == chord && slur->endElement() == chord) {
                        score->undoChangeSpannerElements(slur, dstChord, dstChord);
                        slur->undoChangeProperty(Pid::VOICE, voice);
                    } else if (slur->startElement() == chord) {
                        score->undoChangeSpannerElements(slur, dstChord, slur->endElement());
                    } else if (slur->endElement() == chord) {
                        score->undoChangeSpannerElements(slur, slur->startElement(), dstChord);
                    }
                }
                // create rest to leave behind
                Rest* r = Factory::createRest(s);
                r->setTrack(chord->track());
                r->setDurationType(chord->durationType());
                r->setTicks(chord->ticks());
                r->setTuplet(chord->tuplet());
                r->setParent(s);
                // if there were grace notes, move them
                while (!chord->graceNotes().empty()) {
                    Chord* gc = chord->graceNotes().front();
                    Chord* ngc = Factory::copyChord(*gc);
                    score->undoRemoveElement(gc);
                    ngc->setParent(dstChord);
                    ngc->setTrack(dstChord->track());
                    score->undoAddElement(ngc);
                }
                // remove chord, replace with rest
                score->undoRemoveElement(chord);
                score->undoAddCR(r, m, s->tick());
            }

            // Move lyrics
            for (Lyrics* lyric : chord->lyrics()) {
                if (!lyric || dstChord->lyrics(lyric->no(), lyric->placement())) {
                    continue;
                }

                Lyrics* newLyric = Factory::copyLyrics(*lyric);
                newLyric->setParent(dstChord);
                newLyric->setSelected(false);
                newLyric->setTrack(dstTrack);
                score->undoAddElement(newLyric);
                newElements.push_back(newLyric);

                score->undoRemoveElement(lyric);
            }
        } else if (e->hasVoiceAssignmentProperties()) {
            if (e->isSpannerSegment()) {
                e = toSpannerSegment(e)->spanner();
            }

            track_idx_t dstTrack = e->staffIdx() * VOICES + voice;
            if (!isOperationValid(this, e->track(), dstTrack)) {
                continue;
            }

            e->undoChangeProperty(Pid::VOICE, voice);
            e->undoChangeProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::CURRENT_VOICE_ONLY);
            newElements.push_back(e);
        }
    }

    if (!newElements.empty()) {
        selection().clear();
        select(newElements, SelectType::ADD, muse::nidx);
    }

    setLayoutAll();
}

void Score::changeSelectedElementsVoiceAssignment(VoiceAssignment voiceAssignment)
{
    std::vector<EngravingItem*> newElements;

    for (EngravingItem* e : selection().elements()) {
        if (e->hasVoiceAssignmentProperties()) {
            e->undoChangeProperty(Pid::VOICE_ASSIGNMENT, voiceAssignment);
            newElements.push_back(e);
        }
    }

    if (!newElements.empty()) {
        selection().clear();
        select(newElements, SelectType::ADD, muse::nidx);
    }
}

std::set<staff_idx_t> Score::staffIdxSetFromRange(const track_idx_t trackFrom, const track_idx_t trackTo, StaffAccepted staffAccepted) const
{
    std::set<staff_idx_t> result;

    for (const Part* part : m_score->parts()) {
        if (trackTo < part->startTrack() || trackFrom >= part->endTrack()) {
            continue;
        }

        std::set<staff_idx_t> staffIdxList = part->staveIdxList();
        if (!staffAccepted) {
            result.insert(staffIdxList.cbegin(), staffIdxList.cend());
            continue;
        }

        for (staff_idx_t idx : staffIdxList) {
            const Staff* staff = m_score->staff(idx);
            if (!staff) {
                continue;
            }

            if (staffAccepted(*staff)) {
                result.insert(idx);
            }
        }
    }

    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Score::getProperty(Pid /*id*/) const
{
    LOGD("Score::getProperty: unhandled id");
    return PropertyValue();
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Score::setProperty(Pid /*id*/, const PropertyValue& /*v*/)
{
    LOGD("Score::setProperty: unhandled id");
    setLayoutAll();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Score::propertyDefault(Pid /*id*/) const
{
    return PropertyValue();
}

void Score::resetStyleValue(Sid styleToReset)
{
    const MStyle& defStyle = DefaultStyle::defaultStyle();
    undoChangeStyleVal(styleToReset, defStyle.value(styleToReset));
}

void Score::resetStyleValues(const StyleIdSet& styleIdSet)
{
    if (styleIdSet.empty()) {
        return;
    }

    std::unordered_map<Sid, PropertyValue> values;
    const MStyle& defStyle = DefaultStyle::defaultStyle();

    for (Sid sid : styleIdSet) {
        values.emplace(sid, defStyle.value(sid));
    }

    undoChangeStyleValues(std::move(values));
}

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Score::setStyle(const MStyle& s, const bool overlap)
{
    if (!overlap) {
        style() = s;
        return;
    }

    MStyle styleCopy = s;

    for (int i = static_cast<int>(Sid::NOSTYLE) + 1; i < static_cast<int>(Sid::STYLES); i++) {
        Sid sid = static_cast<Sid>(i);

        if (!styleCopy.isDefault(sid)) {
            style().set(sid, styleCopy.value(sid));
        }
    }
}

//---------------------------------------------------------
//   getTextStyleUserName
//---------------------------------------------------------

TranslatableString Score::getTextStyleUserName(TextStyleType tid) const
{
    if (int(tid) >= int(TextStyleType::USER1) && int(tid) <= int(TextStyleType::USER12)) {
        int idx = int(tid) - int(TextStyleType::USER1);
        Sid sid[] = { Sid::user1Name, Sid::user2Name, Sid::user3Name, Sid::user4Name, Sid::user5Name, Sid::user6Name,
                      Sid::user7Name, Sid::user8Name, Sid::user9Name, Sid::user10Name, Sid::user11Name, Sid::user12Name };
        String userName = style().styleSt(sid[idx]);
        if (!userName.empty()) {
            return TranslatableString::untranslatable(userName);
        }
    }
    return TConv::userName(tid);
}

String Score::name() const
{
    return m_excerpt ? m_excerpt->name() : String();
}

//---------------------------------------------------------
//   addRefresh
//---------------------------------------------------------

void Score::addRefresh(const RectF& r)
{
    m_updateState.refresh.unite(r);
    cmdState().setUpdateMode(UpdateMode::Update);
}

//---------------------------------------------------------
//   staffIdx
//
//  Return index for the staff in the score.
//---------------------------------------------------------

staff_idx_t Score::staffIdx(const Staff* staff) const
{
    staff_idx_t idx = 0;
    for (Staff* s : m_staves) {
        if (s == staff) {
            break;
        }
        ++idx;
    }
    return idx;
}

//---------------------------------------------------------
//   staffIdx
//
///  Return index for the first staff of \a part.
//---------------------------------------------------------

staff_idx_t Score::staffIdx(const Part* part) const
{
    staff_idx_t idx = 0;
    for (Part* p : m_parts) {
        if (p == part) {
            break;
        }
        idx += p->nstaves();
    }
    return idx;
}

Staff* Score::staffById(const ID& staffId) const
{
    for (Staff* staff : m_staves) {
        if (staff->id() == staffId) {
            return staff;
        }
    }

    return nullptr;
}

Part* Score::partById(const ID& partId) const
{
    for (Part* part : m_parts) {
        if (part->id() == partId) {
            return part;
        }
    }

    return nullptr;
}

void Score::clearSystemObjectStaves()
{
    m_systemObjectStaves.clear();
}

void Score::addSystemObjectStaff(Staff* staff)
{
    m_systemObjectStaves.push_back(staff);
}

void Score::removeSystemObjectStaff(Staff* staff)
{
    muse::remove(m_systemObjectStaves, staff);
}

bool Score::isSystemObjectStaff(Staff* staff) const
{
    return muse::contains(m_systemObjectStaves, staff);
}

const std::vector<Part*>& Score::parts() const
{
    return m_parts;
}

size_t Score::visiblePartCount() const
{
    int count = 0;
    for (const Part* part : m_parts) {
        if (part->show()) {
            ++count;
        }
    }
    return count;
}

size_t Score::visibleStavesCount() const
{
    int count = 0;
    for (const Staff* staff : m_staves) {
        if (staff->show()) {
            ++count;
        }
    }
    return count;
}

ShadowNote* Score::shadowNote() const
{
    return m_shadowNote;
}

void Score::rebuildBspTree()
{
    for (Page* page : pages()) {
        page->invalidateBspTree();
    }
}

//---------------------------------------------------------
//   scanElements
//    scan all elements
//---------------------------------------------------------

void Score::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        mb->scanElements(data, func, all);
        if (mb->type() == ElementType::MEASURE) {
            Measure* m = toMeasure(mb);
            Measure* mmr = m->mmRest();
            if (mmr) {
                mmr->scanElements(data, func, all);
            }
        }
    }
    for (Page* page : pages()) {
        for (System* s :page->systems()) {
            s->scanElements(data, func, all);
        }
        func(data, page);
    }
}

//---------------------------------------------------------
//   connectTies
///   Rebuild tie connections.
//---------------------------------------------------------

void Score::connectTies(bool silent)
{
    size_t tracks = nstaves() * VOICES;
    Measure* m = firstMeasure();
    if (!m) {
        return;
    }

    auto connectTiesForChord = [silent](Chord* c, Segment* s, track_idx_t track) -> void {
        for (Note* n : c->notes()) {
            if (n->laissezVib()) {
                continue;
            }
            // connect a tie without end note
            Tie* tie = n->tieFor();
            if (tie) {
                tie->updatePossibleJumpPoints();
            }
            if (tie && !tie->isPartialTie() && !tie->endNote()) {
                Note* nnote;
                nnote = searchTieNote(n);
                if (nnote == 0) {
                    if (!silent) {
                        LOGD("next note at %d track %zu for tie not found", s->tick().ticks(), track);
                        delete tie;
                        n->setTieFor(0);
                    }
                } else {
                    tie->setEndNote(nnote);
                    nnote->setTieBack(tie);
                }
            }
            // connect a glissando without initial note (old glissando format)
            for (Spanner* spanner : n->spannerBack()) {
                if (spanner->isGlissando() && !spanner->startElement()) {
                    Note* initialNote = Glissando::guessInitialNote(n->chord());
                    n->removeSpannerBack(spanner);
                    if (initialNote) {
                        spanner->setStartElement(initialNote);
                        spanner->setEndElement(n);
                        spanner->setTick(initialNote->chord()->tick());
                        spanner->setTick2(n->chord()->tick());
                        spanner->setTrack(n->track());
                        spanner->setTrack2(n->track());
                        spanner->setParent(initialNote);
                        initialNote->add(spanner);
                    } else {
                        delete spanner;
                    }
                }
            }
            // spanner with no end element can happen during copy/paste
            for (Spanner* spanner : n->spannerFor()) {
                if (spanner->endElement() == nullptr) {
                    n->removeSpannerFor(spanner);
                    delete spanner;
                }
            }
        }
    };

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = m->first(st); s; s = s->next1(st)) {
        for (track_idx_t track = 0; track < tracks; ++track) {
            EngravingItem* e = s->element(track);
            if (e == 0 || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            connectTiesForChord(c, s, track);
            for (Chord* gc : c->graceNotes()) {
                connectTiesForChord(gc, s, track);

                for (Note* n : gc->notes()) {
                    // spanner with no end element apparently happens when reading some 206 files
                    // (and possibly in other situations too)
                    for (Spanner* spanner : n->spannerFor()) {
                        if (spanner->endElement() == nullptr) {
                            n->removeSpannerFor(spanner);
                            delete spanner;
                        }
                    }
                }
            }
        }
    }
}

bool Score::autoLayoutEnabled() const
{
    return isOpen();
}

//---------------------------------------------------------
//   doLayout
//    do a complete (re-) layout
//---------------------------------------------------------

void Score::doLayout()
{
    TRACEFUNC;

    doLayoutRange(Fraction(0, 1), Fraction(-1, 1));
}

void Score::doLayoutRange(const Fraction& st, const Fraction& et)
{
    TRACEFUNC;

    Fraction start = st;
    Fraction end = et;

    auto spanners = score()->spannerMap().findOverlapping(st.ticks(), et.ticks());
    for (auto interval : spanners) {
        Spanner* spanner = interval.value;
        if (!spanner->staff()->visible()) {
            continue;
        }
        start = std::min(st, spanner->tick());
        end = std::max(et, spanner->tick2());
    }

    m_engravingFont = engravingFonts()->fontByName(style().value(Sid::musicalSymbolFont).value<String>().toStdString());
    m_layoutOptions.noteHeadWidth = m_engravingFont->width(SymId::noteheadBlack, style().spatium() / SPATIUM20);

    if (this->cmdState().layoutFlags & LayoutFlag::REBUILD_MIDI_MAPPING) {
        if (this->isMaster()) {
            this->masterScore()->rebuildMidiMapping();
        }
    }

    renderer()->layoutScore(this, start, end);

    if (m_resetAutoplace) {
        m_resetAutoplace = false;
        resetAutoplace();
    }

    if (m_resetCrossBeams) {
        m_resetCrossBeams = false;
        resetCrossBeams();
    }
}

void Score::createPaddingTable()
{
    m_paddingTable.createTable(style());
}

//--------------------------------------------------------
// setAutoSpatium()
// Reduces the spatium size as necessary to accommodate all
// staves in the page. Caution: the spatium is expressed in
// DPI, page dimensions in inches, staff sizes in mm.
//--------------------------------------------------------
void Score::autoUpdateSpatium()
{
    static constexpr double breathingSpace = 2.5; // allow breathing space between staves
    static constexpr double minStaffHeight = 2.0; // never make staff smaller than 2.0 mm
    static constexpr double maxStaffHeight = 7.0; // never make staff bigger than 7.0 mm (default value)

    double availableHeight = (style().styleD(Sid::pageHeight)
                              - style().styleD(Sid::pageOddTopMargin)
                              - style().styleD(Sid::pageOddBottomMargin)) * DPI;                    // convert from inches to DPI
    double titleHeight = (!measures()->empty() && measures()->first()->isVBox()) ? measures()->first()->height() : 0.0;
    availableHeight -= titleHeight;
    double totalNeededSpaces = 4 * m_staves.size() * breathingSpace;
    double targetSpatium = availableHeight / totalNeededSpaces;

    double resultingStaffHeight = 4 * targetSpatium / DPI * INCH; // conversion from DPI to mm
    resultingStaffHeight = round(resultingStaffHeight * 10) / 10; // round to nearest 0.1 mm
    if (resultingStaffHeight > maxStaffHeight) {
        return;
    }
    if (resultingStaffHeight < minStaffHeight) {
        resultingStaffHeight = minStaffHeight;
    }

    targetSpatium = (resultingStaffHeight / 4) * DPI / INCH;

    style().setSpatium(targetSpatium);
    createPaddingTable();
}

void Score::addSystemLock(const SystemLock* lock)
{
    m_systemLocks.add(lock);

    lock->startMB()->triggerLayout();
    lock->endMB()->triggerLayout();
}

void Score::removeSystemLock(const SystemLock* lock)
{
    m_systemLocks.remove(lock);

    lock->startMB()->triggerLayout();
    lock->endMB()->triggerLayout();
}

//---------------------------------------------------------
//   updateSwing
//---------------------------------------------------------

void Score::updateSwing()
{
    for (Staff* s : m_staves) {
        s->clearSwingList();
    }
    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }
    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* e : s->annotations()) {
            if (!e->isStaffTextBase()) {
                continue;
            }
            const StaffTextBase* st = toStaffTextBase(e);
            if (st->xmlText().isEmpty()) {
                continue;
            }
            Staff* staff = st->staff();
            if (!st->swing()) {
                continue;
            }
            SwingParameters sp;
            sp.swingRatio = st->swingParameters().swingRatio;
            sp.swingUnit = st->swingParameters().swingUnit;
            if (st->systemFlag()) {
                for (Staff* sta : m_staves) {
                    sta->insertIntoSwingList(s->tick(), sp);
                }
            } else {
                staff->insertIntoSwingList(s->tick(), sp);
            }
        }
    }
}

//---------------------------------------------------------
//   updateCapo
//---------------------------------------------------------

void Score::updateCapo()
{
    for (Staff* s : m_staves) {
        s->clearCapoParams();
    }

    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }

    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        Fraction segmentTick = s->tick();

        for (EngravingItem* e : s->annotations()) {
            if (e->isHarmony()) {
                toHarmony(e)->realizedHarmony().setDirty(true);
            }

            if (!e->isCapo()) {
                continue;
            }

            for (Staff* staff : e->staff()->staffList()) {
                staff->insertCapoParams(segmentTick, toCapo(e)->params());
            }
        }
    }
}

//---------------------------------------------------------
//   updateChannel
//---------------------------------------------------------

void Score::updateChannel()
{
    for (Staff* s : staves()) {
        for (voice_idx_t i = 0; i < VOICES; ++i) {
            s->clearChannelList(i);
        }
    }
    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }
    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* e : s->annotations()) {
            if (e->isInstrumentChange()) {
                for (Staff* staff : e->part()->staves()) {
                    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                        staff->insertIntoChannelList(voice, s->tick(), 0);
                    }
                }
                continue;
            }
            if (!e->isStaffTextBase()) {
                continue;
            }
            const StaffTextBase* st = toStaffTextBase(e);
            for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                String an(st->channelName(voice));
                if (an.isEmpty()) {
                    continue;
                }
                Staff* staff = Score::staff(st->staffIdx());
                int a = staff->part()->instrument(s->tick())->channelIdx(an);
                if (a != -1) {
                    staff->insertIntoChannelList(voice, s->tick(), a);
                }
            }
        }
    }

    for (auto it = spanner().cbegin(); it != spanner().cend(); ++it) {
        Spanner* spanner = (*it).second;
        if (spanner->isPalmMute()) {
            toPalmMute(spanner)->setChannel();
        }
        if (spanner->isVolta()) {
            toVolta(spanner)->setChannel();
        }
    }

    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (Staff* st : staves()) {
            track_idx_t strack = st->idx() * VOICES;
            track_idx_t etrack = strack + VOICES;
            for (track_idx_t track = strack; track < etrack; ++track) {
                if (!s->element(track)) {
                    continue;
                }
                EngravingItem* e = s->element(track);
                if (e->type() != ElementType::CHORD) {
                    continue;
                }
                Chord* c = toChord(e);
                size_t channel = st->channel(c->tick(), c->voice());
                Instrument* instr = c->part()->instrument(c->tick());
                if (channel >= instr->channel().size()) {
                    LOGD() << "Channel " << channel << " too high. Max " << instr->channel().size();
                    channel = 0;
                }
                for (Note* note : c->notes()) {
                    if (note->hidden()) {
                        continue;
                    }
                    note->setSubchannel(static_cast<int>(channel));
                }
            }
        }
    }
}

UndoStack* Score::undoStack() const { return m_masterScore->undoStack(); }
const RepeatList& Score::repeatList()  const { return m_masterScore->repeatList(); }
const RepeatList& Score::repeatList(bool expandRepeats, bool updateTies)  const
{
    return m_masterScore->repeatList(expandRepeats, updateTies);
}

TempoMap* Score::tempomap() const { return m_masterScore->tempomap(); }
TimeSigMap* Score::sigmap() const { return m_masterScore->sigmap(); }
//QQueue<MidiInputEvent>* Score::midiInputQueue() { return _masterScore->midiInputQueue(); }
std::list<MidiInputEvent>& Score::activeMidiPitches() { return m_masterScore->activeMidiPitches(); }
muse::async::Channel<ScoreChanges> Score::changesChannel() const { return m_masterScore->changesChannel(); }

void Score::setUpdateAll() { m_masterScore->setUpdateAll(); }

void Score::setLayoutAll(staff_idx_t staff, const EngravingItem* e) { m_masterScore->setLayoutAll(staff, e); }
void Score::setLayout(const Fraction& tick, staff_idx_t staff, const EngravingItem* e) { m_masterScore->setLayout(tick, staff, e); }
void Score::setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e)
{
    m_masterScore->setLayout(tick1, tick2, staff1, staff2, e);
}

CmdState& Score::cmdState() { return m_masterScore->cmdState(); }
const CmdState& Score::cmdState() const { return m_masterScore->cmdState(); }
void Score::addLayoutFlags(LayoutFlags f) { m_masterScore->addLayoutFlags(f); }
void Score::setInstrumentsChanged(bool v) { m_masterScore->setInstrumentsChanged(v); }

Fraction Score::loopBoundaryTick(LoopBoundaryType type) const { return m_masterScore->loopBoundaryTick(type); }
void Score::setLoopBoundaryTick(LoopBoundaryType type, Fraction tick) { m_masterScore->setLoopBoundaryTick(type, tick); }

//---------------------------------------------------------
//   ScoreLoad::_loading
//    If the _loading > 0 then pushes and pops to
//    the undo stack do not emit a warning.
//    Usually pushes and pops to the undo stack are only
//    valid inside a startCmd() - endCmd(). Exceptions
//    occurred during score loading.
//---------------------------------------------------------

int ScoreLoad::m_loading = 0;
}
