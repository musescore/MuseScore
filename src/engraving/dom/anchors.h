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

#ifndef MU_ENGRAVING_ANCHORS_H
#define MU_ENGRAVING_ANCHORS_H

#include "engravingitem.h"
#include "score.h"

namespace mu::engraving {
class EditTimeTickAnchors
{
public:
    static void updateAnchors(const EngravingItem* item, track_idx_t track);
    static TimeTickAnchor* createTimeTickAnchor(Measure* measure, Fraction relTick, staff_idx_t staffIdx);
    static void updateLayout(Measure* measure);

private:
    static void updateAnchors(Measure* measure, staff_idx_t staffIdx);
};

class TimeTickAnchor : public EngravingItem
{
public:
    TimeTickAnchor(Segment* parent);

    Segment* segment() const { return toSegment(parentItem()); }

    TimeTickAnchor* clone() const override { return new TimeTickAnchor(*this); }

    enum class DrawRegion {
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
} // namespace mu::engraving
#endif // MU_ENGRAVING_ANCHORS_H
