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
#include <functional>

#include "shortcuts_v2/ishortcutsresolver.h"

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"
#include "notationscene/inotationcommandscontroller.h"

namespace mu::context {
class ShortcutResolver : public muse::shortcuts::IShortcutsResolver, public muse::Contextable
{
    muse::ContextInject<muse::ui::INavigationController> navigationController = { this };
    muse::ContextInject<notation::INotationCommandsController> notationCommandsController = { this };

public:
    ShortcutResolver(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    muse::shortcuts::Shortcut selectOne(const muse::shortcuts::ShortcutList& list) const override;

private:
    int scopePriority(const std::string& scope) const;

    bool isNotationFocused() const;
    bool isNotationFocusedAndNoteInputMode() const;

    mutable std::map<std::string, std::function<int()> > m_scopePriorityMap;
};
}
