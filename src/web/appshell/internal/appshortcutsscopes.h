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

#pragma once

#include <memory>

#include "modularity/ioc.h"
#include "shortcuts_v2/ishortcutsresolver.h"
#include "ui/inavigationcontroller.h"
#include "notationscene/inotationcommandscontroller.h"

namespace mu::appshell {
//! NOTE Sets the shortcut scopes of the app: their titles and priorities.
//! The scopes NOTATION* are active only when the notation is focused.
class AppShortcutsScopes : public muse::Contextable, public std::enable_shared_from_this<AppShortcutsScopes>
{
    muse::ContextInject<muse::shortcuts::IShortcutsResolver> shortcutsResolver = { this };
    muse::ContextInject<muse::ui::INavigationController> navigationController = { this };
    muse::ContextInject<notation::INotationCommandsController> notationCommandsController = { this };

public:
    AppShortcutsScopes(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();

private:
    using Condition = bool (AppShortcutsScopes::*)() const;

    bool isNotationActive(Condition condition) const;

    bool isNotationFocused() const;
    bool isNoteInputMode() const;
    bool isTablatureStaff() const;
    bool hasSelection() const;
    bool isSingleSelection() const;
    bool isRangeSelection() const;
    bool isTextEditing() const;
};
}
