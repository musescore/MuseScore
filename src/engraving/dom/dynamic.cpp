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
#include "dynamic.h"

#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "anchors.h"
#include "dynamichairpingroup.h"
#include "expression.h"
#include "hairpin.h"
#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "segment.h"
#include "system.h"
#include "tempo.h"
#include "undo.h"

#include "log.h"

using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//-----------------------------------------------------------------------------
//   Dyn
//    see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//-----------------------------------------------------------------------------

// variant with ligatures, works for both emmentaler and bravura:
const std::vector<Dyn> Dynamic::DYN_LIST = {
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
    { DynamicType::SFFF,    127, -18, true,
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
    { DynamicType::SFFFZ,   127, -18, true,
      "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>" },
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
    { Sid::dynamicsMinDistance, Pid::MIN_DISTANCE },
    { Sid::avoidBarLines, Pid::AVOID_BARLINES },
    { Sid::dynamicsSize, Pid::DYNAMICS_SIZE },
    { Sid::centerOnNotehead, Pid::CENTER_ON_NOTEHEAD },
};

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Segment* parent)
    : TextBase(ElementType::DYNAMIC, parent, TextStyleType::DYNAMICS, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_velocity    = -1;
    m_dynRange    = DynamicRange::PART;
    m_dynamicType = DynamicType::OTHER;
    m_changeInVelocity = 128;
    m_velChangeSpeed = DynamicSpeed::NORMAL;
    initElementStyle(&dynamicsStyle);
}

Dynamic::Dynamic(const Dynamic& d)
    : TextBase(d)
{
    m_dynamicType = d.m_dynamicType;
    m_velocity    = d.m_velocity;
    m_dynRange    = d.m_dynRange;
    m_changeInVelocity = d.m_changeInVelocity;
    m_velChangeSpeed = d.m_velChangeSpeed;
    _avoidBarLines = d._avoidBarLines;
    _dynamicsSize = d._dynamicsSize;
    _centerOnNotehead = d._centerOnNotehead;
}

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
{
    return m_velocity <= 0 ? DYN_LIST[int(dynamicType())].velocity : m_velocity;
}

void Dynamic::setDynRange(DynamicRange range)
{
    m_dynRange = range;

    setVoiceAssignment(dynamicRangeToVoiceAssignment(range));
}

//---------------------------------------------------------
//   changeInVelocity
//---------------------------------------------------------

int Dynamic::changeInVelocity() const
{
    return m_changeInVelocity >= 128 ? DYN_LIST[int(dynamicType())].changeInVelocity : m_changeInVelocity;
}

//---------------------------------------------------------
//   setChangeInVelocity
//---------------------------------------------------------

void Dynamic::setChangeInVelocity(int val)
{
    if (DYN_LIST[int(dynamicType())].changeInVelocity == val) {
        m_changeInVelocity = 128;
    } else {
        m_changeInVelocity = val;
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

    double ratio = score()->tempomap()->tempo(segment()->tick().ticks()).val / Constants::DEFAULT_TEMPO.val;
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

    return Fraction::fromTicks(int(ratio * (speedMult * double(Constants::DIVISION))));
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
    case DynamicType::SFFF:
    case DynamicType::SFFFZ:
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

double Dynamic::customTextOffset() const
{
    if (!_centerOnNotehead || m_dynamicType == DynamicType::OTHER) {
        return 0.0;
    }

    String referenceString = String::fromUtf8(DYN_LIST[int(m_dynamicType)].text);
    if (xmlText() == referenceString) {
        return 0.0;
    }

    Dynamic referenceDynamic(*this);
    referenceDynamic.setXmlText(referenceString);
    renderer()->layoutItem(toTextBase(&referenceDynamic));

    TextFragment referenceFragment;
    if (!referenceDynamic.ldata()->blocks.empty()) {
        TextBlock referenceBlock = referenceDynamic.ldata()->blocks.front();
        if (!referenceBlock.fragments().empty()) {
            referenceFragment = referenceDynamic.ldata()->blocks.front().fragments().front();
        }
    }

    const LayoutData* ldata = this->ldata();
    IF_ASSERT_FAILED(ldata) {
        return 0.0;
    }
    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& fragment : block.fragments()) {
            if (fragment.text == referenceFragment.text) {
                return fragment.pos.x() - referenceFragment.pos.x();
            }
        }
    }

    return 0.0;
}

