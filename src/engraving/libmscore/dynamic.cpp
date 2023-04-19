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
#include "dynamic.h"

#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "chord.h"
#include "dynamichairpingroup.h"
#include "measure.h"
#include "mscore.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "system.h"
#include "tempo.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//-----------------------------------------------------------------------------
//   Dyn
//    see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//-----------------------------------------------------------------------------

struct Dyn {
    DynamicType type;
    int velocity;        ///< associated midi velocity (0-127, -1 = none)
    int changeInVelocity;
    bool accent;         ///< if true add velocity to current chord velocity
    const char* text;    // utf8 text of dynamic
};

// variant with ligatures, works for both emmentaler and bravura:
static Dyn dynList[] = {
    // dynamic:
    { DynamicType::OTHER,  -1, 0,   true, "" },
    { DynamicType::PPPPPP,  1, 0,   false,
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PPPPP,   5, 0,   false,
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PPPP,    10, 0,  false,
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PPP,     16, 0,  false,
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PP,      33, 0,  false,  "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
    { DynamicType::P,       49, 0,  false,  "<sym>dynamicPiano</sym>" },

    { DynamicType::MP,      64, 0,   false, "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>" },
    { DynamicType::MF,      80, 0,   false, "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>" },

    { DynamicType::F,       96, 0,   false, "<sym>dynamicForte</sym>" },
    { DynamicType::FF,      112, 0,  false, "<sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFF,     126, 0,  false, "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFFF,    127, 0,  false,
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFFFF,   127, 0,  false,
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::FFFFFF,  127, 0,  false,
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },

    { DynamicType::FP,      96, -47,  true, "<sym>dynamicForte</sym><sym>dynamicPiano</sym>" },
    { DynamicType::PF,      49, 47,   true, "<sym>dynamicPiano</sym><sym>dynamicForte</sym>" },

    { DynamicType::SF,      112, -18, true, "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>" },
    { DynamicType::SFZ,     112, -18, true, "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::SFF,     126, -18, true, "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::SFFZ,    126, -18, true,
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::SFP,     112, -47, true, "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>" },
    { DynamicType::SFPP,    112, -79, true,
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },

    { DynamicType::RFZ,     112, -18, true, "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
    { DynamicType::RF,      112, -18, true, "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>" },
    { DynamicType::FZ,      112, -18, true, "<sym>dynamicForte</sym><sym>dynamicZ</sym>" },

    { DynamicType::M,       96, -16,  true, "<sym>dynamicMezzo</sym>" },
    { DynamicType::R,       112, -18, true, "<sym>dynamicRinforzando</sym>" },
    { DynamicType::S,       112, -18, true, "<sym>dynamicSforzando</sym>" },
    { DynamicType::Z,       80, 0,    true, "<sym>dynamicZ</sym>" },
    { DynamicType::N,       49, -48,  true, "<sym>dynamicNiente</sym>" }
};

//---------------------------------------------------------
//   dynamicsStyle
//---------------------------------------------------------

static const ElementStyle dynamicsStyle {
    { Sid::dynamicsPlacement, Pid::PLACEMENT },
    { Sid::dynamicsMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Segment* parent)
    : TextBase(ElementType::DYNAMIC, parent, TextStyleType::DYNAMICS, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    _velocity    = -1;
    _dynRange    = DynamicRange::PART;
    _dynamicType = DynamicType::OTHER;
    _changeInVelocity = 128;
    _velChangeSpeed = DynamicSpeed::NORMAL;
    initElementStyle(&dynamicsStyle);
}

Dynamic::Dynamic(const Dynamic& d)
    : TextBase(d)
{
    _dynamicType = d._dynamicType;
    _velocity    = d._velocity;
    _dynRange    = d._dynRange;
    _changeInVelocity = d._changeInVelocity;
    _velChangeSpeed = d._velChangeSpeed;
}

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
{
    return _velocity <= 0 ? dynList[int(dynamicType())].velocity : _velocity;
}

//---------------------------------------------------------
//   changeInVelocity
//---------------------------------------------------------

int Dynamic::changeInVelocity() const
{
    return _changeInVelocity >= 128 ? dynList[int(dynamicType())].changeInVelocity : _changeInVelocity;
}

//---------------------------------------------------------
//   setChangeInVelocity
//---------------------------------------------------------

void Dynamic::setChangeInVelocity(int val)
{
    if (dynList[int(dynamicType())].changeInVelocity == val) {
        _changeInVelocity = 128;
    } else {
        _changeInVelocity = val;
    }
}

//---------------------------------------------------------
//   velocityChangeLength
//    the time over which the velocity change occurs
//---------------------------------------------------------

Fraction Dynamic::velocityChangeLength() const
{
    if (changeInVelocity() == 0) {
        return Fraction::fromTicks(0);
    }

    double ratio = score()->tempomap()->tempo(segment()->tick().ticks()).val / Constants::defaultTempo.val;
    double speedMult;
    switch (velChangeSpeed()) {
    case DynamicSpeed::SLOW:
        speedMult = 1.3;
        break;
    case DynamicSpeed::FAST:
        speedMult = 0.5;
        break;
    case DynamicSpeed::NORMAL:
    default:
        speedMult = 0.8;
        break;
    }

    return Fraction::fromTicks(int(ratio * (speedMult * double(Constants::division))));
}

//---------------------------------------------------------
//   isVelocityChangeAvailable
//---------------------------------------------------------

bool Dynamic::isVelocityChangeAvailable() const
{
    switch (dynamicType()) {
    case DynamicType::FP:
    case DynamicType::SF:
    case DynamicType::SFZ:
    case DynamicType::SFF:
    case DynamicType::SFFZ:
    case DynamicType::SFP:
    case DynamicType::SFPP:
    case DynamicType::RFZ:
    case DynamicType::RF:
    case DynamicType::FZ:
    case DynamicType::M:
    case DynamicType::R:
    case DynamicType::S:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
{
    const StaffType* stType = staffType();

    _skipDraw = false;
    if (stType && stType->isHiddenElementOnTab(score(), Sid::dynamicsShowTabCommon, Sid::dynamicsShowTabSimple)) {
        _skipDraw = true;
        return;
    }

    TextBase::layout();

    Segment* s = segment();
    if (s) {
        track_idx_t t = track() & ~0x3;
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            EngravingItem* e = s->element(t + voice);
            if (!e || (e->isRest() && toRest(e)->ticks() >= measure()->ticks() && measure()->hasVoices(e->staffIdx()))) {
                continue;
            }
            if (e->isChord() && (align() == AlignH::HCENTER)) {
                SymId symId = TConv::symId(dynamicType());

                // FIRST: move it at the center of the notehead width
                // Note: the simplest way to get the correct notehead width (accounting for all
                // the various spatium changes, staff scaling, note scaling...) is to just look
                // at the first note of the chord and get it from there.
                Note* note = toChord(e)->notes().at(0);
                double noteHeadWidth = note->headWidth();// * dynamicMag;
                movePosX(noteHeadWidth * .5);
                // SECOND: center around the SMUFL optical center, rather than using the geometrical
                // center of the bounding box
                double opticalCenter = symSmuflAnchor(symId, SmuflAnchorId::opticalCenter).x();
                if (symId != SymId::noSym && opticalCenter) {
                    double symWidth = symBbox(symId).width();
                    double offset = symWidth / 2 - opticalCenter + symBbox(symId).left();
                    // Account for scaling factors
                    double spatiumScaling = spatium() / score()->spatium();
                    static const double DEFAULT_DYNAMIC_FONT_SIZE = 10.0;
                    double fontScaling = size() / DEFAULT_DYNAMIC_FONT_SIZE;
                    offset *= spatiumScaling * fontScaling;
                    movePosX(offset);
                }
            } else {
                movePosX(e->width() * .5);
            }
            break;
        }
    } else {
        setPos(PointF());
    }
}

//-------------------------------------------------------------------
//   doAutoplace
//
//    Move Dynamic up or down to avoid collisions with other elements.
//-------------------------------------------------------------------

void Dynamic::doAutoplace()
{
    Segment* s = segment();
    if (!(s && autoplace())) {
        return;
    }

    double minDistance = score()->styleS(Sid::dynamicsMinDistance).val() * spatium();
    RectF r = bbox().translated(pos() + s->pos() + s->measure()->pos());
    double yOff = offset().y() - propertyDefault(Pid::OFFSET).value<PointF>().y();
    r.translate(0.0, -yOff);

    Skyline& sl       = s->measure()->system()->staff(staffIdx())->skyline();
    SkylineLine sk(!placeAbove());
    sk.add(r);

    if (placeAbove()) {
        double d = sk.minDistance(sl.north());
        if (d > -minDistance) {
            movePosY(-(d + minDistance));
        }
    } else {
        double d = sl.south().minDistance(sk);
        if (d > -minDistance) {
            movePosY(d + minDistance);
        }
    }
}

//---------------------------------------------------------
//   setDynamicType
//---------------------------------------------------------

void Dynamic::setDynamicType(const String& tag)
{
    std::string utf8Tag = tag.toStdString();
    int n = sizeof(dynList) / sizeof(*dynList);
    for (int i = 0; i < n; ++i) {
        if (TConv::toXml(DynamicType(i)).ascii() == utf8Tag || dynList[i].text == utf8Tag) {
            setDynamicType(DynamicType(i));
            setXmlText(String::fromUtf8(dynList[i].text));
            return;
        }
    }
    LOGD("setDynamicType: other <%s>", muPrintable(tag));
    setDynamicType(DynamicType::OTHER);
    setXmlText(tag);
}

String Dynamic::dynamicText(DynamicType t)
{
    return String::fromUtf8(dynList[int(t)].text);
}

TranslatableString Dynamic::subtypeUserName() const
{
    return TranslatableString::untranslatable(TConv::toXml(dynamicType()).ascii());
}

String Dynamic::translatedSubtypeUserName() const
{
    return String::fromAscii(TConv::toXml(dynamicType()).ascii());
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Dynamic::startEdit(EditData& ed)
{
    TextBase::startEdit(ed);
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Dynamic::endEdit(EditData& ed)
{
    TextBase::endEdit(ed);
    if (xmlText() != String::fromUtf8(dynList[int(_dynamicType)].text)) {
        _dynamicType = DynamicType::OTHER;
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
{
    TextBase::reset();
}

//---------------------------------------------------------
//   getDragGroup
//---------------------------------------------------------

std::unique_ptr<ElementGroup> Dynamic::getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
{
    if (auto g = HairpinWithDynamicsDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    if (auto g = DynamicNearHairpinsDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    return TextBase::getDragGroup(isDragged);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

mu::RectF Dynamic::drag(EditData& ed)
{
    RectF f = EngravingItem::drag(ed);

    //
    // move anchor
    //
    KeyboardModifiers km = ed.modifiers;
    if (km != (ShiftModifier | ControlModifier)) {
        staff_idx_t si = staffIdx();
        Segment* seg = segment();
        score()->dragPosition(canvasPos(), &si, &seg);
        if (seg != segment() || staffIdx() != si) {
            const PointF oldOffset = offset();
            PointF pos1(canvasPos());
            score()->undo(new ChangeParent(this, seg, si));
            setOffset(PointF());
            layout();
            PointF pos2(canvasPos());
            const PointF newOffset = pos1 - pos2;
            setOffset(newOffset);
            ElementEditDataPtr eed = ed.getData(this);
            eed->initOffset += newOffset - oldOffset;
        }
    }
    return f;
}

//---------------------------------------------------------
//   undoSetDynRange
//---------------------------------------------------------

void Dynamic::undoSetDynRange(DynamicRange v)
{
    undoChangeProperty(Pid::DYNAMIC_RANGE, v);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Dynamic::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        return _dynamicType;
    case Pid::DYNAMIC_RANGE:
        return _dynRange;
    case Pid::VELOCITY:
        return velocity();
    case Pid::SUBTYPE:
        return int(_dynamicType);
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return changeInVelocity();
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return _velChangeSpeed;
    default:
        return TextBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Dynamic::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        _dynamicType = v.value<DynamicType>();
        break;
    case Pid::DYNAMIC_RANGE:
        _dynRange = v.value<DynamicRange>();
        break;
    case Pid::VELOCITY:
        _velocity = v.toInt();
        break;
    case Pid::SUBTYPE:
        _dynamicType = v.value<DynamicType>();
        break;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            setChangeInVelocity(v.toInt());
        }
        break;
    case Pid::VELO_CHANGE_SPEED:
        _velChangeSpeed = v.value<DynamicSpeed>();
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
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

PropertyValue Dynamic::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::DYNAMICS;
    case Pid::DYNAMIC_RANGE:
        return DynamicRange::PART;
    case Pid::VELOCITY:
        return -1;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return dynList[int(dynamicType())].changeInVelocity;
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return DynamicSpeed::NORMAL;
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Dynamic::accessibleInfo() const
{
    String s;

    if (dynamicType() == DynamicType::OTHER) {
        s = plainText().simplified();
        if (s.size() > 20) {
            s.truncate(20);
            s += u"…";
        }
    } else {
        s = TConv::translatedUserName(dynamicType());
    }
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), s);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String Dynamic::screenReaderInfo() const
{
    String s;

    if (dynamicType() == DynamicType::OTHER) {
        s = plainText().simplified();
    } else {
        s = TConv::translatedUserName(dynamicType());
    }
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), s);
}
}
