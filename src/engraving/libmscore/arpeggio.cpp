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
#include <QVector>

#include "translation.h"
#include "rw/xml.h"

#include "scorefont.h"
#include "accidental.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "stafflines.h"
#include "part.h"
#include "page.h"
#include "segment.h"
#include "property.h"

using namespace mu;

namespace Ms {
const std::array<const char*, 6> Arpeggio::arpeggioTypeNames = {
    QT_TRANSLATE_NOOP("Palette", "Arpeggio"),
    QT_TRANSLATE_NOOP("Palette", "Up arpeggio"),
    QT_TRANSLATE_NOOP("Palette", "Down arpeggio"),
    QT_TRANSLATE_NOOP("Palette", "Bracket arpeggio"),
    QT_TRANSLATE_NOOP("Palette", "Up arpeggio straight"),
    QT_TRANSLATE_NOOP("Palette", "Down arpeggio straight")
};

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

QString Arpeggio::arpeggioTypeName() const
{
    return qtrc("Palette", arpeggioTypeNames[int(_arpeggioType)]);
}

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(qreal h)
{
    _height = h;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Arpeggio::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.startObject(this);
    EngravingItem::writeProperties(xml);
    writeProperty(xml, Pid::ARPEGGIO_TYPE);
    if (_userLen1 != 0.0) {
        xml.tag("userLen1", _userLen1 / spatium());
    }
    if (_userLen2 != 0.0) {
        xml.tag("userLen2", _userLen2 / spatium());
    }
    if (_span != 1) {
        xml.tag("span", _span);
    }
    writeProperty(xml, Pid::PLAY);
    writeProperty(xml, Pid::TIME_STRETCH);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            _arpeggioType = ArpeggioType(e.readInt());
        } else if (tag == "userLen1") {
            _userLen1 = e.readDouble() * spatium();
        } else if (tag == "userLen2") {
            _userLen2 = e.readDouble() * spatium();
        } else if (tag == "span") {
            _span = e.readInt();
        } else if (tag == "play") {
            _playArpeggio = e.readBool();
        } else if (tag == "timeStretch") {
            _stretch = e.readDouble();
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   symbolLine
//    construct a string of symbols approximating width w
//---------------------------------------------------------

void Arpeggio::symbolLine(SymId end, SymId fill)
{
    qreal top = calcTop();
    qreal bottom = calcBottom();
    qreal w   = bottom - top;
    qreal mag = magS();
    ScoreFont* f = score()->scoreFont();

    symbols.clear();
    qreal w1 = f->advance(end, mag);
    qreal w2 = f->advance(fill, mag);
    int n    = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        symbols.push_back(fill);
    }
    symbols.push_back(end);
}

//---------------------------------------------------------
//   calcTop
//---------------------------------------------------------

qreal Arpeggio::calcTop() const
{
    qreal top = -_userLen1;
    if (!explicitParent()) {
        return top;
    }
    switch (arpeggioType()) {
    case ArpeggioType::BRACKET: {
        qreal lineWidth = score()->styleMM(Sid::ArpeggioLineWidth);
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
//   calcBottom
//---------------------------------------------------------

qreal Arpeggio::calcBottom() const
{
    qreal top = -_userLen1;
    qreal bottom = _height + _userLen2;
    if (!explicitParent()) {
        return bottom;
    }
    switch (arpeggioType()) {
    case ArpeggioType::BRACKET: {
        qreal lineWidth = score()->styleMM(Sid::ArpeggioLineWidth);
        return bottom - top + lineWidth;
    }
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    case ArpeggioType::DOWN: {
        return bottom;
    }
    default: {
        return bottom + spatium() / 2;
    }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Arpeggio::layout()
{
    qreal top = calcTop();
    qreal bottom = calcBottom();
    _hidden = false;
    if (score()->styleB(Sid::ArpeggioHiddenInStdIfTab)) {
        if (staff() && staff()->isPitchedStaff(tick())) {
            for (Staff* s : staff()->staffList()) {
                if (s->score() == score() && s->isTabStaff(tick())) {
                    _hidden = true;
                    setbbox(RectF());
                    return;
                }
            }
        }
    }
    if (staff()) {
        setMag(staff()->staffMag(tick()));
    }
    switch (arpeggioType()) {
    case ArpeggioType::NORMAL: {
        symbolLine(SymId::wiggleArpeggiatoUp, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        RectF r(symBbox(symbols));
        setbbox(RectF(0.0, -r.x() + top, r.height(), r.width()));
    }
    break;

    case ArpeggioType::UP: {
        symbolLine(SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated -90 degrees
        RectF r(symBbox(symbols));
        setbbox(RectF(0.0, -r.x() + top, r.height(), r.width()));
    }
    break;

    case ArpeggioType::DOWN: {
        symbolLine(SymId::wiggleArpeggiatoUpArrow, SymId::wiggleArpeggiatoUp);
        // string is rotated +90 degrees (so that UpArrow turns into a DownArrow)
        RectF r(symBbox(symbols));
        setbbox(RectF(0.0, r.x() + top, r.height(), r.width()));
    }
    break;

    case ArpeggioType::UP_STRAIGHT: {
        qreal _spatium = spatium();
        qreal x1 = _spatium * .5;
        qreal w  = symBbox(SymId::arrowheadBlackUp).width();
        setbbox(RectF(x1 - w * .5, top, w, bottom));
    }
    break;

    case ArpeggioType::DOWN_STRAIGHT: {
        qreal _spatium = spatium();
        qreal x1 = _spatium * .5;
        qreal w  = symBbox(SymId::arrowheadBlackDown).width();
        setbbox(RectF(x1 - w * .5, top, w, bottom));
    }
    break;

    case ArpeggioType::BRACKET: {
        qreal _spatium = spatium();
        qreal w  = score()->styleS(Sid::ArpeggioHookLen).val() * _spatium;
        setbbox(RectF(0.0, top, w, bottom));
        break;
    }
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    if (_hidden) {
        return;
    }

    qreal _spatium = spatium();

    qreal y1 = _bbox.top();
    qreal y2 = _bbox.bottom();

    qreal lineWidth = score()->styleMM(Sid::ArpeggioLineWidth);

    painter->setPen(Pen(curColor(), lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->save();

    switch (arpeggioType()) {
    case ArpeggioType::NORMAL:
    case ArpeggioType::UP:
    {
        RectF r(symBbox(symbols));
        painter->rotate(-90.0);
        score()->scoreFont()->draw(symbols, painter, magS(), PointF(-r.right() - y1, -r.bottom() + r.height()));
    }
    break;

    case ArpeggioType::DOWN:
    {
        RectF r(symBbox(symbols));
        painter->rotate(90.0);
        score()->scoreFont()->draw(symbols, painter, magS(), PointF(-r.left() + y1, -r.top() - r.height()));
    }
    break;

    case ArpeggioType::UP_STRAIGHT:
    {
        RectF r(symBbox(SymId::arrowheadBlackUp));
        qreal x1 = _spatium * .5;
        drawSymbol(SymId::arrowheadBlackUp, painter, PointF(x1 - r.width() * .5, y1 - r.top()));
        y1 -= r.top() * .5;
        painter->drawLine(LineF(x1, y1, x1, y2));
    }
    break;

    case ArpeggioType::DOWN_STRAIGHT:
    {
        RectF r(symBbox(SymId::arrowheadBlackDown));
        qreal x1 = _spatium * .5;

        drawSymbol(SymId::arrowheadBlackDown, painter, PointF(x1 - r.width() * .5, y2 - r.bottom()));
        y2 += r.top() * .5;
        painter->drawLine(LineF(x1, y1, x1, y2));
    }
    break;

    case ArpeggioType::BRACKET:
    {
        qreal w = score()->styleS(Sid::ArpeggioHookLen).val() * _spatium;
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
    PointF p1(0.0, -_userLen1);
    PointF p2(0.0, _height + _userLen2);
    return { p1 + pp, p2 + pp };
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(EditData& ed)
{
    qreal d = ed.delta.y();
    if (ed.curGrip == Grip::START) {
        _userLen1 -= d;
    } else if (ed.curGrip == Grip::END) {
        _userLen2 += d;
    }
    layout();
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<LineF> Arpeggio::dragAnchorLines() const
{
    QVector<LineF> result;

    Chord* c = chord();
    if (c) {
        result << LineF(canvasPos(), c->upNote()->canvasPos());
    }
    return QVector<LineF>();
}

//---------------------------------------------------------
//   gripAnchorLines
//---------------------------------------------------------

QVector<LineF> Arpeggio::gripAnchorLines(Grip grip) const
{
    QVector<LineF> result;

    Chord* _chord = chord();
    if (!_chord) {
        return result;
    }

    const Page* p = toPage(findAncestor(ElementType::PAGE));
    const PointF pageOffset = p ? p->pos() : PointF();

    const PointF gripCanvasPos = gripsPositions()[static_cast<int>(grip)] + pageOffset;

    if (grip == Grip::START) {
        result << LineF(_chord->upNote()->canvasPos(), gripCanvasPos);
    } else if (grip == Grip::END) {
        Note* downNote = _chord->downNote();
        int btrack  = track() + (_span - 1) * VOICES;
        EngravingItem* e = _chord->segment()->element(btrack);
        if (e && e->isChord()) {
            downNote = toChord(e)->downNote();
        }
        result << LineF(downNote->canvasPos(), gripCanvasPos);
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
    if (ed.curGrip != Grip::END || !(ed.modifiers & Qt::ShiftModifier)) {
        return false;
    }

    return ed.key == Qt::Key_Down || ed.key == Qt::Key_Up;
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Arpeggio::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    if (ed.key == Qt::Key_Down) {
        Staff* s = staff();
        Part* part = s->part();
        int n = part->nstaves();
        int ridx = part->staves()->indexOf(s);
        if (ridx >= 0) {
            if (_span + ridx < n) {
                ++_span;
            }
        }
    } else if (ed.key == Qt::Key_Up) {
        if (_span > 1) {
            --_span;
        }
    }

    layout();
    Chord* c = chord();
    rxpos() = -(width() + spatium() * .5);
    c->layoutArpeggio2();
    return true;
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Arpeggio::spatiumChanged(qreal oldValue, qreal newValue)
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

qreal Arpeggio::insetTop() const
{
    qreal top = chord()->upNote()->y() - chord()->upNote()->height() / 2;

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

qreal Arpeggio::insetBottom() const
{
    qreal bottom = chord()->downNote()->y() + chord()->downNote()->height() / 2;

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

qreal Arpeggio::insetWidth() const
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

qreal Arpeggio::insetDistance(QVector<Accidental*>& accidentals, qreal mag_) const
{
    if (accidentals.size() == 0) {
        return 0.0;
    }

    qreal arpeggioTop = insetTop() * mag_;
    qreal arpeggioBottom = insetBottom() * mag_;
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
            qreal currentTop = accidental->note()->pos().y() + accidentalBbox.top() * mag_;
            qreal currentBottom = accidental->note()->pos().y() + accidentalBbox.bottom() * mag_;
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
    qreal accidentalCutOutX = symSmuflAnchor(furthestAccidental->symbol(), SmuflAnchorId::cutOutNW).x() * mag_;
    qreal accidentalCutOutYTop = symSmuflAnchor(furthestAccidental->symbol(), SmuflAnchorId::cutOutNW).y() * mag_;
    qreal accidentalCutOutYBottom = symSmuflAnchor(furthestAccidental->symbol(), SmuflAnchorId::cutOutSW).y() * mag_;

    qreal maximumInset = (score()->styleMM(Sid::ArpeggioAccidentalDistance)
                          - score()->styleMM(Sid::ArpeggioAccidentalDistanceMin)) * mag_;

    if (accidentalCutOutX > maximumInset) {
        accidentalCutOutX = maximumInset;
    }

    RectF bbox = symBbox(furthestAccidental->symbol());
    qreal center = furthestAccidental->note()->pos().y() * mag_;
    qreal top = center + bbox.top() * mag_;
    qreal bottom = center + bbox.bottom() * mag_;
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

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Arpeggio::propertyId(const QStringRef& name) const
{
    if (name == "subtype") {
        return Pid::ARPEGGIO_TYPE;
    }
    return EngravingItem::propertyId(name);
}
}
