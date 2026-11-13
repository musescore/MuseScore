/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and redistribute it and/or modify
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

#include "appshortcutsscopes.h"

using namespace muse;
using namespace muse::shortcuts;
using namespace mu::appshell;

static const QString NOTATION_PANEL_NAME("ScoreView");

void AppShortcutsScopes::init()
{
    if (!shortcutsResolver()) {
        return;
    }

    std::shared_ptr<const AppShortcutsScopes> self = shared_from_this();

    auto whenNotationFocused = [self](Condition condition = nullptr) {
        return [self, condition]() {
            return self->isNotationActive(condition);
        };
    };

    IShortcutsResolver* resolver = shortcutsResolver().get();

    resolver->setScope({ "NAVIGATION", TranslatableString("shortcuts", "NAVIGATION"), 1 });
    resolver->setScope({ "PLAYBACK", TranslatableString("shortcuts", "PLAYBACK"), 2 });

    resolver->setScope({ "NOTATION_PAGE", TranslatableString("shortcuts", "NOTATION PAGE"), 3 });
    resolver->setScope({ "NOTATION", TranslatableString("shortcuts", "NOTATION"), 4, whenNotationFocused() });
    resolver->setScope({ "NOTATION_TAB", TranslatableString("shortcuts", "NOTATION: TAB"), 5,
                         whenNotationFocused(&AppShortcutsScopes::isTablatureStaff) });
    resolver->setScope({ "NOTATION_SELECTION", TranslatableString("shortcuts", "NOTATION: SELECTION"), 6,
                         whenNotationFocused(&AppShortcutsScopes::hasSelection) });
    resolver->setScope({ "NOTATION_SINGLE_SELECTION", TranslatableString("shortcuts", "NOTATION: SINGLE SELECTION"), 7,
                         whenNotationFocused(&AppShortcutsScopes::isSingleSelection) });
    resolver->setScope({ "NOTATION_RANGE_SELECTION", TranslatableString("shortcuts", "NOTATION: RANGE SELECTION"), 7,
                         whenNotationFocused(&AppShortcutsScopes::isRangeSelection) });
    resolver->setScope({ "NOTATION_NOTE_INPUT", TranslatableString("shortcuts", "NOTATION: NOTE INPUT"), 8,
                         whenNotationFocused(&AppShortcutsScopes::isNoteInputMode) });
    resolver->setScope({ "NOTATION_TEXT_EDITING", TranslatableString("shortcuts", "NOTATION: TEXT EDITING"), 9,
                         whenNotationFocused(&AppShortcutsScopes::isTextEditing) });

    resolver->setScope({ "PROJECT", TranslatableString("shortcuts", "PROJECT") });
    resolver->setScope({ "APP", TranslatableString("shortcuts", "APP") });
    resolver->setScope({ "PALETTE", TranslatableString("shortcuts", "PALETTE") });
    resolver->setScope({ "INSTRUMENTS", TranslatableString("shortcuts", "INSTRUMENTS") });
    resolver->setScope({ "AUDIO", TranslatableString("shortcuts", "AUDIO") });
    resolver->setScope({ "DOCK", TranslatableString("shortcuts", "DOCK") });
    resolver->setScope({ "DIAGNOSTICS", TranslatableString("shortcuts", "DIAGNOSTICS") });
}

bool AppShortcutsScopes::isNotationActive(Condition condition) const
{
    if (!isNotationFocused()) {
        return false;
    }

    return !condition || (this->*condition)();
}

bool AppShortcutsScopes::isNotationFocused() const
{
    auto activePanel = navigationController()->activePanel();
    return activePanel && activePanel->name() == NOTATION_PANEL_NAME;
}

bool AppShortcutsScopes::isNoteInputMode() const
{
    return notationCommandsController()->isNoteInputMode();
}

bool AppShortcutsScopes::isTablatureStaff() const
{
    return notationCommandsController()->isTablatureStaff();
}

bool AppShortcutsScopes::hasSelection() const
{
    return notationCommandsController()->hasSelection();
}

bool AppShortcutsScopes::isSingleSelection() const
{
    return notationCommandsController()->isSingleSelection();
}

bool AppShortcutsScopes::isRangeSelection() const
{
    return notationCommandsController()->isRangeSelection();
}

bool AppShortcutsScopes::isTextEditing() const
{
    return notationCommandsController()->isTextEditing();
}
