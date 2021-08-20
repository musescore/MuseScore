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
#include "layout.h"

#include "../score.h"
#include "../masterscore.h"
#include "../measure.h"
#include "../scorefont.h"
#include "../bracket.h"
#include "../chordrest.h"
#include "../box.h"
#include "../marker.h"
#include "../barline.h"
#include "../undo.h"
#include "../part.h"
#include "../keysig.h"
#include "../chord.h"
#include "../stem.h"
#include "../lyrics.h"
#include "../measurenumber.h"
#include "../fingering.h"
#include "../mmrestrange.h"
#include "../stafflines.h"
#include "../tuplet.h"
#include "../tie.h"
#include "../layout.h"

#include "layoutmeasure.h"

using namespace mu::engraving;
using namespace Ms;
//---------------------------------------------------------
//   CmdStateLocker
//---------------------------------------------------------

class CmdStateLocker
{
    Score* m_score = nullptr;
public:
    CmdStateLocker(Score* s)
        : m_score(s) { m_score->cmdState().lock(); }
    ~CmdStateLocker() { m_score->cmdState().unlock(); }
};

//---------------------------------------------------------
//   almostZero
//---------------------------------------------------------

static bool inline almostZero(qreal value)
{
    // 1e-3 is close enough to zero to see it as zero.
    return value > -1e-3 && value < 1e-3;
}

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

