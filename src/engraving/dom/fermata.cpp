/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "fermata.h"

#include "containers.h"

#include "types/symnames.h"
#include "types/translatablestring.h"

#include "chord.h"
#include "chordrest.h"
#include "measure.h"
#include "note.h"
#include "page.h"
#include "score.h"
#include "staff.h"
#include "stafftype.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   fermataStyle
//---------------------------------------------------------

static const ElementStyle fermataStyle {
    { Sid::fermataMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Fermata
//---------------------------------------------------------

Fermata::Fermata(DummyParentOr<Segment> parent)
    : EngravingItem(ElementType::FERMATA, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    setPlacement(PlacementV::ABOVE);
    setPlay(true);
    initElementStyle(&fermataStyle);
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Fermata::subtype() const
{
    return int(m_fermataType);
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Fermata::subtypeUserName() const
{
    return SymNames::userNameForSymId(symIdForFermataType(m_fermataType, placeAbove()));
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Fermata::measure() const
{
    Segment* s = segment();
    return toMeasure(s ? s->ownershipParent() : 0);
}

//---------------------------------------------------------
//   system
//---------------------------------------------------------

System* Fermata::system() const
{
    Measure* m = measure();
    return m ? m->system() : nullptr;
}

//---------------------------------------------------------
//   page
//---------------------------------------------------------

Page* Fermata::page() const
{
    System* s = system();
    return s ? s->page() : nullptr;
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> Fermata::dragAnchorLines() const
{
    std::vector<LineF> result;
    result.push_back(LineF(canvasPos(), layoutParent()->canvasPos()));
    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Fermata::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FERMATA_TYPE:
        return m_fermataType;
    case Pid::SYMBOL:
        return PropertyValue::fromValue(symIdForFermataType(m_fermataType, placeAbove()));
    case Pid::TIME_STRETCH:
        return timeStretch();
    case Pid::PLAY:
        return play();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fermata::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::FERMATA_TYPE:
        setFermataTypeAndTimeStretch(v.value<FermataType>());
        break;
    case Pid::SYMBOL:
        setFermataTypeAndTimeStretch(fermataTypeFromSymId(v.value<SymId>()));
        break;
    case Pid::PLAY:
        setPlay(v.toBool());
        break;
    case Pid::TIME_STRETCH:
        setTimeStretch(v.toDouble());
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Fermata::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PLACEMENT:
        return track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE;
    case Pid::TIME_STRETCH:
        switch (fermataType()) {
        case FermataType::VeryShort:
            return 1.25;
        case FermataType::Short:
        case FermataType::ShortHenze:
            return 1.5;
        case FermataType::Normal:
        case FermataType::Curlew:
        case FermataType::Undefined:
            return 2.0;
        case FermataType::Long:
        case FermataType::LongHenze:
            return 3.0;
        case FermataType::VeryLong:
            return 4.0;
        }
        break;
    case Pid::PLAY:
        return true;
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Fermata::resetProperty(Pid id)
{
    switch (id) {
    case Pid::TIME_STRETCH:
        setProperty(id, propertyDefault(id));
        return;

    default:
        break;
    }
    EngravingItem::resetProperty(id);
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Fermata::mag() const
{
    double m = staff() ? staff()->staffMag(tick()) * style().styleD(Sid::articulationMag) : 1.0;
    if (segment() && segment()->isChordRestType() && segment()->element(track())) {
        m *= toChordRest(segment()->element(track()))->intrinsicMag();
    }
    return m;
}

void Fermata::setFermataTypeAndTimeStretch(FermataType type)
{
    m_fermataType = type;
    m_timeStretch = m_timeStretch == -1 ? propertyDefault(Pid::TIME_STRETCH).value<double>() : -1;
}

struct FermataSymIds {
    SymId above = SymId::noSym;
    SymId below = SymId::noSym;
};

const std::unordered_map<FermataType, FermataSymIds> FERMATA_TYPES = {
    { FermataType::VeryShort,  { SymId::fermataVeryShortAbove,  SymId::fermataVeryShortBelow } },
    { FermataType::Short,      { SymId::fermataShortAbove,      SymId::fermataShortBelow } },
    { FermataType::ShortHenze, { SymId::fermataShortHenzeAbove, SymId::fermataShortHenzeBelow } },
    { FermataType::Normal,     { SymId::fermataAbove,           SymId::fermataBelow } },
    { FermataType::Long,       { SymId::fermataLongAbove,       SymId::fermataLongBelow } },
    { FermataType::LongHenze,  { SymId::fermataLongHenzeAbove,  SymId::fermataLongHenzeBelow } },
    { FermataType::VeryLong,   { SymId::fermataVeryLongAbove,   SymId::fermataVeryLongBelow } },
    { FermataType::Curlew,     { SymId::curlewSign,             SymId::curlewSign } },
};

SymId Fermata::symIdForFermataType(FermataType type, bool above)
{
    const FermataSymIds symIds = muse::value(FERMATA_TYPES, type, FermataSymIds());
    return above ? symIds.above : symIds.below;
}

FermataType Fermata::fermataTypeFromSymId(SymId id)
{
    const auto it = std::find_if(FERMATA_TYPES.cbegin(), FERMATA_TYPES.cend(), [id](const auto& pair) {
        return pair.second.above == id || pair.second.below == id;
    });

    return it != FERMATA_TYPES.cend() ? it->first : FermataType::Undefined;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Fermata::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(),
                                 SymNames::translatedUserNameForSymId(symIdForFermataType(m_fermataType, placeAbove())));
}

Sid Fermata::defaultPosSid() const
{
    return placeAbove() ? Sid::fermataPosAbove : Sid::fermataPosBelow;
}
}
