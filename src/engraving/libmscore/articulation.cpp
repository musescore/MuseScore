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

#include "articulation.h"

#include "draw/fontmetrics.h"

#include "types/symnames.h"
#include "types/typesconv.h"
#include "types/translatablestring.h"
#include "layout/tlayout.h"

#include "beam.h"
#include "chord.h"
#include "chordrest.h"
#include "masterscore.h"
#include "measure.h"
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

Articulation::Articulation(ChordRest* parent)
    : EngravingItem(ElementType::ARTICULATION, parent, ElementFlag::MOVABLE)
{
    _symId         = SymId::noSym;
    _anchor        = ArticulationAnchor::TOP_STAFF;
    _direction     = DirectionV::AUTO;
    _up            = true;
    _ornamentStyle = OrnamentStyle::DEFAULT;

    m_font.setFamily(u"FreeSans", draw::Font::Type::Tablature);
    m_font.setPointSizeF(7.0);

    setPlayArticulation(true);
    initElementStyle(&articulationStyle);
    setupShowOnTabStyles();
}

//---------------------------------------------------------
//   setSymId
//---------------------------------------------------------

void Articulation::setSymId(SymId id)
{
    _symId  = id;
    computeCategories();
    setupShowOnTabStyles();
    _anchor = ArticulationAnchor(propertyDefault(Pid::ARTICULATION_ANCHOR).toInt());
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
    String s = String::fromAscii(SymNames::nameForSymId(_symId).ascii());
    if (s.endsWith(u"Below")) {
        return int(SymNames::symIdByName(s.left(s.size() - 5) + u"Above"));
    } else if (s.endsWith(u"Turned")) {
        return int(SymNames::symIdByName(s.left(s.size() - 6)));
    }

    return int(_symId);
}

//---------------------------------------------------------
//   setUp
//---------------------------------------------------------

