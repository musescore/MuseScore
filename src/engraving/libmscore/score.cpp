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
#include "rw/xml.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"
#include "infrastructure/symbolfonts.h"

#include "articulation.h"
#include "audio.h"
#include "beam.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "clef.h"
#include "excerpt.h"
#include "factory.h"
#include "fermata.h"
#include "glissando.h"
#include "gradualtempochange.h"
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
#include "part.h"
#include "pitchspelling.h"
#include "rehearsalmark.h"
#include "repeatlist.h"
#include "rest.h"
#include "scoreorder.h"
#include "segment.h"
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

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#include "accessibility/accessibleroot.h"
#endif

#include "config.h"

#include "log.h"

using namespace mu;
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
//   MeasureBaseList
//---------------------------------------------------------

MeasureBaseList::MeasureBaseList()
{
    _first = 0;
    _last  = 0;
    _size  = 0;
}

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void MeasureBaseList::push_back(MeasureBase* e)
{
    ++_size;
    if (_last) {
        _last->setNext(e);
        e->setPrev(_last);
        e->setNext(0);
    } else {
        _first = e;
        e->setPrev(0);
        e->setNext(0);
    }
    _last = e;
}

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void MeasureBaseList::push_front(MeasureBase* e)
{
    ++_size;
    if (_first) {
        _first->setPrev(e);
        e->setNext(_first);
        e->setPrev(0);
    } else {
        _last = e;
        e->setPrev(0);
        e->setNext(0);
    }
    _first = e;
}

//---------------------------------------------------------
//   add
//    insert e before e->next()
//---------------------------------------------------------

