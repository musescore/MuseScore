//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "notationinteraction.h"

#include "log.h"
#include <memory>
#include <QRectF>

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/shadownote.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/rest.h"
#include "libmscore/slur.h"
#include "libmscore/system.h"
#include "libmscore/chord.h"
#include "libmscore/elementgroup.h"

#include "notation.h"
#include "scorecallbacks.h"

using namespace mu::domain::notation;
using namespace Ms;

NotationInteraction::NotationInteraction(Notation* notation)
    : m_notation(notation)
{
    m_scoreCallbacks = new ScoreCallbacks();

    m_inputState = new NotationInputState(notation);
    m_shadowNote = new ShadowNote(notation->score());
    m_shadowNote->setVisible(false);

    m_selection = new NotationSelection(notation);

    m_dragData.editData.view = new ScoreCallbacks();
}

NotationInteraction::~NotationInteraction()
{
    delete m_inputState;
    delete m_selection;
    delete m_shadowNote;
    delete m_scoreCallbacks;
}

Ms::Score* NotationInteraction::score() const
{
    return m_notation->score();
}

void NotationInteraction::paint(QPainter* p)
{
    m_shadowNote->draw(p);
}

void NotationInteraction::startNoteEntry()
{
    //! NOTE Coped from `void ScoreView::startNoteEntry()`
    Ms::InputState& is = score()->inputState();
    is.setSegment(0);

    if (score()->selection().isNone()) {
        selectFirstTopLeftOrLast();
    }

    //! TODO Find out what does and why.
    Element* el = score()->selection().element();
    if (!el) {
        el = score()->selection().firstChordRest();
    }

    if (el == nullptr
        || (el->type() != ElementType::CHORD && el->type() != ElementType::REST && el->type() != ElementType::NOTE)) {
        // if no note/rest is selected, start with voice 0
        int track = is.track() == -1 ? 0 : (is.track() / VOICES) * VOICES;
        // try to find an appropriate measure to start in
        Fraction tick = el ? el->tick() : Fraction(0,1);
        el = score()->searchNote(tick, track);
        if (!el) {
            el = score()->searchNote(Fraction(0,1), track);
        }
    }

    if (!el) {
        return;
    }

    if (el->type() == ElementType::CHORD) {
        Chord* c = static_cast<Chord*>(el);
        Note* note = c->selectedNote();
        if (note == 0) {
            note = c->upNote();
        }
        el = note;
    }
    //! ---

    TDuration d(is.duration());
    if (!d.isValid() || d.isZero() || d.type() == TDuration::DurationType::V_MEASURE) {
        is.setDuration(TDuration(TDuration::DurationType::V_QUARTER));
    }
    is.setAccidentalType(AccidentalType::NONE);

    select(el, SelectType::SINGLE, 0);

    is.setRest(false);
    is.setNoteEntryMode(true);

    //! TODO Find out why.
    score()->setUpdateAll();
    score()->update();
    //! ---

    Staff* staff = score()->staff(is.track() / VOICES);
    switch (staff->staffType(is.tick())->group()) {
    case StaffGroup::STANDARD:
        break;
    case StaffGroup::TAB: {
        int strg = 0;                           // assume topmost string as current string
        // if entering note entry with a note selected and the note has a string
        // set InputState::_string to note physical string
        if (el->type() == ElementType::NOTE) {
            strg = (static_cast<Note*>(el))->string();
        }
        is.setString(strg);
        break;
    }
    case StaffGroup::PERCUSSION:
        break;
    }

    m_inputStateChanged.notify();
}

