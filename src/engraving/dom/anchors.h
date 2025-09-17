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

#pragma once

#include "engravingitem.h"
#include "score.h"

namespace mu::engraving {
class Factory;

class EditTimeTickAnchors
{
public:
    static void updateAnchors(const EngravingItem* item);
    static void updateAnchors(Measure* measure, staff_idx_t staffIdx, const std::set<Fraction>& additionalAnchorRelTicks = {});
    static TimeTickAnchor* createTimeTickAnchor(Measure* measure, Fraction relTick, staff_idx_t staffIdx);
    static void updateLayout(Measure* measure);
};

class MoveElementAnchors
{
public:
    static void moveElementAnchors(EngravingItem* element, KeyboardKey key, KeyboardModifier mod);
    static void moveSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff);
    static void checkMeasureBoundariesAndMoveIfNeed(EngravingItem* element);

    static void moveElementAnchorsOnDrag(EngravingItem* element, EditData& ed);

private:
    static bool canAnchorToEndOfPrevious(const EngravingItem* element);
    static void moveSegment(EngravingItem* element, bool forward);
    static Segment* getNewSegment(EngravingItem* element, Segment* curSeg, bool forward);

    static void doMoveSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff);
    static void doMoveSegment(FiguredBass* element, Segment* newSeg, Fraction tickDiff);
    static void doMoveHarmonyOrFretDiagramSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff);

    static void moveSnappedItems(EngravingItem* element, Segment* newSeg, Fraction tickDiff);
    static void rebaseOffsetOnMoveSegment(EngravingItem* element, const PointF& curOffset, Segment* newSeg, Segment* oldSeg);
};

class TimeTickAnchor : public EngravingItem
{
    TimeTickAnchor(Segment* parent);
    friend class Factory;

public:
    Segment* segment() const { return toSegment(parentItem()); }

    TimeTickAnchor* clone() const override { return new TimeTickAnchor(*this); }

    enum class DrawRegion : unsigned char {
        OUT_OF_RANGE,
        EXTENDED_REGION,
        MAIN_REGION
    };

    DrawRegion drawRegion() const;

    voice_idx_t voiceIdx() const;

    struct LayoutData : public EngravingItem::LayoutData {
        bool darker() const { return m_darker; }
        void setDarker(bool v) { m_darker = v; }

    private:
        bool m_darker = false;
    };
    DECLARE_LAYOUTDATA_METHODS(TimeTickAnchor)
};
}
