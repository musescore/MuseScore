/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "tremolosinglechord.h"

#include "draw/types/transform.h"

#include "types/typesconv.h"

#include "style/style.h"

#include "rendering/score/beamtremololayout.h"

#include "beam.h"
#include "chord.h"
#include "stem.h"
#include "system.h"
#include "stafftype.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
TremoloSingleChord::TremoloSingleChord(Chord* parent)
    : EngravingItem(ElementType::TREMOLO_SINGLECHORD, parent, ElementFlag::MOVABLE)
{
}

TremoloSingleChord::TremoloSingleChord(const TremoloSingleChord& t)
    : EngravingItem(t)
{
    setTremoloType(t.tremoloType());
    m_durationType = t.m_durationType;
}

TremoloSingleChord::~TremoloSingleChord()
{
    //
    // delete all references from chords
    //
    if (chord() && chord()->tremoloSingleChord() == this) {
        chord()->setTremoloSingleChord(nullptr);
    }
}

double TremoloSingleChord::chordMag() const
{
    return explicitParent() ? toChord(explicitParent())->intrinsicMag() : 1.0;
}

double TremoloSingleChord::minHeight() const
{
    const double sw = style().styleS(Sid::tremoloLineWidth).val() * chordMag();
    const double td = style().styleS(Sid::tremoloDistance).val() * chordMag();
    return (lines() - 1) * td + sw;
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF TremoloSingleChord::drag(EditData& ed)
{
    return EngravingItem::drag(ed);
}

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void TremoloSingleChord::setTremoloType(TremoloType t)
{
    m_tremoloType = t;
    switch (tremoloType()) {
    case TremoloType::R16:
    case TremoloType::C16:
        m_lines = 2;
        break;
    case TremoloType::R32:
    case TremoloType::C32:
        m_lines = 3;
        break;
    case TremoloType::R64:
    case TremoloType::C64:
        m_lines = 4;
        break;
    default:
        m_lines = 1;
        break;
    }

    styleChanged();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TremoloSingleChord::spatiumChanged(double oldValue, double newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void TremoloSingleChord::localSpatiumChanged(double oldValue, double newValue)
{
    EngravingItem::localSpatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void TremoloSingleChord::styleChanged()
{
    EngravingItem::styleChanged();
    computeShape();
}

staff_idx_t TremoloSingleChord::vStaffIdx() const
{
    return chord() ? chord()->vStaffIdx() : EngravingItem::vStaffIdx();
}

//---------------------------------------------------------
//   basePath
//---------------------------------------------------------

PainterPath TremoloSingleChord::basePath(double /*stretch*/) const
{
    if (isBuzzRoll()) {
        return PainterPath();
    }

    const double sp = spatium() * chordMag();

    double w2  = sp * style().styleS(Sid::tremoloWidth).val() * .5;
    double lw  = sp * style().styleS(Sid::tremoloLineWidth).val();
    double td  = sp * style().styleS(Sid::tremoloDistance).val();

    PainterPath ppath;

    // first line
    ppath.addRect(-w2, 0.0, 2.0 * w2, lw);
    double ty = td;

    // other lines
    for (int i = 1; i < m_lines; i++) {
        ppath.addRect(-w2, ty, 2.0 * w2, lw);
        ty += td;
    }

    Transform shearTransform;
    shearTransform.shear(0.0, -(lw / 2.0) / w2);
    ppath = shearTransform.map(ppath);

    return ppath;
}

void TremoloSingleChord::computeShape()
{
    if (isBuzzRoll()) {
        setbbox(symBbox(SymId::buzzRoll));
    } else {
        m_path = basePath();
        setbbox(m_path.boundingRect());
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void TremoloSingleChord::reset()
{
    EngravingItem::reset();
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    resetProperty(Pid::BEAM_NO_SLOPE);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF TremoloSingleChord::pagePos() const
{
    EngravingObject* e = explicitParent();
    while (e && (!e->isSystem() && e->explicitParent())) {
        e = e->explicitParent();
    }
    if (!e || !e->isSystem()) {
        return pos();
    }
    System* s = toSystem(e);
    double yp = y() + s->staff(vStaffIdx())->y() + s->y();
    return PointF(pageX(), yp);
}

TDuration TremoloSingleChord::durationType() const
{
    return m_durationType;
}

void TremoloSingleChord::setDurationType(TDuration d)
{
    if (m_durationType == d) {
        return;
    }

    m_durationType = d;
    styleChanged();
}

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction TremoloSingleChord::tremoloLen() const
{
    Fraction f;
    switch (lines()) {
    case 1: f.set(1, 8);
        break;
    case 2: f.set(1, 16);
        break;
    case 3: f.set(1, 32);
        break;
    case 4: f.set(1, 64);
        break;
    }
    return f;
}

void TremoloSingleChord::triggerLayout() const
{
    EngravingItem::triggerLayout();
}

bool TremoloSingleChord::needStartEditingAfterSelecting() const
{
    return false;
}

int TremoloSingleChord::gripsCount() const
{
    return 0;
}

Grip TremoloSingleChord::initialEditModeGrip() const
{
    return Grip::NO_GRIP;
}

Grip TremoloSingleChord::defaultGrip() const
{
    return Grip::NO_GRIP;
}

std::vector<PointF> TremoloSingleChord::gripsPositions(const EditData&) const
{
    return std::vector<PointF>();
}

void TremoloSingleChord::endEdit(EditData&)
{
    UNREACHABLE;
}

void TremoloSingleChord::editDrag(EditData&)
{
    UNREACHABLE;
}

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

TranslatableString TremoloSingleChord::subtypeUserName() const
{
    return TConv::userName(tremoloType());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TremoloSingleChord::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TremoloSingleChord::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TREMOLO_TYPE:
        return int(m_tremoloType);
    case Pid::TREMOLO_STYLE:
        UNREACHABLE;
        return int(-1);
    case Pid::PLAY:
        return m_playTremolo;
    default:
        break;
    }
    return EngravingItem::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TremoloSingleChord::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::TREMOLO_TYPE:
        setTremoloType(TremoloType(val.toInt()));
        break;
    case Pid::TREMOLO_STYLE:
        UNREACHABLE;
        break;
    case Pid::STEM_DIRECTION:
        UNREACHABLE;
        break;
    case Pid::USER_MODIFIED:
        UNREACHABLE;
        break;
    case Pid::BEAM_POS:
        UNREACHABLE;
        break;
    case Pid::PLAY:
        setPlayTremolo(val.toBool());
        break;
    default:
        return EngravingItem::setProperty(propertyId, val);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TremoloSingleChord::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TREMOLO_STYLE:
        return style().styleI(Sid::tremoloStyle);
    case Pid::PLAY:
        return true;
    default:
        return EngravingItem::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TremoloSingleChord::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (chord() && chord()->tremoloChordType() == TremoloChordType::TremoloSecondChord) {
        return;
    }
    EngravingItem::scanElements(data, func, all);
}
}
