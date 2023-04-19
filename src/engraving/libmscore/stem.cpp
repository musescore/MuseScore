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
#include "stem.h"

#include <cmath>

#include "rw/xml.h"

#include "draw/types/brush.h"

#include "beam.h"
#include "chord.h"
#include "hook.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "stafftype.h"
#include "tremolo.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

static const ElementStyle stemStyle {
    { Sid::stemWidth, Pid::LINE_WIDTH }
};

Stem::Stem(Chord* parent)
    : EngravingItem(ElementType::STEM, parent)
{
    initElementStyle(&stemStyle);
    resetProperty(Pid::USER_LEN);
}

EngravingItem* Stem::elementBase() const
{
    return parentItem();
}

staff_idx_t Stem::vStaffIdx() const
{
    return staffIdx() + chord()->staffMove();
}

bool Stem::up() const
{
    return chord() ? chord()->up() : true;
}

void Stem::layout()
{
    const bool up = this->up();
    const double _up = up ? -1.0 : 1.0;

    double y1 = 0.0; // vertical displacement to match note attach point
    double y2 = _up * (m_baseLength + m_userLength);

    bool isTabStaff = false;
    if (chord()) {
        setMag(chord()->mag());

        const Staff* staff = this->staff();
        const StaffType* staffType = staff ? staff->staffTypeForElement(chord()) : nullptr;
        isTabStaff = staffType && staffType->isTabStaff();

        if (isTabStaff) {
            if (staffType->stemThrough()) {
                // if stems through staves, gets Y pos. of stem-side note relative to chord other side
                const double staffLinesDistance = staffType->lineDistance().val() * spatium();
                y1 = (chord()->downString() - chord()->upString()) * _up * staffLinesDistance;

                // if fret marks above lines, raise stem beginning by 1/2 line distance
                if (!staffType->onLines()) {
                    y1 -= staffLinesDistance * 0.5;
                }

                // shorten stem by 1/2 lineDist to clear the note and a little more to keep 'air' between stem and note
                y1 += _up * staffLinesDistance * 0.7;
            }
            // in other TAB types, no correction
        } else { // non-TAB
            // move stem start to note attach point
            Note* note = up ? chord()->downNote() : chord()->upNote();
            if ((up && !note->mirror()) || (!up && note->mirror())) {
                y1 = note->stemUpSE().y();
            } else {
                y1 = note->stemDownNW().y();
            }

            setPosY(note->ypos());
        }

        if (chord()->hook() && !chord()->beam()) {
            y2 += chord()->hook()->smuflAnchor().y();
        }

        if (chord()->beam()) {
            y2 -= _up * point(score()->styleS(Sid::beamWidth)) * .5 * chord()->beam()->mag();
        }
    }

    double lineWidthCorrection = lineWidthMag() * 0.5;
    double lineX = isTabStaff ? 0.0 : _up * lineWidthCorrection;
    m_line.setLine(lineX, y1, lineX, y2);

    // HACK: if there is a beam, extend the bounding box of the stem (NOT the stem itself) by half beam width.
    // This way the bbox of the stem covers also the beam position. Hugely helps with all the collision checks.
    double beamCorrection = (chord() && chord()->beam()) ? _up * score()->styleMM(Sid::beamWidth) * mag() / 2 : 0.0;
    // compute line and bounding rectangle
    RectF rect(m_line.p1(), m_line.p2() + PointF(0.0, beamCorrection));
    setbbox(rect.normalized().adjusted(-lineWidthCorrection, 0, lineWidthCorrection, 0));
}

void Stem::setBaseLength(Millimetre baseLength)
{
    m_baseLength = Millimetre(std::abs(baseLength.val()));
    layout();
}

void Stem::spatiumChanged(double oldValue, double newValue)
{
    m_userLength = (m_userLength / oldValue) * newValue;
    layout();
}

//! In chord coordinates
PointF Stem::flagPosition() const
{
    return pos() + PointF(_bbox.left(), up() ? -length() : length());
}

