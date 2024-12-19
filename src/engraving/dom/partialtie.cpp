#include "partialtie.h"
#include "chord.h"
#include "measure.h"
#include "score.h"
#include "note.h"
#include "staff.h"

namespace mu::engraving {
PartialTie::PartialTie(Note* parent)
    : Tie(ElementType::PARTIAL_TIE, parent)
{
}

mu::engraving::PropertyValue mu::engraving::PartialTie::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        return partialSpannerDirection();
    default:
        return Tie::getProperty(propertyId);
    }
}

PropertyValue PartialTie::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        return PartialSpannerDirection::OUTGOING;
    default:
        return Tie::propertyDefault(propertyId);
    }
}

bool PartialTie::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        setPartialSpannerDirection(v.value<PartialSpannerDirection>());
        break;
    default:
        return Tie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

void PartialTie::setStartNote(Note* note)
{
    setPartialSpannerDirection(PartialSpannerDirection::OUTGOING);
    Tie::setStartNote(note);
    Tie::setEndNote(nullptr);
}

void PartialTie::setEndNote(Note* note)
{
    setPartialSpannerDirection(PartialSpannerDirection::INCOMING);
    Tie::setStartNote(nullptr);
    Tie::setEndNote(note);
    setParent(note);
}

bool PartialTie::allJumpPointsInactive() const
{
    if (!isOutgoing()) {
        return false;
    }
    return Tie::allJumpPointsInactive();
}

TieJumpPointList* PartialTie::tieJumpPoints()
{
    if (!isOutgoing()) {
        return nullptr;
    }

    return Tie::tieJumpPoints();
}

const TieJumpPointList* PartialTie::tieJumpPoints() const
{
    if (!isOutgoing()) {
        return nullptr;
    }

    return Tie::tieJumpPoints();
}

Note* PartialTie::startNote() const
{
    if (isOutgoing()) {
        return Tie::startNote();
    }
    return startTie() ? startTie()->startNote() : nullptr;
}

PartialTieSegment::PartialTieSegment(System* parent)
    : TieSegment(ElementType::PARTIAL_TIE_SEGMENT, parent)
{
}

PartialTieSegment::PartialTieSegment(const PartialTieSegment& s)
    : TieSegment(s)
{
}

String PartialTieSegment::formatBarsAndBeats() const
{
    const PartialTie* pt = this->partialTie();
    const Note* note = pt ? pt->note() : nullptr;
    const Chord* chord = note ? note->chord() : nullptr;
    const Segment* seg = chord ? chord->segment() : nullptr;

    if (!seg) {
        return EngravingItem::formatBarsAndBeats();
    }

    return seg->formatBarsAndBeats();
}
}