void NotationInteraction::selectFirstTopLeftOrLast()
{
    // choose page in current view (favor top left quadrant if possible)
    // select first (top/left) chordrest of that page in current view
    // or, CR at last selected position if that is in view
    Page* page = nullptr;
    QSizeF viewSize = m_notation->viewSize();
    QList<QPointF> points;
    points.append(QPoint(viewSize.width() * 0.25, viewSize.height() * 0.25));
    points.append(QPoint(0.0, 0.0));
    points.append(QPoint(0.0, viewSize.height()));
    points.append(QPoint(viewSize.width(), 0.0));
    points.append(QPoint(viewSize.width(), viewSize.height()));
    for (const QPointF& point : points) {
        page = point2page(point);
        if (page) {
            break;
        }
    }

    if (page) {
        ChordRest* topLeft = nullptr;
        qreal tlY = 0.0;
        Fraction tlTick = Fraction(0,1);
        QRectF viewRect  = QRectF(0.0, 0.0, viewSize.width(), viewSize.height());
        QRectF pageRect  = page->bbox().translated(page->x(), page->y());
        QRectF intersect = viewRect & pageRect;
        intersect.translate(-page->x(), -page->y());
        QList<Element*> el = page->items(intersect);
        ChordRest* lastSelected = score()->selection().currentCR();
        for (Element* e : el) {
            // loop through visible elements
            // looking for the CR in voice 1 with earliest tick and highest staff position
            // but stop we find the last selected CR
            ElementType et = e->type();
            if (et == ElementType::NOTE || et == ElementType::REST) {
                if (e->voice()) {
                    continue;
                }
                ChordRest* cr;
                if (et == ElementType::NOTE) {
                    cr = static_cast<ChordRest*>(e->parent());
                    if (!cr) {
                        continue;
                    }
                } else {
                    cr = static_cast<ChordRest*>(e);
                }
                if (cr == lastSelected) {
                    topLeft = cr;
                    break;
                }
                // compare ticks rather than x position
                // to make sure we favor earlier rather than later systems
                // even though later system might have note farther to left
                Fraction crTick = Fraction(0,1);
                if (cr->segment()) {
                    crTick = cr->segment()->tick();
                } else {
                    continue;
                }
                // compare staff Y position rather than note Y position
                // to be sure we do not reject earliest note
                // just because it is lower in pitch than subsequent notes
                qreal crY = 0.0;
                if (cr->measure() && cr->measure()->system()) {
                    crY = cr->measure()->system()->staffYpage(cr->staffIdx());
                } else {
                    continue;
                }
                if (topLeft) {
                    if (crTick <= tlTick && crY <= tlY) {
                        topLeft = cr;
                        tlTick = crTick;
                        tlY = crY;
                    }
                } else {
                    topLeft = cr;
                    tlTick = crTick;
                    tlY = crY;
                }
            }
        }

        if (topLeft) {
            select(topLeft, SelectType::SINGLE);
        }
    }
}

void NotationInteraction::endNoteEntry()
{
    InputState& is = score()->inputState();
    is.setNoteEntryMode(false);
    if (is.slur()) {
        const std::vector<SpannerSegment*>& el = is.slur()->spannerSegments();
        if (!el.empty()) {
            el.front()->setSelected(false);
        }
        is.setSlur(0);
    }

    hideShadowNote();
    m_inputStateChanged.notify();
}

void NotationInteraction::padNote(const Pad& pad)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;
    score()->startCmd();
    score()->padToggle(pad, ed);
    score()->endCmd();
    m_inputStateChanged.notify();
}

void NotationInteraction::putNote(const QPointF& pos, bool replace, bool insert)
{
    score()->startCmd();
    score()->putNote(pos, replace, insert);
    score()->endCmd();
    m_notation->notifyAboutNotationChanged();
}

INotationInputState* NotationInteraction::inputState() const
{
    return m_inputState;
}

mu::async::Notification NotationInteraction::inputStateChanged() const
{
    return m_inputStateChanged;
}

void NotationInteraction::showShadowNote(const QPointF& p)
{
    //! NOTE This method coped from ScoreView::setShadowNote

    const InputState& is = score()->inputState();
    Position pos;
    if (!score()->getPosition(&pos, p, is.voice())) {
        m_shadowNote->setVisible(false);
        return;
    }

    // in any empty measure, pos will be right next to barline
    // so pad this by barNoteDistance
    qreal mag = score()->staff(pos.staffIdx)->staffMag(Fraction(0,1));
    qreal relX = pos.pos.x() - pos.segment->measure()->canvasPos().x();
    pos.pos.rx() -= qMin(relX - score()->styleP(Sid::barNoteDistance) * mag, 0.0);

    m_shadowNote->setVisible(true);

    Staff* staff = score()->staff(pos.staffIdx);
    m_shadowNote->setMag(staff->staffMag(Fraction(0,1)));
    const Instrument* instr = staff->part()->instrument();
    NoteHead::Group noteheadGroup = NoteHead::Group::HEAD_NORMAL;
    int line = pos.line;
    NoteHead::Type noteHead = is.duration().headType();

    if (instr->useDrumset()) {
        const Drumset* ds  = instr->drumset();
        int pitch    = is.drumNote();
        if (pitch >= 0 && ds->isValid(pitch)) {
            line     = ds->line(pitch);
            noteheadGroup = ds->noteHead(pitch);
        }
    }

    m_shadowNote->setLine(line);

    int voice;
    if (is.drumNote() != -1 && is.drumset() && is.drumset()->isValid(is.drumNote())) {
        voice = is.drumset()->voice(is.drumNote());
    } else {
        voice = is.voice();
    }

    SymId symNotehead;
    TDuration d(is.duration());

    if (is.rest()) {
        int yo;
        Rest rest(gscore, d.type());
        rest.setTicks(d.fraction());
        symNotehead = rest.getSymbol(is.duration().type(), 0, staff->lines(pos.segment->tick()), &yo);
        m_shadowNote->setState(symNotehead, voice, d, true);
    } else {
        if (NoteHead::Group::HEAD_CUSTOM == noteheadGroup) {
            symNotehead = instr->drumset()->noteHeads(is.drumNote(), noteHead);
        } else {
            symNotehead = Note::noteHead(0, noteheadGroup, noteHead);
        }

        m_shadowNote->setState(symNotehead, voice, d);
    }

    m_shadowNote->layout();
    m_shadowNote->setPos(pos.pos);
}