void Stem::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (!chord()) { // may be need assert?
        return;
    }

    // hide if second chord of a cross-measure pair
    if (chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    const Staff* staff = this->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(chord()) : nullptr;
    const bool isTablature = staffType && staffType->isTabStaff();

    painter->setPen(Pen(curColor(), lineWidthMag(), PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLine(m_line);

    if (!isTablature) {
        return;
    }

    // TODO: adjust bounding rectangle in layout() for dots and for slash
    double sp = spatium();
    bool isUp = up();

    // slashed half note stem
    if (chord()->durationType().type() == DurationType::V_HALF
        && staffType->minimStyle() == TablatureMinimStyle::SLASHED) {
        // position slashes onto stem
        double y = isUp ? -length() + STAFFTYPE_TAB_SLASH_2STARTY_UP * sp
                   : length() - STAFFTYPE_TAB_SLASH_2STARTY_DN * sp;
        // if stems through, try to align slashes within or across lines
        if (staffType->stemThrough()) {
            double halfLineDist = staffType->lineDistance().val() * sp * 0.5;
            double halfSlashHgt = STAFFTYPE_TAB_SLASH_2TOTHEIGHT * sp * 0.5;
            y = lrint((y + halfSlashHgt) / halfLineDist) * halfLineDist - halfSlashHgt;
        }
        // draw slashes
        double hlfWdt= sp * STAFFTYPE_TAB_SLASH_WIDTH * 0.5;
        double sln   = sp * STAFFTYPE_TAB_SLASH_SLANTY;
        double thk   = sp * STAFFTYPE_TAB_SLASH_THICK;
        double displ = sp * STAFFTYPE_TAB_SLASH_DISPL;
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
    if (nDots > 0 && !staffType->stemThrough()) {
        double x     = chord()->dotPosX();
        double y     = ((STAFFTYPE_TAB_DEFAULTSTEMLEN_DN * 0.2) * sp) * (isUp ? -1.0 : 1.0);
        double step  = score()->styleS(Sid::dotDotDistance).val() * sp;
        for (int dot = 0; dot < nDots; dot++, x += step) {
            drawSymbol(SymId::augmentationDot, painter, PointF(x, y));
        }
    }
}

std::vector<mu::PointF> Stem::gripsPositions(const EditData&) const
{
    return { pagePos() + m_line.p2() };
}

void Stem::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

void Stem::startEditDrag(EditData& ed)
{
    EngravingItem::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::USER_LEN);
}

void Stem::editDrag(EditData& ed)
{
    double yDelta = ed.delta.y();
    m_userLength += up() ? Millimetre(-yDelta) : Millimetre(yDelta);
    layout();
    Chord* c = chord();
    if (c->hook()) {
        c->hook()->move(PointF(0.0, yDelta));
    }
}

void Stem::reset()
{
    undoChangeProperty(Pid::USER_LEN, Millimetre(0.0));
    EngravingItem::reset();
}

bool Stem::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;
    if ((e->type() == ElementType::TREMOLO) && (toTremolo(e)->tremoloType() <= TremoloType::R64)) {
        return true;
    }
    return false;
}

EngravingItem* Stem::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    Chord* ch  = chord();

    switch (e->type()) {
    case ElementType::TREMOLO:
        toTremolo(e)->setParent(ch);
        undoAddElement(e);
        return e;
    default:
        delete e;
        break;
    }
    return 0;
}

PropertyValue Stem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        return lineWidth();
    case Pid::USER_LEN:
        return userLength();
    case Pid::STEM_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(chord()->stemDirection());
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

bool Stem::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::LINE_WIDTH:
        setLineWidth(v.value<Millimetre>());
        break;
    case Pid::USER_LEN:
        setUserLength(v.value<Millimetre>());
        break;
    case Pid::STEM_DIRECTION:
        chord()->setStemDirection(v.value<DirectionV>());
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

PropertyValue Stem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::USER_LEN:
        return 0.0;
    case Pid::STEM_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(DirectionV::AUTO);
    default:
        return EngravingItem::propertyDefault(id);
    }
}