//-------------------------------------------------------------------
//   doAutoplace
//
//    Move Dynamic up or down to avoid collisions with other elements.
//-------------------------------------------------------------------

//void Dynamic::doAutoplace()
//{
//    Segment* s = segment();
//    if (!(s && autoplace())) {
//        return;
//    }

//    double minDistance = style().styleS(Sid::dynamicsMinDistance).val() * spatium();
//    RectF r = bbox().translated(pos() + s->pos() + s->measure()->pos());
//    double yOff = offset().y() - propertyDefault(Pid::OFFSET).value<PointF>().y();
//    r.translate(0.0, -yOff);

//    Skyline& sl       = s->measure()->system()->staff(staffIdx())->skyline();
//    SkylineLine sk(!placeAbove());
//    sk.add(r);

//    if (placeAbove()) {
//        double d = sk.minDistance(sl.north());
//        if (d > -minDistance) {
//            movePosY(-(d + minDistance));
//        }
//    } else {
//        double d = sl.south().minDistance(sk);
//        if (d > -minDistance) {
//            movePosY(d + minDistance);
//        }
//    }
//}

//--------------------------------------------------------------------------
//   manageBarlineCollisions
//      If necessary, offset dynamic left/right to clear barline collisions
//--------------------------------------------------------------------------

void Dynamic::manageBarlineCollisions()
{
    if (!_avoidBarLines || score()->nstaves() <= 1 || anchorToEndOfPrevious() || !isStyled(Pid::OFFSET)) {
        return;
    }

    Segment* thisSegment = segment();
    if (!thisSegment) {
        return;
    }

    System* system = measure()->system();
    if (!system) {
        return;
    }

    staff_idx_t barLineStaff = muse::nidx;
    if (placeAbove()) {
        // need to find the barline from the staff above
        // taking into account there could be invisible staves
        if (staffIdx() == 0) {
            return;
        }
        for (int staffIndex = static_cast<int>(staffIdx()) - 1; staffIndex >= 0; --staffIndex) {
            if (system->staff(staffIndex)->show()) {
                barLineStaff = staffIndex;
                break;
            }
        }
    } else {
        barLineStaff = staffIdx();
    }

    if (barLineStaff == muse::nidx) {
        return;
    }

    if (score()->staff(barLineStaff)->barLineSpan() < 1) {
        return; // Barline doesn't extend through staves
    }

    const double minBarLineDistance = 0.25 * spatium();

    // Check barlines to the left
    Segment* leftBarLineSegment = nullptr;
    for (Segment* segment = thisSegment; segment && segment->measure()->system() == system; segment = segment->prev1enabled()) {
        if (segment->segmentType() & SegmentType::BarLineType) {
            leftBarLineSegment = segment;
            break;
        }
    }
    if (leftBarLineSegment) {
        EngravingItem* e = leftBarLineSegment->elementAt(barLineStaff * VOICES);
        if (e) {
            double leftMargin = ldata()->bbox().translated(pagePos() - offset()).left()
                                - e->ldata()->bbox().translated(e->pagePos()).right()
                                - minBarLineDistance;
            if (leftMargin < 0) {
                mutldata()->moveX(-leftMargin);
                return;
            }
        }
    }

    // Check barlines to the right
    Segment* rightBarLineSegment = nullptr;
    for (Segment* segment = thisSegment; segment && segment->measure()->system() == system; segment = segment->next1enabled()) {
        if (segment->segmentType() & SegmentType::BarLineType) {
            rightBarLineSegment = segment;
            break;
        }
    }

    if (rightBarLineSegment) {
        EngravingItem* e = rightBarLineSegment->elementAt(barLineStaff * VOICES);
        if (e) {
            double rightMargin = e->ldata()->bbox().translated(e->pagePos()).left()
                                 - ldata()->bbox().translated(pagePos() - offset()).right()
                                 - minBarLineDistance;
            if (rightMargin < 0) {
                mutldata()->moveX(rightMargin);
                return;
            }
        }
    }
}