void NotationInteraction::hideShadowNote()
{
    m_shadowNote->setVisible(false);
}

void NotationInteraction::paintShadowNote(QPainter* p)
{
    m_shadowNote->draw(p);
}

Element* NotationInteraction::hitElement(const QPointF& pos, float width) const
{
    QList<Ms::Element*> ll = hitElements(pos, width);
    if (ll.isEmpty()) {
        return nullptr;
    }
    return ll.first();
}

void NotationInteraction::select(Element* e, SelectType type, int staffIdx)
{
    score()->select(e, type, staffIdx);
    m_selectionChanged.notify();
}

INotationSelection* NotationInteraction::selection() const
{
    return m_selection;
}

mu::async::Notification NotationInteraction::selectionChanged() const
{
    return m_selectionChanged;
}

bool NotationInteraction::isDragStarted() const
{
    return m_dragData.dragGroups.size() > 0;
}

void NotationInteraction::DragData::reset()
{
    beginMove = QPointF();
    elementOffset = QPointF();
    editData = Ms::EditData();
    dragGroups.clear();
}

void NotationInteraction::startDrag(const std::vector<Element*>& elems,
                                    const QPointF& eoffset,
                                    const IsDraggable& isDraggable)
{
    m_dragData.reset();
    m_dragData.elementOffset = eoffset;

    for (Element* e : elems) {
        if (!isDraggable(e)) {
            continue;
        }

        std::unique_ptr<Ms::ElementGroup> g = e->getDragGroup(isDraggable);
        if (g && g->enabled()) {
            m_dragData.dragGroups.push_back(std::move(g));
        }
    }

    score()->startCmd();

    for (auto& g : m_dragData.dragGroups) {
        g->startDrag(m_dragData.editData);
    }
}

void NotationInteraction::drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode)
{
    if (m_dragData.beginMove.isNull()) {
        m_dragData.beginMove = fromPos;
        m_dragData.editData.pos = fromPos;
    }

    QPointF normalizedBegin = m_dragData.beginMove - m_dragData.elementOffset;

    QPointF delta = toPos - normalizedBegin;
    QPointF evtDelta = toPos - m_dragData.editData.pos;

    switch (mode) {
    case DragMode::BothXY:
        break;
    case DragMode::OnlyX:
        delta.setY(m_dragData.editData.delta.y());
        evtDelta.setY(0.0);
        break;
    case DragMode::OnlyY:
        delta.setX(m_dragData.editData.delta.x());
        evtDelta.setX(0.0);
        break;
    }

    m_dragData.editData.lastPos = m_dragData.editData.pos;
    m_dragData.editData.hRaster = false;    //mscore->hRaster();
    m_dragData.editData.vRaster = false;    //mscore->vRaster();
    m_dragData.editData.delta   = delta;
    m_dragData.editData.moveDelta = delta - m_dragData.elementOffset;
    m_dragData.editData.evtDelta = evtDelta;
    m_dragData.editData.pos     = toPos;

    for (auto& g : m_dragData.dragGroups) {
        score()->addRefresh(g->drag(m_dragData.editData));
    }

    score()->update();

    m_dragChanged.notify();

    //    QVector<QLineF> anchorLines;
    //    const Selection& sel = score()->selection();
    //    for (Element* e : sel.elements()) {
    //        QVector<QLineF> elAnchorLines = e->dragAnchorLines();
    //        Element* const page = e->findAncestor(ElementType::PAGE);
    //        const QPointF pageOffset((page ? page : e)->pos());

    //        if (!elAnchorLines.isEmpty()) {
    //            for (QLineF& l : elAnchorLines) {
    //                l.translate(pageOffset);
    //            }
    //            anchorLines.append(elAnchorLines);
    //        }
    //    }

    //    if (anchorLines.isEmpty()) {
    //        setDropTarget(0);     // this also resets dropAnchor
    //    } else {
    //        setDropAnchorLines(anchorLines);
    //    }

    //    Element* e = _score->getSelectedElement();
    //    if (e) {
    //        if (_score->playNote()) {
    //            mscore->play(e);
    //            _score->setPlayNote(false);
    //        }
    //    }
    //    updateGrips();
    //    _score->update();
}

