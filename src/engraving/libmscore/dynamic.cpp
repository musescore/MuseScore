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
#include "style/style.h"
#include "io/xml.h"

#include "dynamichairpingroup.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "symid.h"
#include "segment.h"
#include "utils.h"
#include "mscore.h"
#include "chord.h"
#include "undo.h"
#include "musescoreCore.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//-----------------------------------------------------------------------------
//   Dyn
//    see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//-----------------------------------------------------------------------------

struct Dyn {
    int velocity;        ///< associated midi velocity (0-127, -1 = none)
    bool accent;         ///< if true add velocity to current chord velocity
    SymId symId;
    const char* tag;     // name of dynamics, eg. "fff"
    const char* text;    // utf8 text of dynamic
    int changeInVelocity;
};

// variant with ligatures, works for both emmentaler and bravura:

static Dyn dynList[] = {
    // dynamic:
    { -1,   true,  SymId::noSym,            "other-dynamics", "", 0 },
    { 1,    false, SymId::dynamicPPPPPP,    "pppppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
      0 },
    { 5,    false, SymId::dynamicPPPPP,     "ppppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
    { 10,   false, SymId::dynamicPPPP,      "pppp",
      "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
    { 16,   false, SymId::dynamicPPP,       "ppp",    "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
    { 33,   false, SymId::dynamicPP,        "pp",     "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
    { 49,   false, SymId::dynamicPiano,     "p",      "<sym>dynamicPiano</sym>", 0 },
    { 64,   false, SymId::dynamicMP,        "mp",     "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>", 0 },
    { 80,   false, SymId::dynamicMF,        "mf",     "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>", 0 },
    { 96,   false, SymId::dynamicForte,     "f",      "<sym>dynamicForte</sym>", 0 },
    { 112,  false, SymId::dynamicFF,        "ff",     "<sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
    { 126,  false, SymId::dynamicFFF,       "fff",    "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
    { 127,  false, SymId::dynamicFFFF,      "ffff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
    { 127,  false, SymId::dynamicFFFFF,     "fffff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
    { 127,  false, SymId::dynamicFFFFFF,    "ffffff",
      "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
      0 },

    // accents:
    { 96,  true, SymId::dynamicFortePiano,          "fp",     "<sym>dynamicForte</sym><sym>dynamicPiano</sym>", -47 },
    { 49,  true, SymId::noSym,                      "pf",     "<sym>dynamicPiano</sym><sym>dynamicForte</sym>", 47 },
    { 112, true, SymId::dynamicSforzando1,          "sf",     "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>", -18 },
    { 112, true, SymId::dynamicSforzato,            "sfz",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>",
      -18 },
    { 126, true, SymId::noSym,                      "sff",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
      -18 },
    { 126, true, SymId::dynamicSforzatoFF,          "sffz",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
    { 112, true, SymId::dynamicSforzandoPiano,      "sfp",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>",
      -47 },
    { 112, true, SymId::dynamicSforzandoPianissimo, "sfpp",
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", -79 },
    { 112, true, SymId::dynamicRinforzando2,        "rfz",    "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>",
      -18 },
    { 112, true, SymId::dynamicRinforzando1,        "rf",     "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>", -18 },
    { 112, true, SymId::dynamicForzando,            "fz",     "<sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
    { 96,  true, SymId::dynamicMezzo,               "m",      "<sym>dynamicMezzo</sym>", -16 },
    { 112, true, SymId::dynamicRinforzando,         "r",      "<sym>dynamicRinforzando</sym>", -18 },
    { 112, true, SymId::dynamicSforzando,           "s",      "<sym>dynamicSforzando</sym>", -18 },
    { 80,  true, SymId::dynamicZ,                   "z",      "<sym>dynamicZ</sym>", 0 },
    { 49,  true, SymId::dynamicNiente,              "n",      "<sym>dynamicNiente</sym>", -48 }
};

//---------------------------------------------------------
//   dynamicsStyle
//---------------------------------------------------------

static const ElementStyle dynamicsStyle {
    { Sid::dynamicsPlacement, Pid::PLACEMENT },
    { Sid::dynamicsMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   changeSpeedTable
//---------------------------------------------------------

const std::vector<Dynamic::ChangeSpeedItem> Dynamic::changeSpeedTable {
    { Dynamic::Speed::NORMAL,           "normal" },
    { Dynamic::Speed::SLOW,             "slow" },
    { Dynamic::Speed::FAST,             "fast" },
};

//---------------------------------------------------------
//   findInString
//---------------------------------------------------------

// find the longest first match of dynList's dynamic text in s
// used by the MusicXML export to correctly export dynamics embedded
// in spanner begin- or endtexts
// return match's position and length and the dynamic type

int Dynamic::findInString(const QString& s, int& length, QString& type)
{
    length = 0;
    type = "";
    int matchIndex { -1 };
    const int n = sizeof(dynList) / sizeof(*dynList);

    // for all dynamics, find their text in s
    for (int i = 0; i < n; ++i) {
        const QString dynamicText = dynList[i].text;
        const int dynamicLength = dynamicText.length();
        // note: skip entries with empty text
        if (dynamicLength > 0) {
            const auto index = s.indexOf(dynamicText);
            if (index >= 0) {
                // found a match, accept it if
                // - it is the first one
                // - or it starts a the same index but is longer ("pp" versus "p")
                if (matchIndex == -1 || (index == matchIndex && dynamicLength > length)) {
                    matchIndex = index;
                    length = dynamicLength;
                    type = dynList[i].tag;
                }
            }
        }
    }

    return matchIndex;
}

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Segment* parent)
    : TextBase(ElementType::DYNAMIC, parent, Tid::DYNAMICS, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    _velocity    = -1;
    _dynRange    = Range::PART;
    _dynamicType = DynamicType::OTHER;
    _changeInVelocity = 128;
    _velChangeSpeed = Speed::NORMAL;
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

    double ratio = double(score()->tempomap()->tempo(segment()->tick().ticks())) / double(Score::defaultTempo());
    double speedMult;
    switch (velChangeSpeed()) {
    case Dynamic::Speed::SLOW:
        speedMult = 1.3;
        break;
    case Dynamic::Speed::FAST:
        speedMult = 0.5;
        break;
    case Dynamic::Speed::NORMAL:
    default:
        speedMult = 0.8;
        break;
    }

    return Fraction::fromTicks(int(ratio * (speedMult * double(MScore::division))));
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
//   write
//---------------------------------------------------------

void Dynamic::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.startObject(this);
    writeProperty(xml, Pid::DYNAMIC_TYPE);
    writeProperty(xml, Pid::VELOCITY);
    writeProperty(xml, Pid::DYNAMIC_RANGE);
    writeProperty(xml, Pid::VELO_CHANGE);
    writeProperty(xml, Pid::VELO_CHANGE_SPEED);
    TextBase::writeProperties(xml, dynamicType() == DynamicType::OTHER);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag = e.name();
        if (tag == "subtype") {
            setDynamicType(e.readElementText());
        } else if (tag == "velocity") {
            _velocity = e.readInt();
        } else if (tag == "dynType") {
            _dynRange = Range(e.readInt());
        } else if (tag == "veloChange") {
            _changeInVelocity = e.readInt();
        } else if (tag == "veloChangeSpeed") {
            _velChangeSpeed = nameToSpeed(e.readElementText());
        } else if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
{
    TextBase::layout();

    Segment* s = segment();
    if (s) {
        int t = track() & ~0x3;
        for (int voice = 0; voice < VOICES; ++voice) {
            EngravingItem* e = s->element(t + voice);
            if (!e) {
                continue;
            }
            if (e->isChord() && (align() & Align::HCENTER)) {
                SymId symId = dynList[int(dynamicType())].symId;

                // this value is different than chord()->mag() or mag()
                // as it reflects the actual scaling of the text
                // using chord()->mag(), mag() or fontSize will yield
                // undesirable results with small staves or cue notes
                qreal dynamicMag = spatium() / SPATIUM20;

                qreal noteHeadWidth = score()->noteHeadWidth() * dynamicMag;
                rxpos() += noteHeadWidth * .5;

                qreal opticalCenter = symSmuflAnchor(symId, SmuflAnchorId::opticalCenter).x() * dynamicMag;
                if (symId != SymId::noSym && opticalCenter) {
                    static const qreal DEFAULT_DYNAMIC_FONT_SIZE = 10.0;
                    qreal fontScaling = size() / DEFAULT_DYNAMIC_FONT_SIZE;
                    qreal left = symBbox(symId).bottomLeft().x() * dynamicMag; // this is negative per SMuFL spec

                    opticalCenter += fontScaling;
                    left += fontScaling;

                    qreal offset = opticalCenter - left - bbox().width() * 0.5;
                    rxpos() -= offset;
                }
            } else {
                rxpos() += e->width() * .5;
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

    qreal minDistance = score()->styleS(Sid::dynamicsMinDistance).val() * spatium();
    RectF r = bbox().translated(pos() + s->pos() + s->measure()->pos());
    qreal yOff = offset().y() - propertyDefault(Pid::OFFSET).value<PointF>().y();
    r.translate(0.0, -yOff);

    Skyline& sl       = s->measure()->system()->staff(staffIdx())->skyline();
    SkylineLine sk(!placeAbove());
    sk.add(r);

    if (placeAbove()) {
        qreal d = sk.minDistance(sl.north());
        if (d > -minDistance) {
            rypos() += -(d + minDistance);
        }
    } else {
        qreal d = sl.south().minDistance(sk);
        if (d > -minDistance) {
            rypos() += d + minDistance;
        }
    }
}

//---------------------------------------------------------
//   setDynamicType
//---------------------------------------------------------

void Dynamic::setDynamicType(const QString& tag)
{
    int n = sizeof(dynList) / sizeof(*dynList);
    for (int i = 0; i < n; ++i) {
        if (dynList[i].tag == tag || dynList[i].text == tag) {
            setDynamicType(DynamicType(i));
            setXmlText(QString::fromUtf8(dynList[i].text));
            return;
        }
    }
    qDebug("setDynamicType: other <%s>", qPrintable(tag));
    setDynamicType(DynamicType::OTHER);
    setXmlText(tag);
}

//---------------------------------------------------------
//   dynamicTypeName
//---------------------------------------------------------

QString Dynamic::dynamicTypeName(DynamicType type)
{
    return dynList[int(type)].tag;
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
    if (xmlText() != QString::fromUtf8(dynList[int(_dynamicType)].text)) {
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
    Qt::KeyboardModifiers km = ed.modifiers;
    if (km != (Qt::ShiftModifier | Qt::ControlModifier)) {
        int si       = staffIdx();
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
            ElementEditData* eed = ed.getData(this);
            eed->initOffset += newOffset - oldOffset;
        }
    }
    return f;
}

//---------------------------------------------------------
//   undoSetDynRange
//---------------------------------------------------------

void Dynamic::undoSetDynRange(Range v)
{
    undoChangeProperty(Pid::DYNAMIC_RANGE, int(v));
}

//---------------------------------------------------------
//   speedToName
//---------------------------------------------------------

QString Dynamic::speedToName(Speed speed)
{
    for (auto i : Dynamic::changeSpeedTable) {
        if (i.speed == speed) {
            return i.name;
        }
    }
    qFatal("Unrecognised change speed!");
    return "none";   // silence a compiler warning
}

//---------------------------------------------------------
//   nameToSpeed
//---------------------------------------------------------

Dynamic::Speed Dynamic::nameToSpeed(QString name)
{
    for (auto i : Dynamic::changeSpeedTable) {
        if (i.name == name) {
            return i.speed;
        }
    }
    return Speed::NORMAL;     // default
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Dynamic::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        return PropertyValue::fromValue(_dynamicType);
    case Pid::DYNAMIC_RANGE:
        return int(_dynRange);
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
        return int(_velChangeSpeed);
    default:
        return TextBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Dynamic::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        _dynamicType = v.value<DynamicType>();
        break;
    case Pid::DYNAMIC_RANGE:
        _dynRange = Range(v.toInt());
        break;
    case Pid::VELOCITY:
        _velocity = v.toInt();
        break;
    case Pid::SUBTYPE:
        _dynamicType = DynamicType(v.toInt());
        break;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            setChangeInVelocity(v.toInt());
        }
        break;
    case Pid::VELO_CHANGE_SPEED:
        _velChangeSpeed = Speed(v.toInt());
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
    case Pid::SUB_STYLE:
        return int(Tid::DYNAMICS);
    case Pid::DYNAMIC_RANGE:
        return int(Range::PART);
    case Pid::VELOCITY:
        return -1;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return dynList[int(dynamicType())].changeInVelocity;
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return int(Speed::NORMAL);
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Dynamic::propertyId(const QStringRef& name) const
{
    if (name == propertyName(Pid::DYNAMIC_TYPE)) {
        return Pid::DYNAMIC_TYPE;
    }
    return TextBase::propertyId(name);
}

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString Dynamic::propertyUserValue(Pid pid) const
{
    switch (pid) {
    case Pid::DYNAMIC_TYPE:
        return dynamicTypeName();
    default:
        break;
    }
    return TextBase::propertyUserValue(pid);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Dynamic::accessibleInfo() const
{
    QString s;

    if (dynamicType() == DynamicType::OTHER) {
        s = plainText().simplified();
        if (s.length() > 20) {
            s.truncate(20);
            s += "…";
        }
    } else {
        s = dynamicTypeName();
    }
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), s);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString Dynamic::screenReaderInfo() const
{
    QString s;

    if (dynamicType() == DynamicType::OTHER) {
        s = plainText().simplified();
    } else {
        s = dynamicTypeName();
    }
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), s);
}
}
