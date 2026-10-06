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

using namespace muse;
using namespace muse::shortcuts;
using namespace mu::context;

static const std::string NAVIGATION_SCOPE = "NAVIGATION";
static const std::string PLAYBACK_SCOPE = "PLAYBACK";
static const std::string NOTATION_SCOPE = "NOTATION";
static const std::string NOTATION_NOTE_INPUT_SCOPE = "NOTATION_NOTE_INPUT";
static const std::string NOTATION_TEXT_EDITING_SCOPE = "NOTATION_TEXT_EDITING";
static const std::string PROJECT_SCOPE = "PROJECT";
static const std::string APP_SCOPE = "APP";
static const std::string PALETTE_SCOPE = "PALETTE";
static const std::string INSTRUMENTS_SCOPE = "INSTRUMENTS";
static const std::string EXTENSIONS_SCOPE = "EXTENSIONS";
static const std::string AUDIO_SCOPE = "AUDIO";
static const std::string DOCK_SCOPE = "DOCK";
static const std::string WORKSPACE_SCOPE = "WORKSPACE";
static const std::string UPDATE_SCOPE = "UPDATE";
static const std::string DIAGNOSTICS_SCOPE = "DIAGNOSTICS";
static const std::string TESTFLOW_SCOPE = "TESTFLOW";

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
    LOGD() << "Unknown scope: " << scope;
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

    ShortcutList sorted = list;
    std::stable_sort(sorted.begin(), sorted.end(), [this](const Shortcut& a, const Shortcut& b) {
        return scopePriority(a.scope) > scopePriority(b.scope);
    });

    return sorted.front();
}

muse::TranslatableString ShortcutResolver::scopeTitle(const std::string& scopeCode) const
{
    const static std::map<std::string, TranslatableString> scopeTitles = {
        { NAVIGATION_SCOPE, TranslatableString("shortcuts", "NAVIGATION") },
        { PLAYBACK_SCOPE, TranslatableString("shortcuts", "PLAYBACK") },
        { NOTATION_SCOPE, TranslatableString("shortcuts", "NOTATION") },
        { NOTATION_NOTE_INPUT_SCOPE, TranslatableString("shortcuts", "NOTATION: NOTE INPUT") },
        { NOTATION_TEXT_EDITING_SCOPE, TranslatableString("shortcuts", "NOTATION: TEXT EDITING") },
        { PROJECT_SCOPE, TranslatableString("shortcuts", "PROJECT") },
        { APP_SCOPE, TranslatableString("shortcuts", "APP") },
        { PALETTE_SCOPE, TranslatableString("shortcuts", "PALETTE") },
        { INSTRUMENTS_SCOPE, TranslatableString("shortcuts", "INSTRUMENTS") },
        { EXTENSIONS_SCOPE, TranslatableString("shortcuts", "EXTENSIONS") },
        { AUDIO_SCOPE, TranslatableString("shortcuts", "AUDIO") },
        { DOCK_SCOPE, TranslatableString("shortcuts", "DOCK") },
        { WORKSPACE_SCOPE, TranslatableString("shortcuts", "WORKSPACE") },
        { UPDATE_SCOPE, TranslatableString("shortcuts", "UPDATE") },
        { DIAGNOSTICS_SCOPE, TranslatableString("shortcuts", "DIAGNOSTICS") },
        { TESTFLOW_SCOPE, TranslatableString("shortcuts", "TESTFLOW") },
    };

    const std::string upperScopeCode = strings::toUpper(scopeCode);

    auto it = scopeTitles.find(upperScopeCode);
    if (it != scopeTitles.end()) {
        return it->second;
    }
    LOGW() << "Unknown scope: " << scopeCode;
    return TranslatableString::untranslatable(String::fromStdString(upperScopeCode));
}
