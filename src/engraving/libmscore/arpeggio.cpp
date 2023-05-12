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

#include "iengravingfont.h"

#include "types/typesconv.h"
#include "layout/tlayout.h"

#include "accidental.h"
#include "chord.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "property.h"
#include "score.h"
#include "segment.h"
#include "staff.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

Arpeggio::Arpeggio(Chord* parent)
    : EngravingItem(ElementType::ARPEGGIO, parent, ElementFlag::MOVABLE)
{
    _arpeggioType = ArpeggioType::NORMAL;
    setHeight(spatium() * 4);        // for use in palettes
    _span     = 1;
    _userLen1 = 0.0;
    _userLen2 = 0.0;
    _playArpeggio = true;
    _stretch = 1.0;
}

const TranslatableString& Arpeggio::arpeggioTypeName() const
{
    return TConv::userName(_arpeggioType);
}

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(double h)
{
    _height = h;
}

//---------------------------------------------------------
//   symbolLine
//    construct a string of symbols approximating width w
//---------------------------------------------------------

void Arpeggio::symbolLine(SymId end, SymId fill)
{
    double top = calcTop();
    double bottom = calcBottom();
    double w   = bottom - top;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    m_symbols.clear();
    double w1 = f->advance(end, mag);
    double w2 = f->advance(fill, mag);
    int n    = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        m_symbols.push_back(fill);
    }
    m_symbols.push_back(end);
}

//---------------------------------------------------------
//   calcTop
//---------------------------------------------------------

double Arpeggio::calcTop() const
{
    double top = -_userLen1;
    if (!explicitParent()) {
        return top;
    }
    switch (arpeggioType()) {
    case ArpeggioType::BRACKET: {
        double lineWidth = score()->styleMM(Sid::ArpeggioLineWidth);
        return top - lineWidth / 2.0;
    }
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    case ArpeggioType::DOWN: {
        // if the top is in the staff on a space, move it up
        // if the bottom note is on a line, the distance is 0.25 spaces
        // if the bottom note is on a space, the distance is 0.5 spaces
        int topNoteLine = chord()->upNote()->line();
        int lines = staff()->lines(tick());
        int bottomLine = (lines - 1) * 2;
        if (topNoteLine <= 0 || topNoteLine % 2 == 0 || topNoteLine >= bottomLine) {
            return top;
        }
        int downNoteLine = chord()->downNote()->line();
        if (downNoteLine % 2 == 1 && downNoteLine < bottomLine) {
            return top - 0.4 * spatium();
        }
        return top - 0.25 * spatium();
    }
    default: {
        return top - spatium() / 4;
    }
    }
}

//---------------------------------------------------------
//   computeHeight
//---------------------------------------------------------

void Arpeggio::computeHeight(bool includeCrossStaffHeight)
{
    Chord* topChord = chord();
    if (!topChord) {
        return;
    }
    double y = topChord->upNote()->pagePos().y() - topChord->upNote()->headHeight() * .5;

    Note* bottomNote = topChord->downNote();
    if (includeCrossStaffHeight) {
        track_idx_t bottomTrack = track() + (_span - 1) * VOICES;
        EngravingItem* element = topChord->segment()->element(bottomTrack);
        Chord* bottomChord = (element && element->isChord()) ? toChord(element) : topChord;
        bottomNote = bottomChord->downNote();
    }

    double h = bottomNote->pagePos().y() + bottomNote->headHeight() * .5 - y;
    setHeight(h);
}

//---------------------------------------------------------
//   calcBottom
//---------------------------------------------------------

