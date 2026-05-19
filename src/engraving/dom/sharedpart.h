/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "part.h"

namespace mu::engraving {
// Map from origin track to shared track
using SharedTrackMap = std::map<track_idx_t, track_idx_t>;

class SharedPart final : public Part
{
    OBJECT_ALLOCATOR(engraving, SharedPart)
    DECLARE_CLASSOF(ElementType::SHARED_PART)

public:
    SharedPart(Score* score);

    void addOriginPart(Part* p);
    void removeOriginPart(Part* p);
    const std::vector<Part*>& originParts() const { return m_originParts; }

    String partName() const override;

    PropertyValue getProperty(Pid pid) const override;
    PropertyValue propertyDefault(Pid pid) const override;
    bool setProperty(Pid pid, const PropertyValue& v) override;

    bool enabled() const;
    bool show() const override;

    const SharedTrackMap& trackMapAtTick(const Fraction& tick) const;
    void setTrackMapAtTick(const SharedTrackMap& map, const Fraction& tick);
    void removeMapsBetweenTicks(const Fraction& startTick, const Fraction& endTick);
    bool hasTracksMappedToStaff(staff_idx_t absStaffIdx, const Fraction& tick) const;

    bool isSameInstruments() const { return m_isSameInstruments; }

private:
    void computeIsSameInstruments();
    bool m_isSameInstruments = true;

    bool m_enabled = true;
    std::vector<Part*> m_originParts;
    std::map<Fraction, SharedTrackMap> m_trackMapsByTick { { Fraction(0, 1), SharedTrackMap() } };
};
}
