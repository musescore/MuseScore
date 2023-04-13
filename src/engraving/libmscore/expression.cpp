#include "chord.h"
#include "dynamic.h"
#include "dynamichairpingroup.h"
#include "expression.h"
#include "note.h"
#include "segment.h"
#include "score.h"
#include "stafftext.h"

namespace mu::engraving {
static const ElementStyle expressionStyle {
    { Sid::expressionPlacement, Pid::PLACEMENT },
    { Sid::expressionMinDistance, Pid::MIN_DISTANCE },
    { Sid::snapToDynamics, Pid::SNAP_TO_DYNAMICS },
};

Expression::Expression(Segment* parent)
    : TextBase(ElementType::EXPRESSION, parent, TextStyleType::EXPRESSION, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&expressionStyle);
}

Expression::Expression(const Expression& expression)
    : TextBase(expression)
{
    _snapToDynamics = expression._snapToDynamics;
}

PropertyValue Expression::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::EXPRESSION;
    default:
        return TextBase::propertyDefault(id);
    }
}

void Expression::layout()
{
    TextBase::layout();

    Segment* segment = explicitParent() ? toSegment(explicitParent()) : nullptr;
    if (!segment) {
        return;
    }

    if (align().horizontal != AlignH::LEFT) {
        Chord* chordToAlign = nullptr;
        // Look for chord in this staff
        track_idx_t startTrack = track2staff(staffIdx());
        track_idx_t endTrack = startTrack + VOICES;
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* item = segment->elementAt(track);
            if (item && item->isChord()) {
                chordToAlign = toChord(item);
                break;
            }
        }

        if (chordToAlign) {
            Note* note = chordToAlign->notes().at(0);
            double headWidth = note->headWidth();
            bool center = align().horizontal == AlignH::HCENTER;
            movePosX(headWidth * (center ? 0.5 : 1));
        }
    }

    _snappedDynamic = nullptr;
    if (!_snapToDynamics) {
        autoplaceSegmentElement();
        return;
    }

    Dynamic* dynamic = toDynamic(segment->findAnnotation(ElementType::DYNAMIC, track(), track()));
    if (!dynamic || dynamic->placeAbove() != placeAbove()) {
        autoplaceSegmentElement();
        return;
    }

    _snappedDynamic = dynamic;
    dynamic->setSnappedExpression(this);

    // If there is a dynamic on same segment and track, lock this expression to it
    double padding = computeDynamicExpressionDistance();
    double dynamicRight = dynamic->shape().translate(dynamic->pos()).right();
    double expressionLeft = bbox().translated(pos()).left();
    double difference = expressionLeft - dynamicRight - padding;
    movePosX(-difference);

    // Keep expression and dynamic vertically aligned
    autoplaceSegmentElement();
    bool above = placeAbove();
    double yExpression = pos().y();
    double yDynamic = dynamic->pos().y();
    bool expressionIsOuter = above ? yExpression < yDynamic : yExpression > yDynamic;
    if (expressionIsOuter) {
        dynamic->movePosY((yExpression - yDynamic));
    } else {
        movePosY((yDynamic - yExpression));
    }
}

double Expression::computeDynamicExpressionDistance() const
{
    if (!_snappedDynamic) {
        return 0.0;
    }
    // We are essentially faking the kerning behaviour of dynamic VS expression text
    // There's no other way to do this because the dynamic is a different font.
    String dynamicTextString = _snappedDynamic->xmlText();
    String f = String::fromStdString("<sym>dynamicForte</sym>");
    double distance = (dynamicTextString.endsWith(f) ? 0.2 : 0.5) * spatium();
    distance *= 0.5 * (_snappedDynamic->dynamicsSize() + (size() / 10));
    return distance;
}

std::unique_ptr<ElementGroup> Expression::getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
{
    if (auto g = DynamicExpressionDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    return TextBase::getDragGroup(isDragged);
}

void Expression::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    TextBase::undoChangeProperty(id, v, ps);
    if (_snappedDynamic) {
        if (id == Pid::OFFSET && _snappedDynamic->offset() != v.value<PointF>()
            || id == Pid::PLACEMENT && _snappedDynamic->placement() != v.value<PlacementV>()) {
            _snappedDynamic->undoChangeProperty(id, v, ps);
        }
    }
}

bool Expression::acceptDrop(EditData& ed) const
{
    return ed.dropElement->type() == ElementType::DYNAMIC;
}

EngravingItem* Expression::drop(EditData& ed)
{
    EngravingItem* item = ed.dropElement;
    if (!item->isDynamic()) {
        return nullptr;
    }
    if (_snappedDynamic) {
        return _snappedDynamic->drop(ed);
    }
    item->setTrack(track());
    item->setParent(segment());
    score()->undoAddElement(item);
    item->undoChangeProperty(Pid::PLACEMENT, placement(), PropertyFlags::UNSTYLED);
    return item;
}

PropertyValue Expression::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SNAP_TO_DYNAMICS:
        return _snapToDynamics;
    default:
        return TextBase::getProperty(propertyId);
    }
}

bool Expression::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SNAP_TO_DYNAMICS:
        if (_snapToDynamics == false && v.toBool() == true) {
            resetProperty(Pid::OFFSET);
        }
        setSnapToDynamics(v.toBool());
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

void Expression::mapPropertiesFromOldExpressions(StaffText* staffText)
{
    if (staffText->minDistance() != propertyDefault(Pid::MIN_DISTANCE).value<Spatium>()) {
        setMinDistance(staffText->minDistance());
        setPropertyFlags(Pid::MIN_DISTANCE, PropertyFlags::UNSTYLED);
    }
    if (staffText->placement() != propertyDefault(Pid::PLACEMENT).value<PlacementV>()) {
        setPlacement(staffText->placement());
        setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    }
    if (staffText->offset() != propertyDefault(Pid::OFFSET).value<PointF>()) {
        setOffset(staffText->offset());
        setSnapToDynamics(false);
        setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        setPropertyFlags(Pid::SNAP_TO_DYNAMICS, PropertyFlags::UNSTYLED);
    }
}
} // namespace mu::engraving