static bool validMMRestMeasure(Measure* m)
{
    if (m->irregular()) {
        return false;
    }

    int n = 0;
    for (Segment* s = m->first(); s; s = s->next()) {
        for (Element* e : s->annotations()) {
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText()
                  || e->isInstrumentChange())) {
                return false;
            }
        }
        if (s->isChordRestType()) {
            bool restFound = false;
            int tracks = m->score()->ntracks();
            for (int track = 0; track < tracks; ++track) {
                if ((track % VOICES) == 0 && !m->score()->staff(track / VOICES)->show()) {
                    track += VOICES - 1;
                    continue;
                }
                if (s->element(track)) {
                    if (!s->element(track)->isRest()) {
                        return false;
                    }
                    restFound = true;
                }
            }
            for (Element* e : s->annotations()) {
                if (e->isFermata()) {
                    return false;
                }
            }
            if (restFound) {
                ++n;
            }
            // measure is not empty if there is more than one rest
            if (n > 1) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//  breakMultiMeasureRest
//    return true if this measure should start a new
//    multi measure rest
//---------------------------------------------------------

static bool breakMultiMeasureRest(Measure* m)
{
    if (m->breakMultiMeasureRest()) {
        return true;
    }

    if (m->repeatStart()
        || (m->prevMeasure() && m->prevMeasure()->repeatEnd())
        || (m->isIrregular())
        || (m->prevMeasure() && m->prevMeasure()->isIrregular())
        || (m->prevMeasure() && (m->prevMeasure()->sectionBreak()))) {
        return true;
    }

    auto sl = m->score()->spannerMap().findOverlapping(m->tick().ticks(), m->endTick().ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        // break for first measure of volta or textline and first measure *after* volta
        if ((s->isVolta() || s->isTextLine()) && (s->tick() == m->tick() || s->tick2() == m->tick())) {
            return true;
        }
    }

    // break for marker in this measure
    for (Element* e : m->el()) {
        if (e->isMarker()) {
            Marker* mark = toMarker(e);
            if (!(mark->align() & Align::RIGHT)) {
                return true;
            }
        }
    }

    // break for marker & jump in previous measure
    Measure* pm = m->prevMeasure();
    if (pm) {
        for (Element* e : pm->el()) {
            if (e->isJump()) {
                return true;
            } else if (e->isMarker()) {
                Marker* mark = toMarker(e);
                if (mark->align() & Align::RIGHT) {
                    return true;
                }
            }
        }
    }

    // break for MeasureRepeat group
    for (int staffIdx = 0; staffIdx < m->score()->nstaves(); ++staffIdx) {
        if (m->isMeasureRepeatGroup(staffIdx)
            || (m->prevMeasure() && m->prevMeasure()->isMeasureRepeatGroup(staffIdx))) {
            return true;
        }
    }

    for (Segment* s = m->first(); s; s = s->next()) {
        for (Element* e : s->annotations()) {
            if (!e->visible()) {
                continue;
            }
            if (e->isRehearsalMark()
                || e->isTempoText()
                || ((e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isInstrumentChange())
                    && (e->systemFlag() || m->score()->staff(e->staffIdx())->show()))) {
                return true;
            }
        }
        for (int staffIdx = 0; staffIdx < m->score()->nstaves(); ++staffIdx) {
            if (!m->score()->staff(staffIdx)->show()) {
                continue;
            }
            Element* e = s->element(staffIdx * VOICES);
            if (!e || e->generated()) {
                continue;
            }
            if (s->isStartRepeatBarLineType()) {
                return true;
            }
            if (s->isType(SegmentType::KeySig | SegmentType::TimeSig) && m->tick().isNotZero()) {
                return true;
            }
            if (s->isClefType()) {
                if (s->tick() != m->endTick() && m->tick().isNotZero()) {
                    return true;
                }
            }
        }
    }
    if (pm) {
        Segment* s = pm->findSegmentR(SegmentType::EndBarLine, pm->ticks());
        if (s) {
            for (int staffIdx = 0; staffIdx < s->score()->nstaves(); ++staffIdx) {
                BarLine* bl = toBarLine(s->element(staffIdx * VOICES));
                if (bl) {
                    BarLineType t = bl->barLineType();
                    if (t != BarLineType::NORMAL && t != BarLineType::BROKEN && t != BarLineType::DOTTED && !bl->generated()) {
                        return true;
                    } else {
                        break;
                    }
                }
            }
        }
        if (pm->findSegment(SegmentType::Clef, m->tick())) {
            return true;
        }
    }
    return false;
}

Layout::Layout(Ms::Score* score)
    : m_score(score)
{
}

void Layout::doLayoutRange(const Fraction& st, const Fraction& et)
{
    CmdStateLocker cmdStateLocker(m_score);
    LayoutContext lc(m_score);

    Fraction stick(st);
    Fraction etick(et);
    Q_ASSERT(!(stick == Fraction(-1, 1) && etick == Fraction(-1, 1)));

    if (!m_score->last() || (m_score->lineMode() && !m_score->firstMeasure())) {
        qDebug("empty score");
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();
        qDeleteAll(m_score->pages());
        m_score->pages().clear();
        lc.getNextPage();
        return;
    }
//      if (!_systems.isEmpty())
//            return;
    bool layoutAll = stick <= Fraction(0, 1) && (etick < Fraction(0, 1) || etick >= m_score->masterScore()->last()->endTick());
    if (stick < Fraction(0, 1)) {
        stick = Fraction(0, 1);
    }
    if (etick < Fraction(0, 1)) {
        etick = m_score->last()->endTick();
    }

    lc.endTick = etick;
    m_score->_scoreFont = ScoreFont::fontByName(m_score->style().value(Sid::MusicalSymbolFont).toString());
    m_score->_noteHeadWidth = m_score->_scoreFont->width(SymId::noteheadBlack, m_score->spatium() / SPATIUM20);

    if (m_score->cmdState().layoutFlags & LayoutFlag::REBUILD_MIDI_MAPPING) {
        if (m_score->isMaster()) {
            m_score->masterScore()->rebuildMidiMapping();
        }
    }
    if (m_score->cmdState().layoutFlags & LayoutFlag::FIX_PITCH_VELO) {
        m_score->updateVelo();
    }
#if 0 // TODO: needed? It was introduced in ab9774ec4098512068b8ef708167d9aa6e702c50
    if (cmdState().layoutFlags & LayoutFlag::PLAY_EVENTS) {
        createPlayEvents();
    }
#endif

    //---------------------------------------------------
    //    initialize layout context lc
    //---------------------------------------------------

    MeasureBase* m = m_score->tick2measure(stick);
    if (m == 0) {
        m = m_score->first();
    }
    // start layout one measure earlier to handle clefs and cautionary elements
    if (m->prevMeasureMM()) {
        m = m->prevMeasureMM();
    } else if (m->prev()) {
        m = m->prev();
    }
    while (!m->isMeasure() && m->prev()) {
        m = m->prev();
    }

    // if the first measure of the score is part of a multi measure rest
    // m->system() will return a nullptr. We need to find the multi measure
    // rest which replaces the measure range

    if (!m->system() && m->isMeasure() && toMeasure(m)->hasMMRest()) {
        qDebug("  don’t start with mmrest");
        m = toMeasure(m)->mmRest();
    }

//      qDebug("start <%s> tick %d, system %p", m->name(), m->tick(), m->system());

    if (m_score->lineMode()) {
        lc.prevMeasure = 0;
        lc.nextMeasure = m;         //_showVBox ? first() : firstMeasure();
        lc.startTick   = m->tick();
        layoutLinear(layoutAll, lc);
        return;
    }
    if (!layoutAll && m->system()) {
        System* system  = m->system();
        int systemIndex = m_score->_systems.indexOf(system);
        lc.page         = system->page();
        lc.curPage      = m_score->pageIdx(lc.page);
        if (lc.curPage == -1) {
            lc.curPage = 0;
        }
        lc.curSystem   = system;
        lc.systemList  = m_score->_systems.mid(systemIndex);

        if (systemIndex == 0) {
            lc.nextMeasure = m_score->_showVBox ? m_score->first() : m_score->firstMeasure();
        } else {
            System* prevSystem = m_score->_systems[systemIndex - 1];
            lc.nextMeasure = prevSystem->measures().back()->next();
        }

        m_score->_systems.erase(m_score->_systems.begin() + systemIndex, m_score->_systems.end());
        if (!lc.nextMeasure->prevMeasure()) {
            lc.measureNo = 0;
            lc.tick      = Fraction(0, 1);
        } else {
            const MeasureBase* mb = lc.nextMeasure->prev();
            if (mb) {
                mb = mb->findPotentialSectionBreak();
            }
            LayoutBreak* sectionBreak = mb->sectionBreakElement();
            // TODO: also use mb in else clause here?
            // probably not, only actual measures have meaningful numbers
            if (sectionBreak && sectionBreak->startWithMeasureOne()) {
                lc.measureNo = 0;
            } else {
                lc.measureNo = lc.nextMeasure->prevMeasure()->no()                             // will be adjusted later with respect
                               + (lc.nextMeasure->prevMeasure()->irregular() ? 0 : 1);         // to the user-defined offset.
            }
            lc.tick      = lc.nextMeasure->tick();
        }
    } else {
//  qDebug("layoutAll, systems %p %d", &_systems, int(_systems.size()));
        //lc.measureNo   = 0;
        //lc.tick        = 0;
        // qDeleteAll(_systems);
        // _systems.clear();
        // lc.systemList  = _systems;
        // _systems.clear();

        for (System* s : qAsConst(m_score->_systems)) {
            for (Bracket* b : s->brackets()) {
                if (b->selected()) {
                    m_score->_selection.remove(b);
                    m_score->setSelectionChanged(true);
                }
            }
//                  for (SpannerSegment* ss : s->spannerSegments())
//                        ss->setParent(0);
            s->setParent(nullptr);
        }
        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            mb->setSystem(0);
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                toMeasure(mb)->mmRest()->setSystem(0);
            }
        }
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();

        qDeleteAll(m_score->pages());
        m_score->pages().clear();

        lc.nextMeasure = m_score->_showVBox ? m_score->first() : m_score->firstMeasure();
    }

    lc.prevMeasure = 0;

    LayoutMeasure::getNextMeasure(m_score, lc);
    lc.curSystem = lc.collectSystem();

    lc.layout();
}

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Layout::layoutLinear(bool layoutAll, LayoutContext& lc)
{
    lc.score = m_score;
    resetSystems(layoutAll, lc);

    collectLinearSystem(lc);
//      hideEmptyStaves(systems().front(), true);     this does not make sense

    lc.layoutLinear();
}