void MeasureBaseList::add(MeasureBase* e)
{
    MeasureBase* el = e->next();
    if (el == 0) {
        push_back(e);
        return;
    }
    if (el == _first) {
        push_front(e);
        return;
    }
    ++_size;
    e->setPrev(el->prev());
    el->prev()->setNext(e);
    el->setPrev(e);
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* el)
{
    --_size;
    if (el->prev()) {
        el->prev()->setNext(el->next());
    } else {
        _first = el->next();
    }
    if (el->next()) {
        el->next()->setPrev(el->prev());
    } else {
        _last = el->prev();
    }
}

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void MeasureBaseList::insert(MeasureBase* fm, MeasureBase* lm)
{
    ++_size;
    for (MeasureBase* m = fm; m != lm; m = m->next()) {
        ++_size;
    }
    MeasureBase* pm = fm->prev();
    if (pm) {
        pm->setNext(fm);
    } else {
        _first = fm;
    }
    MeasureBase* nm = lm->next();
    if (nm) {
        nm->setPrev(lm);
    } else {
        _last = lm;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* fm, MeasureBase* lm)
{
    --_size;
    for (MeasureBase* m = fm; m != lm; m = m->next()) {
        --_size;
    }
    MeasureBase* pm = fm->prev();
    MeasureBase* nm = lm->next();
    if (pm) {
        pm->setNext(nm);
    } else {
        _first = nm;
    }
    if (nm) {
        nm->setPrev(pm);
    } else {
        _last = pm;
    }
}

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void MeasureBaseList::change(MeasureBase* ob, MeasureBase* nb)
{
    nb->setPrev(ob->prev());
    nb->setNext(ob->next());
    if (ob->prev()) {
        ob->prev()->setNext(nb);
    }
    if (ob->next()) {
        ob->next()->setPrev(nb);
    }
    if (ob == _last) {
        _last = nb;
    }
    if (ob == _first) {
        _first = nb;
    }
    if (nb->type() == ElementType::HBOX || nb->type() == ElementType::VBOX
        || nb->type() == ElementType::TBOX || nb->type() == ElementType::FBOX) {
        nb->setParent(ob->system());
    }
    for (EngravingItem* e : nb->el()) {
        e->setParent(nb);
    }
}

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
    : EngravingObject(ElementType::SCORE, nullptr), _headersText(MAX_HEADERS, nullptr), _footersText(
        MAX_FOOTERS, nullptr),
    _selection(this),
    m_layout(this)
{
    Score::validScores.insert(this);
    _masterScore = 0;
    Layer l;
    l.name          = u"default";
    l.tags          = 1;
    _layer.push_back(l);
    _layerTags[0]   = u"default";

    m_symbolFont = SymbolFonts::fontByName(u"Leland");

    _fileDivision           = Constants::division;
    _style  = DefaultStyle::defaultStyle();

    m_rootItem = new RootItem(this);
    m_rootItem->init();
    createPaddingTable();

    m_shadowNote = new ShadowNote(this);
    m_shadowNote->setVisible(false);
}

Score::Score(MasterScore* parent, bool forcePartStyle /* = true */)
    : Score{}
{
    Score::validScores.insert(this);
    _masterScore = parent;
    if (DefaultStyle::defaultStyleForParts()) {
        _style = *DefaultStyle::defaultStyleForParts();
    } else {
        // inherit most style settings from parent
        _style = parent->style();

        checkChordList();

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
            _style.set(i, DefaultStyle::defaultStyle().value(i));
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
    _style.precomputeValues();
    _synthesizerState = parent->_synthesizerState;
    _mscVersion = parent->_mscVersion;
    createPaddingTable();
}

Score::Score(MasterScore* parent, const MStyle& s)
    : Score{parent}
{
    Score::validScores.insert(this);
    _style  = s;
    createPaddingTable();
}

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
{
    Score::validScores.erase(this);

    for (MuseScoreView* v : viewer) {
        v->removeScore();
    }
    // deselectAll();
    DeleteAll(_systems);   // systems are layout-only objects so we delete
                           // them prior to measures.
    for (MeasureBase* m = _measures.first(); m;) {
        MeasureBase* nm = m->next();
        if (m->isMeasure() && toMeasure(m)->mmRest()) {
            delete toMeasure(m)->mmRest();
        }
        delete m;
        m = nm;
    }

    _spanner.clear();

    DeleteAll(_parts);
    _parts.clear();

    DeleteAll(_staves);
    _staves.clear();

    DeleteAll(_pages);
    _pages.clear();

    _masterScore = nullptr;

    imageStore.clearUnused();

    delete m_rootItem;
    delete m_shadowNote;
}

mu::async::Channel<POS, unsigned> Score::posChanged() const
{
    return m_posChanged;
}

void Score::notifyPosChanged(POS pos, unsigned ticks)
{
    m_posChanged.send(pos, ticks);
}

mu::async::Channel<EngravingItem*> Score::elementDestroyed()
{
    return m_elementDestroyed;
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

    for (Part* part : _parts) {
        excerpt->parts().push_back(part);

        for (track_idx_t track = part->startTrack(); track < part->endTrack(); ++track) {
            excerpt->tracksMapping().insert({ track, track });
        }
    }

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
    _measures.add(m);
}

void Score::setUpTempoMapLater()
{
    _needSetUpTempoMap = true;
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

    for (Staff* staff : _staves) {
        staff->clearTimeSig();
    }

    if (isMaster()) {
        tempomap()->clear();
        sigmap()->clear();
        sigmap()->add(0, SigEvent(fm->ticks(),  fm->timesig(), 0));
    }

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

        rebuildTempoAndTimeSigMaps(m);

        tick += measureTicks;
    }

    if (isMaster()) {
        for (const auto& pair : spanner()) {
            const Spanner* spannerItem = pair.second;
            if (!spannerItem || !spannerItem->isGradualTempoChange()) {
                continue;
            }

            const GradualTempoChange* tempoChange = toGradualTempoChange(spannerItem);
            if (!tempoChange) {
                continue;
            }

            int tickPositionFrom = tempoChange->tick().ticks();
            BeatsPerSecond currentBps = tempomap()->tempo(tickPositionFrom);
            BeatsPerSecond newBps = currentBps * tempoChange->tempoChangeFactor();

            std::map<int, double> tempoCurve = TConv::easingValueCurve(tempoChange->ticks().ticks(),
                                                                       4 /*stepsCount*/,
                                                                       newBps.val - currentBps.val,
                                                                       tempoChange->easingMethod());

            for (const auto& pair : tempoCurve) {
                int tick = tickPositionFrom + pair.first;

                if (tempomap()->find(tick) == tempomap()->end()) {
                    tempomap()->setTempo(tick, BeatsPerSecond(currentBps.val + pair.second));
                }
            }
        }

        if (tempomap()->empty()) {
            tempomap()->setTempo(0, Constants::defaultTempo);
        }
    }

    masterScore()->updateRepeatListTempo();
    _needSetUpTempoMap = false;
}

//---------------------------------------------------------
//    fixTicks
///    updates tempomap and time sig map for a measure
//---------------------------------------------------------

void Score::rebuildTempoAndTimeSigMaps(Measure* measure)
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
            if (length != 0.0) {
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
            if (length != 0.0) {
                tempomap()->setPause(tick.ticks(), length);
            }
        } else if (segment.isTimeSigType()) {
            for (size_t staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                TimeSig* ts = toTimeSig(segment.element(staffIdx * VOICES));
                if (ts) {
                    staff(staffIdx)->addTimeSig(ts);
                }
            }
        } else if (segment.isChordRestType()) {
            if (!isMaster()) {
                continue;
            }
            double stretch = 0.0;
            for (EngravingItem* e : segment.annotations()) {
                if (e->isFermata() && toFermata(e)->play()) {
                    stretch = std::max(stretch, toFermata(e)->timeStretch());
                } else if (e->isTempoText()) {
                    TempoText* tt = toTempoText(e);
                    if (tt->isRelative()) {
                        tt->updateRelative();
                    }
                    tempomap()->setTempo(tt->segment()->tick().ticks(), tt->tempo());
                }
            }
            if (stretch != 0.0 && stretch != 1.0) {
                BeatsPerSecond otempo = tempomap()->tempo(segment.tick().ticks());
                BeatsPerSecond ntempo = otempo.val / stretch;
                tempomap()->setTempo(segment.tick().ticks(), ntempo);

                Fraction currentSegmentEndTick;

                if (segment.next1()) {
                    currentSegmentEndTick = segment.next1()->tick();
                } else {
                    currentSegmentEndTick = segment.tick() + segment.ticks();
                }

                Fraction etick = currentSegmentEndTick - Fraction(1, 480 * 4);
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
            Staff* s1 = _staves[i];
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

void Score::dragPosition(const PointF& p, staff_idx_t* rst, Segment** seg, double spacingFactor) const
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

    constexpr SegmentType st = SegmentType::ChordRest;
    Segment* segment = m->searchSegment(pppp.x(), st, strack, etrack, *seg, spacingFactor);
    if (segment) {
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
    _showInvisible = v;
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
    _showUnprintable = v;
}

//---------------------------------------------------------
//   setShowFrames
//---------------------------------------------------------

void Score::setShowFrames(bool v)
{
    _showFrames = v;
}

//---------------------------------------------------------
//   setShowPageborders
//---------------------------------------------------------

void Score::setShowPageborders(bool v)
{
    _showPageborders = v;
}

//---------------------------------------------------------
//   setMarkIrregularMeasures
//---------------------------------------------------------

void Score::setMarkIrregularMeasures(bool v)
{
    _markIrregularMeasures = v;
}

//---------------------------------------------------------
//   readOnly
//---------------------------------------------------------

bool Score::readOnly() const
{
    return _masterScore->readOnly();
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
    return _isOpen;
}

void Score::setIsOpen(bool open)
{
    _isOpen = open;
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
            int fret = staff->part()->instrument(n->tick())->stringData()->fret(n->pitch(), string, staff);
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
        return _pages.empty() ? nullptr : _pages.front();
    }

    for (Page* page : _pages) {
        RectF r = page->bbox().translated(page->pos());
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
            double sy2 = s->y() + s->bbox().height();
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
            for (size_t iii = i + 1; ii < n; ++iii) {
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
            if (mb->isMeasure() && (x < (mb->x() + mb->bbox().width()))) {
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
    staff_idx_t preferredStaffIdx = mu::nidx;
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
        if (track != mu::nidx) {
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
        staff_idx_t idx = mu::nidx;
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
            x2    = measure->bbox().width();
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
    double lineDist = s->staffType(tick)->lineDistance().val() * (s->isTabStaff(measure->tick()) ? 1 : .5) * mag * spatium();

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
    for (Staff* staff : _staves) {
        staff->spatiumChanged(oldValue, newValue);
    }
    _noteHeadWidth = m_symbolFont->width(SymId::noteheadBlack, newValue / SPATIUM20);
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
            measures()->add(toMeasureBase(m));
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
        cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
        o->staff()->updateOttava();
        setPlaylistDirty();
    }
    break;

    case ElementType::DYNAMIC:
        cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
        setPlaylistDirty();
        break;

    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* ic = toInstrumentChange(element);
        ic->part()->setInstrument(ic->instrument(), ic->segment()->tick());
        addLayoutFlags(LayoutFlag::REBUILD_MIDI_MAPPING);
        cmdState()._instrumentsChanged = true;
    }
    break;

    case ElementType::CHORD:
        setPlaylistDirty();
        // create playlist does not work here bc. tremolos may not be complete
        // createPlayEvents(toChord(element));
        break;

    case ElementType::NOTE:
    case ElementType::TREMOLO:
    case ElementType::ARTICULATION:
    case ElementType::ARPEGGIO:
    {
        if (parent && parent->isChord()) {
            createPlayEvents(toChord(parent));
        }
    }
    break;
    case ElementType::HARMONY:
        element->part()->updateHarmonyChannels(true);
        break;

    default:
        break;
    }
    element->triggerLayout();
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

//      LOGD("Score(%p) EngravingItem(%p)(%s) parent %p(%s)",
//         this, element, element->typeName(), parent, parent ? parent->typeName() : "");

    // special for MEASURE, HBOX, VBOX
    // their parent is not static

    ElementType et = element->type();

    if (et == ElementType::MEASURE
        || (et == ElementType::HBOX && !parent->isVBox())
        || et == ElementType::VBOX
        || et == ElementType::TBOX
        || et == ElementType::FBOX
        ) {
        MeasureBase* mb = toMeasureBase(element);
        measures()->remove(mb);
        System* system = mb->system();

        if (!system) {     // vertical boxes are not shown in continuous view so no system
            assert(lineMode() && (element->isVBox() || element->isTBox()));
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
        addLayoutFlags(LayoutFlag::PLAY_EVENTS);
    // fall through

    case ElementType::VOLTA:
    case ElementType::TRILL:
    case ElementType::VIBRATO:
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::GRADUAL_TEMPO_CHANGE:
    case ElementType::PALM_MUTE:
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

    case ElementType::OTTAVA:
    {
        Ottava* o = toOttava(element);
        o->triggerLayout();
        removeSpanner(o);
        o->staff()->updateOttava();
        cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
        setPlaylistDirty();
    }
    break;

    case ElementType::DYNAMIC:
        cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
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
        cmdState()._instrumentsChanged = true;
    }
    break;

    case ElementType::TREMOLO:
    case ElementType::ARTICULATION:
    case ElementType::ARPEGGIO:
    {
        EngravingItem* cr = element->parentItem();
        if (cr->isChord()) {
            createPlayEvents(toChord(cr));
        }
    }
    break;
    case ElementType::HARMONY:
        element->part()->updateHarmonyChannels(true, true);
        break;

    default:
        break;
    }
}

bool Score::containsElement(const EngravingItem* element) const
{
    if (!element) {
        return false;
    }

    EngravingItem* parent = element->parentItem();
    if (!parent) {
        return false;
    }

    std::vector<EngravingItem*> elements;
    parent->scanElements(&elements, collectElements, false /*all*/);

    return std::find(elements.cbegin(), elements.cend(), element) != elements.cend();
}

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* Score::firstMeasure() const
{
    MeasureBase* mb = _measures.first();
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
    if (m && styleB(Sid::createMultiMeasureRests) && m->hasMMRest()) {
        return m->mmRest();
    }
    return m;
}

//---------------------------------------------------------
//   firstMM
//---------------------------------------------------------

MeasureBase* Score::firstMM() const
{
    MeasureBase* m = _measures.first();
    if (m
        && m->type() == ElementType::MEASURE
        && styleB(Sid::createMultiMeasureRests)
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
    MeasureBase* mb = _measures.first();
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
    for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
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
    MeasureBase* mb = _measures.last();

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
    if (m && styleB(Sid::createMultiMeasureRests)) {
        Measure* m1 = const_cast<Measure*>(toMeasure(m->mmRest1()));
        if (m1) {
            return m1;
        }
    }
    return m;
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

#ifdef SCRIPT_INTERFACE
    // if called from QML/JS, tell QML engine not to garbage collect this object
//      if (seg)
//            QQmlEngine::setObjectOwnership(seg, QQmlEngine::CppOwnership);
#endif
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
//   inputPos
//---------------------------------------------------------

Fraction Score::inputPos() const
{
    return _is.tick();
}

//---------------------------------------------------------
//   scanElementsInRange
//---------------------------------------------------------

void Score::scanElementsInRange(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    Segment* startSeg = _selection.startSegment();
    for (Segment* s = startSeg; s && s != _selection.endSegment(); s = s->next1()) {
        s->scanElements(data, func, all);
        Measure* m = s->measure();
        if (m && s == m->first()) {
            Measure* mmr = m->mmRest();
            if (mmr) {
                mmr->scanElements(data, func, all);
            }
        }
    }
    for (EngravingItem* e : _selection.elements()) {
        if (e->isSpanner()) {
            Spanner* spanner = toSpanner(e);
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
    _selection = s;

    for (EngravingItem* e : _selection.elements()) {
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
    if (mu::contains(_metaTags, s)) {
        return _metaTags.at(s);
    }

    return mu::value(_masterScore->_metaTags, s);
}

//---------------------------------------------------------
//   setMetaTag
//---------------------------------------------------------

void Score::setMetaTag(const String& tag, const String& val)
{
    _metaTags.insert_or_assign(tag, val);
}

//---------------------------------------------------------
//   setSynthesizerState
//---------------------------------------------------------

void Score::setSynthesizerState(const SynthesizerState& s)
{
    // TODO: make undoable
    _synthesizerState = s;
}

//---------------------------------------------------------
//   removeAudio
//---------------------------------------------------------

void Score::removeAudio()
{
    delete _audio;
    _audio = 0;
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
    if (styleB(Sid::concertPitch) != score->styleB(Sid::concertPitch)) {
        score->cmdConcertPitchChanged(styleB(Sid::concertPitch));
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
    MeasureBase* pmb = last();
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

        addMeasure(nmb, 0);
        nmb->setNext(0);
        nmb->setPrev(pmb);
        nmb->setScore(this);

        pmb->setNext(nmb);
        pmb = nmb;
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
    undoAddElement(clef);
    clef->layout();

    undoChangeKeySig(ns, Fraction(0, 1), st->keySigEvent(Fraction(0, 1)));

    masterScore()->rebuildMidiMapping();
    cmdState()._instrumentsChanged = true;
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
                if (mu::contains(tupletMapping, tupletSrc[voice])) {
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
                            DeleteAll(chord->notes());
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
                        if (mu::contains(oldTies, note)) {
                            // Yes! Create a tie between the new notes and remove the
                            // old tie.
                            Tie* tie = oldTies[note].tie->clone();
                            tie->setStartNote(oldTies[note].nnote);
                            tie->setEndNote(nnote);
                            tie->setTrack(nnote->track());
                            undoAddElement(tie);
                            undoRemoveElement(oldTies[note].tie);
                            mu::remove(oldTies, note);
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

    undoRemovePart(part, sidx);
}

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, staff_idx_t idx)
{
    if (!part) {
        return;
    }

    bool inserted = false;
    staff_idx_t staff = 0;

    part->setScore(this);
    assignIdIfNeed(*part);

    for (auto i = _parts.begin(); i != _parts.end(); ++i) {
        if (staff >= idx) {
            _parts.insert(i, part);
            inserted = true;
            break;
        }
        staff += (*i)->nstaves();
    }
    if (!inserted) {
        _parts.push_back(part);
    }
    masterScore()->rebuildMidiMapping();
    setInstrumentsChanged(true);
    markInstrumentsAsPrimary(_parts);
}

void Score::appendPart(Part* part)
{
    if (!part) {
        return;
    }

    part->setScore(this);
    assignIdIfNeed(*part);
    _parts.push_back(part);
    markInstrumentsAsPrimary(_parts);
}

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
{
    part_idx_t index = mu::indexOf(_parts, part);

    if (index == mu::nidx) {
        for (size_t i = 0; i < _parts.size(); ++i) {
            if (_parts[i]->id() == part->id()) {
                index = i;
                break;
            }
        }
    }

    _parts.erase(_parts.begin() + index);
    markInstrumentsAsPrimary(_parts);

    if (_excerpt) {
        for (Part* excerptPart : _excerpt->parts()) {
            if (excerptPart->id() != part->id()) {
                continue;
            }

            mu::remove(_excerpt->parts(), excerptPart);
            markInstrumentsAsPrimary(_excerpt->parts());
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
    _staves.insert(_staves.begin() + idx, staff);

    for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
        Spanner* s = i->second;
        if (s->systemFlag()) {
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
            if (s->track2() != mu::nidx) {
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
    _staves.push_back(staff);

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
    for (System* system : _systems) {
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
            if (s->track2() != mu::nidx) {
                t = s->track2() - VOICES;
                s->setTrack2((t != mu::nidx) ? t : s->track());
            }
        }
    }

    mu::remove(_staves, staff);
    staff->part()->removeStaff(staff);

    updateStavesNumberForSystems();
}

//---------------------------------------------------------
//   adjustBracketsDel
//---------------------------------------------------------

void Score::adjustBracketsDel(size_t sidx, size_t eidx)
{
    for (size_t staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        Staff* staff = _staves[staffIdx];
        for (BracketItem* bi : staff->brackets()) {
            size_t span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx)) {
                continue;
            }
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(span - (eidx - sidx)));
            }
        }
    }
}

//---------------------------------------------------------
//   adjustBracketsIns
//---------------------------------------------------------

void Score::adjustBracketsIns(size_t sidx, size_t eidx)
{
    for (size_t staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        Staff* staff = _staves[staffIdx];
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
        Staff* staff = _staves[staffIdx];
        for (auto i = km.begin(); i != km.end(); ++i) {
            Fraction tick = Fraction::fromTicks(i->first);
            Measure* measure = tick2measure(tick);
            if (!measure) {
                continue;
            }
            if (staff->isDrumStaff(tick)) {
                continue;
            }
            KeySigEvent oKey = i->second;
            KeySigEvent nKey = oKey;
            int diff = -staff->part()->instrument(tick)->transpose().chromatic;
            if (diff != 0 && !styleB(Sid::concertPitch) && !oKey.isAtonal()) {
                nKey.setKey(transposeKey(nKey.key(), diff, staff->part()->preferSharpFlat()));
            }
            staff->setKey(tick, nKey);

            Segment* s = measure->getSegment(SegmentType::KeySig, tick);
            KeySig* keysig = Factory::createKeySig(s);
            keysig->setTrack(staffIdx * VOICES);
            keysig->setKeySigEvent(nKey);
            s->add(keysig);
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
                    // there is a matching time sig on this staff
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
    // normalize the keyevents to concert pitch if necessary
    if (firstStaff && !masterScore()->styleB(Sid::concertPitch) && firstStaff->part()->instrument()->transpose().chromatic) {
        int interval = firstStaff->part()->instrument()->transpose().chromatic;
        normalizedC = transposeKey(normalizedC, interval);
        for (auto i = tmpKeymap.begin(); i != tmpKeymap.end(); ++i) {
            int tick = i->first;
            Key oKey = i->second.key();
            tmpKeymap[tick].setKey(transposeKey(oKey, interval));
        }
    }

    // create initial keyevent for transposing instrument if necessary
    auto i = tmpKeymap.begin();
    if (i == tmpKeymap.end() || i->first != 0) {
        tmpKeymap[0].setKey(normalizedC);
    }

    return tmpKeymap;
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

//---------------------------------------------------------
//   sortSystemStaves
//---------------------------------------------------------

void Score::sortSystemObjects(std::vector<staff_idx_t>& dst)
{
    std::vector<staff_idx_t> moveTo;
    for (Staff* staff : systemObjectStaves) {
        moveTo.push_back(staff->idx());
    }
    // rebuild system object staves
    for (size_t i = 0; i < _staves.size(); i++) {
        staff_idx_t newLocation = mu::indexOf(dst, i);
        if (newLocation == mu::nidx) { //!dst.contains(_staves[i]->idx())) {
            // this staff was removed
            for (size_t j = 0; j < systemObjectStaves.size(); j++) {
                if (_staves[i]->idx() == moveTo[j]) {
                    // the removed staff was a system object staff
                    if (i == _staves.size() - 1 || mu::contains(moveTo, _staves[i + 1]->idx())) {
                        // this staff is at the end of the score, or is right before a new system object staff
                        moveTo[j] = mu::nidx;
                    } else {
                        moveTo[j] = i + 1;
                    }
                }
            }
        } else if (newLocation != _staves[i]->idx() && mu::contains(systemObjectStaves, _staves[i])) {
            // system object staff was moved somewhere, put the system objects at the top of its new group
            staff_idx_t topOfGroup = newLocation;
            String family = _staves[dst[newLocation]]->part()->familyId();
            while (topOfGroup > 0) {
                if (_staves[dst[topOfGroup - 1]]->part()->familyId() != family) {
                    // the staff above is of a different instrument family, current topOfGroup is destination
                    break;
                } else {
                    topOfGroup--;
                }
            }
            moveTo[mu::indexOf(systemObjectStaves, _staves[i])] = dst[topOfGroup];
        }
    }
    for (staff_idx_t i = 0; i < systemObjectStaves.size(); i++) {
        if (moveTo[i] == systemObjectStaves[i]->idx()) {
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
                    if ((e->isJump() || e->isMarker()) && e->isLinked() && e->track() == staff2track(systemObjectStaves[i]->idx())) {
                        if (moveTo[i] == mu::nidx) {
                            // delete this clone
                            m->remove(e);
                            e->unlink();
                            delete e;
                        } else {
                            e->setTrack(staff2track(_staves[moveTo[i]]->idx()));
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
                                if (e->track() == staff2track(systemObjectStaves[i]->idx()) && e->isLinked()) {
                                    if (moveTo[i] == mu::nidx) {
                                        s->removeAnnotation(e);
                                        e->unlink();
                                        delete e;
                                    } else {
                                        e->setTrack(staff2track(_staves[moveTo[i]]->idx()));
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // update systemObjectStaves with the correct staff
            if (moveTo[i] == mu::nidx) {
                systemObjectStaves.erase(systemObjectStaves.begin() + i);
                moveTo.erase(moveTo.begin() + i);
            } else {
                systemObjectStaves[i] = _staves[moveTo[i]];
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
    DeleteAll(systems());
    systems().clear();    //??
    _parts.clear();
    Part* curPart = nullptr;
    std::vector<Staff*> dl;
    std::map<size_t, size_t> trackMap;
    track_idx_t track = 0;
    for (staff_idx_t idx : dst) {
        Staff* staff = _staves[idx];
        if (staff->part() != curPart) {
            curPart = staff->part();
            curPart->clearStaves();
            _parts.push_back(curPart);
        }
        curPart->appendStaff(staff);
        dl.push_back(staff);
        for (size_t itrack = 0; itrack < VOICES; ++itrack) {
            trackMap.insert({ idx* VOICES + itrack, track++ });
        }
    }
    _staves = dl;

    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        m->sortStaves(dst);
        if (m->hasMMRest()) {
            m->mmRest()->sortStaves(dst);
        }
    }
    for (auto i : _spanner.map()) {
        Spanner* sp = i.second;
        voice_idx_t voice    = sp->voice();
        staff_idx_t staffIdx = sp->staffIdx();
        staff_idx_t idx = mu::indexOf(dst, staffIdx);
        if (idx != mu::nidx) {
            sp->setTrack(idx * VOICES + voice);
            if (sp->track2() != mu::nidx) {
                sp->setTrack2(idx * VOICES + (sp->track2() % VOICES));        // at least keep the voice...
            }
        }
    }
    setLayoutAll();
    markInstrumentsAsPrimary(_parts);
}

//---------------------------------------------------------
//   mapExcerptTracks
//---------------------------------------------------------

void Score::mapExcerptTracks(const std::vector<staff_idx_t>& dst)
{
    for (Excerpt* e : masterScore()->excerpts()) {
        const TracksMap& tr = e->tracksMapping();
        TracksMap tracks;
        for (auto it = tr.begin(); it != tr.end(); ++it) {
            staff_idx_t prvStaffIdx = it->first / VOICES;
            staff_idx_t curStaffIdx = mu::indexOf(dst, prvStaffIdx);
            int offset = static_cast<int>((curStaffIdx - prvStaffIdx) * VOICES);
            tracks.insert({ it->first + offset, it->second });
        }
        e->setTracksMapping(tracks);
    }
}

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag)
{
    if (flag == styleB(Sid::concertPitch)) {
        return;
    }

    undoChangeStyleVal(Sid::concertPitch, flag);         // change style flag

    for (Staff* staff : _staves) {
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

        transposeKeys(staffIdx, staffIdx + 1, Fraction(0, 1), lastSegment()->tick(), interval, true, !flag);

        for (Segment* segment = firstSegment(SegmentType::ChordRest); segment; segment = segment->next1(SegmentType::ChordRest)) {
            interval = staff->part()->instrument(segment->tick())->transpose();
            if (!flag) {
                interval.flip();
            }
            for (EngravingItem* e : segment->annotations()) {
                if (!e->isHarmony() || (e->track() < startTrack) || (e->track() >= endTrack)) {
                    continue;
                }
                Harmony* h  = toHarmony(e);
                int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                for (EngravingObject* se : h->linkList()) {
                    // don't transpose all links
                    // just ones resulting from mmrests
                    Harmony* he = toHarmony(se);              // toHarmony() does not work as e is an ScoreElement
                    if (he->staff() == h->staff()) {
                        undoTransposeHarmony(he, rootTpc, baseTpc);
                    }
                }
                //realized harmony should be invalid after a transpose command
                assert(!h->realizedHarmony().valid());
            }
        }
    }
}

//---------------------------------------------------------
//   padToggle
//---------------------------------------------------------

void Score::padToggle(Pad p, const EditData& ed)
{
    int oldDots = _is.duration().dots();
    switch (p) {
    case Pad::NOTE00:
        _is.setDuration(DurationType::V_LONG);
        break;
    case Pad::NOTE0:
        _is.setDuration(DurationType::V_BREVE);
        break;
    case Pad::NOTE1:
        _is.setDuration(DurationType::V_WHOLE);
        break;
    case Pad::NOTE2:
        _is.setDuration(DurationType::V_HALF);
        break;
    case Pad::NOTE4:
        _is.setDuration(DurationType::V_QUARTER);
        break;
    case Pad::NOTE8:
        _is.setDuration(DurationType::V_EIGHTH);
        break;
    case Pad::NOTE16:
        _is.setDuration(DurationType::V_16TH);
        break;
    case Pad::NOTE32:
        _is.setDuration(DurationType::V_32ND);
        break;
    case Pad::NOTE64:
        _is.setDuration(DurationType::V_64TH);
        break;
    case Pad::NOTE128:
        _is.setDuration(DurationType::V_128TH);
        break;
    case Pad::NOTE256:
        _is.setDuration(DurationType::V_256TH);
        break;
    case Pad::NOTE512:
        _is.setDuration(DurationType::V_512TH);
        break;
    case Pad::NOTE1024:
        _is.setDuration(DurationType::V_1024TH);
        break;
    case Pad::REST:
        if (noteEntryMode()) {
            _is.setRest(!_is.rest());
            _is.setAccidentalType(AccidentalType::NONE);
        } else if (selection().isNone()) {
            ed.view()->startNoteEntryMode();
            _is.setDuration(DurationType::V_QUARTER);
            _is.setRest(true);
        } else {
            for (ChordRest* cr : getSelectedChordRests()) {
                if (!cr->isRest()) {
                    setNoteRest(cr->segment(), cr->track(), NoteVal(), cr->durationTypeTicks(), DirectionV::AUTO, false,
                                _is.articulationIds());
                }
            }
        }
        break;
    case Pad::DOT:
        if ((_is.duration().dots() == 1) || (_is.duration() == DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(1);
        }
        break;
    case Pad::DOT2:
        if ((_is.duration().dots() == 2)
            || (_is.duration() == DurationType::V_512TH)
            || (_is.duration() == DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(2);
        }
        break;
    case Pad::DOT3:
        if ((_is.duration().dots() == 3)
            || (_is.duration() == DurationType::V_256TH)
            || (_is.duration() == DurationType::V_512TH)
            || (_is.duration() == DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(3);
        }
        break;
    case Pad::DOT4:
        if ((_is.duration().dots() == 4)
            || (_is.duration() == DurationType::V_128TH)
            || (_is.duration() == DurationType::V_256TH)
            || (_is.duration() == DurationType::V_512TH)
            || (_is.duration() == DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(4);
        }
        break;
    }
    if (p >= Pad::NOTE00 && p <= Pad::NOTE1024) {
        _is.setDots(0);
        //
        // if in "note enter" mode, reset
        // rest flag
        //
        if (noteEntryMode()) {
            if (usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
                switch (oldDots) {
                case 1:
                    padToggle(Pad::DOT, ed);
                    break;
                case 2:
                    padToggle(Pad::DOT2, ed);
                    break;
                case 3:
                    padToggle(Pad::DOT3, ed);
                    break;
                case 4:
                    padToggle(Pad::DOT4, ed);
                    break;
                }

                NoteVal nval;
                DirectionV stemDirection = DirectionV::AUTO;
                if (_is.rest()) {
                    // Enter a rest
                    nval = NoteVal();
                } else {
                    EngravingItem* e = selection().element();
                    if (e && e->isNote()) {
                        // use same pitch etc. as previous note
                        Note* n = toNote(e);
                        nval = n->noteVal();
                        stemDirection = n->chord()->stemDirection();
                    } else {
                        // enter a reasonable default note
                        Staff* s = staff(_is.track() / VOICES);
                        Fraction tick = _is.tick();
                        if (s->isTabStaff(tick)) {
                            // tab - use fret 0 on current string
                            nval.fret = 0;
                            nval.string = _is.string();
                            const Instrument* instr = s->part()->instrument(tick);
                            const StringData* stringData = instr->stringData();
                            nval.pitch = stringData->getPitch(nval.string, nval.fret, s);
                        } else if (s->isDrumStaff(tick)) {
                            // drum - use selected drum palette note
                            int n = _is.drumNote();
                            if (n == -1) {
                                // no selection on palette - find next valid pitch
                                const Drumset* ds = _is.drumset();
                                n = ds->nextPitch(n);
                            }
                            nval = NoteVal(n);
                        } else {
                            // standard staff - use middle line
                            ClefType clef = s->clef(tick);
                            Key key = s->key(tick);
                            int line = ((s->lines(tick) - 1) / 2) * 2;
                            nval = NoteVal(line2pitch(line, clef, key));
                        }
                    }
                }
                setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction(), stemDirection, false, _is.articulationIds());
                _is.moveToNextInputPos();
            } else {
                _is.setRest(false);
            }
        }
    }

    if (noteEntryMode()) {
        return;
    }

    std::vector<ChordRest*> crs;

    if (selection().isSingle()) {
        EngravingItem* e = selection().element();
        ChordRest* cr = InputState::chordRest(e);

        // do not allow to add a dot on a full measure rest
        if (cr && cr->isRest()) {
            Rest* r = toRest(cr);
            if (r->isFullMeasureRest()) {
                _is.setDots(0);
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
            ed.view()->startNoteEntryMode();
            deselect(e);
        }
    } else if (selection().isNone() && p != Pad::REST) {
        TDuration td = _is.duration();
        ed.view()->startNoteEntryMode();
        _is.setDuration(td);
        _is.setAccidentalType(AccidentalType::NONE);
    } else {
        const auto elements = selection().uniqueElements();
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

    for (ChordRest* cr : crs) {
        if (cr->isChord() && (toChord(cr)->isGrace())) {
            //
            // handle appoggiatura and acciaccatura
            //
            undoChangeChordRestLen(cr, _is.duration());
        } else {
            changeCRlen(cr, _is.duration());
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
#endif
}

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(EngravingItem* el)
{
    addRefresh(el->abbox());
    _selection.remove(el);
    setSelectionChanged(true);
    _selection.update();
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

    if (!_selection.elements().empty()) {
        onFocusedItemChanged(_selection.elements().back());
    }
}

void Score::doSelect(EngravingItem* e, SelectType type, staff_idx_t staffIdx)
{
    // Move the playhead to the selected element's preferred play position.
    if (e) {
        const auto playTick = e->playTick();
        if (masterScore()->playPos() != playTick) {
            masterScore()->setPlayPos(playTick);
        }
    }

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
    SelState selState = _selection.state();
    deselectAll();
    if (e == 0) {
        selState = SelState::NONE;
        setUpdateAll();
    } else {
        if (e->isMeasure()) {
            doSelect(e, SelectType::RANGE, staffIdx);
            return;
        }
        addRefresh(e->abbox());
        _selection.add(e);
        _is.setTrack(e->track());
        selState = SelState::LIST;
        if (e->type() == ElementType::NOTE) {
            e = e->parentItem();
        }
        if (e->isChordRest()) {
            _is.setLastSegment(_is.segment());
            _is.setSegment(toChordRest(e)->segment());
        }
    }
    _selection.setActiveSegment(0);
    _selection.setActiveTrack(0);

    _selection.setState(selState);
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
    SelState selState = _selection.state();

    if (_selection.isRange()) {
        doSelect(e, SelectType::SINGLE, 0);
        return;
    }

    if (e->isMeasure()) {
        Measure* m = toMeasure(e);
        Fraction tick  = m->tick();
        if (_selection.isNone()) {
            _selection.setRange(m->tick2segment(tick),
                                m == lastMeasure() ? 0 : m->last(),
                                0,
                                nstaves());
            setUpdateAll();
            selState = SelState::RANGE;
            _selection.updateSelectedElements();
        }
    } else if (!mu::contains(_selection.elements(), e)) {
        addRefresh(e->abbox());
        selState = SelState::LIST;
        _selection.add(e);
    }

    _selection.setState(selState);
}

//---------------------------------------------------------
//   selectRange
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::selectRange(EngravingItem* e, staff_idx_t staffIdx)
{
    track_idx_t activeTrack = e->track();
    // current selection is range extending to end of score?
    bool endRangeSelected = selection().isRange() && selection().endSegment() == nullptr;
    if (e->isMeasure()) {
        Measure* m  = toMeasure(e);
        Fraction tick    = m->tick();
        Fraction etick   = tick + m->ticks();
        activeTrack = staffIdx * VOICES;
        Segment* s1 = m->tick2segment(tick);
        if (!s1) {                        // m is corrupted!
            s1 = m->first(SegmentType::ChordRest);
        }
        Segment* s2 = m == lastMeasure() ? 0 : m->last();
        if (_selection.isNone() || (_selection.isList() && !_selection.isSingle())) {
            if (_selection.isList()) {
                deselectAll();
            }
            _selection.setRange(s1, s2, staffIdx, staffIdx + 1);
        } else if (_selection.isRange()) {
            _selection.extendRangeSelection(s1, s2, staffIdx, tick, etick);
        } else if (_selection.isSingle()) {
            EngravingItem* oe = selection().element();
            if (oe->isNote() || oe->isChordRest()) {
                if (oe->isNote()) {
                    oe = oe->parentItem();
                }
                ChordRest* cr = toChordRest(oe);
                Fraction oetick = cr->segment()->tick();
                Segment* startSegment = cr->segment();
                Segment* endSegment = m->last();
                if (tick < oetick) {
                    startSegment = m->tick2segment(tick);
                    if (etick <= oetick) {
                        SegmentType st = SegmentType::ChordRest | SegmentType::EndBarLine | SegmentType::Clef;
                        endSegment = cr->nextSegmentAfterCR(st);
                    }
                }
                staff_idx_t staffStart = staffIdx;
                staff_idx_t endStaff = staffIdx + 1;
                if (staffStart > cr->staffIdx()) {
                    staffStart = cr->staffIdx();
                } else if (cr->staffIdx() >= endStaff) {
                    endStaff = cr->staffIdx() + 1;
                }
                _selection.setRange(startSegment, endSegment, staffStart, endStaff);
            } else {
                deselectAll();
                _selection.setRange(s1, s2, staffIdx, staffIdx + 1);
            }
        } else {
            LOGD("SELECT_RANGE: measure: sel state %d", int(_selection.state()));
            return;
        }
    } else if (e->isNote() || e->isChordRest()) {
        if (e->isNote()) {
            e = e->parentItem();
        }
        ChordRest* cr = toChordRest(e);

        if (_selection.isNone() || (_selection.isList() && !_selection.isSingle())) {
            if (_selection.isList()) {
                deselectAll();
            }
            SegmentType st = SegmentType::ChordRest | SegmentType::EndBarLine | SegmentType::Clef;
            _selection.setRange(cr->segment(), cr->nextSegmentAfterCR(st), e->staffIdx(), e->staffIdx() + 1);
            activeTrack = cr->track();
        } else if (_selection.isSingle()) {
            EngravingItem* oe = _selection.element();
            if (oe && (oe->isNote() || oe->isRest() || oe->isMMRest())) {
                if (oe->isNote()) {
                    oe = oe->parentItem();
                }
                ChordRest* ocr = toChordRest(oe);

                Segment* endSeg = tick2segmentMM(ocr->segment()->tick() + ocr->actualTicks(), true);
                if (!endSeg) {
                    endSeg = ocr->segment()->next();
                }

                _selection.setRange(ocr->segment(), endSeg, oe->staffIdx(), oe->staffIdx() + 1);
                _selection.extendRangeSelection(cr);
            } else {
                doSelect(e, SelectType::SINGLE, 0);
                return;
            }
        } else if (_selection.isRange()) {
            _selection.extendRangeSelection(cr);
        } else {
            LOGD("sel state %d", int(_selection.state()));
            return;
        }
        if (!endRangeSelected && !_selection.endSegment()) {
            _selection.setEndSegment(cr->segment()->nextCR());
        }
        if (!_selection.startSegment()) {
            _selection.setStartSegment(cr->segment());
        }
    } else {
        // try to select similar in range
        EngravingItem* selectedElement = _selection.element();
        if (selectedElement && e->type() == selectedElement->type()) {
            staff_idx_t idx1 = selectedElement->staffIdx();
            staff_idx_t idx2 = e->staffIdx();
            if (idx2 < idx1) {
                staff_idx_t temp = idx1;
                idx1 = idx2;
                idx2 = temp;
            }

            if (idx1 != mu::nidx && idx2 != mu::nidx) {
                Fraction t1 = selectedElement->tick();
                Fraction t2 = e->tick();
                if (t1 > t2) {
                    Fraction temp = t1;
                    t1 = t2;
                    t2 = temp;
                }
                Segment* s1 = tick2segmentMM(t1, true, SegmentType::ChordRest);
                Segment* s2 = tick2segmentMM(t2, true, SegmentType::ChordRest);
                if (s2) {
                    s2 = s2->next1MM(SegmentType::ChordRest);
                }

                if (s1) {
                    _selection.setRange(s1, s2, idx1, idx2 + 1);
                    selectSimilarInRange(e);
                    if (selectedElement->track() == e->track()) {
                        // limit to this voice only
                        const std::vector<EngravingItem*>& list = _selection.elements();
                        for (EngravingItem* el : list) {
                            if (el->track() != e->track()) {
                                _selection.remove(el);
                            }
                        }
                    }

                    return;
                }
            }
        }
        doSelect(e, SelectType::SINGLE, staffIdx);
        return;
    }

    _selection.setActiveTrack(activeTrack);

    // doing this in note entry mode can clear selection
    if (_selection.startSegment() && !noteEntryMode()) {
        Fraction tick = _selection.startSegment()->tick();
        if (masterScore()->playPos() != tick) {
            masterScore()->setPlayPos(tick);
        }
    }

    _selection.updateSelectedElements();
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

    if ((p->staffStart != mu::nidx)
        && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx()))) {
        return;
    }

    if (p->voice != mu::nidx && p->voice != e->voice()) {
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

    if (p->measure && (p->measure != e->findMeasure())) {
        return;
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
    if ((p->staffStart != mu::nidx)
        && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx()))) {
        return;
    }
    if (p->voice != mu::nidx && p->voice != e->voice()) {
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
    }
    pattern.staffStart = sameStaff ? e->staffIdx() : mu::nidx;
    pattern.staffEnd = sameStaff ? e->staffIdx() + 1 : mu::nidx;
    pattern.voice   = mu::nidx;
    pattern.system  = 0;
    pattern.durationTicks = Fraction(-1, 1);

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
    }
    pattern.staffStart = selection().staffStart();
    pattern.staffEnd = selection().staffEnd();
    pattern.voice   = mu::nidx;
    pattern.system  = 0;
    pattern.durationTicks = Fraction(-1, 1);

    score->scanElementsInRange(&pattern, collectMatch);

    score->select(0, SelectType::SINGLE, 0);
    score->select(pattern.el, SelectType::ADD, 0);
}

//---------------------------------------------------------
//   enableVerticalSpread
//---------------------------------------------------------

bool Score::enableVerticalSpread() const
{
    return styleB(Sid::enableVerticalSpread) && (layoutMode() != LayoutMode::SYSTEM);
}

//---------------------------------------------------------
//   setEnableVerticalSpread
//---------------------------------------------------------

void Score::setEnableVerticalSpread(bool val)
{
    setStyleValue(Sid::enableVerticalSpread, val);
}

//---------------------------------------------------------
//   maxSystemDistance
//---------------------------------------------------------

double Score::maxSystemDistance() const
{
    if (enableVerticalSpread()) {
        return styleMM(Sid::maxSystemSpread);
    } else {
        return styleMM(Sid::maxSystemDistance);
    }
}

//---------------------------------------------------------
//   scoreOrder
//---------------------------------------------------------

ScoreOrder Score::scoreOrder() const
{
    ScoreOrder order = _scoreOrder;
    order.customized = !order.isScoreOrder(this);
    return order;
}

//---------------------------------------------------------
//   setScoreOrder
//---------------------------------------------------------

void Score::setScoreOrder(ScoreOrder order)
{
    _scoreOrder = order;
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
        InstrumentTemplate* tp = searchTemplate(part->instrumentId());
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
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const RectF& bbox)
{
    select(0, SelectType::SINGLE, 0);
    RectF fr(bbox.normalized());
    for (Page* page : pages()) {
        RectF pr(page->bbox());
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
            if (frr.contains(item->abbox())) {
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

    if (_selection.elements().empty()) {
        _selection.setState(SelState::NONE);
        setUpdateAll();
        return;
    }
    _selection.setState(SelState::LIST);

    for (const EngravingItem* e : _selection.elements()) {
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
        _selection.setRange(startSegment, endSegment, startStaff, endStaff + 1);
        if (!_selection.isRange()) {
            _selection.setState(SelState::RANGE);
        }
        _selection.updateSelectedElements();
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
        tempomap()->setTempo(0, Constants::defaultTempo);
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

//---------------------------------------------------------
//   cmdSelectAll
//---------------------------------------------------------

void Score::cmdSelectAll()
{
    if (_measures.size() == 0) {
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
    Segment* s = _selection.startSegment();
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

    _selection.setRange(toMeasure(sm)->first(), toMeasure(em)->last(), 0, nstaves());
}

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void Score::undo(UndoCommand* cmd, EditData* ed) const
{
    undoStack()->push(cmd, ed);
}

//---------------------------------------------------------
//   linkId
//---------------------------------------------------------

int Score::linkId()
{
    return (masterScore()->_linkId)++;
}

// val is a used link id
void Score::linkId(int val)
{
    Score* s = masterScore();
    if (val >= s->_linkId) {
        s->_linkId = val + 1;       // update unused link id
    }
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
//   switchLayer
//---------------------------------------------------------

bool Score::switchLayer(const String& s)
{
    int layerIdx = 0;
    for (const Layer& l : layer()) {
        if (s == l.name) {
            setCurrentLayer(layerIdx);
            return true;
        }
        ++layerIdx;
    }
    return false;
}

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void Score::appendPart(const InstrumentTemplate* t)
{
    Part* part = new Part(this);
    part->initFromInstrTemplate(t);
    size_t n = nstaves();
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
    undoInsertPart(part, static_cast<int>(n));
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

void Score::addSpanner(Spanner* s)
{
    _spanner.addSpanner(s);
    s->added();
    if (s->startElement()) {
        s->startElement()->startingSpanners().push_back(s);
    }
    if (s->endElement()) {
        s->endElement()->endingSpanners().push_back(s);
    }
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeSpanner(Spanner* s)
{
    _spanner.removeSpanner(s);
    s->removed();
    if (s->startElement()) {
        mu::remove_if(s->startElement()->startingSpanners(), [s](Spanner* sp) { return sp == s; });
    }
    if (s->endElement()) {
        mu::remove_if(s->endElement()->endingSpanners(), [s](Spanner* sp) { return sp == s; });
    }
}

//---------------------------------------------------------
//   isSpannerStartEnd
//    does is spanner start or end at tick position tick
//    for track ?
//---------------------------------------------------------

bool Score::isSpannerStartEnd(const Fraction& tick, track_idx_t track) const
{
    for (auto i : _spanner.map()) {
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
    _unmanagedSpanner.insert(s);
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeUnmanagedSpanner(Spanner* s)
{
    _unmanagedSpanner.erase(s);
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
        s = nullptr;
    }
    if (s) {
        return toChordRest(s->element(track));
    }
    return nullptr;
}

//---------------------------------------------------------
//   findCRinStaff
//    find last chord/rest on staff that ends before tick
//---------------------------------------------------------

ChordRest* Score::findCRinStaff(const Fraction& tick, staff_idx_t staffIdx) const
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

    Segment* s      = m->first(SegmentType::ChordRest);
    track_idx_t strack      = staffIdx * VOICES;
    track_idx_t etrack      = strack + VOICES;
    track_idx_t actualTrack = strack;

    Fraction lastTick = Fraction(-1, 1);
    for (Segment* ns = s;; ns = ns->next(SegmentType::ChordRest)) {
        if (ns == 0 || ns->tick() > ptick) {
            break;
        }
        // found a segment; now find longest cr on this staff that does not overlap tick
        for (track_idx_t t = strack; t < etrack; ++t) {
            ChordRest* cr = toChordRest(ns->element(t));
            if (cr) {
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

//---------------------------------------------------------
//   cmdNextPrevSystem
//---------------------------------------------------------

ChordRest* Score::cmdNextPrevSystem(ChordRest* cr, bool next)
{
    auto newCR = cr;
    auto currentMeasure = cr->measure();
    auto currentSystem = currentMeasure->system() ? currentMeasure->system() : currentMeasure->mmRest1()->system();
    if (!currentSystem) {
        return cr;
    }
    auto destinationMeasure = currentSystem->firstMeasure();
    auto firstSegment = destinationMeasure->first(SegmentType::ChordRest);

    // Case: Go to next system
    if (next) {
        if ((destinationMeasure = currentSystem->lastMeasure()->nextMeasure())) {
            // There is a next system present: get it and accommodate for MMRest
            currentSystem = destinationMeasure->system() ? destinationMeasure->system() : destinationMeasure->mmRest1()->system();
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
            if (!(destinationMeasure = destinationMeasure->prevMeasure())) {
                if (!(destinationMeasure = destinationMeasure->prevMeasureMM())) {
                    return cr;
                }
            }
            if (!(currentSystem = destinationMeasure->system() ? destinationMeasure->system() : destinationMeasure->mmRest1()->system())) {
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
            if (score()->styleB(Sid::createMultiMeasureRests) && currentMeasure->hasMMRest()) {
                currentMeasure = currentMeasure->mmRest1();
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
        while (m && m->isEmpty(mu::nidx)) {
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
        if (score()->styleB(Sid::createMultiMeasureRests) && destinationMeasure->hasMMRest()) {
            destinationMeasure = destinationMeasure->mmRest1();
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
    for (size_t track = 0; track < ntracks(); track += VOICES) {
        bool found = false;
        size_t maxLyrics = 1;
        const RepeatList& rlist = repeatList();
        for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->setPlaybackCount(0);
        }
        // follow the repeat segments
        for (const RepeatSegment* rs : rlist) {
            Fraction startTick  = Fraction::fromTicks(rs->tick);
            Fraction endTick    = startTick + Fraction::fromTicks(rs->len());
            for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                int playCount = m->playbackCount();
                for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                    // consider voice 1 only
                    ChordRest* cr = toChordRest(seg->element(static_cast<int>(track)));
                    if (!cr || cr->lyrics().empty()) {
                        continue;
                    }
                    if (cr->lyrics().size() > maxLyrics) {
                        maxLyrics = cr->lyrics().size();
                    }
                    if (playCount >= int(cr->lyrics().size())) {
                        continue;
                    }
                    Lyrics* l = cr->lyrics(playCount, PlacementV::BELOW);            // TODO: ABOVE
                    if (!l) {
                        continue;
                    }
                    found = true;
                    String lyric = l->plainText().trimmed();
                    if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END) {
                        result += lyric + u" ";
                    } else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics::Syllabic::MIDDLE) {
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
        for (unsigned lyricsNumber = 0; lyricsNumber < maxLyrics; lyricsNumber++) {
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                unsigned playCount = m->playbackCount();
                if (lyricsNumber >= playCount) {
                    for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                        // consider voice 1 only
                        ChordRest* cr = toChordRest(seg->element(static_cast<int>(track)));
                        if (!cr || cr->lyrics().empty()) {
                            continue;
                        }
                        if (cr->lyrics().size() > maxLyrics) {
                            maxLyrics = cr->lyrics().size();
                        }
                        if (lyricsNumber >= cr->lyrics().size()) {
                            continue;
                        }
                        Lyrics* l = cr->lyrics(lyricsNumber, PlacementV::BELOW);              // TODO
                        if (!l) {
                            continue;
                        }
                        found = true;
                        String lyric = l->plainText().trimmed();
                        if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END) {
                            result += lyric + u" ";
                        } else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics:: Syllabic::MIDDLE) {
                            result += lyric;
                        }
                    }
                }
            }
        }
        if (found) {
            result += u"\n\n";
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
        Key key = st->key(t);
        if (st->staffType(t)->group() == StaffGroup::PERCUSSION || st->keySigEvent(t).isAtonal()) {         // ignore percussion and custom / atonal key
            continue;
        }
        result = key;
        int diff = st->part()->instrument()->transpose().chromatic; //TODO keySigs and pitched to unpitched instr changes
        if (!styleB(Sid::concertPitch) && diff) {
            result = transposeKey(key, diff, st->part()->preferSharpFlat());
        }
        break;
    }
    return int(result);
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

int Score::duration()
{
    masterScore()->setExpandRepeats(true);
    const RepeatList& rl = repeatList();
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
    const RepeatList& rl = repeatList2();
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

void Score::changeSelectedNotesVoice(voice_idx_t voice)
{
    std::vector<EngravingItem*> el;
    std::vector<EngravingItem*> oel = selection().elements();       // make copy
    for (EngravingItem* e : oel) {
        if (e->type() != ElementType::NOTE) {
            continue;
        }

        Note* note   = toNote(e);
        Chord* chord = note->chord();

        // move grace notes with main chord only
        if (chord->isGrace()) {
            continue;
        }

        if (chord->voice() != voice) {
            Segment* s       = chord->segment();
            Measure* m       = s->measure();
            size_t notes     = chord->notes().size();
            track_idx_t dstTrack     = chord->staffIdx() * VOICES + voice;
            ChordRest* dstCR = toChordRest(s->element(dstTrack));
            Chord* dstChord  = nullptr;

            if (excerpt() && mu::key(excerpt()->tracksMapping(), dstTrack, mu::nidx) == mu::nidx) {
                break;
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
                undoRemoveElement(dstCR);
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
                        expandVoice(s, /*m->first(SegmentType::ChordRest,*/ dstTrack);
                    }
                    makeGapVoice(s, dstTrack, chord->actualTicks(), s->tick());
                }
            }

            // move note to destination chord
            if (dstChord) {
                // create & add new note
                Note* newNote = Factory::copyNote(*note);
                newNote->setSelected(false);
                newNote->setParent(dstChord);
                undoAddElement(newNote);
                el.push_back(newNote);
                // add new chord if one was created
                if (dstChord != dstCR) {
                    undoAddCR(dstChord, m, s->tick());
                }
                // reconnect the tie to this note, if any
                Tie* tie = note->tieBack();
                if (tie) {
                    undoChangeSpannerElements(tie, tie->startNote(), newNote);
                }
                // reconnect the tie from this note, if any
                tie = note->tieFor();
                if (tie) {
                    undoChangeSpannerElements(tie, newNote, tie->endNote());
                }
                // remove original note
                if (notes > 1) {
                    undoRemoveElement(note);
                } else if (notes == 1) {
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
                        undoRemoveElement(gc);
                        ngc->setParent(dstChord);
                        ngc->setTrack(dstChord->track());
                        undoAddElement(ngc);
                    }
                    // remove chord, replace with rest
                    undoRemoveElement(chord);
                    undoAddCR(r, m, s->tick());
                }
            }
        }
    }

    if (!el.empty()) {
        selection().clear();
    }

    select(el, SelectType::ADD, mu::nidx);
    setLayoutAll();
}

std::set<ID> Score::partIdsFromRange(const track_idx_t trackFrom, const track_idx_t trackTo) const
{
    std::set<ID> result;

    for (const Part* part : m_score->parts()) {
        if (trackTo < part->startTrack() || trackFrom >= part->endTrack()) {
            continue;
        }

        result.insert(part->id());
    }

    return result;
}

std::set<staff_idx_t> Score::staffIdsFromRange(const track_idx_t trackFrom, const track_idx_t trackTo) const
{
    std::set<staff_idx_t> result;

    for (const Part* part : m_score->parts()) {
        if (trackTo < part->startTrack() || trackFrom >= part->endTrack()) {
            continue;
        }

        std::set<staff_idx_t> staffIdxList = part->staveIdxList();

        result.insert(staffIdxList.cbegin(), staffIdxList.cend());
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
    undo(new ChangeStyleVal(this, styleToReset, defStyle.value(styleToReset)));
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

TranslatableString Score::getTextStyleUserName(TextStyleType tid)
{
    if (int(tid) >= int(TextStyleType::USER1) && int(tid) <= int(TextStyleType::USER12)) {
        int idx = int(tid) - int(TextStyleType::USER1);
        Sid sid[] = { Sid::user1Name, Sid::user2Name, Sid::user3Name, Sid::user4Name, Sid::user5Name, Sid::user6Name,
                      Sid::user7Name, Sid::user8Name, Sid::user9Name, Sid::user10Name, Sid::user11Name, Sid::user12Name };
        String userName = styleSt(sid[idx]);
        if (!userName.empty()) {
            return TranslatableString::untranslatable(userName);
        }
    }
    return TConv::userName(tid);
}

String Score::name() const
{
    return _excerpt ? _excerpt->name() : String();
}

//---------------------------------------------------------
//   addRefresh
//---------------------------------------------------------

void Score::addRefresh(const mu::RectF& r)
{
    _updateState.refresh.unite(r);
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
    for (Staff* s : _staves) {
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
    for (Part* p : _parts) {
        if (p == part) {
            break;
        }
        idx += p->nstaves();
    }
    return idx;
}

Staff* Score::staffById(const ID& staffId) const
{
    for (Staff* staff : _staves) {
        if (staff->id() == staffId) {
            return staff;
        }
    }

    return nullptr;
}

Part* Score::partById(const ID& partId) const
{
    for (Part* part : _parts) {
        if (part->id() == partId) {
            return part;
        }
    }

    return nullptr;
}

int Score::visiblePartCount() const
{
    int count = 0;
    for (const Part* part : _parts) {
        if (part->show()) {
            ++count;
        }
    }
    return count;
}

ShadowNote& Score::shadowNote() const
{
    return *m_shadowNote;
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

    SegmentType st = SegmentType::ChordRest;
    for (Segment* s = m->first(st); s; s = s->next1(st)) {
        for (track_idx_t i = 0; i < tracks; ++i) {
            EngravingItem* e = s->element(i);
            if (e == 0 || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                // connect a tie without end note
                Tie* tie = n->tieFor();
                if (tie && !tie->endNote()) {
                    Note* nnote;
                    if (_mscVersion <= 114) {
                        nnote = searchTieNote114(n);
                    } else {
                        nnote = searchTieNote(n);
                    }
                    if (nnote == 0) {
                        if (!silent) {
                            LOGD("next note at %d track %zu for tie not found (version %d)", s->tick().ticks(), i, _mscVersion);
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
        }
    }
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

    m_symbolFont = SymbolFonts::fontByName(style().value(Sid::MusicalSymbolFont).value<String>());
    _noteHeadWidth = m_symbolFont->width(SymId::noteheadBlack, spatium() / SPATIUM20);

    m_layoutOptions.updateFromStyle(style());
    m_layout.doLayoutRange(m_layoutOptions, st, et);

    if (_resetAutoplace) {
        _resetAutoplace = false;
        resetAutoplace();
    }

    if (_resetDefaults) {
        _resetDefaults = false;
        resetDefaults();
    }
}

void Score::createPaddingTable()
{
    for (size_t i=0; i < TOT_ELEMENT_TYPES; ++i) {
        for (size_t j=0; j < TOT_ELEMENT_TYPES; ++j) {
            _paddingTable[i][j] = _minimumPaddingUnit;
        }
    }

    const double ledgerPad = 0.25 * spatium();
    const double ledgerLength = styleMM(Sid::ledgerLineLength);

    /* NOTE: the padding value for note->note is NOT minNoteDistance, because minNoteDistance
     * should only apply to notes of the same voice. Notes from different voices should be
     * allowed to get much closer. So we set the general padding at minimumPaddingUnit,
     * but we introduce an appropriate exception for same-voice cases in Shape::minHorizontalDistance().
     */
    _paddingTable[ElementType::NOTE][ElementType::NOTE] = _minimumPaddingUnit;
    _paddingTable[ElementType::NOTE][ElementType::LEDGER_LINE] = 0.35 * spatium();
    _paddingTable[ElementType::NOTE][ElementType::ACCIDENTAL]
        = std::max(static_cast<double>(styleMM(Sid::accidentalNoteDistance)), 0.35 * spatium());
    _paddingTable[ElementType::NOTE][ElementType::REST] = styleMM(Sid::minNoteDistance);
    _paddingTable[ElementType::NOTE][ElementType::CLEF] = 1.0 * spatium();
    _paddingTable[ElementType::NOTE][ElementType::ARPEGGIO] = 0.6 * spatium();
    _paddingTable[ElementType::NOTE][ElementType::BAR_LINE] = styleMM(Sid::noteBarDistance);
    _paddingTable[ElementType::NOTE][ElementType::KEYSIG] = 0.75 * spatium();
    _paddingTable[ElementType::NOTE][ElementType::TIMESIG] = 0.75 * spatium();

    _paddingTable[ElementType::LEDGER_LINE][ElementType::NOTE] = _paddingTable[ElementType::NOTE][ElementType::LEDGER_LINE];
    _paddingTable[ElementType::LEDGER_LINE][ElementType::LEDGER_LINE] = ledgerPad;
    _paddingTable[ElementType::LEDGER_LINE][ElementType::ACCIDENTAL]
        = std::max(static_cast<double>(styleMM(
                                           Sid::accidentalNoteDistance)),
                   _paddingTable[ElementType::NOTE][ElementType::ACCIDENTAL] - ledgerLength / 2);
    _paddingTable[ElementType::LEDGER_LINE][ElementType::REST] = _paddingTable[ElementType::LEDGER_LINE][ElementType::NOTE];
    _paddingTable[ElementType::LEDGER_LINE][ElementType::CLEF]
        = std::max(_paddingTable[ElementType::NOTE][ElementType::CLEF] - ledgerLength / 2, ledgerPad);
    _paddingTable[ElementType::LEDGER_LINE][ElementType::ARPEGGIO] = 0.5 * spatium();
    _paddingTable[ElementType::LEDGER_LINE][ElementType::BAR_LINE]
        = std::max(_paddingTable[ElementType::NOTE][ElementType::BAR_LINE] - ledgerLength, ledgerPad);
    _paddingTable[ElementType::LEDGER_LINE][ElementType::KEYSIG]
        = std::max(_paddingTable[ElementType::NOTE][ElementType::KEYSIG] - ledgerLength / 2, ledgerPad);
    _paddingTable[ElementType::LEDGER_LINE][ElementType::TIMESIG]
        = std::max(_paddingTable[ElementType::NOTE][ElementType::TIMESIG] - ledgerLength / 2, ledgerPad);

    _paddingTable[ElementType::HOOK][ElementType::NOTE] = 0.5 * spatium();
    _paddingTable[ElementType::HOOK][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::HOOK][ElementType::NOTE] - ledgerLength, ledgerPad);
    _paddingTable[ElementType::HOOK][ElementType::ACCIDENTAL] = 0.35 * spatium();
    _paddingTable[ElementType::HOOK][ElementType::REST] = _paddingTable[ElementType::HOOK][ElementType::NOTE];
    _paddingTable[ElementType::HOOK][ElementType::CLEF] = 0.5 * spatium();
    _paddingTable[ElementType::HOOK][ElementType::ARPEGGIO] = 0.35 * spatium();
    _paddingTable[ElementType::HOOK][ElementType::BAR_LINE] = 1 * spatium();
    _paddingTable[ElementType::HOOK][ElementType::KEYSIG] = 1.15 * spatium();
    _paddingTable[ElementType::HOOK][ElementType::TIMESIG] = 1.15 * spatium();

    _paddingTable[ElementType::NOTEDOT][ElementType::NOTE] = std::max(styleMM(Sid::dotNoteDistance), styleMM(Sid::dotDotDistance));
    _paddingTable[ElementType::NOTEDOT][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::NOTEDOT][ElementType::NOTE] - ledgerLength, ledgerPad);
    _paddingTable[ElementType::NOTEDOT][ElementType::ACCIDENTAL] = _paddingTable[ElementType::NOTEDOT][ElementType::NOTE];
    _paddingTable[ElementType::NOTEDOT][ElementType::REST] = _paddingTable[ElementType::NOTEDOT][ElementType::NOTE];
    _paddingTable[ElementType::NOTEDOT][ElementType::CLEF] = 1.0 * spatium();
    _paddingTable[ElementType::NOTEDOT][ElementType::ARPEGGIO] = 0.5 * spatium();
    _paddingTable[ElementType::NOTEDOT][ElementType::BAR_LINE] = 0.8 * spatium();
    _paddingTable[ElementType::NOTEDOT][ElementType::KEYSIG] = 1.35 * spatium();
    _paddingTable[ElementType::NOTEDOT][ElementType::TIMESIG] = 1.35 * spatium();

    _paddingTable[ElementType::REST][ElementType::NOTE] = _paddingTable[ElementType::NOTE][ElementType::REST];
    _paddingTable[ElementType::REST][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::REST][ElementType::NOTE] - ledgerLength / 2, ledgerPad);
    _paddingTable[ElementType::REST][ElementType::ACCIDENTAL] = 0.45 * spatium();
    _paddingTable[ElementType::REST][ElementType::REST] = _paddingTable[ElementType::REST][ElementType::NOTE];
    _paddingTable[ElementType::REST][ElementType::CLEF] = _paddingTable[ElementType::NOTE][ElementType::CLEF];
    _paddingTable[ElementType::REST][ElementType::BAR_LINE] = 1.65 * spatium();
    _paddingTable[ElementType::REST][ElementType::KEYSIG] = 1.5 * spatium();
    _paddingTable[ElementType::REST][ElementType::TIMESIG] = 1.5 * spatium();

    _paddingTable[ElementType::CLEF][ElementType::NOTE] = styleMM(Sid::clefKeyRightMargin);
    _paddingTable[ElementType::CLEF][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::CLEF][ElementType::NOTE] - ledgerLength / 2, ledgerPad);
    _paddingTable[ElementType::CLEF][ElementType::ACCIDENTAL] = 0.75 * spatium();
    _paddingTable[ElementType::CLEF][ElementType::REST] = 1.35 * spatium();
    _paddingTable[ElementType::CLEF][ElementType::CLEF] = 0.75 * spatium();
    _paddingTable[ElementType::CLEF][ElementType::ARPEGGIO] = 1.15 * spatium();
    _paddingTable[ElementType::CLEF][ElementType::BAR_LINE] = styleMM(Sid::clefBarlineDistance);
    _paddingTable[ElementType::CLEF][ElementType::KEYSIG] = styleMM(Sid::clefKeyDistance);
    _paddingTable[ElementType::CLEF][ElementType::TIMESIG] = styleMM(Sid::clefTimesigDistance);

    _paddingTable[ElementType::BAR_LINE][ElementType::NOTE] = styleMM(Sid::barNoteDistance);
    _paddingTable[ElementType::BAR_LINE][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::BAR_LINE][ElementType::NOTE] - ledgerLength, ledgerPad);
    _paddingTable[ElementType::BAR_LINE][ElementType::ACCIDENTAL] = styleMM(Sid::barAccidentalDistance);
    _paddingTable[ElementType::BAR_LINE][ElementType::REST] = styleMM(Sid::barNoteDistance);
    _paddingTable[ElementType::BAR_LINE][ElementType::CLEF] = styleMM(Sid::clefLeftMargin);
    _paddingTable[ElementType::BAR_LINE][ElementType::ARPEGGIO] = 0.65 * spatium();
    _paddingTable[ElementType::BAR_LINE][ElementType::BAR_LINE] = 1.35 * spatium();
    _paddingTable[ElementType::BAR_LINE][ElementType::KEYSIG] = styleMM(Sid::keysigLeftMargin);
    _paddingTable[ElementType::BAR_LINE][ElementType::TIMESIG] = styleMM(Sid::timesigLeftMargin);

    _paddingTable[ElementType::KEYSIG][ElementType::NOTE] = 1.75 * spatium();
    _paddingTable[ElementType::KEYSIG][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::KEYSIG][ElementType::NOTE] - ledgerLength, ledgerPad);
    _paddingTable[ElementType::KEYSIG][ElementType::ACCIDENTAL] = 1.6 * spatium();
    _paddingTable[ElementType::KEYSIG][ElementType::REST] = _paddingTable[ElementType::KEYSIG][ElementType::NOTE];
    _paddingTable[ElementType::KEYSIG][ElementType::CLEF] = 1.0 * spatium();
    _paddingTable[ElementType::KEYSIG][ElementType::ARPEGGIO] = 1.35 * spatium();
    _paddingTable[ElementType::KEYSIG][ElementType::BAR_LINE] = styleMM(Sid::keyBarlineDistance);
    _paddingTable[ElementType::KEYSIG][ElementType::KEYSIG] = 1 * spatium();
    _paddingTable[ElementType::KEYSIG][ElementType::TIMESIG] = styleMM(Sid::keyTimesigDistance);

    _paddingTable[ElementType::TIMESIG][ElementType::NOTE] = 1.35 * spatium();
    _paddingTable[ElementType::TIMESIG][ElementType::LEDGER_LINE]
        = std::max(_paddingTable[ElementType::TIMESIG][ElementType::NOTE] - ledgerLength, ledgerPad);
    _paddingTable[ElementType::TIMESIG][ElementType::ACCIDENTAL] = 0.8 * spatium();
    _paddingTable[ElementType::TIMESIG][ElementType::REST] = _paddingTable[ElementType::TIMESIG][ElementType::NOTE];
    _paddingTable[ElementType::TIMESIG][ElementType::CLEF] = 1.0 * spatium();
    _paddingTable[ElementType::TIMESIG][ElementType::ARPEGGIO] = 1.35 * spatium();
    _paddingTable[ElementType::TIMESIG][ElementType::BAR_LINE] = styleMM(Sid::timesigBarlineDistance);
    _paddingTable[ElementType::TIMESIG][ElementType::KEYSIG] = styleMM(Sid::keyTimesigDistance);
    _paddingTable[ElementType::TIMESIG][ElementType::TIMESIG] = 1.0 * spatium();

    // Obtain the Stem -> * and * -> Stem values from the note equivalents
    _paddingTable[ElementType::STEM] = _paddingTable[ElementType::NOTE];
    for (auto& elem: _paddingTable) {
        elem[ElementType::STEM] = elem[ElementType::NOTE];
    }

    _paddingTable[ElementType::STEM][ElementType::NOTE] = styleMM(Sid::minNoteDistance);
    _paddingTable[ElementType::STEM][ElementType::STEM] = 0.85 * spatium();
    _paddingTable[ElementType::STEM][ElementType::ACCIDENTAL] = 0.35 * spatium();
    _paddingTable[ElementType::STEM][ElementType::LEDGER_LINE] = 0.35 * spatium();
    _paddingTable[ElementType::LEDGER_LINE][ElementType::STEM] = 0.35 * spatium();

    // Ambitus
    _paddingTable[ElementType::AMBITUS].fill(styleMM(Sid::ambitusMargin));
    for (auto& elem: _paddingTable) {
        elem[ElementType::AMBITUS] = styleMM(Sid::ambitusMargin);
    }

    // Breath
    _paddingTable[ElementType::BREATH].fill(1.0 * spatium());
    for (auto& elem: _paddingTable) {
        elem[ElementType::BREATH] = 1.0 * spatium();
    }

    // Temporary hack, because some padding is already constructed inside the lyrics themselves.
    _paddingTable[ElementType::BAR_LINE][ElementType::LYRICS] = 0.0 * spatium();

    // Harmony
    _paddingTable[ElementType::BAR_LINE][ElementType::HARMONY] = 0.5 * styleMM(Sid::minHarmonyDistance);
    _paddingTable[ElementType::HARMONY][ElementType::HARMONY] = styleMM(Sid::minHarmonyDistance);

    // Chordlines
    _paddingTable[ElementType::CHORDLINE].fill(0.35 * spatium());
    for (auto& elem: _paddingTable) {
        elem[ElementType::CHORDLINE] = 0.35 * spatium();
    }
    _paddingTable[ElementType::BAR_LINE][ElementType::CHORDLINE] = 0.65 * spatium();
    _paddingTable[ElementType::CHORDLINE][ElementType::BAR_LINE] = 0.65 * spatium();

    // For the x -> fingering padding use the same values as x -> accidental
    for (auto& elem : _paddingTable) {
        elem[ElementType::FINGERING] = elem[ElementType::ACCIDENTAL];
    }
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

    double availableHeight = (styleD(Sid::pageHeight) - styleD(Sid::pageOddTopMargin) - styleD(Sid::pageOddBottomMargin)) * DPI; // convert from inches to DPI
    double titleHeight = (!measures()->empty() && measures()->first()->isVBox()) ? measures()->first()->height() : 0.0;
    availableHeight -= titleHeight;
    double totalNeededSpaces = 4 * _staves.size() * breathingSpace;
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

    setSpatium(targetSpatium);
    createPaddingTable();
}

UndoStack* Score::undoStack() const { return _masterScore->undoStack(); }
const RepeatList& Score::repeatList()  const { return _masterScore->repeatList(); }
const RepeatList& Score::repeatList2()  const { return _masterScore->repeatList2(); }
TempoMap* Score::tempomap() const { return _masterScore->tempomap(); }
TimeSigMap* Score::sigmap() const { return _masterScore->sigmap(); }
//QQueue<MidiInputEvent>* Score::midiInputQueue() { return _masterScore->midiInputQueue(); }
std::list<MidiInputEvent>& Score::activeMidiPitches() { return _masterScore->activeMidiPitches(); }
async::Channel<ScoreChangesRange> Score::changesChannel() const { return _masterScore->changesChannel(); }

void Score::setUpdateAll() { _masterScore->setUpdateAll(); }

void Score::setLayoutAll(staff_idx_t staff, const EngravingItem* e) { _masterScore->setLayoutAll(staff, e); }
void Score::setLayout(const Fraction& tick, staff_idx_t staff, const EngravingItem* e) { _masterScore->setLayout(tick, staff, e); }
void Score::setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e)
{
    _masterScore->setLayout(tick1, tick2, staff1, staff2, e);
}

CmdState& Score::cmdState() { return _masterScore->cmdState(); }
const CmdState& Score::cmdState() const { return _masterScore->cmdState(); }
void Score::addLayoutFlags(LayoutFlags f) { _masterScore->addLayoutFlags(f); }
void Score::setInstrumentsChanged(bool v) { _masterScore->setInstrumentsChanged(v); }

Fraction Score::pos(POS pos) const { return _masterScore->pos(pos); }
void Score::setPos(POS pos, Fraction tick) { _masterScore->setPos(pos, tick); }

//---------------------------------------------------------
//   ScoreLoad::_loading
//    If the _loading > 0 then pushes and pops to
//    the undo stack do not emit a warning.
//    Usually pushes and pops to the undo stack are only
//    valid inside a startCmd() - endCmd(). Exceptions
//    occurred during score loading.
//---------------------------------------------------------

int ScoreLoad::_loading = 0;
}