double Arpeggio::calcBottom() const
{
    double top = -_userLen1;
    double bottom = _height + _userLen2;
    if (!explicitParent()) {
        return bottom;
    }
    switch (arpeggioType()) {
    case ArpeggioType::BRACKET: {
        double lineWidth = score()->styleMM(Sid::ArpeggioLineWidth);
        return bottom - top + lineWidth;
    }
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    case ArpeggioType::DOWN: {
        return bottom;
    }
    default: {
        return bottom - top + spatium() / 2;
    }
    }
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void Arpeggio::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
//   draw
//---------------------------------------------------------

void Arpeggio::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;

    double _spatium = spatium();

    double y1 = _bbox.top();
    double y2 = _bbox.bottom();

    double lineWidth = score()->styleMM(Sid::ArpeggioLineWidth);

    painter->setPen(Pen(curColor(), lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->save();

    switch (arpeggioType()) {
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    {
        RectF r(symBbox(m_symbols));
        painter->rotate(-90.0);
        score()->engravingFont()->draw(m_symbols, painter, magS(), PointF(-r.right() - y1, -r.bottom() + r.height()));
    }
    break;

    case ArpeggioType::DOWN:
    {
        RectF r(symBbox(m_symbols));
        painter->rotate(90.0);
        score()->engravingFont()->draw(m_symbols, painter, magS(), PointF(-r.left() + y1, -r.top() - r.height()));
    }
    break;

    case ArpeggioType::UP_STRAIGHT:
    {
        RectF r(symBbox(SymId::arrowheadBlackUp));
        double x1 = _spatium * .5;
        drawSymbol(SymId::arrowheadBlackUp, painter, PointF(x1 - r.width() * .5, y1 - r.top()));
        y1 -= r.top() * .5;
        painter->drawLine(LineF(x1, y1, x1, y2));
    }
    break;

    case ArpeggioType::DOWN_STRAIGHT:
    {
        RectF r(symBbox(SymId::arrowheadBlackDown));
        double x1 = _spatium * .5;

        drawSymbol(SymId::arrowheadBlackDown, painter, PointF(x1 - r.width() * .5, y2 - r.bottom()));
        y2 += r.top() * .5;
        painter->drawLine(LineF(x1, y1, x1, y2));
    }
    break;

    case ArpeggioType::BRACKET:
    {
        double w = score()->styleS(Sid::ArpeggioHookLen).val() * _spatium;
        painter->drawLine(LineF(0.0, y1, w, y1));
        painter->drawLine(LineF(0.0, y2, w, y2));
        painter->drawLine(LineF(0.0, y1 - lineWidth / 2, 0.0, y2 + lineWidth / 2));
    }
    break;
    }
    painter->restore();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Arpeggio::gripsPositions(const EditData&) const
{
    const PointF pp(pagePos());
    PointF p1(_bbox.width() / 2, _bbox.top());
    PointF p2(_bbox.width() / 2, _bbox.bottom());
    return { p1 + pp, p2 + pp };
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(EditData& ed)
{
    double d = ed.delta.y();
    if (ed.curGrip == Grip::START) {
        _userLen1 -= d;
    } else if (ed.curGrip == Grip::END) {
        _userLen2 += d;
    }

    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
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
        result.push_back(LineF(_chord->upNote()->canvasPos(), gripCanvasPos));
    } else if (grip == Grip::END) {
        Note* downNote = _chord->downNote();
        track_idx_t btrack  = track() + (_span - 1) * VOICES;
        EngravingItem* e = _chord->segment()->element(btrack);
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
    if (ed.curGrip != Grip::END || !(ed.modifiers & ShiftModifier)) {
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

    if (ed.key == Key_Down) {
        Staff* s = staff();
        Part* part = s->part();
        size_t n = part->nstaves();
        staff_idx_t ridx = mu::indexOf(part->staves(), s);
        if (ridx != mu::nidx) {
            if (_span + ridx < n) {
                ++_span;
            }
        }
    } else if (ed.key == Key_Up) {
        if (_span > 1) {
            --_span;
        }
    }

    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);

    Chord* c = chord();
    setPosX(-(width() + spatium() * .5));
    c->layoutArpeggio2();
    Fraction _tick = tick();
    score()->setLayout(_tick, _tick, staffIdx(), staffIdx() + _span, this);
    return true;
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Arpeggio::spatiumChanged(double oldValue, double newValue)
{
    _userLen1 *= (newValue / oldValue);
    _userLen2 *= (newValue / oldValue);
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

double Arpeggio::insetTop() const
{
    double top = chord()->upNote()->y() - chord()->upNote()->height() / 2;

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

double Arpeggio::insetBottom() const
{
    double bottom = chord()->downNote()->y() + chord()->downNote()->height() / 2;

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
        return (width() - score()->styleMM(Sid::ArpeggioLineWidth)) / 2;
    }

    case ArpeggioType::BRACKET:
    {
        return width() - score()->styleMM(Sid::ArpeggioLineWidth) / 2;
    }
    }
    return 0.0;
}

//---------------------------------------------------------
//   insetDistance
//---------------------------------------------------------

double Arpeggio::insetDistance(std::vector<Accidental*>& accidentals, double mag_) const
{
    if (accidentals.size() == 0) {
        return 0.0;
    }

    double arpeggioTop = insetTop() * mag_;
    double arpeggioBottom = insetBottom() * mag_;
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
            auto accidentalBbox = symBbox(accidental->symbol());
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

    // this cutout means the vertical lines for a ♯, ♭, and ♮ are in the same position
    // if an accidental does not have a cutout (e.g., ♭), this value is 0
    double accidentalCutOutX = symSmuflAnchor(furthestAccidental->symbol(), SmuflAnchorId::cutOutNW).x() * mag_;
    double accidentalCutOutYTop = symSmuflAnchor(furthestAccidental->symbol(), SmuflAnchorId::cutOutNW).y() * mag_;
    double accidentalCutOutYBottom = symSmuflAnchor(furthestAccidental->symbol(), SmuflAnchorId::cutOutSW).y() * mag_;

    double maximumInset = (score()->styleMM(Sid::ArpeggioAccidentalDistance)
                           - score()->styleMM(Sid::ArpeggioAccidentalDistanceMin)) * mag_;

    if (accidentalCutOutX > maximumInset) {
        accidentalCutOutX = maximumInset;
    }

    RectF bbox = symBbox(furthestAccidental->symbol());
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
        return int(_arpeggioType);
    case Pid::TIME_STRETCH:
        return Stretch();
    case Pid::ARP_USER_LEN1:
        return userLen1();
    case Pid::ARP_USER_LEN2:
        return userLen2();
    case Pid::PLAY:
        return _playArpeggio;
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
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}
}