void Articulation::setUp(bool val)
{
    _up = val;
    bool dup = _direction == DirectionV::AUTO ? val : _direction == DirectionV::UP;
    String s = String::fromAscii(SymNames::nameForSymId(_symId).ascii());
    if (s.endsWith(!dup ? u"Above" : u"Below")) {
        String s2 = s.left(s.size() - 5) + (dup ? u"Above" : u"Below");
        _symId = SymNames::symIdByName(s2);
    } else if (s.endsWith(u"Turned")) {
        String s2 = dup ? s.left(s.size() - 6) : s;
        _symId = SymNames::symIdByName(s2);
    } else if (!dup) {
        String s2 = s + u"Turned";
        SymId sym = SymNames::symIdByName(s2);
        if (sym != SymId::noSym) {
            _symId = sym;
        }
    }
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

TranslatableString Articulation::typeUserName() const
{
    if (m_textType != ArticulationTextType::NO_TEXT) {
        return TConv::userName(m_textType);
    }

    return TranslatableString("engraving/sym", SymNames::userNameForSymId(symId()));
}

String Articulation::translatedTypeUserName() const
{
    if (m_textType != ArticulationTextType::NO_TEXT) {
        return TConv::userName(m_textType).translated();
    }

    return SymNames::translatedUserNameForSymId(symId());
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;

    painter->setPen(curColor());

    if (m_textType != ArticulationTextType::NO_TEXT) {
        mu::draw::Font scaledFont(m_font);
        scaledFont.setPointSizeF(m_font.pointSizeF() * magS() * MScore::pixelRatio);
        painter->setFont(scaledFont);
        painter->drawText(bbox(), draw::TextDontClip | draw::AlignLeft | draw::AlignTop, TConv::text(m_textType));
    } else {
        drawSymbol(_symId, painter, PointF(-0.5 * width(), 0.0));
    }
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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Articulation::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
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

    return stType->isHiddenElementOnTab(score(), m_showOnTabStyles.first, m_showOnTabStyles.second);
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
    case Pid::SYMBOL:              return PropertyValue::fromValue(_symId);
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
    return symId2ArticulationName(_symId);
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
        switch (anchorGroup(_symId)) {
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
    return explicitParent() ? parentItem()->mag() * score()->styleD(Sid::articulationMag) : 1.0;
}

void Articulation::computeCategories()
{
    m_categories.setFlag(ArticulationCategory::DOUBLE,
                         _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
                         || _symId == SymId::articTenutoStaccatoAbove || _symId == SymId::articTenutoStaccatoBelow
                         || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow
                         || _symId == SymId::articMarcatoTenutoAbove || _symId == SymId::articMarcatoTenutoBelow
                         || _symId == SymId::articTenutoAccentAbove || _symId == SymId::articTenutoAccentBelow);

    m_categories.setFlag(ArticulationCategory::TENUTO,
                         _symId == SymId::articTenutoAbove || _symId == SymId::articTenutoBelow
                         || _symId == SymId::articMarcatoTenutoAbove || _symId == SymId::articMarcatoTenutoBelow
                         || _symId == SymId::articTenutoAccentAbove || _symId == SymId::articTenutoAccentBelow);

    m_categories.setFlag(ArticulationCategory::STACCATO,
                         _symId == SymId::articStaccatoAbove || _symId == SymId::articStaccatoBelow
                         || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
                         || _symId == SymId::articTenutoStaccatoAbove || _symId == SymId::articTenutoStaccatoBelow
                         || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow
                         || _symId == SymId::tremoloDivisiDots2 || _symId == SymId::tremoloDivisiDots3
                         || _symId == SymId::tremoloDivisiDots4 || _symId == SymId::tremoloDivisiDots6);

    m_categories.setFlag(ArticulationCategory::ACCENT,
                         _symId == SymId::articAccentAbove || _symId == SymId::articAccentBelow
                         || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow);

    m_categories.setFlag(ArticulationCategory::MARCATO,
                         _symId == SymId::articMarcatoAbove || _symId == SymId::articMarcatoBelow
                         || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
                         || _symId == SymId::articMarcatoTenutoAbove || _symId == SymId::articMarcatoTenutoBelow);

    m_categories.setFlag(ArticulationCategory::LUTE_FINGERING,
                         _symId == SymId::stringsThumbPosition || _symId == SymId::luteFingeringRHThumb
                         || _symId == SymId::luteFingeringRHFirst || _symId == SymId::luteFingeringRHSecond
                         || _symId == SymId::luteFingeringRHThird);
}

//---------------------------------------------------------
//   isOrnament
//---------------------------------------------------------

bool Articulation::isOrnament() const
{
    return isOrnament(subtype());
}

bool Articulation::isOrnament(int subtype)
{
    static const std::set<SymId> ornaments {
        SymId::ornamentTurn,
        SymId::ornamentTurnInverted,
        SymId::ornamentTurnSlash,
        SymId::ornamentTrill,
        SymId::brassMuteClosed,
        SymId::ornamentMordent,
        SymId::ornamentShortTrill,
        SymId::ornamentTremblement,
        SymId::ornamentPrallMordent,
        SymId::ornamentLinePrall,
        SymId::ornamentUpPrall,
        SymId::ornamentUpMordent,
        SymId::ornamentPrecompMordentUpperPrefix,
        SymId::ornamentDownMordent,
        SymId::ornamentPrallUp,
        SymId::ornamentPrallDown,
        SymId::ornamentPrecompSlide,
        SymId::ornamentShake3,
        SymId::ornamentShakeMuffat1,
        SymId::ornamentTremblementCouperin,
        SymId::ornamentPinceCouperin
    };

    SymId symId = static_cast<SymId>(subtype);

    return ornaments.find(symId) != ornaments.end();
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
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedTypeUserName());
}

//---------------------------------------------------------
//   doAutoplace
//    check for collisions
//---------------------------------------------------------

void Articulation::doAutoplace()
{
    // rebase vertical offset on drag
    double rebase = 0.0;
    if (offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset();
    }

    if (autoplace() && explicitParent()) {
        Segment* s = segment();
        Measure* m = measure();
        staff_idx_t si = staffIdx();

        double sp = score()->spatium();
        double md = minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        RectF r = bbox().translated(chordRest()->pos() + m->pos() + s->pos() + pos());

        double d;
        bool above = up();     // (anchor() == ArticulationAnchor::TOP_STAFF || anchor() == ArticulationAnchor::TOP_CHORD);
        SkylineLine sk(!above);
        if (above) {
            sk.add(r.x(), r.bottom(), r.width());
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(r.x(), r.top(), r.width());
            d = ss->skyline().south().minDistance(sk);
        }

        if (d > -md) {
            double yd = d + md;
            if (above) {
                yd *= -1.0;
            }
            if (offsetChanged() != OffsetChange::NONE) {
                // user moved element within the skyline
                // we may need to adjust minDistance, yd, and/or offset
                //bool inStaff = placeAbove() ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                if (rebaseMinDistance(md, yd, sp, rebase, above, true)) {
                    r.translate(0.0, rebase);
                }
            }
            movePosY(yd);
            r.translate(PointF(0.0, yd));
        }
    }
    setOffsetChanged(false);
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
    if (_symId == SymId::ornamentTurn || _symId == SymId::ornamentTurnInverted) {
        m_showOnTabStyles = { Sid::turnShowTabCommon, Sid::turnShowTabSimple };
    }

    /// mordent
    if (_symId == SymId::ornamentMordent || _symId == SymId::ornamentShortTrill) {
        m_showOnTabStyles = { Sid::mordentShowTabCommon, Sid::mordentShowTabSimple };
    }

    /// wah
    if (_symId == SymId::brassMuteOpen || _symId == SymId::brassMuteClosed) {
        m_showOnTabStyles = { Sid::wahShowTabCommon, Sid::wahShowTabSimple };
    }

    /// golpe
    if (_symId == SymId::guitarGolpe) {
        m_showOnTabStyles = { Sid::golpeShowTabCommon, Sid::golpeShowTabSimple };
    }
}

