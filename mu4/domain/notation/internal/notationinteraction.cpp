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
#include <QPainter>

#include "ptrutils.h"

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
#include "libmscore/stafflines.h"
#include "libmscore/icon.h"

#include "notation.h"
#include "scorecallbacks.h"

using namespace mu::domain::notation;
using namespace Ms;

NotationInteraction::NotationInteraction(Notation* notation)
    : m_notation(notation)
{
    m_inputState = new NotationInputState(notation);
    m_selection = new NotationSelection(notation);

    m_scoreCallbacks = new ScoreCallbacks();
    m_dragData.ed.view = new ScoreCallbacks();
    m_dropData.ed.view = new ScoreCallbacks();
}

NotationInteraction::~NotationInteraction()
{
    delete m_inputState;
    delete m_selection;
    delete m_shadowNote;
    delete m_scoreCallbacks;
}

void NotationInteraction::init()
{
    m_shadowNote = new ShadowNote(score());
    m_shadowNote->setVisible(false);
}

Ms::Score* NotationInteraction::score() const
{
    return m_notation->score();
}

void NotationInteraction::paint(QPainter* p)
{
    m_shadowNote->draw(p);

    drawAnchorLines(p);
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
        Ms::Chord* c = static_cast<Ms::Chord*>(el);
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
    m_noteAdded.notify();
}

mu::async::Notification NotationInteraction::noteAdded() const
{
    return m_noteAdded;
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

QList<Element*> NotationInteraction::elementsAt(const QPointF& p) const
{
    QList<Element*> el;
    Page* page = point2page(p);
    if (page) {
        el = page->items(p - page->pos());
        std::sort(el.begin(), el.end(), NotationInteraction::elementIsLess);
    }
    return el;
}

Element* NotationInteraction::elementAt(const QPointF& p) const
{
    QList<Element*> el = elementsAt(p);
    Element* e = el.value(0);
    if (e && e->isPage()) {
        e = el.value(1);
    }
    return e;
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
    ed = Ms::EditData();
    dragGroups.clear();
}

void NotationInteraction::startDrag(const std::vector<Element*>& elems,
                                    const QPointF& eoffset,
                                    const IsDraggable& isDraggable)
{
    m_dragData.reset();
    m_dragData.elements = elems;
    m_dragData.elementOffset = eoffset;

    for (Element* e : m_dragData.elements) {
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
        g->startDrag(m_dragData.ed);
    }
}

void NotationInteraction::drag(const QPointF& fromPos, const QPointF& toPos, DragMode mode)
{
    if (m_dragData.beginMove.isNull()) {
        m_dragData.beginMove = fromPos;
        m_dragData.ed.pos = fromPos;
    }

    QPointF normalizedBegin = m_dragData.beginMove - m_dragData.elementOffset;

    QPointF delta = toPos - normalizedBegin;
    QPointF evtDelta = toPos - m_dragData.ed.pos;

    switch (mode) {
    case DragMode::BothXY:
        break;
    case DragMode::OnlyX:
        delta.setY(m_dragData.ed.delta.y());
        evtDelta.setY(0.0);
        break;
    case DragMode::OnlyY:
        delta.setX(m_dragData.ed.delta.x());
        evtDelta.setX(0.0);
        break;
    }

    m_dragData.ed.lastPos = m_dragData.ed.pos;
    m_dragData.ed.hRaster = false;    //mscore->hRaster();
    m_dragData.ed.vRaster = false;    //mscore->vRaster();
    m_dragData.ed.delta   = delta;
    m_dragData.ed.moveDelta = delta - m_dragData.elementOffset;
    m_dragData.ed.evtDelta = evtDelta;
    m_dragData.ed.pos     = toPos;

    for (auto& g : m_dragData.dragGroups) {
        score()->addRefresh(g->drag(m_dragData.ed));
    }

    score()->update();

    QVector<QLineF> anchorLines;
    for (const Element* e : m_dragData.elements) {
        QVector<QLineF> elAnchorLines = e->dragAnchorLines();
        const Ms::Element* page = e->findAncestor(ElementType::PAGE);
        const QPointF pageOffset((page ? page : e)->pos());

        if (!elAnchorLines.isEmpty()) {
            for (QLineF& l : elAnchorLines) {
                l.translate(pageOffset);
            }
            anchorLines.append(elAnchorLines);
        }
    }

    setAnchorLines(anchorLines.toStdVector());

    m_dragChanged.notify();

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
        g->endDrag(m_dragData.ed);
    }

    m_dragData.reset();
    resetAnchorLines();
    score()->endCmd();
    m_dragChanged.notify();
    //    updateGrips();
    //    if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit
    //        && _score->selection().element() == editData.element) {
    //        startEdit(/* editMode */ false);
    //    }
}

