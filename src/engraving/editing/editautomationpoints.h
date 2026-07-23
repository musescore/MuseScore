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

#include <map>
#include <optional>

#include "global/allocator.h"

#include "transaction/undoablecommand.h"

#include "engraving/dom/score.h"
#include "engraving/automation/automationdata.h"

namespace mu::engraving {
class EditAutomationPoints : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, EditAutomationPoints)

public:
    EditAutomationPoints(Score* score, AutomationDataPtr automationData, const AutomationCurveKey& key, const AutomationPointEdits& edits);

    UNDO_TYPE(CommandType::EditAutomationPoints)
    UNDO_NAME("EditAutomationPoints")
    UNDO_CHANGED_OBJECTS({ m_score })

    std::optional<ChangedRange> changedRange() const override;

private:
    void flip() override;

    Score* m_score = nullptr;
    AutomationDataPtr m_automationData;
    AutomationCurveKey m_key;
    std::map<utick_t, std::optional<AutomationPoint> > m_pointStates; // tick -> point to write next, or nullopt to erase
    std::optional<ChangedRange> m_changedRange;
};
}
