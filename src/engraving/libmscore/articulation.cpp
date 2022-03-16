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

#include "rw/compat/read206.h"
#include "rw/xml.h"
#include "types/symnames.h"

#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"
#include "page.h"
#include "barline.h"
#include "masterscore.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
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
    setPlayArticulation(true);
    initElementStyle(&articulationStyle);
}

//---------------------------------------------------------
//   setSymId
//---------------------------------------------------------

void Articulation::setSymId(SymId id)
{
    _symId  = id;
    _anchor = ArticulationAnchor(propertyDefault(Pid::ARTICULATION_ANCHOR).toInt());
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Articulation::subtype() const
{
    QString s = SymNames::nameForSymId(_symId);
    if (s.endsWith("Below")) {
        return int(SymNames::symIdByName(s.left(s.size() - 5) + "Above"));
    } else if (s.endsWith("Turned")) {
        return int(SymNames::symIdByName(s.left(s.size() - 6)));
    } else {
        return int(_symId);
    }
}

//---------------------------------------------------------
//   setUp
//---------------------------------------------------------

void Articulation::setUp(bool val)
{
    _up = val;
    bool dup = _direction == DirectionV::AUTO ? val : _direction == DirectionV::UP;
    QString s = SymNames::nameForSymId(_symId);
    if (s.endsWith(!dup ? "Above" : "Below")) {
        QString s2 = s.left(s.size() - 5) + (dup ? "Above" : "Below");
        _symId = SymNames::symIdByName(s2);
    } else if (s.endsWith("Turned")) {
        QString s2 = dup ? s.left(s.size() - 6) : s;
        _symId = SymNames::symIdByName(s2);
    } else if (!dup) {
        QString s2 = s + "Turned";
        SymId sym = SymNames::symIdByName(s2);
        if (sym != SymId::noSym) {
            _symId = sym;
        }
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(XmlReader& e)
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

bool Articulation::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    if (tag == "subtype") {
        QString s = e.readElementText();
        SymId id = SymNames::symIdByName(s);
        if (id == SymId::noSym) {
            id = compat::Read206::articulationNames2SymId206(s); // compatibility hack for "old" 3.0 scores
        }
        if (id == SymId::noSym || s == "ornamentMordentInverted") {   // SMuFL < 1.30
            id = SymId::ornamentMordent;
        }

        QString programVersion = masterScore()->mscoreVersion();
        if (!programVersion.isEmpty() && programVersion < "3.6") {
            if (id == SymId::noSym || s == "ornamentMordent") {   // SMuFL < 1.30 and MuseScore < 3.6
                id = SymId::ornamentShortTrill;
            }
        }
        setSymId(id);
    } else if (tag == "channel") {
        _channelName = e.attribute("name");
        e.readNext();
    } else if (readProperty(tag, e, Pid::ARTICULATION_ANCHOR)) {
    } else if (tag == "direction") {
        readProperty(e, Pid::DIRECTION);
    } else if (tag == "ornamentStyle") {
        readProperty(e, Pid::ORNAMENT_STYLE);
    } else if (tag == "play") {
        setPlayArticulation(e.readBool());
    } else if (tag == "offset") {
        if (score()->mscVersion() > 114) {
            EngravingItem::readProperties(e);
        } else {
            e.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (EngravingItem::readProperties(e)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.startObject(this);
    if (!_channelName.isEmpty()) {
        xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
    }
    writeProperty(xml, Pid::DIRECTION);
    xml.tag("subtype", SymNames::nameForSymId(_symId));
    writeProperty(xml, Pid::PLAY);
    writeProperty(xml, Pid::ORNAMENT_STYLE);
    for (const StyledProperty& spp : *styledProperties()) {
        writeProperty(xml, spp.pid);
    }
    EngravingItem::writeProperties(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

QString Articulation::typeUserName() const
{
    return SymNames::translatedUserNameForSymId(symId());
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;

    painter->setPen(curColor());
    drawSymbol(_symId, painter, PointF(-0.5 * width(), 0.0));
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
    RectF b(symBbox(_symId));
    setbbox(b.translated(-0.5 * b.width(), 0.0));
}

//---------------------------------------------------------
//   layoutCloseToNote
//    Needed to figure out the layout policy regarding
//    distance to the note and placement in relation to
//    slur.
//---------------------------------------------------------

bool Articulation::layoutCloseToNote() const
{
    return (isStaccato() || isTenuto()) && !isDouble();
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<mu::LineF> Articulation::dragAnchorLines() const
{
    QVector<LineF> result;
    result << LineF(canvasPos(), parentItem()->canvasPos());
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

    case SymId::guitarFadeIn:
    case SymId::guitarFadeOut:
    case SymId::guitarVolumeSwell:
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

    default:
        break;
    }
    return AnchorGroup::OTHER;
}

//---------------------------------------------------------
//   symId2ArticulationName
//---------------------------------------------------------

const char* Articulation::symId2ArticulationName(SymId symId)
{
    switch (symId) {
    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
    case SymId::articStaccatissimoStrokeAbove:
    case SymId::articStaccatissimoStrokeBelow:
    case SymId::articStaccatissimoWedgeAbove:
    case SymId::articStaccatissimoWedgeBelow:
        return "staccatissimo";

    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
        return "staccato";

    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
        return "sforzatoStaccato";

    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
        return "marcatoStaccato";

    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
        return "portato";

    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
        return "sforzatoTenuto";

    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
        return "marcatoTenuto";

    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
        return "tenuto";

    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:
        return "marcato";

    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
        return "sforzato";

    case SymId::brassMuteOpen:
        return "open";

    case SymId::brassMuteClosed:
        return "closed";

    case SymId::stringsHarmonic:
        return "harmonic";

    case SymId::ornamentMordent:
        return "mordent";

    default:
        return "---";
    }
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Articulation::propertyId(const QStringRef& xmlName) const
{
    if (xmlName == "subtype") {
        return Pid::SYMBOL;
    }
    return EngravingItem::propertyId(xmlName);
}

//---------------------------------------------------------
//   articulationName
//---------------------------------------------------------

const char* Articulation::articulationName() const
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
        Q_ASSERT(false);           // should never be reached
        Q_FALLTHROUGH();
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

qreal Articulation::mag() const
{
    return explicitParent() ? parentItem()->mag() * score()->styleD(Sid::articulationMag) : 1.0;
}

bool Articulation::isTenuto() const
{
    return _symId == SymId::articTenutoAbove || _symId == SymId::articTenutoBelow;
}

bool Articulation::isStaccato() const
{
    return _symId == SymId::articStaccatoAbove || _symId == SymId::articStaccatoBelow
           || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
           || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow;
}

bool Articulation::isAccent() const
{
    return _symId == SymId::articAccentAbove || _symId == SymId::articAccentBelow
           || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow;
}

bool Articulation::isMarcato() const
{
    return _symId == SymId::articMarcatoAbove || _symId == SymId::articMarcatoBelow
           || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
           || _symId == SymId::articMarcatoTenutoAbove || _symId == SymId::articMarcatoTenutoBelow;
}

bool Articulation::isDouble() const
{
    return _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
           || _symId == SymId::articAccentStaccatoAbove || _symId == SymId::articAccentStaccatoBelow
           || _symId == SymId::articMarcatoTenutoAbove || _symId == SymId::articMarcatoTenutoBelow;
}

//---------------------------------------------------------
//   isLuteFingering
//---------------------------------------------------------

bool Articulation::isLuteFingering() const
{
    return _symId == SymId::stringsThumbPosition
           || _symId == SymId::luteFingeringRHThumb
           || _symId == SymId::luteFingeringRHFirst
           || _symId == SymId::luteFingeringRHSecond
           || _symId == SymId::luteFingeringRHThird;
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
        SymId::ornamentPrecompSlide
    };

    SymId symId = static_cast<SymId>(subtype);

    return ornaments.find(symId) != ornaments.end();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Articulation::accessibleInfo() const
{
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), typeUserName());
}

//---------------------------------------------------------
//   doAutoplace
//    check for collisions
//---------------------------------------------------------

void Articulation::doAutoplace()
{
    // rebase vertical offset on drag
    qreal rebase = 0.0;
    if (offsetChanged() != OffsetChange::NONE) {
        rebase = rebaseOffset();
    }

    if (autoplace() && explicitParent()) {
        Segment* s = segment();
        Measure* m = measure();
        int si     = staffIdx();

        qreal sp = score()->spatium();
        qreal md = minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        RectF r = bbox().translated(chordRest()->pos() + m->pos() + s->pos() + pos());

        qreal d;
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
            qreal yd = d + md;
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
            rypos() += yd;
            r.translate(PointF(0.0, yd));
        }
    }
    setOffsetChanged(false);
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
            ArticulationGroup group = articulationAboveSplitGroups[articulationSymbolId];
            result.insert(group.first);
            result.insert(group.second);
        } else {
            result.insert(articulationSymbolId);
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

    return joinArticulations(splittedArticulations);
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
