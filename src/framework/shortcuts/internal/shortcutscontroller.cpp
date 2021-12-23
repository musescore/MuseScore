/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "shortcutscontroller.h"

#include "log.h"

using namespace mu::shortcuts;
using namespace mu::actions;

void ShortcutsController::init()
{
    interactiveProvider()->currentUri().ch.onReceive(this, [this](const Uri&) {
        //! NOTE: enable process shortcuts only for non-widget objects
        shortcutsRegister()->setActive(!interactiveProvider()->topWindowIsWidget());
    });
}

void ShortcutsController::activate(const std::string& sequence)
{
    LOGD() << sequence;

    ShortcutList shortcuts = shortcutsRegister()->shortcutsForSequence(sequence);
    IF_ASSERT_FAILED(!shortcuts.empty()) {
        return;
    }

    for (const Shortcut& sc : shortcuts) {
        //! NOTE Check if the shortcut itself is allowed
        if (!uiContextResolver()->isShortcutContextAllowed(sc.context)) {
            continue;
        }

        //! NOTE Check if the action is allowed
        ui::UiActionState st = aregister()->actionState(sc.action);
        if (!st.enabled) {
            continue;
        }

        dispatcher()->dispatch(sc.action);
    }
}

bool ShortcutsController::isRegistered(const std::string& sequence) const
{
    return shortcutsRegister()->isRegistered(sequence);
}
