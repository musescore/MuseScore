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

#include "editautomationpoints.h"

#include "engraving/dom/repeatlist.h"
#include "engraving/dom/staff.h"

#include "log.h"

using namespace mu::engraving;

static std::optional<ChangedRange> computeChangedRange(const Score* score, const AutomationCurveKey& key,
                                                       const std::map<utick_t, std::optional<AutomationPoint> >& pointStates)
{
    IF_ASSERT_FAILED(score && !pointStates.empty()) {
        return std::nullopt;
    }

    size_t staffIdx = muse::nidx;
    for (size_t i = 0; i < score->nstaves(); ++i) {
        const Staff* staff = score->staff(i);
        if (staff && staff->id() == key.staffId) {
            staffIdx = i;
            break;
        }
    }

    IF_ASSERT_FAILED(staffIdx != muse::nidx) {
        return std::nullopt;
    }

    const RepeatList& repeatList = score->repeatList();

    //! NOTE: utick2tick can be non-monotonic across repeat passes (e.g. D.S. al Coda),
    //! so the base-tick extremes aren't necessarily at the utick extremes
    Fraction tickFrom = Fraction::fromTicks(repeatList.utick2tick(pointStates.begin()->first));
    Fraction tickTo = tickFrom;
    for (const auto& state : pointStates) {
        const Fraction tick = Fraction::fromTicks(repeatList.utick2tick(state.first));
        if (tick < tickFrom) {
            tickFrom = tick;
        }
        if (tick > tickTo) {
            tickTo = tick;
        }
    }

    return ChangedRange { tickFrom, tickTo, staffIdx, staffIdx };
}

EditAutomationPoints::EditAutomationPoints(Score* score, AutomationDataPtr automationData, const AutomationCurveKey& key,
                                           const AutomationPointEdits& edits)
    : m_score(score), m_automationData(automationData), m_key(key)
{
    assert(score && automationData && !edits.empty());

    // Collapse the edit list into the final value to write at each touched tick,
    // replaying the same erase-then-write order AutomationData::editPoints itself would apply
    for (const AutomationPointEdit& edit : edits) {
        if (edit.moveFrom && *edit.moveFrom != edit.tick) {
            m_pointStates[*edit.moveFrom] = std::nullopt;
        }
        m_pointStates[edit.tick] = edit.point;
    }
}

std::optional<ChangedRange> EditAutomationPoints::changedRange() const
{
    return m_changedRange;
}

void EditAutomationPoints::flip()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_score && m_automationData) {
        return;
    }

    AutomationCurve curve = m_automationData->curve(m_key);
    m_changedRange = computeChangedRange(m_score, m_key, m_pointStates);

    std::map<utick_t, std::optional<AutomationPoint> > previousStates;
    for (const auto& [tick, point] : m_pointStates) {
        const auto it = curve.find(tick);
        if (it != curve.cend()) {
            previousStates.emplace(tick, it->second);
        } else {
            previousStates.emplace(tick, std::nullopt);
        }

        if (point) {
            curve.insert_or_assign(tick, *point);
        } else {
            curve.erase(tick);
        }
    }

    AutomationCurveMap curvesToReplace;
    curvesToReplace.emplace(m_key, std::move(curve));
    m_automationData->replaceCurves(curvesToReplace);
    m_pointStates = std::move(previousStates);
}
