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

#include "articulation.h"

#include "style/style.h"
#include "types/symnames.h"
#include "types/typesconv.h"
#include "types/translatablestring.h"

#include "beam.h"
#include "chord.h"
#include "chordrest.h"
#include "measure.h"
#include "page.h"
#include "staff.h"
#include "stafftype.h"
#include "system.h"
#include "note.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace muse::draw;

//---------------------------------------------------------
//   articulationStyle
//---------------------------------------------------------

static const ElementStyle articulationStyle {
    { Sid::articulationMinDistance, Pid::MIN_DISTANCE },
//      { Sid::articulationOffset, Pid::OFFSET },
    { Sid::articulationAnchorDefault, Pid::ARTICULATION_ANCHOR },
};

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(ChordRest* parent, ElementType type)
    : EngravingItem(type, parent, ElementFlag::MOVABLE)
{
    m_symId         = SymId::noSym;
    m_anchor        = ArticulationAnchor::AUTO;
    m_direction     = DirectionV::AUTO;
    m_ornamentStyle = OrnamentStyle::DEFAULT;
    m_playArticulation = true;

    initElementStyle(&articulationStyle);
    setupShowOnTabStyles();
}

//---------------------------------------------------------
//   setSymId
//---------------------------------------------------------

void Articulation::setSymId(SymId id)
{
    m_symId  = id;
    computeCategories();
    setupShowOnTabStyles();
    m_anchor = ArticulationAnchor(propertyDefault(Pid::ARTICULATION_ANCHOR).toInt());
    m_textType = ArticulationTextType::NO_TEXT;
}

//---------------------------------------------------------
//   setTextType
//---------------------------------------------------------

