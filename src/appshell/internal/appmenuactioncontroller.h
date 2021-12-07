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
#ifndef MU_APPSHELL_APPMENUCONTROLLER_H
#define MU_APPSHELL_APPMENUCONTROLLER_H

#include <QObject>

#include "actions/actionable.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "shortcuts/ishortcutsregister.h"
#include "actions/iactionsdispatcher.h"

#include "iappmenuactioncontroller.h"

namespace mu::appshell {
class AppMenuActionController : public IAppMenuActionController, public actions::Actionable, public async::Asyncable
{
    INJECT(appshell, shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)

public:
    void init();

    async::Channel<std::string> openMenuChannel() const override;

private:
    async::Channel<std::string> m_openMenuChannel;
};
}

#endif // MU_APPSHELL_APPLICATIONCONTROLLER_H
