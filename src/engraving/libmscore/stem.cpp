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

#include <cmath>

#include "stem.h"
#include "staff.h"
#include "chord.h"
#include "score.h"
#include "stafftype.h"
#include "hook.h"
#include "tremolo.h"
#include "note.h"
#include "xml.h"
#include "draw/brush.h"

// TEMPORARY HACK!!
#include "symid.h"
// END OF HACK

using namespace mu;
using namespace mu::draw;

namespace Ms {
static const ElementStyle stemStyle {
    { Sid::stemWidth,                          Pid::LINE_WIDTH },
};

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
    : Element(s)
{
    initElementStyle(&stemStyle);
    resetProperty(Pid::USER_LEN);
}

//---------------------------------------------------------
//   elementBase
//---------------------------------------------------------
Element* Stem::elementBase() const
{
    return parent();
}

//---------------------------------------------------------
//   vStaffIdx
//---------------------------------------------------------

int Stem::vStaffIdx() const
{
    return staffIdx() + chord()->staffMove();
}

//---------------------------------------------------------
//   up
//---------------------------------------------------------

bool Stem::up() const
{
    return chord() ? chord()->up() : true;
}

//---------------------------------------------------------
//   stemLen
//---------------------------------------------------------

qreal Stem::stemLen() const
{
    return up() ? -_len : _len;
}

//-------------------------------------------------------------------
//   layout
//    For beamed notes this is called twice. The final stem
//    length can only be calculated after stretching of the measure.
//    We need a guessed stem shape to calculate the minimal distance
//    between segments. The guessed stem must have at least the
//    right direction.
//-------------------------------------------------------------------

void Stem::layout()
{
    qreal l    = _len + _userLen;
    qreal _up  = up() ? -1.0 : 1.0;
    l         *= _up;

    qreal y1 = 0.0;                             // vertical displacement to match note attach point
    const Staff* stf = staff();
    if (chord()) {
        setMag(chord()->mag());
        Fraction tick = chord()->tick();
        const StaffType* st = stf ? stf->staffType(tick) : 0;
        if (st && st->isTabStaff()) {                 // TAB staves
            if (st->stemThrough()) {
                // if stems through staves, gets Y pos. of stem-side note relative to chord other side
                qreal lineDist = st->lineDistance().val() * spatium();
                y1             = (chord()->downString() - chord()->upString()) * _up * lineDist;
                // if fret marks above lines, raise stem beginning by 1/2 line distance
                if (!st->onLines()) {
                    y1 -= lineDist * 0.5;
                }
                // shorten stem by 1/2 lineDist to clear the note and a little more to keep 'air' between stem and note
                lineDist *= 0.7 * mag();
                y1       += _up * lineDist;
            }
            // in other TAB types, no correction
        } else {                                // non-TAB
            // move stem start to note attach point
            Note* n = up() ? chord()->downNote() : chord()->upNote();
            if ((up() && !n->mirror()) || (!up() && n->mirror())) {
                y1 += n->stemUpSE().y();
            } else {
                y1 += n->stemDownNW().y();
            }
            rypos() = n->rypos();
        }
    }

    qreal lw5 = 0.5 * lineWidthMag();

    line.setLine(0.0, y1, 0.0, l);

    // compute bounding rectangle
    RectF r(line.p1(), line.p2());
    setbbox(r.normalized().adjusted(-lw5, -lw5, lw5, lw5));
}

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Stem::setLen(qreal v)
{
    _len = (v < 0.0) ? -v : v;
    layout();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Stem::spatiumChanged(qreal oldValue, qreal newValue)
{
    _userLen = (_userLen / oldValue) * newValue;
    layout();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (!chord()) { // may be need assert?
        return;
    }

    // hide if second chord of a cross-measure pair
    if (chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    const Staff* st      = staff();
    const StaffType* stt = st ? st->staffType(chord()->tick()) : 0;
    bool useTab          = stt && stt->isTabStaff();

    painter->setPen(Pen(curColor(), lineWidthMag(), PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(line);

    if (!useTab) {
        return;
    }

    // TODO: adjust bounding rectangle in layout() for dots and for slash
    qreal sp = spatium();
    bool isUp = up();

    // slashed half note stem
    if (chord()->durationType().type() == TDuration::DurationType::V_HALF && stt->minimStyle() == TablatureMinimStyle::SLASHED) {
        // position slashes onto stem
        qreal y = isUp ? -(_len + _userLen) + STAFFTYPE_TAB_SLASH_2STARTY_UP * sp : (_len + _userLen) - STAFFTYPE_TAB_SLASH_2STARTY_DN * sp;
        // if stems through, try to align slashes within or across lines
        if (stt->stemThrough()) {
            qreal halfLineDist = stt->lineDistance().val() * sp * 0.5;
            qreal halfSlashHgt = STAFFTYPE_TAB_SLASH_2TOTHEIGHT * sp * 0.5;
            y = lrint((y + halfSlashHgt) / halfLineDist) * halfLineDist - halfSlashHgt;
        }
        // draw slashes
        qreal hlfWdt= sp * STAFFTYPE_TAB_SLASH_WIDTH * 0.5;
        qreal sln   = sp * STAFFTYPE_TAB_SLASH_SLANTY;
        qreal thk   = sp * STAFFTYPE_TAB_SLASH_THICK;
        qreal displ = sp * STAFFTYPE_TAB_SLASH_DISPL;
        PainterPath path;
        for (int i = 0; i < 2; ++i) {
            path.moveTo(hlfWdt, y);                   // top-right corner
            path.lineTo(hlfWdt, y + thk);             // bottom-right corner
            path.lineTo(-hlfWdt, y + thk + sln);      // bottom-left corner
            path.lineTo(-hlfWdt, y + sln);            // top-left corner
            path.closeSubpath();
            y += displ;
        }
        painter->setBrush(Brush(curColor()));
        painter->setNoPen();
        painter->drawPath(path);
    }

    // dots
    // NOT THE BEST PLACE FOR THIS?
    // with tablatures and stems beside staves, dots are not drawn near 'notes', but near stems
    int nDots = chord()->dots();
    if (nDots > 0 && !stt->stemThrough()) {
        qreal x     = chord()->dotPosX();
        qreal y     = ((STAFFTYPE_TAB_DEFAULTSTEMLEN_DN * 0.2) * sp) * (isUp ? -1.0 : 1.0);
        qreal step  = score()->styleS(Sid::dotDotDistance).val() * sp;
        for (int dot = 0; dot < nDots; dot++, x += step) {
            drawSymbol(SymId::augmentationDot, painter, PointF(x, y));
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Stem::write(XmlWriter& xml) const
{
    xml.stag(this);
    Element::writeProperties(xml);
    writeProperty(xml, Pid::USER_LEN);
    writeProperty(xml, Pid::LINE_WIDTH);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Stem::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    if (readProperty(tag, e, Pid::USER_LEN)) {
    } else if (readStyledProperty(e, tag)) {
    } else if (Element::readProperties(e)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> Stem::gripsPositions(const EditData&) const
{
    return { pagePos() + line.p2() };
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Stem::startEdit(EditData& ed)
{
    Element::startEdit(ed);
    ElementEditData* eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Stem::editDrag(EditData& ed)
{
    qreal yDelta = ed.delta.y();
    _userLen += up() ? -yDelta : yDelta;
    layout();
    Chord* c = chord();
    if (c->hook()) {
        c->hook()->move(PointF(0.0, ed.delta.y()));
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Stem::reset()
{
    undoChangeProperty(Pid::USER_LEN, 0.0);
    Element::reset();
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Stem::acceptDrop(EditData& data) const
{
    Element* e = data.dropElement;
    if ((e->type() == ElementType::TREMOLO) && (toTremolo(e)->tremoloType() <= TremoloType::R64)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Stem::drop(EditData& data)
{
    Element* e = data.dropElement;
    Chord* ch  = chord();

    switch (e->type()) {
    case ElementType::TREMOLO:
        e->setParent(ch);
        score()->undoAddElement(e);
        return e;
    default:
        delete e;
        break;
    }
    return 0;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Stem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return lineWidth();
    case Pid::USER_LEN:
        return userLen();
    case Pid::STEM_DIRECTION:
        return QVariant::fromValue<Direction>(chord()->stemDirection());
    default:
        return Element::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Stem::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        setLineWidth(v.toReal());
        break;
    case Pid::USER_LEN:
        setUserLen(v.toDouble());
        break;
    case Pid::STEM_DIRECTION:
        chord()->setStemDirection(v.value<Direction>());
        break;
    default:
        return Element::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Stem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::USER_LEN:
        return 0.0;
//            case Pid::LINE_WIDTH:
//                  return score()->styleP(Sid::stemWidth);
    case Pid::STEM_DIRECTION:
        return QVariant::fromValue<Direction>(Direction::AUTO);
    default:
        return Element::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   hookPos
//    in chord coordinates
//---------------------------------------------------------

PointF Stem::hookPos() const
{
    PointF p(pos() + line.p2());

    qreal xoff = 0.5 * lineWidthMag();
    p.rx() += xoff;
    return p;
}
}