mu::async::Notification NotationInteraction::dragChanged() const
{
    return m_dragChanged;
}

//! NOTE Copied from ScoreView::dragEnterEvent
void NotationInteraction::startDrop(const QByteArray& edata)
{
    if (m_dropData.ed.dropElement) {
        delete m_dropData.ed.dropElement;
        m_dropData.ed.dropElement = nullptr;
    }

    XmlReader e(edata);
    m_dropData.ed.dragOffset = QPointF();
    Fraction duration;      // dummy
    ElementType type = Element::readType(e, &m_dropData.ed.dragOffset, &duration);

    Element* el = Element::create(type, score());
    if (el) {
        if (type == ElementType::BAR_LINE || type == ElementType::ARPEGGIO || type == ElementType::BRACKET) {
            double spatium = score()->spatium();
            el->setHeight(spatium * 5);
        }
        m_dropData.ed.dropElement = el;
        m_dropData.ed.dropElement->setParent(0);
        m_dropData.ed.dropElement->read(e);
        m_dropData.ed.dropElement->layout();
    }
}

//! NOTE Copied from ScoreView::dragMoveEvent
bool NotationInteraction::isDropAccepted(const QPointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.ed.dropElement) {
        return false;
    }

    m_dropData.ed.pos = pos;
    m_dropData.ed.modifiers = modifiers;

    switch (m_dropData.ed.dropElement->type()) {
    case ElementType::VOLTA:
        return dragMeasureAnchorElement(pos);
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::HAIRPIN:
    case ElementType::TEXTLINE:
        return dragTimeAnchorElement(pos);
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::DYNAMIC:
    case ElementType::KEYSIG:
    case ElementType::CLEF:
    case ElementType::TIMESIG:
    case ElementType::BAR_LINE:
    case ElementType::ARPEGGIO:
    case ElementType::BREATH:
    case ElementType::GLISSANDO:
    case ElementType::MEASURE_NUMBER:
    case ElementType::BRACKET:
    case ElementType::ARTICULATION:
    case ElementType::FERMATA:
    case ElementType::CHORDLINE:
    case ElementType::BEND:
    case ElementType::ACCIDENTAL:
    case ElementType::TEXT:
    case ElementType::FINGERING:
    case ElementType::TEMPO_TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::NOTEHEAD:
    case ElementType::TREMOLO:
    case ElementType::LAYOUT_BREAK:
    case ElementType::MARKER:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::REHEARSAL_MARK:
    case ElementType::JUMP:
    case ElementType::REPEAT_MEASURE:
    case ElementType::ICON:
    case ElementType::CHORD:
    case ElementType::SPACER:
    case ElementType::SLUR:
    case ElementType::HARMONY:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::AMBITUS:
    case ElementType::TREMOLOBAR:
    case ElementType::FIGURED_BASS:
    case ElementType::LYRICS:
    case ElementType::FRET_DIAGRAM:
    case ElementType::STAFFTYPE_CHANGE: {
        Element* e = dropTarget(m_dropData.ed);
        if (e) {
            if (!e->isMeasure()) {
                setDropTarget(e);
            }
            return true;
        } else {
            return false;
        }
    }
    break;
    default:
        break;
    }

    return false;
}

