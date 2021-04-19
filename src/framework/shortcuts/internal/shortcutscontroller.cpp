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

void ShortcutsController::activate(const std::string& sequence)
{
    LOGD() << sequence;

    ShortcutList shortcuts = shortcutsRegister()->shortcutsForSequence(sequence);
    IF_ASSERT_FAILED(!shortcuts.empty()) {
        return;
    }

    for (const Shortcut& sc: shortcuts) {
        ui::UiActionState st = aregister()->actionState(sc.action);
        if (!st.enabled) {
            continue;
        }

        dispatcher()->dispatch(sc.action);
    }
}