void Articulation::setTextType(ArticulationTextType textType)
{
    m_textType = textType;
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Articulation::subtype() const
{
    if (m_textType != ArticulationTextType::NO_TEXT) {
        return int(m_textType);
    }
    String s = String::fromAscii(SymNames::nameForSymId(m_symId).ascii());
    if (s.endsWith(u"Below")) {
        return int(SymNames::symIdByName(s.left(s.size() - 5) + u"Above"));
    } else if (s.endsWith(u"Turned")) {
        return int(SymNames::symIdByName(s.left(s.size() - 6)));
    }

    return int(m_symId);
}

//---------------------------------------------------------
//   setUp
//---------------------------------------------------------

void Articulation::setUp(bool val)
{
    Articulation::LayoutData* ldata = mutldata();
    ldata->up = val;

    //! NOTE member of Articulation m_symId - this is `given` data
    //! member of LayoutData m_symId - this is layout data
    //! I would not like to change the `given` data here, but they are changing for backward compatibility
    //! Even better, I wouldn’t want symId to be `given` data,
    //! it would be better if there was some type,
    //! and from it we would already figure out how (with what symbol) to display it

    bool dup = m_direction == DirectionV::AUTO ? val : m_direction == DirectionV::UP;
    String s = String::fromAscii(SymNames::nameForSymId(m_symId).ascii());
    if (s.endsWith(!dup ? u"Above" : u"Below")) {
        String s2 = s.left(s.size() - 5) + (dup ? u"Above" : u"Below");
        m_symId = SymNames::symIdByName(s2);
    } else if (s.endsWith(u"Turned")) {
        String s2 = dup ? s.left(s.size() - 6) : s;
        m_symId = SymNames::symIdByName(s2);
    } else if (!dup) {
        String s2 = s + u"Turned";
        SymId sym = SymNames::symIdByName(s2);
        if (sym != SymId::noSym) {
            m_symId = sym;
        }
    }

    ldata->symId = m_symId;
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

muse::TranslatableString Articulation::typeUserName() const
{
    if (m_textType != ArticulationTextType::NO_TEXT) {
        return TranslatableString("engraving", "Articulation text");
    }

    return TranslatableString("engraving", "Articulation");
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Articulation::subtypeUserName() const
{
    if (m_textType != ArticulationTextType::NO_TEXT) {
        return TConv::userName(m_textType);
    }

    return SymNames::userNameForSymId(symId());
}

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Articulation::chordRest() const
{
    if (explicitParent() && explicitParent()->isChordRest()) {
        return toChordRest(explicitParent());
    }
    return 0;
}

Segment* Articulation::segment() const
{
    ChordRest* cr = chordRest();
    if (!cr) {
        return 0;
    }

    Segment* s = 0;
    if (cr->isGrace()) {
        if (cr->explicitParent()) {
            s = toSegment(cr->explicitParent()->explicitParent());
        }
    } else {
        s = toSegment(cr->explicitParent());
    }

    return s;
}

Measure* Articulation::measure() const
{
    Segment* s = segment();
    return toMeasure(s ? s->explicitParent() : 0);
}

System* Articulation::system() const
{
    Measure* m = measure();
    return toSystem(m ? m->explicitParent() : 0);
}

Page* Articulation::page() const
{
    System* s = system();
    return toPage(s ? s->explicitParent() : 0);
}

bool Articulation::isHiddenOnTabStaff() const
{
    if (m_showOnTabStyles.first == Sid::NOSTYLE || m_showOnTabStyles.second == Sid::NOSTYLE) {
        return false;
    }

    const StaffType* stType = staffType();

    if (!stType || !stType->isTabStaff()) {
        return false;
    }

    return stType->isHiddenElementOnTab(m_showOnTabStyles.first, m_showOnTabStyles.second);
}

//---------------------------------------------------------
//   layoutCloseToNote
//    Needed to figure out the layout policy regarding
//    distance to the note and placement in relation to
//    slur.
//---------------------------------------------------------

bool Articulation::layoutCloseToNote() const
{
    Staff* s = staff();
    if (s && s->staffType()->isTabStaff() && isStaccato()) {
        return false;
    }

    return (isStaccato() || isTenuto()) && !isDouble();
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> Articulation::dragAnchorLines() const
{
    std::vector<LineF> result;
    result.push_back(LineF(canvasPos(), parentItem()->canvasPos()));
    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Articulation::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:              return PropertyValue::fromValue(m_symId);
    case Pid::DIRECTION:           return PropertyValue::fromValue<DirectionV>(direction());
    case Pid::ARTICULATION_ANCHOR: return int(anchor());
    case Pid::ORNAMENT_STYLE:      return ornamentStyle();
    case Pid::PLAY:                return bool(playArticulation());
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Articulation::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SYMBOL:
        setSymId(v.value<SymId>());
        break;
    case Pid::DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::ARTICULATION_ANCHOR:
        setAnchor(ArticulationAnchor(v.toInt()));
        break;
    case Pid::PLAY:
        setPlayArticulation(v.toBool());
        break;
    case Pid::ORNAMENT_STYLE:
        setOrnamentStyle(v.value<OrnamentStyle>());
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

PropertyValue Articulation::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DIRECTION:
        return DirectionV::AUTO;

    case Pid::ORNAMENT_STYLE:
        return OrnamentStyle::DEFAULT;

    case Pid::PLAY:
        return true;

    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   anchorGroup
//---------------------------------------------------------

Articulation::AnchorGroup Articulation::anchorGroup(SymId symId)
{
    switch (symId) {
    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:

    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
    case SymId::articLaissezVibrerAbove:
    case SymId::articLaissezVibrerBelow:
    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
    case SymId::articStaccatissimoStrokeAbove:
    case SymId::articStaccatissimoStrokeBelow:
    case SymId::articStaccatissimoWedgeAbove:
    case SymId::articStaccatissimoWedgeBelow:
    case SymId::articStressAbove:
    case SymId::articStressBelow:
    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
    case SymId::articUnstressAbove:
    case SymId::articUnstressBelow:

    case SymId::articSoftAccentAbove:
    case SymId::articSoftAccentBelow:
    case SymId::articSoftAccentStaccatoAbove:
    case SymId::articSoftAccentStaccatoBelow:
    case SymId::articSoftAccentTenutoAbove:
    case SymId::articSoftAccentTenutoBelow:
    case SymId::articSoftAccentTenutoStaccatoAbove:
    case SymId::articSoftAccentTenutoStaccatoBelow:

    case SymId::wiggleSawtooth:
    case SymId::wiggleSawtoothWide:
    case SymId::wiggleVibratoLargeFaster:
    case SymId::wiggleVibratoLargeSlowest:
        return AnchorGroup::ARTICULATION;

    case SymId::luteFingeringRHThumb:
    case SymId::luteFingeringRHFirst:
    case SymId::luteFingeringRHSecond:
    case SymId::luteFingeringRHThird:
        return AnchorGroup::LUTE_FINGERING;

    case SymId::tremoloDivisiDots2:
    case SymId::tremoloDivisiDots3:
    case SymId::tremoloDivisiDots4:
    case SymId::tremoloDivisiDots6:
        return AnchorGroup::ARTICULATION;

    default:
        break;
    }
    return AnchorGroup::OTHER;
}

//---------------------------------------------------------
//   symId2ArticulationName
//---------------------------------------------------------

String Articulation::symId2ArticulationName(SymId symId)
{
    switch (symId) {
    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
    case SymId::articStaccatissimoStrokeAbove:
    case SymId::articStaccatissimoStrokeBelow:
    case SymId::articStaccatissimoWedgeAbove:
    case SymId::articStaccatissimoWedgeBelow:
        return u"staccatissimo";

    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
    case SymId::tremoloDivisiDots2:
    case SymId::tremoloDivisiDots3:
    case SymId::tremoloDivisiDots4:
    case SymId::tremoloDivisiDots6:
        return u"staccato";

    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
        return u"sforzatoStaccato";

    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
        return u"marcatoStaccato";

    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
        return u"portato";

    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
        return u"sforzatoTenuto";

    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
        return u"marcatoTenuto";

    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
        return u"tenuto";

    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:
        return u"marcato";

    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
        return u"accent";

    case SymId::dynamicSforzato:
    case SymId::dynamicSforzando:
        return u"sforzato";

    case SymId::brassMuteOpen:
        return u"open";

    case SymId::brassMuteClosed:
        return u"closed";

    case SymId::stringsHarmonic:
        return u"harmonic";

    case SymId::ornamentMordent:
        return u"mordent";

    default:
        return u"---";
    }
}

//---------------------------------------------------------
//   articulationName
//---------------------------------------------------------

String Articulation::articulationName() const
{
    return symId2ArticulationName(m_symId);
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid Articulation::getPropertyStyle(Pid id) const
{
    switch (id) {
    case Pid::MIN_DISTANCE:
        return EngravingItem::getPropertyStyle(id);

    case Pid::ARTICULATION_ANCHOR: {
        switch (anchorGroup(m_symId)) {
        case AnchorGroup::ARTICULATION:
            return Sid::articulationAnchorDefault;
        case AnchorGroup::LUTE_FINGERING:
            return Sid::articulationAnchorLuteFingering;
        case AnchorGroup::OTHER:
            return Sid::articulationAnchorOther;
        }
    }
        assert(false);           // should never be reached
    // fallthrough
    default:
        return Sid::NOSTYLE;
    }
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Articulation::resetProperty(Pid id)
{
    switch (id) {
    case Pid::DIRECTION:
    case Pid::ORNAMENT_STYLE:
        setProperty(id, propertyDefault(id));
        return;
    case Pid::ARTICULATION_ANCHOR:
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

double Articulation::mag() const
{
    return explicitParent() ? parentItem()->mag() * style().styleD(Sid::articulationMag) : 1.0;
}

void Articulation::computeCategories()
{
    m_categories.setFlag(ArticulationCategory::DOUBLE,
                         m_symId == SymId::articMarcatoStaccatoAbove || m_symId == SymId::articMarcatoStaccatoBelow
                         || m_symId == SymId::articTenutoStaccatoAbove || m_symId == SymId::articTenutoStaccatoBelow
                         || m_symId == SymId::articAccentStaccatoAbove || m_symId == SymId::articAccentStaccatoBelow
                         || m_symId == SymId::articMarcatoTenutoAbove || m_symId == SymId::articMarcatoTenutoBelow
                         || m_symId == SymId::articTenutoAccentAbove || m_symId == SymId::articTenutoAccentBelow);

    m_categories.setFlag(ArticulationCategory::TENUTO,
                         m_symId == SymId::articTenutoAbove || m_symId == SymId::articTenutoBelow
                         || m_symId == SymId::articMarcatoTenutoAbove || m_symId == SymId::articMarcatoTenutoBelow
                         || m_symId == SymId::articTenutoAccentAbove || m_symId == SymId::articTenutoAccentBelow);

    m_categories.setFlag(ArticulationCategory::STACCATO,
                         m_symId == SymId::articStaccatoAbove || m_symId == SymId::articStaccatoBelow
                         || m_symId == SymId::articMarcatoStaccatoAbove || m_symId == SymId::articMarcatoStaccatoBelow
                         || m_symId == SymId::articTenutoStaccatoAbove || m_symId == SymId::articTenutoStaccatoBelow
                         || m_symId == SymId::articAccentStaccatoAbove || m_symId == SymId::articAccentStaccatoBelow
                         || m_symId == SymId::tremoloDivisiDots2 || m_symId == SymId::tremoloDivisiDots3
                         || m_symId == SymId::tremoloDivisiDots4 || m_symId == SymId::tremoloDivisiDots6);

    m_categories.setFlag(ArticulationCategory::ACCENT,
                         m_symId == SymId::articAccentAbove || m_symId == SymId::articAccentBelow
                         || m_symId == SymId::articAccentStaccatoAbove || m_symId == SymId::articAccentStaccatoBelow);

    m_categories.setFlag(ArticulationCategory::MARCATO,
                         m_symId == SymId::articMarcatoAbove || m_symId == SymId::articMarcatoBelow
                         || m_symId == SymId::articMarcatoStaccatoAbove || m_symId == SymId::articMarcatoStaccatoBelow
                         || m_symId == SymId::articMarcatoTenutoAbove || m_symId == SymId::articMarcatoTenutoBelow);

    m_categories.setFlag(ArticulationCategory::LUTE_FINGERING,
                         m_symId == SymId::stringsThumbPosition || m_symId == SymId::luteFingeringRHThumb
                         || m_symId == SymId::luteFingeringRHFirst || m_symId == SymId::luteFingeringRHSecond
                         || m_symId == SymId::luteFingeringRHThird);
    m_categories.setFlag(ArticulationCategory::LAISSEZ_VIB,
                         m_symId == SymId::articLaissezVibrerAbove || m_symId == SymId::articLaissezVibrerBelow);
}

bool Articulation::isBasicArticulation() const
{
    static const std::set<SymId> articulations{
        SymId::articAccentAbove, SymId::articAccentBelow,
        SymId::articStaccatoAbove, SymId::articStaccatoBelow,
        SymId::articTenutoAbove, SymId::articTenutoBelow,
        SymId::articMarcatoAbove, SymId::articMarcatoBelow,

        SymId::articAccentStaccatoAbove, SymId::articAccentStaccatoBelow,
        SymId::articMarcatoStaccatoAbove, SymId::articMarcatoStaccatoBelow,
        SymId::articMarcatoTenutoAbove, SymId::articMarcatoTenutoBelow,
        SymId::articTenutoStaccatoAbove, SymId::articTenutoStaccatoBelow,
        SymId::articTenutoAccentAbove, SymId::articTenutoAccentBelow,
        SymId::articStaccatissimoAbove, SymId::articStaccatissimoBelow,
        SymId::articStaccatissimoStrokeAbove, SymId::articStaccatissimoStrokeBelow,
        SymId::articStaccatissimoWedgeAbove, SymId::articStaccatissimoWedgeBelow,
        SymId::articStressAbove, SymId::articStressBelow
    };
    SymId symId = static_cast<SymId>(subtype());
    return articulations.find(symId) != articulations.end();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Articulation::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

void Articulation::setupShowOnTabStyles()
{
    /// staccato
    if (isStaccato()) {
        m_showOnTabStyles = { Sid::staccatoShowTabCommon, Sid::staccatoShowTabSimple };
    }

    /// accent
    if (isAccent() || isMarcato()) {
        m_showOnTabStyles = { Sid::accentShowTabCommon, Sid::accentShowTabSimple };
    }

    /// turn
    if (m_symId == SymId::ornamentTurn || m_symId == SymId::ornamentTurnInverted) {
        m_showOnTabStyles = { Sid::turnShowTabCommon, Sid::turnShowTabSimple };
    }

    /// mordent
    if (m_symId == SymId::ornamentMordent || m_symId == SymId::ornamentShortTrill) {
        m_showOnTabStyles = { Sid::mordentShowTabCommon, Sid::mordentShowTabSimple };
    }

    /// wah
    if (m_symId == SymId::brassMuteOpen || m_symId == SymId::brassMuteClosed) {
        m_showOnTabStyles = { Sid::wahShowTabCommon, Sid::wahShowTabSimple };
    }

    /// golpe
    if (m_symId == SymId::guitarGolpe) {
        m_showOnTabStyles = { Sid::golpeShowTabCommon, Sid::golpeShowTabSimple };
    }
}

void Articulation::styleChanged()
{
    bool isGolpeThumb = m_symId == SymId::guitarGolpe && m_anchor == ArticulationAnchor::BOTTOM;
    EngravingItem::styleChanged();
    if (isGolpeThumb) {
        setAnchor(ArticulationAnchor::BOTTOM);
    }
}

bool Articulation::isOnCrossBeamSide() const
{
    ChordRest* cr = chordRest();
    if (!cr || !cr->isChord()) {
        return false;
    }
    Chord* chord = toChord(cr);
    return chord->beam() && (chord->beam()->cross() || chord->staffMove() != 0) && (up() == chord->up());
}

staff_idx_t Articulation::vStaffIdx() const
{
    ChordRest* cr = chordRest();
    if (!cr) {
        return staffIdx();
    }
    return cr->vStaffIdx();
}

struct ArticulationGroup
{
    SymId first;
    SymId second;

    bool operator <(const ArticulationGroup& other) const
    {
        if ((first == other.first && second == other.second)
            || (first == other.second && second == other.first)) {
            return false;
        }

        if (first != other.first) {
            return first < other.first;
        }

        return second < other.second;
    }
};

namespace mu::engraving {
static const std::map<SymId, ArticulationGroup> ARTICULATION_ABOVE_SPLIT_GROUPS = {
    { SymId::articAccentStaccatoAbove, { SymId::articStaccatoAbove, SymId::articAccentAbove } },
    { SymId::articTenutoAccentAbove, { SymId::articTenutoAbove, SymId::articAccentAbove } },
    { SymId::articTenutoStaccatoAbove, { SymId::articTenutoAbove, SymId::articStaccatoAbove } },
    { SymId::articMarcatoStaccatoAbove, { SymId::articStaccatoAbove, SymId::articMarcatoAbove } },
    { SymId::articMarcatoTenutoAbove, { SymId::articTenutoAbove, SymId::articMarcatoAbove } },
};

static const std::map<SymId, ArticulationGroup> ARTICULATION_BELOW_SPLIT_GROUP = {
    { SymId::articAccentStaccatoBelow, { SymId::articStaccatoBelow, SymId::articAccentBelow } },
    { SymId::articTenutoAccentBelow, { SymId::articTenutoBelow, SymId::articAccentBelow } },
    { SymId::articTenutoStaccatoBelow, { SymId::articTenutoBelow, SymId::articStaccatoBelow } },
    { SymId::articMarcatoStaccatoBelow, { SymId::articStaccatoBelow, SymId::articMarcatoBelow } },
    { SymId::articMarcatoTenutoBelow, { SymId::articTenutoBelow, SymId::articMarcatoBelow } },
};

static const std::map<ArticulationGroup, SymId> ARTICULATION_ABOVE_JOIN_GROUPS = {
    { { SymId::articStaccatoAbove, SymId::articAccentAbove }, SymId::articAccentStaccatoAbove },
    { { SymId::articTenutoAbove, SymId::articAccentAbove }, SymId::articTenutoAccentAbove },
    { { SymId::articTenutoAbove, SymId::articStaccatoAbove }, SymId::articTenutoStaccatoAbove },
    { { SymId::articStaccatoAbove, SymId::articMarcatoAbove }, SymId::articMarcatoStaccatoAbove },
    { { SymId::articTenutoAbove, SymId::articMarcatoAbove }, SymId::articMarcatoTenutoAbove },
};

static const std::map<SymId, SymId> ARTICULATION_PLACEMENTS = {
    { SymId::articStaccatissimoAbove, SymId::articStaccatissimoBelow },
    { SymId::articStaccatissimoStrokeAbove, SymId::articStaccatissimoStrokeBelow },
    { SymId::articStaccatissimoWedgeAbove, SymId::articStaccatissimoWedgeBelow },
    { SymId::articStaccatoAbove, SymId::articStaccatoBelow },
    { SymId::articAccentStaccatoAbove, SymId::articAccentStaccatoBelow },
    { SymId::articMarcatoStaccatoAbove, SymId::articMarcatoStaccatoBelow },
    { SymId::articTenutoStaccatoAbove, SymId::articTenutoStaccatoBelow },
    { SymId::articTenutoAccentAbove, SymId::articTenutoAccentBelow },
    { SymId::articMarcatoTenutoAbove, SymId::articMarcatoTenutoBelow },
    { SymId::articTenutoAbove, SymId::articTenutoBelow },
    { SymId::articMarcatoAbove, SymId::articMarcatoBelow },
    { SymId::articAccentAbove, SymId::articAccentBelow }
};

std::set<SymId> splitArticulations(const std::set<SymId>& articulationSymbolIds)
{
    std::set<SymId> result;
    for (const SymId& articulationSymbolId : articulationSymbolIds) {
        auto artic = ARTICULATION_ABOVE_SPLIT_GROUPS.find(articulationSymbolId);
        if (artic != ARTICULATION_ABOVE_SPLIT_GROUPS.end()) {
            // check above
            const ArticulationGroup& group = artic->second;
            result.insert(group.first);
            result.insert(group.second);
        } else {
            // check below
            artic = ARTICULATION_BELOW_SPLIT_GROUP.find(articulationSymbolId);
            if (artic != ARTICULATION_BELOW_SPLIT_GROUP.end()) {
                const ArticulationGroup& group = artic->second;
                result.insert(group.first);
                result.insert(group.second);
            } else {
                result.insert(articulationSymbolId);
            }
        }
    }

    return result;
}

std::set<SymId> joinArticulations(const std::set<SymId>& articulationSymbolIds)
{
    SymIdList result;

    SymIdList vsymbolIds(articulationSymbolIds.begin(), articulationSymbolIds.end());

    std::sort(vsymbolIds.begin(), vsymbolIds.end(), [](SymId l, SymId r) {
        return l > r;
    });

    std::set<SymId> splittedSymbols;

    auto symbolSelected = [&splittedSymbols](const SymId& symbolId) -> bool {
        return splittedSymbols.find(symbolId) != splittedSymbols.end();
    };

    for (size_t i = 0; i < vsymbolIds.size(); i++) {
        if (symbolSelected(vsymbolIds[i])) {
            continue;
        }

        bool found = false;
        for (size_t j = i + 1; j < vsymbolIds.size(); j++) {
            if (symbolSelected(vsymbolIds[i]) || symbolSelected(vsymbolIds[j])) {
                continue;
            }

            ArticulationGroup group = { vsymbolIds[i], vsymbolIds[j] };
            auto joinArticulation = ARTICULATION_ABOVE_JOIN_GROUPS.find(group);
            if (joinArticulation != ARTICULATION_ABOVE_JOIN_GROUPS.end()) {
                result.push_back(joinArticulation->second);
                splittedSymbols.insert(vsymbolIds[i]);
                splittedSymbols.insert(vsymbolIds[j]);
                found = true;
            }
        }

        if (!found) {
            result.push_back(vsymbolIds[i]);
            splittedSymbols.insert(vsymbolIds[i]);
        }
    }

    std::sort(result.begin(), result.end(), [](SymId l, SymId r) {
        return l < r;
    });

    return std::set<SymId>(result.begin(), result.end());
}

std::set<SymId> updateArticulations(const std::set<SymId>& articulationSymbolIds, SymId articulationSymbolId,
                                    ArticulationsUpdateMode updateMode)
{
    std::set<SymId> splittedArticulations = splitArticulations(articulationSymbolIds);

    switch (articulationSymbolId) {
    case SymId::articMarcatoAbove: {
        if (splittedArticulations.find(SymId::articAccentAbove) != splittedArticulations.end()) {
            splittedArticulations.erase(SymId::articAccentAbove);
            splittedArticulations.insert(SymId::articMarcatoAbove);
        } else if (splittedArticulations.find(SymId::articMarcatoAbove) != splittedArticulations.end()) {
            if (updateMode == ArticulationsUpdateMode::Remove) {
                splittedArticulations.erase(SymId::articMarcatoAbove);
            }
        } else {
            splittedArticulations.insert(SymId::articMarcatoAbove);
        }
        break;
    }
    case SymId::articAccentAbove: {
        if (splittedArticulations.find(SymId::articMarcatoAbove) != splittedArticulations.end()) {
            splittedArticulations.erase(SymId::articMarcatoAbove);
            splittedArticulations.insert(SymId::articAccentAbove);
        } else if (splittedArticulations.find(SymId::articAccentAbove) != splittedArticulations.end()) {
            if (updateMode == ArticulationsUpdateMode::Remove) {
                splittedArticulations.erase(SymId::articAccentAbove);
            }
        } else {
            splittedArticulations.insert(SymId::articAccentAbove);
        }
        break;
    }
    case SymId::articTenutoAbove: {
        if (splittedArticulations.find(SymId::articTenutoAbove) != splittedArticulations.end()) {
            if (updateMode == ArticulationsUpdateMode::Remove) {
                splittedArticulations.erase(SymId::articTenutoAbove);
            }
        } else {
            splittedArticulations.insert(SymId::articTenutoAbove);
        }
        break;
    }
    case SymId::articStaccatoAbove: {
        if (splittedArticulations.find(SymId::articStaccatoAbove) != splittedArticulations.end()) {
            if (updateMode == ArticulationsUpdateMode::Remove) {
                splittedArticulations.erase(SymId::articStaccatoAbove);
            }
        } else {
            splittedArticulations.insert(SymId::articStaccatoAbove);
        }
        break;
    }
    default:
        break;
    }
    // we no longer want to join articulations when adding or removing individual ones. combined articulations can still
    // be added by the palette (which doesn't perform any of this splitting)
    return splittedArticulations;
}

std::set<SymId> flipArticulations(const std::set<SymId>& articulationSymbolIds, PlacementV placement)
{
    std::set<SymId> result;
    switch (placement) {
    case PlacementV::ABOVE:
        for (const SymId& articulationSymbolId: articulationSymbolIds) {
            bool found = false;
            for (auto it = ARTICULATION_PLACEMENTS.begin(); it != ARTICULATION_PLACEMENTS.end(); ++it) {
                if (it->second == articulationSymbolId) {
                    result.insert(it->first);
                    found = true;
                    break;
                }
            }

            if (!found) {
                result.insert(articulationSymbolId);
            }
        }
        break;
    case PlacementV::BELOW:
        for (const SymId& articulationSymbolId: articulationSymbolIds) {
            bool found = false;
            for (auto it = ARTICULATION_PLACEMENTS.begin(); it != ARTICULATION_PLACEMENTS.end(); ++it) {
                if (it->first == articulationSymbolId) {
                    result.insert(it->second);
                    found = true;
                    break;
                }
            }

            if (!found) {
                result.insert(articulationSymbolId);
            }
        }
    }

    return result;
}
}
