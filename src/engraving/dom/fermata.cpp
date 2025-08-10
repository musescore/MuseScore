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

#include "fermata.h"

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
    { Sid::fermataPosAbove, Pid::OFFSET },
    { Sid::fermataMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Fermata
//---------------------------------------------------------

Fermata::Fermata(EngravingItem* parent)
    : EngravingItem(ElementType::FERMATA, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    setPlacement(PlacementV::ABOVE);
    m_symId = SymId::noSym;
    setPlay(true);
    initElementStyle(&fermataStyle);
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Fermata::subtype() const
{
    String s = String::fromAscii(SymNames::nameForSymId(m_symId).ascii());
    if (s.endsWith(u"Below")) {
        return int(SymNames::symIdByName(s.left(s.size() - 5) + u"Above"));
    } else {
        return int(m_symId);
    }
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Fermata::subtypeUserName() const
{
    return SymNames::userNameForSymId(symId());
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Fermata::measure() const
{
    Segment* s = segment();
    return toMeasure(s ? s->explicitParent() : 0);
}

//---------------------------------------------------------
//   system
//---------------------------------------------------------

System* Fermata::system() const
{
    Measure* m = measure();
    return toSystem(m ? m->explicitParent() : 0);
}

//---------------------------------------------------------
//   page
//---------------------------------------------------------

Page* Fermata::page() const
{
    System* s = system();
    return toPage(s ? s->explicitParent() : 0);
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> Fermata::dragAnchorLines() const
{
    std::vector<LineF> result;
    result.push_back(LineF(canvasPos(), parentItem()->canvasPos()));
    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Fermata::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return PropertyValue::fromValue(m_symId);
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
    case Pid::SYMBOL:
        setSymIdAndTimeStretch(v.value<SymId>());
        break;
    case Pid::PLACEMENT: {
        PlacementV p = v.value<PlacementV>();
        if (p != placement()) {
            String s = String::fromAscii(SymNames::nameForSymId(m_symId).ascii());
            bool up = placeAbove();
            if (s.endsWith(up ? u"Above" : u"Below")) {
                String s2 = s.left(s.size() - 5) + (up ? u"Below" : u"Above");
                m_symId = SymNames::symIdByName(s2);
            }
            setPlacement(p);
        }
    }
    break;
    case Pid::PLAY:
        setPlay(v.toBool());
        break;
    case Pid::TIME_STRETCH:
        setTimeStretch(v.toDouble());
        score()->setUpTempoMapLater();
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
//   getPropertyStyle
//---------------------------------------------------------

Sid Fermata::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return placeAbove() ? Sid::fermataPosAbove : Sid::fermataPosBelow;
    }
    return EngravingObject::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Fermata::mag() const
{
    double m = staff() ? staff()->staffMag(tick()) * style().styleD(Sid::articulationMag) : 1.0;
    if (segment() && segment()->isChordRestType() && segment()->element(track())) {
        m *= toChordRest(segment()->element(track()))->mag();
    }
    return m;
}

void Fermata::setSymIdAndTimeStretch(SymId id)
{
    m_symId = id;
    m_timeStretch = m_timeStretch == -1 ? propertyDefault(Pid::TIME_STRETCH).value<double>() : -1;
}

FermataType Fermata::fermataType() const
{
    static const std::unordered_map<SymId, FermataType> FERMATA_TYPES = {
        { SymId::fermataAbove, FermataType::Normal },
        { SymId::fermataBelow, FermataType::Normal },
        { SymId::fermataLongAbove, FermataType::Long },
        { SymId::fermataLongBelow, FermataType::Long },
        { SymId::fermataLongHenzeAbove, FermataType::LongHenze },
        { SymId::fermataLongHenzeBelow, FermataType::LongHenze },
        { SymId::fermataVeryLongAbove, FermataType::VeryLong },
        { SymId::fermataVeryLongBelow, FermataType::VeryLong },
        { SymId::fermataShortHenzeAbove, FermataType::ShortHenze },
        { SymId::fermataShortHenzeBelow, FermataType::ShortHenze },
        { SymId::fermataVeryShortAbove, FermataType::VeryShort },
        { SymId::fermataVeryShortBelow, FermataType::VeryShort },
        { SymId::fermataShortAbove, FermataType::Short },
        { SymId::fermataShortBelow, FermataType::Short },
    };

    auto search = FERMATA_TYPES.find(symId());

    if (search != FERMATA_TYPES.cend()) {
        return search->second;
    }

    return FermataType::Undefined;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Fermata::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), SymNames::translatedUserNameForSymId(symId()));
}

void Fermata::added()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}

void Fermata::removed()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}
}
