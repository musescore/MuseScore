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

#include "engraving/automation/internal/scoreautomationcontroller.h"

#include "engraving/dom/measure.h"
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
    if (const std::optional<muse::ID> staffId = key.staffId()) {
        for (size_t i = 0; i < score->nstaves(); ++i) {
            const Staff* staff = score->staff(i);
            if (staff && staff->id() == *staffId) {
                staffIdx = i;
                break;
            }
        }

        IF_ASSERT_FAILED(staffIdx != muse::nidx) {
            return std::nullopt;
        }
    }
    // else: Global/Instrument-scoped key - not tied to any staff, leave staffIdx as muse::nidx

    const RepeatList& repeatList = score->expandedRepeatList();

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

static void widenChangedRange(const Score* score, const AutomationCurveKey& key, const AutomationPointEdits& edits,
                              ChangedRange& changedRange)
{
    if (key.type == AutomationType::Tempo) {
        const Measure* lastMeasure = score->lastMeasure();
        if (lastMeasure) {
            changedRange.tickTo = lastMeasure->endTick();
        }
        return;
    }

    const RepeatList& repeatList = score->expandedRepeatList();
    for (const AutomationPointEdit& edit : edits) {
        const Fraction tick = Fraction::fromTicks(repeatList.utick2tick(edit.tick));
        changedRange.tickFrom = std::min(changedRange.tickFrom, tick);
        changedRange.tickTo = std::max(changedRange.tickTo, tick);
    }
}

EditAutomationPoints::EditAutomationPoints(Score* score, ScoreAutomationController* controller,
                                           const AutomationCurveKey& key, const AutomationPointEdits& edits)
    : m_score(score), m_controller(controller), m_key(key)
{
    assert(score && controller && !edits.empty());

    // Collapse the edit list into the final value to write at each touched tick,
    // replaying the same erase-then-write order AutomationData::editPoints itself would apply
    for (const AutomationPointEdit& edit : edits) {
        if (const auto* setPoint = std::get_if<AutomationPointEdit::SetPoint>(&edit.change)) {
            m_pointStates[edit.tick] = setPoint->point;
        } else if (const auto* movePoint = std::get_if<AutomationPointEdit::MovePoint>(&edit.change)) {
            if (movePoint->from != edit.tick) {
                m_pointStates[movePoint->from] = std::nullopt;
            }
            m_pointStates[edit.tick] = movePoint->point;
        } else {
            m_pointStates[edit.tick] = std::nullopt;
        }
    }
}

std::optional<ChangedRange> EditAutomationPoints::changedRange() const
{
    return m_changedRange;
}

void EditAutomationPoints::flip()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_score && m_controller && m_controller->automationData()) {
        return;
    }

    const AutomationCurve& curve = m_controller->automationData()->curve(m_key);
    m_changedRange = computeChangedRange(m_score, m_key, m_pointStates);

    std::map<utick_t, std::optional<AutomationPoint> > previousStates;
    AutomationPointEdits automationEdits;
    automationEdits.reserve(m_pointStates.size());

    for (const auto& [tick, point] : m_pointStates) {
        const auto it = curve.find(tick);
        if (it != curve.cend()) {
            previousStates.emplace(tick, it->second);
        } else {
            previousStates.emplace(tick, std::nullopt);
        }

        if (point) {
            automationEdits.push_back({ tick, AutomationPointEdit::SetPoint { *point } });
        } else {
            automationEdits.push_back({ tick, AutomationPointEdit::ErasePoint {} });
        }
    }

    if (m_changedRange) {
        m_controller->editPoints(m_key, automationEdits);
        widenChangedRange(m_score, m_key, automationEdits, *m_changedRange);
    }

    m_pointStates = std::move(previousStates);
}