void NotationInteraction::endDrag()
{
    for (auto& g : m_dragData.dragGroups) {
        g->endDrag(m_dragData.editData);
    }

    m_dragData.reset();
    //score->selection().unlock("drag");
    //setDropTarget(0);   // this also resets dropAnchor
    score()->endCmd();
//    updateGrips();
//    if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit
//        && _score->selection().element() == editData.element) {
//        startEdit(/* editMode */ false);
//    }
}

mu::async::Notification NotationInteraction::dragChanged()
{
    return m_dragChanged;
}

Ms::Page* NotationInteraction::point2page(const QPointF& p) const
{
    if (score()->layoutMode() == Ms::LayoutMode::LINE) {
        return score()->pages().isEmpty() ? 0 : score()->pages().front();
    }
    foreach (Ms::Page* page, score()->pages()) {
        if (page->bbox().translated(page->pos()).contains(p)) {
            return page;
        }
    }
    return nullptr;
}

QList<Ms::Element*> NotationInteraction::hitElements(const QPointF& p_in, float w) const
{
    Ms::Page* page = point2page(p_in);
    if (!page) {
        return QList<Ms::Element*>();
    }

    QList<Ms::Element*> ll;

    QPointF p = p_in - page->pos();

    QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

    QList<Ms::Element*> el = page->items(r);
    //! TODO
    //    for (int i = 0; i < MAX_HEADERS; i++)
    //        if (score()->headerText(i) != nullptr)      // gives the ability to select the header
    //            el.push_back(score()->headerText(i));
    //    for (int i = 0; i < MAX_FOOTERS; i++)
    //        if (score()->footerText(i) != nullptr)      // gives the ability to select the footer
    //            el.push_back(score()->footerText(i));
    //! -------

    for (Ms::Element* e : el) {
        e->itemDiscovered = 0;
        if (!e->selectable() || e->isPage()) {
            continue;
        }
        if (e->contains(p)) {
            ll.append(e);
        }
    }

    int n = ll.size();
    if ((n == 0) || ((n == 1) && (ll[0]->isMeasure()))) {
        //
        // if no relevant element hit, look nearby
        //
        for (Ms::Element* e : el) {
            if (e->isPage() || !e->selectable()) {
                continue;
            }
            if (e->intersects(r)) {
                ll.append(e);
            }
        }
    }

    if (!ll.empty()) {
        std::sort(ll.begin(), ll.end(), NotationInteraction::elementIsLess);
    }

    return ll;
}

bool NotationInteraction::elementIsLess(const Ms::Element* e1, const Ms::Element* e2)
{
    if (!e1->selectable()) {
        return false;
    }
    if (!e2->selectable()) {
        return true;
    }
    if (e1->isNote() && e2->isStem()) {
        return true;
    }
    if (e2->isNote() && e1->isStem()) {
        return false;
    }
    if (e1->z() == e2->z()) {
        // same stacking order, prefer non-hidden elements
        if (e1->type() == e2->type()) {
            if (e1->type() == Ms::ElementType::NOTEDOT) {
                const Ms::NoteDot* n1 = static_cast<const Ms::NoteDot*>(e1);
                const Ms::NoteDot* n2 = static_cast<const Ms::NoteDot*>(e2);
                if (n1->note() && n1->note()->hidden()) {
                    return false;
                } else if (n2->note() && n2->note()->hidden()) {
                    return true;
                }
            } else if (e1->type() == Ms::ElementType::NOTE) {
                const Ms::Note* n1 = static_cast<const Ms::Note*>(e1);
                const Ms::Note* n2 = static_cast<const Ms::Note*>(e2);
                if (n1->hidden()) {
                    return false;
                } else if (n2->hidden()) {
                    return true;
                }
            }
        }
        // different types, or same type but nothing hidden - use track
        return e1->track() <= e2->track();
    }

    // default case, use stacking order
    return e1->z() <= e2->z();
}
