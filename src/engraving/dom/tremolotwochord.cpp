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

#include "tremolotwochord.h"

#include "draw/types/transform.h"

#include "types/typesconv.h"

#include "style/style.h"

#include "rendering/score/beamtremololayout.h"

#include "chord.h"
#include "stem.h"
#include "system.h"
#include "stafftype.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   tremoloStyle
//---------------------------------------------------------

static const ElementStyle TREMOLO_STYLE {
    { Sid::tremoloStyle, Pid::TREMOLO_STYLE }
};

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

TremoloTwoChord::TremoloTwoChord(Chord* parent)
    : BeamBase(ElementType::TREMOLO_TWOCHORD, parent, ElementFlag::MOVABLE)
{
    initElementStyle(&TREMOLO_STYLE);
}

TremoloTwoChord::TremoloTwoChord(const TremoloTwoChord& t)
    : BeamBase(t)
{
    setTremoloType(t.tremoloType());
    m_chord1       = t.chord1();
    m_chord2       = t.chord2();
    m_durationType = t.m_durationType;
}

TremoloTwoChord::~TremoloTwoChord()
{
    //
    // delete all references from chords
    //
    if (m_chord1 && m_chord1->tremoloTwoChord() == this) {
        m_chord1->setTremoloTwoChord(nullptr);
    }
    if (m_chord2 && m_chord2->tremoloTwoChord() == this) {
        m_chord2->setTremoloTwoChord(nullptr);
    }

    clearBeamSegments();
}

//---------------------------------------------------------
//   chordMag
//---------------------------------------------------------

double TremoloTwoChord::chordMag() const
{
    return explicitParent() ? toChord(explicitParent())->intrinsicMag() : 1.0;
}

//---------------------------------------------------------
//   minHeight
//---------------------------------------------------------

double TremoloTwoChord::minHeight() const
{
    const double sw = style().styleS(Sid::tremoloLineWidth).val() * chordMag();
    const double td = style().styleS(Sid::tremoloDistance).val() * chordMag();
    return (lines() - 1) * td + sw;
}

//---------------------------------------------------------
//   chordBeamAnchor
//---------------------------------------------------------