//---------------------------------------------------------
//   resetSystems
//    in linear mode there is only one page
//    which contains one system
//---------------------------------------------------------

void Layout::resetSystems(bool layoutAll, LayoutContext& lc)
{
    Page* page = 0;
    if (layoutAll) {
        for (System* s : qAsConst(m_score->_systems)) {
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setParent(0);
            }
        }
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();
        qDeleteAll(m_score->pages());
        m_score->pages().clear();
        if (!m_score->firstMeasure()) {
            qDebug("no measures");
            return;
        }

        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            mb->setSystem(0);
        }

        page = new Page(m_score);
        m_score->pages().push_back(page);
        page->bbox().setRect(0.0, 0.0, m_score->loWidth(), m_score->loHeight());
        page->setNo(0);

        System* system = new System(m_score);
        m_score->_systems.push_back(system);
        page->appendSystem(system);
        system->adjustStavesNumber(m_score->nstaves());
    } else {
        if (m_score->pages().isEmpty()) {
            return;
        }
        page = m_score->pages().front();
        System* system = m_score->systems().front();
        system->clear();
        system->adjustStavesNumber(m_score->nstaves());
    }
    lc.page = page;
}

//---------------------------------------------------------
//   collectLinearSystem
//   Append all measures to System. VBox is not included to System
//---------------------------------------------------------