//! NOTE Copied from ScoreView::dropEvent
bool NotationInteraction::drop(const QPointF& pos, Qt::KeyboardModifiers modifiers)
{
    if (!m_dropData.ed.dropElement) {
        return false;
    }

    IF_ASSERT_FAILED(m_dropData.ed.dropElement->score() == score()) {
        return false;
    }

    bool accepted = false;

    m_dropData.ed.pos       = pos;
    m_dropData.ed.modifiers = modifiers;

    bool firstStaffOnly = false;
    bool applyUserOffset = false;
    //bool triggerSpannerDropApplyTour = m_dropData.ed.dropElement->isSpanner();
    m_dropData.ed.dropElement->styleChanged();
    score()->startCmd();
    score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    switch (m_dropData.ed.dropElement->type()) {
    case ElementType::VOLTA:
        // voltas drop to first staff by default, or closest staff if Control is held
        firstStaffOnly = !(m_dropData.ed.modifiers & Qt::ControlModifier);
    // fall-thru
    case ElementType::OTTAVA:
    case ElementType::TRILL:
    case ElementType::PEDAL:
    case ElementType::LET_RING:
    case ElementType::VIBRATO:
    case ElementType::PALM_MUTE:
    case ElementType::HAIRPIN:
    case ElementType::TEXTLINE:
    {
        Spanner* spanner = ptr::checked_cast<Spanner>(m_dropData.ed.dropElement);
        score()->cmdAddSpanner(spanner, pos, firstStaffOnly);
        score()->setUpdateAll();
        accepted = true;
    }
    break;
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::IMAGE:
        applyUserOffset = true;
    // fall-thru
    case ElementType::DYNAMIC:
    case ElementType::FRET_DIAGRAM:
    case ElementType::HARMONY:
    {
        Element* el = elementAt(pos);
        if (el == 0 || el->type() == ElementType::STAFF_LINES) {
            int staffIdx;
            Segment* seg;
            QPointF offset;
            el = score()->pos2measure(pos, &staffIdx, 0, &seg, &offset);
            if (el && el->isMeasure()) {
                m_dropData.ed.dropElement->setTrack(staffIdx * VOICES);
                if (m_dropData.ed.dropElement->isImage()) {
                    m_dropData.ed.dropElement->setParent(el);
                    offset = pos - el->canvasPos();
                } else {
                    m_dropData.ed.dropElement->setParent(seg);
                }
                if (applyUserOffset) {
                    m_dropData.ed.dropElement->setOffset(offset);
                }
                score()->undoAddElement(m_dropData.ed.dropElement);
            } else {
                qDebug("cannot drop here");
                delete m_dropData.ed.dropElement;
                m_dropData.ed.dropElement = nullptr;
            }
        } else {
            score()->addRefresh(el->canvasBoundingRect());
            score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());

            if (!el->acceptDrop(m_dropData.ed)) {
                qDebug("drop %s onto %s not accepted", m_dropData.ed.dropElement->name(), el->name());
                break;
            }
            Element* dropElement = el->drop(m_dropData.ed);
            score()->addRefresh(el->canvasBoundingRect());
            if (dropElement) {
                score()->select(dropElement, SelectType::SINGLE, 0);
                score()->addRefresh(dropElement->canvasBoundingRect());
            }
        }
    }
        accepted = true;
        break;
    case ElementType::HBOX:
    case ElementType::VBOX:
    case ElementType::KEYSIG:
    case ElementType::CLEF:
    case ElementType::TIMESIG:
    case ElementType::BAR_LINE:
    case ElementType::ARPEGGIO:
    case ElementType::BREATH:
    case ElementType::GLISSANDO:
    case ElementType::MEASURE_NUMBER:
    case ElementType::BRACKET:
    case ElementType::ARTICULATION:
    case ElementType::FERMATA:
    case ElementType::CHORDLINE:
    case ElementType::BEND:
    case ElementType::ACCIDENTAL:
    case ElementType::TEXT:
    case ElementType::FINGERING:
    case ElementType::TEMPO_TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::NOTEHEAD:
    case ElementType::TREMOLO:
    case ElementType::LAYOUT_BREAK:
    case ElementType::MARKER:
    case ElementType::STAFF_STATE:
    case ElementType::INSTRUMENT_CHANGE:
    case ElementType::REHEARSAL_MARK:
    case ElementType::JUMP:
    case ElementType::REPEAT_MEASURE:
    case ElementType::ICON:
    case ElementType::NOTE:
    case ElementType::CHORD:
    case ElementType::SPACER:
    case ElementType::SLUR:
    case ElementType::BAGPIPE_EMBELLISHMENT:
    case ElementType::AMBITUS:
    case ElementType::TREMOLOBAR:
    case ElementType::FIGURED_BASS:
    case ElementType::LYRICS:
    case ElementType::STAFFTYPE_CHANGE: {
        Element* el = dropTarget(m_dropData.ed);
        if (!el) {
            if (!dropCanvas(m_dropData.ed.dropElement)) {
                qDebug("cannot drop %s(%p) to canvas", m_dropData.ed.dropElement->name(), m_dropData.ed.dropElement);
                delete m_dropData.ed.dropElement;
                m_dropData.ed.dropElement = nullptr;
            }
            break;
        }
        score()->addRefresh(el->canvasBoundingRect());

        // TODO: HACK ALERT!
        if (el->isMeasure() && m_dropData.ed.dropElement->isLayoutBreak()) {
            Measure* m = toMeasure(el);
            if (m->isMMRest()) {
                el = m->mmRestLast();
            }
        }

        Element* dropElement = el->drop(m_dropData.ed);
        if (dropElement && dropElement->isInstrumentChange()) {
            NOT_IMPLEMENTED;
            // mscore->currentScoreView()->selectInstrument(toInstrumentChange(dropElement));
        }
        score()->addRefresh(el->canvasBoundingRect());
        if (dropElement) {
            if (!score()->noteEntryMode()) {
                score()->select(dropElement, SelectType::SINGLE, 0);
            }
            score()->addRefresh(dropElement->canvasBoundingRect());
        }
        accepted = true;
    }
    break;
    default:
        delete m_dropData.ed.dropElement;
        break;
    }
    m_dropData.ed.dropElement = nullptr;
    setDropTarget(nullptr);         // this also resets dropRectangle and dropAnchor
    score()->endCmd();
    // update input cursor position (must be done after layout)
