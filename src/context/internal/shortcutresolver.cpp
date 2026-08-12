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
#include "shortcutresolver.h"

#include <functional>

#include "global/containers.h"

#include "log.h"

using namespace muse::shortcuts;
using namespace mu::context;

static const std::string NAVIGATION_SCOPE = "NAVIGATION";
static const std::string PLAYBACK_SCOPE = "PLAYBACK";
static const std::string NOTATION_SCOPE = "NOTATION";
static const std::string NOTATION_NOTE_INPUT_SCOPE = "NOTATION_NOTE_INPUT";
static const std::string NOTATION_TEXT_EDITING_SCOPE = "NOTATION_TEXT_EDITING";

static const QString NOTATION_PANEL_NAME("ScoreView");

int ShortcutResolver::scopePriority(const std::string& scope) const
{
    if (m_scopePriorityMap.empty()) {
        m_scopePriorityMap = {
            { NAVIGATION_SCOPE, []() { return 1; } },
            { PLAYBACK_SCOPE, []() { return 2; } },
            { NOTATION_SCOPE, [this]() { return isNotationFocused() ? 3 : -1; } },
            { NOTATION_NOTE_INPUT_SCOPE, [this]() { return isNotationFocusedAndNoteInputMode() ? 4 : -1; } },
            { NOTATION_TEXT_EDITING_SCOPE, [this]() { return isNotationFocused() ? 5 : -1; } },
        };
    }

    auto priority = muse::value(m_scopePriorityMap, scope, nullptr);
    if (priority) {
        return priority();
    }
    return 0;
}

bool ShortcutResolver::isNotationFocused() const
{
    auto activePanel = navigationController()->activePanel();
    return activePanel && activePanel->name() == NOTATION_PANEL_NAME;
}

bool ShortcutResolver::isNotationFocusedAndNoteInputMode() const
{
    return isNotationFocused() && notationCommandsController()->isNoteInputMode();
}

Shortcut ShortcutResolver::selectOne(const ShortcutList& list) const
{
    IF_ASSERT_FAILED(list.size() > 0) {
        return Shortcut();
    }

    LOGD() << "selectOne: " << list.size();
    for (const Shortcut& shortcut : list) {
        LOGD() << "shortcut: " << shortcut.command;
    }

    ShortcutList sorted = list;
    sorted.sort([this](const Shortcut& a, const Shortcut& b) {
        return scopePriority(a.scope) > scopePriority(b.scope);
    });

    return sorted.front();
}
