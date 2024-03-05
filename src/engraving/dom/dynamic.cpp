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

using namespace mu;
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
    m_anchorToEndOfPrevious = d.m_anchorToEndOfPrevious;
}

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
{
    return m_velocity <= 0 ? DYN_LIST[int(dynamicType())].velocity : m_velocity;
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

bool Dynamic::isEditAllowed(EditData& ed) const
{
    if (ed.editTextualProperties) {
        return TextBase::isEditAllowed(ed);
    }

    static const std::set<KeyboardKey> ARROW_KEYS {
        Key_Left,
        Key_Right,
        Key_Up,
        Key_Down
    };

    return muse::contains(ARROW_KEYS, static_cast<KeyboardKey>(ed.key));
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
    if (!_avoidBarLines || score()->nstaves() <= 1 || anchorToEndOfPrevious()) {
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
    std::string utf8Tag = tag.toStdString();
    size_t n = DYN_LIST.size();
    for (size_t i = 0; i < n; ++i) {
        if (TConv::toXml(DynamicType(i)).ascii() == utf8Tag || DYN_LIST[i].text == utf8Tag) {
            setDynamicType(DynamicType(i));
            setXmlText(String::fromUtf8(DYN_LIST[i].text));
            return;
        }
    }
    LOGD("setDynamicType: other <%s>", muPrintable(tag));
    setDynamicType(DynamicType::OTHER);
    setXmlText(tag);
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

    if (!(item->isDynamic() || item->isExpression())) {
        return nullptr;
    }

    item->setTrack(track());
    item->setParent(segment());
    score()->undoAddElement(item);
    item->undoChangeProperty(Pid::PLACEMENT, placement(), PropertyFlags::UNSTYLED);
    if (item->isDynamic()) {
        toDynamic(item)->setAnchorToEndOfPrevious(anchorToEndOfPrevious());
        score()->undoRemoveElement(this); // swap this dynamic for the newly added one
    }
    return item;
}

int Dynamic::dynamicVelocity(DynamicType t)
{
    return DYN_LIST[int(t)].velocity;
}

TranslatableString Dynamic::subtypeUserName() const
{
    return TranslatableString::untranslatable(TConv::toXml(dynamicType()).ascii());
}

String Dynamic::translatedSubtypeUserName() const
{
    return String::fromAscii(TConv::toXml(dynamicType()).ascii());
}

void Dynamic::startEdit(EditData& ed)
{
    if (ed.editTextualProperties) {
        TextBase::startEdit(ed);
    } else {
        startEditNonTextual(ed);
    }
}

bool Dynamic::edit(EditData& ed)
{
    if (ed.editTextualProperties) {
        return TextBase::edit(ed);
    } else {
        return editNonTextual(ed);
    }
}

bool Dynamic::editNonTextual(EditData& ed)
{
    if (ed.key == Key_Shift) {
        if (ed.isKeyRelease) {
            score()->hideAnchors();
        } else {
            EditTimeTickAnchors::updateAnchors(this, track());
        }
        triggerLayout();
        return true;
    }

    if (!isEditAllowed(ed)) {
        return false;
    }

    bool leftRightKey = ed.key == Key_Left || ed.key == Key_Right;
    bool altMod = ed.modifiers & AltModifier;
    bool shiftMod = ed.modifiers & ShiftModifier;

    bool changeAnchorType = shiftMod && altMod && leftRightKey;
    if (changeAnchorType) {
        undoChangeProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS, !anchorToEndOfPrevious(), propertyFlags(Pid::ANCHOR_TO_END_OF_PREVIOUS));
    }
    bool doesntNeedMoveSeg = changeAnchorType && ((ed.key == Key_Left && anchorToEndOfPrevious())
                                                  || (ed.key == Key_Right && !anchorToEndOfPrevious()));
    if (doesntNeedMoveSeg) {
        checkMeasureBoundariesAndMoveIfNeed();
        return true;
    }

    bool moveSeg = shiftMod && (ed.key == Key_Left || ed.key == Key_Right);
    if (moveSeg) {
        bool moved = moveSegment(ed);
        EditTimeTickAnchors::updateAnchors(this, track());
        checkMeasureBoundariesAndMoveIfNeed();
        return moved;
    }

    if (shiftMod) {
        return false;
    }

    if (!nudge(ed)) {
        return false;
    }

    triggerLayout();
    return true;
}

void Dynamic::checkMeasureBoundariesAndMoveIfNeed()
{
    /* Dynamics are always assigned to a ChordRest segment if available at this tick,
     * EXCEPT if we are at a measure boundary. In this case, if anchorToEndOfPrevious()
     * we must assign to a TimeTick segment at the end of previous measure, otherwise to a
     * ChordRest segment at the start of the next measure. */

    Segment* curSeg = segment();
    Fraction curTick = curSeg->tick();
    Measure* curMeasure = curSeg->measure();
    Measure* prevMeasure = curMeasure->prevMeasure();
    bool needMoveToNext = curTick == curMeasure->endTick() && !anchorToEndOfPrevious();
    bool needMoveToPrevious = curSeg->rtick().isZero() && anchorToEndOfPrevious() && prevMeasure;

    if (!needMoveToPrevious && !needMoveToNext) {
        return;
    }

    Segment* newSeg = nullptr;
    if (needMoveToPrevious) {
        newSeg = prevMeasure->findSegment(SegmentType::TimeTick, curSeg->tick());
        if (!newSeg) {
            TimeTickAnchor* anchor = EditTimeTickAnchors::createTimeTickAnchor(prevMeasure, curTick - prevMeasure->tick(), staffIdx());
            EditTimeTickAnchors::updateLayout(prevMeasure);
            newSeg = anchor->segment();
        }
    } else {
        newSeg = curSeg->next1(SegmentType::ChordRest);
    }

    if (newSeg && newSeg->tick() == curTick) {
        score()->undoChangeParent(this, newSeg, staffIdx());
        if (snappedExpression()) {
            score()->undoChangeParent(snappedExpression(), newSeg, staffIdx());
        }
    }
}

bool Dynamic::moveSegment(const EditData& ed)
{
    bool forward = ed.key == Key_Right;
    if (!(ed.modifiers & AltModifier)) {
        if (anchorToEndOfPrevious()) {
            undoResetProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS);
            if (forward) {
                return true;
            }
        }
    }

    Segment* curSeg = segment();
    IF_ASSERT_FAILED(curSeg) {
        return false;
    }

    Segment* newSeg = forward ? curSeg->next1ChordRestOrTimeTick() : curSeg->prev1ChordRestOrTimeTick();
    if (!newSeg) {
        return false;
    }

    score()->undoChangeParent(this, newSeg, staffIdx());
    moveSnappedItems(newSeg, newSeg->tick() - curSeg->tick());

    return true;
}