void Layout::collectLinearSystem(LayoutContext& lc)
{
    System* system = m_score->systems().front();
    system->setInstrumentNames(/* longNames */ true);

    PointF pos;
    bool firstMeasure = true;       //lc.startTick.isZero();

    //set first measure to lc.nextMeasures for following
    //utilizing in getNextMeasure()
    lc.nextMeasure = m_score->_measures.first();
    lc.tick = Fraction(0, 1);
    LayoutMeasure::getNextMeasure(m_score, lc);

    while (lc.curMeasure) {
        qreal ww = 0.0;
        if (lc.curMeasure->isVBox() || lc.curMeasure->isTBox()) {
            lc.curMeasure->setParent(nullptr);
            LayoutMeasure::getNextMeasure(m_score, lc);
            continue;
        }
        system->appendMeasure(lc.curMeasure);
        if (lc.curMeasure->isMeasure()) {
            Measure* m = toMeasure(lc.curMeasure);
            if (m->mmRest()) {
                m->mmRest()->setSystem(nullptr);
            }
            if (firstMeasure) {
                system->layoutSystem(pos.rx());
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                m->addSystemHeader(true);
                pos.rx() += system->leftMargin();
                firstMeasure = false;
            } else if (m->header()) {
                m->removeSystemHeader();
            }
            if (m->trailer()) {
                m->removeSystemTrailer();
            }
            if (m->tick() >= lc.startTick && m->tick() <= lc.endTick) {
                // for measures in range, do full layout
                m->createEndBarLines(false);
                m->computeMinWidth();
                ww = m->width();
                m->stretchMeasure(ww);
            } else {
                // for measures not in range, use existing layout
                ww = m->width();
                if (m->pos() != pos) {
                    // fix beam positions
                    // other elements with system as parent are processed in layoutSystemElements()
                    // but full beam processing is expensive and not needed if we adjust position here
                    PointF p = pos - m->pos();
                    for (const Segment& s : m->segments()) {
                        if (!s.isChordRestType()) {
                            continue;
                        }
                        for (int track = 0; track < m_score->ntracks(); ++track) {
                            Element* e = s.element(track);
                            if (e) {
                                ChordRest* cr = toChordRest(e);
                                if (cr->beam() && cr->beam()->elements().front() == cr) {
                                    cr->beam()->rpos() += p;
                                }
                            }
                        }
                    }
                }
            }
            m->setPos(pos);
            m->layoutStaffLines();
        } else if (lc.curMeasure->isHBox()) {
            lc.curMeasure->setPos(pos + PointF(toHBox(lc.curMeasure)->topGap(), 0.0));
            lc.curMeasure->layout();
            ww = lc.curMeasure->width();
        }
        pos.rx() += ww;

        LayoutMeasure::getNextMeasure(m_score, lc);
    }

    system->setWidth(pos.x());
}

//---------------------------------------------------------
//   layoutDrumsetChord
//---------------------------------------------------------

