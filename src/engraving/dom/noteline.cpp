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

#include "noteline.h"
#include "linkedobjects.h"
#include "factory.h"
#include "note.h"
#include "textline.h"

namespace mu::engraving {
static const ElementStyle noteLineStyle {
    { Sid::noteLinePlacement,                  Pid::PLACEMENT },
    { Sid::noteLineFontFace,                   Pid::BEGIN_FONT_FACE },
    { Sid::noteLineFontFace,                   Pid::CONTINUE_FONT_FACE },
    { Sid::noteLineFontFace,                   Pid::END_FONT_FACE },
    { Sid::noteLineFontSize,                   Pid::BEGIN_FONT_SIZE },
    { Sid::noteLineFontSize,                   Pid::CONTINUE_FONT_SIZE },
    { Sid::noteLineFontSize,                   Pid::END_FONT_SIZE },
    { Sid::noteLineFontStyle,                  Pid::BEGIN_FONT_STYLE },
    { Sid::noteLineFontStyle,                  Pid::CONTINUE_FONT_STYLE },
    { Sid::noteLineFontStyle,                  Pid::END_FONT_STYLE },
    { Sid::noteLineAlign,                      Pid::BEGIN_TEXT_ALIGN },
    { Sid::noteLineAlign,                      Pid::CONTINUE_TEXT_ALIGN },
    { Sid::noteLineAlign,                      Pid::END_TEXT_ALIGN },
    { Sid::noteLineFontSpatiumDependent,       Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
};

Sid NoteLineSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return Sid::NOSTYLE;
    }
    return TextLineBaseSegment::getPropertyStyle(pid);
}

NoteLineSegment::NoteLineSegment(Spanner* sp, System* parent)
    : TextLineBaseSegment(ElementType::NOTELINE_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
}

EngravingItem* NoteLineSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::NOTELINE_PLACEMENT) {
        return noteLine();
    }

    return TextLineBaseSegment::propertyDelegate(pid);
}

Sid NoteLine::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return Sid::NOSTYLE;
    }
    return TextLineBase::getPropertyStyle(pid);
}

NoteLine::NoteLine(EngravingItem* parent)
    : TextLineBase(ElementType::NOTELINE, parent, ElementFlag::MOVABLE)
{
    setAnchor(Spanner::Anchor::NOTE);
    setDiagonal(true);
    setSystemFlag(false);
    setPlacement(PlacementV::ABOVE);

    initElementStyle(&noteLineStyle);

    setBeginText(u"");
    setContinueText(u"");
    setEndText(u"");
    setBeginTextOffset(PointF(0, 0));
    setContinueTextOffset(PointF(0, 0));
    setEndTextOffset(PointF(0, 0));
    setLineVisible(true);

    setBeginHookType(HookType::NONE);
    setEndHookType(HookType::NONE);
    setBeginHookHeight(Spatium(1.5));
    setEndHookHeight(Spatium(1.5));
    setGapBetweenTextAndLine(Spatium(0.5));

    setLineEndPlacement(NoteLineEndPlacement::OFFSET_ENDS);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::END_TEXT_PLACE);
}

NoteLine::NoteLine(const NoteLine& nl)
    : TextLineBase(nl)
{
}

LineSegment* NoteLine::createLineSegment(System* parent)
{
    NoteLineSegment* seg = new NoteLineSegment(this, parent);
    seg->setTrack(track());
    return seg;
}

PropertyValue NoteLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BEGIN_TEXT:
    case Pid::CONTINUE_TEXT:
    case Pid::END_TEXT:
        return "";
    case Pid::LINE_VISIBLE:
        return true;
    case Pid::BEGIN_TEXT_OFFSET:
    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PointF(0, 0);
    case Pid::BEGIN_HOOK_TYPE:
    case Pid::END_HOOK_TYPE:
        return HookType::NONE;
    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::LEFT;
    case Pid::BEGIN_HOOK_HEIGHT:
    case Pid::END_HOOK_HEIGHT:
        return Spatium(1.5);
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    case Pid::DIAGONAL:
        return true;
    case Pid::NOTELINE_PLACEMENT:
        return NoteLineEndPlacement::OFFSET_ENDS;
    case Pid::OFFSET:
        return PointF();
    default:
        return TextLineBase::propertyDefault(propertyId);
    }
}

PropertyValue NoteLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    case Pid::NOTELINE_PLACEMENT:
        return m_lineEndPlacement;
    default:
        return TextLineBase::getProperty(propertyId);
    }
}

bool NoteLine::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::NOTELINE_PLACEMENT:
        setLineEndPlacement(val.value<NoteLineEndPlacement>());
        break;
    default:
        return TextLineBase::setProperty(propertyId, val);
    }

    return true;
}

void NoteLine::addLineAttachPoints()
{
    assert(anchor() == Spanner::Anchor::NOTE);

    NoteLineSegment* frontSeg = toNoteLineSegment(frontSegment());
    NoteLineSegment* backSeg = toNoteLineSegment(backSegment());
    Note* startNote = nullptr;
    Note* endNote = nullptr;
    if (startElement() && startElement()->isNote()) {
        startNote = toNote(startElement());
    }
    if (endElement() && endElement()->isNote()) {
        endNote = toNote(endElement());
    }
    if (!frontSeg || !backSeg || !startNote || !endNote) {
        return;
    }
    double startX = frontSeg->ldata()->pos().x();
    double endX = backSeg->pos2().x() + backSeg->ldata()->pos().x(); // because pos2 is relative to ipos
    // Here we don't pass y() because its value is unreliable during the first stages of layout.
    // The y() is irrelevant anyway for horizontal spacing.
    startNote->addLineAttachPoint(PointF(startX, 0.0), this);
    endNote->addLineAttachPoint(PointF(endX, 0.0), this);
}

void NoteLine::reset()
{
    undoResetProperty(Pid::NOTELINE_PLACEMENT);
    TextLineBase::reset();
}

NoteLine* NoteLine::createFromTextLine(TextLine* textLine)
{
    assert(textLine->anchor() == Spanner::Anchor::NOTE);
    Note* startNote = toNote(textLine->startElement());
    Note* endNote = toNote(textLine->endElement());

    NoteLine* noteLine = Factory::createNoteLine(startNote);
    noteLine->setParent(startNote);
    noteLine->setStartElement(startNote);
    noteLine->setTrack(textLine->track());
    noteLine->setTick(textLine->tick());
    noteLine->setEndElement(endNote);
    noteLine->setTick2(textLine->tick2());
    noteLine->setVisible(textLine->visible());

    // Preserve old layout style
    noteLine->setLineEndPlacement(NoteLineEndPlacement::LEFT_EDGE);

    for (Pid pid : textLine->textLineBasePropertyIds()) {
        noteLine->setProperty(pid, textLine->getProperty(pid));
    }

    for (const SpannerSegment* oldSeg : textLine->spannerSegments()) {
        LineSegment* newSeg = noteLine->createLineSegment(toSystem(oldSeg->parent()));
        newSeg->setOffset(oldSeg->offset());
        newSeg->setUserOff2(oldSeg->userOff2());

        noteLine->add(newSeg);
    }

    LinkedObjects* links = textLine->links();
    noteLine->setLinks(links);
    if (links) {
        links->push_back(noteLine);
    }

    return noteLine;
}
} // namespace mu::engraving