PointF TremoloTwoChord::chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const
{
    return rendering::score::BeamTremoloLayout::chordBeamAnchor(this->ldata(), chord, anchorType);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF TremoloTwoChord::drag(EditData& ed)
{
    int idx = directionIdx();
    double dy = ed.pos.y() - ed.lastPos.y();

    double y1 = m_beamFragment.py1[idx];
    double y2 = m_beamFragment.py2[idx];

    y1 += dy;
    y2 += dy;

    double _spatium = spatium();
    // Because of the logic in TremoloTwoChord::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();

    return canvasBoundingRect();
}

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void TremoloTwoChord::setTremoloType(TremoloType t)
{
    IF_ASSERT_FAILED(t >= TremoloType::C8) {
        return;
    }
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
//   reset
//---------------------------------------------------------

void TremoloTwoChord::reset()
{
    EngravingItem::reset();
    if (userModified()) {
        //undoChangeProperty(Pid::BEAM_POS, PropertyValue::fromValue(beamPos()));
        undoChangeProperty(Pid::USER_MODIFIED, false);
    }
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    resetProperty(Pid::BEAM_NO_SLOPE);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF TremoloTwoChord::pagePos() const
{
    EngravingObject* e = explicitParent();
    while (e && (!e->isSystem() && e->explicitParent())) {
        e = e->explicitParent();
    }
    if (!e || !e->isSystem()) {
        return pos();
    }
    System* s = toSystem(e);
    double yp = y() + s->staff(staffIdx())->y() + s->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void TremoloTwoChord::setDirection(DirectionV d)
{
    if (direction() == d) {
        return;
    }

    doSetDirection(d);

    if (d != DirectionV::AUTO) {
        setUp(d == DirectionV::UP);
    }
    if (m_chord1) {
        m_chord1->setStemDirection(d);
    }
    if (m_chord2) {
        m_chord2->setStemDirection(d);
    }
}

//---------------------------------------------------------
//   crossStaffBeamBetween
//    Return true if tremolo is two-note cross-staff and beams between staves
//---------------------------------------------------------

bool TremoloTwoChord::crossStaffBeamBetween() const
{
    return ((m_chord1->staffMove() > m_chord2->staffMove()) && m_chord1->up() && !m_chord2->up())
           || ((m_chord1->staffMove() < m_chord2->staffMove()) && !m_chord1->up() && m_chord2->up());
}

TDuration TremoloTwoChord::durationType() const
{
    return m_durationType;
}

void TremoloTwoChord::setDurationType(TDuration d)
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

Fraction TremoloTwoChord::tremoloLen() const
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

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void TremoloTwoChord::setBeamPos(const PairF& bp)
{
    int idx = directionIdx();
    setUserModified(true);
    setGenerated(false);

    double _spatium = spatium();
    m_beamFragment.py1[idx] = bp.first * _spatium;
    m_beamFragment.py2[idx] = bp.second * _spatium;
}

//---------------------------------------------------------
//   beamPos
//---------------------------------------------------------

PairF TremoloTwoChord::beamPos() const
{
    int idx = directionIdx();
    double _spatium = spatium();
    return PairF(m_beamFragment.py1[idx] / _spatium, m_beamFragment.py2[idx] / _spatium);
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void TremoloTwoChord::triggerLayout() const
{
    if (m_chord1 && m_chord2) {
        toChordRest(m_chord1)->triggerLayout();
        toChordRest(m_chord2)->triggerLayout();
    } else {
        EngravingItem::triggerLayout();
    }
}

bool TremoloTwoChord::needStartEditingAfterSelecting() const
{
    return true;
}

int TremoloTwoChord::gripsCount() const
{
    return 3;
}

Grip TremoloTwoChord::initialEditModeGrip() const
{
    return Grip::END;
}

Grip TremoloTwoChord::defaultGrip() const
{
    return Grip::MIDDLE;
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> TremoloTwoChord::gripsPositions(const EditData&) const
{
    int idx = directionIdx();

    int y = pagePos().y();
    double beamStartX = m_startAnchor.x() + m_chord1->pageX();
    double beamEndX = m_endAnchor.x() + m_chord1->pageX(); // intentional--chord1 is start x
    double middleX = (beamStartX + beamEndX) / 2;
    double middleY = (m_beamFragment.py1[idx] + y + m_beamFragment.py2[idx] + y) / 2;

    return {
        PointF(beamStartX, m_beamFragment.py1[idx] + y),
        PointF(beamEndX, m_beamFragment.py2[idx] + y),
        PointF(middleX, middleY)
    };
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void TremoloTwoChord::editDrag(EditData& ed)
{
    int idx = directionIdx();
    double dy = ed.delta.y();
    double y1 = m_beamFragment.py1[idx];
    double y2 = m_beamFragment.py2[idx];

    if (ed.curGrip == Grip::MIDDLE) {
        y1 += dy;
        y2 += dy;
    } else if (ed.curGrip == Grip::START) {
        y1 += dy;
    } else if (ed.curGrip == Grip::END) {
        y2 += dy;
    }

    double _spatium = spatium();
    // Because of the logic in Beam::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();
}

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

TranslatableString TremoloTwoChord::subtypeUserName() const
{
    return TConv::userName(tremoloType());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TremoloTwoChord::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

//---------------------------------------------------------
//   customStyleApplicable
//---------------------------------------------------------

bool TremoloTwoChord::customStyleApplicable() const
{
    return (durationType().type() == DurationType::V_HALF)
           && (staffType()->group() != StaffGroup::TAB);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TremoloTwoChord::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TREMOLO_TYPE:
        return int(m_tremoloType);
    case Pid::TREMOLO_STYLE:
        return int(m_style);
    case Pid::PLAY:
        return m_playTremolo;
    default:
        break;
    }
    return BeamBase::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TremoloTwoChord::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::TREMOLO_TYPE:
        setTremoloType(TremoloType(val.toInt()));
        break;
    case Pid::TREMOLO_STYLE:
        if (customStyleApplicable()) {
            setTremoloStyle(TremoloStyle(val.toInt()));
        }
        break;
    case Pid::BEAM_POS:
        if (userModified()) {
            setBeamPos(val.value<PairF>());
        }
        break;
    case Pid::PLAY:
        setPlayTremolo(val.toBool());
        break;
    default:
        return BeamBase::setProperty(propertyId, val);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TremoloTwoChord::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TREMOLO_STYLE:
        return style().styleI(Sid::tremoloStyle);
    case Pid::PLAY:
        return true;
    default:
        return BeamBase::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TremoloTwoChord::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (chord() && chord()->tremoloChordType() == TremoloChordType::TremoloSecondChord) {
        return;
    }
    EngravingItem::scanElements(data, func, all);
}

void TremoloTwoChord::clearBeamSegments()
{
    BeamSegment* chord1Segment = m_chord1 ? m_chord1->beamlet() : nullptr;
    BeamSegment* chord2Segment = m_chord2 ? m_chord2->beamlet() : nullptr;

    if (chord1Segment || chord2Segment) {
        for (BeamSegment* segment : m_beamSegments) {
            if (chord1Segment && chord1Segment == segment) {
                m_chord1->setBeamlet(nullptr);
            } else if (chord2Segment && chord2Segment == segment) {
                m_chord2->setBeamlet(nullptr);
            }
        }
    }

    BeamBase::clearBeamSegments();
}

int TremoloTwoChord::maxCRMove() const
{
    return std::max(m_chord1->staffMove(), m_chord2->staffMove());
}

int TremoloTwoChord::minCRMove() const
{
    return std::min(m_chord1->staffMove(), m_chord2->staffMove());
}

// used for palettes
PainterPath TremoloTwoChord::basePath(double stretch) const
{
    bool tradAlternate = m_style == TremoloStyle::TRADITIONAL_ALTERNATE;
    if (tradAlternate && muse::RealIsEqual(stretch, 0.)) {
        // this shape will have to be constructed after the stretch
        // is known
        return PainterPath();
    }

    // TODO: This should be a style setting, to replace tremoloStrokeLengthMultiplier
    static constexpr double stemGapSp = 0.65;

    const double sp = spatium() * chordMag();

    // overall width of two-note tremolos should not be changed if chordMag() isn't 1.0
    double w2  = sp * style().styleS(Sid::tremoloWidth).val() * .5 / chordMag();
    double lw  = sp * style().styleS(Sid::tremoloLineWidth).val();
    double td  = sp * style().styleS(Sid::tremoloDistance).val();

    PainterPath ppath;

    // first line
    ppath.addRect(-w2, 0.0, 2.0 * w2, lw);
    double ty = td;

    // other lines
    for (int i = 1; i < m_lines; i++) {
        if (tradAlternate) {
            double stemWidth1 = m_chord1->stem()->lineWidthMag() / stretch;
            double stemWidth2 = m_chord2->stem()->lineWidthMag() / stretch;
            double inset = (stemGapSp * spatium()) / stretch;

            ppath.addRect(-w2 + inset + stemWidth1, ty,
                          2.0 * w2 - (inset * 2.) - (stemWidth2 + stemWidth1), lw);
        } else {
            ppath.addRect(-w2, ty, 2.0 * w2, lw);
        }
        ty += td;
    }

    if (!explicitParent()) {
        // for the palette or for one-note tremolos
        Transform shearTransform;
        shearTransform.shear(0.0, -(lw / 2.0) / w2);
        ppath = shearTransform.map(ppath);
    }

    return ppath;
}

void TremoloTwoChord::computeShape()
{
    m_path = basePath();
    setbbox(m_path.boundingRect());
}
}