//---------------------------------------------------------
//   setDynamicType
//---------------------------------------------------------

void Dynamic::setDynamicType(const String& tag)
{
    const auto dynamicInfo = parseDynamicText(tag);

    if (dynamicInfo.first == DynamicType::OTHER) {
        LOGD("setDynamicType: other <%s>", muPrintable(tag));
    }

    setDynamicType(dynamicInfo.first);
    setXmlText(dynamicInfo.second);
}

std::pair<DynamicType, String> Dynamic::parseDynamicText(const String& tag) const
{
    std::string utf8Tag = tag.toStdString();
    const std::regex dynamicRegex(R"((?:<sym>.*?</sym>)+|(?:\b)[fmnprsz]+(?:\b(?=[^>]|$)))");
    auto begin = std::sregex_iterator(utf8Tag.begin(), utf8Tag.end(), dynamicRegex);
    for (auto it = begin; it != std::sregex_iterator(); ++it) {
        const std::smatch match = *it;
        const std::string matchStr = match.str();
        size_t n = DYN_LIST.size();
        for (size_t i = 0; i < n; ++i) {
            if (TConv::toXml(DynamicType(i)).ascii() == matchStr || DYN_LIST[i].text == matchStr) {
                utf8Tag.replace(match.position(0), match.length(0), DYN_LIST[i].text);
                return { DynamicType(i), String::fromStdString(utf8Tag) };
            }
        }
    }
    return { DynamicType::OTHER, tag };
}

String Dynamic::dynamicText(DynamicType t)
{
    return String::fromUtf8(DYN_LIST[int(t)].text);
}

Expression* Dynamic::snappedExpression() const
{
    EngravingItem* item = ldata()->itemSnappedAfter();
    return item && item->isExpression() ? toExpression(item) : nullptr;
}

HairpinSegment* Dynamic::findSnapBeforeHairpinAcrossSystemBreak() const
{
    /* Normally it is the hairpin which looks for a snappable dynamic. Except if this dynamic
     * is on the first beat of next system, in which case it needs to look back for a hairpin. */
    Segment* seg = segment();
    Measure* measure = seg ? seg->measure() : nullptr;
    System* system = measure ? measure->system() : nullptr;
    bool isOnFirstBeatOfSystem = system && system->firstMeasure() == measure && seg->rtick().isZero();
    if (!isOnFirstBeatOfSystem) {
        return nullptr;
    }

    Measure* prevMeasure = measure->prevMeasure();
    System* prevSystem = prevMeasure ? prevMeasure->system() : nullptr;
    if (!prevSystem) {
        return nullptr;
    }

    for (SpannerSegment* spannerSeg : prevSystem->spannerSegments()) {
        if (!spannerSeg->isHairpinSegment() || spannerSeg->track() != track() || spannerSeg->spanner()->tick2() != tick()) {
            continue;
        }
        HairpinSegment* hairpinSeg = toHairpinSegment(spannerSeg);
        if (hairpinSeg->findElementToSnapAfter() == this) {
            return hairpinSeg;
        }
    }

    return nullptr;
}

bool Dynamic::acceptDrop(EditData& ed) const
{
    ElementType droppedType = ed.dropElement->type();
    return droppedType == ElementType::DYNAMIC || droppedType == ElementType::EXPRESSION || droppedType == ElementType::HAIRPIN;
}

EngravingItem* Dynamic::drop(EditData& ed)
{
    EngravingItem* item = ed.dropElement;

    if (item->isHairpin()) {
        score()->addHairpinToDynamic(toHairpin(item), this);
        return item;
    }

    if (item->isDynamic()) {
        Dynamic* dynamic = toDynamic(item);
        undoChangeProperty(Pid::DYNAMIC_TYPE, dynamic->dynamicType());
        undoChangeProperty(Pid::TEXT, dynamic->xmlText());
        delete dynamic;
        ed.dropElement = this;
        return this;
    }

    if (item->isExpression()) {
        item->setTrack(track());
        item->setParent(segment());
        toExpression(item)->setVoiceAssignment(voiceAssignment());
        score()->undoAddElement(item);
        return item;
    }

    return nullptr;
}

