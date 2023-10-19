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

#include "arpeggio.h"

#include <cmath>

#include "containers.h"

#include "types/typesconv.h"

#include "accidental.h"
#include "chord.h"
#include "mscoreview.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "property.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

Arpeggio::Arpeggio(Chord* parent)
    : EngravingItem(ElementType::ARPEGGIO, parent, ElementFlag::MOVABLE)
{
    m_arpeggioType = ArpeggioType::NORMAL;
    m_span     = 1;
    m_userLen1 = 0.0;
    m_userLen2 = 0.0;
    m_playArpeggio = true;
    m_stretch = 1.0;
}

Arpeggio::~Arpeggio()
{
    // Remove reference to this arpeggio in any chords it may have spanned
    removeChords(track(), track() + m_span);
}

const TranslatableString& Arpeggio::arpeggioTypeName() const
{
    return TConv::userName(m_arpeggioType);
}

void Arpeggio::findChords()
{
    m_maxChordPad = 0.0;
    Chord* _chord = chord();
    track_idx_t strack = track();
    track_idx_t etrack = track() + (m_span - 1);
    _chord->setSpanArpeggio(this);

    for (track_idx_t track = strack; track <= etrack; track++) {
        EngravingItem* e = _chord->segment()->element(track);
        if (e && e->isChord()) {
            toChord(e)->setSpanArpeggio(this);
        }
    }
}

void Arpeggio::removeChords(track_idx_t strack, track_idx_t etrack)
{
    Chord* _chord = chord();
    for (track_idx_t track = strack; track <= etrack; track++) {
        EngravingItem* e = _chord->segment()->element(track);
        if (e && e->isChord() && toChord(e)->spanArpeggio() == this) {
            toChord(e)->setSpanArpeggio(nullptr);
        }
    }
}

void Arpeggio::rebaseStartAnchor(int direction)
{
    if (direction == 0) {
        return;
    }
    if (direction == 1) {
        // Move arpeggio to chord above
        Staff* s = staff();
        Part* part = s->part();
        staff_idx_t topStaff = *part->staveIdxList().begin();
        track_idx_t topTrack = topStaff * VOICES;
        if (track() > topTrack) {
            // Loop through voices til we find a chord
            for (track_idx_t curTrack = track() - 1; curTrack >= topTrack; curTrack--) {
                EngravingItem* e = chord()->segment()->element(curTrack);
                if (e && e->isChord()) {
                    int newSpan = m_span + track() - curTrack;
                    if (newSpan != 0) {
                        undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                        score()->undo(new ChangeParent(this, e, e->staffIdx()));
                        break;
                    }
                }
            }
        }
    } else if (direction == -1) {
        // Move arpeggio to chord below
        for (track_idx_t curTrack = track() + 1; curTrack <= track() + m_span - 1; curTrack++) {
            EngravingItem* e = chord()->segment()->element(curTrack);
            if (e && e->isChord()) {
                int newSpan = m_span + track() - curTrack;
                if (newSpan != 0) {
                    undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                    score()->undo(new ChangeParent(this, e, e->staffIdx()));
                    break;
                }
            }
        }
    }
}