static void layoutDrumsetChord(Chord* c, const Drumset* drumset, const StaffType* st, qreal spatium)
{
    for (Note* note : c->notes()) {
        int pitch = note->pitch();
        if (!drumset->isValid(pitch)) {
            // qDebug("unmapped drum note %d", pitch);
        } else if (!note->fixed()) {
            note->undoChangeProperty(Pid::HEAD_GROUP, int(drumset->noteHead(pitch)));
            int line = drumset->line(pitch);
            note->setLine(line);

            int off  = st->stepOffset();
            qreal ld = st->lineDistance().val();
            note->rypos()  = (line + off * 2.0) * spatium * .5 * ld;
        }
    }
}

//---------------------------------------------------------
//   extendedStemLenWithTwoNotesTremolo
//    Goal: To extend stem of one of the chords to make the tremolo less steep
//    Returns a modified pair of stem lengths of two chords
//---------------------------------------------------------

std::pair<qreal, qreal> Layout::extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, qreal stemLen1, qreal stemLen2)
{
    const qreal spatium = tremolo->spatium();
    Chord* c1 = tremolo->chord1();
    Chord* c2 = tremolo->chord2();
    Stem* s1 = c1->stem();
    Stem* s2 = c2->stem();
    const qreal sgn1 = c1->up() ? -1.0 : 1.0;
    const qreal sgn2 = c2->up() ? -1.0 : 1.0;
    const qreal stemTipDistance = (s1 && s2) ? (s2->pagePos().y() + stemLen2) - (s1->pagePos().y() + stemLen1)
                                  : (c2->stemPos().y() + stemLen2) - (c1->stemPos().y() + stemLen1);

    // same staff & same direction: extend one of the stems
    if (c1->staffMove() == c2->staffMove() && c1->up() == c2->up()) {
        const bool stem1Higher = stemTipDistance > 0.0;
        if (std::abs(stemTipDistance) > 1.0 * spatium) {
            if ((c1->up() && !stem1Higher) || (!c1->up() && stem1Higher)) {
                return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - 1.0 * spatium), stemLen2 };
            } else {   /* if ((c1->up() && stem1Higher) || (!c1->up() && !stem1Higher)) */
                return { stemLen1, stemLen2 + sgn2 * (std::abs(stemTipDistance) - 1.0 * spatium) };
            }
        }
    }

// TODO: cross-staff two-note tremolo. Currently doesn't generate the right result in some cases.
#if 0
    // cross-staff & beam between staves: extend both stems by the same length
    else if (tremolo->crossStaffBeamBetween()) {
        const qreal sw = tremolo->score()->styleS(Sid::tremoloStrokeWidth).val();
        const qreal td = tremolo->score()->styleS(Sid::tremoloDistance).val();
        const qreal tremoloMinHeight = ((tremolo->lines() - 1) * td + sw) * spatium;
        const qreal dy = c1->up() ? tremoloMinHeight - stemTipDistance : tremoloMinHeight + stemTipDistance;
        const bool tooShort = dy > 1.0 * spatium;
        const bool tooLong = dy < -1.0 * spatium;
        const qreal idealDistance = 1.0 * spatium - tremoloMinHeight;

        if (tooShort) {
            return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - idealDistance) / 2.0,
                     stemLen2 + sgn2 * (std::abs(stemTipDistance) - idealDistance) / 2.0 };
        } else if (tooLong) {
            return { stemLen1 - sgn1 * (std::abs(stemTipDistance) + idealDistance) / 2.0,
                     stemLen2 - sgn2 * (std::abs(stemTipDistance) + idealDistance) / 2.0 };
        }
    }
#endif

    return { stemLen1, stemLen2 };
}

//---------------------------------------------------------
//   processLines
//---------------------------------------------------------