int Dynamic::dynamicVelocity(DynamicType t)
{
    return DYN_LIST[int(t)].velocity;
}

TranslatableString Dynamic::subtypeUserName() const
{
    if (dynamicType() == DynamicType::OTHER) {
        String s = plainText().simplified();
        if (s.size() > 20) {
            s.truncate(20);
            s += u"…";
        }
        return TranslatableString::untranslatable(s);
    } else {
        return TConv::userName(dynamicType());
    }
}

void Dynamic::editDrag(EditData& ed)
{
    const bool hasLeftGrip = this->hasLeftGrip();
    const bool hasRightGrip = this->hasRightGrip();

    // Right grip (when two grips/when single grip)
    if ((int(ed.curGrip) == 1 && hasLeftGrip && hasRightGrip) || (int(ed.curGrip) == 0 && !hasLeftGrip && hasRightGrip)) {
        m_rightDragOffset += ed.evtDelta.x();
        if (m_rightDragOffset < 0) {
            m_rightDragOffset = 0;
        }
        return;
    }

    // Left grip (when two grips or single grip)
    if (int(ed.curGrip) == 0 && hasLeftGrip) {
        m_leftDragOffset += ed.evtDelta.x();
        if (m_leftDragOffset > 0) {
            m_leftDragOffset = 0;
        }
        return;
    }

    TextBase::editDrag(ed);
}

void Dynamic::endEditDrag(EditData& ed)
{
    m_leftDragOffset = m_rightDragOffset = 0.0;

    TextBase::endEditDrag(ed);
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
{
    undoResetProperty(Pid::DIRECTION);
    undoResetProperty(Pid::CENTER_BETWEEN_STAVES);
    TextBase::reset();
    Expression* snappedExp = snappedExpression();
    if (snappedExp && snappedExp->getProperty(Pid::OFFSET) != snappedExp->propertyDefault(Pid::OFFSET)) {
        snappedExp->reset();
    }
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
    if (auto g = DynamicExpressionDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    return TextBase::getDragGroup(isDragged);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Dynamic::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        return m_dynamicType;
    case Pid::VELOCITY:
        return velocity();
    case Pid::SUBTYPE:
        return int(m_dynamicType);
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return changeInVelocity();
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return m_velChangeSpeed;
    case Pid::AVOID_BARLINES:
        return avoidBarLines();
    case Pid::DYNAMICS_SIZE:
        return _dynamicsSize;
    case Pid::CENTER_ON_NOTEHEAD:
        return _centerOnNotehead;
    case Pid::PLAY:
        return playDynamic();
    case Pid::ANCHOR_TO_END_OF_PREVIOUS:
        return anchorToEndOfPrevious();
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
        if (v.type() == P_TYPE::DYNAMIC_TYPE) {
            setDynamicType(v.value<DynamicType>());
            break;
        }
        setDynamicType(v.value<String>());
        break;
    case Pid::VELOCITY:
        m_velocity = v.toInt();
        break;
    case Pid::SUBTYPE:
        m_dynamicType = v.value<DynamicType>();
        break;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            setChangeInVelocity(v.toInt());
        }
        break;
    case Pid::VELO_CHANGE_SPEED:
        m_velChangeSpeed = v.value<DynamicSpeed>();
        break;
    case Pid::AVOID_BARLINES:
        setAvoidBarLines(v.toBool());
        break;
    case Pid::DYNAMICS_SIZE:
        _dynamicsSize = v.toDouble();
        break;
    case Pid::CENTER_ON_NOTEHEAD:
        _centerOnNotehead = v.toBool();
        break;
    case Pid::PLAY:
        setPlayDynamic(v.toBool());
        break;
    case Pid::ANCHOR_TO_END_OF_PREVIOUS:
        setAnchorToEndOfPrevious(v.toBool());
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
    case Pid::VELOCITY:
        return -1;
    case Pid::VELO_CHANGE:
        if (isVelocityChangeAvailable()) {
            return DYN_LIST[int(dynamicType())].changeInVelocity;
        } else {
            return PropertyValue();
        }
    case Pid::VELO_CHANGE_SPEED:
        return DynamicSpeed::NORMAL;
    case Pid::PLAY:
        return true;
    case Pid::ANCHOR_TO_END_OF_PREVIOUS:
        return false;
    default:
        return TextBase::propertyDefault(id);
    }
}