void Arpeggio::rebaseEndAnchor(int direction)
{
    if (direction == 0) {
        return;
    }
    if (direction == 1) {
        // Move end to chord above
        for (track_idx_t curTrack = track() + m_span - 2; curTrack >= track(); curTrack--) {
            EngravingItem* e = chord()->segment()->element(curTrack);
            if (e && e->isChord()) {
                int newSpan = curTrack - track() + 1;
                if (newSpan != 0) {
                    toChord(e)->setSpanArpeggio(nullptr);
                    undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                }
                break;
            }
        }
    } else if (direction == -1) {
        // Move end to chord below
        Staff* s = staff();
        Part* part = s->part();
        size_t n = part->nstaves();
        staff_idx_t ridx = mu::indexOf(part->staves(), s);
        track_idx_t btrack = n * VOICES;
        if (ridx != mu::nidx) {
            if (track() + m_span < btrack) {
                // Loop through voices til we find a chord
                for (track_idx_t curTrack = track() + m_span; curTrack < btrack; curTrack++) {
                    EngravingItem* e = chord()->segment()->element(curTrack);
                    if (e && e->isChord()) {
                        int newSpan = curTrack - track() + 1;
                        toChord(e)->setSpanArpeggio(this);
                        undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
                        break;
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Arpeggio::gripsPositions(const EditData&) const
{
    const LayoutData* ldata = this->ldata();
    const PointF pp(pagePos());
    PointF p1(ldata->bbox().width() / 2, ldata->bbox().top());
    PointF p2(ldata->bbox().width() / 2, ldata->bbox().bottom());
    return { p1 + pp, p2 + pp };
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(EditData& ed)
{
    double d = ed.delta.y();
    if (ed.curGrip == Grip::START) {
        m_userLen1 -= d;
        PointF pos = PointF(chord()->canvasPos().x(), ed.pos.y());
        EngravingItem* e = ed.view()->elementNear(pos);
        if (e && e->isNote()) {
            Chord* c = toNote(e)->chord();
            int newSpan = m_span + track() - c->track();
            undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
            score()->undo(new ChangeParent(this, c, c->staffIdx()));
            m_userLen1 = 0;
        }
    } else if (ed.curGrip == Grip::END) {
        m_userLen2 += d;
        // Increase span
        PointF pos = PointF(chord()->canvasPos().x(), ed.pos.y());
        EngravingItem* e = ed.view()->elementNear(pos);
        if (e && e->isNote()) {
            int newSpan = std::abs(int(track() - e->track())) + 1;
            undoChangeProperty(Pid::ARPEGGIO_SPAN, newSpan);
            m_userLen2 = 0;
        }
    }

    renderer()->layoutItem(this);
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> Arpeggio::dragAnchorLines() const
{
    std::vector<LineF> result;

    Chord* c = chord();
    if (c) {
        result.push_back(LineF(canvasPos(), c->upNote()->canvasPos()));
    }
    return std::vector<LineF>();
}

//---------------------------------------------------------
//   gripAnchorLines
//---------------------------------------------------------

std::vector<LineF> Arpeggio::gripAnchorLines(Grip grip) const
{
    std::vector<LineF> result;

    Chord* _chord = chord();
    if (!_chord) {
        return result;
    }

    const Page* p = toPage(findAncestor(ElementType::PAGE));
    const PointF pageOffset = p ? p->pos() : PointF();

    const PointF gripCanvasPos = gripsPositions()[static_cast<int>(grip)] + pageOffset;

    if (grip == Grip::START) {
        Note* upNote = _chord->upNote();
        EngravingItem* e = _chord->segment()->element(track());
        if (e && e->isChord()) {
            upNote = toChord(e)->upNote();
        }
        result.push_back(LineF(upNote->canvasPos(), gripCanvasPos));
    } else if (grip == Grip::END) {
        Note* downNote = _chord->downNote();
        EngravingItem* e = _chord->segment()->element(track() + m_span - 1);
        if (e && e->isChord()) {
            downNote = toChord(e)->downNote();
        }

        result.push_back(LineF(downNote->canvasPos(), gripCanvasPos));
    }
    return result;
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Arpeggio::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::ARP_USER_LEN1);
    eed->pushProperty(Pid::ARP_USER_LEN2);
}

bool Arpeggio::isEditAllowed(EditData& ed) const
{
    if ((ed.curGrip != Grip::END && ed.curGrip != Grip::START) || !(ed.modifiers & ShiftModifier)) {
        return false;
    }

    return ed.key == Key_Down || ed.key == Key_Up;
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Arpeggio::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    if (ed.curGrip == Grip::START) {
        if (ed.key == Key_Down) {
            rebaseStartAnchor(-1);
        } else if (ed.key == Key_Up) {
            rebaseStartAnchor(1);
        }
    }

    if (ed.curGrip == Grip::END) {
        if (ed.key == Key_Down) {
            rebaseEndAnchor(-1);
        } else if (ed.key == Key_Up) {
            if (m_span > 1) {
                rebaseEndAnchor(1);
            }
        }
    }

    renderer()->layoutOnEdit(this);

    return true;
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Arpeggio::spatiumChanged(double oldValue, double newValue)
{
    m_userLen1 *= (newValue / oldValue);
    m_userLen2 *= (newValue / oldValue);
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Arpeggio::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::ARPEGGIO;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Arpeggio::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ARPEGGIO:
    {
        Arpeggio* a = toArpeggio(e);
        if (explicitParent()) {
            score()->undoRemoveElement(this);
        }
        a->setTrack(track());
        a->setParent(explicitParent());
        score()->undoAddElement(a);
    }
        return e;
    default:
        delete e;
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Arpeggio::reset()
{
    undoChangeProperty(Pid::ARP_USER_LEN1, 0.0);
    undoChangeProperty(Pid::ARP_USER_LEN2, 0.0);
    EngravingItem::reset();
}

bool Arpeggio::crossStaff() const
{
    return (track() + span() - 1) / VOICES != staffIdx();
}

//
// INSET:
// Arpeggios have inset white space. For instance, the bracket
// "[" shape has whitespace inside of the "C". Symbols like
// accidentals can fit inside this whitespace. These inset
// functions are used to get the size of the inner dimensions
// for this area on all arpeggios.
//

//---------------------------------------------------------
//   insetTop
//---------------------------------------------------------

double Arpeggio::insetTop(Chord* c) const
{
    double top = c->upNote()->y() - c->upNote()->height() / 2;

    // use wiggle width, not height, since it's rotated 90 degrees
    if (arpeggioType() == ArpeggioType::UP) {
        top += symBbox(SymId::wiggleArpeggiatoUpArrow).width();
    } else if (arpeggioType() == ArpeggioType::UP_STRAIGHT) {
        top += symBbox(SymId::arrowheadBlackUp).width();
    }

    return top;
}

//---------------------------------------------------------
//   insetBottom
//---------------------------------------------------------

double Arpeggio::insetBottom(Chord* c) const
{
    double bottom = c->downNote()->y() + c->downNote()->height() / 2;

    // use wiggle width, not height, since it's rotated 90 degrees
    if (arpeggioType() == ArpeggioType::DOWN) {
        bottom -= symBbox(SymId::wiggleArpeggiatoUpArrow).width();
    } else if (arpeggioType() == ArpeggioType::DOWN_STRAIGHT) {
        bottom -= symBbox(SymId::arrowheadBlackDown).width();
    }

    return bottom;
}

//---------------------------------------------------------
//   insetWidth
//---------------------------------------------------------

double Arpeggio::insetWidth() const
{
    switch (arpeggioType()) {
    case ArpeggioType::NORMAL:
    {
        return 0.0;
    }

    case ArpeggioType::UP:
    case ArpeggioType::DOWN:
    {
        // use wiggle height, not width, since it's rotated 90 degrees
        return (width() - symBbox(SymId::wiggleArpeggiatoUp).height()) / 2;
    }

    case ArpeggioType::UP_STRAIGHT:
    case ArpeggioType::DOWN_STRAIGHT:
    {
        return (width() - style().styleMM(Sid::ArpeggioLineWidth)) / 2;
    }

    case ArpeggioType::BRACKET:
    {
        return width() - style().styleMM(Sid::ArpeggioLineWidth) / 2;
    }
    }
    return 0.0;
}

//---------------------------------------------------------
//   insetDistance
//---------------------------------------------------------

double Arpeggio::insetDistance(std::vector<Accidental*>& accidentals, double mag_, Chord* chord) const
{
    if (accidentals.size() == 0) {
        return 0.0;
    }
    if (!chord) {
        return 0.0;
    }

    // We have not calculated vertical positioning of staves on the page yet, so this is a bit of a hack
    // Only be concerned about the top or bottom of the arpeggio when the chord belongs to that stave
    // Otherwise set to some large value
    bool startStaff = chord->vStaffIdx() == vStaffIdx();
    bool endStaff = chord->vStaffIdx() == endTrack() / VOICES;

    double arpeggioTop = startStaff ? insetTop(chord) * mag_ : -10000;
    double arpeggioBottom = endStaff ? insetBottom(chord) * mag_ : 10000;

    ArpeggioType type = arpeggioType();
    bool hasTopArrow = type == ArpeggioType::UP
                       || type == ArpeggioType::UP_STRAIGHT
                       || type == ArpeggioType::BRACKET;
    bool hasBottomArrow = type == ArpeggioType::DOWN
                          || type == ArpeggioType::DOWN_STRAIGHT
                          || type == ArpeggioType::BRACKET;

    Accidental* furthestAccidental = nullptr;
    for (auto accidental : accidentals) {
        if (furthestAccidental) {
            bool currentIsFurtherX = accidental->x() < furthestAccidental->x();
            bool currentIsSameX = accidental->x() == furthestAccidental->x();
            auto accidentalBbox = symBbox(accidental->symId());
            double currentTop = accidental->note()->pos().y() + accidentalBbox.top() * mag_;
            double currentBottom = accidental->note()->pos().y() + accidentalBbox.bottom() * mag_;
            bool collidesWithTop = currentTop <= arpeggioTop && hasTopArrow;
            bool collidesWithBottom = currentBottom >= arpeggioBottom && hasBottomArrow;

            if (currentIsFurtherX || (currentIsSameX && (collidesWithTop || collidesWithBottom))) {
                furthestAccidental = accidental;
            }
        } else {
            furthestAccidental = accidental;
        }
    }

    IF_ASSERT_FAILED(furthestAccidental) {
        return 0.0;
    }

    // this cutout means the vertical lines for a ♯, ♭, and ♮ are in the same position
    // if an accidental does not have a cutout (e.g., ♭), this value is 0
    double accidentalCutOutX = symSmuflAnchor(furthestAccidental->symId(), SmuflAnchorId::cutOutNW).x() * mag_;
    double accidentalCutOutYTop = symSmuflAnchor(furthestAccidental->symId(), SmuflAnchorId::cutOutNW).y() * mag_;
    double accidentalCutOutYBottom = symSmuflAnchor(furthestAccidental->symId(), SmuflAnchorId::cutOutSW).y() * mag_;

    double maximumInset = (style().styleMM(Sid::ArpeggioAccidentalDistance)
                           - style().styleMM(Sid::ArpeggioAccidentalDistanceMin)) * mag_;

    if (accidentalCutOutX > maximumInset) {
        accidentalCutOutX = maximumInset;
    }

    RectF bbox = symBbox(furthestAccidental->symId());
    double center = furthestAccidental->note()->pos().y() * mag_;
    double top = center + bbox.top() * mag_;
    double bottom = center + bbox.bottom() * mag_;
    bool collidesWithTop = hasTopArrow && top <= arpeggioTop;
    bool collidesWithBottom = hasBottomArrow && bottom >= arpeggioBottom;
    bool cutoutCollidesWithTop = collidesWithTop && top - accidentalCutOutYTop >= arpeggioTop;
    bool cutoutCollidesWithBottom = collidesWithBottom && bottom - accidentalCutOutYBottom <= arpeggioBottom;

    if (collidesWithTop || collidesWithBottom) {
        // optical adjustment for one edge case
        if (accidentalCutOutX == 0.0 || cutoutCollidesWithTop || cutoutCollidesWithBottom) {
            return accidentalCutOutX + maximumInset;
        }
        return accidentalCutOutX;
    }

    return insetWidth() + accidentalCutOutX;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

engraving::PropertyValue Arpeggio::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ARPEGGIO_TYPE:
        return int(m_arpeggioType);
    case Pid::TIME_STRETCH:
        return Stretch();
    case Pid::ARP_USER_LEN1:
        return userLen1();
    case Pid::ARP_USER_LEN2:
        return userLen2();
    case Pid::PLAY:
        return m_playArpeggio;
    case Pid::ARPEGGIO_SPAN:
        return m_span;
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Arpeggio::setProperty(Pid propertyId, const engraving::PropertyValue& val)
{
    switch (propertyId) {
    case Pid::ARPEGGIO_TYPE:
        setArpeggioType(ArpeggioType(val.toInt()));
        break;
    case Pid::TIME_STRETCH:
        setStretch(val.toDouble());
        break;
    case Pid::ARP_USER_LEN1:
        setUserLen1(val.toDouble());
        break;
    case Pid::ARP_USER_LEN2:
        setUserLen2(val.toDouble());
        break;
    case Pid::PLAY:
        setPlayArpeggio(val.toBool());
        break;
    case Pid::ARPEGGIO_SPAN:
        setSpan(val.toInt());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue Arpeggio::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::ARP_USER_LEN1:
        return 0.0;
    case Pid::ARP_USER_LEN2:
        return 0.0;
    case Pid::TIME_STRETCH:
        return 1.0;
    case Pid::PLAY:
        return true;
    case Pid::ARPEGGIO_SPAN:
        return 1;
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}