//    if (noteEntryMode()) {
//        moveCursor();
//    }
//    if (triggerSpannerDropApplyTour) {
//        TourHandler::startTour("spanner-drop-apply");
//    }
    if (accepted) {
        m_dropChanged.notify();
    }
    return accepted;
}

//! NOTE Copied from ScoreView::dragLeaveEvent
void NotationInteraction::endDrop()
{
    if (m_dropData.ed.dropElement) {
        score()->setUpdateAll();
        delete m_dropData.ed.dropElement;
        m_dropData.ed.dropElement = nullptr;
        score()->update();
    }
    setDropTarget(nullptr);
}

mu::async::Notification NotationInteraction::dropChanged() const
{
    return m_dropChanged;
}

//! NOTE Copied from ScoreView::dropCanvas
bool NotationInteraction::dropCanvas(Element* e)
{
    if (e->isIcon()) {
        switch (toIcon(e)->iconType()) {
        case IconType::VFRAME:
            score()->insertMeasure(ElementType::VBOX, 0);
            break;
        case IconType::HFRAME:
            score()->insertMeasure(ElementType::HBOX, 0);
            break;
        case IconType::TFRAME:
            score()->insertMeasure(ElementType::TBOX, 0);
            break;
        case IconType::FFRAME:
            score()->insertMeasure(ElementType::FBOX, 0);
            break;
        case IconType::MEASURE:
            score()->insertMeasure(ElementType::MEASURE, 0);
            break;
        default:
            return false;
        }
        delete e;
        return true;
    }
    return false;
}

