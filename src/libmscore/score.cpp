//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of class Score (partial).
*/

#include <assert.h>
#include <cmath>
#include <QBuffer>

#include "score.h"
#include "fermata.h"
#include "imageStore.h"
#include "key.h"
#include "sig.h"
#include "clef.h"
#include "tempo.h"
#include "measure.h"
#include "page.h"
#include "undo.h"
#include "system.h"
#include "select.h"
#include "segment.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "slur.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "tuplet.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "line.h"
#include "volta.h"
#include "measurerepeat.h"
#include "ottava.h"
#include "barline.h"
#include "box.h"
#include "utils.h"
#include "excerpt.h"
#include "stafftext.h"
#include "repeatlist.h"
#include "keysig.h"
#include "beam.h"
#include "stafftype.h"
#include "tempotext.h"
#include "articulation.h"
#include "revisions.h"
#include "tie.h"
#include "tiemap.h"
#include "layoutbreak.h"
#include "harmony.h"
#include "mscore.h"
#include "scoreOrder.h"

#include "bracket.h"
#include "audio.h"
#include "instrtemplate.h"
#include "sym.h"
#include "rehearsalmark.h"
#include "breath.h"
#include "instrchange.h"
#include "synthesizerstate.h"

namespace Ms {
MasterScore* gscore;                 ///< system score, used for palettes etc.
std::set<Score*> Score::validScores;

bool noSeq           = false;
bool noMidi          = false;
bool midiInputTrace  = false;
bool midiOutputTrace = false;

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
    fixupSystems();
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
    fixupSystems();
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
    fixupSystems();
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
    fixupSystems();
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
        nb->setSystem(ob->system());
    }
    foreach (Element* e, nb->el()) {
        e->setParent(nb);
    }
    fixupSystems();
}

//---------------------------------------------------------
//   fixupSystems
///   After modifying measures, make sure each measure
///   belongs to some system. This is to make sure the
///   score tree contains all the measures in some system.
//---------------------------------------------------------

void MeasureBaseList::fixupSystems()
{
    MeasureBase* m = _first;
    while (m != _last) {
        m = m->next();
        if (m->prev()->system() && !m->system()) {
            m->setSystem(m->prev()->system());
            if (m->isMeasure() && !toMeasure(m)->hasMMRest()) {
                m->system()->appendMeasure(m);
            }
        }
    }
}

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
    : ScoreElement(this), _headersText(MAX_HEADERS, nullptr), _footersText(MAX_FOOTERS, nullptr), _selection(this), _selectionFilter(this)
{
    Score::validScores.insert(this);
    _masterScore = 0;
    Layer l;
    l.name          = "default";
    l.tags          = 1;
    _layer.append(l);
    _layerTags[0]   = "default";

    _scoreFont = ScoreFont::fontFactory("Leland");

    _fileDivision           = MScore::division;
    _style  = MScore::defaultStyle();
//      accInfo = tr("No selection");     // ??
    accInfo = "No selection";
    _scoreOrder = nullptr;
}

