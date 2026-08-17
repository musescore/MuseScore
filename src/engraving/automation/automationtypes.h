/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/types/id.h"
#include "global/types/number.h"
#include "global/types/sharedmap.h"

#include "mpe/automationpoint.h"

#include "engraving/infrastructure/eid.h"
#include "engraving/types/types.h"

#include <algorithm>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace mu::engraving {
struct AutomationPoint {
    using Ease = muse::mpe::AutomationPoint::Ease;
    using ArrivalFromPrevious = muse::mpe::AutomationPoint::ArrivalFromPrevious;
    using ExplicitArrival = muse::mpe::AutomationPoint::ExplicitArrival;
    using InValue = muse::mpe::AutomationPoint::InValue;

    muse::mpe::AutomationPoint value;
    std::optional<EID> itemId; // valid if it was created from an engraving item (e.g., Dynamic)
    bool generated = false; // true if the point was generated automatically and hasn't been edited by the user

    bool operator==(const AutomationPoint& p) const
    {
        return value == p.value && itemId == p.itemId && generated == p.generated;
    }
};

using utick_t = int;
using AutomationCurve = muse::SharedMap<utick_t, AutomationPoint>;

inline std::optional<AutomationPoint::Ease> ease(const AutomationPoint& point) noexcept
{
    return muse::mpe::ease(point.value);
}

inline muse::real_t resolveInValue(const AutomationCurve& curve, AutomationCurve::const_iterator it)
{
    const std::optional<muse::real_t> prevOutValue = it == curve.begin()
                                                     ? std::nullopt : std::optional(std::prev(it)->second.value.outValue);
    return muse::mpe::resolveInValue(it->second.value, prevOutValue);
}

enum class AutomationType : unsigned char {
    Unknown = 0,
    Dynamics,
    Volume,
    Pan,
};

struct AutomationCurveKey {
    //! NOTE: applies to a whole instrument (e.g. Volume, Pan)
    struct Instrument {
        InstrumentTrackId trackId;

        bool isValid() const { return trackId.isValid(); }
        bool operator==(const Instrument& o) const { return trackId == o.trackId; }
        bool operator<(const Instrument& o) const { return trackId < o.trackId; }
    };

    //! NOTE: applies to a specific staff (and optionally voice) (e.g. Dynamics)
    struct StaffVoice {
        muse::ID staffId;
        std::optional<size_t> voiceIdx;

        bool isValid() const { return staffId.isValid(); }
        bool operator==(const StaffVoice& o) const { return staffId == o.staffId && voiceIdx == o.voiceIdx; }
        bool operator<(const StaffVoice& o) const { return std::tie(staffId, voiceIdx) < std::tie(o.staffId, o.voiceIdx); }
    };

    //! NOTE: std::monostate scope means the key applies to the whole score (e.g. Tempo)
    using Scope = std::variant<std::monostate, Instrument, StaffVoice>;

    AutomationType type = AutomationType::Unknown;
    Scope scope;

    static AutomationCurveKey global(AutomationType type)
    {
        AutomationCurveKey key;
        key.type = type;
        return key;
    }

    static AutomationCurveKey instrument(AutomationType type, const InstrumentTrackId& trackId)
    {
        AutomationCurveKey key;
        key.type = type;
        key.scope = Instrument { trackId };
        return key;
    }

    static AutomationCurveKey staff(AutomationType type, const muse::ID& staffId, std::optional<size_t> voiceIdx = std::nullopt)
    {
        AutomationCurveKey key;
        key.type = type;
        key.scope = StaffVoice { staffId, voiceIdx };
        return key;
    }

    bool isValid() const
    {
        if (type == AutomationType::Unknown) {
            return false;
        }

        return std::visit([](const auto& s) {
            using T = std::decay_t<decltype(s)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return true;
            } else {
                return s.isValid();
            }
        }, scope);
    }

    std::optional<InstrumentTrackId> trackId() const
    {
        const Instrument* instrument = std::get_if<Instrument>(&scope);
        return instrument ? std::optional(instrument->trackId) : std::nullopt;
    }

    std::optional<muse::ID> staffId() const
    {
        const StaffVoice* staffVoice = std::get_if<StaffVoice>(&scope);
        return staffVoice ? std::optional(staffVoice->staffId) : std::nullopt;
    }

    std::optional<size_t> voiceIdx() const
    {
        const StaffVoice* staffVoice = std::get_if<StaffVoice>(&scope);
        return staffVoice ? staffVoice->voiceIdx : std::nullopt;
    }

    //! NOTE: same staff, but the shared (voice-independent) curve
    AutomationCurveKey withoutVoice() const
    {
        AutomationCurveKey copy = *this;
        if (StaffVoice* staffVoice = std::get_if<StaffVoice>(&copy.scope)) {
            staffVoice->voiceIdx = std::nullopt;
        }
        return copy;
    }

    bool operator==(const AutomationCurveKey& k) const
    {
        return type == k.type && scope == k.scope;
    }

    bool operator<(const AutomationCurveKey& k) const
    {
        return std::tie(type, scope) < std::tie(k.type, k.scope);
    }
};

using AutomationCurveMap = muse::SharedMap<AutomationCurveKey, AutomationCurve>;

struct AutomationPointEdit {
    //! NOTE: write point at tick
    struct SetPoint {
        AutomationPoint point;
    };
    //! NOTE: write point at tick, removing whatever point currently sits at from
    struct MovePoint {
        AutomationPoint point;
        utick_t from = 0;
    };
    //! NOTE: erase whatever point currently sits at tick
    struct ErasePoint {};

    utick_t tick = 0; // destination tick for SetPoint/MovePoint, or the tick to erase for ErasePoint
    std::variant<SetPoint, MovePoint, ErasePoint> change;
};

using AutomationPointEdits = std::vector<AutomationPointEdit>;

struct AutomationChanges {
    bool isFullReset = false;
    std::set<AutomationCurveKey> affectedKeys;
    utick_t tickFrom = -1;
    utick_t tickTo = -1;

    bool isEmpty() const
    {
        return !isFullReset && affectedKeys.empty();
    }

    void extend(utick_t from, utick_t to)
    {
        tickFrom = (tickFrom < 0) ? from : std::min(tickFrom, from);
        tickTo = std::max(tickTo, to);
    }

    void extend(const AutomationCurveKey& key, utick_t from, utick_t to)
    {
        affectedKeys.insert(key);
        extend(from, to);
    }

    void clear()
    {
        *this = {};
    }
};
}