//! NOTE Copied from ScoreView::getDropTarget
Element* NotationInteraction::dropTarget(Ms::EditData& ed) const
{
    QList<Element*> el = elementsAt(ed.pos);
    for (Element* e : el) {
        if (e->isStaffLines()) {
            if (el.size() > 2) {          // is not first class drop target
                continue;
            }
            e = toStaffLines(e)->measure();
        }
        if (e->acceptDrop(ed)) {
            return e;
        }
    }
    return nullptr;
}

//! NOTE Copied from ScoreView::dragMeasureAnchorElement
bool NotationInteraction::dragMeasureAnchorElement(const QPointF& pos)
{
    int staffIdx;
    Segment* seg;
    MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, &seg, 0);
    if (!(m_dropData.ed.modifiers & Qt::ControlModifier)) {
        staffIdx = 0;
    }
    int track = staffIdx * VOICES;

    if (mb && mb->isMeasure()) {
        Measure* m = toMeasure(mb);
        System* s  = m->system();
        qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
        QRectF b(m->canvasBoundingRect());
        if (pos.x() >= (b.x() + b.width() * .5) && m != score()->lastMeasureMM()
            && m->nextMeasure()->system() == m->system()) {
            m = m->nextMeasure();
        }
        QPointF anchor(m->canvasBoundingRect().x(), y);
        setAnchorLines({ QLineF(pos, anchor) });
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        m_dropData.ed.dropElement->setTrack(track);
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        m_dragChanged.notify();
        return true;
    }
    m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    setDropTarget(nullptr);
    return false;
}

//! NOTE Copied from ScoreView::dragTimeAnchorElement
bool NotationInteraction::dragTimeAnchorElement(const QPointF& pos)
{
    int staffIdx;
    Segment* seg;
    MeasureBase* mb = score()->pos2measure(pos, &staffIdx, 0, &seg, 0);
    int track  = staffIdx * VOICES;

    if (mb && mb->isMeasure() && seg->element(track)) {
        Measure* m = toMeasure(mb);
        System* s  = m->system();
        qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
        QPointF anchor(seg->canvasBoundingRect().x(), y);
        setAnchorLines({ QLineF(pos, anchor) });
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        m_dropData.ed.dropElement->setTrack(track);
        m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
        m_dragChanged.notify();
        return true;
    }
    m_dropData.ed.dropElement->score()->addRefresh(m_dropData.ed.dropElement->canvasBoundingRect());
    setDropTarget(nullptr);
    return false;
}

void NotationInteraction::setDropTarget(Element* el)
{
    if (m_dropData.dropTarget != el) {
        if (m_dropData.dropTarget) {
            m_dropData.dropTarget->setDropTarget(false);
            m_dropData.dropTarget = nullptr;
        }

        m_dropData.dropTarget = el;
        if (m_dropData.dropTarget) {
            m_dropData.dropTarget->setDropTarget(true);
        }
    }

    m_anchorLines.clear();

    //! TODO
    //    if (dropRectangle.isValid()) {
    //        dropRectangle = QRectF();
    //    }
    //! ---

    m_dragChanged.notify();
}

void NotationInteraction::setAnchorLines(const std::vector<QLineF>& anchorList)
{
    m_anchorLines = anchorList;
}

void NotationInteraction::resetAnchorLines()
{
    m_anchorLines.clear();
}

void NotationInteraction::drawAnchorLines(QPainter* painter)
{
    if (m_anchorLines.empty()) {
        return;
    }

    const auto dropAnchorColor = configuration()->anchorLineColor();
    QPen pen(QBrush(dropAnchorColor), 2.0 / painter->worldTransform().m11(), Qt::DotLine);

    for (const QLineF& anchor : m_anchorLines) {
        painter->setPen(pen);
        painter->drawLine(anchor);

        qreal d = 4.0 / painter->worldTransform().m11();
        QRectF rect(-d, -d, 2 * d, 2 * d);

        painter->setBrush(QBrush(dropAnchorColor));
        painter->setPen(Qt::NoPen);
        rect.moveCenter(anchor.p1());
        painter->drawEllipse(rect);
        rect.moveCenter(anchor.p2());
        painter->drawEllipse(rect);
    }
}