static void processLines(System* system, std::vector<Spanner*> lines, bool align)
{
    std::vector<SpannerSegment*> segments;
    for (Spanner* sp : lines) {
        SpannerSegment* ss = sp->layoutSystem(system);         // create/layout spanner segment for this system
        if (ss->autoplace()) {
            segments.push_back(ss);
        }
    }

    if (align && segments.size() > 1) {
        const int nstaves = system->staves()->size();
        constexpr qreal minY = -1000000.0;
        const qreal defaultY = segments[0]->rypos();
        std::vector<qreal> y(nstaves, minY);

        for (SpannerSegment* ss : segments) {
            if (ss->visible()) {
                qreal& staffY = y[ss->staffIdx()];
                staffY = qMax(staffY, ss->rypos());
            }
        }
        for (SpannerSegment* ss : segments) {
            if (!ss->isStyled(Pid::OFFSET)) {
                continue;
            }
            const qreal staffY = y[ss->staffIdx()];
            if (staffY > minY) {
                ss->rypos() = staffY;
            } else {
                ss->rypos() = defaultY;
            }
        }
    }

    //
    // add shapes to skyline
    //
    for (SpannerSegment* ss : segments) {
        if (ss->addToSkyline()) {
            system->staff(ss->staffIdx())->skyline().add(ss->shape().translated(ss->pos()));
        }
    }
}

//---------------------------------------------------------
//   isTopTuplet
//    returns true for the first CR of a tuplet that is not cross-staff
//---------------------------------------------------------

static bool isTopTuplet(ChordRest* cr)
{
    Tuplet* t = cr->tuplet();
    if (t && t->elements().front() == cr) {
        // find top level tuplet
        while (t->tuplet()) {
            t = t->tuplet();
        }
        // consider tuplet cross if anything moved within it
        if (t->cross()) {
            return false;
        } else {
            return true;
        }
    }

    // no tuplet or not first element
    return false;
}

//---------------------------------------------------------
//   layoutTies
//---------------------------------------------------------