void Dynamic::moveSnappedItems(Segment* newSeg, Fraction tickDiff) const
{
    if (EngravingItem* itemSnappedBefore = ldata()->itemSnappedBefore()) {
        if (itemSnappedBefore->isHairpinSegment()) {
            Hairpin* hairpinBefore = toHairpinSegment(itemSnappedBefore)->hairpin();
            Fraction newHairpinDuration = hairpinBefore->ticks() + tickDiff;
            if (newHairpinDuration > Fraction(0, 1)) {
                hairpinBefore->undoChangeProperty(Pid::SPANNER_TICKS, newHairpinDuration);
            } else {
                hairpinBefore->undoChangeProperty(Pid::SPANNER_TICK, hairpinBefore->tick() + tickDiff);
            }
        }
    }

    if (EngravingItem* itemSnappedAfter = ldata()->itemSnappedAfter()) {
        Hairpin* hairpinAfter = itemSnappedAfter->isHairpinSegment() ? toHairpinSegment(itemSnappedAfter)->hairpin() : nullptr;
        if (itemSnappedAfter->isExpression()) {
            Expression* expressionAfter = toExpression(itemSnappedAfter);
            Segment* curExpressionSegment = expressionAfter->segment();
            if (curExpressionSegment != newSeg) {
                score()->undoChangeParent(expressionAfter, newSeg, expressionAfter->staffIdx());
            }
            EngravingItem* possibleHairpinAfterExpr = expressionAfter->ldata()->itemSnappedAfter();
            if (!hairpinAfter && possibleHairpinAfterExpr->isHairpinSegment()) {
                hairpinAfter = toHairpinSegment(possibleHairpinAfterExpr)->hairpin();
            }
        }
        if (hairpinAfter) {
            Fraction newHairpinDuration = hairpinAfter->ticks() - tickDiff;
            hairpinAfter->undoChangeProperty(Pid::SPANNER_TICK, hairpinAfter->tick() + tickDiff);
            if (newHairpinDuration > Fraction(0, 1)) {
                hairpinAfter->undoChangeProperty(Pid::SPANNER_TICKS, newHairpinDuration);
            }
        }
    }
}