Sid Dynamic::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::PLACEMENT:
        return Sid::dynamicsPlacement;
    default:
        return TextBase::getPropertyStyle(pid);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Dynamic::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
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

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void Dynamic::drawEditMode(Painter* p, EditData& ed, double currentViewScaling)
{
    if (ed.editTextualProperties) {
        TextBase::drawEditMode(p, ed, currentViewScaling);
    } else {
        EngravingItem::drawEditMode(p, ed, currentViewScaling);
    }
}

bool Dynamic::isTextualEditAllowed(EditData& ed) const
{
    if (ed.key == Key_Tab) {
        return false;
    }

    return TextBase::isTextualEditAllowed(ed);
}

//---------------------------------------------------------
//   hasLeftHairpin
//---------------------------------------------------------

bool Dynamic::hasLeftGrip() const
{
    if (segment()->tick().isZero()) {
        return false; // Don't show the left grip for the leftmost dynamic with tick zero
    }
    return m_leftHairpin == nullptr;
}

//---------------------------------------------------------
//   hasRightHairpin
//---------------------------------------------------------

bool Dynamic::hasRightGrip() const
{
    return m_rightHairpin == nullptr;
}

//---------------------------------------------------------
//   findAdjacentHairpins
//---------------------------------------------------------

void Dynamic::findAdjacentHairpins()
{
    m_leftHairpin = nullptr;
    m_rightHairpin = nullptr;

    const Fraction tick = segment()->tick();
    const int intTick = tick.ticks();

    const auto& spanners = score()->spannerMap().findOverlapping(intTick - 1, intTick + 1);
    for (auto i : spanners) {
        Spanner* sp = i.value;
        if (sp->track() == track() && sp->isHairpin()) {
            Hairpin* hp = toHairpin(sp);
            if (hp->tick() == tick) {
                m_rightHairpin = hp;
            } else if (hp->tick2() == tick) {
                m_leftHairpin = hp;
            }
        }
    }
}

Shape Dynamic::symShapeWithCutouts(SymId id) const
{
    Staff* stf = staff();
    double staffMag = stf ? stf->staffMag(tick()) : 1.0;
    Shape shape = score()->engravingFont()->shapeWithCutouts(id, magS() * staffMag * dynamicsSize());
    for (ShapeElement& element : shape.elements()) {
        element.setItem(this);
    }

    return shape;
}

//---------------------------------------------------------
//   gripsCount
//---------------------------------------------------------

int Dynamic::gripsCount() const
{
    if (empty()) {
        return 0;
    }

    const bool hasLeftGrip = this->hasLeftGrip();
    const bool hasRightGrip = this->hasRightGrip();

    if (hasLeftGrip && hasRightGrip) {
        return 2;
    } else if (hasLeftGrip || hasRightGrip) {
        return 1;
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Dynamic::gripsPositions(const EditData&) const
{
    const LayoutData* ldata = this->ldata();
    const PointF pp(pagePos());
    double md = score()->style().styleS(Sid::hairpinMinDistance).val() * spatium(); // Minimum distance between dynamic and grip

    // Calculated by subtracting the y-value of the dynamic's pagePos from the y-value of hairpin's Grip::START position in HairpinSegment::gripsPositions
    const double GRIP_VERTICAL_OFFSET = -11.408;

    PointF leftOffset(-ldata->bbox().width() / 2 - md + m_leftDragOffset, GRIP_VERTICAL_OFFSET);
    PointF rightOffset(ldata->bbox().width() / 2 + md + m_rightDragOffset, GRIP_VERTICAL_OFFSET);

    const bool hasLeftGrip = this->hasLeftGrip();
    const bool hasRightGrip = this->hasRightGrip();

    if (hasLeftGrip && hasRightGrip) {
        return { pp + leftOffset, pp + rightOffset };
    } else if (hasLeftGrip) {
        return { pp + leftOffset };
    } else if (hasRightGrip) {
        return { pp + rightOffset };
    } else {
        return {};
    }
}
