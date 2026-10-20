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

#include <optional>

#include "part.h"

namespace mu::engraving {
// Map from origin track to shared track
using SharedTrackMap = std::map<track_idx_t, track_idx_t>;
std::string dump(const SharedTrackMap& map);

struct StaveSharingState {
    StaveSharingState() {}
    StaveSharingState(SharedTrackMap trackMap)
        : m_sharedTrackMap(trackMap) {}

    // User data
    const std::optional<SharedTrackMap>& userTrackMap() const { return m_userTrackMap; }
    void setUserTrackMap(std::optional<SharedTrackMap> map) { m_userTrackMap = map; }

    bool isUserModified() const { return m_userTrackMap.has_value(); }

    void setIsReset(bool val) { m_isReset = val; }
    bool isReset() const { return m_isReset; }

    // Data calculated at layout
    const SharedTrackMap& sharedTrackMap() const { return m_sharedTrackMap; }
    void setSharedTrackMap(SharedTrackMap map) { m_sharedTrackMap = map; }

    void setIsUserMapValid(bool val) { m_isUserMapValid = val; }
    bool isUserMapValid() const { return m_isUserMapValid; }

    bool operator==(const StaveSharingState& other) const
    {
        return m_sharedTrackMap == other.m_sharedTrackMap
               && m_userTrackMap == other.m_userTrackMap
               && m_isReset == other.m_isReset
               && m_isUserMapValid == other.m_isUserMapValid;
    }

    bool operator!=(const StaveSharingState& other) const { return !(*this == other); }

private:
    // User data
    std::optional<SharedTrackMap> m_userTrackMap;
    bool m_isReset = false;

    // Data calculated at layout
    SharedTrackMap m_sharedTrackMap;
    bool m_isUserMapValid = false;
};

class SharedPart final : public Part
{
    OBJECT_ALLOCATOR(engraving, SharedPart)
    DECLARE_CLASSOF(ElementType::SHARED_PART)

public:
    SharedPart(Score* score);

    void addOriginPart(Part* p);
    void removeOriginPart(Part* p);
    const std::vector<Part*>& originParts() const { return m_originParts; }

    const Part* originPart(track_idx_t sharedTrack, const Fraction& tick) const override;
    using Part::originPart;
    Part* originPartForStaff(staff_idx_t sharedStaffIdx, const Fraction& tick) const;

    String partName() const override;

    PropertyValue getProperty(Pid pid) const override;
    PropertyValue propertyDefault(Pid pid) const override;
    bool setProperty(Pid pid, const PropertyValue& v) override;

    bool enabled() const;
    bool show() const override;

    const StaveSharingState& staveSharingStateAtTick(const Fraction& tick) const;
    std::optional<StaveSharingState> exactStaveSharingStateAtTick(const Fraction& tick) const;
    void setStaveSharingStateAtTick(const StaveSharingState& state, const Fraction& tick);
    void removeStaveSharingStateAtTick(const Fraction& tick);
    void removeStaveSharingStatesBetweenTicks(const Fraction& startTick, const Fraction& endTick, bool removeUserChanges);
    std::map<Fraction, StaveSharingState> staveSharingStatesBetweenTicks(const Fraction& startTick, const Fraction& endTick) const;

    bool isSameInstrumentsAtTick(const Fraction& tick);

    bool isSameInstruments() const { return m_isSameInstruments; }

private:
    Part* mappedOriginPart(track_idx_t sharedTrack, const Fraction& tick) const;

    void computeIsSameInstruments();
    bool m_isSameInstruments = true;

    bool m_enabled = true;
    std::vector<Part*> m_originParts;
    std::map<Fraction, StaveSharingState> m_staveSharingStateByTick { { Fraction(0, 1), StaveSharingState() } };
};
}