bool Dynamic::nudge(const EditData& ed)
{
    bool ctrlMod = ed.modifiers & ControlModifier;
    double step = spatium() * (ctrlMod ? MScore::nudgeStep10 : MScore::nudgeStep);
    PointF addOffset = PointF();
    switch (ed.key) {
    case Key_Up:
        addOffset = PointF(0.0, -step);
        break;
    case Key_Down:
        addOffset = PointF(0.0, step);
        break;
    case Key_Left:
        addOffset = PointF(-step, 0.0);
        break;
    case Key_Right:
        addOffset = PointF(step, 0.0);
        break;
    default:
        return false;
    }
    undoChangeProperty(Pid::OFFSET, offset() + addOffset, PropertyFlags::UNSTYLED);
    return true;
}

void Dynamic::editDrag(EditData& ed)
{
    EditTimeTickAnchors::updateAnchors(this, track());

    KeyboardModifiers km = ed.modifiers;
    if (km != (ShiftModifier | ControlModifier)) {
        staff_idx_t si = staffIdx();
        Segment* seg = segment();
        static constexpr double spacingFactor = 1.0;
        score()->dragPosition(canvasPos(), &si, &seg, spacingFactor, allowTimeAnchor());
        if (seg != segment() || staffIdx() != si) {
            const PointF oldOffset = offset();
            PointF pos1(canvasPos());
            score()->undoChangeParent(this, seg, staffIdx());
            Expression* snappedExpr = snappedExpression();
            if (snappedExpr) {
                score()->undoChangeParent(snappedExpr, seg, staffIdx());
            }
            setOffset(PointF());

            renderer()->layoutItem(this);

            PointF pos2(canvasPos());
            const PointF newOffset = pos1 - pos2;
            setOffset(newOffset);
            setOffsetChanged(true);
            ElementEditDataPtr eed = ed.getData(this);
            eed->initOffset += newOffset - oldOffset;
        }
    }

    EngravingItem::editDrag(ed);
}

void Dynamic::endEdit(EditData& ed)
{
    if (ed.editTextualProperties) {
        TextBase::endEdit(ed);
        if (!xmlText().contains(String::fromUtf8(DYN_LIST[int(m_dynamicType)].text))) {
            m_dynamicType = DynamicType::OTHER;
        }
    } else {
        endEditNonTextual(ed);
    }
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
//   undoSetDynRange
//---------------------------------------------------------

void Dynamic::undoSetDynRange(DynamicRange v)
{
    TextBase::undoChangeProperty(Pid::DYNAMIC_RANGE, v);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Dynamic::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DYNAMIC_TYPE:
        return m_dynamicType;
    case Pid::DYNAMIC_RANGE:
        return m_dynRange;
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
        m_dynamicType = v.value<DynamicType>();
        break;
    case Pid::DYNAMIC_RANGE:
        m_dynRange = v.value<DynamicRange>();
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
    case Pid::DYNAMIC_RANGE:
        return DynamicRange::PART;
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