Score::Score(MasterScore* parent, bool forcePartStyle /* = true */)
    : Score{}
{
    Score::validScores.insert(this);
    _masterScore = parent;
    if (MScore::defaultStyleForParts()) {
        _style = *MScore::defaultStyleForParts();
    } else {
        // inherit most style settings from parent
        _style = parent->style();

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
            _style.set(i, MScore::defaultStyle().value(i));
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
}

Score::Score(MasterScore* parent, const MStyle& s)
    : Score{parent}
{
    Score::validScores.insert(this);
    _style  = s;
}

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
{
    Score::validScores.erase(this);

    foreach (MuseScoreView* v, viewer) {
        v->removeScore();
    }
    // deselectAll();
    qDeleteAll(_systems);   // systems are layout-only objects so we delete
                            // them prior to measures.
    for (MeasureBase* m = _measures.first(); m;) {
        MeasureBase* nm = m->next();
        if (m->isMeasure() && toMeasure(m)->mmRest()) {
            delete toMeasure(m)->mmRest();
        }
        delete m;
        m = nm;
    }

    for (auto it = _spanner.cbegin(); it != _spanner.cend(); ++it) {
        delete it->second;
    }
    _spanner.clear();

    qDeleteAll(_parts);
    qDeleteAll(_staves);
//      qDeleteAll(_pages);         // TODO: check
    _masterScore = 0;

    imageStore.clearUnused();
}

//---------------------------------------------------------
//   Score::clone
//         To create excerpt clone to show when changing PageSettings
//         Use MasterScore::clone() instead
//---------------------------------------------------------

Score* Score::clone()
{
    // TODO: see comments reagrding setting version in corresponding code in 3.x branch
    // and also compare to MasterScore::clone()
    Excerpt* excerpt = new Excerpt(masterScore());
    excerpt->setTitle(title());

    for (Part* part : _parts) {
        excerpt->parts().append(part);

        for (int track = part->startTrack(); track < part->endTrack(); ++track) {
            excerpt->tracks().insert(track, track);
        }
    }

    masterScore()->initExcerpt(excerpt);
    masterScore()->removeExcerpt(excerpt);

    return excerpt->partScore();
}

//---------------------------------------------------------
//   Score::onElementDestruction
//    Ensure correct state of the score after destruction
//    of the element (e.g. remove invalid pointers etc.).
//---------------------------------------------------------

void Score::onElementDestruction(Element* e)
{
    Score* score = e->score();
    if (!score || Score::validScores.find(score) == Score::validScores.end()) {
        // No score or the score is already deleted
        return;
    }
    score->selection().remove(e);
    score->cmdState().unsetElement(e);
    for (MuseScoreView* v : qAsConst(score->viewer)) {
        v->onElementDestruction(e);
    }
}

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

void Score::addMeasure(MeasureBase* m, MeasureBase* pos)
{
    m->setNext(pos);
    _measures.add(m);
}

//---------------------------------------------------------
//    fixTicks
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

void Score::fixTicks()
{
    Fraction tick = Fraction(0,1);
    Measure* fm = firstMeasure();
    if (fm == 0) {
        return;
    }

    for (Staff* staff : qAsConst(_staves)) {
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
    // Now done in getNextMeasure(), do we keep?
    if (tempomap()->empty()) {
        tempomap()->setTempo(0, _defaultTempo);
    }
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
                setPause(startTick, mb->pause());
            }
        }

        // Add pauses from the end of the previous measure (at measure->tick()):
        for (Segment* s = measure->first(); s && s->tick() == startTick; s = s->prev1()) {
            if (!s->isBreathType()) {
                continue;
            }
            qreal length = 0.0;
            for (Element* e : s->elist()) {
                if (e && e->isBreath()) {
                    length = qMax(length, toBreath(e)->pause());
                }
            }
            if (length != 0.0) {
                setPause(startTick, length);
            }
        }
    }

    for (Segment& segment : measure->segments()) {
        if (segment.isBreathType()) {
            if (!isMaster()) {
                continue;
            }
            qreal length = 0.0;
            Fraction tick = segment.tick();
            // find longest pause
            for (int i = 0, n = ntracks(); i < n; ++i) {
                Element* e = segment.element(i);
                if (e && e->isBreath()) {
                    Breath* b = toBreath(e);
                    length = qMax(length, b->pause());
                }
            }
            if (length != 0.0) {
                setPause(tick, length);
            }
        } else if (segment.isTimeSigType()) {
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                TimeSig* ts = toTimeSig(segment.element(staffIdx * VOICES));
                if (ts) {
                    staff(staffIdx)->addTimeSig(ts);
                }
            }
        } else if (segment.isChordRestType()) {
            if (!isMaster()) {
                continue;
            }
            qreal stretch = 0.0;
            for (Element* e : segment.annotations()) {
                if (e->isFermata() && toFermata(e)->play()) {
                    stretch = qMax(stretch, toFermata(e)->timeStretch());
                } else if (e->isTempoText()) {
                    TempoText* tt = toTempoText(e);
                    if (tt->isRelative()) {
                        tt->updateRelative();
                    }
                    setTempo(tt->segment(), tt->tempo());
                }
            }
            if (stretch != 0.0 && stretch != 1.0) {
                qreal otempo = tempomap()->tempo(segment.tick().ticks());
                qreal ntempo = otempo / stretch;
                setTempo(segment.tick(), ntempo);
                Fraction etick = segment.tick() + segment.ticks() - Fraction(1, 480 * 4);
                auto e = tempomap()->find(etick.ticks());
                if (e == tempomap()->end()) {
                    setTempo(etick, otempo);
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

Measure* Score::pos2measure(const QPointF& p, int* rst, int* pitch, Segment** seg, QPointF* offset) const
{
    Measure* m = searchMeasure(p);
    if (m == 0) {
        return 0;
    }

    System* s = m->system();
    qreal y   = p.y() - s->canvasPos().y();

    const int i = s->searchStaff(y);

    // search for segment + offset
    QPointF pppp = p - m->canvasPos();
    int strack = i * VOICES;
    if (!staff(i)) {
        return 0;
    }
//      int etrack = staff(i)->part()->nstaves() * VOICES + strack;
    int etrack = VOICES + strack;

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
            *offset = pppp - QPointF(segment->x(), sstaff->bbox().y());
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

void Score::dragPosition(const QPointF& p, int* rst, Segment** seg, qreal spacingFactor) const
{
    const System* preferredSystem = (*seg) ? (*seg)->system() : nullptr;
    Measure* m = searchMeasure(p, preferredSystem, spacingFactor);
    if (m == 0 || m->isMMRest()) {
        return;
    }

    System* s = m->system();
    qreal y   = p.y() - s->canvasPos().y();

    const int i = s->searchStaff(y, *rst, spacingFactor);

    // search for segment + offset
    QPointF pppp = p - m->canvasPos();
    int strack = staff2track(i);
    if (!staff(i)) {
        return;
    }
    int etrack = staff2track(i + 1);

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
//   state
//---------------------------------------------------------

ScoreContentState Score::state() const
{
    return ScoreContentState(this, undoStack()->state());
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

//---------------------------------------------------------
//   setPlaylistDirty
//---------------------------------------------------------

void MasterScore::setPlaylistDirty()
{
    _playlistDirty = true;
    _repeatList->setScoreChanged();
    _repeatList2->setScoreChanged();
}

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell()
{
    for (int i = 0; i < nstaves(); ++i) {
        std::vector<Note*> notes;
        for (Segment* s = firstSegment(SegmentType::All); s; s = s->next1()) {
            int strack = i * VOICES;
            int etrack = strack + VOICES;
            for (int track = strack; track < etrack; ++track) {
                Element* e = s->element(track);
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

void Score::spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment)
{
    for (int i = startStaff; i < endStaff; ++i) {
        std::vector<Note*> notes;
        for (Segment* s = startSegment; s && s != endSegment; s = s->next()) {
            int strack = i * VOICES;
            int etrack = strack + VOICES;
            for (int track = strack; track < etrack; ++track) {
                Element* e = s->element(track);
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
    int staff      = n->staffIdx();
    int startTrack = staff * VOICES + n->voice() - 1;
    int endTrack   = 0;
    while (seg) {
        if (seg->segmentType() == SegmentType::ChordRest) {
            for (int track = startTrack; track >= endTrack; --track) {
                Element* e = seg->element(track);
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
    int staff      = n->staffIdx();
    int startTrack = staff * VOICES + n->voice() + 1;
    int endTrack   = staff * VOICES + VOICES;
    while (seg) {
        if (seg->segmentType() == SegmentType::ChordRest) {
            for (int track = startTrack; track < endTrack; ++track) {
                Element* e = seg->element(track);
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

    int opt = Ms::computeWindow(notes, 0, 7);
    note->setTpc(Ms::tpc(3, note->pitch(), opt));
}

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void Score::appendPart(Part* p)
{
    _parts.append(p);
}

//---------------------------------------------------------
//   searchPage
//    p is in canvas coordinates
//---------------------------------------------------------

Page* Score::searchPage(const QPointF& p) const
{
    for (Page* page : pages()) {
        QRectF r = page->bbox().translated(page->pos());
        if (r.contains(p)) {
            return page;
        }
    }
    return 0;
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

QList<System*> Score::searchSystem(const QPointF& pos, const System* preferredSystem, qreal spacingFactor,
                                   qreal preferredSpacingFactor) const
{
    QList<System*> systems;
    Page* page = searchPage(pos);
    if (page == 0) {
        return systems;
    }
    qreal y = pos.y() - page->pos().y();    // transform to page relative
    const QList<System*>* sl = &page->systems();
    qreal y2;
    int n = sl->size();
    for (int i = 0; i < n; ++i) {
        System* s = sl->at(i);
        System* ns = 0;                   // next system row
        int ii = i + 1;
        for (; ii < n; ++ii) {
            ns = sl->at(ii);
            if (ns->y() != s->y()) {
                break;
            }
        }
        if ((ii == n) || (ns == 0)) {
            y2 = page->height();
        } else {
            qreal currentSpacingFactor;
            qreal sy2 = s->y() + s->bbox().height();
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
            systems.append(s);
            for (int iii = i + 1; ii < n; ++iii) {
                if (sl->at(iii)->y() != s->y()) {
                    break;
                }
                systems.append(sl->at(iii));
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

Measure* Score::searchMeasure(const QPointF& p, const System* preferredSystem, qreal spacingFactor, qreal preferredSpacingFactor) const
{
    QList<System*> systems = searchSystem(p, preferredSystem, spacingFactor, preferredSpacingFactor);
    for (System* system : qAsConst(systems)) {
        qreal x = p.x() - system->canvasPos().x();
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

static Segment* getNextValidInputSegment(Segment* segment, int track, int voice)
{
    if (segment == nullptr) {
        return nullptr;
    }

    Q_ASSERT(segment->segmentType() == SegmentType::ChordRest);

    ChordRest* chordRest = nullptr;
    for (Segment* s1 = segment; s1; s1 = s1->prev(SegmentType::ChordRest)) {
        Element* element = s1->element(track + voice);
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

bool Score::getPosition(Position* pos, const QPointF& p, int voice) const
{
    System* preferredSystem = nullptr;
    int preferredStaffIdx = -1;
    const qreal spacingFactor = 0.5;
    const qreal preferredSpacingFactor = 0.75;
    if (noteEntryMode() && inputState().staffGroup() != StaffGroup::TAB) {
        // for non-tab staves, prefer the current system & staff
        // this makes it easier to add notes far above or below the staff
        // not helpful for tab since notes are not entered above or below
        Segment* seg = inputState().segment();
        if (seg) {
            preferredSystem = seg->system();
        }
        int track = inputState().track();
        if (track >= 0) {
            preferredStaffIdx = track >> 2;
        }
    }
    Measure* measure = searchMeasure(p, preferredSystem, spacingFactor, preferredSpacingFactor);
    if (measure == 0) {
        return false;
    }

    pos->fret = FRET_NONE;
    //
    //    search staff
    //
    pos->staffIdx      = 0;
    SysStaff* sstaff   = 0;
    System* system     = measure->system();
    qreal y           = p.y() - system->pagePos().y();
    for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
        Staff* st = staff(pos->staffIdx);
        if (!st->part()->show()) {
            continue;
        }
        qreal sy2;
        SysStaff* ss = system->staff(pos->staffIdx);
        if (!ss->show()) {
            continue;
        }
        int nidx = -1;
        SysStaff* nstaff = 0;

        // find next visible staff
        for (int i = pos->staffIdx + 1; i < nstaves(); ++i) {
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
                nidx = i;
            }
            break;
        }

        if (nstaff) {
            qreal currentSpacingFactor;
            if (pos->staffIdx == preferredStaffIdx) {
                currentSpacingFactor = preferredSpacingFactor;
            } else if (nidx == preferredStaffIdx) {
                currentSpacingFactor = 1.0 - preferredSpacingFactor;
            } else {
                currentSpacingFactor = spacingFactor;
            }
            qreal s1y2 = ss->bbox().bottom();
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
    QPointF pppp(p - measure->canvasPos());
    qreal x         = pppp.x();
    Segment* segment = 0;
    pos->segment     = 0;

    // int track = pos->staffIdx * VOICES + voice;
    int track = pos->staffIdx * VOICES;

    for (segment = measure->first(SegmentType::ChordRest); segment;) {
        segment = getNextValidInputSegment(segment, track, voice);
        if (segment == 0) {
            break;
        }
        Segment* ns = getNextValidInputSegment(segment->next(SegmentType::ChordRest), track, voice);

        qreal x1 = segment->x();
        qreal x2;
        qreal d;
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
    const qreal mag     = s->staffMag(tick);
    // in TABs, step from one string to another; in other staves, step on and between lines
    qreal lineDist = s->staffType(tick)->lineDistance().val() * (s->isTabStaff(measure->tick()) ? 1 : .5) * mag * spatium();

    const qreal yOff = sstaff->yOffset();  // Get system staff vertical offset (usually for 1-line staves)
    pos->line  = lrint((pppp.y() - sstaff->bbox().y() - yOff) / lineDist);
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
    pos->pos  = QPointF(x, y) + measure->canvasPos();
    return true;
}

//---------------------------------------------------------
//   checkHasMeasures
//---------------------------------------------------------

bool Score::checkHasMeasures() const
{
    Page* page = pages().isEmpty() ? 0 : pages().front();
    const QList<System*>* sl = page ? &page->systems() : 0;
    if (sl == 0 || sl->empty() || sl->front()->measures().empty()) {
        qDebug("first create measure, then repeat operation");
        return false;
    }
    return true;
}

#if 0
//---------------------------------------------------------
//   moveBracket
//    columns are counted from right to left
//---------------------------------------------------------

void Score::moveBracket(int staffIdx, int srcCol, int dstCol)
{
    for (System* system : systems()) {
        system->moveBracket(staffIdx, srcCol, dstCol);
    }
}

#endif

//---------------------------------------------------------
//   spatiumHasChanged
//---------------------------------------------------------

static void spatiumHasChanged(void* data, Element* e)
{
    qreal* val = (qreal*)data;
    e->spatiumChanged(val[0], val[1]);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Score::spatiumChanged(qreal oldValue, qreal newValue)
{
    qreal data[2];
    data[0] = oldValue;
    data[1] = newValue;
    scanElements(data, spatiumHasChanged, true);
    for (Staff* staff : qAsConst(_staves)) {
        staff->spatiumChanged(oldValue, newValue);
    }
    _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, newValue / SPATIUM20);
}

//---------------------------------------------------------
//   updateStyle
//---------------------------------------------------------

static void updateStyle(void*, Element* e)
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
    setLayoutAll();
}

//---------------------------------------------------------
//   getCreateMeasure
//    - return Measure for tick
//    - create new Measure(s) if there is no measure for
//      this tick
//---------------------------------------------------------

Measure* Score::getCreateMeasure(const Fraction& tick)
{
    Measure* last = lastMeasure();
    if (last == 0 || ((last->tick() + last->ticks()) <= tick)) {
        Fraction lastTick  = last ? (last->tick() + last->ticks()) : Fraction(0,1);
        while (tick >= lastTick) {
            Measure* m = new Measure(this);
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

void Score::addElement(Element* element)
{
    Element* parent = element->parent();
    element->triggerLayout();

//      qDebug("Score(%p) Element(%p)(%s) parent %p(%s)",
//         this, element, element->name(), parent, parent ? parent->name() : "");

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
        int n = b->elements().size();
        for (int i = 0; i < n; ++i) {
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
    case ElementType::PALM_MUTE:
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
        foreach (SpannerSegment* ss, o->spannerSegments()) {
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

    case ElementType::TEMPO_TEXT:
        fixTicks();           // rebuilds tempomap
        break;

    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* ic = toInstrumentChange(element);
        ic->part()->setInstrument(ic->instrument(), ic->segment()->tick());
#if 0
        int tickStart = ic->segment()->tick();
        auto i = ic->part()->instruments()->upper_bound(tickStart);
        int tickEnd;
        if (i == ic->part()->instruments()->end()) {
            tickEnd = -1;
        } else {
            tickEnd = i->first;
        }
        Interval oldV = ic->part()->instrument(tickStart)->transpose();
        ic->part()->setInstrument(ic->instrument(), tickStart);
        transpositionChanged(ic->part(), oldV, tickStart, tickEnd);
#endif
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

void Score::removeElement(Element* element)
{
    Element* parent = element->parent();
    element->triggerLayout();

//      qDebug("Score(%p) Element(%p)(%s) parent %p(%s)",
//         this, element, element->name(), parent, parent ? parent->name() : "");

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
            Q_ASSERT(lineMode() && (element->isVBox() || element->isTBox()));
            return;
        }

        Page* page = system->page();
        if (element->isBox() && system->measures().size() == 1) {
            auto i = std::find(page->systems().begin(), page->systems().end(), system);
            page->systems().erase(i);
            mb->setSystem(0);
            if (page->systems().isEmpty()) {
                // Remove this page, since it is now empty.
                // This involves renumbering and repositioning all subsequent pages.
                QPointF pos = page->pos();
                auto ii = std::find(pages().begin(), pages().end(), page);
                pages().erase(ii);
                while (ii != pages().end()) {
                    page = *ii;
                    page->setNo(page->no() - 1);
                    QPointF p = page->pos();
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
        element->setParent(0);
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
    case ElementType::PALM_MUTE:
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
    case ElementType::TEMPO_TEXT:
        fixTicks();           // rebuilds tempomap
        break;
    case ElementType::INSTRUMENT_CHANGE: {
        InstrumentChange* ic = toInstrumentChange(element);
        ic->part()->removeInstrument(ic->segment()->tick());
#if 0
        int tickStart = ic->segment()->tick();
        auto i = ic->part()->instruments()->upper_bound(tickStart);
        int tickEnd;
        if (i == ic->part()->instruments()->end()) {
            tickEnd = -1;
        } else {
            tickEnd = i->first;
        }
        Interval oldV = ic->part()->instrument(tickStart)->transpose();
        ic->part()->removeInstrument(tickStart);
        transpositionChanged(ic->part(), oldV, tickStart, tickEnd);
#endif
        addLayoutFlags(LayoutFlag::REBUILD_MIDI_MAPPING);
        cmdState()._instrumentsChanged = true;
    }
    break;

    case ElementType::TREMOLO:
    case ElementType::ARTICULATION:
    case ElementType::ARPEGGIO:
    {
        Element* cr = element->parent();
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
    return m ? m->endTick() : Fraction(0,1);
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

qreal Score::utick2utime(int tick) const
{
    return repeatList().utick2utime(tick);
}

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int Score::utime2utick(qreal utime) const
{
    return repeatList().utime2utick(utime);
}

//---------------------------------------------------------
//   setExpandRepeats
//---------------------------------------------------------

void MasterScore::setExpandRepeats(bool expand)
{
    if (_expandRepeats == expand) {
        return;
    }
    _expandRepeats = expand;
    setPlaylistDirty();
}

//---------------------------------------------------------
//   updateRepeatListTempo
///   needed for usage in Seq::processMessages
//---------------------------------------------------------

void MasterScore::updateRepeatListTempo()
{
    _repeatList->updateTempo();
    _repeatList2->updateTempo();
}

//---------------------------------------------------------
//   repeatList
//---------------------------------------------------------

const RepeatList& MasterScore::repeatList() const
{
    _repeatList->update(_expandRepeats);
    return *_repeatList;
}

//---------------------------------------------------------
//   repeatList2
//---------------------------------------------------------

const RepeatList& MasterScore::repeatList2() const
{
    _repeatList2->update(false);
    return *_repeatList2;
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

void Score::scanElementsInRange(void* data, void (* func)(void*, Element*), bool all)
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
    for (Element* e : _selection.elements()) {
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

    foreach (Element* e, _selection.elements()) {
        e->setSelected(true);
    }
}

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

Text* Score::getText(Tid tid)
{
    MeasureBase* m = first();
    if (m && m->isVBox()) {
        for (Element* e : m->el()) {
            if (e->isText() && toText(e)->tid() == tid) {
                return toText(e);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   metaTag
//---------------------------------------------------------

QString Score::metaTag(const QString& s) const
{
    if (_metaTags.contains(s)) {
        return _metaTags.value(s);
    }
    return _masterScore->_metaTags.value(s);
}

//---------------------------------------------------------
//   setMetaTag
//---------------------------------------------------------

void Score::setMetaTag(const QString& tag, const QString& val)
{
    _metaTags.insert(tag, val);
}

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void MasterScore::addExcerpt(Excerpt* ex)
{
    Score* score = ex->partScore();

    int nstaves { 1 }; // Initialise to 1 to force writing of the first part.
    for (Staff* s : score->staves()) {
        const LinkedElements* ls = s->links();
        if (ls == 0) {
            continue;
        }

        for (auto le : *ls) {
            if (le->score() != this) {
                continue;
            }

            // For instruments with multiple staves, every staff will point to the
            // same part. To prevent adding the same part several times to the excerpt,
            // add only the part of the first staff pointing to the part.
            Staff* ps = toStaff(le);
            if (!(--nstaves)) {
                ex->parts().append(ps->part());
                nstaves = ps->part()->nstaves();
            }
            break;
        }
    }

    if (ex->tracks().isEmpty()) {   // SHOULDN'T HAPPEN, protected in the UI, but it happens during read-in!!!
        QMultiMap<int, int> tracks;
        for (Staff* s : score->staves()) {
            const LinkedElements* ls = s->links();
            if (ls == 0) {
                continue;
            }
            for (auto le : *ls) {
                Staff* ps = toStaff(le);
                if (ps->primaryStaff()) {
                    for (int i = 0; i < VOICES; i++) {
                        tracks.insert(ps->idx() * VOICES + i % VOICES, s->idx() * VOICES + i % VOICES);
                    }
                    break;
                }
            }
        }
        ex->setTracks(tracks);
    }
    excerpts().append(ex);
    setExcerptsChanged(true);
}

//---------------------------------------------------------
//   removeExcerpt
//---------------------------------------------------------

void MasterScore::removeExcerpt(Excerpt* ex)
{
    if (excerpts().removeOne(ex)) {
        setExcerptsChanged(true);
        // delete ex;
    } else {
        qDebug("removeExcerpt:: ex not found");
    }
}

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

MasterScore* MasterScore::clone()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    XmlWriter xml(this, &buffer);
    xml.header();

    xml.stag("museScore version=\"" MSC_VERSION "\"");
    write(xml, false);
    xml.etag();

    buffer.close();

    XmlReader r(buffer.buffer());
    MasterScore* score = new MasterScore(style());
    score->read1(r, true);

    score->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
    score->doLayout();
    return score;
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
        qDebug("Score to append has %d parts and %d staves, but this score only has %d parts and %d staves.",
               score->parts().size(), score->staves().size(), parts().size(), staves().size());
        return false;
    }

    if (!last()) {
        qDebug("This score doesn't have any MeasureBase objects.");
        return false;
    }

    // apply Page/Section Breaks if desired
    if (addPageBreak) {
        if (!last()->pageBreak()) {
            last()->undoSetBreak(false, LayoutBreak::Type::LINE);       // remove line break if exists
            last()->undoSetBreak(true, LayoutBreak::Type::PAGE);        // apply page break
        }
    } else if (!last()->lineBreak() && !last()->pageBreak()) {
        last()->undoSetBreak(true, LayoutBreak::Type::LINE);
    }

    if (addSectionBreak && !last()->sectionBreak()) {
        last()->undoSetBreak(true, LayoutBreak::Type::SECTION);
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
        for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Fraction f;
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (int v = 0; v < VOICES; ++v) {
                    ChordRest* cr = toChordRest(s->element(staffIdx * VOICES + v));
                    if (cr == 0) {
                        continue;
                    }
                    f += cr->actualTicks();
                }
            }
            if (f.isZero()) {
                addRest(m->tick(), staffIdx * VOICES, TDuration(TDuration::DurationType::V_MEASURE), 0);
            }
        }
    }

    // at first added measure, check if we need to add Clef/Key/TimeSig
    //  this is needed if it was changed and needs to be changed back
    int n = nstaves();
    Fraction otick = fmb->tick(), ctick = tickOfAppend;
    for (int staffIdx = 0; staffIdx < n; ++staffIdx) {   // iterate over all staves
        int trackIdx = staff2track(staffIdx);     // idx of irst track on the staff
        Staff* staff = this->staff(staffIdx);
        Staff* ostaff = score->staff(staffIdx);

        // check if key signature needs to be changed
        if (ostaff->key(otick) != staff->key(ctick)) {
            Segment* ns = firstAppendedMeasure->undoGetSegment(SegmentType::KeySig, ctick);
            KeySigEvent nkse = KeySigEvent(ostaff->keySigEvent(otick));
            KeySig* nks = new KeySig(this);
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
        ns->setParent(0);
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

void Score::splitStaff(int staffIdx, int splitPoint)
{
//      qDebug("split staff %d point %d", staffIdx, splitPoint);

    //
    // create second staff
    //
    Staff* st = staff(staffIdx);
    Part* p  = st->part();
    Staff* ns = new Staff(this);
    ns->init(st);
    ns->setPart(p);
    // convert staffIdx from score-relative to part-relative
    int staffIdxPart = staffIdx - p->staff(0)->idx();
    undoInsertStaff(ns, staffIdxPart + 1, false);

    Clef* clef = new Clef(this);
    clef->setClefType(ClefType::F);
    clef->setTrack((staffIdx + 1) * VOICES);
    Segment* seg = firstMeasure()->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
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
    int strack = staffIdx * VOICES;
    int dtrack = (staffIdx + 1) * VOICES;

    // Keep track of ties to be reconnected.
    struct OldTie {
        Tie* tie;
        Note* nnote;
    };
    QMap<Note*, OldTie> oldTies;

    // Notes under the split point can be part of a tuplet, so keep track
    // of the tuplet mapping too!
    QMap<Tuplet*, Tuplet*> tupletMapping;
    Tuplet* tupletSrc[VOICES] = { };
    Tuplet* tupletDst[VOICES] = { };

    for (Segment* s = firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (int voice = 0; voice < VOICES; ++voice) {
            Element* e = s->element(strack + voice);

            if (!e) {
                continue;
            }
            if (toDurationElement(e)->tuplet()) {
                tupletSrc[voice] = toDurationElement(e)->tuplet();
                if (tupletMapping.contains(tupletSrc[voice])) {
                    tupletDst[voice] = tupletMapping[tupletSrc[voice]];
                } else {
                    tupletDst[voice] = new Tuplet(*tupletSrc[voice]);
                    tupletDst[voice]->setTrack(dtrack);
                    tupletMapping.insert(tupletSrc[voice], tupletDst[voice]);
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
                QList<Note*> removeNotes;
                for (Note* note : c->notes()) {
                    if (note->pitch() >= splitPoint) {
                        continue;
                    } else {
                        Chord* chord = toChord(s->element(dtrack + voice));
                        Q_ASSERT(!chord || (chord->isChord()));
                        if (!chord) {
                            chord = new Chord(*c);
                            qDeleteAll(chord->notes());
                            chord->notes().clear();
                            chord->setTuplet(tupletDst[voice]);
                            chord->setTrack(dtrack + voice);
                            undoAddElement(chord);
                        }
                        Note* nnote = new Note(*note);
                        if (note->tieFor()) {
                            // Save the note and the tie for processing later.
                            // Use the end note as index in the map so, when this is found
                            // we know the tie has to be recreated.
                            oldTies.insert(note->tieFor()->endNote(), OldTie { note->tieFor(), nnote });
                        }
                        nnote->setTrack(dtrack + voice);
                        chord->add(nnote);
                        nnote->updateLine();
                        removeNotes.append(note);
                        createRestDst = false;
                        lengthDst = chord->actualDurationType();

                        // Is the note the last note of a tie?
                        if (oldTies.contains(note)) {
                            // Yes! Create a tie between the new notes and remove the
                            // old tie.
                            Tie* tie = oldTies[note].tie->clone();
                            tie->setStartNote(oldTies[note].nnote);
                            tie->setEndNote(nnote);
                            tie->setTrack(nnote->track());
                            undoAddElement(tie);
                            undoRemoveElement(oldTies[note].tie);
                            oldTies.remove(note);
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
                                for (ScoreElement* ee : slur->linkList()) {
                                    Slur* lslur = toSlur(ee);
                                    lslur->setStartElement(0);
                                }
                            }
                            if (slur->endCR() == chord) {
                                slur->undoChangeProperty(Pid::SPANNER_TRACK2, slur->track2() + VOICES);
                                for (ScoreElement* ee : slur->linkList()) {
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

    int sidx   = staffIdx(part);
    int n      = part->nstaves();

    for (int i = 0; i < n; ++i) {
        cmdRemoveStaff(sidx);
    }

    undoRemovePart(part, sidx);
}

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, int idx)
{
    bool inserted = false;
    int staff = 0;
    for (QList<Part*>::iterator i = _parts.begin(); i != _parts.end(); ++i) {
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
}

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
{
    int index = _parts.indexOf(part);

    if (index == -1) {
        for (int i = 0; i < _parts.size(); ++i) {
            if (_parts[i]->id() == part->id()) {
                index = i;
                break;
            }
        }
    }

    _parts.removeAt(index);

    if (_excerpt) {
        for (Part* excerptPart : _excerpt->parts()) {
            if (excerptPart->id() != part->id()) {
                continue;
            }

            _excerpt->parts().removeOne(excerptPart);
            break;
        }
    }

    masterScore()->rebuildMidiMapping();
    setInstrumentsChanged(true);
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Score::insertStaff(Staff* staff, int ridx)
{
    staff->part()->insertStaff(staff, ridx);

    int idx = staffIdx(staff->part()) + ridx;
    _staves.insert(idx, staff);

    for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
        Spanner* s = i->second;
        if (s->systemFlag()) {
            continue;
        }
        if (s->staffIdx() >= idx) {
            int t = s->track() + VOICES;
            if (t >= ntracks()) {
                t = ntracks() - 1;
            }
            s->setTrack(t);
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setTrack(t);
            }
            if (s->track2() != -1) {
                t = s->track2() + VOICES;
                s->setTrack2(t < ntracks() ? t : s->track());
            }
        }
    }
#if 0
    for (Spanner* s : staff->score()->unmanagedSpanners()) {
        if (s->systemFlag()) {
            continue;
        }
        if (s->staffIdx() >= idx) {
            int t = s->track() + VOICES;
            s->setTrack(t < ntracks() ? t : ntracks() - 1);
            if (s->track2() != -1) {
                t = s->track2() + VOICES;
                s->setTrack2(t < ntracks() ? t : s->track());
            }
        }
    }
#endif
}

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
{
    int idx = staff->idx();
    for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
        Spanner* s = i->second;
        if (s->staffIdx() > idx) {
            int t = s->track() - VOICES;
            if (t < 0) {
                t = 0;
            }
            s->setTrack(t);
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setTrack(t);
            }
            if (s->track2() != -1) {
                t = s->track2() - VOICES;
                s->setTrack2(t >= 0 ? t : s->track());
            }
        }
    }
#if 0
    for (Spanner* s : staff->score()->unmanagedSpanners()) {
        if (s->staffIdx() > idx) {
            int t = s->track() - VOICES;
            s->setTrack(t >= 0 ? t : 0);
            if (s->track2() != -1) {
                t = s->track2() - VOICES;
                s->setTrack2(t >= 0 ? t : s->track());
            }
        }
    }
#endif
    _staves.removeAll(staff);
    staff->part()->removeStaff(staff);
}

//---------------------------------------------------------
//   adjustBracketsDel
//---------------------------------------------------------

void Score::adjustBracketsDel(int sidx, int eidx)
{
    for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        Staff* staff = _staves[staffIdx];
        for (BracketItem* bi : staff->brackets()) {
            int span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx)) {
                continue;
            }
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
                bi->undoChangeProperty(Pid::BRACKET_SPAN, span - (eidx - sidx));
            }
        }
#if 0 // TODO
        int span = staff->barLineSpan();
        if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
            int newSpan = span - (eidx - sidx) + 1;
            int lastSpannedStaffIdx = staffIdx + newSpan - 1;
            int tick = 0;
            undoChangeBarLineSpan(staff, newSpan, 0, (_staves[lastSpannedStaffIdx]->lines(0) - 1) * 2);
        }
#endif
    }
}

//---------------------------------------------------------
//   adjustBracketsIns
//---------------------------------------------------------

void Score::adjustBracketsIns(int sidx, int eidx)
{
    for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
        Staff* staff = _staves[staffIdx];
        for (BracketItem* bi : staff->brackets()) {
            int span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx)) {
                continue;
            }
            if ((sidx >= staffIdx) && (eidx < (staffIdx + span))) {
                bi->undoChangeProperty(Pid::BRACKET_SPAN, span + (eidx - sidx));
            }
        }
#if 0 // TODO
        int span = staff->barLineSpan();
        if ((sidx >= staffIdx) && (eidx < (staffIdx + span))) {
            int idx = staffIdx + span - 1;
            if (idx >= _staves.size()) {
                idx = _staves.size() - 1;
            }
            undoChangeBarLineSpan(staff, span, 0, (_staves[idx]->lines() - 1) * 2);
        }
#endif
    }
}

//---------------------------------------------------------
//   adjustKeySigs
//---------------------------------------------------------

void Score::adjustKeySigs(int sidx, int eidx, KeyList km)
{
    for (int staffIdx = sidx; staffIdx < eidx; ++staffIdx) {
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
            if (diff != 0 && !styleB(Sid::concertPitch) && !oKey.custom() && !oKey.isAtonal()) {
                nKey.setKey(transposeKey(nKey.key(), diff, staff->part()->preferSharpFlat()));
            }
            staff->setKey(tick, nKey);
            KeySig* keysig = new KeySig(this);
            keysig->setTrack(staffIdx * VOICES);
            keysig->setKeySigEvent(nKey);
            Segment* s = measure->getSegment(SegmentType::KeySig, tick);
            s->add(keysig);
        }
    }
}

//---------------------------------------------------------
//   cmdRemoveStaff
//---------------------------------------------------------

void Score::cmdRemoveStaff(int staffIdx)
{
    Staff* s = staff(staffIdx);
    adjustBracketsDel(staffIdx, staffIdx + 1);

    undoRemoveStaff(s);

    // remove linked staff and measures in linked staves in excerpts
    // unlink staff in the same score

    if (s->links()) {
        Staff* sameScoreLinkedStaff = 0;
        auto staves = s->links();
        for (auto le : *staves) {
            Staff* staff = toStaff(le);
            if (staff == s) {
                continue;
            }
            Score* lscore = staff->score();
            if (lscore != this) {
                lscore->undoRemoveStaff(staff);
                s->score()->undo(new Unlink(staff));
            } else {   // linked staff in the same score
                sameScoreLinkedStaff = staff;
            }
        }
        if (sameScoreLinkedStaff) {
//                  s->score()->undo(new Unlink(sameScoreLinkedStaff)); // once should be enough
            s->score()->undo(new Unlink(s));       // once should be enough
        }
    }
}

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Score::sortStaves(QList<int>& dst)
{
    qDeleteAll(systems());
    systems().clear();    //??
    _parts.clear();
    Part* curPart = 0;
    QList<Staff*> dl;
    QMap<int, int> trackMap;
    int track = 0;
    foreach (int idx, dst) {
        Staff* staff = _staves[idx];
        if (staff->part() != curPart) {
            curPart = staff->part();
            curPart->staves()->clear();
            _parts.push_back(curPart);
        }
        curPart->staves()->push_back(staff);
        dl.push_back(staff);
        for (int itrack = 0; itrack < VOICES; ++itrack) {
            trackMap.insert(idx * VOICES + itrack, track++);
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
        if (sp->systemFlag()) {
            continue;
        }
        int voice    = sp->voice();
        int staffIdx = sp->staffIdx();
        int idx = dst.indexOf(staffIdx);
        if (idx >= 0) {
            sp->setTrack(idx * VOICES + voice);
            if (sp->track2() != -1) {
                sp->setTrack2(idx * VOICES + (sp->track2() % VOICES));        // at least keep the voice...
            }
        }
    }
    setLayoutAll();
}

//---------------------------------------------------------
//   mapExcerptTracks
//---------------------------------------------------------

void Score::mapExcerptTracks(QList<int>& dst)
{
    for (Excerpt* e : excerpts()) {
        QMultiMap<int, int> tr = e->tracks();
        QMultiMap<int, int> tracks;
        for (QMap<int, int>::iterator it = tr.begin(); it != tr.end(); ++it) {
            int prvStaffIdx = it.key() / VOICES;
            int curStaffIdx = dst.indexOf(prvStaffIdx);
            int offset = (curStaffIdx - prvStaffIdx) * VOICES;
            tracks.insert(it.key() + offset, it.value());
        }
        e->tracks() = tracks;
    }
}

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag)
{
    undoChangeStyleVal(Sid::concertPitch, flag);         // change style flag

    for (Staff* staff : qAsConst(_staves)) {
        if (staff->staffType(Fraction(0,1))->group() == StaffGroup::PERCUSSION) {         // TODO
            continue;
        }
        // if this staff has no transposition, and no instrument changes, we can skip it
        Interval interval = staff->part()->instrument()->transpose(); //tick?
        if (interval.isZero() && staff->part()->instruments()->size() == 1) {
            continue;
        }
        if (!flag) {
            interval.flip();
        }

        int staffIdx   = staff->idx();
        int startTrack = staffIdx * VOICES;
        int endTrack   = startTrack + VOICES;

        transposeKeys(staffIdx, staffIdx + 1, Fraction(0,1), lastSegment()->tick(), interval, true, !flag);

        for (Segment* segment = firstSegment(SegmentType::ChordRest); segment; segment = segment->next1(SegmentType::ChordRest)) {
            interval = staff->part()->instrument(segment->tick())->transpose();
            if (!flag) {
                interval.flip();
            }
            for (Element* e : segment->annotations()) {
                if (!e->isHarmony() || (e->track() < startTrack) || (e->track() >= endTrack)) {
                    continue;
                }
                Harmony* h  = toHarmony(e);
                int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                for (ScoreElement* se : h->linkList()) {
                    // don't transpose all links
                    // just ones resulting from mmrests
                    Harmony* he = toHarmony(se);              // toHarmony() does not work as e is an ScoreElement
                    if (he->staff() == h->staff()) {
                        undoTransposeHarmony(he, rootTpc, baseTpc);
                    }
                }
                //realized harmony should be invalid after a transpose command
                Q_ASSERT(!h->realizedHarmony().valid());
            }
        }
    }
}

//---------------------------------------------------------
//   addAudioTrack
//---------------------------------------------------------

void Score::addAudioTrack()
{
    // TODO
}

//---------------------------------------------------------
//   padToggle
//---------------------------------------------------------

void Score::padToggle(Pad p, const EditData& ed)
{
    int oldDots = _is.duration().dots();
    switch (p) {
    case Pad::NOTE00:
        _is.setDuration(TDuration::DurationType::V_LONG);
        break;
    case Pad::NOTE0:
        _is.setDuration(TDuration::DurationType::V_BREVE);
        break;
    case Pad::NOTE1:
        _is.setDuration(TDuration::DurationType::V_WHOLE);
        break;
    case Pad::NOTE2:
        _is.setDuration(TDuration::DurationType::V_HALF);
        break;
    case Pad::NOTE4:
        _is.setDuration(TDuration::DurationType::V_QUARTER);
        break;
    case Pad::NOTE8:
        _is.setDuration(TDuration::DurationType::V_EIGHTH);
        break;
    case Pad::NOTE16:
        _is.setDuration(TDuration::DurationType::V_16TH);
        break;
    case Pad::NOTE32:
        _is.setDuration(TDuration::DurationType::V_32ND);
        break;
    case Pad::NOTE64:
        _is.setDuration(TDuration::DurationType::V_64TH);
        break;
    case Pad::NOTE128:
        _is.setDuration(TDuration::DurationType::V_128TH);
        break;
    case Pad::NOTE256:
        _is.setDuration(TDuration::DurationType::V_256TH);
        break;
    case Pad::NOTE512:
        _is.setDuration(TDuration::DurationType::V_512TH);
        break;
    case Pad::NOTE1024:
        _is.setDuration(TDuration::DurationType::V_1024TH);
        break;
    case Pad::REST:
        if (noteEntryMode()) {
            _is.setRest(!_is.rest());
            _is.setAccidentalType(AccidentalType::NONE);
        } else if (selection().isNone()) {
            ed.view->startNoteEntryMode();
            _is.setDuration(TDuration::DurationType::V_QUARTER);
            _is.setRest(true);
        } else {
            for (ChordRest* cr : getSelectedChordRests()) {
                if (!cr->isRest()) {
                    setNoteRest(cr->segment(), cr->track(), NoteVal(), cr->durationTypeTicks(), Direction::AUTO, false,
                                _is.articulationIds());
                }
            }
        }
        break;
    case Pad::DOT:
        if ((_is.duration().dots() == 1) || (_is.duration() == TDuration::DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(1);
        }
        break;
    case Pad::DOTDOT:
        if ((_is.duration().dots() == 2)
            || (_is.duration() == TDuration::DurationType::V_512TH)
            || (_is.duration() == TDuration::DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(2);
        }
        break;
    case Pad::DOT3:
        if ((_is.duration().dots() == 3)
            || (_is.duration() == TDuration::DurationType::V_256TH)
            || (_is.duration() == TDuration::DurationType::V_512TH)
            || (_is.duration() == TDuration::DurationType::V_1024TH)) {
            _is.setDots(0);
        } else {
            _is.setDots(3);
        }
        break;
    case Pad::DOT4:
        if ((_is.duration().dots() == 4)
            || (_is.duration() == TDuration::DurationType::V_128TH)
            || (_is.duration() == TDuration::DurationType::V_256TH)
            || (_is.duration() == TDuration::DurationType::V_512TH)
            || (_is.duration() == TDuration::DurationType::V_1024TH)) {
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
                    padToggle(Pad::DOTDOT, ed);
                    break;
                case 3:
                    padToggle(Pad::DOT3, ed);
                    break;
                case 4:
                    padToggle(Pad::DOT4, ed);
                    break;
                }

                NoteVal nval;
                Direction stemDirection = Direction::AUTO;
                if (_is.rest()) {
                    // Enter a rest
                    nval = NoteVal();
                } else {
                    Element* e = selection().element();
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
                            nval.pitch = stringData->getPitch(nval.string, nval.fret, s, tick);
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
        Element* e = selection().element();
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
            ed.view->startNoteEntryMode();
            deselect(e);
        }
    } else if (selection().isNone() && p != Pad::REST) {
        TDuration td = _is.duration();
        ed.view->startNoteEntryMode();
        _is.setDuration(td);
        _is.setAccidentalType(AccidentalType::NONE);
    } else {
        const auto elements = selection().uniqueElements();
        bool canAdjustLength = true;
        for (Element* e : elements) {
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

            for (const SymId& articulationId: _is.articulationIds()) {
                Articulation* na = new Articulation(cr->score());
                na->setSymId(articulationId);
                addArticulation(cr, na);
            }
        }
    }
}

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(Element* el)
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

void Score::select(Element* e, SelectType type, int staffIdx)
{
    // Move the playhead to the selected element's preferred play position.
    if (e) {
        const auto playTick = e->playTick();
        if (masterScore()->playPos() != playTick) {
            masterScore()->setPlayPos(playTick);
        }
    }

    if (MScore::debugMode) {
        qDebug("select element <%s> type %d(state %d) staff %d",
               e ? e->name() : "", int(type), int(selection().state()), e ? e->staffIdx() : -1);
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
    }
    setSelectionChanged(true);
}

//---------------------------------------------------------
//   selectSingle
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::selectSingle(Element* e, int staffIdx)
{
    SelState selState = _selection.state();
    deselectAll();
    if (e == 0) {
        selState = SelState::NONE;
        setUpdateAll();
    } else {
        if (e->isMeasure()) {
            select(e, SelectType::RANGE, staffIdx);
            return;
        }
        addRefresh(e->abbox());
        _selection.add(e);
        _is.setTrack(e->track());
        selState = SelState::LIST;
        if (e->type() == ElementType::NOTE) {
            e = e->parent();
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
    if (_layoutMode != LayoutMode::PAGE) {
        setLayoutMode(LayoutMode::PAGE);
        doLayout();
    }
}

//---------------------------------------------------------
//   selectAdd
//---------------------------------------------------------

void Score::selectAdd(Element* e)
{
    SelState selState = _selection.state();

    if (_selection.isRange()) {
        select(0, SelectType::SINGLE, 0);
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
    } else if (!_selection.elements().contains(e)) {
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

void Score::selectRange(Element* e, int staffIdx)
{
    int activeTrack = e->track();
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
            Element* oe = selection().element();
            if (oe->isNote() || oe->isChordRest()) {
                if (oe->isNote()) {
                    oe = oe->parent();
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
                int staffStart = staffIdx;
                int endStaff = staffIdx + 1;
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
            qDebug("SELECT_RANGE: measure: sel state %d", int(_selection.state()));
            return;
        }
    } else if (e->isNote() || e->isChordRest()) {
        if (e->isNote()) {
            e = e->parent();
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
            Element* oe = _selection.element();
            if (oe && (oe->isNote() || oe->isRest() || oe->isMMRest())) {
                if (oe->isNote()) {
                    oe = oe->parent();
                }
                ChordRest* ocr = toChordRest(oe);

                Segment* endSeg = tick2segmentMM(ocr->segment()->tick() + ocr->actualTicks(), true);
                if (!endSeg) {
                    endSeg = ocr->segment()->next();
                }

                _selection.setRange(ocr->segment(), endSeg, oe->staffIdx(), oe->staffIdx() + 1);
                _selection.extendRangeSelection(cr);
            } else {
                select(e, SelectType::SINGLE, 0);
                return;
            }
        } else if (_selection.isRange()) {
            _selection.extendRangeSelection(cr);
        } else {
            qDebug("sel state %d", int(_selection.state()));
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
        Element* selectedElement = _selection.element();
        if (selectedElement && e->type() == selectedElement->type()) {
            int idx1 = selectedElement->staffIdx();
            int idx2 = e->staffIdx();
            if (idx2 < idx1) {
                int temp = idx1;
                idx1 = idx2;
                idx2 = temp;
            }

            if (idx1 >= 0 && idx2 >= 0) {
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
                        const QList<Element*>& list = _selection.elements();
                        for (Element* el : list) {
                            if (el->track() != e->track()) {
                                _selection.remove(el);
                            }
                        }
                    }

                    return;
                }
            }
        }
        select(e, SelectType::SINGLE, staffIdx);
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

void Score::collectMatch(void* data, Element* e)
{
    ElementPattern* p = static_cast<ElementPattern*>(data);
    if (p->type != int(e->type())) {
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

    if ((p->staffStart != -1)
        && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx()))) {
        return;
    }

    if (p->voice != -1 && p->voice != e->voice()) {
        return;
    }

    if (p->system) {
        Element* ee = e;
        do {
            if (ee->type() == ElementType::SYSTEM) {
                if (p->system != ee) {
                    return;
                }
                break;
            }
            ee = ee->parent();
        } while (ee);
    }

    if (e->isRest() && p->durationTicks != Fraction(-1,1)) {
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

    p->el.append(e);
}

//---------------------------------------------------------
//   collectNoteMatch
//---------------------------------------------------------

void Score::collectNoteMatch(void* data, Element* e)
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
    if (p->string != STRING_NONE && p->string != n->string()) {
        return;
    }
    if (p->tpc != Tpc::TPC_INVALID && p->tpc != n->tpc()) {
        return;
    }
    if (p->notehead != NoteHead::Group::HEAD_INVALID && p->notehead != n->headGroup()) {
        return;
    }
    if (p->durationType.type() != TDuration::DurationType::V_INVALID && p->durationType != n->chord()->actualDurationType()) {
        return;
    }
    if (p->durationTicks != Fraction(-1,1) && p->durationTicks != n->chord()->actualTicks()) {
        return;
    }
    if ((p->staffStart != -1)
        && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx()))) {
        return;
    }
    if (p->voice != -1 && p->voice != e->voice()) {
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
    p->el.append(n);
}

//---------------------------------------------------------
//   selectSimilar
//---------------------------------------------------------

void Score::selectSimilar(Element* e, bool sameStaff)
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
    pattern.staffStart = sameStaff ? e->staffIdx() : -1;
    pattern.staffEnd = sameStaff ? e->staffIdx() + 1 : -1;
    pattern.voice   = -1;
    pattern.system  = 0;
    pattern.durationTicks = Fraction(-1,1);

    score->scanElements(&pattern, collectMatch);

    score->select(0, SelectType::SINGLE, 0);
    for (Element* ee : qAsConst(pattern.el)) {
        score->select(ee, SelectType::ADD, 0);
    }
}

//---------------------------------------------------------
//   selectSimilarInRange
//---------------------------------------------------------

void Score::selectSimilarInRange(Element* e)
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
    pattern.voice   = -1;
    pattern.system  = 0;
    pattern.durationTicks = Fraction(-1,1);

    score->scanElementsInRange(&pattern, collectMatch);

    score->select(0, SelectType::SINGLE, 0);
    for (Element* ee : qAsConst(pattern.el)) {
        score->select(ee, SelectType::ADD, 0);
    }
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

qreal Score::maxSystemDistance() const
{
    if (enableVerticalSpread()) {
        return styleP(Sid::maxSystemSpread);
    } else {
        return styleP(Sid::maxSystemDistance);
    }
}

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
{
    select(0, SelectType::SINGLE, 0);
    QRectF fr(bbox.normalized());
    foreach (Page* page, pages()) {
        QRectF pr(page->bbox());
        QRectF frr(fr.translated(-page->pos()));
        if (pr.right() < frr.left()) {
            continue;
        }
        if (pr.left() > frr.right()) {
            break;
        }

        QList<Element*> el = page->items(frr);
        for (int i = 0; i < el.size(); ++i) {
            Element* e = el.at(i);
            if (frr.contains(e->abbox())) {
                if (e->type() != ElementType::MEASURE && e->selectable()) {
                    select(e, SelectType::ADD, 0);
                }
            }
        }
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
    int startStaff        = 0x7fffffff;
    int endStaff          = 0;
    const ChordRest* endCR = 0;

    if (_selection.elements().empty()) {
        _selection.setState(SelState::NONE);
        setUpdateAll();
        return;
    }
    _selection.setState(SelState::LIST);

    foreach (const Element* e, _selection.elements()) {
        if (e->type() != ElementType::NOTE && e->type() != ElementType::REST) {
            continue;
        }
        ++noteRestCount;
        if (e->type() == ElementType::NOTE) {
            e = e->parent();
        }
        Segment* seg = static_cast<const ChordRest*>(e)->segment();
        if ((startSegment == 0) || (*seg < *startSegment)) {
            startSegment = seg;
        }
        if ((endSegment == 0) || (*seg > *endSegment)) {
            endSegment = seg;
            endCR = static_cast<const ChordRest*>(e);
        }
        int idx = e->staffIdx();
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

void Score::addLyrics(const Fraction& tick, int staffIdx, const QString& txt)
{
    if (txt.trimmed().isEmpty()) {
        return;
    }
    Measure* measure = tick2measure(tick);
    Segment* seg     = measure->findSegment(SegmentType::ChordRest, tick);
    if (seg == 0) {
        qDebug("no segment found for lyrics<%s> at tick %d",
               qPrintable(txt), tick.ticks());
        return;
    }

    bool lyricsAdded = false;
    for (int voice = 0; voice < VOICES; ++voice) {
        int track = staffIdx * VOICES + voice;
        ChordRest* cr = toChordRest(seg->element(track));
        if (cr) {
            Lyrics* l = new Lyrics(this);
            l->setXmlText(txt);
            l->setTrack(track);
            cr->add(l);
            lyricsAdded = true;
            break;
        }
    }
    if (!lyricsAdded) {
        qDebug("no chord/rest for lyrics<%s> at tick %d, staff %d",
               qPrintable(txt), tick.ticks(), staffIdx);
    }
}

//---------------------------------------------------------
//   setTempo
//    convenience function to access TempoMap
//---------------------------------------------------------

void Score::setTempo(Segment* segment, qreal tempo)
{
    setTempo(segment->tick(), tempo);
}

void Score::setTempo(const Fraction& tick, qreal tempo)
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
    resetTempoRange(Fraction(0,1), Fraction(std::numeric_limits<int>::max(), 1));
}

//---------------------------------------------------------
//   resetTempoRange
//    Reset tempo and timesig maps in the given range.
//    Start tick included, end tick excluded.
//---------------------------------------------------------

void Score::resetTempoRange(const Fraction& tick1, const Fraction& tick2)
{
    const bool zeroInRange = (tick1 <= Fraction(0,1) && tick2 > Fraction(0,1));
    tempomap()->clearRange(tick1.ticks(), tick2.ticks());
    if (zeroInRange) {
        tempomap()->setTempo(0, _defaultTempo);
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

void Score::setPause(const Fraction& tick, qreal seconds)
{
    tempomap()->setPause(tick.ticks(), seconds);
    setPlaylistDirty();
}

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Score::tempo(const Fraction& tick) const
{
    return tempomap()->tempo(tick.ticks());
}

//---------------------------------------------------------
//   loWidth
//---------------------------------------------------------

qreal Score::loWidth() const
{
    return styleD(Sid::pageWidth) * DPI;
}

//---------------------------------------------------------
//   loHeight
//---------------------------------------------------------

qreal Score::loHeight() const
{
    return styleD(Sid::pageHeight) * DPI;
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

QList<Score*> Score::scoreList()
{
    QList<Score*> scores;
    Score* root = masterScore();
    scores.append(root);
    for (const Excerpt* ex : root->excerpts()) {
        if (ex->partScore()) {
            scores.append(ex->partScore());
        }
    }
    return scores;
}

//---------------------------------------------------------
//   switchLayer
//---------------------------------------------------------

bool Score::switchLayer(const QString& s)
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
    int n = nstaves();
    for (int i = 0; i < t->nstaves(); ++i) {
        Staff* staff = new Staff(this);
        staff->setPart(part);
        StaffType* stt = staff->staffType(Fraction(0,1));
        stt->setLines(t->staffLines[i]);
        stt->setSmall(t->smallStaff[i]);
        if (i == 0) {
            staff->setBracketType(0, t->bracket[0]);
            staff->setBracketSpan(0, t->nstaves());
        }
        undoInsertStaff(staff, i);
    }
    part->staves()->front()->setBarLineSpan(part->nstaves());
    undoInsertPart(part, n);
    fixTicks();
    masterScore()->rebuildMidiMapping();
}

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n)
{
    for (int i = 0; i < n; ++i) {
        insertMeasure(ElementType::MEASURE, 0, false);
    }
}

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Score::addSpanner(Spanner* s)
{
    _spanner.addSpanner(s);
}

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeSpanner(Spanner* s)
{
    _spanner.removeSpanner(s);
}

//---------------------------------------------------------
//   isSpannerStartEnd
//    does is spanner start or end at tick position tick
//    for track ?
//---------------------------------------------------------

bool Score::isSpannerStartEnd(const Fraction& tick, int track) const
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
//   setPos
//---------------------------------------------------------

void MasterScore::setPos(POS pos, Fraction tick)
{
    if (tick < Fraction(0,1)) {
        tick = Fraction(0,1);
    }
    if (tick > lastMeasure()->endTick()) {
        // End Reverb may last longer than written notation, but cursor position should not
        tick = lastMeasure()->endTick();
    }

    _pos[int(pos)] = tick;
    // even though tick position might not have changed, layout might have
    // so we should update cursor here
    // however, we must be careful not to call setPos() again while handling posChanged, or recursion results
    for (Score* s : scoreList()) {
        emit s->posChanged(pos, unsigned(tick.ticks()));
    }
}

//---------------------------------------------------------
//   uniqueStaves
//---------------------------------------------------------

QList<int> Score::uniqueStaves() const
{
    QList<int> sl;

    for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        Staff* s = staff(staffIdx);
        if (s->links()) {
            bool alreadyInList = false;
            for (int idx : sl) {
                if (s->links()->contains(staff(idx))) {
                    alreadyInList = true;
                    break;
                }
            }
            if (alreadyInList) {
                continue;
            }
        }
        sl.append(staffIdx);
    }
    return sl;
}

//---------------------------------------------------------
//   findCR
//    find chord/rest <= tick in track
//---------------------------------------------------------

ChordRest* Score::findCR(Fraction tick, int track) const
{
    Measure* m = tick2measureMM(tick);
    if (!m) {
        qDebug("findCR: no measure for tick %d", tick.ticks());
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
        Element* el = ns->element(track);
        if (el && el->isRest() && toRest(el)->isGap()) {
            continue;
        } else if (el) {
            s = ns;
        }
    }

    if (!s) {
        return nullptr;
    }
    Element* el = s->element(track);
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

ChordRest* Score::findCRinStaff(const Fraction& tick, int staffIdx) const
{
    Fraction ptick = tick - Fraction::fromTicks(1);
    Measure* m = tick2measureMM(ptick);
    if (!m) {
        qDebug("findCRinStaff: no measure for tick %d", ptick.ticks());
        return 0;
    }
    // attach to first rest all spanner when mmRest
    if (m->isMMRest()) {
        ptick = m->tick();
    }

    Segment* s      = m->first(SegmentType::ChordRest);
    int strack      = staffIdx * VOICES;
    int etrack      = strack + VOICES;
    int actualTrack = strack;

    Fraction lastTick = Fraction(-1,1);
    for (Segment* ns = s;; ns = ns->next(SegmentType::ChordRest)) {
        if (ns == 0 || ns->tick() > ptick) {
            break;
        }
        // found a segment; now find longest cr on this staff that does not overlap tick
        for (int t = strack; t < etrack; ++t) {
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

Element* Score::cmdNextPrevSection(Element* el, bool dir) const
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
//    Get an Element* as Box or ChordRest depending on
//    MeasureBase
//---------------------------------------------------------

Element* Score::getScoreElementOfMeasureBase(MeasureBase* mb) const
{
    Element* el { nullptr };
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
//   setSoloMute
//   called once at opening file, adds soloMute marks
//---------------------------------------------------------

void MasterScore::setSoloMute()
{
    for (unsigned i = 0; i < _midiMapping.size(); i++) {
        Channel* b = _midiMapping[i].articulation();
        if (b->solo()) {
            b->setSoloMute(false);
            for (unsigned j = 0; j < _midiMapping.size(); j++) {
                Channel* a = _midiMapping[j].articulation();
                bool sameMidiMapping = _midiMapping[i].port() == _midiMapping[j].port()
                                       && _midiMapping[i].channel() == _midiMapping[j].channel();
                a->setSoloMute((i != j && !a->solo() && !sameMidiMapping));
                a->setSolo(i == j || a->solo() || sameMidiMapping);
            }
        }
    }
}

//---------------------------------------------------------
//   setImportedFilePath
//---------------------------------------------------------

void Score::setImportedFilePath(const QString& filePath)
{
    _importedFilePath = filePath;
}

//---------------------------------------------------------
//   nmeasure
//---------------------------------------------------------

int Score::nmeasures() const
{
    int n = 0;
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
        for (; m && m->isFullMeasureRest(); firstMeasure = m, m = m->prevMeasure()) {
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
        for (int i = 0; i < ntracks(); ++i) {
            ChordRest* cr = toChordRest(seg->element(i));
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
        for (Element* e : seg->annotations()) {
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
        for (int i = 0; i < ntracks(); ++i) {
            ChordRest* cr = toChordRest(seg->element(i));
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
        for (Element* e : seg->annotations()) {
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

QString Score::extractLyrics()
{
    QString result;
    masterScore()->setExpandRepeats(true);
    SegmentType st = SegmentType::ChordRest;
    for (int track = 0; track < ntracks(); track += VOICES) {
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
                    ChordRest* cr = toChordRest(seg->element(track));
                    if (!cr || cr->lyrics().empty()) {
                        continue;
                    }
                    if (cr->lyrics().size() > maxLyrics) {
                        maxLyrics = cr->lyrics().size();
                    }
                    if (playCount >= int(cr->lyrics().size())) {
                        continue;
                    }
                    Lyrics* l = cr->lyrics(playCount, Placement::BELOW);            // TODO: ABOVE
                    if (!l) {
                        continue;
                    }
                    found = true;
                    QString lyric = l->plainText().trimmed();
                    if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END) {
                        result += lyric + " ";
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
                        ChordRest* cr = toChordRest(seg->element(track));
                        if (!cr || cr->lyrics().empty()) {
                            continue;
                        }
                        if (cr->lyrics().size() > maxLyrics) {
                            maxLyrics = cr->lyrics().size();
                        }
                        if (lyricsNumber >= cr->lyrics().size()) {
                            continue;
                        }
                        Lyrics* l = cr->lyrics(lyricsNumber, Placement::BELOW);              // TODO
                        if (!l) {
                            continue;
                        }
                        found = true;
                        QString lyric = l->plainText().trimmed();
                        if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END) {
                            result += lyric + " ";
                        } else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics:: Syllabic::MIDDLE) {
                            result += lyric;
                        }
                    }
                }
            }
        }
        if (found) {
            result += "\n\n";
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
    for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        Staff* st = staff(staffIdx);
        constexpr Fraction t(0,1);
        Key key = st->key(t);
        if (st->staffType(t)->group() == StaffGroup::PERCUSSION || st->keySigEvent(t).custom() || st->keySigEvent(t).isAtonal()) {         // ignore percussion and custom / atonal key
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
    const RepeatSegment* rs = rl.last();
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
    const RepeatSegment* rs = rl.last();
    return lrint(utick2utime(rs->utick + rs->len()));
}

//---------------------------------------------------------
//   createRehearsalMarkText
//---------------------------------------------------------

QString Score::createRehearsalMarkText(RehearsalMark* current) const
{
    Fraction tick = current->segment()->tick();
    RehearsalMark* before = 0;
    RehearsalMark* after = 0;
    for (Segment* s = firstSegment(SegmentType::All); s; s = s->next1()) {
        for (Element* e : s->annotations()) {
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
    QString s = "A";
    QString s1 = before ? before->xmlText() : "";
    QString s2 = after ? after->xmlText() : "";
    if (s1.isEmpty()) {
        return s;
    }
    s = nextRehearsalMarkText(before, current);       // try to sequence
    if (s == current->xmlText()) {
        // no sequence detected (or current happens to be correct)
        return s;
    } else if (s == s2) {
        // next in sequence already present
        if (s1[0].isLetter()) {
            if (s1.size() == 2) {
                s = s1[0] + QChar::fromLatin1(s1[1].toLatin1() + 1);          // BB, BC, CC
            } else {
                s = s1 + QChar::fromLatin1('1');                              // B, B1, C
            }
        } else {
            s = s1 + QChar::fromLatin1('A');                                  // 2, 2A, 3
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

QString Score::nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const
{
    QString previousText = previous->xmlText();
    QString fallback = current ? current->xmlText() : previousText + "'";

    if (previousText.length() == 1 && previousText[0].isLetter()) {
        // single letter sequence
        if (previousText == "Z") {
            return "AA";
        } else if (previousText == "z") {
            return "aa";
        } else {
            return QChar::fromLatin1(previousText[0].toLatin1() + 1);
        }
    } else if (previousText.length() == 2 && previousText[0].isLetter() && previousText[1].isLetter()) {
        // double letter sequence
        if (previousText[0] == previousText[1]) {
            // repeated letter sequence
            if (previousText.toUpper() != "ZZ") {
                QString c = QChar::fromLatin1(previousText[0].toLatin1() + 1);
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
            return QString("%1").arg(n);
        } else {
            // use number sequence
            n = previousText.toInt() + 1;
            return QString("%1").arg(n);
        }
    }
}

//---------------------------------------------------------
//   changeSelectedNotesVoice
//    moves selected notes into specified voice if possible
//---------------------------------------------------------

void Score::changeSelectedNotesVoice(int voice)
{
    QList<Element*> el;
    QList<Element*> oel = selection().elements();       // make copy
    for (Element* e : oel) {
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
            int dstTrack     = chord->staffIdx() * VOICES + voice;
            ChordRest* dstCR = toChordRest(s->element(dstTrack));
            Chord* dstChord  = nullptr;

            if (excerpt() && excerpt()->tracks().key(dstTrack, -1) == -1) {
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
                dstChord = new Chord(this);
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
                    dstChord = new Chord(this);
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
                Note* newNote = new Note(*note);
                newNote->setSelected(false);
                newNote->setParent(dstChord);
                undoAddElement(newNote);
                el.append(newNote);
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
                    Rest* r = new Rest(this);
                    r->setTrack(chord->track());
                    r->setDurationType(chord->durationType());
                    r->setTicks(chord->ticks());
                    r->setTuplet(chord->tuplet());
                    r->setParent(s);
                    // if there were grace notes, move them
                    while (!chord->graceNotes().empty()) {
                        Chord* gc = chord->graceNotes().first();
                        Chord* ngc = new Chord(*gc);
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
    for (Element* e : el) {
        select(e, SelectType::ADD, -1);
    }
    setLayoutAll();
}

#if 0
//---------------------------------------------------------
//   cropPage - crop a single page score to the content
///    margins will be applied on the 4 sides
//---------------------------------------------------------

void Score::cropPage(qreal margins)
{
    if (npages() == 1) {
        Page* page = pages()[0];
        if (page) {
            QRectF ttbox = page->tbbox();

            qreal margin = margins / INCH;
            f.setSize(QSizeF((ttbox.width() / DPI) + 2 * margin, (ttbox.height() / DPI) + 2 * margin));

            qreal offset = curFormat->oddLeftMargin() - ttbox.x() / DPI;
            if (offset < 0) {
                offset = 0.0;
            }
            f.setOddLeftMargin(margin + offset);
            f.setEvenLeftMargin(margin + offset);
            f.setOddBottomMargin(margin);
            f.setOddTopMargin(margin);
            f.setEvenBottomMargin(margin);
            f.setEvenTopMargin(margin);

            undoChangePageFormat(&f, spatium(), pageNumberOffset());
        }
    }
}

#endif

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Score::getProperty(Pid /*id*/) const
{
    qDebug("Score::getProperty: unhandled id");
    return QVariant();
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Score::setProperty(Pid /*id*/, const QVariant& /*v*/)
{
    qDebug("Score::setProperty: unhandled id");
    setLayoutAll();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Score::propertyDefault(Pid /*id*/) const
{
    return QVariant();
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

QString Score::getTextStyleUserName(Tid tid)
{
    QString name = "";
    if (int(tid) >= int(Tid::USER1) && int(tid) <= int(Tid::USER12)) {
        int idx = int(tid) - int(Tid::USER1);
        Sid sid[] = { Sid::user1Name, Sid::user2Name, Sid::user3Name, Sid::user4Name, Sid::user5Name, Sid::user6Name,
                      Sid::user7Name, Sid::user8Name, Sid::user9Name, Sid::user10Name, Sid::user11Name, Sid::user12Name };
        name = styleSt(sid[idx]);
    }
    if (name == "") {
        name = textStyleUserName(tid);
    }
    return name;
}

//---------------------------------------------------------
//   MasterScore
//---------------------------------------------------------

MasterScore::MasterScore()
    : Score()
{
    _tempomap    = new TempoMap;
    _sigmap      = new TimeSigMap();
    _repeatList  = new RepeatList(this);
    _repeatList2 = new RepeatList(this);
    _revisions   = new Revisions;
    setMasterScore(this);

    _pos[int(POS::CURRENT)] = Fraction(0,1);
    _pos[int(POS::LEFT)]    = Fraction(0,1);
    _pos[int(POS::RIGHT)]   = Fraction(0,1);

#if defined(Q_OS_WIN)
    metaTags().insert("platform", "Microsoft Windows");
#elif defined(Q_OS_MAC)
    metaTags().insert("platform", "Apple Macintosh");
#elif defined(Q_OS_LINUX)
    metaTags().insert("platform", "Linux");
#else
    metaTags().insert("platform", "Unknown");
#endif
    metaTags().insert("movementNumber", "");
    metaTags().insert("movementTitle", "");
    metaTags().insert("workNumber", "");
    metaTags().insert("workTitle", "");
    metaTags().insert("arranger", "");
    metaTags().insert("composer", "");
    metaTags().insert("lyricist", "");
    metaTags().insert("poet", "");
    metaTags().insert("translator", "");
    metaTags().insert("source", "");
    metaTags().insert("copyright", "");
    metaTags().insert("creationDate", QDate::currentDate().toString(Qt::ISODate));
}

MasterScore::MasterScore(const MStyle& s)
    : MasterScore{}
{
    _movements = new Movements;
    _movements->push_back(this);
    setStyle(s);
}

MasterScore::~MasterScore()
{
    delete _revisions;
    delete _repeatList;
    delete _repeatList2;
    delete _sigmap;
    delete _tempomap;
    qDeleteAll(_excerpts);
}

//---------------------------------------------------------
//   setMovements
//---------------------------------------------------------

void MasterScore::setMovements(Movements* m)
{
    _movements = m;
    if (_movements) {
        _movements->push_back(this);
    }
}

//---------------------------------------------------------
//   isSavable
//---------------------------------------------------------

bool MasterScore::isSavable() const
{
    // TODO: check if file can be created if it does not exist
    return fileInfo()->isWritable() || !fileInfo()->exists();
}

//---------------------------------------------------------
//   setTempomap
//---------------------------------------------------------

void MasterScore::setTempomap(TempoMap* tm)
{
    delete _tempomap;
    _tempomap = tm;
}

//---------------------------------------------------------
//   removeOmr
//---------------------------------------------------------

void MasterScore::removeOmr()
{
    _showOmr = false;
    _omr = 0;
}

//---------------------------------------------------------
//   setName
//---------------------------------------------------------

void MasterScore::setName(const QString& ss)
{
    QString s(ss);
    s.replace('/', '_');      // for sanity
    if (!(s.endsWith(".mscz") || s.endsWith(".mscx"))) {
        s += ".mscz";
    }
    info.setFile(s);
}

//---------------------------------------------------------
//   title
//---------------------------------------------------------

QString MasterScore::title() const
{
    return fileInfo()->completeBaseName();
}

QString Score::title() const
{
    return _excerpt->title();
}

//---------------------------------------------------------
//   addRefresh
//---------------------------------------------------------

void Score::addRefresh(const QRectF& r)
{
    _updateState.refresh |= r;
    cmdState().setUpdateMode(UpdateMode::Update);
}

//---------------------------------------------------------
//   staffIdx
//
///  Return index for the first staff of \a part.
//---------------------------------------------------------

int Score::staffIdx(const Part* part) const
{
    int idx = 0;
    for (Part* p : _parts) {
        if (p == part) {
            break;
        }
        idx += p->nstaves();
    }
    return idx;
}

Staff* Score::staff(const QString& staffId) const
{
    for (Staff* staff : _staves) {
        if (staff->id() == staffId) {
            return staff;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   setUpdateAll
//---------------------------------------------------------

void MasterScore::setUpdateAll()
{
    _cmdState.setUpdateMode(UpdateMode::UpdateAll);
}

//---------------------------------------------------------
//   setLayoutAll
//---------------------------------------------------------

void MasterScore::setLayoutAll(int staff, const Element* e)
{
    _cmdState.setTick(Fraction(0,1));
    _cmdState.setTick(measures()->last() ? measures()->last()->endTick() : Fraction(0,1));

    if (e && e->score() == this) {
        // TODO: map staff number properly
        const int startStaff = staff == -1 ? 0 : staff;
        const int endStaff = staff == -1 ? (nstaves() - 1) : staff;
        _cmdState.setStaff(startStaff);
        _cmdState.setStaff(endStaff);

        _cmdState.setElement(e);
    }
}

//---------------------------------------------------------
//   setLayout
//---------------------------------------------------------

void MasterScore::setLayout(const Fraction& t, int staff, const Element* e)
{
    if (t >= Fraction(0,1)) {
        _cmdState.setTick(t);
    }

    if (e && e->score() == this) {
        // TODO: map staff number properly
        _cmdState.setStaff(staff);
        _cmdState.setElement(e);
    }
}

void MasterScore::setLayout(const Fraction& tick1, const Fraction& tick2, int staff1, int staff2, const Element* e)
{
    if (tick1 >= Fraction(0,1)) {
        _cmdState.setTick(tick1);
    }
    if (tick2 >= Fraction(0,1)) {
        _cmdState.setTick(tick2);
    }

    if (e && e->score() == this) {
        // TODO: map staff number properly
        _cmdState.setStaff(staff1);
        _cmdState.setStaff(staff2);

        _cmdState.setElement(e);
    }
}

//---------------------------------------------------------
//   setPlaybackScore
//---------------------------------------------------------

void MasterScore::setPlaybackScore(Score* score)
{
    if (_playbackScore == score) {
        return;
    }

    _playbackScore = score;
    _playbackSettingsLinks.clear();

    if (!_playbackScore) {
        return;
    }

    for (MidiMapping& mm : _midiMapping) {
        mm.articulation()->setSoloMute(true);
    }
    for (Part* part : score->parts()) {
        for (auto& i : *part->instruments()) {
            Instrument* instr = i.second;
            for (Channel* ch : instr->channel()) {
                Channel* pChannel = playbackChannel(ch);
                Q_ASSERT(pChannel);
                if (!pChannel) {
                    continue;
                }
                _playbackSettingsLinks.emplace_back(pChannel, ch, /* excerpt */ true);
            }
        }
    }
}

//---------------------------------------------------------
//   updateExpressive
//    change patches to their expressive equivalent or vica versa, if possible
//    This works only with MuseScore general soundfont
//
//    The first version of the function decides whether to make patches expressive
//    or not, based on the synth settings. The second will switch patches based on
//    the value of the expressive parameter.
//---------------------------------------------------------

void MasterScore::updateExpressive(Synthesizer* synth)
{
    SynthesizerState s = synthesizerState();
    SynthesizerGroup g = s.group("master");

    int method = 1;
    for (const IdValue& idVal : g) {
        if (idVal.id == 4) {
            method = idVal.data.toInt();
            break;
        }
    }

    updateExpressive(synth, (method != 0));
}

void MasterScore::updateExpressive(Synthesizer* synth, bool expressive, bool force /* = false */)
{
    if (!synth) {
        return;
    }

    if (!force) {
        SynthesizerState s = synthesizerState();
        SynthesizerGroup g = s.group("master");

        for (const IdValue& idVal : g) {
            if (idVal.id == 4) {
                int method = idVal.data.toInt();
                if (expressive == (method == 0)) {
                    return;           // method and expression change don't match, so don't switch}
                }
            }
        }
    }

    for (Part* p : parts()) {
        const InstrumentList* il = p->instruments();
        for (auto it = il->begin(); it != il->end(); it++) {
            Instrument* i = it->second;
            i->switchExpressive(this, synth, expressive, force);
        }
    }
}

//---------------------------------------------------------
//   rebuildAndUpdateExpressive
//    implicitly rebuild midi mappings as well. Should be preferred over
//    just updateExpressive, in most cases.
//---------------------------------------------------------

void MasterScore::rebuildAndUpdateExpressive(Synthesizer* synth)
{
    // Rebuild midi mappings to make sure we have playback channels
    rebuildMidiMapping();

    updateExpressive(synth);

    // Rebuild midi mappings again to be safe
    rebuildMidiMapping();
}

//---------------------------------------------------------
//   isTopScore
//---------------------------------------------------------

bool Score::isTopScore() const
{
    return !(isMaster() && static_cast<const MasterScore*>(this)->prev());
}

//---------------------------------------------------------
//   Movements
//---------------------------------------------------------

Movements::Movements()
    : std::vector<MasterScore*>()
{
    _undo = new UndoStack();
}

Movements::~Movements()
{
    qDeleteAll(_pages);
    delete _undo;
}

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
