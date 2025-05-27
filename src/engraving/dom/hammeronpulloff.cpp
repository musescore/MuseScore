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

#include "hammeronpulloff.h"
#include "note.h"
#include "score.h"
#include "stafftype.h"
#include "style/textstyle.h"
#include "system.h"

namespace mu::engraving {
HammerOnPullOff::HammerOnPullOff(EngravingItem* parent)
    : Slur(parent, ElementType::HAMMER_ON_PULL_OFF)
{
}

HammerOnPullOff::HammerOnPullOff(const HammerOnPullOff& other)
    : Slur(other)
{
}

HammerOnPullOffSegment::HammerOnPullOffSegment(System* parent)
    : SlurSegment(parent, ElementType::HAMMER_ON_PULL_OFF_SEGMENT)
{
}

HammerOnPullOffSegment::HammerOnPullOffSegment(const HammerOnPullOffSegment& other)
    : SlurSegment(other)
{
}

Color HammerOnPullOffSegment::curColor() const
{
    if (score()->printing() || !MScore::warnGuitarBends || isValid()) {
        return SlurSegment::curColor();
    }

    auto engravingConf = configuration();
    return selected() ? engravingConf->criticalSelectedColor() : engravingConf->criticalColor();
}

void HammerOnPullOffSegment::scanElements(void* data, void (*func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child : scanChildren()) {
        child->scanElements(data, func, all);
    }

    func(data, this);
}

EngravingObjectList HammerOnPullOffSegment::scanChildren() const
{
    EngravingObjectList children;
    for (HammerOnPullOffText* hopo : m_hopoText) {
        children.push_back(hopo);
    }

    return children;
}

void HammerOnPullOffSegment::setTrack(track_idx_t idx)
{
    m_track = idx;
    for (HammerOnPullOffText* hopo : m_hopoText) {
        hopo->setTrack(idx);
    }
}

void HammerOnPullOffSegment::updateHopoText()
{
    Chord* startChord = nullptr;
    if (isSingleBeginType()) {
        EngravingItem* startEl = hammerOnPullOff()->startElement();
        startChord = startEl && startEl->isChord() ? toChord(startEl) : nullptr;
    } else {
        ChordRest* firstCR = system()->firstChordRest(track());
        startChord = firstCR && firstCR->isChord() ? toChord(firstCR) : nullptr;
    }

    Chord* endChord = nullptr;
    if (isSingleEndType()) {
        EngravingItem* endEl = hammerOnPullOff()->endElement();
        endChord = endEl && endEl->isChord() ? toChord(endEl) : nullptr;
    } else {
        // If the segment doesn't end in this system, the endChord is the first chord of next system
        ChordRest* lastCR = system()->lastChordRest(track());
        if (lastCR) {
            lastCR = toChordRest(lastCR->segment()->next1WithElemsOnTrack(track())->element(track()));
        }
        endChord = lastCR && lastCR->isChord() ? toChord(lastCR) : nullptr;
    }

    if (!startChord || !endChord) {
        muse::DeleteAll(m_hopoText);
        m_hopoText.clear();
        return;
    }

    std::vector<HopoTextRegion> hopoTextRegions = computeHopoTextRegions(startChord, endChord);
    size_t regionCount = hopoTextRegions.size();

    size_t curRegionIdx = 0;
    for (; curRegionIdx < regionCount; ++curRegionIdx) {
        HopoTextRegion curRegion = hopoTextRegions[curRegionIdx];

        HammerOnPullOffText* curHopoText;
        if (curRegionIdx < m_hopoText.size()) {
            // Reuse existing, if available
            curHopoText = m_hopoText[curRegionIdx];
        } else {
            // Create new
            curHopoText = new HammerOnPullOffText(this);
            m_hopoText.push_back(curHopoText);
        }

        curHopoText->setParent(this);
        curHopoText->setTrack(track());
        curHopoText->setIsValid(curRegion.isValid);
        curHopoText->setIsHammerOn(curRegion.isHammerOn);
        curHopoText->setXmlText(style().styleB(Sid::hopoUpperCase) ? (curRegion.isHammerOn ? "H" : "P") : (curRegion.isHammerOn ? "h" : "p"));
        curHopoText->setStartChord(curRegion.startChord);
        curHopoText->setEndChord(curRegion.endChord);
    }

    // Delete unused
    if (curRegionIdx < m_hopoText.size()) {
        size_t unusedHopoCount = m_hopoText.size() - curRegionIdx;
        for (size_t i = 0; i < unusedHopoCount; ++i) {
            delete m_hopoText.back();
            m_hopoText.pop_back();
        }
    }
}

std::vector<HammerOnPullOffSegment::HopoTextRegion> HammerOnPullOffSegment::computeHopoTextRegions(Chord* startChord, Chord* endChord)
{
    std::vector<HopoTextRegion> result;
    bool isTabStaff = staffType()->isTabStaff();
    if ((isTabStaff && !style().styleB(Sid::hopoShowOnTabStaves)) || (!isTabStaff && !style().styleB(Sid::hopoShowOnStandardStaves))) {
        return result;
    }

    for (Chord* curChord = startChord; curChord != endChord;) {
        IF_ASSERT_FAILED(curChord->tick() <= endChord->tick()) {
            break;
        }

        Chord* nextChord = curChord->next();
        if (!nextChord) {
            break;
        }

        Note* curNote = curChord->upNote();
        Note* nextNote = nextChord->upNote();
        bool isValid = isTabStaff ? nextNote->string() == curNote->string() : nextNote->pitch() != curNote->pitch();
        bool isHammerOn = isTabStaff ? nextNote->fret() > curNote->fret() : nextNote->pitch() > curNote->pitch();

        bool startNewRegion = result.empty() || style().styleB(Sid::hopoShowAll) || result.back().isHammerOn != isHammerOn;

        if (startNewRegion) {
            HopoTextRegion region;
            region.startChord = curChord;
            region.endChord = nextChord;
            region.isValid = isValid;
            region.isHammerOn = isHammerOn;
            result.push_back(region);
        } else {
            HopoTextRegion& curRegion = result.back();
            curRegion.endChord = nextChord;
            curRegion.isValid = curRegion.isValid && isValid;
        }

        curChord = nextChord;
    }

    return result;
}

bool HammerOnPullOffSegment::isUserModified() const
{
    for (const HammerOnPullOffText* hopoText : m_hopoText) {
        if (hopoText->isUserModified()) {
            return true;
        }
    }

    return SlurSegment::isUserModified();
}

bool HammerOnPullOffSegment::isValid() const
{
    for (const HammerOnPullOffText* hopoText : m_hopoText) {
        if (!hopoText->isValid()) {
            return false;
        }
    }

    return true;
}

void HammerOnPullOffSegment::reset()
{
    for (HammerOnPullOffText* hopoText : m_hopoText) {
        hopoText->reset();
    }

    SlurTieSegment::reset();
}

static ElementStyle hopoStyle;

HammerOnPullOffText::HammerOnPullOffText(HammerOnPullOffSegment* parent)
    : TextBase(ElementType::HAMMER_ON_PULL_OFF_TEXT, parent, TextStyleType::HAMMER_ON_PULL_OFF,
               ElementFlag::MOVABLE | ElementFlag::GENERATED)
{
    resetProperty(Pid::PLACEMENT);
    initElementStyle(&hopoStyle);
}

HammerOnPullOffText::HammerOnPullOffText(const HammerOnPullOffText& h)
    : TextBase(h)
{
}

std::vector<LineF> HammerOnPullOffText::dragAnchorLines() const
{
    std::vector<LineF> result;

    PointF p1 = canvasPos();

    HammerOnPullOffSegment* hopoSeg = toHammerOnPullOffSegment(parent());
    const Shape& hopoSegShape = hopoSeg->ldata()->shape();
    double x = ldata()->pos().x();// + hopoSegShape.bbox().x();
    double y = hopoSeg->hammerOnPullOff()->up() ? hopoSegShape.topAtX(x) : hopoSegShape.bottomAtX(x);

    PointF p2 = PointF(x, y) + hopoSeg->canvasPos();

    result.push_back(LineF(p1, p2));

    return result;
}

bool HammerOnPullOffText::isUserModified() const
{
    for (const TextStyleProperty& p : *textStyle(textStyleType())) {
        if (getProperty(p.pid) != propertyDefault(p.pid)) {
            return true;
        }
    }

    return TextBase::isUserModified();
}

Color HammerOnPullOffText::curColor() const
{
    if (score()->printing()) {
        return TextBase::curColor();
    }

    auto engravingConf = configuration();
    if (isValid() || !MScore::warnGuitarBends) {
        return selected() || (parentItem() && parentItem()->selected()) ? engravingConf->selectionColor() : TextBase::curColor();
    }

    return selected() || parentItem()->selected() ? engravingConf->criticalSelectedColor() : engravingConf->criticalColor();
}

PropertyValue HammerOnPullOffText::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    default:
        return TextBase::propertyDefault(id);
    }
}
} // namespace mu::engraving