void Articulation::styleChanged()
{
    bool isGolpeThumb = _symId == SymId::guitarGolpe && _anchor == ArticulationAnchor::BOTTOM_STAFF;
    EngravingItem::styleChanged();
    if (isGolpeThumb) {
        setAnchor(ArticulationAnchor::BOTTOM_STAFF);
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

static std::map<SymId, ArticulationGroup> articulationAboveSplitGroups = {
    { SymId::articAccentStaccatoAbove, { SymId::articStaccatoAbove, SymId::articAccentAbove } },
    { SymId::articTenutoAccentAbove, { SymId::articTenutoAbove, SymId::articAccentAbove } },
    { SymId::articTenutoStaccatoAbove, { SymId::articTenutoAbove, SymId::articStaccatoAbove } },
    { SymId::articMarcatoStaccatoAbove, { SymId::articStaccatoAbove, SymId::articMarcatoAbove } },
    { SymId::articMarcatoTenutoAbove, { SymId::articTenutoAbove, SymId::articMarcatoAbove } },
};

static std::map<SymId, ArticulationGroup> articulationBelowSplitGroups = {
    { SymId::articAccentStaccatoBelow, { SymId::articStaccatoBelow, SymId::articAccentBelow } },
    { SymId::articTenutoAccentBelow, { SymId::articTenutoBelow, SymId::articAccentBelow } },
    { SymId::articTenutoStaccatoBelow, { SymId::articTenutoBelow, SymId::articStaccatoBelow } },
    { SymId::articMarcatoStaccatoBelow, { SymId::articStaccatoBelow, SymId::articMarcatoBelow } },
    { SymId::articMarcatoTenutoBelow, { SymId::articTenutoBelow, SymId::articMarcatoBelow } },
};

static std::map<ArticulationGroup, SymId> articulationAboveJoinGroups = {
    { { SymId::articStaccatoAbove, SymId::articAccentAbove }, SymId::articAccentStaccatoAbove },
    { { SymId::articTenutoAbove, SymId::articAccentAbove }, SymId::articTenutoAccentAbove },
    { { SymId::articTenutoAbove, SymId::articStaccatoAbove }, SymId::articTenutoStaccatoAbove },
    { { SymId::articStaccatoAbove, SymId::articMarcatoAbove }, SymId::articMarcatoStaccatoAbove },
    { { SymId::articTenutoAbove, SymId::articMarcatoAbove }, SymId::articMarcatoTenutoAbove },
};

static std::map<SymId, SymId> articulationPlacements = {
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
    for (const SymId& articulationSymbolId: articulationSymbolIds) {
        auto artic = articulationAboveSplitGroups.find(articulationSymbolId);
        if (artic != articulationAboveSplitGroups.end()) {
            // check above
            ArticulationGroup group = articulationAboveSplitGroups[articulationSymbolId];
            result.insert(group.first);
            result.insert(group.second);
        } else {
            // check below
            artic = articulationBelowSplitGroups.find(articulationSymbolId);
            if (artic != articulationBelowSplitGroups.end()) {
                ArticulationGroup group = articulationBelowSplitGroups[articulationSymbolId];
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
            auto joinArticulation = articulationAboveJoinGroups.find(group);
            if (joinArticulation != articulationAboveJoinGroups.end()) {
                result.push_back(articulationAboveJoinGroups.at(group));
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
            for (auto it = articulationPlacements.begin(); it != articulationPlacements.end(); ++it) {
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
            for (auto it = articulationPlacements.begin(); it != articulationPlacements.end(); ++it) {
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