void NotationInteraction::moveSelection(MoveDirection d, MoveSelectionType type)
{
    IF_ASSERT_FAILED(MoveDirection::Left == d || MoveDirection::Right == d) {
        return;
    }

    IF_ASSERT_FAILED(MoveSelectionType::Undefined != type) {
        return;
    }

    if (MoveSelectionType::Element == type) {
        moveElementSelection(d);
        return;
    }

    //! NOTE Previously, the `Score::move` method directly expected commands (actions)
    //! Now the `Notation` provides only notation management methods,
    //! and interpretation of actions is the responsibility of `NotationActionController`

    auto typeToString = [](MoveSelectionType type) {
                            switch (type) {
                            case MoveSelectionType::Undefined: return QString();
                            case MoveSelectionType::Element:   return QString();
                            case MoveSelectionType::Chord:     return QString("chord");
                            case MoveSelectionType::Measure:   return QString("measure");
                            case MoveSelectionType::Track:     return QString("track");
                            }
                            return QString();
                        };

    QString cmd;
    if (MoveDirection::Left == d) {
        cmd = "prev-";
    }

    if (MoveDirection::Right == d) {
        cmd = "next-";
    }

    cmd += typeToString(type);

    score()->move(cmd);
    m_selectionChanged.notify();
}

void NotationInteraction::moveElementSelection(MoveDirection d)
{
    Element* el = score()->selection().element();
    if (!el && !score()->selection().elements().isEmpty()) {
        el = score()->selection().elements().last();
    }

    if (!el) {
        ChordRest* cr = score()->selection().currentCR();
        if (cr) {
            if (cr->isChord()) {
                if (MoveDirection::Left == d) {
                    el = toChord(cr)->upNote();
                } else {
                    el = toChord(cr)->downNote();
                }
            } else if (cr->isRest()) {
                el = cr;
            }
            score()->select(el);
        }
    }

    Element* toEl = nullptr;
    if (el) {
        toEl = (MoveDirection::Left == d) ? score()->prevElement() : score()->nextElement();
    } else {
        toEl = (MoveDirection::Left == d) ? score()->lastElement() : score()->firstElement();
    }

    if (toEl) {
        score()->select(toEl, SelectType::SINGLE, 0);
        if (toEl->type() == ElementType::NOTE || toEl->type() == ElementType::HARMONY) {
            score()->setPlayNote(true);
        }
    }

    m_selectionChanged.notify();
}

void NotationInteraction::movePitch(MoveDirection d, PitchMode mode)
{
    IF_ASSERT_FAILED(MoveDirection::Up == d || MoveDirection::Down == d) {
        return;
    }

    QList<Note*> el = score()->selection().uniqueNotes();
    IF_ASSERT_FAILED(!el.isEmpty()) {
        return;
    }

    score()->startCmd();

    bool isUp = MoveDirection::Up == d;
    score()->upDown(isUp, mode);

    score()->endCmd();

    m_dragChanged.notify();
}

void NotationInteraction::moveText(MoveDirection d, bool quickly)
{
    Element* el = score()->selection().element();
    IF_ASSERT_FAILED(el && el->isTextBase()) {
        return;
    }

    score()->startCmd();

    qreal step = quickly ? MScore::nudgeStep10 : MScore::nudgeStep;
    step = step * el->spatium();

    switch (d) {
    case MoveDirection::Undefined:
        IF_ASSERT_FAILED(d != MoveDirection::Undefined) {
            return;
        }
        break;
    case MoveDirection::Left:
        el->undoChangeProperty(Pid::OFFSET, el->offset() - QPointF(step, 0.0), PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Right:
        el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF(step, 0.0), PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Up:
        el->undoChangeProperty(Pid::OFFSET, el->offset() - QPointF(0.0, step), PropertyFlags::UNSTYLED);
        break;
    case MoveDirection::Down:
        el->undoChangeProperty(Pid::OFFSET, el->offset() + QPointF(0.0, step), PropertyFlags::UNSTYLED);
        break;
    }

    score()->endCmd();

    m_dragChanged.notify();
}