static void layoutTies(Chord* ch, System* system, const Fraction& stick)
{
    SysStaff* staff = system->staff(ch->staffIdx());
    if (!staff->show()) {
        return;
    }
    for (Note* note : ch->notes()) {
        Tie* t = note->tieFor();
        if (t) {
            TieSegment* ts = t->layoutFor(system);
            if (ts && ts->addToSkyline()) {
                staff->skyline().add(ts->shape().translated(ts->pos()));
            }
        }
        t = note->tieBack();
        if (t) {
            if (t->startNote()->tick() < stick) {
                TieSegment* ts = t->layoutBack(system);
                if (ts && ts->addToSkyline()) {
                    staff->skyline().add(ts->shape().translated(ts->pos()));
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutHarmonies
//---------------------------------------------------------

static void layoutHarmonies(const std::vector<Segment*>& sl)
{
    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if (e->isHarmony()) {
                Harmony* h = toHarmony(e);
                // For chord symbols that coincide with a chord or rest,
                // a partial layout can also happen (if needed) during ChordRest layout
                // in order to calculate a bbox and allocate its shape to the ChordRest.
                // But that layout (if it happens at all) does not do autoplace,
                // so we need the full layout here.
                h->layout();
                h->autoplaceSegmentElement();
            }
        }
    }
}

//---------------------------------------------------------
//   alignHarmonies
//---------------------------------------------------------

static void alignHarmonies(const System* system, const std::vector<Segment*>& sl, bool harmony, const qreal maxShiftAbove,
                           const qreal maxShiftBelow)
{
    // Help class.
    // Contains harmonies/fretboard per segment.
    class HarmonyList : public QList<Element*>
    {
        QMap<const Segment*, QList<Element*> > elements;
        QList<Element*> modified;

        Element* getReferenceElement(const Segment* s, bool above, bool visible) const
        {
            // Returns the reference element for aligning.
            // When a segments contains multiple harmonies/fretboard, the lowest placed
            // element (for placement above, otherwise the highest placed element) is
            // used for alignment.
            Element* element { nullptr };
            for (Element* e : elements[s]) {
                // Only chord symbols have styled offset, fretboards don't.
                if (!e->autoplace() || (e->isHarmony() && !e->isStyled(Pid::OFFSET)) || (visible && !e->visible())) {
                    continue;
                }
                if (!element) {
                    element = e;
                } else {
                    if ((e->placeAbove() && above && (element->y() < e->y()))
                        || (e->placeBelow() && !above && (element->y() > e->y()))) {
                        element = e;
                    }
                }
            }
            return element;
        }

    public:
        HarmonyList()
        {
            elements.clear();
            modified.clear();
        }

        void append(const Segment* s, Element* e)
        {
            elements[s].append(e);
        }

        qreal getReferenceHeight(bool above) const
        {
            // The reference height is the height of
            //    the lowest element if placed above
            // or
            //    the highest element if placed below.
            bool first { true };
            qreal ref { 0.0 };
            for (auto s : elements.keys()) {
                Element* e { getReferenceElement(s, above, true) };
                if (!e) {
                    continue;
                }
                if (e->placeAbove() && above) {
                    ref = first ? e->y() : qMin(ref, e->y());
                    first = false;
                } else if (e->placeBelow() && !above) {
                    ref = first ? e->y() : qMax(ref, e->y());
                    first = false;
                }
            }
            return ref;
        }

        bool align(bool above, qreal reference, qreal maxShift)
        {
            // Align the elements. If a segment contains multiple elements,
            // only the reference elements is used in the algorithm. All other
            // elements will remain their original placement with respect to
            // the reference element.
            bool moved { false };
            if (almostZero(reference)) {
                return moved;
            }

            for (auto s : elements.keys()) {
                QList<Element*> handled;
                Element* be = getReferenceElement(s, above, false);
                if (!be) {
                    // If there are only invisible elements, we have to use an invisible
                    // element for alignment reference.
                    be = getReferenceElement(s, above, true);
                }
                if (be && ((above && (be->y() < (reference + maxShift))) || ((!above && (be->y() > (reference - maxShift)))))) {
                    qreal shift = be->rypos();
                    be->rypos() = reference - be->ryoffset();
                    shift -= be->rypos();
                    for (Element* e : elements[s]) {
                        if ((above && e->placeBelow()) || (!above && e->placeAbove())) {
                            continue;
                        }
                        modified.append(e);
                        handled.append(e);
                        moved = true;
                        if (e != be) {
                            e->rypos() -= shift;
                        }
                    }
                    for (auto e : handled) {
                        elements[s].removeOne(e);
                    }
                }
            }
            return moved;
        }

        void addToSkyline(const System* system)
        {
            for (Element* e : qAsConst(modified)) {
                const Segment* s = toSegment(e->parent());
                const MeasureBase* m = toMeasureBase(s->parent());
                system->staff(e->staffIdx())->skyline().add(e->shape().translated(e->pos() + s->pos() + m->pos()));
                if (e->isFretDiagram()) {
                    FretDiagram* fd = toFretDiagram(e);
                    Harmony* h = fd->harmony();
                    if (h) {
                        system->staff(e->staffIdx())->skyline().add(h->shape().translated(h->pos() + fd->pos() + s->pos() + m->pos()));
                    } else {
                        system->staff(e->staffIdx())->skyline().add(fd->shape().translated(fd->pos() + s->pos() + m->pos()));
                    }
                }
            }
        }
    };

    if (almostZero(maxShiftAbove) && almostZero(maxShiftBelow)) {
        return;
    }

    // Collect all fret diagrams and chord symbol and store them per staff.
    // In the same pass, the maximum height is collected.
    QMap<int, HarmonyList> staves;
    for (const Segment* s : sl) {
        for (Element* e : s->annotations()) {
            if ((harmony && e->isHarmony()) || (!harmony && e->isFretDiagram())) {
                staves[e->staffIdx()].append(s, e);
            }
        }
    }

    for (int idx: staves.keys()) {
        // Align the objects.
        // Algorithm:
        //    - Find highest placed harmony/fretdiagram.
        //    - Align all harmony/fretdiagram objects placed between height and height-maxShiftAbove.
        //    - Repeat for all harmony/fretdiagram objects below heigt-maxShiftAbove.
        bool moved { true };
        int pass { 0 };
        while (moved && (pass++ < 10)) {
            moved = false;
            moved |= staves[idx].align(true, staves[idx].getReferenceHeight(true), maxShiftAbove);
            moved |= staves[idx].align(false, staves[idx].getReferenceHeight(false), maxShiftBelow);
        }

        // Add all aligned objects to the sky line.
        staves[idx].addToSkyline(system);
    }
}
